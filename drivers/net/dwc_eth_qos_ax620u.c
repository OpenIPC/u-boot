// SPDX-License-Identifier: GPL-2.0+
/*
 * Axera AX620U glue for the DWC Ethernet QoS driver: the MAC's bus clock
 * select, reset and interface select in the AP clock block (0x49ba000),
 * the RMII reference clock direction, and an external PHY's reset GPIO.
 *
 * No AX620U source is published. Written from the cameras' U-Boot 2020.04
 * (its "axera,ax620-eqos" glue, ops at 0x400c4ee0 of the image loaded at
 * 0x40000400) and their devicetree; the DWC core is driven as mainline's.
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

#include "dwc_eth_qos.h"

#define AX620U_AP_CLK		0x049ba000
/* The MAC's bus clock: 24, 100, 200 MHz or (3) the 312 MHz the board has */
#define AX_EMAC_BUS_CLK_SEL	(AX620U_AP_CLK + 0x00)
#define  AX_EMAC_BUS_CLK_MASK	GENMASK(1, 0)
/* RGMII TX clock, 10/100/1000 */
#define  AX_RGMII_TX_SEL_SHIFT	16
#define  AX_RGMII_TX_SEL_MASK	GENMASK(17, 16)
/* RMII RX clock divider, 0xa (2.5 MHz) or 0x1 (25 MHz), and its update */
#define AX_EMAC_RMII_DIV	(AX620U_AP_CLK + 0x18)
#define  AX_RMII_RX_DIVN_MASK	GENMASK(3, 0)
#define  AX_RMII_RX_DIVN_UP	BIT(4)
#define AX_EMAC_SW_RST		(AX620U_AP_CLK + 0x20)
#define  AX_EMAC_RST		BIT(4)
#define AX_EMAC_IF_SEL		(AX620U_AP_CLK + 0x34)
#define  AX_PHY_IF_RGMII	BIT(10)
/* "rmii-phy-dir" "input": the PHY's TXC is an input, the SoC drives it */
#define AX_EMAC_RMII_DIR	0x049bc500
#define  AX_RMII_TXC_IN		BIT(3)

/* The stock "emac-bus-clock" (312 MHz): the CSR clock MDIO divides */
#define AX_EMAC_BUS_CLK		312000000
#define AX_MDIO_CR_250_300	5

static int ax620u_eqos_probe_resources(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	phy_interface_t interface = dev_read_phy_mode(dev);
	int ret;

	if (interface != PHY_INTERFACE_MODE_RMII &&
	    interface != PHY_INTERFACE_MODE_RGMII) {
		pr_err("unsupported phy-mode %d\n", interface);
		return -EINVAL;
	}

	/* Optional; requested deasserted, the PHY out of reset */
	ret = gpio_request_by_name(dev, "phy-reset-gpios", 0,
				   &eqos->phy_reset_gpio, GPIOD_IS_OUT);
	if (ret && ret != -ENOENT) {
		pr_err("phy-reset-gpios: %d\n", ret);
		return ret;
	}
	return 0;
}

static int ax620u_eqos_remove_resources(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	if (dm_gpio_is_valid(&eqos->phy_reset_gpio))
		dm_gpio_free(dev, &eqos->phy_reset_gpio);
	return 0;
}

static ulong ax620u_eqos_get_tick_clk_rate(struct udevice *dev)
{
	return dev_read_u32_default(dev, "emac-bus-clock", AX_EMAC_BUS_CLK);
}

