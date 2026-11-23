// SPDX-License-Identifier: GPL-2.0+
/*
 * Rockchip glue for the DWC Ethernet QoS driver, for the camera SoCs whose
 * MAC is a dwmac-4.20a:
 *
 *   RV1106/RV1103, RV1103B/RV1106B: RMII to the SoC's own 10/100 PHY. The
 *     GRF holds the RMII mode bit, the RMII clock divider for 10/100, and
 *     the PHY's enable, 24 MHz clock and MDIO address; the two dies differ
 *     only in where those registers are, and the RMII mode bit.
 *   RV1126/RV1109: an external PHY. The GRF selects the interface; the CRU
 *     clocks it (the device tree's assigned clocks pick the 50 MHz source
 *     and the RMII or RGMII clock mode, clk_mac_speed sets the speed).
 *
 * Any of them may reset its PHY by a GPIO, as Linux's stmmac binding has it
 * (snps,reset-gpio, snps,reset-active-low, snps,reset-delays-us).
 *
 * Written from the vendor U-Boot (rockchip-linux/u-boot next-dev,
 * drivers/net/gmac_rockchip.c: rv1106_set_to_rmii, rv1103b_set_to_rmii,
 * rv1106_set_rmii_speed, rv1106_gmac_integrated_phy_powerup,
 * rv1126_set_to_rmii, rv1126_set_rgmii_speed), in the shape of
 * openipc-axera's dwc_eth_qos_axera.c.
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <log.h>
#include <net.h>
#include <phy.h>
#include <reset.h>
#include <syscon.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/hardware.h>
#include <linux/bitops.h>
#include <linux/delay.h>

#include "dwc_eth_qos.h"

/* GMAC_CLK_CON: the RMII clock divider for 10/100 (RV1106, RV1103B) */
#define RV1106_GMAC_CLK_RMII_DIV_MASK	GENMASK(3, 2)
#define RV1106_GMAC_CLK_RMII_DIV2	BIT(2)		/* 25 MHz, 100M */
#define RV1106_GMAC_CLK_RMII_DIV20	0		/* 2.5 MHz, 10M */

/* MACPHY_CON0/1: the integrated PHY (RV1106, RV1103B) */
#define RV1106_MACPHY_SHUTDOWN		BIT(1)
#define RV1106_MACPHY_XMII_SEL_MASK	GENMASK(6, 5)
#define RV1106_MACPHY_XMII_SEL_RMII	BIT(6)
#define RV1106_MACPHY_24M_CLK_SEL_MASK	GENMASK(9, 7)
#define RV1106_MACPHY_24M_CLK_SEL_24M	(BIT(8) | BIT(9))
#define RV1106_MACPHY_PHY_ID_MASK	GENMASK(14, 10)
#define RV1106_MACPHY_PHY_ID_2		BIT(11)		/* MDIO address 2 */
#define RV1106_MACPHY_BGS_MASK		GENMASK(3, 0)

/* MAC_CON0: the PHY interface (RV1126) */
#define RV1126_GMAC_PHY_INTF_SEL_MASK	GENMASK(6, 4)
#define RV1126_GMAC_PHY_INTF_SEL_RGMII	BIT(4)
#define RV1126_GMAC_PHY_INTF_SEL_RMII	BIT(6)

/*
 * The MAC's CSR clock range for MDC, as the vendor gives it (100-150 MHz),
 * and the rate the 1 us tick counts
 */
#define RK_EQOS_CSR_RATE		100000000
#define RK_EQOS_MDIO_CR_100_150		1

/* What differs between the SoCs; GRF offsets in the device tree's GRF */
struct rk_eqos_soc {
	const char *compatible;
	/* the integrated PHY's RMII glue (0: none) */
	u32 gmac_clk_con;
	u32 macphy_con0;
	u32 macphy_con1;
	u32 rmii_mode;
	/* the interface select of a MAC with an external PHY (0: none) */
	u32 intf_con;
};

static const struct rk_eqos_soc rk_eqos_socs[] = {
	/* VO GRF at 0xff060000 */
	{ "rockchip,rv1106-gmac", 0x60004, 0x60028, 0x6002c, BIT(0), 0 },
	/* SYS GRF at 0x20150000 */
	{ "rockchip,rv1103b-gmac", 0x500a4, 0x500b0, 0x500b4, BIT(2), 0 },
	/* GRF at 0xfe000000 */
	{ "rockchip,rv1126-gmac", 0, 0, 0, 0, 0x70 },
};

