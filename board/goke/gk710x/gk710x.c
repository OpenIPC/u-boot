// SPDX-License-Identifier: GPL-2.0+
/*
 * Goke GK7101 / GK7102 camera board, after the vendor U-Boot 2012.10
 * (board/goke/gk7101/gk7101.c, drivers/net/goke_eth.c) with its HAL calls
 * replaced by the silicon accesses they make.
 */

#include <common.h>
#include <env.h>
#include <init.h>
#include <malloc.h>
#include <miiphy.h>
#include <net.h>
#include <netdev.h>
#include <openipc.h>
#include <phy.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <asm/arch/gk710x.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * The board's pins before the console: UART0 reaches its pins only
 * through the GPIO matrix, and the boot ROM is not known to route it.
 */
int board_early_init_f(void)
{
	gk_gpio_map_early();
	return 0;
}

int board_init(void)
{
	gd->bd->bi_boot_params = GK_DDR_BASE + 0x100;
	return 0;
}

int misc_init_r(void)
{
	gk_gpio_map_apply();
	return openipc_helper();
}

#ifdef CONFIG_ETH_DESIGNWARE
/*
 * The internal 10/100 PHY is not on the GMAC's MDIO bus: its MII registers
 * are memory mapped, register n in the low half of the word at +4n (the
 * vendor kernel's gkhw_mdio_read() with phytype=0). It answers at address 0.
 */
/* An ID for the internal PHY, which has none: "GK", 0x7101 */
#define GK_EPHY_FAKE_ID		0x474b7101

/*
 * The internal PHY's MII registers, memory mapped. Its BMSR does not set
 * the link bit: the vendor kernel driver reports the link up once
 * autonegotiation is complete, and so does this.
 */
static int gk_ephy_read(struct mii_dev *bus, int addr, int devad, int reg)
{
	u32 val;

	if (addr)
		return 0xffff;
	val = readl(GK_EPHY_BASE + 4 * reg) & 0xffff;
	if (reg == MII_BMSR && (val & BMSR_ANEGCOMPLETE))
		val |= BMSR_LSTATUS;
	/*
	 * It has no ID either (a GK7101 reads 0 in both ID registers, the
	 * vendor kernel prints "mii id = 0"), and phylib skips a PHY whose ID
	 * is 0. Give it one that no driver claims, so the generic driver runs
	 * it.
	 */
	if (reg == MII_PHYSID1 && !val &&
	    !(readl(GK_EPHY_BASE + 4 * MII_PHYSID2) & 0xffff))
		val = GK_EPHY_FAKE_ID >> 16;
	else if (reg == MII_PHYSID2 && !val &&
		 !(readl(GK_EPHY_BASE + 4 * MII_PHYSID1) & 0xffff))
		val = GK_EPHY_FAKE_ID & 0xffff;
	return val;
}

/*
 * BMCR's soft reset is left out: the vendor U-Boot never resets the
 * internal PHY, and phy_connect() would do it after gk_ephy_init()'s
 * analog trim.
 */
static int gk_ephy_write(struct mii_dev *bus, int addr, int devad, int reg,
			 u16 val)
{
	if (reg == MII_BMCR)
		val &= ~BMCR_RESET;
	if (!addr)
		writel(val, GK_EPHY_BASE + 4 * reg);
	return 0;
}

/* The analog trim gk7101_ethphy_init() writes, byte by byte */
static const struct {
	u16 off;
	u8 val;
} gk_ephy_trim[] = {
	{ 0x3bc, 0x59 }, { 0x3e8, 0x21 }, { 0x0e4, 0x80 }, { 0x3f8, 0x00 },
	{ 0x3f9, 0x00 }, { 0x36d, 0x07 }, { 0x395, 0x11 }, { 0x398, 0x80 },
	{ 0x399, 0xd1 }, { 0x3a8, 0x00 }, { 0x371, 0x40 }, { 0x375, 0x05 },
	{ 0x3d4, 0x46 }, { 0x341, 0x00 }, { 0x474, 0x03 }, { 0x475, 0x00 },
	{ 0x368, 0x56 },	/* gain shift */
	{ 0x29d, 0x02 },	/* det max */
	{ 0x2a1, 0x01 },	/* det min */
	{ 0x2ed, 0x18 },	/* snr len (EMC noise) */
	{ 0x285, 0x15 },	/* snr k */
	{ 0x589, 0x00 },	/* 100 gat */
	{ 0x460, 0x43 },	/* 200 gat */
	{ 0x471, 0x41 },	/* en_100t_phase */
	{ 0x055, 0x00 },
	{ 0x3b0, 0xb0 }, { 0x3b1, 0x30 },	/* analog */
};

/* REG_EPHY_POWER in the vendor kernel: 0 all on, 0x3ff all off */
#define GK_EPHY_POWER		0x474

