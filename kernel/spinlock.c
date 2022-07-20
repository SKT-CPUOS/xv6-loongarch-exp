// Mutual exclusion spin locks.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "loongarch.h"
#include "proc.h"
#include "defs.h"

void
initlock(struct spinlock *lk, char *name)
{
  lk->name = name;
  lk->locked = 0;
  lk->cpu = 0;
}

// Acquire the lock.
// Loops (spins) until the lock is acquired.
void
acquire(struct spinlock *lk)
{
  push_off(); // disable interrupts to avoid deadlock.
  if(holding(lk))
    panic("acquire");

  while(__sync_lock_test_and_set(&lk->locked, 1) != 0)
    ;

  // Tell the C compiler and the processor to not move loads or stores
  // past this point, to ensure that the critical section's memory
  // references happen strictly after the lock is acquired.
  __sync_synchronize();

  // Record info about lock acquisition for holding() and debugging.
  lk->cpu = mycpu();
}

// Release the lock.
void
release(struct spinlock *lk)
{
  if(!holding(lk))
    panic("release");

  lk->cpu = 0;

  // Tell the C compiler and the CPU to not move loads or stores
  // past this point, to ensure that all the stores in the critical
  // section are visible to other CPUs before the lock is released,
  // and that loads in the critical section occur strictly before
  // the lock is released.
  __sync_synchronize();

  // Release the lock, equivalent to lk->locked = 0.
  // This code doesn't use a C assignment, since the C standard
  // implies that an assignment might be implemented with
  // multiple store instructions.
  __sync_lock_release(&lk->locked);

  pop_off();
}

// Check whether this cpu is holding the lock.
// Interrupts must be off.
int
holding(struct spinlock *lk)
{
  int r;
  r = (lk->locked && lk->cpu == mycpu());
  return r;
}

// push_off/pop_off are like intr_off()/intr_on() except that they are matched:
// it takes two pop_off()s to undo two push_off()s.  Also, if interrupts
// are initially off, then push_off, pop_off leaves them off.

void
push_off(void)
{
  int old = intr_get();

  intr_off();
  if(mycpu()->noff == 0)
    mycpu()->intena = old;
  mycpu()->noff += 1;
}

void
pop_off(void)
{
  struct cpu *c = mycpu();
  if(intr_get())
    panic("pop_off - interruptible");
  if(c->noff < 1)
    panic("pop_off");
  c->noff -= 1;
  if(c->noff == 0 && c->intena)
    intr_on();
}


int sem_used_count = 0;
struct sem sems[SEM_MAX_NUM];

void initsem() {
  int i;
  for(i = 0; i < SEM_MAX_NUM; i++) {
    initlock(&(sems[i].lock), "semaphore");
    sems[i].allocated = 0;
  }
}

int sys_sem_create() {
  int n_sem,i;
  if(argint(0, &n_sem) < 0) {
    return -1;
  }

  for(i = 0; i < SEM_MAX_NUM; i++) {
    acquire(&sems[i].lock);
    if(sems[i].allocated == 0) {
      sems[i].allocated = 1;
      sems[i].resource_count = n_sem;
      printf("create %d sem\n",i);
      release(&sems[i].lock);
      return i;
    }
    release(&sems[i].lock);
  }

  return -1;


}

int sys_sem_free() {
  int id;
  if(argint(0, &id) < 0) {
    return -1;
  }

  acquire(&sems[id].lock);

  if(sems[id].allocated == 1 && sems[id].resource_count > 0) {
    sems[id].allocated = 0;
    printf("free %d sem\n", id);
  
  }

  release(&sems[id].lock);

  return 0;
}

int sys_sem_p() {
  int id;
  if(argint(0, &id) < 0) 
    return -1;
  acquire(&sems[id].lock);
  sems[id].resource_count--;
  if(sems[id].resource_count < 0) {
    sleep(&sems[id], &sems[id].lock);
  }
  release(&sems[id].lock);

  return 0;
}

int sys_sem_v() {
  int id;
  if(argint(0, &id) < 0)
    return -1;
    
  acquire(&sems[id].lock);
  sems[id].resource_count+=1;

  if(sems[id].resource_count < 1) {
    wakeup1p(&sems[id]);
  }

  release(&sems[id].lock);
  return 0;
}