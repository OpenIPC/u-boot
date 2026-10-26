// SPDX-License-Identifier: GPL-2.0+
/*
 * Fullhan camera boards, after the vendor U-Boots 2010.06 (board/Fullhan/
 * fh*_devboard, drivers/net/fh_gmac*, drivers/mmc/fh_mmc*).
 *
 * The pads U-Boot itself uses are set here; the kernels apply their own
 * pin and clock set-up from their board files.
 */

#include <common.h>
#include <dwmmc.h>
#include <init.h>
#include <miiphy.h>
#include <mmc.h>
#include <net.h>
#include <netdev.h>
#include <openipc.h>
#include <phy.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/fullhan.h>
#include <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

struct fh_pad {
	u8 pad;
	u32 val;
	u32 mask;
};

/*
 * Pad words: function in 27:24, pulls 20 and 16, input enable 12, Schmitt
 * trigger 8, drive 6:4, output enable 28. The values are the words the
 * vendor pinctrl leaves for UART0, the flash and SD0 (and RMII, for an
 * external PHY), and only the fields it writes are written: the V200 one
 * sets output enable, the older ones keep it as it is, and all keep slew
 * (bit 0) and 1.8 V (bit 7).
 */
#if defined(CONFIG_FULLHAN_V200) || defined(CONFIG_FULLHAN_865X)
#define FH_PAD_FIELDS	0x1f111170
#else
#define FH_PAD_FIELDS	0x0f111170
#endif
#define PAD(n, v)	{ n, v, FH_PAD_FIELDS }
#define PAD_FUNC(n, f)	{ n, (f) << 24 | BIT(12), 0x0f000000 | BIT(12) }

static const struct fh_pad fh_pads[] = {
#if defined(CONFIG_FULLHAN_V200) || defined(CONFIG_FULLHAN_865X)
	PAD(3, 0x10001100), PAD(4, 0x10001100),		/* UART0 TX, RX */
	PAD(42, 0x10001140),				/* SSI0 CLK */
	PAD(43, 0x11001120),				/* GPIO6: flash CS */
	PAD(44, 0x10001140), PAD(45, 0x10001140),	/* SSI0 TXD, RXD */
	PAD(46, 0x10001140), PAD(47, 0x10001140),	/* SSI0 D2, D3 */
	PAD(52, 0x10001120),				/* SD0 CD */
	PAD(53, 0x10001130),				/* SD0 CLK */
	PAD(54, 0x10001120), PAD(55, 0x10001120),	/* SD0 CMD, D0 */
	PAD(56, 0x10001120), PAD(57, 0x10001120),	/* SD0 D1, D2 */
	PAD(58, 0x10001120),				/* SD0 D3 */
#elif defined(CONFIG_FULLHAN_V100)
	PAD(50, 0x00101130), PAD(51, 0x00101130),	/* UART0 RX, TX */
	PAD(56, 0x00001130), PAD(57, 0x00101130),	/* SSI0 CLK, TXD */
	PAD(58, 0x01101130),				/* GPIO54: flash CS */
	PAD(60, 0x00101130),				/* SSI0 RXD */
	PAD(15, 0x00101130), PAD(16, 0x00001130),	/* RMII CLK, REF_CLK */
	PAD(17, 0x00101130), PAD(18, 0x00101130),	/* MDC, MDIO */
	PAD(22, 0x00101130), PAD(23, 0x00101130),	/* RXD0, RXD1 */
	PAD(26, 0x00101130),				/* RXDV */
	PAD(28, 0x00101130), PAD(29, 0x00101130),	/* TXD0, TXD1 */
	PAD(32, 0x00101130),				/* TXEN */
	PAD(46, 0x00001130),				/* GPIO11: PHY reset */
	PAD(61, 0x00101130), PAD(63, 0x00001130),	/* SD0 CD, CLK */
	PAD(64, 0x00101130), PAD(65, 0x00101130),	/* SD0 CMD, D0 */
	PAD(66, 0x00101130), PAD(67, 0x00101130),	/* SD0 D1, D2 */
	PAD(68, 0x00101130),				/* SD0 D3 */
#elif defined(CONFIG_FULLHAN_8626)
	PAD(50, 0x00011100), PAD(51, 0x00011100),	/* UART0 RX, TX */
	PAD(56, 0x00001140), PAD(57, 0x00001100),	/* SSI0 CLK, TXD */
	PAD(58, 0x01011100),				/* GPIO54: flash CS */
	PAD(60, 0x00011100),				/* SSI0 RXD */
	PAD(15, 0x00011140), PAD(16, 0x00001140),	/* RMII CLK, REF_CLK */
	PAD(17, 0x00011140), PAD(18, 0x00001100),	/* MDC, MDIO */
	PAD(22, 0x00011100), PAD(23, 0x00011100),	/* RXD0, RXD1 */
	PAD(26, 0x00011100),				/* RXDV */
	PAD(28, 0x00001100), PAD(29, 0x00001100),	/* TXD0, TXD1 */
	PAD(32, 0x00001100),				/* TXEN */
	PAD(46, 0x01001100),				/* GPIO11: PHY reset */
	PAD(61, 0x00011100), PAD(63, 0x00001140),	/* SD0 CD, CLK */
	PAD(64, 0x00011100), PAD(65, 0x00011100),	/* SD0 CMD, D0 */
	PAD(66, 0x00011140), PAD(67, 0x00011100),	/* SD0 D1, D2 */
	PAD(68, 0x00011100),				/* SD0 D3 */
#elif defined(CONFIG_FULLHAN_8632)
	/* Only the functions: the pulls stay as they are */
	PAD_FUNC(58, 1),				/* GPIO54: flash CS */
	PAD_FUNC(37, 1),				/* GPIO2: PHY reset */
	PAD_FUNC(40, 1),				/* GPIO5: SD0 power */
	{ 59, 0x03001000, 0xffffffff },			/* AC_MCLK: RMII clock */
	PAD_FUNC(15, 0), PAD_FUNC(16, 0), PAD_FUNC(17, 0),	/* RMII */
	PAD_FUNC(18, 0), PAD_FUNC(21, 0), PAD_FUNC(22, 0),
	PAD_FUNC(23, 0), PAD_FUNC(26, 0), PAD_FUNC(27, 0),
	PAD_FUNC(28, 0), PAD_FUNC(29, 0), PAD_FUNC(32, 0),
	PAD_FUNC(61, 0), PAD_FUNC(63, 0), PAD_FUNC(64, 0),	/* SD0 */
	PAD_FUNC(65, 0),
#endif
};

