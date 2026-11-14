// SPDX-License-Identifier: GPL-2.0+
/*
 * Anyka AK39 camera board, after the vendor U-Boot 2013.10
 * (board/anyka/ak3918/ak3918.c, drivers/net/ak_ethernet.c).
 */

#include <common.h>
#include <env.h>
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

#if defined(CONFIG_ETH_DESIGNWARE) && defined(CONFIG_AK39_AV100)
/*
 * AV100 family (AV100, EV300L), after the AV100 SDK's ak_eth.c, its clock
 * driver and its device trees: RMII to an external PHY, the MAC clock out
 * (OPCLK1, 50 MHz) on pad 0.
 *
 * Pads 0-8 and 11, shared-pin CFG0 fields 26:0 and CFG1 5:3: OPCLK1 (0: 3),
 * RXER (1: 3), RXDV (2: 4), MDIO (3: 2), TXEN (4: 4), MDC (5: 3), RXD1
 * (6: 3), TXD0 (7: 3), TXD1 (8: 5), RXD0 (11: 5), the SDK's EVB and the
 * HVT3610S3 camera's ethernet_rmii pins alike. Input enabled; pull-up on
 * MDIO, pull-downs on the data pins, none on the clock (anyka,pull 0x10010,
 * 0x10011, 0x10200); the clock pad at
 * 8 mA and TXD0/TXD1 fast, as their pin groups say.
 */
#define AK_MAC_PINS0_CLR	0x7ffffff
#define AK_MAC_PINS0		(3 << 0 | 3 << 3 | 4 << 6 | 2 << 9 | 4 << 12 | \
				 3 << 15 | 3 << 18 | 3 << 21 | 5 << 24)
#define AK_MAC_PINS1_CLR	(0x7 << 3)
#define AK_MAC_PINS1		(5 << 3)
#define AK_MAC_PADS		(GENMASK(8, 0) | BIT(11))
#define AK_MAC_PULLS		(AK_MAC_PADS & ~BIT(0))
#define AK_MAC_PULLUPS		BIT(3)
#define AK_SYS_PAD_DRV0		(AK_SYSCTRL_BASE + 0x1a4)	/* 2 bits a pad */
#define AK_SYS_PAD_SLEW0	(AK_SYSCTRL_BASE + 0x1d0)	/* 0 = fast */

/*
 * MAC_OPCLK = PLL2 / (div + 1), SYSCTRL +0x18: enable (bit 8) with the
 * apply bit (9) clear, the divider in 5:0, then apply and wait.
 */
#define AK_SYS_MAC_OPCLK	(AK_SYSCTRL_BASE + 0x18)
#define AK_MAC_OPCLK_EN		BIT(8)
#define AK_MAC_OPCLK_APPLY	BIT(9)
#define AK_MAC_SPEED_100	BIT(23)		/* in +0x14 */
#define AK_RST_MAC		BIT(13)

/* The GPIO function of the default PHY reset pads (function 0 each) */
static const struct {
	u8 gpio, cfg, shift;
} ak_gpio_pads[] = {
	{ 24, 2, 18 }, { 29, 3, 3 }, { 71, 8, 27 },
};

static void ak_gpio_mux(uint gpio)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(ak_gpio_pads); i++)
		if (ak_gpio_pads[i].gpio == gpio)
			clrbits_le32(AK_SYS_SHAREPIN_CFG(ak_gpio_pads[i].cfg),
				     0x7 << ak_gpio_pads[i].shift);
}

/*
 * The PHY reset (active low) is board wiring: GPIO 71 on the HVT3610S3,
 * 24 on the SDK's AV100N EVB, 29 on its AV100X EVB, 49, 55 or 57 on other
 * SDK boards. The default is OpenIPC's per SoC, from the board it stands
 * for; "phyreset=<gpio>" overrides it, "phyreset=none" (or -1) skips it.
 */
#if defined(CONFIG_TARGET_AK3918EV300L)
#define AK_AV100_PHY_RESET	71
#elif defined(CONFIG_TARGET_AK3918AV100X)
#define AK_AV100_PHY_RESET	29
#else
#define AK_AV100_PHY_RESET	24
#endif
static void ak_phy_reset(void)
{
	const char *s = env_get("phyreset");
	long gpio = AK_AV100_PHY_RESET;

	if (s)
		gpio = (*s >= '0' && *s <= '9') ? simple_strtol(s, NULL, 10) : -1;
	if (gpio < 0 || gpio > 104)
		return;
	ak_gpio_mux(gpio);
	setbits_le32(AK_GPIO_OUT(gpio), BIT(gpio % 32));
	setbits_le32(AK_GPIO_DIR(gpio), BIT(gpio % 32));
	mdelay(5);
	/*
	 * The timing of the HVT3610S3's own ak_eth.ko, which finds the PHY:
	 * 20 ms low, then 200 ms before the first MDIO access.
	 */
	clrbits_le32(AK_GPIO_OUT(gpio), BIT(gpio % 32));
	mdelay(20);
	setbits_le32(AK_GPIO_OUT(gpio), BIT(gpio % 32));
	mdelay(200);
}

