// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.

#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "loongarch.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"
#include "memlayout.h"

struct {
  struct spinlock lock;
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  struct buf head;
} bcache;

void
binit(void)
{
  struct buf *b;

  initlock(&bcache.lock, "bcache");

  // Create linked list of buffers
  bcache.head.prev = &bcache.head;
  bcache.head.next = &bcache.head;
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    b->next = bcache.head.next;
    b->prev = &bcache.head;
    initsleeplock(&b->lock, "buffer");
    bcache.head.next->prev = b;
    bcache.head.next = b;
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;

  acquire(&bcache.lock);

  // Is the block already cached?
  for(b = bcache.head.next; b != &bcache.head; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.lock);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  for(b = bcache.head.prev; b != &bcache.head; b = b->prev){
    if(b->refcnt == 0) {
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;
      release(&bcache.lock);
      acquiresleep(&b->lock);
      return b;
    }
  }
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    ramdiskrw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  ramdiskrw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  acquire(&bcache.lock);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = bcache.head.next;
    b->prev = &bcache.head;
    bcache.head.next->prev = b;
    bcache.head.next = b;
  }
  
  release(&bcache.lock);
}

void
bpin(struct buf *b) {
  acquire(&bcache.lock);
  b->refcnt++;
  release(&bcache.lock);
}

void
bunpin(struct buf *b) {
  acquire(&bcache.lock);
  b->refcnt--;
  release(&bcache.lock);
}


// blockno 起始的连续 4 块盘块数据拷贝到 va 起始的物理页帧中
void read_page_from_disk(int dev, char *pa, uint blockno)
{
  // printf("从磁盘换入\n");
  struct buf* b; // xv6 的读写必须经过缓存块
    for(int i = 0; i < 4; i ++) { // 物理页帧分 4 片存入 4 个盘块
      b = bread(dev, blockno + i); // 将磁盘数据读到缓存块
      // cprintf("读出来吗？:%d",b->data[1]);
      memmove((void *)(pa + i * 1024), b->data, 1024); // 将缓存块数据写入物理页帧
      brelse(b); // 释放缓存块
    }
}



// 将 4096 字节的物理页帧写到 blockno 起始的连续 8 块盘块中
 void write_page_to_disk(int dev, char *pa, uint blockno) 
 {
  begin_op();
  // begin_op();
 	struct buf* b;
  for(int i = 0; i < 4; i ++) 
 	{
    // printf("写出到磁盘\n");
 		b = bread(dev, blockno + i); // 获取设备 1 上第 blockno+i 个盘块
    memmove(b->data, (void *)((uint64)(pa + i * 1024) | DMWIN_MASK), 1024); // 将数据移动到物理内存上
    // memset(b->data, 0, PGSIZE);
    bwrite(b);
    // log_write(b); // 写磁盘
    // log_write(b);
    brelse(b); // 释放缓存块
  }

  end_op();
 }

