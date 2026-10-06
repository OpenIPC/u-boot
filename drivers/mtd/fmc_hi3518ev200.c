/*
 * fmc_hi3518ev200.c
 *
 * The Flash Memory Controller v100 Device Driver for hisilicon
 *
 * Copyright (c) 2016 HiSilicon Technologies Co., Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/platform.h>
#include <fmc_common.h>

#include "fmc_spi_ids.h"

/*****************************************************************************/
void fmc_srst_init(void)
{
	unsigned int old_val, regval;

	old_val = regval = readl(CRG_REG_BASE + FMC_CRG48);
	regval |= FMC_CRG48_CLK_EN;
	if (regval != old_val)
		writel(regval, (CRG_REG_BASE + FMC_CRG48));

	old_val = regval = readl(CRG_REG_BASE + FMC_CRG48);
	regval &= ~FMC_CRG48_SOFT_RST_REQ;
	if (regval != old_val)
		writel(regval, (CRG_REG_BASE + FMC_CRG48));
}

/*****************************************************************************/
void fmc_set_fmc_system_clock(struct spi_op *op, int clk_en)
{
	unsigned int old_val, regval;
	unsigned int eco_reserv, mux_io_reg, mux_io_tmp;
	const char *str[] = {"12", "99"};

	old_val = regval = readl(CRG_REG_BASE + FMC_CRG48);

	regval &= ~FMC_CLK_SEL_MASK;

	if (op && op->clock) {
		regval |= op->clock & FMC_CLK_SEL_MASK;
		fmc_pr(DTR_DB, "\t|||*-get the setting clock value: %#x\n",
			op->clock);
	} else {
		eco_reserv = readl(CRG_REG_BASE + FMC_CRG65);
		mux_io_reg = GET_FMC_MUXIO_CLK(eco_reserv);

		/* reset the mux_IO to 24MHz if mux_IO was set to 99MHz */
		if (mux_io_reg) {
			fmc_pr(BT_DBG, "\t||-get mux_IO clock: %sMHz\n",
				str[mux_io_reg]);
			mux_io_tmp = FMC_MUXIO_CLK_SEL(mux_io_reg);
			mux_io_reg = GET_FMC_MUXIO_CLK(mux_io_tmp);
			writel(mux_io_reg, (CRG_REG_BASE + FMC_CRG65));
			fmc_pr(BT_DBG, "\t||-set mux_IO clock: %sMHz\n",
				str[mux_io_reg]);
		}
		regval |= FMC_CLK_SEL_24M;	/* Default Clock */
	}
	if (clk_en)
		regval |= FMC_CRG48_CLK_EN;
	else
		regval &= ~FMC_CRG48_CLK_EN;

	if (regval != old_val) {
		fmc_pr(DTR_DB, "\t|||*-setting system clock [%#x]%#x\n",
			FMC_CRG48, regval);
		writel(regval, (CRG_REG_BASE + FMC_CRG48));
	}
}

/*****************************************************************************/
void fmc_get_fmc_best_2x_clock(unsigned int *clock)
{
	int ix;
	unsigned int clk_reg, clk_type;
	const char *str[] = {"12", "74.25", "62.5", "99"};

#define CLK_2X(_clk)	(((_clk) + 1) >> 1)
	unsigned int sysclk[] = {
		CLK_2X(24),	FMC_CLK_SEL_24M,
		CLK_2X(125),	FMC_CLK_SEL_125M,
		CLK_2X(148),	FMC_CLK_SEL_148_5M,
		CLK_2X(198),	FMC_CLK_SEL_198M,
		0,		0,
	};
#undef CLK_2X

	clk_reg = FMC_CLK_SEL_24M;
	clk_type = GET_FMC_CLK_TYPE(clk_reg);
	fmc_pr(QE_DBG, "\t|*-matching flash clock %d\n", *clock);
	for (ix = 0; sysclk[ix]; ix += 2) {
		if (*clock < sysclk[ix])
			break;
		clk_reg = sysclk[ix + 1];
		clk_type = GET_FMC_CLK_TYPE(clk_reg);
		fmc_pr(QE_DBG, "\t||-select system clock: %sMHz\n",
			str[clk_type]);
	}

	fmc_pr(QE_DBG, "\t|*-matched best system clock: %sMHz\n",
		str[clk_type]);
	*clock = clk_reg;
}
