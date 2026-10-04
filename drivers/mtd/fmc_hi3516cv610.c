// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 HiSilicon (Shanghai) Technologies CO., LIMITED.
 *
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/platform.h>
#include <fmc_common.h>
#include "fmc_spi_ids.h"

#define REG_IO_BASE 0x10260000

#define REG_SPI_CLK		(REG_IO_BASE + 0x0010)
#define REG_SPI_CS0		(REG_IO_BASE + 0x0018)
#define REG_SPI_CS1		(REG_IO_BASE + 0x0028)
#define REG_SPI_MOSI_IO0	(REG_IO_BASE + 0x000C)
#define REG_SPI_MISO_IO1	(REG_IO_BASE + 0x001C)
#define REG_SPI_WP_IO2		(REG_IO_BASE + 0x0020)
#define REG_SPI_HOLD_IO3	(REG_IO_BASE + 0x0014)

void fmc_srst_init(void)
{
	unsigned int old_val;
	unsigned int regval;

	old_val = regval = readl(CRG_REG_BASE + REG_FMC_CRG);
	regval |= FMC_CLK_ENABLE;
	if (regval != old_val) {
		fmc_pr(DTR_DB, "\t|||*-setting fmc clk enable[%#x]%#x\n",
				REG_FMC_CRG, regval);
		writel(regval, (CRG_REG_BASE + REG_FMC_CRG));
	}
	old_val = regval = readl(CRG_REG_BASE + REG_FMC_CRG);
	regval &= ~FMC_SOFT_RST_REQ;
	if (regval != old_val) {
		fmc_pr(DTR_DB, "\t|||*-setting fmc srst[%#x]%#x\n",
			REG_FMC_CRG, regval);
		writel(regval, (CRG_REG_BASE + REG_FMC_CRG));
	}
}

static void hi3516cv610_spi_io_config(void)
{
	static unsigned int io_config_flag = 1;

	if (!io_config_flag)
		return;

#if (CONFIG_FMC_MAX_CS_NUM == 1)
	if (((readl(NFC_STATE_REG) >> NFC_STATE_BIT_OFF) & \
			NFC_STATA_MASK) == NFC_1_8_STATA) {
		writel(0x12b1, REG_SPI_CLK);
		writel(0x1131, REG_SPI_CS0);
		writel(0x1261, REG_SPI_MOSI_IO0);
		writel(0x1261, REG_SPI_MISO_IO1);
		writel(0x12e1, REG_SPI_WP_IO2);
		writel(0x1161, REG_SPI_HOLD_IO3);
	} else {
		writel(0x1291, REG_SPI_CLK);
		writel(0x1131, REG_SPI_CS0);
		writel(0x1261, REG_SPI_MOSI_IO0);
		writel(0x1261, REG_SPI_MISO_IO1);
		writel(0x12E1, REG_SPI_WP_IO2);
		writel(0x1161, REG_SPI_HOLD_IO3);
	}
#elif (CONFIG_FMC_MAX_CS_NUM == 2)
	if (((readl(NFC_STATE_REG) >> NFC_STATE_BIT_OFF) & \
			NFC_STATA_MASK) == NFC_1_8_STATA) {
		writel(0x1251, REG_SPI_CLK);
		writel(0x1131, REG_SPI_CS0);
		writel(0x1133, REG_SPI_CS1);
		writel(0x1231, REG_SPI_MOSI_IO0);
		writel(0x1231, REG_SPI_MISO_IO1);
		writel(0x12B1, REG_SPI_WP_IO2);
		writel(0x1131, REG_SPI_HOLD_IO3);
	} else {
		writel(0x1251, REG_SPI_CLK);
		writel(0x1131, REG_SPI_CS0);
		writel(0x1133, REG_SPI_CS1);
		writel(0x1231, REG_SPI_MOSI_IO0);
		writel(0x1231, REG_SPI_MISO_IO1);
		writel(0x12B1, REG_SPI_WP_IO2);
		writel(0x1131, REG_SPI_HOLD_IO3);
	}
#else
	fmc_pr(FMC_INFO, "\t|||*-Macro definition err:CONFIG_FMC_MAX_CS_NUM=%d\n", CONFIG_FMC_MAX_CS_NUM);
#endif
	io_config_flag = 0;
}

void fmc_set_fmc_system_clock(struct spi_op *op, int clk_en)
{
	unsigned int old_val;
	unsigned int regval;

	old_val = regval = readl(CRG_REG_BASE + REG_FMC_CRG);

	regval &= ~FMC_CLK_SEL_MASK;

	if (op && op->clock) {
		regval |= op->clock & FMC_CLK_SEL_MASK;
		fmc_pr(DTR_DB, "\t|||*-get the setting clock value: %#x\n",
				op->clock);
	} else {
		regval |= fmc_clk_sel(FMC_CLK_24M);	/* Default Clock */
		hi3516cv610_spi_io_config();
	}

	if (clk_en)
		regval |= FMC_CLK_ENABLE;
	else
		regval &= ~FMC_CLK_ENABLE;

	if (regval != old_val) {
		fmc_pr(DTR_DB, "\t|||*-setting system clock [%#x]%#x\n",
				REG_FMC_CRG, regval);
		writel(regval, (CRG_REG_BASE + REG_FMC_CRG));
	}
}

