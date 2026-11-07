/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * U-Boot is started by the vendor's IPL_CUST, and skips the CP15 setup
 * (SKIP_LOWLEVEL_INIT). Ifado's IPL_CUST jumps with the MMU and the data
 * cache still on, its page table at 0x20004000, where the kernel later
 * builds its own: clean the data cache and turn both off before anything
 * runs, as cpu_init_cp15 would. Where the MMU is already off this is a
 * branch.
 */
#include <linux/linkage.h>

ENTRY(save_boot_params)
	mrc	p15, 0, r0, c1, c0, 0		@ SCTLR
	tst	r0, #(1 << 0)			@ M
	beq	1f
	push	{r1-r12, lr}			@ IPL_CUST's stack
	bl	v7_flush_dcache_all
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #(1 << 2)		@ C
	bic	r0, r0, #(1 << 0)		@ M
	mcr	p15, 0, r0, c1, c0, 0
	isb
	mov	r0, #0
	mcr	p15, 0, r0, c8, c7, 0		@ TLBIALL
	mcr	p15, 0, r0, c7, c5, 0		@ ICIALLU
	mcr	p15, 0, r0, c7, c5, 6		@ BPIALL
	dsb
	isb
	pop	{r1-r12, lr}
1:	b	save_boot_params_ret
ENDPROC(save_boot_params)
