/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Novatek NA51103 (NT98332) and NA51090 (NT98633), the NVR dies: memory
 * map, after the vendor U-Boot 2019.04 (arch-nvt-na51103_a64/IOAddress.h,
 * arch-nvt-na51090_a64/IOAddress.h) and the boards' device trees. The
 * blocks sit above 4 GiB, at 0x2_F000_0000; most keep the NA51089
 * register layout at new offsets.
 */

#ifndef __NA511XX_H
#define __NA511XX_H

#include <linux/bitops.h>

#define NVT_DDR_BASE		0x00000000
#define NVT_IO(off)		(0x2F0000000UL + (off))

#define NVT_TOP_BASE		NVT_IO(0x010000)	/* pinmux, chip ID */
#define NVT_CG_BASE		NVT_IO(0x020000)	/* clocks, resets */
#define NVT_PAD_BASE		NVT_IO(0x030000)
#define NVT_GPIO_BASE		NVT_IO(0x040000)
#define NVT_FLASH_BASE		NVT_IO(0x180000)
#define NVT_UART0_BASE		NVT_IO(0x280000)
#ifdef CONFIG_NVT_NA51090
#define NVT_WDT_BASE		NVT_IO(0x110000)
#define NVT_ETH_BASE		NVT_IO(0x200000)	/* the vendor's eth0 */
#else
#define NVT_WDT_BASE		NVT_IO(0x060000)
#define NVT_ETH_BASE		NVT_IO(0x440000)
#endif
#define NVT_IO_SIZE		0x10000000		/* to 0x2_FFFF_FFFF */

/* TOP */
#define NVT_TOP_CHIP_ID		(NVT_TOP_BASE + 0xF0)	/* [31:16] */
#define NVT_CHIP_NA51103	0x8B20
#define NVT_CHIP_NA51090	0xBC21

/* CG: per-module clock enables and resets (active low) */
#define NVT_CG_WDT_CLKEN	(NVT_CG_BASE + 0x7C)
#define NVT_CG_WDT_RSTN		(NVT_CG_BASE + 0x9C)
#define NVT_CG_WDT_EN		BIT(4)
#define NVT_CG_WDT_RST		BIT(12)

/* Watchdog: every CTRL write carries the key in [31:16] */
#define NVT_WDT_CTRL		(NVT_WDT_BASE + 0x00)
#define NVT_WDT_MANUAL		(NVT_WDT_BASE + 0x0C)
#define NVT_WDT_KEY		0x5A960000
#define NVT_WDT_EN		BIT(0)

/* The MAC's CSR clock is the 150 MHz APB: MDC divider CR 1 (/62) */
#define NVT_ETH_MDIO_CR		1

/* UART0, a 16550 with 32-bit registers */
#define NVT_UART_HZ		48000000

/*
 * What the loader leaves in DDR (/nvt_memory_cfg of its device tree):
 * the device tree at 0x100000, SHMINFO at 0xA00000, BL31 at 0x1F00000
 * (secure, kept out of reach: no-map in the kernel's tree too), and
 * U-Boot where /nvt_memory_cfg/uboot says, 32 MiB below the end of the
 * first 512 MiB / 1 GiB.
 */
#define NVT_SHMINFO		(NVT_DDR_BASE + 0x00A00000)
#define NVT_SHM_LD_NVT		0x00		/* "LD_NVT" */
#define NVT_SHM_FDT_ADDR	0x2C
#define NVT_BL31_BASE		(NVT_DDR_BASE + 0x01F00000)
#define NVT_BL31_SIZE		0x00100000
#ifdef CONFIG_NVT_NA51090
#define NVT_UBOOT_BASE		(NVT_DDR_BASE + 0x3E000000)
#else
#define NVT_UBOOT_BASE		(NVT_DDR_BASE + 0x1E000000)
#endif
#define NVT_UBOOT_SIZE		0x02000000

#endif
