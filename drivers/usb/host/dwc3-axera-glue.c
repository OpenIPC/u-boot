// SPDX-License-Identifier: GPL-2.0+
/*
 * Axera AX620E glue for the DWC3 USB 2.0 controller ("axera,ax620e-dwc3",
 * the vendor's node around "snps,dwc3"): its clocks, resets and host mode
 * in FLASH_SYS_GLB, before xhci-dwc3 probes the controller below it.
 *
 * Written from the vendor SDK's U-Boot 2020.04 (drivers/usb/dwc3/
 * dwc3-axera.c). That one also drops the flash bus clock (CLK_MUX0
 * [8:6]) to 100 MHz; here it stays as the first stage set it, as the
 * Ethernet MAC's AXI clock hangs off it too.
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <mach/ax620e.h>

#define AX_USB_CLK_EB0		(AX_FLASH_SYS_GLB + 0x04)
#define  AX_USB2_REF_EB		BIT(12)
#define  AX_USB2_REF_ALT_EB	BIT(14)
#define AX_USB_CLK_EB1		(AX_FLASH_SYS_GLB + 0x08)
#define  AX_USB2_BUS_EB		BIT(5)
#define AX_USB_SW_RST		(AX_FLASH_SYS_GLB + 0x14)
#define  AX_USB2_PHY_RST	BIT(24)
#define  AX_USB2_VCC_RST	BIT(25)
#define AX_USB2_CTRL		(AX_FLASH_SYS_GLB + 0x40)
/* Set: device mode (VBUS valid forced); clear: host */
#define  AX_USB2_VBUSVALID	BIT(6)

static int ax620e_dwc3_probe(struct udevice *dev)
{
	setbits_le32(AX_USB_CLK_EB0, AX_USB2_REF_EB | AX_USB2_REF_ALT_EB);
	setbits_le32(AX_USB_CLK_EB1, AX_USB2_BUS_EB);

	setbits_le32(AX_USB_SW_RST, AX_USB2_PHY_RST | AX_USB2_VCC_RST);
	mdelay(1);
	clrbits_le32(AX_USB_SW_RST, AX_USB2_PHY_RST | AX_USB2_VCC_RST);
	mdelay(1);

	clrbits_le32(AX_USB2_CTRL, AX_USB2_VBUSVALID);

	return 0;
}

static const struct udevice_id ax620e_dwc3_ids[] = {
	{ .compatible = "axera,ax620e-dwc3" },
	{ }
};

/* A simple bus: its post_bind binds the "snps,dwc3" child */
U_BOOT_DRIVER(ax620e_dwc3) = {
	.name = "ax620e-dwc3",
	.id = UCLASS_SIMPLE_BUS,
	.of_match = ax620e_dwc3_ids,
	.probe = ax620e_dwc3_probe,
};
