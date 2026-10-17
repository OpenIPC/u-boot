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
