/*
 * USB2/USB3 PHY + XHCI glue for hi3519v101.
 *
 * Derived from HiSilicon SDK hiusb-xhci-3519.c
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/platform.h>
#include <usb/xhci.h>
#include <linux/delay.h>

#define USB3_CTRL_REG_BASE	0x10180000
#define IO_REG_USB3_CTRL	(CRG_REG_BASE + 0xb8)
#define USB3_VCC_SRST_REQ2	(1 << 0)

#define GTXTHRCFG		0xc108
#define GRXTHRCFG		0xc10c
#define REG_GCTL		0xc110
#define REG_GUSB2PHYCFG0	0xc200
#define BIT_UTMI_ULPI		(0x1 << 4)
#define BIT_UTMI_8_16		(0x1 << 3)
#define REG_GUSB3PIPECTL0	0xc2c0
#define PCS_SSP_SOFT_RESET	(0x1 << 31)

int xhci_hcd_init(int index, struct xhci_hccr **hccr, struct xhci_hcor **hcor)
{
	if ((hccr == NULL) || (hcor == NULL))
		return -EINVAL;

	*hccr = (struct xhci_hccr *)USB3_CTRL_REG_BASE;
	*hcor = (struct xhci_hcor *)((uintptr_t)*hccr +
			HC_LENGTH(xhci_readl(&(*hccr)->cr_capbase)));

	return 0;
}

void phy_hiusb_init(int index)
{
	unsigned int reg;

	writel(0x300, CRG_REG_BASE + 0xac);
	mdelay(10);

	/* de-assert usb3_vcc_srst_req */
	reg = readl(IO_REG_USB3_CTRL);
	reg &= ~USB3_VCC_SRST_REQ2;
	writel(reg, IO_REG_USB3_CTRL);
	mdelay(100);

	reg = readl(USB3_CTRL_REG_BASE + REG_GUSB3PIPECTL0);
	reg |= PCS_SSP_SOFT_RESET;
	writel(reg, USB3_CTRL_REG_BASE + REG_GUSB3PIPECTL0);

	/* USB2 PHY chose ulpi 8bit interface */
	reg = readl(USB3_CTRL_REG_BASE + REG_GUSB2PHYCFG0);
	reg &= ~BIT_UTMI_ULPI;
	reg &= ~BIT_UTMI_8_16;
	writel(reg, USB3_CTRL_REG_BASE + REG_GUSB2PHYCFG0);
	mdelay(20);

	reg = readl(USB3_CTRL_REG_BASE + REG_GCTL);
	reg &= ~(0x3 << 12);
	reg |= (0x1 << 12); /* [13:12] 01: Host; 10: Device; 11: OTG */
	writel(reg, USB3_CTRL_REG_BASE + REG_GCTL);
	mdelay(20);

	reg = readl(USB3_CTRL_REG_BASE + REG_GUSB3PIPECTL0);
	reg &= ~PCS_SSP_SOFT_RESET;
	reg &= ~(1 << 17); /* disable suspend */
	writel(reg, USB3_CTRL_REG_BASE + REG_GUSB3PIPECTL0);
	mdelay(100);

	writel(0x23100000, USB3_CTRL_REG_BASE + GTXTHRCFG);
	writel(0x23100000, USB3_CTRL_REG_BASE + GRXTHRCFG);
	mdelay(20);
}

void xhci_hcd_stop(int index)
{
	unsigned int reg;

	reg = readl(IO_REG_USB3_CTRL);
	writel(reg | USB3_VCC_SRST_REQ2, IO_REG_USB3_CTRL);
	mdelay(500);
}
