/*
 * (C) Copyright 2013
 * David Feng <fenghua@phytium.com.cn>
 *
 * SPDX-License-Identifier:    GPL-2.0+
 */

#include <asm-offsets.h>
#include <config.h>
#include <linux/linkage.h>
#include <asm/armv8/mmu.h>

/*************************************************************************
 *
 * Startup Code (reset vector)
 *
 *************************************************************************/
/*
 * Branch according to exception level
 */
.macro	switch_el, xreg, el3_label, el2_label, el1_label
	mrs	\xreg, CurrentEL
	cmp	\xreg, 0xc
	b.eq	\el3_label
	cmp	\xreg, 0x8
	b.eq	\el2_label
	cmp	\xreg, 0x4
	b.eq	\el1_label
.endm

/*
 * Enter Exception.
 * This will save the processor state that is ELR/X0~X30
 * to the stack frame.
 */
.macro	exception_entry
	stp	x29, x30, [sp, #-16]!
	stp	x27, x28, [sp, #-16]!
	stp	x25, x26, [sp, #-16]!
	stp	x23, x24, [sp, #-16]!
	stp	x21, x22, [sp, #-16]!
	stp	x19, x20, [sp, #-16]!
	stp	x17, x18, [sp, #-16]!
	stp	x15, x16, [sp, #-16]!
	stp	x13, x14, [sp, #-16]!
	stp	x11, x12, [sp, #-16]!
	stp	x9, x10, [sp, #-16]!
	stp	x7, x8, [sp, #-16]!
	stp	x5, x6, [sp, #-16]!
	stp	x3, x4, [sp, #-16]!
	stp	x1, x2, [sp, #-16]!

	/* Could be running at EL3/EL2/EL1 */
	switch_el x11, 3f, 2f, 1f
3:	mrs	x1, esr_el3
	mrs	x2, elr_el3
	b	0f
2:	mrs	x1, esr_el2
	mrs	x2, elr_el2
	b	0f
1:	mrs	x1, esr_el1
	mrs	x2, elr_el1
0:
	stp	x2, x0, [sp, #-16]!
	mov	x0, sp
.endm

.globl _start
_start:
    b reset

.balignl 64,0xdeadbeef
__blank_zone_start:
.fill 11008,1,0
__blank_zone_end:

.globl _blank_zone_start
_blank_zone_start:
.quad __blank_zone_start

.globl _blank_zone_end
_blank_zone_end:
.quad __blank_zone_end
.balignl 16,0xdeadbeef

.align 3

.globl _TEXT_BASE
_TEXT_BASE:
    .quad TEXT_BASE

/*
 * These are defined in the linker script.
 */
.globl _end_ofs
_end_ofs:
    .quad _end - _start

.globl _bss_start_ofs
_bss_start_ofs:
    .quad __bss_start - _start

.globl _bss_end_ofs
_bss_end_ofs:
    .quad __bss_end - _start

reset:
    /*
     * Could be EL3/EL2/EL1, Initial State:
     * Little Endian, MMU Disabled, i/dCache Disabled
     */
    adr    x0, vectors
    switch_el x1, 3f, 2f, 1f
3:  msr    vbar_el3, x0
    mrs    x0, scr_el3
    orr    x0, x0, #0xf             /* SCR_EL3.NS|IRQ|FIQ|EA */
    msr    scr_el3, x0
    msr    cptr_el3, xzr            /* Enable FP/SIMD */
#ifdef COUNTER_FREQUENCY
    ldr    x0, =COUNTER_FREQUENCY
    msr    cntfrq_el0, x0           /* Initialize CNTFRQ */
#endif
    b      0f
2:  msr    vbar_el2, x0
    mov    x0, #0x33ff
    msr    cptr_el2, x0             /* Enable FP/SIMD */
    b      0f
1:  msr    vbar_el1, x0
    mov    x0, #3 << 20
    msr    cpacr_el1, x0            /* Enable FP/SIMD */
0:

    /*
     * Cache/BPB/TLB Invalidate
     * i-cache is invalidated before enabled in icache_enable()
     * tlb is invalidated before mmu is enabled in dcache_enable()
     * d-cache is invalidated before enabled in dcache_enable()
     */

normal_start_flow:
    /* set stack for C code  */
    ldr    x0, =(CONFIG_SYS_INIT_SP_ADDR)
    bic    sp, x0, #0xf            /* 16-byte alignment for ABI compliance */

    ldr     x0, =REG_SELF_BOOT_FALG
    ldr     w1, [x0]
    ldr     w2, =SELF_BOOT_UART_NUDE
    cmp     w1, w2
    b.eq    print_startup
    ldr     w2, =SELF_BOOT_UART_NO_NUDE
    cmp     w1, w2
    b.eq    print_startup
    /*
     * During the uart download process, uart has to work in the
     * preset mode. In this case, uart_early_init should not be
     * called as it will reset the former uart configuration.
     */
    bl      uart_early_init

print_startup:
    adr     x0, Str_SystemSartup
    bl      uart_early_puts

jump_to_ddr:
    adr    x0, _start_armboot
    ldr    x30,[x0]
    ret
.align 3
_start_armboot: .quad start_armboot

/*-----------------------------------------------------------------------*/

/*
 * Exception vectors.
 */
	.align	3
	.globl	vectors
vectors:
	.align	3
	b	_do_bad_sync	/* Current EL Synchronous Thread */

	.align	3
	b	_do_bad_irq	/* Current EL IRQ Thread */

	.align	3
	b	_do_bad_fiq	/* Current EL FIQ Thread */

	.align	3
	b	_do_bad_error	/* Current EL Error Thread */

	.align	3
	b	_do_sync	/* Current EL Synchronous Handler */

	.align	3
	b	_do_irq		/* Current EL IRQ Handler */

	.align	3
	b	_do_fiq		/* Current EL FIQ Handler */

	.align	3
	b	_do_error	/* Current EL Error Handler */


_do_bad_sync:
	exception_entry
	bl	do_bad_sync
	b	exception_exit

_do_bad_irq:
	exception_entry
	bl	do_bad_irq
	b	exception_exit

_do_bad_fiq:
	exception_entry
	bl	do_bad_fiq
	b	exception_exit

_do_bad_error:
	exception_entry
	bl	do_bad_error
	b	exception_exit

_do_sync:
	exception_entry
	bl	do_sync
	b	exception_exit

_do_irq:
	exception_entry
	bl	do_irq
	b	exception_exit

_do_fiq:
	exception_entry
	bl	do_fiq
	b	exception_exit

_do_error:
	exception_entry
	bl	do_error
	b	exception_exit

exception_exit:
	ldp	x2, x0, [sp],#16
	switch_el x11, 3f, 2f, 1f
3:	msr	elr_el3, x2
	b	0f
2:	msr	elr_el2, x2
	b	0f
1:	msr	elr_el1, x2
0:
	ldp	x1, x2, [sp],#16
	ldp	x3, x4, [sp],#16
	ldp	x5, x6, [sp],#16
	ldp	x7, x8, [sp],#16
	ldp	x9, x10, [sp],#16
	ldp	x11, x12, [sp],#16
	ldp	x13, x14, [sp],#16
	ldp	x15, x16, [sp],#16
	ldp	x17, x18, [sp],#16
	ldp	x19, x20, [sp],#16
	ldp	x21, x22, [sp],#16
	ldp	x23, x24, [sp],#16
	ldp	x25, x26, [sp],#16
	ldp	x27, x28, [sp],#16
	ldp	x29, x30, [sp],#16
	eret

/*
 * void __asm_invalidate_icache_all(void)
 *
 * invalidate all tlb entries.
 */
ENTRY(__asm_invalidate_icache_all)
	ic	ialluis
	isb	sy
	ret
ENDPROC(__asm_invalidate_icache_all)

.align  3
Str_SystemSartup:
.ascii "\r\n\r\nSystem startup\r\n\0"
