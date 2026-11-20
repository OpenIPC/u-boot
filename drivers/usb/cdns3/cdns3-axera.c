// SPDX-License-Identifier: GPL-2.0+
/*
 * Axera AX620U glue for the Cadence USB3 dual-role controller
 * ("axera,cdns-usb", the node around "cdns,usb3" in the cameras' U-Boot
 * tree): its resets, PHY controls and clocks in the AP clock block
 * (0x49ba000), set before the cdns3 core probes the controller.
 *
 * No AX620U source is published, and the cameras' U-Boot has no USB
 * host. Written from their 4.19 kernel (CONFIG_USB_CDNS3_AXERA): its
 * "axera usb init" (0xc054f490 in the image) and the clocks it then
 * enables, whose gates, muxes and divider come from the kernel's AP
 * clock tables (the struct layout of Axera's AX620E clock driver).
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/compat.h>
#include <linux/delay.h>
#include <linux/usb/otg.h>

#include "core.h"

#define AX620U_AP_CLK		0x049ba000
/* The clock muxes: stb_predft 0 = 32 kHz crystal; otg_fast 0 = 24 MHz */
#define AX_USB_CLK_MUX		(AX620U_AP_CLK + 0x000)
#define  AX_STB_PREDFT_SEL	BIT(28)
#define  AX_OTG_FAST_SEL_MASK	GENMASK(24, 23)
/* otg_fast's divider (value + 1) and its update strobe */
#define AX_USB_CLK_DIV		(AX620U_AP_CLK + 0x018)
#define  AX_OTG_FAST_DIV_MASK	GENMASK(14, 11)
#define  AX_OTG_FAST_DIV_UP	BIT(15)
#define AX_USB_SW_RST		(AX620U_AP_CLK + 0x024)
#define  AX_USB_RST		GENMASK(11, 9)
#define AX_USB_CTRL0		(AX620U_AP_CLK + 0x044)
#define  AX_USB_CTRL0_CLR	BIT(1)
#define AX_USB_CTRL1		(AX620U_AP_CLK + 0x25c)
#define  AX_USB_CTRL1_SET	GENMASK(22, 20)
/* The gates' write-one-to-set aliases (of +0x08, +0x0c, +0x14) */
#define AX_CLK_EB0_SET		(AX620U_AP_CLK + 0x1fc)
#define  AX_LPM_PREDFT_EB	BIT(13)
#define  AX_OTG_FAST_EB		BIT(14)
#define  AX_STB_PREDFT_EB	BIT(16)
#define  AX_USB2PHY_EB		BIT(17)
#define  AX_USB_REF_EB		BIT(18)
#define AX_CLK_EB1_SET		(AX620U_AP_CLK + 0x204)
#define  AX_ACLK_USB_EB		BIT(3)
#define AX_CLK_EB3_SET		(AX620U_AP_CLK + 0x214)
#define  AX_PCLK_USB_EB		BIT(21)

static int ax620u_usb_probe(struct udevice *dev)
{
	/* In the stock order: reset, the two controls, then the clocks */
	setbits_le32(AX_USB_SW_RST, AX_USB_RST);
	udelay(2000);
	clrbits_le32(AX_USB_SW_RST, AX_USB_RST);
	clrbits_le32(AX_USB_CTRL0, AX_USB_CTRL0_CLR);
	setbits_le32(AX_USB_CTRL1, AX_USB_CTRL1_SET);

	clrbits_le32(AX_USB_CLK_MUX, AX_STB_PREDFT_SEL | AX_OTG_FAST_SEL_MASK);
	clrbits_le32(AX_USB_CLK_DIV, AX_OTG_FAST_DIV_MASK);
	setbits_le32(AX_USB_CLK_DIV, AX_OTG_FAST_DIV_UP);
	mdelay(1);
	clrbits_le32(AX_USB_CLK_DIV, AX_OTG_FAST_DIV_UP);

	writel(AX_PCLK_USB_EB, AX_CLK_EB3_SET);
	writel(AX_ACLK_USB_EB, AX_CLK_EB1_SET);
	writel(AX_USB_REF_EB | AX_USB2PHY_EB | AX_STB_PREDFT_EB |
	       AX_OTG_FAST_EB | AX_LPM_PREDFT_EB, AX_CLK_EB0_SET);

	return 0;
}

static const struct udevice_id ax620u_usb_ids[] = {
	{ .compatible = "axera,cdns-usb" },
	{ }
};

U_BOOT_DRIVER(cdns3_axera) = {
	.name = "cdns3-axera",
	.id = UCLASS_NOP,
	.of_match = ax620u_usb_ids,
	.bind = cdns3_bind,
	.probe = ax620u_usb_probe,
};
