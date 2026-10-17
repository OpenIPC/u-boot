/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Xiongmai XM5xx early init.
 *
 * U-Boot runs in place from SPI NOR, and DDR is not up yet: the only RAM is
 * the 4 KiB SRAM at 0x04000000. Bring up PLLs and DDR here, on an SRAM
 * stack and without global data, so that board_init_f() can then keep its
 * stack, global data and early malloc in DRAM (CONFIG_SYS_INIT_RAM_ADDR).
 */

#include <linux/linkage.h>
#include <asm/arch/xm.h>

ENTRY(lowlevel_init)
	ldr	sp, =XM_SRAM_TOP
	push	{ip, lr}
	bl	xm_early_init
	pop	{ip, pc}
ENDPROC(lowlevel_init)
