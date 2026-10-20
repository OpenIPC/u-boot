/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Xiongmai XM530 / XM550: memory map, console and timer. The environment and the
 * rest of the OpenIPC configuration come from openipc-common.h.
 */

#ifndef __XM530_H
#define __XM530_H

#include <asm/arch/xm.h>

#define CONFIG_SYS_SDRAM_BASE		XM_DDR_BASE
#define PHYS_SDRAM_1_SIZE		XM_DDR_MAX

/*
 * board_init_f() stack, global data and early malloc: in DDR, which
 * lowlevel_init has brought up
 */
#define CONFIG_SYS_INIT_RAM_ADDR	(XM_DDR_BASE + 0x100000)
#define CONFIG_SYS_INIT_RAM_SIZE	0x10000

#define CONFIG_PL011_CLOCK		XM_XCLK
#define CONFIG_PL01x_PORTS		{ (void *)XM_UART0_BASE }

#define CONFIG_SYS_TIMER_RATE		(XM_XCLK / XM_TIMER_PRESCALE)
#define CONFIG_SYS_TIMER_COUNTER	XM_TIMER_VALUE
#define CONFIG_SYS_TIMER_COUNTS_DOWN

#include "openipc-common.h"

#endif
