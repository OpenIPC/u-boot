/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Fullhan camera SoCs: memory map, console and timer. The environment and
 * the rest of the OpenIPC configuration come from openipc-common.h.
 */

#ifndef __FULLHAN_H
#define __FULLHAN_H

#include <asm/arch/fullhan.h>

#define CONFIG_SYS_SDRAM_BASE		FH_DDR_BASE

/* The boot ROM (or its first stage) loads U-Boot at its link address */
#define CONFIG_SYS_UBOOT_BASE		CONFIG_SYS_TEXT_BASE

/* board_init_f() stack and global data, below U-Boot */
#define CONFIG_SYS_INIT_RAM_ADDR	(FH_DDR_BASE + 0x700000)
#define CONFIG_SYS_INIT_RAM_SIZE	0x100000

/* DesignWare 16550 with 32-bit registers */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		FH_UART_CLK
#define CONFIG_SYS_NS16550_COM1		FH_UART0_BASE

#define CONFIG_SYS_TIMER_RATE		FH_TIMER_RATE
#define CONFIG_SYS_TIMER_COUNTER	FH_TIMER0_VALUE
#define CONFIG_SYS_TIMER_COUNTS_DOWN

#include "openipc-common.h"

#endif
