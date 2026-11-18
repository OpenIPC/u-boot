// SPDX-License-Identifier: GPL-2.0+
/*
 * Axera AX620E glue for the DWC Ethernet QoS driver: the MAC's clocks,
 * resets and interface select in FLASH_SYS_GLB, and the SoC's own 10/100
 * PHY (its clock, bandgap trim and reset), or an external PHY's reset GPIO.
 *
 * Written from the vendor SDK's U-Boot 2020.04 (drivers/net/axera_emac.c,
 * a copy of this driver with the same glue built in).
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <log.h>
#include <net.h>
#include <phy.h>
#include <reset.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <mach/ax620e.h>

#include "dwc_eth_qos.h"

#define AX_EMAC_CLK_MUX0	(AX_FLASH_SYS_GLB + 0x00)
#define  AX_RGMII_TX_SEL_SHIFT	4
#define AX_EMAC_CLK_EB0		(AX_FLASH_SYS_GLB + 0x04)
#define  AX_PTP_REF_EB		BIT(2)
#define  AX_RGMII_TX_EB		BIT(3)
#define  AX_RMII_PHY_EB		BIT(4)
#define  AX_EPHY_CLK_EB		BIT(13)
#define AX_EMAC_CLK_EB1		(AX_FLASH_SYS_GLB + 0x08)
#define  AX_EMAC_ACLK_EB	BIT(2)
#define AX_EMAC_SW_RST		(AX_FLASH_SYS_GLB + 0x14)
#define  AX_EMAC_RST		BIT(8)
#define  AX_EPHY_RST		BIT(9)
#define AX_EMAC_EPHY0		(AX_FLASH_SYS_GLB + 0x20)
#define  AX_EPHY_SHUTDOWN	BIT(0)
#define  AX_EPHY_CLK_SEL_SHIFT	5
#define  AX_EPHY_CLK_SEL_MASK	(7 << AX_EPHY_CLK_SEL_SHIFT)
#define  AX_EPHY_OTP_BG_SHIFT	24
#define  AX_EPHY_OTP_BG_MASK	(0xf << AX_EPHY_OTP_BG_SHIFT)
#define AX_EMAC_EMAC0		(AX_FLASH_SYS_GLB + 0x28)
/* As the vendor U-Boot and kernel drive it: bit 9 selects RGMII (else
 * RMII), bit 10 the external pads (else the internal PHY) */
#define  AX_PHY_IF_RGMII	BIT(9)
#define  AX_EXT_PAD_SEL		BIT(10)
#define AX_EMAC_SYS_GLB0	(AX_FLASH_SYS_GLB + 0x144)
#define  AX_RMII_RX_DIVN_MASK	0xf
#define  AX_RMII_RX_DIVN_UP	BIT(4)
#define  AX_RMII_RX_DIV_EN	BIT(5)

/* The MAC's CSR clock, which the MDIO divider and the 1 us tick count */
#define AX_EMAC_CSR_CLK		312000000
#define AX_MDIO_CR_300_500	6

/* The SoC's own PHY, or "axera,out_rmii": an external one on RMII */
static bool ax_eqos_internal_phy(struct udevice *dev)
{
	return dev_read_phy_mode(dev) == PHY_INTERFACE_MODE_RMII &&
	       !dev_read_bool(dev, "axera,out_rmii");
}

static int ax_eqos_probe_resources(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	phy_interface_t interface = eqos->config->interface(dev);
	int ret;

	/* The interface select and TX clock are known for these two only */
	if (interface != PHY_INTERFACE_MODE_RMII &&
	    interface != PHY_INTERFACE_MODE_RGMII) {
		pr_err("unsupported phy-mode %d\n", interface);
		return -EINVAL;
	}

	/* Optional; but one the node names must be had */
	ret = gpio_request_by_name(dev, "phy-reset-gpios", 0,
				   &eqos->phy_reset_gpio,
				   GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE);
	if (ret && ret != -ENOENT) {
		pr_err("phy-reset-gpios: %d\n", ret);
		return ret;
	}
	return 0;
}

static int ax_eqos_remove_resources(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	if (dm_gpio_is_valid(&eqos->phy_reset_gpio))
		dm_gpio_free(dev, &eqos->phy_reset_gpio);
	return 0;
}

/* Bus clocks, the RMII/RGMII/PHY/PTP clocks, and the interface select */
static int ax_eqos_start_clks(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	phy_interface_t interface = eqos->config->interface(dev);
	u32 sel;

	setbits_le32(AX_EMAC_CLK_EB1, AX_EMAC_ACLK_EB);
	setbits_le32(AX_EMAC_CLK_EB0, AX_RMII_PHY_EB | AX_RGMII_TX_EB |
		     AX_EPHY_CLK_EB | AX_PTP_REF_EB);
	setbits_le32(AX_EMAC_SYS_GLB0, AX_RMII_RX_DIV_EN);

	if (interface == PHY_INTERFACE_MODE_RMII)
		sel = ax_eqos_internal_phy(dev) ? 0 : AX_EXT_PAD_SEL;
	else
		sel = AX_PHY_IF_RGMII | AX_EXT_PAD_SEL;
	clrsetbits_le32(AX_EMAC_EMAC0, AX_PHY_IF_RGMII | AX_EXT_PAD_SEL, sel);
	return 0;
}

