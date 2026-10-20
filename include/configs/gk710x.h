/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Goke GK7101 / GK7102: memory map, console and timer. The environment and
 * the rest of the OpenIPC configuration come from openipc-common.h.
 */

#ifndef __GK710X_H
#define __GK710X_H

#include <asm/arch/gk710x.h>

#define CONFIG_SYS_SDRAM_BASE		GK_DDR_BASE
#define PHYS_SDRAM_1_SIZE		GK_DDR_MAX

/* The boot ROM copies U-Boot to its link address and starts it there */
#define CONFIG_SYS_UBOOT_BASE		CONFIG_SYS_TEXT_BASE

/* board_init_f() stack and global data, below U-Boot as the vendor's */
#define CONFIG_SYS_INIT_RAM_ADDR	(GK_DDR_BASE + 0x700000)
#define CONFIG_SYS_INIT_RAM_SIZE	0x100000

/* 16550 with 32-bit registers */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		GK_UART_CLK
#define CONFIG_SYS_NS16550_COM1		GK_UART0_BASE

#define CONFIG_SYS_TIMER_RATE		GK_APB_CLK
#define CONFIG_SYS_TIMER_COUNTER	GK_TIMER1_STATUS
#define CONFIG_SYS_TIMER_COUNTS_DOWN

/* Bitstream buffer above the kernel's mem=, as the vendor U-Boot sizes it */
#if defined(CONFIG_TARGET_GK7101) || defined(CONFIG_TARGET_GK7101S)
#define GK_BSB_MIB			4
#else
#define GK_BSB_MIB			2
#endif

#include "openipc-common.h"

#endif
