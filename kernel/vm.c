#include "param.h"
#include "types.h"
#include "memlayout.h"
#include "elf.h"
#include "loongarch.h"
#include "defs.h"
#include "fs.h"
#include "spinlock.h"
#include "proc.h"

void
tlbinit(void)
{
  asm volatile("invtlb  0x0,$zero,$zero");
  w_csr_stlbps(0xcU);
  w_csr_asid(0x0U);//todo
  w_csr_tlbrehi(0xcU);
}

void
vminit(void)//todo
{
  pagetable_t kpgtbl;

  kpgtbl = (pagetable_t) kalloc();
  memset(kpgtbl, 0, PGSIZE);
  proc_mapstacks(kpgtbl);

  w_csr_pgdl((uint64)kpgtbl);
  tlbinit();

  w_csr_pwcl((PTEWIDTH << 30)|(DIR2WIDTH << 25)|(DIR2BASE << 20)|(DIR1WIDTH << 15)|(DIR1BASE << 10)|(PTWIDTH << 5)|(PTBASE << 0));
  w_csr_pwch((DIR4WIDTH << 18)|(DIR3WIDTH << 6)|(DIR3BASE << 0));
}

// Return the address of the PTE in page table pagetable
// that corresponds to virtual address va.  If alloc!=0,
// create any required page-table pages.
//
// The paging scheme has four levels of page-table
// pages. A page-table page contains 512 64-bit PTEs.
// A 64-bit virtual address is split into five fields:
//   48..63 -- must be zero.
//   39..47 -- 9 bits of level-3 index.
//   30..38 -- 9 bits of level-2 index.
//   21..29 -- 9 bits of level-1 index.
//   12..20 -- 9 bits of level-0 index.
//    0..11 -- 12 bits of byte offset within the page.
pte_t *
walk(pagetable_t pagetable, uint64 va, int alloc)
{
  if(va >= MAXVA)
    panic("walk");

  for(int level = 3; level > 0; level--) {
    pte_t *pte = &pagetable[PX(level, va)];
    if(*pte & PTE_V) {
      pagetable = (pagetable_t)(PTE2PA(*pte) | DMWIN_MASK);      
    } else {
      if(!alloc || (pagetable = (pde_t*)kalloc()) == 0)
        return 0;
      memset(pagetable, 0, PGSIZE);
      *pte = PA2PTE(pagetable) | PTE_V;
    }
  }
  return &pagetable[PX(0, va)];
}

// Look up a virtual address, return the physical address,
// or 0 if not mapped.
// Can only be used to look up user pages.
uint64
walkaddr(pagetable_t pagetable, uint64 va)
{
  pte_t *pte;
  uint64 pa;

  if(va >= MAXVA)
    return 0;

  pte = walk(pagetable, va, 0);
  if(pte == 0)
    return 0;
  if((*pte & PTE_V) == 0)
    return 0;
  if((*pte & PTE_PLV) == 0)
    return 0;
  pa = PTE2PA(*pte);
  return pa;
}

// Create PTEs for virtual addresses starting at va that refer to
// physical addresses starting at pa. va and size might not
// be page-aligned. Returns 0 on success, -1 if walk() couldn't
// allocate a needed page-table page.
int
mappages(pagetable_t pagetable, uint64 va, uint64 size, uint64 pa, uint64 perm)
{
  uint64 a, last;
  pte_t *pte;

  if(size == 0)
    panic("mappages: size");
  
  a = PGROUNDDOWN(va);
  last = PGROUNDDOWN(va + size - 1);
  for(;;){
    if((pte = walk(pagetable, a, 1)) == 0)
      return -1;
    if(*pte & PTE_V)
      panic("mappages: remap");
    *pte = PA2PTE(pa) | perm | PTE_V;
    if(a == last)
      break;
    a += PGSIZE;
    pa += PGSIZE;
  }
  return 0;
}

// Remove npages of mappings starting from va. va must be
// page-aligned. The mappings must exist.
// Optionally free the physical memory.
void
uvmunmap(pagetable_t pagetable, uint64 va, uint64 npages, int do_free)
{
  uint64 a;
  pte_t *pte;

  if((va % PGSIZE) != 0)
    panic("uvmunmap: not aligned");

  for(a = va; a < va + npages*PGSIZE; a += PGSIZE){
    if((pte = walk(pagetable, a, 0)) == 0) {
      continue;
    }
      // panic("uvmunmap: walk");
    if(*pte == 0) {
      continue;
    }
    if((*pte & PTE_V) == 0)
      panic("uvmunmap: not mapped");
    if(PTE_FLAGS(*pte) == PTE_V)
      panic("uvmunmap: not a leaf");
    if(do_free){
      if((*pte & PTE_SWAPPED) == 0) {
        uint64 pa = PTE2PA(*pte);    
        kfree((void*)(pa | DMWIN_MASK));
      }
      
    }
    *pte = 0;
  }
}

