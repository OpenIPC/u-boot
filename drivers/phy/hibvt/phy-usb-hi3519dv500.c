/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 *
 * From the vendor SDK U-Boot (Hi3519DV500 SDK V1.0.1.1, drivers/phy/vendor/
 * phy-usb-hi3519dv500.c), as OpenIPC/u-boot-hi3519dv500 carried it; register
 * addresses from asm/arch-hi3519dv500/platform.h.
 */
#include <asm/arch-hi3519dv500/platform.h>
#include <dm.h>
#include <usb.h>
#include <usb/xhci.h>
#include <linux/delay.h>
#include "phy-usb.h"

#define PINOUT_REG_VBUS					0x179F0010
#define PINOUT_USB_VAL					0x1201
#define USB3_MISC_REG					0x17A40208
#define USB3_MISC_DISABLE_SS_BIT		(0x1U << 9)

/* USB CRG configuration */
#define USB2_PHY_CRG_DEFAULT_VALUE		0x107
#define USB3_PHY_CRG_DEFAULT_VALUE		0x3
#define USB3_CTRL_CRG_DEFAULT_VALUE		0x10003

#define USB3_CRG_PCLK_OCC_SEL		(0x1U << 18)
#define USB3_CRG_FREECLK_CKSEL		(0x1U << 16)
#define USB3_CRG_PIPE_CKEN			(0x1U << 12)
#define USB3_CRG_UTMI_CKEN			(0x1U << 8)
#define USB3_CRG_SUSPEND_CKEN		(0x1U << 6)
#define USB3_CRG_REF_CKEN			(0x1U << 5)
#define USB3_CRG_BUS_CKEN			(0x1U << 4)
#define USB3_CRG_SRST_REQ			(0x1U << 0)

#define USB3_PHY_CRG_REF_CKEN		(0x1U << 4)
#define USB3_PHY_CRG_APB_CKEN		(0x1U << 2)
#define USB3_PHY_CRG_SRST_REQ		(0x1U << 1)
#define USB3_PHY_SRST_REQ			(0x1U << 0)

#define USB2_PHY_XTAIL_CKEN			(0x1U << 4)
#define USB2_PHY_APB_CKEN			(0x1U << 6)
#define USB2_PHY_APB_SRST_REQ		(0x1U << 2)
#define USB2_PHY_CRG_TREQ			(0x1U << 1)
#define USB2_PHY_CRG_REQ			(0x1U << 0)

/* USB CTRL register offset and config */
#define GTXTHRCFG					0xc108
#define GRXTHRCFG					0xc10c
#define REG_GCTL					0xc110

#define USB_TXPKT_CNT_SEL			(0x1U << 29)
#define USB_TXPKT_CNT				(0x11U << 24)
#define USB_MAXTX_BURST_SIZE		(0x1U << 20)
#define CLEAN_USB3_GTXTHRCFG		0x0

#define REG_GUSB3PIPECTL0			0xc2c0
#define PCS_SSP_SOFT_RESET			(0x1U << 31)
#define SUSPEND_ENABLE				(0x1U <<17)

#define PORT_CAP_DIR_MASK			(0x3U << 12)
#define PORT_CAP_DIR_HOST			(0x1U << 12)

/* USB2.0 PHY releated */
#define USB2_PHY_ADDR_OFFSET	(0x14)
#define USB2_PHY_PLLCK_MASK		(0x3)

#define OTP_USB_VENDOR_ID (0x101E0020)
#define OTP_USB_VENDOR_ID_MASK 0xFFFF

/* U2 Performance optimization */
#define U2_PHY_ANA_CFG0						(0x0)
#define U2_PHY_RG_HSTX_MBIAS_MASK			(0xF << 0)
#define U2_PHY_RG_HSTX_MBIAS_VAL			((0xB << 0) & U2_PHY_RG_HSTX_MBIAS_MASK)

#define U2_PHY_RG_HSTX_DEEN_MASK			(0x1 << 5)
#define U2_PHY_RG_HSTX_DEEN_VAL				((0x1 << 5) & U2_PHY_RG_HSTX_DEEN_MASK)

#define U2_PHY_RG_HSTX_DE_MASK				(0xF << 8)
#define U2_PHY_RG_HSTX_DE_VAL				((0x4 << 8) & U2_PHY_RG_HSTX_DE_MASK)

#define U2_PHY_ANA_CFG2						(0x8)
#define U2_PHY_RG_RT_TRIM_MASK				(0x1F << 8)
#define U2_PHY_RG_RT_TRIM_VAL				((0x18 << 8) & U2_PHY_RG_RT_TRIM_MASK)

