/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Novatek NA51089, NA51068: memory map, console and timer. The
 * environment and the rest of the OpenIPC configuration come from
 * openipc-common.h.
 */

#ifndef __NOVATEK_H
#define __NOVATEK_H

#ifdef CONFIG_NVT_NA51068
#include <asm/arch/na51068.h>
#else
#include <asm/arch/na51089.h>
#endif

#define CONFIG_SYS_SDRAM_BASE		NVT_DDR_BASE
/* The DDR in the package, or on the board */
#define PHYS_SDRAM_1_SIZE		CONFIG_NVT_DDR_SIZE

/* Linux's share of a 512 MiB part, as on the SDK's boards (mem=) */
#if defined(CONFIG_TARGET_NT98321)
#define OPENIPC_OSMEM			"160M"
#elif defined(CONFIG_TARGET_NT98323)
#define OPENIPC_OSMEM			"192M"
#elif CONFIG_NVT_DDR_SIZE >= 0x20000000
#define OPENIPC_OSMEM			"256M"
#endif

#ifdef CONFIG_NVT_NA51068
/*
 * The device tree and an initrd stay in the first 128 MiB: the SDK's
 * kernel 4.9 hangs before its first message with them just below
 * mem=192M, where bootm would put them
 */
#define CONFIG_SYS_BOOTMAPSZ		0x08000000
#endif

/* The loader starts U-Boot at its link address */
#define CONFIG_SYS_UBOOT_BASE		CONFIG_SYS_TEXT_BASE

/* board_init_f() stack and global data, below U-Boot */
#define CONFIG_SYS_INIT_RAM_ADDR	(CONFIG_SYS_TEXT_BASE - 0x100000)
#define CONFIG_SYS_INIT_RAM_SIZE	0x100000

/* Console: UART0, a 16550 with 32-bit registers, at 24 (48) MHz */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_MEM32
#ifdef CONFIG_NVT_NA51068
#define CONFIG_SYS_NS16550_CLK		48000000
#else
#define CONFIG_SYS_NS16550_CLK		24000000
#endif
#define CONFIG_SYS_NS16550_COM1		NVT_UART0_BASE

#ifndef CONFIG_NVT_NA51068
/* Timer 0 of the timer block, free running at 1 MHz */
#define CONFIG_SYS_TIMER_RATE		NVT_TIMER_HZ
#define CONFIG_SYS_TIMER_COUNTER	NVT_TIMER0_COUNT
#endif
/* NA51068: the A9 global timer (arch/arm/mach-novatek/na51068.c) */

#include "openipc-common.h"

#endif
