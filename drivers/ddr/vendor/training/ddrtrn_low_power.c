/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2024. All rights reserved.
 */

#include "ddrtrn_training.h"
#include "hal/ddrtrn_hal_context.h"

#ifdef DDR_LOW_POWER_CONFIG

#define DMC_DDRPHY_CKG_ENABLE  0
#define DMC_DDRPHY_CKG_DISABLE 1
#define DMC_DDRPHY_AC_IO_OE_ENABLE  0
#define DMC_DDRPHY_AC_IO_OE_DISABLE 1
#define DMC_CKG_ENABLE  0
#define DMC_CKG_DISABLE 1

/*
@brief    DDR disable dxctl clkgated mclk gating
 */
void ddrtrn_disable_clkgated_mclk(void)
{
	unsigned int i, j, base_phy, byte_num, phy_block_num;
	unsigned int phy_num = ddrtrn_hal_get_phy_num();

	for (i = 0; i < phy_num; i++) {
		base_phy = ddrtrn_hal_get_phy_addr(i);
		byte_num = ddrtrn_hal_get_phy_total_byte_num(i);
		phy_block_num = byte_num / NUM_FOR_REMAINING;
		for (j = 0; j < phy_block_num; j++) {
			ddrtrn_hal_disable_clkgated_mclk(base_phy, j);
		}
	}
}

/**
@brief  disable clk
            DMC DDRPHY CKG
            DMC DDRPHY AC IO OE
            DMC CKG
 
@param      base_dmc         dmc address
**/
static void ddrtrn_enter_lp_mode_process(unsigned int base_dmc)
{
	/* DMC DDRPHY CKG disable */
	ddrtrn_hal_dmc_ddrphy_ckg_en_cfg(base_dmc, DMC_DDRPHY_CKG_DISABLE);
	/* DMC DDRPHY AC IO OE disable */
	ddrtrn_hal_dmc_ddrphy_ac_io_oe_en_cfg(base_dmc, DMC_DDRPHY_AC_IO_OE_DISABLE);
	/* DMC CKG disable */
	ddrtrn_hal_dmc_ckg_en_cfg(base_dmc, DMC_CKG_DISABLE);
}

/*
@brief    DDR enter low power mode
 */
void ddrtrn_enter_lp_mode(void)
{
	unsigned int i, j, dmc_num, base_dmc;
	unsigned int phy_num = ddrtrn_hal_get_phy_num();

	for (i = 0; i < phy_num; i++) {
		dmc_num = ddrtrn_hal_get_phy_dmc_num(i);
		for (j = 0; j < dmc_num; j++) {
			base_dmc = ddrtrn_hal_get_phy_dmc_addr(i, j);
			ddrtrn_enter_lp_mode_process(base_dmc);
		}
	}
}

/**
@brief  enable clk
            DMC DDRPHY CKG
            DMC DDRPHY AC IO OE
            DMC CKG
 
@param      base_dmc         dmc address
**/
static void ddrtrn_exit_lp_mode_process(unsigned int base_dmc)
{
	/* DMC DDRPHY CKG enable */
	ddrtrn_hal_dmc_ddrphy_ckg_en_cfg(base_dmc, DMC_DDRPHY_CKG_ENABLE);
	/* DMC DDRPHY AC IO OE enable */
	ddrtrn_hal_dmc_ddrphy_ac_io_oe_en_cfg(base_dmc, DMC_DDRPHY_AC_IO_OE_ENABLE);
	/* DMC CKG enable */
	ddrtrn_hal_dmc_ckg_en_cfg(base_dmc, DMC_CKG_ENABLE);
}

/*
@brief    DDR exit low power mode
 */
void ddrtrn_exit_lp_mode(void)
{
	unsigned int i, j, dmc_num, base_dmc;
	unsigned int phy_num = ddrtrn_hal_get_phy_num();

	for (i = 0; i < phy_num; i++) {
		dmc_num = ddrtrn_hal_get_phy_dmc_num(i);
		for (j = 0; j < dmc_num; j++) {
			base_dmc = ddrtrn_hal_get_phy_dmc_addr(i, j);
			ddrtrn_exit_lp_mode_process(base_dmc);
		}
	}
}
#endif