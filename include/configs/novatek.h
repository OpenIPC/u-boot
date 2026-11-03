/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Novatek NA51089, NA51055 / NA51084, NA51103, NA51090: memory map,
 * console and timer. The environment and the rest of the OpenIPC
 * configuration come from openipc-common.h.
 */

#ifndef __NOVATEK_H
#define __NOVATEK_H

#include <asm/arch/hardware.h>

#define CONFIG_SYS_SDRAM_BASE		NVT_DDR_BASE
/* The DDR in the package, or on the board */
#define PHYS_SDRAM_1_SIZE		CONFIG_NVT_DDR_SIZE

/* Linux's share of the DDR by default (mem=) */
#define OPENIPC_OSMEM			CONFIG_NVT_OSMEM

#ifdef CONFIG_NVT_NA511XX
/*
 * Staging buffer for kernels and images: past the loader's areas and
 * BL31 (0x1F00000, secure), where the vendor U-Boot stages them
 */
#define OPENIPC_BASEADDR		0x02000000
#ifdef CONFIG_NVT_NA51090
/*
 * The board's loader, tree and BL31 stay in the first 768 KiB of the NAND
 * (loader 0, tree 0x40000, BL31 0x80000); ${ubfile} is U-Boot's slot
 * alone, 0xC0000..0x1A0000. The environment is the board's uenv
 * partition (0x1A0000), UBI follows (0x1E0000).
 */
#define OPENIPC_UBWRITE			"nand erase 0xc0000 0xe0000; nand write ${baseaddr} 0xc0000 ${filesize}"
#define OPENIPC_NAND_MTDPARTS		"1664k(boot),256k(env),-(ubi)"
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#endif
#ifdef CONFIG_NVT_NA51103
/*
 * The board's loader and device tree stay in the first 128 KiB of the
 * NOR: ${ubfile} is U-Boot's slot alone, 0x20000..0x40000
 * (openipc/nvt-nvr-image.py)
 */
#define OPENIPC_UBWRITE			"sf probe 0; sf erase 0x20000 0x20000; sf write ${baseaddr} 0x20000 ${filesize}"
#endif
#endif

/* The loader starts U-Boot at its link address */
#define CONFIG_SYS_UBOOT_BASE		CONFIG_SYS_TEXT_BASE

/* board_init_f() stack and global data, below U-Boot */
#define CONFIG_SYS_INIT_RAM_ADDR	(CONFIG_SYS_TEXT_BASE - 0x100000)
#define CONFIG_SYS_INIT_RAM_SIZE	0x100000

/* Console: UART0, a 16550 with 32-bit registers */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_MEM32
#ifdef CONFIG_NVT_NA511XX
#define CONFIG_SYS_NS16550_CLK		NVT_UART_HZ
#else
#define CONFIG_SYS_NS16550_CLK		24000000
#endif
#define CONFIG_SYS_NS16550_COM1		NVT_UART0_BASE

#ifndef CONFIG_ARM64
/* Timer 0 of the timer block, free running at 1 MHz */
#define CONFIG_SYS_TIMER_RATE		NVT_TIMER_HZ
#define CONFIG_SYS_TIMER_COUNTER	NVT_TIMER0_COUNT
#endif

#include "openipc-common.h"

#endif
