// SPDX-License-Identifier: GPL-2.0+
/*
 * Novatek NA51068 DVR/NVR board (NT98321, NT98323): the device tree the
 * loader hands over, the memory split the vendor kernels expect in it,
 * the Ethernet glue and the SD host.
 */

#include <common.h>
#include <env.h>
#include <fdt_support.h>
#include <init.h>
#include <lmb.h>
#include <malloc.h>
#include <mmc.h>
#include <netdev.h>
#include <openipc.h>
#include <phy_interface.h>
#include <sdhci.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/na51068.h>
#include <linux/delay.h>
#include <linux/libfdt.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	return 0;
}

/*
 * The loader leaves its device tree, which describes the board for the
 * vendor kernels, in DDR and its address in SHMINFO. OpenIPC uImages carry
 * no device tree, so bootm passes this one (${fdtaddr}). It lies where the
 * kernel is unpacked, so it is copied away first.
 */
static void nvt_fdt_init(void)
{
	const void *fdt = nvt_loader_fdt();
	void *copy;

	if (!fdt) {
		printf("FDT:   no device tree from the loader\n");
		return;
	}
	copy = malloc(fdt_totalsize(fdt));
	if (!copy)
		return;
	memcpy(copy, fdt, fdt_totalsize(fdt));
	env_set_hex("fdtaddr", (ulong)copy);
}

int misc_init_r(void)
{
	int ret = openipc_helper();

	/* After the helper's first saveenv: the address is this boot's */
	nvt_fdt_init();
	return ret;
}

/*
 * U-Boot relocates to the top of the loader's region for it
 * (/nvt_memory_cfg/uboot), as on NA51089: clear of core2entry2 at the
 * top of the 512 MiB, where the second core waits for Linux.
 */
ulong board_get_usable_ram_top(ulong total_size)
{
	return NVT_UBOOT_BASE + NVT_UBOOT_SIZE;
}

/* What Linux gets of the DDR (mem=); the vendor media modules get the rest */
static ulong nvt_osmem(void)
{
	char *end;
	ulong osmem = ustrtoul(env_get("osmem") ?: "", &end, 0);

	if (!osmem || osmem > gd->ram_size)
		osmem = gd->ram_size;
	return osmem;
}

/*
 * The device tree and initrd go where Linux can reach them: everything
 * past ${osmem} is reserved. lmb refuses a reservation that overlaps one
 * it has (U-Boot's own, up to the end of DDR), so only the gaps are.
 */
void board_lmb_reserve(struct lmb *lmb)
{
	struct lmb_property rsv[8];
	phys_addr_t lo = NVT_DDR_BASE + nvt_osmem();
	phys_addr_t hi = NVT_DDR_BASE + gd->ram_size;
	unsigned long i, n = min_t(unsigned long, lmb->reserved.cnt,
				   ARRAY_SIZE(rsv));

	/* A copy: the reservations below insert into the sorted list */
	memcpy(rsv, &lmb->reserved.region[0], n * sizeof(rsv[0]));
	for (i = 0; i < n && lo < hi; i++) {
		phys_addr_t base = rsv[i].base;
		phys_addr_t end = base + rsv[i].size;

		if (end <= lo || base >= hi)
			continue;
		if (base > lo)
			lmb_reserve(lmb, lo, base - lo);
		lo = max(lo, end);
	}
	if (lo < hi)
		lmb_reserve(lmb, lo, hi - lo);
}

/*
 * As the vendor U-Boot's nvt_boot: Linux gets the first ${osmem} of the
 * DDR, and the media pool of the vendor modules (/hdal-memory/media) the
 * rest.
 */
int ft_board_setup(void *blob, struct bd_info *bd)
{
	ulong osmem = nvt_osmem();
	fdt32_t media[2];
	int node, ret;

	ret = fdt_fixup_memory(blob, NVT_DDR_BASE, osmem);
	if (ret)
		return ret;

	node = fdt_path_offset(blob, "/hdal-memory/media");
	if (node < 0)
		return 0;
	media[0] = cpu_to_fdt32(NVT_DDR_BASE + osmem);
	media[1] = cpu_to_fdt32(gd->ram_size - osmem);
	return fdt_setprop(blob, node, "reg", media, sizeof(media));
}