// create an empty user page table.
// returns 0 if out of memory.
pagetable_t
uvmcreate()
{
  pagetable_t pagetable;
  pagetable = (pagetable_t) kalloc();
  if(pagetable == 0)
    return 0;
  memset(pagetable, 0, PGSIZE);
  return pagetable;
}

// Load the user initcode into address 0 of pagetable,
// for the very first process.
// sz must be less than a page.
void
uvminit(pagetable_t pagetable, uchar *src, uint sz)
{
  char *mem;

  if(sz >= PGSIZE)
    panic("inituvm: more than a page");
  mem = kalloc();
  memset(mem, 0, PGSIZE);
  mappages(pagetable, 0, PGSIZE, (uint64)mem, PTE_P|PTE_W|PTE_PLV|PTE_MAT);
  memmove(mem, src, sz);
}//todo

// Allocate PTEs and physical memory to grow process from oldsz to
// newsz, which need not be page aligned.  Returns new size or 0 on error.
uint64
uvmalloc(pagetable_t pagetable, uint64 oldsz, uint64 newsz)
{
  char *mem;
  uint64 a;

  if(newsz < oldsz)
    return oldsz;

  oldsz = PGROUNDUP(oldsz);
  for(a = oldsz; a < newsz; a += PGSIZE){
    mem = kalloc();
    if(mem == 0){
      uvmdealloc(pagetable, a, oldsz);
      return 0;
    }
    memset(mem, 0, PGSIZE);
    if(mappages(pagetable, a, PGSIZE, (uint64)mem, PTE_P|PTE_W|PTE_PLV|PTE_MAT|PTE_D) != 0){
      kfree(mem);
      uvmdealloc(pagetable, a, oldsz);
      return 0;
    }
  }
  return newsz;
}

// Deallocate user pages to bring the process size from oldsz to
// newsz.  oldsz and newsz need not be page-aligned, nor does newsz
// need to be less than oldsz.  oldsz can be larger than the actual
// process size.  Returns the new process size.
uint64
uvmdealloc(pagetable_t pagetable, uint64 oldsz, uint64 newsz)
{
  if(newsz >= oldsz)
    return oldsz;

  if(PGROUNDUP(newsz) < PGROUNDUP(oldsz)){
    int npages = (PGROUNDUP(oldsz) - PGROUNDUP(newsz)) / PGSIZE;
    uvmunmap(pagetable, PGROUNDUP(newsz), npages, 1);
  }

  return newsz;
}

// Recursively free page-table pages.
// All leaf mappings must already have been removed.
void
freewalk(pagetable_t pagetable)
{
  // there are 2^9 = 512 PTEs in a page table.
  for(int i = 0; i < 512; i++){
    pte_t pte = pagetable[i];
    if((pte & PTE_V) && (PTE_FLAGS(pte) == PTE_V)){
      // this PTE points to a lower-level page table.
      uint64 child = (PTE2PA(pte) | DMWIN_MASK);
      freewalk((pagetable_t)child);
      pagetable[i] = 0;
    } else if(pte & PTE_V){
      panic("freewalk: leaf");
    }
  }
  kfree((void*)pagetable);
}

// Free user memory pages,
// then free page-table pages.
void
uvmfree(pagetable_t pagetable, uint64 sz)
{
  if(sz > 0)
    uvmunmap(pagetable, 0, PGROUNDUP(sz)/PGSIZE, 1);
  freewalk(pagetable);
}

// Given a parent process's page table, copy
// its memory into a child's page table.
// Copies both the page table and the
// physical memory.
// returns 0 on success, -1 on failure.
// frees any allocated pages on failure.
int
uvmcopy(pagetable_t old, pagetable_t new, uint64 sz)
{
  pte_t *pte;
  uint64 pa, i;
  uint flags;
  char *mem;

  for(i = 0; i < sz; i += PGSIZE){
    if((pte = walk(old, i, 0)) == 0)
      panic("uvmcopy: pte should exist");
    if((*pte & PTE_V) == 0)
      panic("uvmcopy: page not present");
    pa = PTE2PA(*pte);
    flags = PTE_FLAGS(*pte);
    if((mem = kalloc()) == 0)
      goto err;
    memmove(mem, (char*)(pa | DMWIN_MASK), PGSIZE);
    if(mappages(new, i, PGSIZE, (uint64)mem, flags) != 0){
      kfree(mem);
      goto err;
    }
  }
  return 0;

 err:
  uvmunmap(new, 0, i / PGSIZE, 1);
  return -1;
}

// mark a PTE invalid for user access.
// used by exec for the user stack guard page.
void
uvmclear(pagetable_t pagetable, uint64 va)
{
  pte_t *pte;
  
  pte = walk(pagetable, va, 0);
  if(pte == 0)
    panic("uvmclear");
  *pte &= ~PTE_PLV;
}

