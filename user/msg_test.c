#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/syscall.h"
// #include "traps.h"
#include "kernel/memlayout.h"


struct msg{
  int type;
  char *dataaddr;
}s1,s2,g;

void msg_test()
{
  int mqid = mqget(123);
  // int msg_len = 48;
  // s1.dataaddr = "total number:47 : hello, this is child process.";
  int pid = fork();
  if(pid == 0){
    s1.type = 1;
    s1.dataaddr = "This is the first message!";
    msgsnd(mqid, &s1, 27);
    s1.type = 2;
    s1.dataaddr = "Hello, another message comes!";
    msgsnd(mqid, &s1, 30);
    s1.type = 3;
    s1.dataaddr = "This is the third message, and this message has great many characters!";
    msgsnd(mqid, &s1, 70);
    // sleep(10);
    // for(int i=0; i<70; i++)
    // {
    //   s1.type = i;
    //   msgsnd(mqid, &s1, msg_len);
    // }
    printf("all messages have been sent.\n");
  } else if (pid >0)
  {
    // sleep(10);     // sleep保证子进程消息写入
    // g.dataaddr = malloc(msg_len);
    // for(int i=0; i<70; i++)
    // {
    //   g.type = i;
    //   msgrcv(mqid, &g, msg_len);
    //   printf(1, "读取第%d个消息： %s\n", i, g.dataaddr);

    // }
    sleep(10);      // sleep保证子进程消息写入
    g.dataaddr = malloc(70);
    g.type = 2;
    msgrcv(mqid,&g, 30);
    printf("receive the %dth message： %s\n", 2, g.dataaddr);
    g.type = 1;
    msgrcv(mqid,&g, 27);
    printf("receive the %dth message： %s\n", 1, g.dataaddr);
    g.type = 3;
    msgrcv(mqid,&g, 70);
    printf("receive the %dth message： %s\n", 3, g.dataaddr);

    wait(0);

  }
  exit(0);
}


int
main(int argc, char *argv[])
{
  // printf(1, "消息队列测试\n");
  msg_test();
  exit(0);
}