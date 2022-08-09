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
  int mqid = mqget(123);        //使用消息队列
  // int msg_len = 48;
  // s1.dataaddr = "total number:47 : hello, this is child process.";
  int pid = fork();
  if(pid == 0){               //子进程
    s1.type = 1;
    s1.dataaddr = "This is the first message!\n";



    msgsnd(mqid, s1.type, 28, s1.dataaddr); //发送消息1

    s1.type = 2;
    s1.dataaddr = "Hello, another message comes!\n";
    msgsnd(mqid, s1.type, 31, s1.dataaddr); //发送消息2
    s1.type = 3;
    s1.dataaddr = "This is the third message, and this message has great many characters!\n";
    msgsnd(mqid, s1.type, 72, s1.dataaddr); //发送消息3

    printf("all messages have been sent.\n");
  } else if (pid >0)          //以下是父进程
  {

    // sleep(10);      // sleep保证子进程消息写入
    g.dataaddr = malloc(70);
    g.type = 2;
    msgrcv(mqid,g.type, 31, g.dataaddr);    //读入消息2
    printf("receive the %dth message： %s\n", 2, g.dataaddr);
    g.type = 1;
    msgrcv(mqid,g.type, 28, g.dataaddr);    //读入消息1
    printf("receive the %dth message： %s\n", 1, g.dataaddr);
    g.type = 3;
    msgrcv(mqid,g.type, 70, g.dataaddr);    //读入消息3
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