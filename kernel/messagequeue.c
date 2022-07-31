#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "loongarch.h"
#include "proc.h"
#include "spinlock.h"

struct msg {
    struct msg *next;
    long type;
    char *dataaddr;
    int  datasize;
};

struct mq {
    int key;
    int status;
    struct msg *msgs;
    int maxbytes;
    int curbytes;
    int refcount;
};

struct spinlock mqlock;
struct mq mqs[MQMAX];
struct proc* wqueue[NPROC];
int wstart=0;

struct proc* rqueue[NPROC];
int rstart=0;

void
mqinit()
{
    printf("mqinit.\n");
    initlock(&mqlock,"mqlock");
    for(int i =0;i<MQMAX;++i){
        mqs[i].status = 0;
    }
}

int findkey(int key)
{
    int idx =-1;
    for(int i = 0;i<MQMAX;++i){
        if(mqs[i].status != 0 && mqs[i].key == key){
            idx = i;
            break;
        }
    }
    return idx;
}

int newmq(int key)
{
    int idx =-1;
    for(int i=0;i<MQMAX;++i){
        if(mqs[i].status == 0){
            idx = i;
            break;
        }
    }
    if(idx == -1){
        printf("newmq failed: can not get idx.\n");
        return -1;
    }
    mqs[idx].key = key;
    mqs[idx].status = 1;
    mqs[idx].msgs = (struct msg*)kalloc();
    if(mqs[idx].msgs == 0){
        printf("newmq failed: can not alloc page.\n");
        return -1;
    }
    memset(mqs[idx].msgs,0,PGSIZE);
    mqs[idx].msgs -> next = 0;
    mqs[idx].msgs -> datasize = 0;
    mqs[idx].maxbytes = PGSIZE;
    mqs[idx].curbytes = 16;
    mqs[idx].refcount = 1;
    proc->mqmask |= 1 << idx;
    return idx;

}

int
mqget(uint key)
{

    
    acquire(&mqlock);
    int idx = findkey(key);
    if(idx != -1){
        if(!(proc->mqmask >> idx & 1)){
            proc->mqmask |= 1 << idx;
            mqs[idx].refcount++;
        }
        release(&mqlock);
        return idx;
    }
    idx = newmq(key);
    release(&mqlock);
    return idx;
}
int
msgsnd(uint mqid, void* msg, int sz)
{
    if(mqid<0 || MQMAX<=mqid || mqs[mqid].status == 0){
        return -1;
    }

    char *data = (char *)(*((int *) (msg + 4)));
    int  *type = (int *)msg;

    if(mqs[mqid].msgs == 0){
        printf("msgsnd failed: msgs == 0.\n");
        return -1;
    }

    acquire(&mqlock);

    while(1){
        if(mqs[mqid].curbytes + sz + 16 <= mqs[mqid].maxbytes){
            struct msg *m = mqs[mqid].msgs;
            while(m->next != 0){
                m = m -> next;
            }
            m->next = (void *)m + m->datasize + 16;
            m = m -> next;
            m->type = *(type);
            m->next = 0;
            m->dataaddr = (void*)m + 16;
            m->datasize = sz;
            memmove(m->dataaddr, data, sz);
            mqs[mqid].curbytes += (sz+16);

            for(int i=0; i<rstart; i++)
            {
                wakeup(rqueue[i]);
            }
            rstart = 0;

            release(&mqlock);
            return 0;
        } else {
            printf("msgsnd: can not alloc: pthread: %d sleep.\n",proc->pid);
            wqueue[wstart++] = proc;

            sleep(proc,&mqlock);
        }
        
    }

    return -1;
}



int reloc(int mqid)
{
    struct msg *pages = mqs[mqid].msgs;
    struct msg *m  = pages;
    struct msg *t;
    struct msg *pre = pages;
    while (m != 0)
    {
        t = m->next;
        memmove(pages, m, m->datasize+16);
        pages->next = (struct msg *)((char *)pages + pages->datasize + 16);
        pages->dataaddr = ((char *)pages + 16);
        pre = pages;
        pages = pages->next;
        m = t;
    }
    pre->next = 0;
    return 0;
}



int
msgrcv(uint mqid, void* msg, int sz)
{
    if(mqid<0 || MQMAX<=mqid || mqs[mqid].status ==0){
        return -1;
    }
    int *type = msg;
    int *data = msg + 4;
    acquire(&mqlock);
    
    while(1){
        struct msg *m = mqs[mqid].msgs->next;
        struct msg *pre = mqs[mqid].msgs;
        while (m != 0)
        {
            if(m->type == *type){
                memmove((char *)*data, m->dataaddr, sz);
                pre->next = m->next;
                mqs[mqid].curbytes -= (m->datasize + 16);
                reloc(mqid);

                for(int i=0; i<wstart; i++)
                {
                    wakeup(wqueue[i]);
                }
                wstart = 0;

                release(&mqlock);
                return 0;
            }
            pre = m;
            m = m->next;
        }
        printf("msgrcv: can not read: pthread: %d sleep.\n",proc->pid);
        rqueue[rstart++] = proc;
        sleep(proc,&mqlock);
    }
    return -1;
}

void
rmmq(int mqid)
{
    kfree((char *)mqs[mqid].msgs);
    mqs[mqid].status = 0;
}

void
releasemq2(int mask)
{
    acquire(&mqlock);
    for(int id = 0;id<MQMAX;++id){
        if( mask >> id & 0x1){
            mqs[id].refcount--;
            if(mqs[id].refcount == 0){
                rmmq(id);
            }
        }
    }
    release(&mqlock);
}


void
releasemq(uint key)
{
    //cprintf("releasemq: %d.\n",key);
  int idx= findkey(key);
  if (idx!=-1){
      acquire(&mqlock);
          mqs[idx].refcount--;   //引用数目减1
            if(mqs[idx].refcount == 0)  //引用数目为0时候需要回收物理内存
                rmmq(idx);
       release(&mqlock);
     }
}



void
addmqcount(uint mask)
{
    acquire(&mqlock);
    for (int key = 0; key < MQMAX; key++)
    {
        if(mask >> key & 1){
            mqs[key].refcount++;
        }
    }
    release(&mqlock);
}