/* The bus clock select, the interface, the RMII reference clock direction */
static int ax620u_eqos_start_clks(struct udevice *dev)
{
	ulong rate = ax620u_eqos_get_tick_clk_rate(dev);
	const char *dir;
	u32 sel;

	switch (rate) {
	case 24000000:
		sel = 0;
		break;
	case 100000000:
		sel = 1;
		break;
	case 200000000:
		sel = 2;
		break;
	default:
		sel = 3;
		break;
	}
	clrsetbits_le32(AX_EMAC_BUS_CLK_SEL, AX_EMAC_BUS_CLK_MASK, sel);

	if (dev_read_phy_mode(dev) == PHY_INTERFACE_MODE_RGMII) {
		setbits_le32(AX_EMAC_IF_SEL, AX_PHY_IF_RGMII);
		return 0;
	}
	clrbits_le32(AX_EMAC_IF_SEL, AX_PHY_IF_RGMII);

	/* The stock glue only ever sets this bit, for "input" */
	dir = dev_read_string(dev, "rmii-phy-dir");
	if (dir && !strcmp(dir, "input"))
		setbits_le32(AX_EMAC_RMII_DIR, AX_RMII_TXC_IN);
	return 0;
}

/* The MAC, 50 ms in reset; then the PHY, 100 ms in reset and 200 ms after */
static int ax620u_eqos_start_resets(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	setbits_le32(AX_EMAC_SW_RST, AX_EMAC_RST);
	mdelay(50);
	clrbits_le32(AX_EMAC_SW_RST, AX_EMAC_RST);
	mdelay(50);

	if (dm_gpio_is_valid(&eqos->phy_reset_gpio)) {
		dm_gpio_set_value(&eqos->phy_reset_gpio, 1);
		mdelay(100);
		dm_gpio_set_value(&eqos->phy_reset_gpio, 0);
		mdelay(200);
	}
	return 0;
}

/* RMII: the receive clock divider; RGMII: the transmit clock */
static int ax620u_eqos_set_tx_clk_speed(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int speed = eqos->phy->speed;

	if (dev_read_phy_mode(dev) == PHY_INTERFACE_MODE_RMII) {
		if (speed != SPEED_10 && speed != SPEED_100)
			return -EINVAL;
		clrsetbits_le32(AX_EMAC_RMII_DIV, AX_RMII_RX_DIVN_MASK,
				(speed == SPEED_10 ? 0xa : 0x1) |
				AX_RMII_RX_DIVN_UP);
	} else {
		clrsetbits_le32(AX_EMAC_BUS_CLK_SEL, AX_RGMII_TX_SEL_MASK,
				(speed == SPEED_1000 ? 2 :
				 speed == SPEED_100 ? 1 : 0) <<
				AX_RGMII_TX_SEL_SHIFT);
	}
	return 0;
}

static struct eqos_ops eqos_ax620u_ops = {
	.eqos_inval_desc = eqos_inval_desc_generic,
	.eqos_flush_desc = eqos_flush_desc_generic,
	.eqos_inval_buffer = eqos_inval_buffer_generic,
	.eqos_flush_buffer = eqos_flush_buffer_generic,
	.eqos_probe_resources = ax620u_eqos_probe_resources,
	.eqos_remove_resources = ax620u_eqos_remove_resources,
	.eqos_stop_resets = eqos_null_ops,
	.eqos_start_resets = ax620u_eqos_start_resets,
	.eqos_stop_clks = eqos_null_ops,
	.eqos_start_clks = ax620u_eqos_start_clks,
	.eqos_calibrate_pads = eqos_null_ops,
	.eqos_disable_calibration = eqos_null_ops,
	.eqos_set_tx_clk_speed = ax620u_eqos_set_tx_clk_speed,
	.eqos_get_enetaddr = eqos_null_ops,
	.eqos_get_tick_clk_rate = ax620u_eqos_get_tick_clk_rate,
};

/* As the stock glue's config: MDIO 100 us, reset 50 ms, RX queue 0 AV */
struct eqos_config __maybe_unused eqos_ax620u_config = {
	.reg_access_always_ok = false,
	.mdio_wait = 100,
	.swr_wait = 50,
	.config_mac = EQOS_MAC_RXQ_CTRL0_RXQ0EN_ENABLED_AV,
	.config_mac_mdio = AX_MDIO_CR_250_300,
	.axi_bus_width = EQOS_AXI_WIDTH_64,
	.interface = dev_read_phy_mode,
	.ops = &eqos_ax620u_ops,
};
