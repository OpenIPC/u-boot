/*
 *  armboot - Startup Code for ARM Cortex CPU-core
 */

#include <platform.h>

.globl _start
.globl	sm_vect_table
.globl	reset_vect_table
_start:
	b       reset

/*
 *************************************************************************
 *
 * Interrupt handling
 *
 *************************************************************************
 */
soft_reset:
	/* reset */
	b       .

.balignl 32, 0xdeadbeef
reset_vect_table:
b	.		/* Reset			*/
b	.		/* Undefined instruction	*/
b	boot		/* Secure monitor call	TODO	*/
b	.		/* Prefetch abort		*/
b	.		/* Data abort			*/
b	.		/* Reserved			*/
b	.		/* IRQ				*/
b	.		/* FIQ				*/


.balignl 32, 0xdeadbeef
sm_vect_table:
b	.		/* Reset			*/
b	.		/* Undefined instruction	*/
b	handle_smc	/* Secure monitor call	TODO	*/
b	.		/* Prefetch abort		*/
b	.		/* Data abort			*/
b	.		/* Reserved			*/
b	.		/* IRQ				*/
b	.		/* FIQ				*/

.balignl 16, 0xdeadbeef

.macro read_mvbar reg
mrc p15, 0, \reg, c12, c0, 1
.endm

.macro write_mvbar reg
mcr p15, 0, \reg, c12, c0, 1
.endm


reset:
	ldr	r0, =reset_vect_table
	write_mvbar r0
	smc #0
boot:
dbg_ctl:
boot_core_check:
	/* set stack to sram */
	ldr     r0, =STACK_ADDR
	mov     sp, r0
	
	/* clear_bss */
	ldr r0, =__bss_start
	mov r1, #0
	ldr r2, =__bss_end
	subs r2, r2, r0
	bl clear_data

	ldr	r0, =sm_vect_table
	write_mvbar r0

	/********add here******/
	bl main_entry

clear_data:
	add r2, r0, r2
	mov r3, #0x4

clear_data_loop:
	str r1, [r0]
	adds r0, r0, r3
	cmp r0, r2
	blt clear_data_loop
	bx lr
