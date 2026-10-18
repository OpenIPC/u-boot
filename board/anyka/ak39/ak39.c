// SPDX-License-Identifier: GPL-2.0+
/*
 * Anyka AK39 camera board, after the vendor U-Boot 2013.10
 * (board/anyka/ak3918/ak3918.c, drivers/net/ak_ethernet.c).
 */

#include <common.h>
#include <init.h>
#include <net.h>
#include <netdev.h>
#include <openipc.h>
#include <phy.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/anyka.h>
#include <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	gd->bd->bi_boot_params = AK_DDR_BASE + 0x100;
	return 0;
}

int misc_init_r(void)
{
	return openipc_helper();
}

#ifdef CONFIG_ETH_DESIGNWARE
/*
 * The EV300 vendor U-Boot's RMII set-up: the MAC pins, the 50 MHz PHY
 * clock out on GPIO 47 and a PHY reset pulse on GPIO 0. The EV330 has the
 * same shared-pin layout. The EV200's is different and not known, so it
 * gets only the MAC itself.
 */
static void ak_rmii_init(void)
{
#ifndef CONFIG_AK39_EV200
	int i;

	clrsetbits_le32(AK_SYS_SHAREPIN0, 0x3 << 10, 0x1 << 10);
	clrsetbits_le32(AK_SYS_SHAREPIN2, 0x0fffffff, 0x1450525);
	/* GPIO 12, the PHY interrupt, an input */
	clrbits_le32(AK_SYS_SHAREPIN2, BIT(4));
	clrbits_le32(AK_GPIO_DIR(12), BIT(12));
	/* No pull-down on GPIO 47, the clock out */
	setbits_le32(AK_SYS_PULL0, BIT(8));

	/* RMII, 100 Mbit/s, 50 MHz from the PLL */
	clrbits_le32(AK_SYS_MAC_CLK, BIT(22));
	setbits_le32(AK_SYS_MAC_CLK, BIT(23) | BIT(21) | BIT(18) | BIT(16));
	clrbits_le32(AK_SYS_CLK_GATE, AK_CLK_MAC);
	/* The vendor toggles the clock source select, "at least twice" */
	for (i = 0; i < 6; i++) {
		setbits_le32(AK_SYS_MAC_CLK, BIT(20));
		clrbits_le32(AK_SYS_MAC_CLK, BIT(20));
	}

	/* PHY reset on GPIO 0 */
	clrbits_le32(AK_SYS_SHAREPIN0, 0x3);
	setbits_le32(AK_GPIO_DIR(0), BIT(0));
	setbits_le32(AK_GPIO_OUT(0), BIT(0));
	mdelay(5);
	clrbits_le32(AK_GPIO_OUT(0), BIT(0));
	mdelay(25);
	setbits_le32(AK_GPIO_OUT(0), BIT(0));
	mdelay(5);
#else
	clrbits_le32(AK_SYS_CLK_GATE, AK_CLK_MAC);
#endif
}

int board_eth_init(struct bd_info *bis)
{
	ak_rmii_init();
	return designware_initialize(AK_GMAC_BASE, PHY_INTERFACE_MODE_RMII);
}
#endif
