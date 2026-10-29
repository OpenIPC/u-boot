/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Ingenic T-series ISVP cameras: memory map. The hardware set-up is in
 * mach/ingenic.h, the environment and the rest of the OpenIPC
 * configuration in openipc-common.h.
 */

#ifndef __INGENIC_ISVP_H
#define __INGENIC_ISVP_H

/* cached (KSEG0) address; the board_init_f() stack is 4 MiB above it */
#define CONFIG_SYS_SDRAM_BASE		0x80000000
#define CONFIG_SYS_INIT_SP_OFFSET	0x400000

/* The DDR in the package */
#if defined(CONFIG_T40A)
#define PHYS_SDRAM_1_SIZE		0x40000000
#elif defined(CONFIG_T40XP)
#define PHYS_SDRAM_1_SIZE		0x10000000
#elif defined(CONFIG_DDR2_128M) || defined(CONFIG_DDR3_128M) || \
    defined(CONFIG_T23X) || defined(CONFIG_T40N)
#define PHYS_SDRAM_1_SIZE		0x08000000
#else
#define PHYS_SDRAM_1_SIZE		0x04000000
#endif

#define CONFIG_SYS_MONITOR_LEN		(512 * 1024)

#include "openipc-common.h"

#endif
