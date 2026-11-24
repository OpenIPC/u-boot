/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * GM8126 first stage: entry. The mask ROM copies this from the flash to
 * the MCP100's SRAM and jumps to its start; DDR is still off and at
 * 0x10000000.
 */

	.section .text.start, "ax"
	.globl	_start
_start:
	mrs	r0, cpsr
	bic	r0, r0, #0x1f
	orr	r0, r0, #0xd3			/* SVC, IRQ and FIQ off */
	msr	cpsr_cxsf, r0

	/* instruction cache on, as the vendor's */
	mov	r0, #0
	mcr	p15, 0, r0, c7, c5, 0
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, #0x1000
	mcr	p15, 0, r0, c1, c0, 0

	ldr	sp, =_stack_top
	ldr	r0, =__bss_start
	ldr	r1, =__bss_end
	mov	r2, #0
1:	cmp	r0, r1
	strlo	r2, [r0], #4
	blo	1b
	bl	main
2:	b	2b

/*
 * The frequency change's interrupt, entered through the vector main()
 * puts at 0x18: drop it at its source and mask it. The core was in WFI.
 */
	.globl	fcs_irq
fcs_irq:
	sub	lr, lr, #4
	stmfd	sp!, {r0-r3, r12, lr}
	bl	fcs_handler
	ldmfd	sp!, {r0-r3, r12, pc}^

	.globl	fcs_wait
fcs_wait:
	mrs	r2, cpsr
	bic	r1, r2, #0x1f
	orr	r1, r1, #0xd2			/* IRQ mode's stack */
	msr	cpsr_c, r1
	ldr	sp, =_irq_stack_top
	msr	cpsr_c, r2

	bic	r1, r2, #0x80			/* IRQs on */
	msr	cpsr_c, r1
	ldr	r0, =0x9900000c			/* PMODE: start the change */
	ldr	r1, [r0]
	orr	r1, r1, #4
	str	r1, [r0]
	mov	r0, #0
	mcr	p15, 0, r0, c7, c0, 4		/* WFI */
	nop
	nop
	msr	cpsr_c, r2
	mov	pc, lr
	.ltorg
