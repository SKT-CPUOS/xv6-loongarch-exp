# Initial process that execs /init.
# This code runs in user space.

#include "syscall.h"

# exec(init, argv)
.globl start
start:
        la   $a0, init
        la   $a1, argv
        li.d $a7, SYS_exec
        syscall 0

# for(;;) exit();
exit:
        li.d $a7, SYS_exit
        syscall 0
        bl exit

# char init[] = "/init\0";
init:
  .string "/init\0"

# char *argv[] = { init, 0 };
.p2align 2
argv:
  .long init
  .long 0