#define U2_PHY_RG_VDISCREF_SEL_MASK			(0x7 << 16)
#define U2_PHY_RG_VDISCREF_SEL_VAL			((0x5 <<16) & U2_PHY_RG_VDISCREF_SEL_MASK)

#define U2_PHY_RG_TEST_TX_MASK				(0x3 << 20)
#define U2_PHY_RG_TEST_TX_VAL				((0x3 << 20) & U2_PHY_RG_TEST_TX_MASK)

/* OTP USB2 PHY0 TRIM */
#define OTP_REG_BASE						0x101E0000
#define OTP_USB2_PHY0_TRIM					(0x124)
#define USB2_PHY_TRIM_MASK					(0x1F)
#define USB2_PHY_TRIM_MAX					(0x1D)
#define ANA_CFG2_TRIM_SHIFT					(8)
#define USB2_PHY_TRIM_MIN					(0x9)
#define USB_U32_MAX							(0xFFFFFFFF)

static int otp_trim_val = USB_U32_MAX;

uint16_t get_usb_vendor_id(void)
{
	uint32_t reg_val;

	reg_val = readl(OTP_USB_VENDOR_ID);
	reg_val &= OTP_USB_VENDOR_ID_MASK;

	return (uint16_t)reg_val;
}

static void get_trim_from_otp(void)
{
	if (otp_trim_val != USB_U32_MAX)
		return;

	otp_trim_val = readl(OTP_REG_BASE + OTP_USB2_PHY0_TRIM);
}

int xhci_hcd_init(int index, struct xhci_hccr **ret_hccr, struct xhci_hcor **ret_hcor)
{
	if (index != 0)
		return -1;

	*ret_hccr = (struct xhci_hccr *)(USB3_CTRL_REG_BASE);
	*ret_hcor = (struct xhci_hcor *)((long) *ret_hccr
			+ HC_LENGTH(xhci_readl(&(*ret_hccr)->cr_capbase)));

	return 0;
}

void usb_eye_config(void)
{
	unsigned int reg, reg_trim;

	reg = readl(USB2_PHY_BASE_ADDR + U2_PHY_ANA_CFG0);
	reg &= ~(U2_PHY_RG_HSTX_MBIAS_MASK | U2_PHY_RG_HSTX_DEEN_MASK | U2_PHY_RG_HSTX_DE_MASK);
	reg |= (U2_PHY_RG_HSTX_MBIAS_VAL | U2_PHY_RG_HSTX_DEEN_VAL | U2_PHY_RG_HSTX_DE_VAL);
	writel(reg, USB2_PHY_BASE_ADDR + U2_PHY_ANA_CFG0);
	udelay(U_LEVEL6);

	reg = readl(USB2_PHY_BASE_ADDR + U2_PHY_ANA_CFG2);
	reg &= ~(U2_PHY_RG_VDISCREF_SEL_MASK | U2_PHY_RG_TEST_TX_MASK);
	reg |= (U2_PHY_RG_VDISCREF_SEL_VAL | U2_PHY_RG_TEST_TX_VAL);
	writel(reg, USB2_PHY_BASE_ADDR + U2_PHY_ANA_CFG2);
	udelay(U_LEVEL6);

	reg = readl(USB2_PHY_BASE_ADDR + U2_PHY_ANA_CFG2);
	reg &= ~(U2_PHY_RG_RT_TRIM_MASK);
	reg |= U2_PHY_RG_RT_TRIM_VAL;

	/* trim val */
	if (otp_trim_val == USB_U32_MAX)
		goto trim_default;

	reg_trim = otp_trim_val;
	reg_trim = (reg_trim & USB2_PHY_TRIM_MASK);
	if ((reg_trim < USB2_PHY_TRIM_MIN) || (reg_trim > USB2_PHY_TRIM_MAX))
		goto trim_default;

	reg &= ~(USB2_PHY_TRIM_MASK << ANA_CFG2_TRIM_SHIFT);
	reg |= (reg_trim << ANA_CFG2_TRIM_SHIFT);

trim_default:
	writel(reg, USB2_PHY_BASE_ADDR + U2_PHY_ANA_CFG2);
	return;
}

