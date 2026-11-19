/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * AX620U first stage entry. The boot ROM copies flash 0..(0x800 + size)
 * to the on-chip RAM at 0 (boot header at 0 and its copy at 0x400) and
 * starts every core at 0x800 in ARM state.
 *
 * Core 0 copies the vector table to 0, clears .bss, sets the stack and
 * calls main(). The other cores are released by main() writing 0x800 to
 * SMP_BOOT_ADDR: each marks its pen_release[] slot -1, waits until main()
 * clears SMP_BOOT_ADDR, then sleeps until the next stage writes an entry
 * there and jumps to it.
 */

#define SMP_BOOT_ADDR	0x02220300
#define STACK_TOP	0x00017ffc

	.syntax unified
	.arm
	.section .text.start, "ax"
	.globl	_start
_start:
	ldr	pc, _reset
	ldr	pc, _undef
	ldr	pc, _swi
	ldr	pc, _pabort
	ldr	pc, _dabort
	ldr	pc, _unused
	ldr	pc, _irq
	ldr	pc, _fiq

reset:
	ldr	r0, =_start
	mov	r1, #0
	ldmia	r0!, {r2-r9}
	stmia	r1!, {r2-r9}

	mrc	p15, 0, r0, c0, c0, 5	@ MPIDR
	and	r0, r0, #0xf
	cmp	r0, #0
	beq	primary

	mrc	p15, 0, r0, c0, c0, 5
	and	r0, r0, #0xf
	adr	r1, _pen_release
	ldr	r1, [r1]
	lsl	r0, r0, #2
	add	r1, r1, r0
	mvn	r2, #0
	str	r2, [r1]		@ pen_release[cpu] = -1
1:	ldr	r0, =SMP_BOOT_ADDR
	ldr	r0, [r0]
	cmp	r0, #0
	bne	1b			@ until main() clears it
2:	wfe
	ldr	r0, =SMP_BOOT_ADDR
	ldr	r0, [r0]
	cmp	r0, #0
	beq	2b
	bx	r0

primary:
	ldr	sp, =STACK_TOP
	ldr	r0, =__bss_start
	ldr	r1, =__bss_end
	mov	r2, #0
3:	str	r2, [r0], #4
	cmp	r0, r1
	blo	3b
	ldr	r0, =pen_release
	str	r2, [r0], #4
	str	r2, [r0], #4
	str	r2, [r0], #4
	str	r2, [r0], #4
	blx	main
undef:	b	undef
swi:	b	swi
pabort:	b	pabort
dabort:	b	dabort
unused:	b	unused
irq:	b	irq
fiq:	b	fiq

_pen_release:	.word	pen_release
_reset:		.word	reset
_undef:		.word	undef
_swi:		.word	swi
_pabort:	.word	pabort
_dabort:	.word	dabort
_unused:	.word	unused
_irq:		.word	irq
_fiq:		.word	fiq
	.ltorg
