#include "kernel/types.h"
#include "user/user.h"
#define NTHREAD 4
#define PGSIZE 4096
struct {
  int pid;
  void *ustack;
  int used;
} threads[NTHREAD];

void add_thread(int *pid, void *ustack) {
  for (int i = 0; i < NTHREAD; i++) {
    if (threads[i].used == 0) {
      threads[i].pid = *pid;
      threads[i].ustack = ustack;
      threads[i].used = 1;
      break;
    }
  }
}

void remove_thread(int *pid) {
  for (int i = 0; i < NTHREAD; i++) {
    if (threads[i].used && threads[i].pid == *pid) {
      free(threads[i].ustack);
      threads[i].pid = 0;
      threads[i].ustack = 0;
      threads[i].used = 0;
      break;
    }
  }
}

int thread_create(void (*start_routine)(void *), int* arg) {
  static int first = 1;
  if (first) {
    first = 0;
    for (int i = 0; i < NTHREAD; i++) {
      threads[i].pid = 0;
      threads[i].ustack = 0;
      threads[i].used = 0;
    }
  }

  void *stack = malloc(PGSIZE);

  int pid = clone(start_routine, stack, (void *)arg);
  add_thread(&pid, stack);
  return pid;
}

int thread_join(void) {
  for (int i = 0; i < NTHREAD; i++) {
    if (threads[i].used == 1) {
      int pid = join();
      if (pid > 0) {
        remove_thread(&pid);
        return pid;
      }
    }
  }
  return 0;
}

void printTCB(void) {
  for (int i = 0; i < NTHREAD; i++)
    printf("TCB %d: %d\n", i, threads[i].used);
}

