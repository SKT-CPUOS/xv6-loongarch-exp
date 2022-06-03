# code to switch between user and kernel space.

	# kernel.ld causes this to be aligned
        # to a page boundary.
        #
#include "asm.h"

.section u_eentry
.globl uservec
.align 4

uservec:    
	#
        # trap.c sets eentry to point here, so
        # traps from user space start here,
        # in privilege0, but with a
        # user page table.
        #
        # CSR.SAVE points to where the process's p->trapframe is
        # mapped into user space, at TRAPFRAME.
        #
        
	# swap a0 and SAVE
        # so that a0 is TRAPFRAME
        csrwr  $a0, LOONGARCH_CSR_SAVE0

        # save the user registers in TRAPFRAME
        st.d   $ra, $a0, 0
        st.d   $tp, $a0, 8
        st.d   $sp, $a0, 16
        st.d   $a1, $a0, 32
        st.d   $a2, $a0, 40
        st.d   $a3, $a0, 48
        st.d   $a4, $a0, 56
        st.d   $a5, $a0, 64
        st.d   $a6, $a0, 72
        st.d   $a7, $a0, 80
        st.d   $t0, $a0, 88
        st.d   $t1, $a0, 96
        st.d   $t2, $a0, 104
        st.d   $t3, $a0, 112
        st.d   $t4, $a0, 120
        st.d   $t5, $a0, 128
        st.d   $t6, $a0, 136
        st.d   $t7, $a0, 144
        st.d   $t8, $a0, 152
        st.d   $r21, $a0,160
        st.d   $fp, $a0, 168
        st.d   $s0, $a0, 176
        st.d   $s1, $a0, 184
        st.d   $s2, $a0, 192
        st.d   $s3, $a0, 200
        st.d   $s4, $a0, 208
        st.d   $s5, $a0, 216
        st.d   $s6, $a0, 224
        st.d   $s7, $a0, 232
        st.d   $s8, $a0, 240

	# save the user a0 in p->trapframe->a0
        csrrd  $t0, LOONGARCH_CSR_SAVE0
        st.d   $t0, $a0, 24

        # restore kernel stack pointer from p->trapframe->kernel_sp
        ld.d   $sp, $a0, 248

        # make tp hold the current hartid, from p->trapframe->kernel_hartid
        ld.d   $tp, $a0, 272

        # load the address of usertrap(), p->trapframe->kernel_trap
        ld.d   $t0, $a0, 256

        # restore kernel page table from p->trapframe->kernel_pgdl
        ld.d   $t1, $a0, 280
        csrwr  $t1, LOONGARCH_CSR_PGDL
        invtlb 0x0,$zero,$zero

        # jump to usertrap(), which does not return
        jirl   $zero ,$t0, 0

.globl userret
userret:
        # userret(TRAPFRAME, pagetable)
        # switch from kernel to user.
        # usertrapret() calls here.
        # a0: TRAPFRAME, in user page table.
        # a1: user page table, for pgdl.

        # switch to the user page table.
        csrwr  $a1, LOONGARCH_CSR_PGDL
        invtlb 0x0,$zero,$zero

        # put the saved user a0 in SAVE0, so we
        # can swap it with our a0 (TRAPFRAME) in the last step.
        ld.d   $t0, $a0, 24
        csrwr  $t0, LOONGARCH_CSR_SAVE0

        # restore all but a0 from TRAPFRAME
        ld.d   $ra, $a0, 0
        ld.d   $tp, $a0, 8
        ld.d   $sp, $a0, 16
        ld.d   $a1, $a0, 32
        ld.d   $a2, $a0, 40
        ld.d   $a3, $a0, 48
        ld.d   $a4, $a0, 56
        ld.d   $a5, $a0, 64
        ld.d   $a6, $a0, 72
        ld.d   $a7, $a0, 80
        ld.d   $t0, $a0, 88
        ld.d   $t1, $a0, 96
        ld.d   $t2, $a0, 104
        ld.d   $t3, $a0, 112
        ld.d   $t4, $a0, 120
        ld.d   $t5, $a0, 128
        ld.d   $t6, $a0, 136
        ld.d   $t7, $a0, 144
        ld.d   $t8, $a0, 152
        ld.d   $r21, $a0,160
        ld.d   $fp, $a0, 168
        ld.d   $s0, $a0, 176
        ld.d   $s1, $a0, 184
        ld.d   $s2, $a0, 192
        ld.d   $s3, $a0, 200
        ld.d   $s4, $a0, 208
        ld.d   $s5, $a0, 216
        ld.d   $s6, $a0, 224
        ld.d   $s7, $a0, 232
        ld.d   $s8, $a0, 240

	# restore user a0, and save TRAPFRAME in SAVE0
        csrwr  $a0, LOONGARCH_CSR_SAVE0
        
        # return to user mode and user pc.
        # usertrapret() set up prmd and era.
        ertn
