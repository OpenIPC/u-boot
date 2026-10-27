/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * SigmaStar infinity3: memory map, console and timer. The environment and
 * the rest of the OpenIPC configuration come from openipc-common.h.
 */

#ifndef __SIGMASTAR_H_CONFIG
#define __SIGMASTAR_H_CONFIG

#include <asm/arch/sigmastar.h>

#define CONFIG_SYS_SDRAM_BASE		SS_DDR_BASE
/* The DDR in the package: the MSC316s carry twice as much */
#if defined(CONFIG_TARGET_MSC316DC) || defined(CONFIG_TARGET_MSC316DM)
#define PHYS_SDRAM_1_SIZE		0x08000000
#else
#define PHYS_SDRAM_1_SIZE		0x04000000
#endif

/* board_init_f() stack and global data, below U-Boot */
#define CONFIG_SYS_INIT_RAM_ADDR	(CONFIG_SYS_TEXT_BASE - 0x200000)
#define CONFIG_SYS_INIT_RAM_SIZE	0x100000

/* Console: UART0, an 8250 with a register every 8 bytes */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-8)
#define CONFIG_SYS_NS16550_CLK		SS_UART_HZ
#define CONFIG_SYS_NS16550_COM1		SS_UART0_BASE

/* PIU timer 0, read by timer_read_counter() */
#define CONFIG_SYS_TIMER_RATE		SS_XTAL_HZ

#include "openipc-common.h"

#endif
