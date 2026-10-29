// SPDX-License-Identifier: GPL-2.0+
/*
 * The SD card on the SDHCI MSC of the Ingenic T40, as the vendor U-Boot
 * sets it up (drivers/mmc/jz_sdhci.c of OpenIPC/u-boot-t40), on mainline's
 * generic SDHCI core, PIO, without driver model.
 *
 * The card clock comes from the CPM: the vendor's set_clock writes twice
 * the card clock into the MSC clock divider and starts the controller's
 * clock (CLOCK_CONTROL = 7), then the core programs its divider relative to
 * the 50 MHz the vendor gave as the maximum, as the vendor core did.
 *
 * Copyright (c) 2016 Ingenic Semiconductor Co.,Ltd
 */

#include <common.h>
#include <malloc.h>
#include <mmc.h>
#include <sdhci.h>
#include <asm/arch/base.h>
#include <asm/arch/clk.h>

#define JZ_SDHCI_MAX_CLK	50000000
#define JZ_SDHCI_MIN_CLK	400000

static void jz_set_mmc_clk(struct sdhci_host *host, u32 div)
{
	/* the core passes its divider; the vendor's hook took the clock */
	unsigned int clock = host->mmc->clock;

	sdhci_writew(host, 0x7, SDHCI_CLOCK_CONTROL);
#ifdef CONFIG_JZ_MMC_MSC0
	clk_set_rate(MSC, clock * 2);
#endif
#ifdef CONFIG_JZ_MMC_MSC1
	clk_set_rate(MSC1, clock * 2);
#endif
}

static const struct sdhci_ops jz_sdhci_ops = {
	.set_clock	= jz_set_mmc_clk,
};

static int jz_sdhci_init(u32 regbase, int index)
{
	struct sdhci_host *host;

	host = calloc(1, sizeof(*host));
	if (!host) {
		printf("sdhci__host malloc fail!\n");
		return 1;
	}

	host->name = "MSC";
	host->ioaddr = (void *)regbase;
	host->quirks = SDHCI_QUIRK_NO_HISPD_BIT | SDHCI_QUIRK_BROKEN_VOLTAGE
		| SDHCI_QUIRK_WAIT_SEND_CMD;
	host->voltages = MMC_VDD_27_28 | MMC_VDD_28_29 | MMC_VDD_29_30 |
		MMC_VDD_30_31 | MMC_VDD_31_32 | MMC_VDD_32_33 | MMC_VDD_33_34 |
		MMC_VDD_165_195;
	host->ops = &jz_sdhci_ops;
	host->index = index;
	host->host_caps = MMC_MODE_4BIT;
	/* the divider counts from the vendor's 50 MHz, not the caps' base */
	host->max_clk = JZ_SDHCI_MAX_CLK;

	return add_sdhci(host, JZ_SDHCI_MAX_CLK, JZ_SDHCI_MIN_CLK);
}

void jz_mmc_init(void)
{
#ifdef CONFIG_JZ_MMC_MSC0
	jz_sdhci_init(MSC0_BASE, 0);
#endif
#ifdef CONFIG_JZ_MMC_MSC1
	jz_sdhci_init(MSC1_BASE, 1);
#endif
}
