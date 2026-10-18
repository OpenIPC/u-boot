/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * The boot ROM has already run the PLL and DDR set-up from the flash
 * header and copied U-Boot into DDR: nothing is left to do here.
 */

#include <linux/linkage.h>

ENTRY(lowlevel_init)
	mov	pc, lr
ENDPROC(lowlevel_init)
