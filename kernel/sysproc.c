#include "types.h"
#include "loongarch.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "messagequeue.h"


uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

//messagequeue.c
int
sys_mqget(void)
{
  int key;
  if(argint(0,&key)<0)
    return -1;
  return mqget(key);
}

// struct msg{
//   int type;
//   char *dataaddr;
// }s1;

int
sys_msgsnd(void)
{
  int mqid;
  
  int sz;

  int type;
  char* msg = "";
  if(argint(0, &mqid) < 0 || argint(1, &type) < 0 || argint(2, &sz) < 0 || argstr(3,msg,sz)<0)
    return -1;


  // printf("type:%d\n", type);
  // printf("msg:%s", msg);
  
  return msgsnd(mqid,type,sz, msg);
}

int
sys_msgrcv(void)
{
  int mqid;
  int sz;
  int type;
  // char* msg = ;
  uint64 addr;
  if(argint(0, &mqid) < 0 || argint(1, &type) < 0 || argint(2, &sz) < 0 || argaddr(3, &addr) < 0)
    return -1;
  return msgrcv(mqid,type,sz, addr); 
}