/* The glue's own state: these SoCs have one MAC */
static struct {
	const struct rk_eqos_soc *soc;
	struct gpio_desc phy_reset;
	u32 phy_reset_us[3];
} rk_eqos;

static const struct rk_eqos_soc *rk_eqos_soc(struct udevice *dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(rk_eqos_socs); i++)
		if (device_is_compatible(dev, rk_eqos_socs[i].compatible))
			return &rk_eqos_socs[i];
	return NULL;
}

static void rk_eqos_grf_clrset(u32 reg, u32 clr, u32 set)
{
	void *grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);

	rk_clrsetreg(grf + reg, clr, set);
}

/* The SoC's own PHY: its node says "phy-is-integrated" */
static bool rk_eqos_integrated_phy(struct eqos_priv *eqos)
{
	return ofnode_valid(eqos->phy_of_node) &&
	       ofnode_read_bool(eqos->phy_of_node, "phy-is-integrated");
}

static int rk_eqos_probe_resources(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	phy_interface_t interface = eqos->config->interface(dev);
	struct ofnode_phandle_args args;
	int ret;

	rk_eqos.soc = rk_eqos_soc(dev);
	if (!rk_eqos.soc)
		return -ENODEV;
	if (interface != PHY_INTERFACE_MODE_RMII &&
	    !(rk_eqos.soc->intf_con && interface == PHY_INTERFACE_MODE_RGMII)) {
		pr_err("unsupported phy-mode %d\n", interface);
		return -EINVAL;
	}

	ret = reset_get_by_name(dev, "stmmaceth", &eqos->reset_ctl);
	if (ret) {
		pr_err("reset stmmaceth: %d\n", ret);
		return ret;
	}

	if (!dev_read_phandle_with_args(dev, "phy-handle", NULL, 0, 0, &args))
		eqos->phy_of_node = args.node;

	/* An external PHY's reset GPIO, Linux's stmmac binding */
	ret = gpio_request_by_name(dev, "snps,reset-gpio", 0,
				   &rk_eqos.phy_reset, GPIOD_IS_OUT);
	if (ret && ret != -ENOENT) {
		pr_err("snps,reset-gpio: %d\n", ret);
		return ret;
	}
	if (dm_gpio_is_valid(&rk_eqos.phy_reset)) {
		if (dev_read_bool(dev, "snps,reset-active-low"))
			rk_eqos.phy_reset.flags |= GPIOD_ACTIVE_LOW;
		if (dev_read_u32_array(dev, "snps,reset-delays-us",
				       rk_eqos.phy_reset_us, 3))
			memset(rk_eqos.phy_reset_us, 0,
			       sizeof(rk_eqos.phy_reset_us));
	}
	return 0;
}

static int rk_eqos_remove_resources(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	if (dm_gpio_is_valid(&rk_eqos.phy_reset))
		dm_gpio_free(dev, &rk_eqos.phy_reset);
	reset_free(&eqos->reset_ctl);
	return 0;
}

static int rk_eqos_start_clks(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	const struct rk_eqos_soc *soc = rk_eqos.soc;

	if (soc->intf_con)
		rk_eqos_grf_clrset(soc->intf_con, RV1126_GMAC_PHY_INTF_SEL_MASK,
				   eqos->config->interface(dev) ==
				   PHY_INTERFACE_MODE_RMII ?
				   RV1126_GMAC_PHY_INTF_SEL_RMII :
				   RV1126_GMAC_PHY_INTF_SEL_RGMII);
	if (soc->rmii_mode)
		rk_eqos_grf_clrset(soc->gmac_clk_con, soc->rmii_mode,
				   soc->rmii_mode);
	return 0;
}

/*
 * The integrated PHY: held in reset while it is enabled on RMII, on the
 * 24 MHz clock, at MDIO address 2. The bandgap trim (MACPHY_CON1) stays 0:
 * the kernel trims it from the OTP when it powers the PHY up again.
 */
static void rk_eqos_macphy_powerup(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	const struct rk_eqos_soc *soc = rk_eqos.soc;
	struct reset_ctl phy_reset;
	bool has_reset;

	has_reset = !reset_get_by_index_nodev(eqos->phy_of_node, 0, &phy_reset);
	if (has_reset)
		reset_assert(&phy_reset);
	udelay(20);
	rk_eqos_grf_clrset(soc->macphy_con0,
			   RV1106_MACPHY_SHUTDOWN | RV1106_MACPHY_XMII_SEL_MASK |
			   RV1106_MACPHY_24M_CLK_SEL_MASK |
			   RV1106_MACPHY_PHY_ID_MASK,
			   RV1106_MACPHY_XMII_SEL_RMII |
			   RV1106_MACPHY_24M_CLK_SEL_24M | RV1106_MACPHY_PHY_ID_2);
	rk_eqos_grf_clrset(soc->macphy_con1, RV1106_MACPHY_BGS_MASK, 0);
	udelay(20);
	if (has_reset) {
		reset_deassert(&phy_reset);
		reset_free(&phy_reset);
	}
	mdelay(30);
}