static u32 nvt_fdt_u32(const void *fdt, const char *path, const char *prop,
		       u32 def)
{
	const fdt32_t *val = NULL;
	int node;

	if (fdt) {
		node = fdt_path_offset(fdt, path);
		if (node >= 0)
			val = fdt_getprop(fdt, node, prop, NULL);
	}
	return val ? fdt32_to_cpu(*val) : def;
}

#ifdef CONFIG_NVT_EQOS
/*
 * Ethernet: two MACs, each wired by /top@fe030000/eth pinmux in the
 * loader's device tree to an RGMII or RMII PHY on its own pads, or to the
 * embedded PHY (0x1201 on the SDK's NT98323 boards: MAC 0 RGMII, the
 * embedded PHY on MAC 1). Pins, clocks, resets and the PHY as by the
 * vendor U-Boot's eth_na51068.c; nvt_eqos does the MAC. Each MAC with a
 * PHY is registered, eth0 first: U-Boot goes on to the next if one has
 * no link.
 */
#define ETH_RGMII(n)		BIT(n)		/* 0x001, 0x002 */
#define ETH_RMII(n)		BIT(4 + (n))	/* 0x010, 0x020 */
#define ETH_EMB_PHY(n)		BIT(8 + (n))	/* 0x100, 0x200 */

#define CG(off)			(NVT_CG_BASE + (off))
#define TOP(off)		(NVT_TOP_BASE + (off))
#define PAD(off)		(NVT_PAD_BASE + (off))
#define CG_AXI_RSTN		CG(0x54)
#define CG_MOD_RSTN		CG(0x58)
#define CG_BUS_GATE		CG(0x60)
#define CG_MOD_GATE		CG(0x70)
#define CG_GPIO_GATE		CG(0x74)
#define CG_ETH_GLUE		CG(0xE4)
#define TOP_EMB_PHY		TOP(0xC8)	/* bit 0: to MAC 0, else MAC 1 */

/* The glue, at +0x3000; the embedded PHY at +0x3800 */
#define ETH_PHY_SEL(b)		((b) + 0x3004)
#define PHY_SEL_EXT		BIT(8)
#define ETH_LED_CTRL(b)		((b) + 0x300C)
#define ETH_CLK_CTRL(b)		((b) + 0x3014)
#define CLK_REF_IN		BIT(0)
#define CLK_REF_PAD		BIT(2)
#define CLK_REF_OUT		BIT(4)
#define CLK_TXD_SRC		(3 << 30)
#define CLK_TXD_RGMII		(1 << 30)
#define PHY(b, off)		((b) + 0x3800 + (off))
#define PHY_SW_RESET		BIT(1)
#define PHY_PLLREGWR		BIT(3)

int nvt_eqos_register(const char *name, ulong base, phy_interface_t intf,
		      u32 cr);

static const struct nvt_mac {
	ulong base;
	const char *node;
	u8 axi_rst, mod_rst, glue_rst, bus_gate, mod_gate;
	u8 gpio_gate, gpio_rst, refclk_gate, phyclk_out;
	ulong phyclk_mux;	/* the PHY's 25 MHz on a pad, [2:0] */
	u32 phy_rst;		/* the PHY's reset GPIO, the EVB's */
} nvt_macs[2] = {
	{ NVT_ETH0_BASE, "/eth0@fcc00000", 1, 9, 8, 16, 9, 5, 29, 4, 7,
	  TOP(0x70), 2 * 32 + 21 },
	{ NVT_ETH1_BASE, "/eth1@fcd00000", 2, 10, 24, 17, 10, 4, 28, 20, 23,
	  TOP(0x50), 1 * 32 + 17 },
};