static void ak_eth_init(void)
{
	uint div = DIV_ROUND_UP(ak_pll2_clk(), 50000000) - 1;
	int spins = 1000000;

	clrsetbits_le32(AK_SYS_SHAREPIN_CFG(0), AK_MAC_PINS0_CLR, AK_MAC_PINS0);
	clrsetbits_le32(AK_SYS_SHAREPIN_CFG(1), AK_MAC_PINS1_CLR, AK_MAC_PINS1);
	setbits_le32(AK_SYS_PAD_IE(0), AK_MAC_PADS);
	clrbits_le32(AK_SYS_PUPD_EN(0), BIT(0));
	setbits_le32(AK_SYS_PUPD_EN(0), AK_MAC_PULLS);
	/*
	 * PU_PD_SEL: 1 = pull-up (the vendor pinctrl drivers' "pullup"; in
	 * their anyka,pull words bit 0 set means pull-down).
	 */
	clrsetbits_le32(AK_SYS_PUPD_SEL(0), AK_MAC_PULLS & ~AK_MAC_PULLUPS,
			AK_MAC_PULLUPS);
#ifdef CONFIG_TARGET_AK3918EV300L
	/*
	 * Pad 23, the HVT3610S3 DT's ethernet_rst_pins: GPIO (function 0),
	 * input, pulled up, never driven (its reset-gpios is 71, AIN0). On
	 * the SDK's EVBs that group is the PHY reset pin itself (24), pulled
	 * up the same way. Its vendor Linux sets this and finds the PHY;
	 * U-Boot left the pad alone and found none.
	 */
	clrbits_le32(AK_SYS_SHAREPIN_CFG(2), 0x7 << 12);
	clrbits_le32(AK_GPIO_DIR(23), BIT(23));
	setbits_le32(AK_SYS_PAD_IE(0), BIT(23));
	setbits_le32(AK_SYS_PUPD_EN(0), BIT(23));
	setbits_le32(AK_SYS_PUPD_SEL(0), BIT(23));
	mdelay(10);
#endif
	clrsetbits_le32(AK_SYS_PAD_DRV0, 0x3, 0x2);
	clrbits_le32(AK_SYS_PAD_SLEW0, BIT(7) | BIT(8));

	/* 100 Mbit/s, the 50 MHz RMII clock, the MAC's bus clock */
	setbits_le32(AK_SYS_MAC_CLK, AK_MAC_SPEED_100);
	clrbits_le32(AK_SYS_MAC_OPCLK, AK_MAC_OPCLK_APPLY);
	setbits_le32(AK_SYS_MAC_OPCLK, AK_MAC_OPCLK_EN);
	clrsetbits_le32(AK_SYS_MAC_OPCLK, 0x3f, min(div, 0x3fU));
	setbits_le32(AK_SYS_MAC_OPCLK, AK_MAC_OPCLK_APPLY);
	while ((readl(AK_SYS_MAC_OPCLK) & AK_MAC_OPCLK_APPLY) && --spins)
		;
	if (!spins)
		printf("ETH:   MAC clock divider not taken\n");
	clrbits_le32(AK_SYS_CLK_GATE, AK_CLK_MAC);
	/*
	 * Only release the MAC reset, never assert it: the SDK's ak_eth.c
	 * says that on this generation a MAC reset after the first release
	 * also resets the DDR controller (the MAC's DMA buffer is not fully
	 * reset and its FIFO pointers go wrong). The DesignWare driver's own
	 * DMA soft reset is enough.
	 */
	clrbits_le32(AK_SYS_SOFT_RST, AK_RST_MAC);

	ak_phy_reset();
}

int board_eth_init(struct bd_info *bis)
{
	ak_eth_init();
	return designware_initialize(AK_GMAC_BASE, PHY_INTERFACE_MODE_RMII);
}
#elif defined(CONFIG_ETH_DESIGNWARE)
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
