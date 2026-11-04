/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Novatek NA51068 (NT98321, NT98323): memory map, after the NT9832x SDK's
 * U-Boot 2016.07 (arch-nvt-na51068/IOAddress.h), kernel 4.9.118
 * (plat-na51068) and board device trees.
 */

#ifndef __NA51068_H
#define __NA51068_H

#include <linux/bitops.h>

#define NVT_DDR_BASE		0x00000000

#define NVT_SCU_BASE		0xF9C00000	/* A9 MPCore: SCU, GIC, timers */
#define NVT_GTIMER_BASE		(NVT_SCU_BASE + 0x200)
#define NVT_PL310_BASE		0xF9C20000
#define NVT_SDHCI_BASE		0xFA600000
#define NVT_FLASH_BASE		0xFA900000	/* Faraday FTSPI020 */
#define NVT_ETH0_BASE		0xFCC00000
#define NVT_ETH1_BASE		0xFCD00000
#define NVT_CG_BASE		0xFE000000	/* clocks, PLLs, resets */
#define NVT_TOP_BASE		0xFE030000	/* pinmux, chip ID */
#define NVT_PAD_BASE		0xFE040000
#define NVT_UART0_BASE		0xFE200000
#define NVT_GPIO1_BASE		0xFE420000
#define NVT_GPIO2_BASE		0xFE440000
#define NVT_WDT_BASE		0xFE8A0000

/* TOP */
#define NVT_TOP_CHIP_ID		(NVT_TOP_BASE + 0xF0)	/* [31:16] */

/* CG: the CPU's clock, from PLL16, eight times its rate */
#define NVT_CG_CPU_CKCTRL	(NVT_CG_BASE + 0x100)
#define NVT_CPU_CK_APLL		BIT(31)		/* else 80 MHz */
#define NVT_CG_PLL16		(NVT_CG_BASE + 0x107C0)	/* ratio at +0x20/24/28 */

/*
 * The A9 global timer counts at PLL16's rate, an eighth of the CPU's:
 * the loader leaves it as it is, the vendor U-Boot enables it.
 */
#define NVT_GTIMER_LO		(NVT_GTIMER_BASE + 0x00)
#define NVT_GTIMER_HI		(NVT_GTIMER_BASE + 0x04)
#define NVT_GTIMER_CTRL		(NVT_GTIMER_BASE + 0x08)

/* Watchdog: every CTRL write carries the key in [31:16] */
#define NVT_CG_APB_RSTN		(NVT_CG_BASE + 0x5C)
#define NVT_CG_WDT_RSTN		BIT(28)
#define NVT_CG_APB_GATE		(NVT_CG_BASE + 0x78)
#define NVT_CG_WDT_GATE		BIT(15)
#define NVT_WDT_CTRL		(NVT_WDT_BASE + 0x00)
#define NVT_WDT_MANUAL		(NVT_WDT_BASE + 0x0C)
#define NVT_WDT_KEY		0x5A960000
#define NVT_WDT_EN		BIT(0)

/*
 * What the loader leaves in DDR (the /nvt_memory_cfg node of its device
 * tree, openipc/na51068.dtsi): the device tree at 0x80000, SHMINFO at
 * 0xA00000, its own code from 0x100000. U-Boot runs where
 * /nvt_memory_cfg/uboot says: at 0x1E000000, as on every board of the
 * SDK (all have 512 MiB or more), above Linux's memory and its CMA pools
 * (0x02000000-0x04800000), below the second core's pen at 0x1FF80000.
 */
#define NVT_SHMINFO		(NVT_DDR_BASE + 0x00A00000)
#define NVT_SHM_LD_NVT		0x00		/* "LD_NVT" */
#define NVT_SHM_FDT_ADDR	0x2C
#define NVT_UBOOT_BASE		(NVT_DDR_BASE + 0x1E000000)
#define NVT_UBOOT_SIZE		0x01F80000

#ifndef __ASSEMBLY__
/* The loader's device tree, from SHMINFO, or NULL (mach-novatek/na51068.c) */
const void *nvt_loader_fdt(void);
#endif

#endif