/* The four FTGPIO010 banks, 32 pins each */
static const ulong nvt_gpio_bank[4] = {
	0xFE400000, NVT_GPIO1_BASE, NVT_GPIO2_BASE, 0xFE640000,
};

#define GPIO_DIR		0x08
#define GPIO_SET		0x10
#define GPIO_CLEAR		0x14

/* Hold an external PHY in reset for 10 ms, then 50 ms to wake up */
static void nvt_phy_reset(u32 pin)
{
	ulong bank;

	if (pin / 32 >= ARRAY_SIZE(nvt_gpio_bank))
		return;
	bank = nvt_gpio_bank[pin / 32];
	setbits_le32(bank + GPIO_DIR, BIT(pin % 32));
	writel(BIT(pin % 32), bank + GPIO_CLEAR);
	mdelay(10);
	writel(BIT(pin % 32), bank + GPIO_SET);
	mdelay(50);
}

/* The pads of an external PHY, as the vendor's mac0/1_pinmux_en() */
static void nvt_mac_pins(int n, phy_interface_t intf)
{
	bool rgmii = intf == PHY_INTERFACE_MODE_RGMII;

	if (n == 0) {
		/* Drive: PHYCLK 8 mA, and for RGMII the Tx pads too */
		clrsetbits_le32(PAD(0xC0), rgmii ? 0xffc0003 : 0xfc0033,
				rgmii ? 0x5540001 : 0x1);
		clrsetbits_le32(TOP(0xC8), 0xf << 4, (rgmii ? 1 : 4) << 4);
		clrsetbits_le32(TOP(0x70), rgmii ? 0xffffff0f : 0xfff0f,
				rgmii ? 0x11111101 : 0x00222202);
		clrsetbits_le32(TOP(0x74), rgmii ? 0xffffffff : 0xff00fff0,
				rgmii ? 0x11111111 : 0x22002220);
		clrsetbits_le32(PAD(0x40), 0xf000000c, 0xa0000008);
	} else {
		if (rgmii) {
			clrsetbits_le32(PAD(0xA0), 0x30003, 0x10001);
			clrsetbits_le32(PAD(0xA4), 0x3ff, 0x155);
		} else {
			clrsetbits_le32(PAD(0xA0), 0xf, 0x1);
			clrbits_le32(PAD(0xA4), 0x3f);
		}
		clrsetbits_le32(TOP(0x50), rgmii ? 0xffffff0f : 0xffff0f,
				rgmii ? 0x66666606 : 0x222202);
		clrsetbits_le32(TOP(0x54), rgmii ? 0xffffff0f : 0xff00fff0,
				rgmii ? 0x66666606 : 0x22002220);
		clrsetbits_le32(TOP(0xC8), 0xf << 8, (rgmii ? 1 : 4) << 8);
	}
}

/* Clocks and resets, as the vendor's mac0/1_clk_en() */
static void nvt_mac_clocks(const struct nvt_mac *m, phy_interface_t intf)
{
	clrbits_le32(CG_AXI_RSTN, BIT(m->axi_rst));
	clrbits_le32(CG_MOD_RSTN, BIT(m->mod_rst));
	clrbits_le32(CG_ETH_GLUE, BIT(m->glue_rst));
	clrbits_le32(CG_BUS_GATE, BIT(m->bus_gate));
	clrbits_le32(CG_MOD_GATE, BIT(m->mod_gate));
	clrbits_le32(CG_GPIO_GATE, BIT(m->gpio_gate));
	/* The embedded PHY: MAC 0 gates its RMII reference in, MAC 1 not */
	if (intf == PHY_INTERFACE_MODE_MII) {
		if (m->base == NVT_ETH0_BASE)
			setbits_le32(CG_ETH_GLUE, BIT(m->refclk_gate));
		else
			clrbits_le32(CG_ETH_GLUE, BIT(m->refclk_gate));
	}
	setbits_le32(CG_AXI_RSTN, BIT(m->axi_rst));
	setbits_le32(CG_MOD_RSTN, BIT(m->mod_rst) | BIT(m->gpio_rst));
	setbits_le32(CG_ETH_GLUE, BIT(m->glue_rst));
}

