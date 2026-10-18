/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Anyka AK39: memory map, console and timer. The environment and the rest
 * of the OpenIPC configuration come from openipc-common.h.
 */

#ifndef __ANYKA_H
#define __ANYKA_H

#include <asm/arch/anyka.h>

#define CONFIG_SYS_SDRAM_BASE		AK_DDR_BASE
/* The DDR2 in the package: the AK3919s carry twice as much */
#if defined(CONFIG_TARGET_AK3919EV300) || defined(CONFIG_TARGET_AK3919EV330)
#define PHYS_SDRAM_1_SIZE		0x08000000
#else
#define PHYS_SDRAM_1_SIZE		0x04000000
#endif

/* The boot ROM (EV330: its first stage) copies U-Boot to its link address */
#define CONFIG_SYS_UBOOT_BASE		CONFIG_SYS_TEXT_BASE

/* board_init_f() stack and global data, below U-Boot */
#define CONFIG_SYS_INIT_RAM_ADDR	(AK_DDR_BASE + 0x700000)
#define CONFIG_SYS_INIT_RAM_SIZE	0x100000

/* Timer 1 at the crystal rate, read by timer_read_counter() */
#define CONFIG_SYS_TIMER_RATE		AK_XTAL_HZ

#include "openipc-common.h"

#endif
