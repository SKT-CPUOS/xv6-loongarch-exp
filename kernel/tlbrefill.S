#include "asm.h"

.section tlbrentry
.globl handle_tlbr
.align 0x4
handle_tlbr:
	csrwr	$t0, LOONGARCH_CSR_TLBRSAVE
	csrrd	$t0, LOONGARCH_CSR_PGD
//	lddir	$t0, $t0, 4
//	addi.d  $t0, $t0, -1
	lddir	$t0, $t0, 3
	addi.d  $t0, $t0, -1
	lddir	$t0, $t0, 2
	addi.d  $t0, $t0, -1
	lddir   $t0, $t0, 1
	addi.d  $t0, $t0, -1
	ldpte	$t0, 0
	ldpte	$t0, 1
	tlbfill
	csrrd	$t0, LOONGARCH_CSR_TLBRSAVE
	ertn
	