/* The embedded PHY: power on, then its own reset, as the vendor */
static void nvt_emb_phy_on(ulong b)
{
	setbits_le32(PHY(b, 0xF8), BIT(7));
	udelay(20);
	clrbits_le32(PHY(b, 0xC8), BIT(0));
	udelay(200);
	clrbits_le32(PHY(b, 0xC8), BIT(1));
	udelay(250);
	clrbits_le32(PHY(b, 0x2E8), BIT(0));
	clrbits_le32(PHY(b, 0xCC), BIT(0));
	setbits_le32(PHY(b, 0xDC), BIT(0));
	clrbits_le32(PHY(b, 0x9C), BIT(0));

	setbits_le32(PHY(b, 0x000), PHY_SW_RESET);
	/* The break-link timer, and the LEDs blinking, as the vendor */
	writel(0x53, PHY(b, 0x288));
	writel(0x07, PHY(b, 0x28C));
	writel(0xC9, PHY(b, 0x284));
	writel(0x40, PHY(b, 0x100));
	writel(0x312, ETH_LED_CTRL(b));
	mdelay(10);
	clrsetbits_le32(PHY(b, 0x000), PHY_SW_RESET, PHY_PLLREGWR);
}

/* The PHY side, as the vendor's extphy0/1_clk_en() */
static void nvt_mac_phy(const void *fdt, const struct nvt_mac *m,
			phy_interface_t intf)
{
	ulong b = m->base;

	if (intf == PHY_INTERFACE_MODE_MII) {
		clrbits_le32(ETH_PHY_SEL(b), PHY_SEL_EXT);
		nvt_emb_phy_on(b);
		return;
	}
	if (intf == PHY_INTERFACE_MODE_RMII) {
		u32 v = readl(ETH_CLK_CTRL(b)) & ~(CLK_REF_PAD | CLK_TXD_SRC);

		if (nvt_fdt_u32(fdt, m->node, "ref-clk-out", 0))
			v = (v & ~CLK_REF_IN) | CLK_REF_OUT;
		writel(v, ETH_CLK_CTRL(b));
	} else {
		setbits_le32(ETH_CLK_CTRL(b), CLK_TXD_RGMII);
	}
	setbits_le32(ETH_PHY_SEL(b), PHY_SEL_EXT);
	writel(0x1FF, ETH_LED_CTRL(b));

	/* The PHY's 25 MHz crystal clock from the SoC */
	if (nvt_fdt_u32(fdt, m->node, "sp-clk", 0)) {
		setbits_le32(CG_ETH_GLUE, BIT(m->phyclk_out));
		clrsetbits_le32(m->phyclk_mux, 0x7,
				intf == PHY_INTERFACE_MODE_RMII ? 2 :
				m->base == NVT_ETH0_BASE ? 1 : 6);
	}
	nvt_phy_reset(nvt_fdt_u32(fdt, m->node, "phy-rst", m->phy_rst));
}