/* phy_usb_init
 * Desc: usb phy initialization. After initialization, some registers(CRG)
 *       are the same as excuting following instructions:
 * writel(0x150, 0x110138c0);
 * writel(0x3, 0x17350014);
 * writel(0x00051172, 0x11013940);
*/
void phy_usb_init(int index)
{
	unsigned long flags;
	unsigned int reg;

	local_irq_save(flags);

	if (otp_trim_val == USB_U32_MAX)
		get_trim_from_otp();

	/* 0. VBUS pin_out config */
	writel(PINOUT_USB_VAL, PINOUT_REG_VBUS);

	/* 1. default crg value */
	writel(USB3_CTRL_CRG_DEFAULT_VALUE, USB3_CTRL_CRG);
	writel(USB3_PHY_CRG_DEFAULT_VALUE, USB3_PHY_CRG);
	writel(USB2_PHY_CRG_DEFAULT_VALUE, USB2_PHY_CRG);

	/* 2. disable SuperSpeed */
	reg = readl(USB3_MISC_REG);
	reg |= (USB3_MISC_DISABLE_SS_BIT);
	writel(reg, USB3_MISC_REG);

	/* 3. ctrl crg cken */
	reg = readl(USB3_CTRL_CRG);
	reg |= (USB3_CRG_PCLK_OCC_SEL | USB3_CRG_FREECLK_CKSEL |
			USB3_CRG_PIPE_CKEN | USB3_CRG_UTMI_CKEN |
			USB3_CRG_SUSPEND_CKEN | USB3_CRG_REF_CKEN |
			USB3_CRG_BUS_CKEN);
	writel(reg, USB3_CTRL_CRG);

	/* 4. U2 phy crg */
	reg = readl(USB2_PHY_CRG);
	reg |= (USB2_PHY_XTAIL_CKEN | USB2_PHY_APB_CKEN);
	reg &= ~(USB2_PHY_APB_SRST_REQ);
	writel(reg, USB2_PHY_CRG);
	udelay(U_LEVEL5);

	/* 5. eye patten */
	usb_eye_config();
	udelay(U_LEVEL5);

	/* 6. U2 phy pll clk */
	reg = readl(USB2_PHY_CRG);
	reg &= ~USB2_PHY_CRG_REQ;
	writel(reg, USB2_PHY_CRG);

	reg = readl(USB2_PHY_BASE_ADDR + USB2_PHY_ADDR_OFFSET);
	reg &= USB2_PHY_PLLCK_MASK;
	writel(reg, USB2_PHY_BASE_ADDR + USB2_PHY_ADDR_OFFSET);

	udelay(U_LEVEL10);

	/* 7. U2 phy TREQ */
	reg = readl(USB2_PHY_CRG);
	reg &= ~USB2_PHY_CRG_TREQ;
	writel(reg, USB2_PHY_CRG);
	udelay(U_LEVEL6);

	/* 8. ctrl vcc rst */
	reg = readl(USB3_CTRL_CRG);
	reg &= ~USB3_CRG_SRST_REQ;
	writel(reg, USB3_CTRL_CRG);
	udelay(U_LEVEL6);

	/* 9. controller configuration */
	reg = readl(USB3_CTRL_REG_BASE + REG_GUSB3PIPECTL0);
	reg |= PCS_SSP_SOFT_RESET;
	writel(reg, USB3_CTRL_REG_BASE + REG_GUSB3PIPECTL0);
	udelay(U_LEVEL6);

	reg = readl(USB3_CTRL_REG_BASE + REG_GCTL);
	reg &= ~PORT_CAP_DIR_MASK;
	reg |= PORT_CAP_DIR_HOST;   /* [13:12] 01: Host; 10: Device; 11: OTG */
	writel(reg, USB3_CTRL_REG_BASE + REG_GCTL);
	udelay(U_LEVEL6);

	reg = readl(USB3_CTRL_REG_BASE + REG_GUSB3PIPECTL0);
	reg &= ~(PCS_SSP_SOFT_RESET | SUSPEND_ENABLE);
	writel(reg, USB3_CTRL_REG_BASE + REG_GUSB3PIPECTL0);
	udelay(U_LEVEL6);

	reg &= CLEAN_USB3_GTXTHRCFG;
	reg |= USB_TXPKT_CNT_SEL;
	reg |= USB_TXPKT_CNT;
	reg |= USB_MAXTX_BURST_SIZE;
	writel(reg, USB3_CTRL_REG_BASE + GTXTHRCFG);
	writel(reg, USB3_CTRL_REG_BASE + GRXTHRCFG);
	udelay(U_LEVEL6);

	local_irq_restore(flags);
}

void xhci_hcd_stop(int index)
{
	if (index != 0)
		return;

	/* write default crg value */
	writel(USB3_CTRL_CRG_DEFAULT_VALUE, USB3_CTRL_CRG);
	writel(USB3_PHY_CRG_DEFAULT_VALUE, USB3_PHY_CRG);
	writel(USB2_PHY_CRG_DEFAULT_VALUE, USB2_PHY_CRG);
	udelay(U_LEVEL6);
}

