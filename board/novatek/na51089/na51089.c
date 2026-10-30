// SPDX-License-Identifier: GPL-2.0+
/*
 * Novatek NA51089 camera board: the device tree the loader hands over,
 * and the memory split the vendor kernels expect in it.
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
#include <asm/arch/na51089.h>
#include <linux/delay.h>
#include <linux/libfdt.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * The rest of the vendor U-Boot's board set-up (nvt-na51089/
 * na51089_hw_init.c: nvt_ivot_hw_init_early(), nvt_ivot_hw_init()), in
 * its order. Its PHY reset on D_GPIO(1) is left out: that pin is its
 * EVB's (nvt-na51089-evb.h), not every board's.
 */
#define CG_SYSCLK		(NVT_CG_BASE + 0x10)
#define SYSCLK_CPU		(3 << 0)	/* 80 MHz, PLL8, 480 MHz */
#define SYSCLK_CPU_80		0
#define SYSCLK_CPU_PLL8		1
#define SYSCLK_CPU_480		2
#define SYSCLK_APB		(3 << 8)	/* 48, 60, 80, 120 MHz */
#define CG_PLL_EN		(NVT_CG_BASE + 0x00)
#define CG_PLL_STAT		(NVT_CG_BASE + 0x04)
#define CG_PLL8			BIT(8)		/* the CPU's PLL */
#define CG_CLK_SWITCH		(NVT_CG_BASE + 0xA0)
#define CG_CLKEN_70		(NVT_CG_BASE + 0x70)
#define CG_PLL8_RATIO(n)	(NVT_CG_BASE + 0x4420 + 4 * (n))
#define OTP_TIMING		0xF0660010

/* Wait for (reg & mask) != 0; the vendor waits for ever */
static void nvt_wait_set(ulong reg, u32 mask)
{
	int i;

	for (i = 0; i < 100000 && !(readl(reg) & mask); i++)
		udelay(1);
}

/* A new CPU clock source takes effect through this handshake */
static void nvt_cpu_clk_sel(u32 sel)
{
	clrsetbits_le32(CG_SYSCLK, SYSCLK_CPU, sel);
	writel(0x34, CG_CLK_SWITCH);
	nvt_wait_set(CG_CLK_SWITCH, BIT(3));
	writel(0x28, CG_CLK_SWITCH);
}

/* The vendor's nvt_cpu_freq: 80, 480, or anything else as 960 MHz */
static void nvt_set_cpu_mhz(u32 mhz)
{
	u32 sel = readl(CG_SYSCLK) & SYSCLK_CPU;

	if (mhz == 80 || mhz == 480) {
		u32 want = mhz == 80 ? SYSCLK_CPU_80 : SYSCLK_CPU_480;

		if (sel == want)
			return;
		nvt_cpu_clk_sel(want);
		clrbits_le32(CG_PLL_EN, CG_PLL8);
		return;
	}
	if (sel != SYSCLK_CPU_PLL8) {
		if (!(readl(CG_PLL_EN) & CG_PLL8)) {
			setbits_le32(CG_PLL_EN, CG_PLL8);
			nvt_wait_set(CG_PLL_STAT, CG_PLL8);
		}
		nvt_cpu_clk_sel(SYSCLK_CPU_PLL8);
	}
	/* 960 MHz: 12 MHz * 0xA00000 / 2^17 */
	writel(0x00, CG_PLL8_RATIO(0));
	writel(0x00, CG_PLL8_RATIO(1));
	writel(0xA0, CG_PLL8_RATIO(2));
}

/* The vendor's nvt_apb_freq: 48, 60, 120, or anything else as 80 MHz */
static void nvt_set_apb_mhz(u32 mhz)
{
	u32 sel = mhz == 48 ? 0 : mhz == 60 ? 1 : mhz == 120 ? 3 : 2;

	clrsetbits_le32(CG_SYSCLK, SYSCLK_APB, sel << 8);
}

