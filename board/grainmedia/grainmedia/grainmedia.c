// SPDX-License-Identifier: GPL-2.0+
/*
 * Grain Media GM8135S / GM8136 / GM8136S camera boards, after the GM8136
 * SDK's U-Boot 2013.01 (board/gm/gm8136/gm8136.c) and kernel
 * (drivers/mmc/host/sdhci-ftsdc021.c).
 *
 * The pins and clocks U-Boot itself uses are set here; the kernel sets
 * its own.
 */

#include <common.h>
#include <env.h>
#include <init.h>
#include <mmc.h>
#include <net.h>
#include <netdev.h>
#include <openipc.h>
#include <sdhci.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/grainmedia.h>
#include <linux/delay.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

static void gm_pmu_update(unsigned long reg, u32 mask, u32 val)
{
	writel((readl(reg) & ~mask) | (val & mask), reg);
}

int board_init(void)
{
	gd->bd->bi_boot_params = GM_DDR_BASE + 0x100;
	return 0;
}

/*
 * The vendor kernel takes the whole DRAM as mem= and carves the media
 * pool (frammap) out of it with gmmem=; the SDK U-Boot picks one of its
 * cmd1..cmd4 command lines by the DRAM size. Set them so, unless the
 * environment already has them.
 */
static void gm_set_mem(void)
{
	unsigned long mb = gd->ram_size / SZ_1M;
	unsigned long gmmem;
	char buf[16];

	switch (mb) {
	case 64:
		gmmem = 34;
		break;
	case 128:
		gmmem = 90;
		break;
	case 256:
		gmmem = 190;
		break;
	default:
		gmmem = mb * 27 / 32;	/* 432 of 512 */
		break;
	}

	if (!env_get("osmem")) {
		sprintf(buf, "%luM", mb);
		env_set("osmem", buf);
	}
	if (!env_get("gmmem")) {
		sprintf(buf, "%luM", gmmem);
		env_set("gmmem", buf);
	}
}

int misc_init_r(void)
{
	gm_set_mem();
	return openipc_helper();
}

#ifdef CONFIG_FTGMAC100_NODM
/*
 * RMII, as the SDK U-Boot's board_eth_init(): the MAC and the PHY's
 * 50 MHz reference from PLL2, the MAC pins, and the PHY's reset, GPIO 50
 * (GPIO1 bit 18).
 */
#define GM_PHY_RESET		BIT(18)

int board_eth_init(struct bd_info *bis)
{
	unsigned long pll2 = gm_pll2_hz() / 1000000;

	setbits_le32(GM_PMU_DRIVE1, 3);
	clrbits_le32(GM_PMU_CLK_SEL, BIT(4) | BIT(5));	/* from PLL2 */
	gm_pmu_update(GM_PMU_PINMUX3, 0x003fffff, 0x155555);
	gm_pmu_update(GM_PMU_MISC, BIT(5) | BIT(6), BIT(6));
	if (pll2 % 50)
		printf("GMAC: PLL2 %lu MHz does not divide to 50 MHz\n", pll2);
	gm_pmu_update(GM_PMU_CLK_DIV, 0xf00, (pll2 / 50 - 1) << 8);
	clrbits_le32(GM_PMU_GATE_PHY, BIT(5));		/* PHY clock out */
	clrbits_le32(GM_PMU_GATE_AHB, GM_AHB_GMAC);

	clrbits_le32(GM_PMU_GATE_APB, GM_APB_GPIO1);
	setbits_le32(GM_GPIO1_BASE + GM_GPIO_DOUT, GM_PHY_RESET);
	setbits_le32(GM_GPIO1_BASE + GM_GPIO_DIR, GM_PHY_RESET);
	mdelay(1);
	clrbits_le32(GM_GPIO1_BASE + GM_GPIO_DOUT, GM_PHY_RESET);
	mdelay(10);
	setbits_le32(GM_GPIO1_BASE + GM_GPIO_DOUT, GM_PHY_RESET);
	mdelay(30);

	return ftgmac100_initialize(GM_GMAC_BASE);
}
#endif

#ifdef CONFIG_MMC_SDHCI
/*
 * SD0, an FTSDC021: an SDHCI that does not report its base clock, which
 * is PLL2 / 3 / 2 at the kernel's divider. The pins and the drive
 * strength are the kernel's for SD0.
 */
static struct sdhci_host gm_sdc0;

int board_mmc_init(struct bd_info *bis)
{
	struct sdhci_host *host = &gm_sdc0;

	clrbits_le32(GM_PMU_CLK_SEL, BIT(7));		/* from PLL2 */
	gm_pmu_update(GM_PMU_CLK_DIV, 0x7, 2);
	gm_pmu_update(GM_PMU_DRIVE0, 0x70, 0x20);
	gm_pmu_update(GM_PMU_PINMUX2, 0xfc03fc, 0x540154);
	clrbits_le32(GM_PMU_GATE_AHB, GM_AHB_SDC0);

	host->name = "ftsdc021";
	host->ioaddr = (void *)GM_SDC0_BASE;
	host->max_clk = gm_pll2_hz() / 3 / 2;
	host->bus_width = 4;
	host->voltages = MMC_VDD_32_33 | MMC_VDD_33_34;
	host->quirks = SDHCI_QUIRK_BROKEN_VOLTAGE;

	return add_sdhci(host, 25000000, 400000);
}
#endif
