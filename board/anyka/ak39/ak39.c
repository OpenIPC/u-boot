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
 * What the vendor U-Boots do before their GMAC driver runs: the MAC pins,
 * the PHY clock out on GPIO 47, the MAC clock, a MAC reset.
 *
 * - EV300 (SDK, and the same shared-pin layout on the EV330): RMII, the
 *   PHY clock 50 MHz, and a PHY reset pulse on GPIO 0.
 * - EV200 (the V380E2 camera's U-Boot, traced): MII, 25 MHz, no PHY reset.
 *   Its shared-pin register 0 is one bit narrower at the bottom, so GPIO
 *   47's field is bits 10:9 (the 4.4 kernel port's pinctrl table agrees);
 *   the MAC pins, GPIO 10-24 in shared-pin register 2, are the EV300's.
 */
#ifdef CONFIG_AK39_EV200
#define AK_ETH_MODE		PHY_INTERFACE_MODE_MII
#define AK_GPIO47_SHIFT		9
#define AK_MAC_PINS		0x575f5a5	/* MII */
#define AK_MAC_CLK_SET		(BIT(23) | BIT(22) | BIT(18) | BIT(16))
#define AK_MAC_CLK_CLR		BIT(15)
#else
#define AK_ETH_MODE		PHY_INTERFACE_MODE_RMII
#define AK_GPIO47_SHIFT		10
#define AK_MAC_PINS		0x1450525	/* RMII */
#define AK_MAC_CLK_SET		(BIT(23) | BIT(21) | BIT(18) | BIT(16))
#define AK_MAC_CLK_CLR		BIT(22)
#endif

#define AK_RST_MAC		BIT(13)

static void ak_eth_init(void)
{
	int i;

	clrsetbits_le32(AK_SYS_SHAREPIN0, 0x3 << AK_GPIO47_SHIFT,
			0x1 << AK_GPIO47_SHIFT);
	clrsetbits_le32(AK_SYS_SHAREPIN2, 0x0fffffff, AK_MAC_PINS);
	/* GPIO 12, the PHY interrupt, an input */
	clrbits_le32(AK_SYS_SHAREPIN2, BIT(4));
	clrbits_le32(AK_GPIO_DIR(12), BIT(12));
	/* No pull-down on GPIO 47, the clock out */
	setbits_le32(AK_SYS_PULL0, BIT(8));

	/* 100 Mbit/s, the PHY clock from the PLL */
	clrsetbits_le32(AK_SYS_MAC_CLK, AK_MAC_CLK_CLR, AK_MAC_CLK_SET);
	clrbits_le32(AK_SYS_CLK_GATE, AK_CLK_MAC);
	/* The vendor toggles the clock source select, "at least twice" */
	for (i = 0; i < 6; i++) {
		setbits_le32(AK_SYS_MAC_CLK, BIT(20));
		clrbits_le32(AK_SYS_MAC_CLK, BIT(20));
	}

	setbits_le32(AK_SYS_SOFT_RST, AK_RST_MAC);
	mdelay(5);
	clrbits_le32(AK_SYS_SOFT_RST, AK_RST_MAC);

#ifndef CONFIG_AK39_EV200
	/* PHY reset on GPIO 0 */
	clrbits_le32(AK_SYS_SHAREPIN0, 0x3);
	setbits_le32(AK_GPIO_DIR(0), BIT(0));
	setbits_le32(AK_GPIO_OUT(0), BIT(0));
	mdelay(5);
	clrbits_le32(AK_GPIO_OUT(0), BIT(0));
	mdelay(25);
	setbits_le32(AK_GPIO_OUT(0), BIT(0));
	mdelay(5);
#endif
}

int board_eth_init(struct bd_info *bis)
{
	ak_eth_init();
	return designware_initialize(AK_GMAC_BASE, AK_ETH_MODE);
}
#endif
