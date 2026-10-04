/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#include <asm/arch-hi3516cv610/platform.h>
#include <dm.h>
#include <usb.h>
#include <usb/xhci.h>
#include <linux/delay.h>
#include "phy-usb.h"

/* OTP USB base register */
#define OTP_REG_BASE                            0x101E0000
#define OTP_USB_MISC                            (OTP_REG_BASE + 0x001C)
#define OTP_USB_PHY_PARA_SEL_OFFSET             0x8
#define OTP_USB_PHY_PARA_SEL_MASK               (0x3 << OTP_USB_PHY_PARA_SEL_OFFSET)
#define OTP_USB2_PHY0_TRIM                      (OTP_REG_BASE + 0x110)
#define OTP_USB2_PHY0_TRIM_OFFSET               21
#define OTP_USB2_PHY0_TRIM_MIN                  (0x9)
#define OTP_USB2_PHY0_TRIM_MAX                  (0x1d)
#define OTP_USB_VENDOR_ID                       (OTP_REG_BASE + 0x20)
#define OTP_USB_VENDOR_ID_MASK                  0xFFFF

/* VBUS IO register */
#define VBUS_IO_REG                             0x10260060
#define VBUS_IO_DEFAULT_VAL                     0x1200
#define OTP_VBUS_MASK                           (0x1 << 14)
/* VBUS_IN_PULL_UP_ENABLE_VAL is for QFN, no vbus pin, pull up internal */
#define VBUS_IN_PULL_UP_ENABLE_VAL              0x1131
/* VBUS_IN_PULL_UP_DISABLE_VAL is for BGA, vbus pin exist */
#define VBUS_IN_PULL_UP_DISABLE_VAL             0x1031

/* USB PHY base register */
#define USB2_0_PHY_BASE_ADDR                    0x10310000
#define USB2_0_PHY_ANA_CFG0_ADDR_OFFSET         0x0
#define USB2_0_PHY_ANA_CFG2_ADDR_OFFSET         0x8
#define USB2_0_PHY_ANA_CFG5_ADDR_OFFSET         0x14
#define USB2_0_PHY_ANA_CFG0_DEFAULT_VAL         0xA33C82B
#define USB2_0_PHY_ANA_CFG2_DEFAULT_VAL         0x0050F0F

#define USB2_0_PHY_PLLCK_MASK                   (0x3U << 0)
#define USB2_0_PHY_PLLCK_VAL                    (0x3U << 0)

/* USB Controller register ,base reg USB_P0_REG_BASE */
#define USB_P0_REG_BASE                         0x10300000
#define REG_GCTL                                0xC110
#define PORT_CAP_DIR_MASK                       (0x3 << 12)
#define PORT_CAP_DIR_DEVICE                     (0x2 << 12)
#define PORT_CAP_DIR_HOST                       (0x1 << 12)
 
/* USB CRG register */
#define REG_BASE_CRG                            0x11010000
#define USB2_0_PHY_CRG_OFFSET                   0x38C0
#define USB2_0_PHY_CRG_DEFAULT_VAL              0x00000033
#define USB2_0_PHY_CRG_APB_SREQ                 (0x1 << 2)
#define USB2_0_PHY_CRG_UTMI_RST_REQ             (0x1 << 1)
#define USB2_0_PHY_CRG_RST_REQ                  (0x1 << 0)
#define USB2_0_PHY_TRIM_OFFSET                  8
#define USB2_0_PHY_TRIM_MASK                    (0x1F << USB2_0_PHY_TRIM_OFFSET)
#define USB2_CRG_CTRL                           0x38C8
#define USB2_CRG_DEFAULT_VAL                    0x00010131
#define USB2_CRG_SRST_REQ                       (0x1 << 0)