int board_eth_init(struct bd_info *bis)
{
	const void *fdt = nvt_loader_fdt();
	u32 pins = nvt_fdt_u32(fdt, "/top@fe030000/eth", "pinmux",
			       ETH_EMB_PHY(1));
	u32 misc = nvt_fdt_u32(fdt, "/top@fe030000/misc", "pinmux", 0);
	int n, ret = 0;

	/* The embedded PHY's LEDs, on either of two pad pairs */
	if (misc & BIT(20))
		clrsetbits_le32(TOP(0x20), 0xff0, 0x550);
	else if (misc & BIT(21))
		clrsetbits_le32(TOP(0x44), 0xff, 0x33);

	for (n = 0; n < 2; n++) {
		const struct nvt_mac *m = &nvt_macs[n];
		phy_interface_t intf;
		char name[8];

		writel(0x3FF, ETH_LED_CTRL(m->base));
		if (fdt && fdt_path_offset(fdt, m->node) < 0)
			continue;
		if (pins & ETH_RMII(n))
			intf = PHY_INTERFACE_MODE_RMII;
		else if (pins & ETH_RGMII(n))
			intf = PHY_INTERFACE_MODE_RGMII;
		else if (pins & ETH_EMB_PHY(n))
			intf = PHY_INTERFACE_MODE_MII;
		else
			continue;

		if (intf == PHY_INTERFACE_MODE_MII) {
			if (n == 0)
				clrsetbits_le32(TOP_EMB_PHY, 0xf << 4, BIT(0));
			else
				clrbits_le32(TOP_EMB_PHY, 0xf << 8 | BIT(0));
		} else {
			nvt_mac_pins(n, intf);
		}
		nvt_mac_clocks(m, intf);
		nvt_mac_phy(fdt, m, intf);

		/* MDC: the CSR clock is the 150 MHz APB, CR 1 (/62) */
		snprintf(name, sizeof(name), "eth%d", n);
		if (!nvt_eqos_register(name, m->base, intf, 1))
			ret++;
	}
	return ret;
}
#endif

#ifdef CONFIG_MMC_SDHCI
/*
 * The SD card: a standard SDHCI 3.0 at 0xFA600000 (the vendor's
 * nvt_ivot_sdc021.c), 4-bit, its base clock from its capabilities, up to
 * the device tree's max-frequency. Its pins as /top@fe030000/sdio pinmux
 * says: 0x10 the first pad set, 0x20 the second (the SDK's boards).
 */
#define SDIO_PINS_1ST		BIT(4)
#define SDIO_PINS_2ND		BIT(5)

int board_mmc_init(struct bd_info *bis)
{
	const void *fdt = nvt_loader_fdt();
	u32 pins = nvt_fdt_u32(fdt, "/top@fe030000/sdio", "pinmux",
			       SDIO_PINS_2ND);
	u32 max_hz = nvt_fdt_u32(fdt, "/mmc@fa600000", "max-frequency",
				 12000000);
	struct sdhci_host *host;

	if (!(pins & (SDIO_PINS_1ST | SDIO_PINS_2ND)))
		return 0;
	setbits_le32(CG(0x54), BIT(10));	/* out of reset */
	clrbits_le32(CG(0x68), BIT(10));	/* clock on */
	clrbits_le32(CG(0x18C), 0x3f << 8);	/* the clock undivided */
	/* Pins, and pull-ups on the data and command lines */
	if (pins & SDIO_PINS_1ST) {
		clrsetbits_le32(TOP(0x28), 0xf << 28, BIT(28) | BIT(30));
		clrsetbits_le32(TOP(0x2C), 0xf | 0xf << 28,
				0x5 | BIT(28) | BIT(30));
		clrsetbits_le32(TOP(0x30), 0xfff | 0xf << 16,
				0x555 | BIT(16) | BIT(18));
		clrsetbits_le32(PAD(0x04), 0xf << 14 | 0x3 << 30,
				BIT(15) | BIT(17) | BIT(31));
		clrsetbits_le32(PAD(0x08), 0x33f, BIT(1) | BIT(3) | BIT(5) |
				BIT(9));
	} else {
		clrsetbits_le32(TOP(0x2C), 0x770, 0x550);
		clrsetbits_le32(TOP(0x40), 0x77700000, 0x66600000);
		clrsetbits_le32(TOP(0x44), 0x77, 0x66);
		clrsetbits_le32(PAD(0x04), 0xf << 18, BIT(19) | BIT(21));
		clrsetbits_le32(PAD(0x10), 0x3ff << 10, BIT(11) | BIT(13) |
				BIT(15) | BIT(17) | BIT(19));
	}

	host = calloc(1, sizeof(*host));
	if (!host)
		return -ENOMEM;
	host->name = "sdhci";
	host->ioaddr = (void *)NVT_SDHCI_BASE;
	host->host_caps = MMC_MODE_4BIT;
	return add_sdhci(host, max_hz, 400000);
}
#endif
