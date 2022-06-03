.section merrentry
.globl machine_trap
.globl handle_merr
.align 0x4
handle_merr:
    bl machine_trap
    ertn