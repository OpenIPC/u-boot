// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024 Rockchip Electronics Co., Ltd
 *
 * RV1103B / RV1106B: what the SPL sets up before U-Boot, from the vendor
 * U-Boot (rockchip-linux/u-boot next-dev, arch/arm/mach-rockchip/rv1103b/
 * rv1103b.c): the secure timer, the firewall and watchdog setup, and the
 * HPMCU the mask ROM leaves running. The thunder-boot, MCU-firmware and
 * USB-plug paths are left out.
 */
#include <common.h>
#include <dm.h>
#include <init.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/arch-rockchip/bootrom.h>
#include <asm/arch-rockchip/boot_mode.h>
#include <asm/arch-rockchip/hardware.h>
#include <linux/bitops.h>
#include <linux/delay.h>

/* The vendor kernel's reboot reasons (its boot_mode.h; not in mainline) */
#define BOOT_PANIC			(REBOOT_FLAG + 7)
#define BOOT_WATCHDOG			(REBOOT_FLAG + 8)

#define PERI_CRU_BASE			0x20000000
#define PERICRU_PERISOFTRST_CON10	0x0a28

#define TOP_CRU_BASE			0x20060000
#define TOPCRU_CRU_GLBRST_ST		0x0c04
#define TOPCRU_CRU_GLB_RST_CON		0x0c10
#define GLB_WDTn_RST_ST			GENMASK(9, 6)
#define WDT_SYS_RST_ST			GENMASK(15, 10)

#define SYS_GRF_BASE			0x20150000
#define GRF_SYS_PERI_CON2		0x0008
#define GRF_SYS_USBPHY_CON0		0x0050

#define SGRF_SYS_BASE			0x20250000
#define SGRF_SYS_SOC_CON2		0x0008
#define SGRF_SYS_OTP_CON		0x0018
#define FIREWALL_CON0			0x0020
#define FIREWALL_CON1			0x0024
#define FIREWALL_CON2			0x0028
#define FIREWALL_CON3			0x002c
#define FIREWALL_CON4			0x0030
#define FIREWALL_CON5			0x0034
#define FIREWALL_CON7			0x003c

#define USBPHY_APB_BASE			0x20e10000
#define USBPHY_FSLS_DIFF_RECEIVER	0x0100

#define STIMER_BASE			0x20500000
#define STIMER_CONTROL			0x04
#define STIMER_LOAD_COUNT0		0x14
#define STIMER_LOAD_COUNT1		0x18
#define STIMER_EN			BIT(0)

/* The flash and the SD card on the SFC and SD controller of the device tree */
const char * const boot_devices[BROM_LAST_BOOTSOURCE + 1] = {
	[BROM_BOOTSOURCE_SPINOR] = "/spi@20d40000/flash@0",
	[BROM_BOOTSOURCE_SPINAND] = "/spi@20d40000/flash@0",
};

void board_debug_uart_init(void)
{
	/* UART0 M0: the mask ROM and the DDR blob have set its pins */
}

#ifdef CONFIG_SPL_BUILD
/* The secure timer behind the arch timer, its control at +0x4 on this SoC */
void rockchip_stimer_init(void)
{
	if (readl(STIMER_BASE + STIMER_CONTROL) & STIMER_EN)
		return;
	writel(0x00010000, STIMER_BASE + STIMER_CONTROL);
	asm volatile("mcr p15, 0, %0, c14, c0, 0"
		     : : "r"(CONFIG_COUNTER_FREQUENCY));
	writel(0xffffffff, STIMER_BASE + STIMER_LOAD_COUNT0);
	writel(0xffffffff, STIMER_BASE + STIMER_LOAD_COUNT1);
	writel(0x00010001, STIMER_BASE + STIMER_CONTROL);
}

/* The mask ROM releases the HPMCU; U-Boot does not use it */
void spl_board_prepare_for_boot(void)
{
	writel(0x00f000f0, PERI_CRU_BASE + PERICRU_PERISOFTRST_CON10);
}

int arch_cpu_init(void)
{
	u32 glbrst_st = readl(TOP_CRU_BASE + TOPCRU_CRU_GLBRST_ST);

	/* A watchdog reset: say so in the boot mode, unless it was a panic */
	if (glbrst_st & GLB_WDTn_RST_ST) {
		if (readl(CONFIG_ROCKCHIP_BOOT_MODE_REG) != BOOT_PANIC)
			writel(BOOT_WATCHDOG, CONFIG_ROCKCHIP_BOOT_MODE_REG);
		writel(glbrst_st & ~WDT_SYS_RST_ST,
		       TOP_CRU_BASE + TOPCRU_CRU_GLBRST_ST);
	}

	/* Every device non-secure, and the OTP */
	writel(0xffff0000, SGRF_SYS_BASE + FIREWALL_CON0);
	writel(0xffff0000, SGRF_SYS_BASE + FIREWALL_CON1);
	writel(0xffff0000, SGRF_SYS_BASE + FIREWALL_CON2);
	writel(0xffff0000, SGRF_SYS_BASE + FIREWALL_CON3);
	writel(0xffff0000, SGRF_SYS_BASE + FIREWALL_CON4);
	writel(0xffff0000, SGRF_SYS_BASE + FIREWALL_CON5);
	writel(0x01f00000, SGRF_SYS_BASE + FIREWALL_CON7);
	writel(0x00020000, SGRF_SYS_BASE + SGRF_SYS_OTP_CON);

	/* Either watchdog's reset output resets the SoC */
	writel(0x00010001, SYS_GRF_BASE + GRF_SYS_PERI_CON2);
	writel(0x00010001, SGRF_SYS_BASE + SGRF_SYS_SOC_CON2);
	/*
	 * TSADC and watchdog trigger the global reset, the first reset, and
	 * the global reset resets the PMU
	 */
	writel(0x0000ffdf, TOP_CRU_BASE + TOPCRU_CRU_GLB_RST_CON);

	/* The USB2 PHY suspended, its FS/LS receiver off (power) */
	writel(0x01ff01d1, SYS_GRF_BASE + GRF_SYS_USBPHY_CON0);
	writel(0x00000000, USBPHY_APB_BASE + USBPHY_FSLS_DIFF_RECEIVER);

	return 0;
}
#endif