static void gk_ephy_init(void)
{
	ulong start;
	int i;

	/* Its reference clock: the audio PLL at 96 MHz / 4, out on a pin */
	writel(GK_PLL_AUDIO_EPHY, GK_RCT_PLL_AUDIO_CTRL);
	writel(0, GK_RCT_PLL_AUDIO_FRAC);
	writel(4, GK_RCT_DIV_AUDIO_PST);
	writel(1, GK_RCT_DIV_AUDIO_PRE);
	writel(readl(GK_GPIO_PER_SEL) & ~1, GK_GPIO_PER_SEL);

	writel(0, GK_EPHY_BASE + 0x540);	/* MDIO error count */
	writel(0xc400, GK_EPHY_BASE + 0xc8);	/* mux: test out */
	writel(0x810a, GK_EPHY_BASE + 0xe0);	/* signal select */
	writel(7, GK_EPHY_BASE + 0x588);	/* DAC 100M clock gate for 10M */
	for (i = 0; i < ARRAY_SIZE(gk_ephy_trim); i++)
		writeb(gk_ephy_trim[i].val, GK_EPHY_BASE + gk_ephy_trim[i].off);
	/* 10BASE-T waveform */
	writeb((readb(GK_EPHY_BASE + 0x5d0) & 0xf8) | 0x06, GK_EPHY_BASE + 0x5d0);
	/* EEE off */
	writeb(readb(GK_EPHY_BASE + 0x059) | 0x40, GK_EPHY_BASE + 0x059);
	writel(readl(GK_EPHY_BASE + 0x58) | BIT(12), GK_EPHY_BASE + 0x58);

	/* The MAC talks to the internal PHY, not the MII pins */
	writel(readl(GK_AHB_GENERAL0) & ~BIT(1), GK_AHB_GENERAL0);

	/*
	 * Power it fully on, as the vendor kernel does before it uses the
	 * PHY (GK_EPHY_POWER_ON() in GKETH_open()): the trim above leaves
	 * 0x0003, which the vendor U-Boot can live with because it never
	 * reads the PHY. Then give it time to answer on its MII registers.
	 */
	writel(0, GK_EPHY_BASE + GK_EPHY_POWER);
	start = get_timer(0);
	while ((readl(GK_EPHY_BASE + 4 * MII_PHYSID1) & 0xffff) == 0xffff &&
	       get_timer(start) < 500)
		udelay(1000);
}

/*
 * An external PHY (phytype 1: MII, 25 MHz; 2: RMII, 50 MHz), as the
 * vendor's gk7101_eth_config_register() and gk7101_ethphy_init() set it
 * up: its clock from the audio PLL, the MAC on the MII pins, a reset
 * pulse on the map's PHY reset pin, RXD_3's input route as for 100 Mb/s.
 * The PHY is reached over the MAC's own MDIO.
 */
static int gk_extphy_init(int phytype)
{
	unsigned int rst = gk_gpio_map_phy_reset();

	writel(readl(GK_GPIO_PER_SEL) | 1, GK_GPIO_PER_SEL);
	writel(GK_PLL_AUDIO_EXTPHY, GK_RCT_PLL_AUDIO_CTRL);
	writel(0x00555555, GK_RCT_PLL_AUDIO_FRAC);
	writel(phytype == 2 ? 2 : 4, GK_RCT_DIV_AUDIO_PST);
	writel(1, GK_RCT_DIV_AUDIO_PRE);
	if (rst < 63) {
		gk_gpio_config(rst, GK_GPIO_OUTPUT_0);
		mdelay(200);
		gk_gpio_config(rst, GK_GPIO_OUTPUT_1);
		mdelay(200);
	}
	writel(readl(GK_AHB_GENERAL0) | BIT(1), GK_AHB_GENERAL0);
	writel(1, GK_GPIO_INPUT_CFG(30));

	return designware_initialize(GK_GMAC_BASE, phytype == 2 ?
				     PHY_INTERFACE_MODE_RMII :
				     PHY_INTERFACE_MODE_MII);
}

int board_eth_init(struct bd_info *bis)
{
	int phytype = env_get_ulong("phytype", 10, 0);
	struct mii_dev *bus;
	int ret;

	gk_gpio_map_phy(phytype);
	if (phytype)
		return gk_extphy_init(phytype);

	bus = mdio_alloc();
	if (!bus)
		return -ENOMEM;
	gk_ephy_init();

	/* designware_initialize() takes a bus already named after its device */
	snprintf(bus->name, sizeof(bus->name), "dwmac.%x", GK_GMAC_BASE);
	bus->read = gk_ephy_read;
	bus->write = gk_ephy_write;
	ret = mdio_register(bus);
	if (ret) {
		free(bus);
		return ret;
	}

	return designware_initialize(GK_GMAC_BASE, PHY_INTERFACE_MODE_MII);
}
#endif
