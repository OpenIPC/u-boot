/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Novatek NA51089 (NT98562, NT98566): memory map, after the vendor
 * U-Boot 2019.04 (arch-nvt-na51089_a32/IOAddress.h) and the SDK's
 * device trees.
 */

#ifndef __NA51089_H
#define __NA51089_H

#include <linux/bitops.h>

#define NVT_DDR_BASE		0x00000000

#define NVT_TOP_BASE		0xF0010000	/* pinmux, chip ID */
#define NVT_CG_BASE		0xF0020000	/* clocks, PLLs, resets */
#define NVT_PAD_BASE		0xF0030000
#define NVT_TIMER_BASE		0xF0040000
#define NVT_WDT_BASE		0xF0050000
#define NVT_GPIO_BASE		0xF0070000
#define NVT_UART0_BASE		0xF0290000
#define NVT_ETH_BASE		0xF02B0000
#define NVT_ETH_PHY_BASE	0xF02B3800
#define NVT_FLASH_BASE		0xF0400000
#define NVT_SDIO1_BASE		0xF0420000
#define NVT_PL310_BASE		0xFFE00000

/* TOP */
#define NVT_TOP_CHIP_ID		(NVT_TOP_BASE + 0xF0)	/* [31:16] */
#define NVT_CHIP_NA51089	0x7021

/* CG: per-module clock enables and resets (active low) */
#define NVT_CG_CLKEN0		(NVT_CG_BASE + 0x74)
#define NVT_CG_RSTN0		(NVT_CG_BASE + 0x84)
#define NVT_CG_FLASH		BIT(0)
#define NVT_CG_WDT		BIT(17)

/*
 * Timer 0 of the twenty in the timer block: the loader leaves it running
 * free at 1 MHz over the full 32 bits. The vendor U-Boot reads it as the
 * microseconds since power-on.
 */
#define NVT_TIMER0_COUNT	(NVT_TIMER_BASE + 0x108)
#define NVT_TIMER_HZ		1000000

/* Watchdog: every CTRL write carries the key in [31:16] */
#define NVT_WDT_CTRL		(NVT_WDT_BASE + 0x00)
#define NVT_WDT_MANUAL		(NVT_WDT_BASE + 0x0C)
#define NVT_WDT_KEY		0x5A960000
#define NVT_WDT_EN		BIT(0)

/*
 * What the loader leaves in DDR (the /nvt_memory_cfg node of its device
 * tree): the device tree at 0x100000 and the SHMINFO block at 0x200000.
 * U-Boot runs where /nvt_memory_cfg/uboot says.
 */
#define NVT_SHMINFO		(NVT_DDR_BASE + 0x00200000)
#define NVT_SHM_LD_NVT		0x00		/* "LD_NVT" */
#define NVT_SHM_FDT_ADDR	0x2C
/* /nvt_memory_cfg/uboot: where U-Boot is loaded, and where it stays */
#define NVT_UBOOT_BASE		(NVT_DDR_BASE + 0x03900000)
#define NVT_UBOOT_SIZE		0x006C0000

#endif
