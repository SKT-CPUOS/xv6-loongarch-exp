#include "kernel/fcntl.h"
#include "kernel/types.h"
#include "user/uthread.h"
#include "user/user.h"
volatile int global = 1;
int F(int n) {
  if (n < 0)
    printf("input a positive integer\n");
  else if (n == 1 || n == 2)
    return 1;
  else {
    return F(n - 1) + F(n - 2);
  }
  return 0;
}
void worker(void* arg) {
    global = 100;
    printf("thread %d is worker.\n", *(int *)arg);

    global = F(15);
    write(3, "hello\n", 6);
    exit(0);
}

int main(){
    int t = 1;
    open("tmp", O_RDWR | O_CREATE);
    int pid = thread_create(worker, &t);
    sleep(10);
    thread_join();
    printf("thread id = %d\n", pid);
    printf("global = %d\n", global);
    
    exit(0);
}