// SPDX-License-Identifier: GPL-2.0+
/*
 * Xiongmai XM510 camera board, from the vendor U-Boot 2014.04
 * (board/xm/xm510/xm510.c). PLL and DDR set-up live in arch/arm/mach-xm.
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
	u32 v;

	/* GPIO8_1 is the PHY reset: pulse it low */
	writel(readl(XM_GPIO78_MULT_USE_EN) | 0x01, XM_GPIO78_MULT_USE_EN);
	writel(readl(XM_GPIO8_OUT_EN) | 0x02, XM_GPIO8_OUT_EN);
	v = readl(XM_GPIO8_OUT_REG);
	writel(v & ~0x02, XM_GPIO8_IO);
	mdelay(100);
	writel(v | 0x02, XM_GPIO8_IO);
	mdelay(100);

	return designware_initialize(XM_GMAC_BASE, PHY_INTERFACE_MODE_RMII);
}
#endif