static void fh_pinmux_init(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(fh_pads); i++)
		fh_pmu_update(FH_PMU_PAD(fh_pads[i].pad), fh_pads[i].mask,
			      fh_pads[i].val);
}

int board_init(void)
{
	gd->bd->bi_boot_params = FH_DDR_BASE + 0x100;
	fh_pinmux_init();
	return 0;
}

int misc_init_r(void)
{
	return openipc_helper();
}

#ifdef CONFIG_ETH_DESIGNWARE
#define GMAC_GMII_ADDR		(FH_GMAC_BASE + 0x10)
#define GMAC_GMII_DATA		(FH_GMAC_BASE + 0x14)
#define GMII_BUSY		BIT(0)
#define GMII_WRITE		BIT(1)
#define GMII_CSR_100_150M	(1 << 2)	/* the vendor drivers' range */

/*
 * MDIO before the network device exists: the PHY set-up the vendor drivers
 * do before their generic PHY code runs
 */
static int fh_mdio(int addr, int reg, int write, u16 val)
{
	int t;

	if (write)
		writel(val, GMAC_GMII_DATA);
	writel(addr << 11 | reg << 6 | GMII_CSR_100_150M |
	       (write ? GMII_WRITE : 0) | GMII_BUSY, GMAC_GMII_ADDR);
	for (t = 0; t < 10000; t++) {
		if (!(readl(GMAC_GMII_ADDR) & GMII_BUSY))
			return write ? 0 : readl(GMAC_GMII_DATA) & 0xffff;
		udelay(1);
	}
	return -ETIMEDOUT;
}

#define fh_mdio_read(a, r)	fh_mdio(a, r, 0, 0)
#define fh_mdio_write(a, r, v)	fh_mdio(a, r, 1, v)

/* The first address answering with a PHY ID, or -1 */
static int fh_phy_find(u32 *id)
{
	int addr;

	for (addr = 0; addr < 32; addr++) {
		int hi = fh_mdio_read(addr, MII_PHYSID1);
		int lo = fh_mdio_read(addr, MII_PHYSID2);

		if (hi < 0 || lo < 0 || (hi == 0xffff && lo == 0xffff) ||
		    (!hi && !lo))
			continue;
		*id = hi << 16 | lo;
		return addr;
	}
	return -1;
}

