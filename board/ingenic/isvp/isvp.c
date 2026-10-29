// SPDX-License-Identifier: GPL-2.0+
/*
 * Ingenic ISVP boards (the T-series camera reference design and the
 * cameras built on it): board/ingenic/isvp{,_t20,_t21,_t23,_t30,_t31}/board.c of
 * OpenIPC/u-boot-ingenic and board/ingenic/isvp_t40/board.c of
 * OpenIPC/u-boot-t40, one file for every SoC.
 *
 * The board-specific parts are what OpenIPC added there for cameras that
 * differ from the reference board: SD card power switches on GPIOs and,
 * when there is no Ethernet PHY, "nogmac" for the kernel.
 *
 * Copyright (c) 2013-2019 Ingenic Semiconductor Co.,Ltd
 */

#include <common.h>
#include <env.h>
#include <init.h>
#include <net.h>
#include <netdev.h>
#include <openipc.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/clk.h>
#include <mach/ingenic.h>

/* The clock parents clk_init() sets in the SPL */
struct cgu_clk_src cgu_clk_src[] = {
#if defined(CONFIG_T10)
	{VPU, MPLL},
	{MACPHY, MPLL},
	{MSC, APLL},
	{SSI, MPLL},
	{CIM, MPLL},
	{ISP, MPLL},
	{I2S, APLL},
#elif defined(CONFIG_T20)
	{VPU, MPLL},
	{MACPHY, MPLL},
	{MSC, APLL},
	{SSI, MPLL},
	{CIM, VPLL},
	{ISP, MPLL},
	{I2S, APLL},
#elif defined(CONFIG_T21) || defined(CONFIG_T30)
	{HELIX, MPLL},
	{MACPHY, MPLL},
	{MSC, APLL},
	{SSI, MPLL},
	{CIM, VPLL},
	{ISP, MPLL},
	{I2S, VPLL},
#elif defined(CONFIG_T23)
	{AVPU, MPLL},
	{MACPHY, CONFIG_MACPHY_SEL_PLL},
	{MSC, APLL},
	{SSI, MPLL},
	{CIM, MPLL},
	{ISP, MPLL},
	{I2S_SPK, APLL},
	{I2S_MIC, APLL},
#elif defined(CONFIG_T31)
	{AVPU, MPLL},
	{MACPHY, MPLL},
	{MSC, APLL},
	{SSI, MPLL},
	{CIM, VPLL},
	{ISP, MPLL},
	{I2S_SPK, APLL},
	{I2S_MIC, APLL},
#elif defined(CONFIG_T40)
	{EL150, APLL},
	{MACPHY, MPLL},
	{MSC, MPLL},
	{SFC_CLK, MPLL},
	{CIM0, APLL},
	{CIM1, APLL},
	{CIM2, APLL},
	{ISP, MPLL},
	{I2S, APLL},
#elif defined(CONFIG_T41)
	{EL200, APLL},
	{MACPHY, MPLL},
	{MSC, APLL},
	{MSC1, MPLL},
	{SFC0, MPLL},
	{SFC1, MPLL},
	{CIM0, APLL},
	{ISPA, MPLL},
	{ISPS, MPLL},
	{ISPM, MPLL},
	{I2S, APLL},
#endif
	{SRC_EOF, SRC_EOF}
};

#ifndef CONFIG_SPL_BUILD

int jz_net_initialize(struct bd_info *bis);
void jz_mmc_init(void);

int board_early_init_f(void)
{
#if defined(CONFIG_T23)
	/* UART1 PB23/PB24 pull-up, for boards without external pull-ups */
	writel(0x1800000, 0xb0011114);
	writel(0x1800000, 0xb0011128);
	/* PB06 (MAC TX clock) input with pull-down, less clock coupling */
	writel(0x40, 0xb0011118);
	writel(0x40, 0xb0011124);
	/* MSC0 pins drive low by default, against their leakage */
	writel(0x3f, 0xb0011018);
	writel(0x3f, 0xb0011024);
	writel(0x3f, 0xb0011038);
	writel(0x3f, 0xb0011048);
#elif defined(CONFIG_T40)
	/* GPIO port A PA00-PA22 at 1.8 V, as the T40 vendor board does */
	writel(0x1, GPIO_BASE + PXGPDCRS);
#elif defined(CONFIG_T41)
	/* The T41 vendor board leaves PA06-PA19 at their default, 1.8 V */
#elif defined(CONFIG_T31) && !defined(CONFIG_T31A) && !defined(CONFIG_T31AL)
	/* PA25/PA26 (flash IO2/IO3) inputs without pulls, except on the
	 * quad-capable T31A/T31AL */
	gpio_direction_input(25);
	gpio_direction_input(26);
	gpio_disable_pull_down(25);
	gpio_disable_pull_up(25);
	gpio_disable_pull_down(26);
	gpio_disable_pull_up(26);
#endif
	return 0;
}

int misc_init_r(void)
{
#if defined(CONFIG_T23) || defined(CONFIG_T31)
	/* used for usb_dete */
	gpio_enable_pull_up(GPIO_PB(24));	/* UART1 rx */
#endif
#if defined(CONFIG_T31)
	/* HiChip boards: SD card power */
	gpio_request(62, "hichip_mmc_enable");
	gpio_direction_output(62, 0);
#endif
	return 0;
}

/*
 * After the network: board_eth_init() powers the SD card on the cameras
 * without an Ethernet PHY, and the vendor U-Boot ran its SD update only
 * after eth_initialize() too.
 */
int last_stage_init(void)
{
	openipc_helper();
	return 0;
}

#if defined(CONFIG_JZ_MMC) || defined(CONFIG_JZ_SDHCI)
int board_mmc_init(struct bd_info *bd)
{
	jz_mmc_init();
	return 0;
}
#endif

#ifdef CONFIG_JZ_GMAC
int board_eth_init(struct bd_info *bis)
{
	int ret = jz_net_initialize(bis);

	if (ret >= 0)
		return ret;

	/* No Ethernet PHY: a camera that uses these pins otherwise */
#if defined(CONFIG_T20)
	/* Wyze V2: USB and SD card power, SD card detect pulse */
	gpio_request(47, "wyze_usb_enable");
	gpio_direction_output(47, 0);
	gpio_request(43, "wyze_cd_enable");
	gpio_direction_output(43, 1);
	gpio_request(48, "wyze_mmc_enable");
	gpio_direction_output(48, 0);
	udelay(1000);
	gpio_direction_output(43, 0);
	if (!env_get("extras"))
		env_set("extras", "nogmac");
#elif defined(CONFIG_T31)
	/* PPS */
	gpio_request(61, "pps_mmc_enable");
	gpio_direction_output(61, 0);
	/* Wyze V3 */
	gpio_request(48, "wyze_mmc_enable");
	gpio_direction_output(48, 0);
	if (!env_get("extras"))
		env_set("extras", "nogmac");
#endif
	return ret;
}
#endif /* CONFIG_JZ_GMAC */

int checkboard(void)
{
	printf("Board: ISVP (Ingenic XBurst %s SoC)\n", CONFIG_PRODUCT_SOC);
	return 0;
}

#endif /* !CONFIG_SPL_BUILD */
