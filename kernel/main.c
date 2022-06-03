#include "types.h"
#include "param.h"
#include "loongarch.h"
#include "defs.h"
#include "memlayout.h"

// entry.S needs one stack per CPU.
__attribute__ ((aligned (16))) char stack0[4096 * NCPU];

volatile static int started = 0;

// entry.S jumps here on stack0.
void
main()
{
   if(cpuid() == 0){
    consoleinit();
    printfinit();
    
    kinit();         // physical page allocator
//printf("kinit\n");
    vminit();        // create kernel page table
//printf("vminit\n");
    procinit();      // process table
//printf("procinit\n");
    trapinit();      // trap vectors
//printf("trapinit\n");
    apic_init();     // set up LS7A1000 interrupt controller
//printf("apicinit\n");
    extioi_init();   // extended I/O interrupt controller
//printf("extioi_init\n");
    binit();         // buffer cache
//printf("binit\n");
    iinit();         // inode table
//printf("iinit\n");
    fileinit();      // file table
//printf("fileinit\n");
    ramdiskinit();   // emulated hard disk
//printf("ramdiskinit\n");
    userinit();      // first user process
//printf("userinit\n");
    __sync_synchronize();
    started = 1;
  } else {
    while(started == 0)
      ;
    __sync_synchronize();
    printf("hart %d starting\n", cpuid());
  }
    scheduler(); 
}