/* SYS_CTRL bit 24 is the MAC's 10/100 clock, beside the MAC's own FES */
void designware_board_adjust_link(struct phy_device *phydev)
{
	if (phydev->speed == SPEED_100)
		setbits_le32(FH_PMU_SYS_CTRL, FH_SYS_CTRL_MAC_100M);
	else
		clrbits_le32(FH_PMU_SYS_CTRL, FH_SYS_CTRL_MAC_100M);
}

#define PHY_ID_IP101G		0x02430c54
#define PHY_ID_RTL8201		0x001cc816
#define FH_SYS_CTRL_RMII_CLK	BIT(28)
#define FH_SYS_CTRL_EXT_CLK	BIT(26)

/* The external PHYs' own set-up, as the vendor GMAC_PhyInit() does it */
static void fh_ext_phy_init(int addr, u32 id)
{
	if ((id & 0xfffffff0) == (PHY_ID_IP101G & 0xfffffff0)) {
		fh_mdio_write(addr, 20, 16);
		fh_mdio_write(addr, 16, 0x1006);
#ifndef CONFIG_FULLHAN_8632
		fh_mdio_write(addr, 26, 0xc5ed);	/* RXD drive */
#endif
		fh_mdio_write(addr, 20, 16);
	} else if ((id & 0xfffffff0) == (PHY_ID_RTL8201 & 0xfffffff0)) {
		fh_mdio_write(addr, 31, 7);
		fh_mdio_write(addr, 16, 0x1ffa);	/* RMII, clock in */
		fh_mdio_write(addr, 31, 0);
	}
}

#if defined(CONFIG_FULLHAN_V200) || defined(CONFIG_FULLHAN_865X)
/*
 * V200 and FH865x boards have either an RMII PHY on the MAC pads (an
 * FH8852V200 camera with a JL1101 is one) or use the SoC's own 10/100 PHY
 * (ID 0x00441400). As the vendor's auto_find_phy() does, try the
 * external one first -- the pads in their RMII functions, the internal PHY
 * off, the PHY reset with RXDV strapped low -- and fall back to the
 * internal one when nothing answers on MDIO. Unlike the vendor U-Boots,
 * which take only the IDs on their list, any PHY that answers is taken.
 */
#define FH_EPHY_REG0		(FH_PMU_BASE + 0x2108)
#define FH_PMU_SWRSTN_NSR	(FH_PMU_BASE + 0x078)
#define FH_PMU_EPHY_PARAM	(FH_PMU_BASE + 0x340)	/* trim, from eFuse */

#define FH_PHY_RESET_GPIO	29
#define FH_PHY_RESET_PAD	64
#define FH_RXDV_GPIO		54
#define FH_RXDV_PAD		27

/* RMII_CLK, REF_CLK (the 50 MHz out), TXD0/1, TXEN, RXD0/1, RXDV, MDC, MDIO */
#define FH_RMII_PAD0		20
#define FH_RMII_PADS		10

static const u32 fh_rmii_pad_val[FH_RMII_PADS] = {
	0x10001120, 0x10001120, 0x10001120, 0x10001120, 0x10001120,
	0x10001120, 0x10001120, 0x10001120, 0x10001100, 0x10001100,
};

static const u16 fh_ephy_regs[][2] = {
	{ 0x1f, 0x100 }, { 0x12, 0x4824 }, { 0x1f, 0x200 }, { 0x18, 0 },
	{ 0x1f, 0x600 }, { 0x10, 0x555b }, { 0x14, 0x708f }, { 0x17, 0x575 },
	{ 0x18, 0 }, { 0x19, 0x408 }, { 0x1c, 0x8880 }, { 0x1f, 0x800 },
	{ 0x1d, 0x844 }, { 0x1f, 0x600 }, { 0x15, 0x1320 },
};

/* The MAC out of reset, at 100M until the link says otherwise */
static void fh_gmac_reset(void)
{
	setbits_le32(FH_PMU_SYS_CTRL, FH_SYS_CTRL_MAC_100M | FH_SYS_CTRL_RMII_CLK);
	fh_pmu_reset(FH_PMU_SWRST_AHB, FH_SWRST_GMAC);
}

