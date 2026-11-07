/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * SigmaStar: memory map, console and timer. The environment and
 * the rest of the OpenIPC configuration come from openipc-common.h.
 */

#ifndef __SIGMASTAR_H_CONFIG
#define __SIGMASTAR_H_CONFIG

#include <asm/arch/sigmastar.h>

#define CONFIG_SYS_SDRAM_BASE		SS_DDR_BASE
/*
 * The most DDR the part carries, for get_ram_size() on the dies that probe;
 * the MSC316s and the SSD202D carry twice as much as their siblings. The
 * later dies read the DDR's size from the MIU.
 */
#if defined(CONFIG_TARGET_SSR621Q)
#define PHYS_SDRAM_1_SIZE		0x10000000
#elif defined(CONFIG_TARGET_MSC316DC) || defined(CONFIG_TARGET_MSC316DM) || \
    defined(CONFIG_TARGET_SSD202D) || defined(CONFIG_TARGET_SSC375) || \
    defined(CONFIG_TARGET_SSC034A)
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

#ifdef CONFIG_ENV_IS_IN_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#endif

/* PIU timer 0, read by timer_read_counter() */
#define CONFIG_SYS_TIMER_RATE		SS_XTAL_HZ

#include "openipc-common.h"

#endif
