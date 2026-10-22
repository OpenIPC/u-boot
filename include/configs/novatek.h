/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Novatek NA51089: memory map, console and timer. The environment and the
 * rest of the OpenIPC configuration come from openipc-common.h.
 */

#ifndef __NOVATEK_H
#define __NOVATEK_H

#include <asm/arch/na51089.h>

#define CONFIG_SYS_SDRAM_BASE		NVT_DDR_BASE
/* The DDR in the package */
#ifdef CONFIG_TARGET_NT98566
#define PHYS_SDRAM_1_SIZE		0x08000000
#else
#define PHYS_SDRAM_1_SIZE		0x04000000
#endif

/* The loader starts U-Boot at its link address */
#define CONFIG_SYS_UBOOT_BASE		CONFIG_SYS_TEXT_BASE

/* board_init_f() stack and global data, below U-Boot */
#define CONFIG_SYS_INIT_RAM_ADDR	(CONFIG_SYS_TEXT_BASE - 0x100000)
#define CONFIG_SYS_INIT_RAM_SIZE	0x100000

/* Console: UART0, a 16550 with 32-bit registers, at 24 MHz */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_MEM32
#define CONFIG_SYS_NS16550_CLK		24000000
#define CONFIG_SYS_NS16550_COM1		NVT_UART0_BASE

/* Timer 0 of the timer block, free running at 1 MHz */
#define CONFIG_SYS_TIMER_RATE		NVT_TIMER_HZ
#define CONFIG_SYS_TIMER_COUNTER	NVT_TIMER0_COUNT

#include "openipc-common.h"

#endif
