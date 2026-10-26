/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Grain Media GM8135S / GM8136 / GM8136S: memory map, console and timer.
 * The environment and the rest of the OpenIPC configuration come from
 * openipc-common.h.
 */

#ifndef __GRAINMEDIA_H
#define __GRAINMEDIA_H

#include <asm/arch/grainmedia.h>

#define CONFIG_SYS_SDRAM_BASE		GM_DDR_BASE

/* nsboot loads U-Boot to DDR 0, its link address */
#define CONFIG_SYS_UBOOT_BASE		CONFIG_SYS_TEXT_BASE

/* board_init_f() stack and global data, above U-Boot */
#define CONFIG_SYS_INIT_RAM_ADDR	(GM_DDR_BASE + 0x700000)
#define CONFIG_SYS_INIT_RAM_SIZE	0x100000

/* FTUART010: a 16550 with 32-bit registers, at 25 MHz from nsboot */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		GM_UART_CLK
#define CONFIG_SYS_NS16550_COM1		GM_UART0_BASE

/* Timer 0 counting APB clocks down */
#define CONFIG_SYS_TIMER_RATE		gm_apb_hz()
#define CONFIG_SYS_TIMER_COUNTER	GM_TIMER0_COUNTER
#define CONFIG_SYS_TIMER_COUNTS_DOWN

#include "openipc-common.h"

#endif