/*
 * The internal PHY: its clock select stepped 4 then 6, the bandgap trim
 * from the eFuses (the first stage's misc_info; 0xc and 0xd from an
 * untrimmed part read as 0), then out of shutdown and reset.
 */
static void ax_eqos_ephy_reset(void)
{
	struct ax_misc_info *mi = (void *)AX_MISC_INFO_ADDR;
	u32 bgs = mi->bgs & 0xf;

	clrsetbits_le32(AX_EMAC_EPHY0, AX_EPHY_CLK_SEL_MASK,
			4 << AX_EPHY_CLK_SEL_SHIFT);
	mdelay(10);
	clrsetbits_le32(AX_EMAC_EPHY0, AX_EPHY_CLK_SEL_MASK,
			6 << AX_EPHY_CLK_SEL_SHIFT);
	mdelay(10);

	if (mi->trim != 0x7 && (bgs == 0xc || bgs == 0xd))
		bgs = 0;
	clrsetbits_le32(AX_EMAC_EPHY0, AX_EPHY_OTP_BG_MASK,
			bgs << AX_EPHY_OTP_BG_SHIFT);

	setbits_le32(AX_EMAC_SW_RST, AX_EPHY_RST);
	clrbits_le32(AX_EMAC_EPHY0, AX_EPHY_SHUTDOWN);
	mdelay(15);
	clrbits_le32(AX_EMAC_SW_RST, AX_EPHY_RST);
	udelay(15);
}

static int ax_eqos_start_resets(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	setbits_le32(AX_EMAC_SW_RST, AX_EMAC_RST);
	mdelay(50);
	clrbits_le32(AX_EMAC_SW_RST, AX_EMAC_RST);
	mdelay(50);

	if (ax_eqos_internal_phy(dev)) {
		ax_eqos_ephy_reset();
	} else if (dm_gpio_is_valid(&eqos->phy_reset_gpio)) {
		dm_gpio_set_value(&eqos->phy_reset_gpio, 1);
		mdelay(100);
		dm_gpio_set_value(&eqos->phy_reset_gpio, 0);
		mdelay(200);
	}
	return 0;
}

/* RMII: the receive clock divider, 25 MHz or 2.5 MHz; RGMII: the TX clock */
static int ax_eqos_set_tx_clk_speed(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int speed = eqos->phy->speed;

	if (eqos->config->interface(dev) == PHY_INTERFACE_MODE_RMII) {
		if (speed != SPEED_10 && speed != SPEED_100)
			return -EINVAL;
		clrsetbits_le32(AX_EMAC_SYS_GLB0, AX_RMII_RX_DIVN_MASK,
				(speed == SPEED_10 ? 0xa : 0x1) |
				AX_RMII_RX_DIVN_UP);
	} else {
		clrsetbits_le32(AX_EMAC_CLK_MUX0, 3 << AX_RGMII_TX_SEL_SHIFT,
				(speed == SPEED_1000 ? 2 :
				 speed == SPEED_100 ? 1 : 0) <<
				AX_RGMII_TX_SEL_SHIFT);
	}
	return 0;
}

static ulong ax_eqos_get_tick_clk_rate(struct udevice *dev)
{
	return dev_read_u32_default(dev, "axera,csr-clock", AX_EMAC_CSR_CLK);
}

static struct eqos_ops eqos_axera_ops = {
	.eqos_inval_desc = eqos_inval_desc_generic,
	.eqos_flush_desc = eqos_flush_desc_generic,
	.eqos_inval_buffer = eqos_inval_buffer_generic,
	.eqos_flush_buffer = eqos_flush_buffer_generic,
	.eqos_probe_resources = ax_eqos_probe_resources,
	.eqos_remove_resources = ax_eqos_remove_resources,
	.eqos_stop_resets = eqos_null_ops,
	.eqos_start_resets = ax_eqos_start_resets,
	.eqos_stop_clks = eqos_null_ops,
	.eqos_start_clks = ax_eqos_start_clks,
	.eqos_calibrate_pads = eqos_null_ops,
	.eqos_disable_calibration = eqos_null_ops,
	.eqos_set_tx_clk_speed = ax_eqos_set_tx_clk_speed,
	.eqos_get_enetaddr = eqos_null_ops,
	.eqos_get_tick_clk_rate = ax_eqos_get_tick_clk_rate,
};

struct eqos_config __maybe_unused eqos_axera_config = {
	.reg_access_always_ok = false,
	.mdio_wait = 100,
	.swr_wait = 50,
	.config_mac = EQOS_MAC_RXQ_CTRL0_RXQ0EN_ENABLED_AV,
	.config_mac_mdio = AX_MDIO_CR_300_500,
	.axi_bus_width = EQOS_AXI_WIDTH_64,
	.interface = dev_read_phy_mode,
	.ops = &eqos_axera_ops,
};
