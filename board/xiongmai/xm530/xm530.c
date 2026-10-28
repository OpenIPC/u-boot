// SPDX-License-Identifier: GPL-2.0+
/*
 * Xiongmai XM530 / XM550 camera board, from the vendor U-Boot 2014.04
 * (board/xm/xm530/xm530.c). PLL and DDR set-up live in arch/arm/mach-xm.
 */

#include <common.h>
#include <init.h>
#include <net.h>
#include <netdev.h>
#include <openipc.h>
#include <phy.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/xm.h>
#include <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	gd->bd->bi_boot_params = XM_DDR_BASE + 0x100;
	return 0;
}

int misc_init_r(void)
{
	return openipc_helper();
}

#ifdef CONFIG_ETH_DESIGNWARE
#ifdef CONFIG_XM_V200
/*
 * The V200 series has an external RMII PHY: its pads, then its reset
 * on pad 0x224 (output low, then high), as the vendor U-Boots set them.
 */
int board_eth_init(struct bd_info *bis)
{
#ifdef CONFIG_TARGET_XM535
	static const u32 pads[] = { 0x82, 0x32, 0x32, 0x72, 0x42, 0x42, 0x42, 0x42, 0x32 };
	const u32 rmii = 0x18;
#else
	static const u32 pads[] = { 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82 };
	const u32 rmii = 0x20;
#endif
	int i;

	for (i = 0; i < ARRAY_SIZE(pads); i++)
		writel(pads[i], XM_GPIO_BASE + 0x200 + i * 4);
	writel(rmii, XM_GPIO_BASE + 0xC48);
	writel(0x400, XM_GPIO_BASE + 0x224);
	mdelay(100);
	writel(0xC00, XM_GPIO_BASE + 0x224);
	mdelay(100);

	return designware_initialize(XM_GMAC_BASE, PHY_INTERFACE_MODE_RMII);
}
#else
int board_eth_init(struct bd_info *bis)
{
	/* MAC glue: RMII, clocks, then the internal PHY out of reset */
	writel(0, XM_GMAC_GLUE + 0x00);
	writel(1, XM_GMAC_GLUE + 0x04);
	writel(1, XM_GMAC_GLUE + 0x08);
	writel(0, XM_GMAC_GLUE + 0x10);
	writel(0, XM_GMAC_GLUE + 0x14);
	writel(1, XM_GMAC_GLUE + 0x18);
	writel(7, XM_GMAC_GLUE + 0x1C);
	writel(1, XM_GMAC_GLUE + 0x20);
	writel(1, XM_GMAC_GLUE + 0x38);
	writel(1, XM_GMAC_GLUE + 0x3C);
	writel(1, XM_GMAC_GLUE + 0x40);
	writel(1, XM_GMAC_GLUE + 0x50);
	writel(0, XM_GMAC_GLUE + 0x0C);
	udelay(100);
	writel(1, XM_GMAC_GLUE + 0x0C);
	udelay(100);
	writel(0x80, XM_GPIO_BASE + 0x404);

	return designware_initialize(XM_GMAC_BASE, PHY_INTERFACE_MODE_RMII);
}
#endif
#endif