/* The clocks the loader's device tree asks for: /cpus/cpu@0 */
static void nvt_set_clocks(void)
{
	const void *shm = (const void *)NVT_SHMINFO;
	const void *fdt;
	const fdt32_t *val;
	int node;

	if (memcmp(shm + NVT_SHM_LD_NVT, "LD_NVT", 6))
		return;
	fdt = (const void *)readl(shm + NVT_SHM_FDT_ADDR);
	if (fdt_check_header(fdt))
		return;
	node = fdt_first_subnode(fdt, fdt_path_offset(fdt, "/cpus"));
	if (node < 0)
		return;
	val = fdt_getprop(fdt, node, "clock-frequency", NULL);
	if (val)
		nvt_set_cpu_mhz(fdt32_to_cpu(*val) / 1000000);
	/* Sic, as the vendor spells it */
	val = fdt_getprop(fdt, node, "apb-frequenccy", NULL);
	if (val)
		nvt_set_apb_mhz(fdt32_to_cpu(*val) / 1000000);
}

int board_init(void)
{
	/* The eFuse controller's timing for the APB clock it runs at */
	static const u32 otp_timing[4] = {
		0x692618, 0x8B3026, 0xAF4032, 0xFF6050,
	};

	/* nvt_ivot_hw_init_early() */
	writel(otp_timing[(readl(CG_SYSCLK) >> 8) & 3], OTP_TIMING);
	clrbits_le32(CG_CLKEN_70, BIT(16));

	/* nvt_ivot_hw_init() */
	nvt_set_clocks();
	/* AXI bridge timeouts */
	writel(0x7c, 0xF0012018);
	writel(0x7c, 0xF0013018);
	/* The USB PHY, by the chip's revision, then powered down */
	if (readl(NVT_TOP_CHIP_ID) == 0x50210000) {
		setbits_le32(0xFF600400, BIT(21));
		setbits_le32(0xFF6001C8, BIT(31));
	} else {
		setbits_le32(0xF0600310, BIT(1));
		setbits_le32(0xF06001C8, BIT(5));
	}
	writel(0xFF, 0xF06010D0);
	writel(0x34, 0xF06010D4);
	writel(0x14, 0xF0601008);
	writel(0x88, 0xF0601000);
	writel(0x35, 0xF0601030);
	writel(0xFF, 0xF060102C);
	writel(0x20, 0xF0601028);
	writel(0x93, 0xF0601024);
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
	const void *shm = (const void *)NVT_SHMINFO;
	const void *fdt;
	void *copy;

	if (memcmp(shm + NVT_SHM_LD_NVT, "LD_NVT", 6)) {
		printf("FDT:   no device tree from the loader\n");
		return;
	}
	fdt = (const void *)readl(shm + NVT_SHM_FDT_ADDR);
	if (fdt_check_header(fdt)) {
		printf("FDT:   bad device tree from the loader at %p\n", fdt);
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
 * U-Boot relocates to the top of the loader's region for it, not of the
 * DDR, as the vendor U-Boot does (its ram_size is that region). On the
 * NT98566 the top of the DDR is past 64 MiB, and there the MAC's DMA
 * never took a Tx descriptor (OpenIPC/u-boot#95). On the NT98562 it
 * also keeps clear of /nvt_memory_cfg/core2entry2, just above.
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

#ifdef CONFIG_NVT_EQOS
/*
 * Ethernet: the pins, clocks and embedded PHY around the MAC
 * (drivers/net/nvt_eqos.c), set up as by the vendor U-Boot's
 * eth_na51089.c. How the board is wired comes from the loader's device
 * tree, which the vendor kernels read too: /top@f0010000/eth pinmux.
 * Xiongmai boards say 0x22, the embedded PHY with its LEDs on the second
 * pads.
 */
#define ETH_PIN_RMII		BIT(0)	/* an external PHY on the LCD pads */
#define ETH_PIN_INTERNAL	BIT(1)	/* the embedded PHY */
#define ETH_PIN_LED1		BIT(4)	/* its LEDs on the first pad pair */
#define ETH_PIN_LED2		BIT(5)	/* or the second */
#define ETH_PIN_RMII_2		BIT(6)	/* an external PHY on the P pads */
#define ETH_PIN_LED_ONLY	(3 << 10) /* one LED: the pair's 1st, 2nd */

#define TOP_PINMUX_ETH		(NVT_TOP_BASE + 0x14)	/* RMII [3:2] [31:30] */
#define TOP_PINMUX_MISC		(NVT_TOP_BASE + 0x18)
#define MISC_PHY_LED(n)		((n) << 13)	/* LED pair 1, 2 */
#define MISC_PHY_CLK		BIT(28)		/* 25 MHz out to a PHY */
#define TOP_GPIO_P		(NVT_TOP_BASE + 0xA8)	/* pads as GPIO */
#define TOP_GPIO_DSI		(NVT_TOP_BASE + 0xD0)
#define TOP_GPIO_LCD		(NVT_TOP_BASE + 0xE8)
#define TOP_SRAM_SD		(NVT_TOP_BASE + 0x1000)
#define SRAM_SD_ETH		BIT(18)
#define PAD_DRV_P		(NVT_PAD_BASE + 0x50)
#define PAD_DRV_DSI		(NVT_PAD_BASE + 0x98)
#define CG_PLL6			BIT(6)
#define CG_CLKEN2		(NVT_CG_BASE + 0x78)
#define CG_ETH_PHY_CLK		BIT(24)
#define CG_ETH			BIT(29)		/* in CLKEN0, RSTN0 */

#define ETH_PLL_RSTN		(NVT_ETH_BASE + 0x84)	/* [31:30] */
#define ETH_PHY_SEL		(NVT_ETH_BASE + 0x3004)
#define PHY_SEL_EXT		BIT(4)
#define ETH_LED_CTRL		(NVT_ETH_BASE + 0x300C)
#define LED_INV			(3 << 8)
#define ETH_RMII_CLK		(NVT_ETH_BASE + 0x3014)
#define RMII_REF_IN		BIT(0)
#define RMII_REF_OUT		BIT(4)
#define RMII_REF_CLK_I		BIT(5)
#define RMII_TXD_SRC		(3 << 30)
#define PHY(off)		(NVT_ETH_PHY_BASE + (off))
#define PHY_CTRL		PHY(0x000)
#define PHY_SW_RESET		BIT(1)
#define PHY_PLLREGWR		BIT(3)

int nvt_eqos_initialize(phy_interface_t intf);

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

/* RMII: pins, and which way the 50 MHz reference clock goes */
static void nvt_eth_rmii(const void *fdt, u32 pins)
{
	bool rmii2 = !(pins & ETH_PIN_RMII);

	if (rmii2) {
		setbits_le32(TOP_PINMUX_ETH, BIT(31) | BIT(3));
		clrsetbits_le32(TOP_GPIO_P, 0x73f, 0x800);
		setbits_le32(PAD_DRV_P, BIT(16));	/* REF_CLK, 8 mA */
	} else {
		setbits_le32(TOP_PINMUX_ETH, BIT(30) | BIT(2));
		clrsetbits_le32(TOP_GPIO_LCD, 0x3bf, 0x40);
		setbits_le32(PAD_DRV_DSI, BIT(10));	/* REF_CLK, 8 mA */
	}
	clrsetbits_le32(ETH_RMII_CLK, RMII_REF_IN | RMII_REF_OUT | RMII_TXD_SRC,
			RMII_REF_CLK_I |
			(nvt_fdt_u32(fdt, "/eth@f02b0000", "ref-clk-out", 0) ?
			 RMII_REF_OUT : RMII_REF_IN));
	setbits_le32(ETH_PHY_SEL, PHY_SEL_EXT);

	/* The PHY's 25 MHz crystal clock from the SoC */
	if (nvt_fdt_u32(fdt, "/eth@f02b0000", "sp-clk", 0)) {
		setbits_le32(CG_CLKEN2, CG_ETH_PHY_CLK);
		setbits_le32(TOP_PINMUX_MISC, MISC_PHY_CLK);
		if (rmii2) {
			clrbits_le32(TOP_GPIO_P, BIT(7));
			setbits_le32(PAD_DRV_P, BIT(14));
		} else {
			clrbits_le32(TOP_GPIO_LCD, BIT(10));
		}
	}
	/* The vendor resets the PHY on an EVB GPIO here, not done */
}

/* The embedded PHY: PLL out of reset, power on, then its own reset */
static void nvt_eth_phy_on(u32 led)
{
	mdelay(20);
	clrbits_le32(ETH_PLL_RSTN, 3 << 30);
	mdelay(20);
	setbits_le32(ETH_PLL_RSTN, 3 << 30);
	mdelay(20);

	setbits_le32(PHY(0xF8), BIT(7));
	udelay(20);
	clrbits_le32(PHY(0xC8), BIT(0));
	udelay(200);
	clrbits_le32(PHY(0xC8), BIT(1));
	udelay(250);
	clrbits_le32(PHY(0x2E8), BIT(0));
	clrbits_le32(PHY(0xCC), BIT(0));
	setbits_le32(PHY(0xDC), BIT(0));
	clrbits_le32(PHY(0x9C), BIT(0));

	setbits_le32(PHY_CTRL, PHY_SW_RESET);
	/* The break-link timer, and the LED mode, as the vendor */
	writel(0x53, PHY(0x288));
	writel(0x07, PHY(0x28C));
	writel(0xC9, PHY(0x284));
	writel(0x40, PHY(0x100));
	writel(led, ETH_LED_CTRL);
	mdelay(10);
	clrsetbits_le32(PHY_CTRL, PHY_SW_RESET, PHY_PLLREGWR);
}

int board_eth_init(struct bd_info *bis)
{
	const void *fdt = (const void *)env_get_hex("fdtaddr", 0);
	u32 pins = nvt_fdt_u32(fdt, "/top@f0010000/eth", "pinmux",
			       ETH_PIN_INTERNAL);
	bool rmii = pins & (ETH_PIN_RMII | ETH_PIN_RMII_2);
	u32 led = 0x312, pads = 3;	/* both LEDs, blinking */
	int n = pins & ETH_PIN_LED1 ? 1 : pins & ETH_PIN_LED2 ? 2 : 0;

	if (!rmii && !(pins & ETH_PIN_INTERNAL))
		return 0;

	/* The PHY's LEDs, on either of two pad pairs */
	if (n) {
		if (pins & ETH_PIN_LED_ONLY) {
			pads = pins & BIT(10) ? 1 : 2;
			led = 0x300;
		}
		clrbits_le32(TOP_GPIO_DSI, n == 1 ? pads : pads << 5);
		clrsetbits_le32(TOP_PINMUX_MISC, MISC_PHY_LED(3),
				MISC_PHY_LED(n));
	}
	if (nvt_fdt_u32(fdt, "/phy@f02b3800", "led-inv", 0))
		led &= ~LED_INV;

	clrbits_le32(TOP_PINMUX_ETH, 0xc000000c);
	if (!rmii)
		clrbits_le32(TOP_PINMUX_MISC, MISC_PHY_CLK);

	setbits_le32(CG_PLL_EN, CG_PLL6);
	udelay(10);
	setbits_le32(NVT_CG_CLKEN0, CG_ETH);
	udelay(10);
	setbits_le32(NVT_CG_RSTN0, CG_ETH);
	udelay(10);
	clrbits_le32(TOP_SRAM_SD, SRAM_SD_ETH);

	if (rmii)
		nvt_eth_rmii(fdt, pins);
	else
		clrbits_le32(ETH_PHY_SEL, 3 << 4);
	/* Powered either way, as by the vendor */
	nvt_eth_phy_on(led);

	return nvt_eqos_initialize(rmii ? PHY_INTERFACE_MODE_RMII :
				   PHY_INTERFACE_MODE_MII);
}
#endif
