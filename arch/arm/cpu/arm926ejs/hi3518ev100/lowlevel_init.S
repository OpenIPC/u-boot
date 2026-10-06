/*
 * Early init for Hi3518EV100: clear the BootROM flash remap, set up TCM and
 * I-cache, then apply the register table (reg_info) spliced into the blank
 * zone at image offset 64 by the mkboot step.
 */

#include <config.h>
#include <asm/arch/platform.h>

	.text
	.align	2
	.global	lowlevel_init
	.type	lowlevel_init, %function
lowlevel_init:
	/* clear remap: SCTL SC_CTRL bit8 */
	ldr	r4, =REG_BASE_SCTL
	ldr	r0, [r4, #REG_SC_CTRL]
	orr	r0, r0, #(1 << 8)
	str	r0, [r4, #REG_SC_CTRL]

	/* Setup TCM (enabled, 2KB) */
	ldr	r0, =(1 | (MEM_CONF_ITCM_SIZE << 2) | MEM_BASE_ITCM)
	mcr	p15, 0, r0, c9, c1, 1

	/* enable I-cache now */
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, #0x00001000	/* set bit 12 (I) I-Cache */
	mcr	p15, 0, r0, c1, c0, 0

	/* apply the reg_info table */
	ldr	r0, =(_start + 0x40)
	bl	init_registers

	mov	pc, lr

	.text
	.align	2
	.global	init_registers
	.type	init_registers, %function
init_registers:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	ldr	r2, [r0, #0]
	cmp	r2, #0
	mov	r6, #1
	bne	.L43
.L2:
	ldr	r3, [r0, #4]
	cmp	r3, #0
	bne	.L43
	ldr	r3, [r0, #8]
	cmp	r3, #0
	ldrne	ip, [r0, #12]
	bne	.L3
	ldr	ip, [r0, #12]
	cmp	ip, #0
	beq	.L6
.L3:
	cmp	r1, #0
	beq	.L7
.L45:
	tst	ip, #2
	beq	.L8
	and	r4, ip, #248
	mov	r4, r4, lsr #3
	add	r4, r4, #1
	cmp	r4, #32
	movne	r4, r6, asl r4
	andne	ip, ip, #63488
	ldr	r5, [r2, #0]
	movne	ip, ip, lsr #11
	subne	r4, r4, #1
	bicne	r5, r5, r4, asl ip
	ldrne	r4, [r0, #4]
	ldreq	ip, [r0, #4]
	orrne	ip, r5, r4, asl ip
	cmp	r3, #0
	str	ip, [r2, #0]
	beq	.L11
.L29:
@ 38 "lowlevel_init.c" 1
	nop
@ 0 "" 2
	subs	r3, r3, #1
	bne	.L29
.L11:
	add	r0, r0, #16
.L46:
	ldr	r2, [r0, #0]
	cmp	r2, #0
	beq	.L2
.L43:
	cmp	r1, #0
	ldr	r3, [r0, #8]
	ldr	ip, [r0, #12]
	bne	.L45
.L7:
	tst	ip, #4
	beq	.L18
	and	r4, ip, #248
	mov	r4, r4, lsr #3
	add	r4, r4, #1
	cmp	r4, #32
	movne	r4, r6, asl r4
	andne	ip, ip, #63488
	ldr	r5, [r2, #0]
	movne	ip, ip, lsr #11
	subne	r4, r4, #1
	bicne	r5, r5, r4, asl ip
	ldrne	r4, [r0, #4]
	ldreq	ip, [r0, #4]
	orrne	ip, r5, r4, asl ip
	cmp	r3, #0
	str	ip, [r2, #0]
	beq	.L11
.L32:
@ 38 "lowlevel_init.c" 1
	nop
@ 0 "" 2
	subs	r3, r3, #1
	bne	.L32
	add	r0, r0, #16
	b	.L46
.L8:
	tst	ip, #131072
	bne	.L13
	cmp	r3, #0
	beq	.L11
.L31:
@ 38 "lowlevel_init.c" 1
	nop
@ 0 "" 2
	subs	r3, r3, #1
	bne	.L31
	add	r0, r0, #16
	b	.L46
.L18:
	tst	ip, #262144
	bne	.L22
	cmp	r3, #0
	beq	.L11
.L34:
@ 38 "lowlevel_init.c" 1
	nop
@ 0 "" 2
	subs	r3, r3, #1
	bne	.L34
	add	r0, r0, #16
	b	.L46
.L13:
	and	r4, ip, #16252928
	mov	r4, r4, lsr #19
	add	r4, r4, #1
	mov	r7, r6, asl r4
	ldr	r5, [r0, #4]
	mov	r8, ip, lsr #27
	sub	r7, r7, #1
.L16:
	ldr	ip, [r2, #0]
	cmp	r4, #32
	andne	ip, r7, ip, lsr r8
@ 38 "lowlevel_init.c" 1
	nop
@ 0 "" 2
	cmp	r5, ip
	bne	.L16
	cmp	r3, #0
	beq	.L11
.L30:
@ 38 "lowlevel_init.c" 1
	nop
@ 0 "" 2
	subs	r3, r3, #1
	bne	.L30
	add	r0, r0, #16
	b	.L46
.L22:
	and	r4, ip, #16252928
	mov	r4, r4, lsr #19
	add	r4, r4, #1
	mov	r8, r6, asl r4
	ldr	r5, [r0, #4]
	sub	r8, r8, #1
	mov	r7, ip, lsr #27
.L25:
	ldr	ip, [r2, #0]
	cmp	r4, #32
	andne	ip, r8, ip, lsr r7
@ 38 "lowlevel_init.c" 1
	nop
@ 0 "" 2
	cmp	r5, ip
	bne	.L25
	cmp	r3, #0
	beq	.L11
.L33:
@ 38 "lowlevel_init.c" 1
	nop
@ 0 "" 2
	subs	r3, r3, #1
	bne	.L33
	add	r0, r0, #16
	b	.L46
.L6:
@ 38 "lowlevel_init.c" 1
	nop
@ 0 "" 2
	bx	lr
