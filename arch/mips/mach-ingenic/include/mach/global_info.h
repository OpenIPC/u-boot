/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * The clock set-up the vendor SPL passes between pll_init() and the DDR
 * code (asm/global_info.h of OpenIPC/u-boot-ingenic, without the burner
 * fields).
 */

#ifndef __MACH_GLOBAL_INFO_H
#define __MACH_GLOBAL_INFO_H

#include <linux/types.h>

struct global_info {
	uint32_t extal;
	uint32_t cpufreq;
	uint32_t ddrfreq;
	uint32_t ddr_div;
	uint32_t uart_idx;
	uint32_t baud_rate;
};

#endif