static int fh_ext_phy_setup(void)
{
	u32 saved[FH_RMII_PADS + 1], id;
	int i, addr;

	for (i = 0; i < FH_RMII_PADS; i++) {
		saved[i] = readl(FH_PMU_PAD(FH_RMII_PAD0 + i));
		writel(fh_rmii_pad_val[i], FH_PMU_PAD(FH_RMII_PAD0 + i));
	}
	saved[FH_RMII_PADS] = readl(FH_PMU_PAD(FH_PHY_RESET_PAD));
	writel(0x10001100, FH_PMU_PAD(FH_PHY_RESET_PAD));	/* GPIO29 */

	/* fh_in_out_phy_sel_init(EXTERNAL_PHY): the internal PHY off */
	clrbits_le32(FH_EPHY_REG0, BIT(24));
	setbits_le32(FH_PMU_CLK_GATE1, BIT(0));
	fh_pmu_update(FH_PMU_CLK_DIV6, 0xf000000, 1 << 24);	/* 50 MHz ref */
	clrbits_le32(FH_PMU_CLK_GATE, BIT(25) | BIT(28));
	setbits_le32(FH_PMU_SYS_CTRL, FH_SYS_CTRL_EXT_CLK);

	/* fh_gmac_phy_reset(): RXDV is a strap, low through the reset */
	writel(0x01001030, FH_PMU_PAD(FH_RXDV_PAD));
	fh_gpio_output(FH_RXDV_GPIO, 0);
	fh_gpio_output(FH_PHY_RESET_GPIO, 1);
	mdelay(150);
	fh_gpio_output(FH_PHY_RESET_GPIO, 0);
	mdelay(10);
	fh_gpio_output(FH_PHY_RESET_GPIO, 1);
	writel(fh_rmii_pad_val[FH_RXDV_PAD - FH_RMII_PAD0],
	       FH_PMU_PAD(FH_RXDV_PAD));
	mdelay(10);

	fh_gmac_reset();
	addr = fh_phy_find(&id);
	if (addr >= 0) {
		printf("external PHY %08x at %d, ", id, addr);
		fh_ext_phy_init(addr, id);
		return 0;
	}

	/* Nothing there: the pads back as the board had them */
	for (i = 0; i < FH_RMII_PADS; i++)
		writel(saved[i], FH_PMU_PAD(FH_RMII_PAD0 + i));
	writel(saved[FH_RMII_PADS], FH_PMU_PAD(FH_PHY_RESET_PAD));
	return -ENODEV;
}

/*
 * The internal PHY: fh_in_out_phy_sel_init(INTERNAL_PHY),
 * fh_inside_phy_reset() and the register writes of fh_gmac_phyt.c
 */
static void fh_int_phy_setup(void)
{
	u32 trim, id;
	int addr, i, r;

	/* The MAC on the internal PHY, clocked from it */
	setbits_le32(FH_EPHY_REG0, BIT(24));
	setbits_le32(FH_PMU_CLK_GATE, BIT(25) | BIT(28));
	clrbits_le32(FH_PMU_CLK_GATE1, BIT(0));
	clrbits_le32(FH_PMU_SYS_CTRL, FH_SYS_CTRL_EXT_CLK);

	/* Reset it, then load the eFuse trim */
	setbits_le32(FH_EPHY_REG0, BIT(31));
	udelay(5);
	clrbits_le32(FH_EPHY_REG0, BIT(31));
	clrbits_le32(FH_PMU_SWRSTN_NSR, BIT(30));
	mdelay(10);
	setbits_le32(FH_PMU_SWRSTN_NSR, BIT(30));
	udelay(12);
	trim = readl(FH_PMU_EPHY_PARAM);
	fh_pmu_update(FH_EPHY_REG0, 0xf << 27,
		      (trim & BIT(4) ? trim & 0xf : 0) << 27);

	fh_gmac_reset();
	addr = fh_phy_find(&id);
	if (addr < 0)
		return;
	printf("internal PHY %08x at %d, ", id, addr);
	for (i = 0; i < ARRAY_SIZE(fh_ephy_regs); i++)
		fh_mdio_write(addr, fh_ephy_regs[i][0], fh_ephy_regs[i][1]);
	r = fh_mdio_read(addr, 0x15);
	if (r >= 0)
		fh_mdio_write(addr, 0x15,
			      (r & ~0x1f1f) | 0x1000 | 0x1f | BIT(13));
	fh_mdio_write(addr, 0x1f, 0);
}

static void fh_phy_setup(void)
{
	if (fh_ext_phy_setup())
		fh_int_phy_setup();
}
#else
/*
 * An RMII PHY on the pads: IP101G, or RTL8201. RXDV is a strap, held low
 * through its GPIO while the PHY comes out of reset (fh_gmac_phy_reset()).
 */
#if defined(CONFIG_FULLHAN_8632)
#define FH_PHY_RESET_GPIO	2
#define FH_PAD_RXDV_GPIO	0x01101000
#define FH_PAD_RXDV		0x00101000
#else
#define FH_PHY_RESET_GPIO	11
#define FH_PAD_RXDV_GPIO	0x01101030
#define FH_PAD_RXDV		0x00101030
#endif
#define FH_RXDV_GPIO		41
#define FH_PAD_RXDV_NUM		26
#define FH_SYS_CTRL_RMII	(BIT(24) | BIT(25) | BIT(26))

