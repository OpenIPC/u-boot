// SPDX-License-Identifier: GPL-2.0+
/*
 * Novatek NA51103 (NT98332) / NA51090 (NT98633) NVR boards: the device
 * tree the loader hands over, the memory split the vendor kernels expect
 * in it, and the Ethernet set-up.
 *
 * The boot chain is the board's own up to here: the mask ROM, the board's
 * loader (DDR training), its BL31 (TF-A v2.2), which enters U-Boot in
 * AArch64 at EL2 and stays resident at 0x1F00000 for PSCI.
 */

#include <common.h>
#include <env.h>
#include <fdt_support.h>
#include <init.h>
#include <lmb.h>
#include <malloc.h>
#include <netdev.h>
#include <openipc.h>
#include <phy_interface.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
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
 * kernel is put (0x80000 on), so it is copied away first.
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
 * (/nvt_memory_cfg/uboot), as the vendor U-Boot does, not to the top of
 * the DDR.
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

/* Reserve [lo, hi) where lmb has nothing yet: it refuses overlaps */
static void nvt_lmb_reserve_gaps(struct lmb *lmb, phys_addr_t lo,
				 phys_addr_t hi)
{
	struct lmb_property rsv[8];
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
 * The device tree and initrd go where Linux can reach them: everything
 * past ${osmem} is reserved. BL31's secure megabyte is the tree's
 * /reserved-memory, which bootm reserves itself.
 */
void board_lmb_reserve(struct lmb *lmb)
{
	nvt_lmb_reserve_gaps(lmb, NVT_DDR_BASE + nvt_osmem(),
			     NVT_DDR_BASE + gd->ram_size);
}

/* Write a <base size> reg in the cells of the node's parent */
static int nvt_set_reg(void *blob, int node, u64 base, u64 size)
{
	int parent = fdt_parent_offset(blob, node);
	int ac = fdt_address_cells(blob, parent);
	int sc = fdt_size_cells(blob, parent);
	fdt32_t reg[8];
	int i;

	if (ac < 1 || sc < 1 || ac + sc > ARRAY_SIZE(reg))
		return -FDT_ERR_BADNCELLS;
	/* Values in the low cells; the vendor's 4-cell addresses lead with 0 */
	for (i = 0; i < ac; i++)
		reg[i] = cpu_to_fdt32(i == ac - 1 ? (u32)base :
				      i == ac - 2 ? (u32)(base >> 32) : 0);
	for (i = 0; i < sc; i++)
		reg[ac + i] = cpu_to_fdt32(i == sc - 1 ? (u32)size :
					   i == sc - 2 ? (u32)(size >> 32) : 0);
	return fdt_setprop(blob, node, "reg", reg, (ac + sc) * sizeof(reg[0]));
}

/*
 * As the vendor U-Boot's nvt_boot: Linux gets the first ${osmem} of the
 * DDR, and the media pool of the vendor modules (/hdal-memory/media) the
 * rest. The tree keeps BL31 out of Linux's reach itself
 * (/reserved-memory, no-map).
 */
int ft_board_setup(void *blob, struct bd_info *bd)
{
	ulong osmem = nvt_osmem();
	int node, ret;

	ret = fdt_fixup_memory(blob, NVT_DDR_BASE, osmem);
	if (ret)
		return ret;

	node = fdt_path_offset(blob, "/hdal-memory/media");
	if (node < 0)
		return 0;
	return nvt_set_reg(blob, node, NVT_DDR_BASE + osmem,
			   gd->ram_size - osmem);
}

#ifdef CONFIG_NVT_EQOS
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

int nvt_eqos_initialize(phy_interface_t intf);

/*
 * /top/eth pinmux in the loader's tree (the vendor's PIN_ETH_CFG): how
 * the first MAC is wired
 */
#define ETH_PIN_RGMII		BIT(0)	/* an RGMII PHY on the first pads */
#define ETH_PIN_RMII		BIT(4)	/* an RMII PHY on the first pads */
#define ETH_PIN_REFCLK		BIT(8)	/* the PHY's 25 MHz from the SoC */
#define ETH_PIN_RST		BIT(9)	/* the PHY's reset on E_GPIO 1 */

#define TOP(off)		(NVT_TOP_BASE + (off))
#define CG(off)			(NVT_CG_BASE + (off))
#define PAD(off)		(NVT_PAD_BASE + (off))
#define GLUE(off)		(NVT_ETH_BASE + 0x3000 + (off))
#define EPHY(off)		(NVT_ETH_BASE + 0x3800 + (off))

#define GPIO_DIR(n)		(NVT_GPIO_BASE + 0x30 + ((n) >> 5) * 4)
#define GPIO_SET(n)		(NVT_GPIO_BASE + 0x60 + ((n) >> 5) * 4)
#define GPIO_CLR(n)		(NVT_GPIO_BASE + 0x90 + ((n) >> 5) * 4)
#define E_GPIO(n)		(0x60 + (n))

#ifdef CONFIG_NVT_NA51103
/* A GPIO as an output, at @val */
static void nvt_gpio_out(unsigned int n, int val)
{
	writel(BIT(n & 31), val ? GPIO_SET(n) : GPIO_CLR(n));
	setbits_le32(GPIO_DIR(n), BIT(n & 31));
}

/*
 * The embedded PHY's power-on, as eth_na51103.c's eth_phy_poweron() and
 * its PHY reset, break-link timer and LED settings
 */
static void nvt_eth_phy_on(void)
{
	setbits_le32(EPHY(0xF8), BIT(7));
	udelay(20);
	clrbits_le32(EPHY(0xC8), BIT(0));
	udelay(200);
	clrbits_le32(EPHY(0xC8), BIT(1));
	udelay(250);
	clrbits_le32(EPHY(0x2E8), BIT(0));
	clrbits_le32(EPHY(0xCC), BIT(0));
	setbits_le32(EPHY(0xDC), BIT(0));
	clrbits_le32(EPHY(0x9C), BIT(0));

	setbits_le32(EPHY(0x000), BIT(1));
	writel(0x53, EPHY(0x288));
	writel(0x07, EPHY(0x28C));
	writel(0xC9, EPHY(0x284));
	writel(0x40, EPHY(0x100));
	mdelay(10);
	clrbits_le32(EPHY(0x000), BIT(1));
}

/*
 * The first MAC (0x2_F044_0000) as the vendor U-Boot's eth_na51103.c sets
 * it up: mac0_pinmux_en(), mac0_clk_en(), extphy0_clk_en(), for its first
 * pad set (the vendor's second one is compiled out). Xiongmai's
 * NBD80N36RA says 0x701: an RGMII PHY with its clock and reset from the
 * SoC.
 */
static phy_interface_t nvt_eth_setup(const void *fdt)
{
	u32 pins = nvt_fdt_u32(fdt, "/top/eth", "pinmux", 0);
	bool ext_clk = pins & ETH_PIN_REFCLK ||
		       nvt_fdt_u32(fdt, "/eth0", "sp-clk", 0);
	phy_interface_t intf;

	if (pins & ETH_PIN_RMII) {
		intf = PHY_INTERFACE_MODE_RMII;
		clrsetbits_le32(TOP(0x0C), 7 << 0, 2 << 0);
		clrsetbits_le32(TOP(0x0C), 3 << 12, 1 << 12);	/* MDIO */
		clrbits_le32(TOP(0xB0), 0xCE3C);		/* not GPIO */
	} else if (pins & ETH_PIN_RGMII) {
		intf = PHY_INTERFACE_MODE_RGMII;
		/* PHY clock, Tx clock, control and data at 8 mA */
		clrsetbits_le32(PAD(0x90), 0xf, 0x1);
		clrsetbits_le32(PAD(0x94), 0xffffff, 0x111111);
		clrsetbits_le32(TOP(0x0C), 7 << 0, 1 << 0);
		clrsetbits_le32(TOP(0x0C), 3 << 12, 1 << 12);	/* MDIO */
		clrbits_le32(TOP(0xB0), 0xFFFC);		/* not GPIO */
	} else {
		intf = PHY_INTERFACE_MODE_MII;
		setbits_le32(TOP(0x0C), BIT(31));	/* the embedded PHY */
	}

	/* PLL7, the MAC's clock, out of reset */
	setbits_le32(CG(0x00), BIT(7));
	udelay(10);
	setbits_le32(CG(0x70), BIT(27));
	udelay(10);
	setbits_le32(CG(0x94), BIT(16));
	udelay(10);

	if (intf == PHY_INTERFACE_MODE_RMII) {
		u32 v = readl(GLUE(0x14));

		/* The reference clock: from the PHY, or out to it */
		v &= ~(BIT(2) | BIT(4) | 3 << 30);
		v |= BIT(0) | BIT(5);
		if (nvt_fdt_u32(fdt, "/eth0", "ref-clk-out", 0))
			v = (v & ~BIT(0)) | BIT(4);
		writel(v, GLUE(0x14));
	} else if (intf == PHY_INTERFACE_MODE_RGMII) {
		/* As the vendor: [31:30] kept, Tx data from RGMII */
		writel((readl(GLUE(0x14)) & (3 << 30)) | 1 << 30 | 3 << 4,
		       GLUE(0x14));
	}
	if (intf == PHY_INTERFACE_MODE_MII) {
		clrbits_le32(GLUE(0x04), 3 << 4);	/* the embedded PHY */
		nvt_eth_phy_on();
	} else {
		setbits_le32(GLUE(0x04), BIT(4));	/* an external one */
		writel(0x1FF, GLUE(0x0C));
	}

	if (intf != PHY_INTERFACE_MODE_MII && ext_clk) {
		/* The PHY's 25 MHz out of the SoC */
		setbits_le32(CG(0x74), BIT(7));
		setbits_le32(TOP(0x0C), BIT(8));
		clrbits_le32(TOP(0xB0), BIT(0));
	}
	if (intf != PHY_INTERFACE_MODE_MII) {
		/* The PHY's reset, on E_GPIO 1 */
		nvt_gpio_out(E_GPIO(1), 0);
		mdelay(10);
		nvt_gpio_out(E_GPIO(1), 1);
		mdelay(50);
	}
	return intf;
}
#endif /* CONFIG_NVT_NA51103 */

int board_eth_init(struct bd_info *bis)
{
	const void *fdt = nvt_loader_fdt();

	/*
	 * phylib binds the PHY as the vendor does, as MII: the PHY keeps
	 * its strapped RGMII delays, and the MAC follows the speed it
	 * negotiates (1000, 100 or 10 Mb/s).
	 */
	nvt_eth_setup(fdt);
	return nvt_eqos_initialize(PHY_INTERFACE_MODE_MII);
}
#endif /* CONFIG_NVT_EQOS */