// Copy from kernel to user.
// Copy len bytes from src to virtual address dstva in a given page table.
// Return 0 on success, -1 on error.
int
copyout(pagetable_t pagetable, uint64 dstva, char *src, uint64 len)
{
  uint64 n, va0, pa0;

  while(len > 0){
    va0 = PGROUNDDOWN(dstva);
    pa0 = walkaddr(pagetable, va0);
    if(pa0 == 0)
      return -1;
    n = PGSIZE - (dstva - va0);
    if(n > len)
      n = len;
    memmove((void *)((pa0 + (dstva - va0)) | DMWIN_MASK), src, n);

    len -= n;
    src += n;
    dstva = va0 + PGSIZE;
  }
  return 0;
}//todo

// Copy from user to kernel.
// Copy len bytes to dst from virtual address srcva in a given page table.
// Return 0 on success, -1 on error.
int
copyin(pagetable_t pagetable, char *dst, uint64 srcva, uint64 len)
{
  uint64 n, va0, pa0;

  while(len > 0){
    va0 = PGROUNDDOWN(srcva);
    pa0 = walkaddr(pagetable, va0);
    if(pa0 == 0)
      return -1;
    n = PGSIZE - (srcva - va0);
    if(n > len)
      n = len;
    memmove(dst, (void *)((pa0 + (srcva - va0)) | DMWIN_MASK), n);

    len -= n;
    dst += n;
    srcva = va0 + PGSIZE;
  }
  return 0;
}//todo

// Copy a null-terminated string from user to kernel.
// Copy bytes to dst from virtual address srcva in a given page table,
// until a '\0', or max.
// Return 0 on success, -1 on error.
int
copyinstr(pagetable_t pagetable, char *dst, uint64 srcva, uint64 max)
{
  uint64 n, va0, pa0;
  int got_null = 0;

  while(got_null == 0 && max > 0){
    va0 = PGROUNDDOWN(srcva);
    pa0 = walkaddr(pagetable, va0);
    if(pa0 == 0)
      return -1;
    n = PGSIZE - (srcva - va0);
    if(n > max)
      n = max;

    char *p = (char *) ((pa0 + (srcva - va0)) | DMWIN_MASK);
    while(n > 0){
      if(*p == '\0'){
        *dst = '\0';
        got_null = 1;
        break;
      } else {
        *dst = *p;
      }
      --n;
      --max;
      p++;
      dst++;
    }

    srcva = va0 + PGSIZE;
  }
  if(got_null){
    return 0;
  } else {
    return -1;
  }
}//todo

/*
1.查看该进程的页表
2.找一个有映射的页表项
3.将其对应的页帧的内容，持久到文件系统
4.释放其页帧
*/
void 
swapout(struct proc *p) { // 换出一个物理页帧
    pte_t *pte;
    pagetable_t pgdir = p->pagetable;
    uint64 a = p->sz - 1; // 起始地址
    a = PGROUNDDOWN(a); // 向下取整
    


    for(; a >= p->swap_start; a -= PGSIZE) {
      pte = walk(pgdir, a, 0);
      if(*pte  & PTE_P && ((*pte & PTE_SWAPPED) == 0 )) { // 找到一个映射页,并且没有被换出

        uint64 pa = walkaddr(pgdir, a); 

        uint blockno = balloc4(1); // 申请 4 个盘块

        write_page_to_disk(1, (void *)pa, blockno); // 写入磁盘
        
        kfree((void *)(pa | DMWIN_MASK));//释放对应的页帧


        *pte = (blockno << 12); // 记录盘块号
        *pte = (*pte | PTE_SWAPPED);//将其swapped位置1

        return;
      }
    }
 }

void
swapin(char* mem, pte_t *pte){ // 线性地址、页表项
  uint blockno = ((uint)*pte >> 12); // 取磁盘号
  read_page_from_disk(1, mem, blockno); 
}
// struct  
// {
//   struct spinlock lock; 
//   int use_lock;
// }mylock; //防止多个进程，对同一个页帧进行抢夺，出现的同步问题


void
pgfault()
{
  uint64 addr = PGROUNDDOWN(r_csr_badv()); // 获取导致中断的线性地址,要取整

  
  struct proc *proc = myproc();

  char* mem;
  

 
  if(r_csr_badv() > proc->sz) { // 越界
    printf("kalloc out of memory!  %p  %d\n",addr,proc->pid);
    proc->killed = 1;
    return;
  } 

 

  mem = kalloc(); // 分配一块物理页帧
  while(mem == 0) { // 没有页帧了
    printf("执行换出操作\n");

    swapout(myproc()); // 进程空间找个页扔出去
    mem = kalloc(); // 再分配一次
  }

  pte_t* pte = walk(proc->pagetable, addr, 1);//查看va的页表项
  
 
  if((*pte & PTE_SWAPPED) == 0) { // 第一次分配
    memset(mem, 0, PGSIZE);
  }
  else { // 已分配但被换出的页
    printf("将物理页从磁盘调入内存\n");
    swapin(mem, pte); // 根据 pte 的盘块号将数据取回来
  }

  *pte = (*pte & (~PTE_V));
  // 最后建立映射
  mappages(proc->pagetable, addr, PGSIZE, (uint64)mem, PTE_PLV | PTE_P | PTE_W | PTE_MAT | PTE_D);
}