static void fh_phy_setup(void)
{
	u32 id;
	int addr;

#ifdef CONFIG_FULLHAN_8632
	/* The PHY's 50 MHz reference from AC_MCLK: PLL1 / 12 */
	writel(0x0b77030b, FH_PMU_CLK_DIV6);
#endif
	writel(FH_PAD_RXDV_GPIO, FH_PMU_PAD(FH_PAD_RXDV_NUM));
	fh_gpio_output(FH_RXDV_GPIO, 0);
	fh_gpio_output(FH_PHY_RESET_GPIO, 1);
	mdelay(150);
	fh_gpio_output(FH_PHY_RESET_GPIO, 0);
	mdelay(10);
	fh_gpio_output(FH_PHY_RESET_GPIO, 1);
	writel(FH_PAD_RXDV, FH_PMU_PAD(FH_PAD_RXDV_NUM));
	mdelay(10);

	setbits_le32(FH_PMU_SYS_CTRL, FH_SYS_CTRL_RMII);
	fh_pmu_reset(FH_PMU_SWRST_AHB, FH_SWRST_GMAC);

	addr = fh_phy_find(&id);
	if (addr >= 0)
		fh_ext_phy_init(addr, id);
}
#endif

int board_eth_init(struct bd_info *bis)
{
	fh_phy_setup();
	return designware_initialize(FH_GMAC_BASE, PHY_INTERFACE_MODE_RMII);
}
#endif

#ifdef CONFIG_MMC_DW
/*
 * SD0: a DesignWare MMC with its internal DMA, at a 50 MHz card clock
 * source. Power is GPIO 5, low on, before V200.
 */
static struct dwmci_host fh_sdc0;

static void fh_sdc0_setup(void)
{
#if defined(CONFIG_FULLHAN_V200) || defined(CONFIG_FULLHAN_865X)
	clrbits_le32(FH_PMU_CLK_GATE, FH_GATE_SDC0);
	clrbits_le32(FH_PMU_CLK_GATE1, BIT(2));
	fh_pmu_update(FH_PMU_CLK_DIV3, 0xf00, 2 << 8);	/* 1200 / 8 / 3 */
	/* Drive at 180 degrees, sample at 0 */
	fh_pmu_update(FH_PMU_CLK_GATE1, 0xff << 16, 8 << 20);
#elif defined(CONFIG_FULLHAN_V100)
	clrbits_le32(FH_PMU_CLK_GATE, FH_GATE_SDC0);
	fh_pmu_update(FH_PMU_CLK_DIV3, 0xf00, 0);		/* 100 / 2 */
	fh_pmu_update(FH_PMU_CLK_GATE1, 0x33 << 16, 2 << 20);
	fh_gpio_output(5, 0);
#elif defined(CONFIG_FULLHAN_8626)
	clrbits_le32(FH_PMU_CLK_GATE, FH_GATE_SDC0);
	fh_pmu_update(FH_PMU_CLK_DIV2, 0xf0000, 0);		/* 100 / 2 */
	fh_pmu_update(FH_PMU_BASE + 0x28, 0x780, 0x100);
	fh_gpio_output(5, 0);
#elif defined(CONFIG_FULLHAN_8632)
	fh_pmu_update(FH_PMU_CLK_DIV3, 0xf00, 5 << 8);		/* 600 / 2 / 6 */
	fh_pmu_update(FH_PMU_CLK_GATE1, 0x33 << 16, 2 << 20);
	fh_gpio_output(5, 0);
#endif
	fh_pmu_reset(FH_PMU_SWRST_AHB, FH_SWRST_SDC0);
}

int board_mmc_init(struct bd_info *bis)
{
	struct dwmci_host *host = &fh_sdc0;
	int ret;

	fh_sdc0_setup();
	host->name = "fh_sdc0";
	host->ioaddr = (void *)FH_SDC0_BASE;
	host->bus_hz = 50000000;
	host->dev_index = 0;
	host->buswidth = 4;
	ret = add_dwmci(host, 25000000, 400000);
#ifdef CONFIG_FULLHAN_8632
	/* D2 and D3 are not bonded out on the FH8632's package */
	if (!ret)
		host->cfg.host_caps &= ~MMC_MODE_4BIT;
#endif
	return ret;
}
#endif
