/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * nsboot has set up DDR and the clocks and loaded U-Boot into DDR:
 * nothing is left to do here.
 */

#include <linux/linkage.h>

ENTRY(lowlevel_init)
	mov	pc, lr
ENDPROC(lowlevel_init)
