/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * The mask ROM has already run the PLL and DDR set-up from the flash header
 * and copied U-Boot into DDR. Ungate every clock and release every soft
 * reset, as the vendor U-Boot does.
 */

#include <linux/linkage.h>

ENTRY(lowlevel_init)
	ldr	r0, =0x0800001c
	mov	r1, #0
	str	r1, [r0]
	ldr	r0, =0x08000020
	str	r1, [r0]
	mov	pc, lr
ENDPROC(lowlevel_init)