#define HZ_200M 200

void fmc_get_fmc_best_2x_clock(unsigned int *clock)
{
	int ix;
	unsigned int clk_reg;
	unsigned int clk_type;
	const char *str[] = {"12", "50", "75", "100"};
	int nfc_voltage_state = NFC_3_3_STATA;

	unsigned int sys_2x_clk[] = {
		clk_2x(24), fmc_clk_sel(FMC_CLK_24M),
		clk_2x(100),	fmc_clk_sel(FMC_CLK_100M),
		clk_2x(150),	fmc_clk_sel(FMC_CLK_150M),
		clk_2x(200),    fmc_clk_sel(FMC_CLK_200M),
		0,		0,
	};
	if (((readl(NFC_STATE_REG) >> NFC_STATE_BIT_OFF) & \
			NFC_STATA_MASK) == NFC_1_8_STATA) {
		nfc_voltage_state = NFC_1_8_STATA;
		fmc_pr(QE_DBG, "\t|||*-nfc voltage is 1.8v\n");
	}

	clk_type = FMC_CLK_24M;
	clk_reg = fmc_clk_sel(clk_type);
	fmc_pr(QE_DBG, "\t|||*-matching flash clock %d\n", *clock);
	for (ix = 0; sys_2x_clk[ix]; ix += _2B) {
		if (*clock < sys_2x_clk[ix])
			break;
		if (nfc_voltage_state == NFC_1_8_STATA && sys_2x_clk[ix] >= clk_2x(HZ_200M))
			break;
		clk_reg = sys_2x_clk[ix + 1];
		clk_type = get_fmc_clk_type(clk_reg);
		fmc_pr(QE_DBG, "\t||||-select system clock: %sMHz\n",
				str[clk_type]);
	}
#ifdef CONFIG_DTR_MODE_SUPPORT
	fmc_pr(DTR_DB, "best system clock for SDR.\n");
#endif
	fmc_pr(QE_DBG, "\t|||*-matched best system clock: %sMHz\n",
			str[clk_type]);
	*clock = clk_reg;
}

#ifdef CONFIG_DTR_MODE_SUPPORT
#define HZ_396M 396
void fmc_get_fmc_best_4x_clock(unsigned int *clock)
{
	int ix;
	unsigned int clk_reg;
	unsigned int clk_type;
	int nfc_voltage_state = NFC_3_3_STATA;
	const char *str[] = {"6", "25", "37.125", "49.5", "59.375", "66", "74.25", "99"};

	unsigned int sys_4x_clk[] = {
		clk_4x(24), fmc_clk_sel(FMC_CLK_24M),
		clk_4x(100),	fmc_clk_sel(FMC_CLK_100M),
		clk_4x(150),	fmc_clk_sel(FMC_CLK_150M),
		clk_4x(200),	fmc_clk_sel(FMC_CLK_200M),
		clk_4x(238),	fmc_clk_sel(FMC_CLK_238M),
		clk_4x(264),	fmc_clk_sel(FMC_CLK_264M),
		clk_4x(297),	fmc_clk_sel(FMC_CLK_297M),
		clk_4x(396),	fmc_clk_sel(FMC_CLK_396M),
		0,		0,
	};

	if (((readl(NFC_STATE_REG) >> NFC_STATE_BIT_OFF) & \
			NFC_STATA_MASK) == NFC_1_8_STATA) {
		nfc_voltage_state = NFC_1_8_STATA;
		fmc_pr(QE_DBG, "\t|||*-nfc voltage is 1.8v\n");
	}

	clk_type = FMC_CLK_24M;
	clk_reg = fmc_clk_sel(clk_type);
	fmc_pr(DTR_DB, "\t|||*-matching flash clock %d\n", *clock);
	for (ix = 0; (sys_4x_clk[ix] && ((ix + 1) < sizeof(sys_4x_clk))); ix += _2B) {
		if (nfc_voltage_state == NFC_1_8_STATA && sys_4x_clk[ix] >= clk_4x(HZ_396M))
			break;
		if (*clock < sys_4x_clk[ix])
			break;
		clk_reg = sys_4x_clk[ix + 1];
		clk_type = get_fmc_clk_type(clk_reg);
		fmc_pr(DTR_DB, "\t||||-select system clock: %sMHz\n",
			str[clk_type]);
	}
	fmc_pr(DTR_DB, "best system clock for DTR.\n");
	fmc_pr(DTR_DB, "\t|||*-matched best system clock: %sMHz\n",
		str[clk_type]);
	*clock = clk_reg;
}

#endif/* CONFIG_DTR_MODE_SUPPORT */
