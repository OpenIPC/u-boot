/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Novatek NA51089, NA51055 / NA51084, NS02501, NA51068, NA51103, NA51090: memory
 * map, console, timer, and the SPI NAND layout. The environment and the rest of the OpenIPC
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

#if defined(CONFIG_ENV_IS_IN_NAND) && !defined(CONFIG_NVT_NA511XX)
/*
 * The NA51089 / NT9852x cameras on SPI NAND (openipc/nvt-nand.dtsi): the
 * loader 0, the tree 0x40000 and its copy 0x80000, U-Boot 0xC0000; the
 * environment at 0x2C0000, UBI from 0x300000. ${ubfile} is the boot
 * image from the loader on, as on NOR.
 */
#define OPENIPC_UBWRITE			"nand erase 0x0 0x2c0000; nand write ${baseaddr} 0x0 ${filesize}"
#define OPENIPC_NAND_MTDPARTS		"2816k(boot),256k(env),-(ubi)"
#define CONFIG_SYS_MAX_NAND_DEVICE	1
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

/* Console: UART0, a 16550 with 32-bit registers */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_MEM32
#ifdef CONFIG_NVT_NA511XX
#define CONFIG_SYS_NS16550_CLK		NVT_UART_HZ
#elif defined(CONFIG_NVT_NA51068) || defined(CONFIG_NVT_NS02501)
/* NA51068, and NS02501 (the SDK's tree and its U-Boot's soc.c): 48 MHz */
#define CONFIG_SYS_NS16550_CLK		48000000
#else
#define CONFIG_SYS_NS16550_CLK		24000000
#endif
#define CONFIG_SYS_NS16550_COM1		NVT_UART0_BASE

#if !defined(CONFIG_ARM64) && !defined(CONFIG_NVT_NA51068)
/* Timer 0 of the timer block, free running at 1 MHz */
#define CONFIG_SYS_TIMER_RATE		NVT_TIMER_HZ
#define CONFIG_SYS_TIMER_COUNTER	NVT_TIMER0_COUNT
#endif
/* NA51068: the A9 global timer (arch/arm/mach-novatek/na51068.c) */

#include "openipc-common.h"

#endif