/* An external PHY's reset pulse, with the binding's delays around it */
static void rk_eqos_phy_reset(void)
{
	dm_gpio_set_value(&rk_eqos.phy_reset, 0);
	udelay(rk_eqos.phy_reset_us[0]);
	dm_gpio_set_value(&rk_eqos.phy_reset, 1);
	udelay(rk_eqos.phy_reset_us[1]);
	dm_gpio_set_value(&rk_eqos.phy_reset, 0);
	udelay(rk_eqos.phy_reset_us[2]);
}

static int rk_eqos_start_resets(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	reset_assert(&eqos->reset_ctl);
	udelay(2);
	reset_deassert(&eqos->reset_ctl);

	if (rk_eqos_integrated_phy(eqos))
		rk_eqos_macphy_powerup(dev);
	else if (dm_gpio_is_valid(&rk_eqos.phy_reset))
		rk_eqos_phy_reset();
	return 0;
}

static int rk_eqos_stop_resets(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	return reset_assert(&eqos->reset_ctl);
}

static int rk_eqos_set_tx_clk_speed(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	const struct rk_eqos_soc *soc = rk_eqos.soc;
	struct clk clk;
	ulong rate;
	u32 div = 0;
	int ret;

	switch (eqos->phy->speed) {
	case SPEED_10:
		div = RV1106_GMAC_CLK_RMII_DIV20;
		rate = 2500000;
		break;
	case SPEED_100:
		div = RV1106_GMAC_CLK_RMII_DIV2;
		rate = 25000000;
		break;
	case SPEED_1000:
		if (soc->intf_con) {
			rate = 125000000;
			break;
		}
		fallthrough;
	default:
		pr_err("unsupported speed %d\n", eqos->phy->speed);
		return -EINVAL;
	}

	if (soc->intf_con) {
		/* The CRU divides the clock mode the device tree chose */
		ret = clk_get_by_name(dev, "clk_mac_speed", &clk);
		if (ret)
			return ret;
		rate = clk_set_rate(&clk, rate);
		return IS_ERR_VALUE(rate) ? (int)rate : 0;
	}
	/* On the RV1103B the 100M divider is its RMII mode bit too, as the
	 * vendor U-Boot has it */
	rk_eqos_grf_clrset(soc->gmac_clk_con, RV1106_GMAC_CLK_RMII_DIV_MASK,
			   div);
	return 0;
}

static ulong rk_eqos_get_tick_clk_rate(struct udevice *dev)
{
	return RK_EQOS_CSR_RATE;
}

static struct eqos_ops eqos_rockchip_ops = {
	.eqos_inval_desc = eqos_inval_desc_generic,
	.eqos_flush_desc = eqos_flush_desc_generic,
	.eqos_inval_buffer = eqos_inval_buffer_generic,
	.eqos_flush_buffer = eqos_flush_buffer_generic,
	.eqos_probe_resources = rk_eqos_probe_resources,
	.eqos_remove_resources = rk_eqos_remove_resources,
	.eqos_stop_resets = rk_eqos_stop_resets,
	.eqos_start_resets = rk_eqos_start_resets,
	.eqos_stop_clks = eqos_null_ops,
	.eqos_start_clks = rk_eqos_start_clks,
	.eqos_calibrate_pads = eqos_null_ops,
	.eqos_disable_calibration = eqos_null_ops,
	.eqos_set_tx_clk_speed = rk_eqos_set_tx_clk_speed,
	.eqos_get_enetaddr = eqos_null_ops,
	.eqos_get_tick_clk_rate = rk_eqos_get_tick_clk_rate,
};

/* RX queue 0 enabled for DCB, as Linux's stmmac sets it by default */
struct eqos_config __maybe_unused eqos_rockchip_config = {
	.reg_access_always_ok = false,
	.mdio_wait = 10000,
	.swr_wait = 200,
	.config_mac = EQOS_MAC_RXQ_CTRL0_RXQ0EN_ENABLED_DCB,
	.config_mac_mdio = RK_EQOS_MDIO_CR_100_150,
	.axi_bus_width = EQOS_AXI_WIDTH_64,
	.interface = dev_read_phy_mode,
	.ops = &eqos_rockchip_ops,
};
