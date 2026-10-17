// SPDX-License-Identifier: GPL-2.0+
/*
 * Xiongmai XM5xx: timer, reset and DRAM size, common to the family.
 */

#include <common.h>
#include <cpu_func.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/xm.h>
#include <mmc.h>
#include <sdhci.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Runs before DDR, from flash, on the SRAM stack: no global data, no .data
 * or .bss writes. The SP804 is started here so the DDR bring-up can wait on
 * it; lib/time.c reads the same counter (CONFIG_SYS_TIMER_COUNTER) later.
 */
static void xm_timer_start(void)
{
	writel(0, XM_TIMER_CTRL);
	writel(0xffffffff, XM_TIMER_LOAD);
	writel(XM_TIMER_CTRL_RUN, XM_TIMER_CTRL);
}

void xm_early_udelay(unsigned int us)
{
	const u32 rate = XM_XCLK / XM_TIMER_PRESCALE;
	const u32 ticks = (u32)(((u64)us * rate + 999999) / 1000000) + 1;
	const u32 start = readl(XM_TIMER_VALUE);

	while (start - readl(XM_TIMER_VALUE) < ticks)
		;
}

void xm_soc_reset(void)
{
	writel(1, XM_SYS_SOFT_RSTEN);
	writel(0xca110000, XM_SYS_SOFT_RST);
	while (1)
		;
}

void xm_early_init(void)
{
	xm_timer_start();
	xm_pll_init();
	xm_ddr_init();
}

int timer_init(void)
{
	/* Already running since lowlevel_init; restart only if stopped */
	if (readl(XM_TIMER_CTRL) != XM_TIMER_CTRL_RUN)
		xm_timer_start();
	gd->arch.tbl = 0;
	gd->arch.tbu = 0;
	return 0;
}

void reset_cpu(void)
{
	xm_soc_reset();
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((long *)XM_DDR_BASE, XM_DDR_MAX);
	return 0;
}

#ifdef CONFIG_DISPLAY_CPUINFO
int print_cpuinfo(void)
{
	printf("CPU:   Xiongmai %s\n", CONFIG_PRODUCT_SOC);
	return 0;
}
#endif

#ifdef CONFIG_MMC_SDHCI
/*
 * SDIO0: what the vendor tree drives as "arasan" is a standard SDHCI with
 * a 50 MHz base clock, so the generic driver serves it. It is how SD-card
 * recovery (common/openipc.c) reaches the card.
 */
int board_mmc_init(struct bd_info *bis)
{
	static struct sdhci_host host;

#ifdef CONFIG_TARGET_XM510
	writel(readl(XM_GPIO5_MULT_USE_EN) | 0xf, XM_GPIO5_MULT_USE_EN);
#endif
	host.name = "xm-sdhci";
	host.ioaddr = (void *)XM_SDIO0_BASE;
	host.max_clk = 50000000;
	host.voltages = MMC_VDD_32_33 | MMC_VDD_33_34;
	return add_sdhci(&host, 50000000, 400000);
}
#endif
