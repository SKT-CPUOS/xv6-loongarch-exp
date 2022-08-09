#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


int
main(int argc, char *argv[]) {
  // int pid = getpid();   
  // map(pid);
  
  char* m1 = (char*)myalloc(2 * 4096);
  char* m2 = (char*)myalloc(3 * 4096);
  char* m3 = (char*)myalloc(1 * 4096);
  char* m4 = (char*)myalloc(7 * 4096);
  char* m5 = (char*)myalloc(9 * 4096);

  m1[0] = 'a';
  m1[1] = '\0';
  m2[2] = 'b';
  m3[2] = 'b';
  m4[2] = 'b';
  m5[2] = 'b';

  printf("m1:%s\n",m1);
  myfree((uint64)m2);


//  尝试往空洞写数据 
//   m2[1] = 'p';

  myfree((uint64)m4);
  
  // map(pid);
  sleep(5000);
  myfree((uint64)m1);
  myfree((uint64)m3);
  myfree((uint64)m5);



  exit(0);
}