#define USB_U32_MAX							    (0xFFFFFFFF)

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

	otp_trim_val = readl(OTP_USB2_PHY0_TRIM) >> OTP_USB2_PHY0_TRIM_OFFSET;
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
	unsigned int reg_val;

	writel(USB2_0_PHY_ANA_CFG0_DEFAULT_VAL, USB2_0_PHY_BASE_ADDR + USB2_0_PHY_ANA_CFG0_ADDR_OFFSET);
	writel(USB2_0_PHY_ANA_CFG2_DEFAULT_VAL, USB2_0_PHY_BASE_ADDR + USB2_0_PHY_ANA_CFG2_ADDR_OFFSET);

	/* trim setting */
	if ((otp_trim_val >= OTP_USB2_PHY0_TRIM_MIN) && (otp_trim_val <= OTP_USB2_PHY0_TRIM_MAX)) {
		reg_val = readl(USB2_0_PHY_BASE_ADDR + USB2_0_PHY_ANA_CFG2_ADDR_OFFSET);
		reg_val &= ~(USB2_0_PHY_TRIM_MASK);
		reg_val |= otp_trim_val << USB2_0_PHY_TRIM_OFFSET;
		writel(reg_val, USB2_0_PHY_BASE_ADDR + USB2_0_PHY_ANA_CFG2_ADDR_OFFSET);
	}
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
	reg = readl(OTP_USB_MISC) & OTP_VBUS_MASK;
	if (reg == OTP_VBUS_MASK) {
		writel(VBUS_IN_PULL_UP_DISABLE_VAL, VBUS_IO_REG);
	} else {
		writel(VBUS_IN_PULL_UP_ENABLE_VAL, VBUS_IO_REG);
	}

	/* phy init step1. default crg value */
	writel(USB2_0_PHY_CRG_DEFAULT_VAL, REG_BASE_CRG + USB2_0_PHY_CRG_OFFSET);

	/* phy init step2. reset por */
	reg = readl(REG_BASE_CRG + USB2_0_PHY_CRG_OFFSET);
	reg &= ~(USB2_0_PHY_CRG_RST_REQ);
	writel(reg, REG_BASE_CRG + USB2_0_PHY_CRG_OFFSET);

	/* phy init step3. pllclk */
	reg = readl(USB2_0_PHY_BASE_ADDR + USB2_0_PHY_ANA_CFG5_ADDR_OFFSET);
	reg |= USB2_0_PHY_PLLCK_VAL;
	writel(reg, USB2_0_PHY_BASE_ADDR + USB2_0_PHY_ANA_CFG5_ADDR_OFFSET);

	/* phy init step4. trim  and step5. eye patten */
	usb_eye_config();

	/* phy init step6.  UTMI rst */
	udelay(U_LEVEL10);
	reg = readl(REG_BASE_CRG + USB2_0_PHY_CRG_OFFSET);
	reg &= ~(USB2_0_PHY_CRG_UTMI_RST_REQ);
	writel(reg, REG_BASE_CRG + USB2_0_PHY_CRG_OFFSET);

	/* ctrl init step1. write default val */
	writel(USB2_CRG_DEFAULT_VAL, REG_BASE_CRG + USB2_CRG_CTRL);

	/* ctrl init step2. ctrl vcc rst */
	reg = readl(REG_BASE_CRG + USB2_CRG_CTRL);
	reg &= ~(USB2_CRG_SRST_REQ);
	writel(reg, REG_BASE_CRG + USB2_CRG_CTRL);
	udelay(U_LEVEL9);

	/* ctrl init step3. controller host mode */
	reg = readl(USB_P0_REG_BASE + REG_GCTL);
	reg &= ~(PORT_CAP_DIR_MASK);
	reg |= (PORT_CAP_DIR_HOST);   /* [13:12] 01: Host; 10: Device; 11: OTG */
	writel(reg, USB_P0_REG_BASE + REG_GCTL);
	local_irq_restore(flags);
}

void xhci_hcd_stop(int index)
{
	unsigned int reg;
	if (index != 0)
		return;

	/* 1. write vbus io default val */
	writel(VBUS_IO_DEFAULT_VAL, VBUS_IO_REG);

	/* 2. write control default val */
	writel(USB2_CRG_DEFAULT_VAL, REG_BASE_CRG + USB2_CRG_CTRL);

	/* 3. eye apb_rst_req b[2]:1 */
	reg = readl(REG_BASE_CRG + USB2_0_PHY_CRG_OFFSET);
	reg |= (USB2_0_PHY_CRG_APB_SREQ);
	writel(reg, REG_BASE_CRG + USB2_0_PHY_CRG_OFFSET);
	udelay(U_LEVEL1);

	/* 4. write phy crg default val */
	writel(USB2_0_PHY_CRG_DEFAULT_VAL, REG_BASE_CRG + USB2_0_PHY_CRG_OFFSET);
}

