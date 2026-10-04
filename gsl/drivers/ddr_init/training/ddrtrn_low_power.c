/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
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
@brief    DDR disable dxctl clkgated mclk gating on one PHY
 */
static void ddrtrn_disable_clkgated_mclk_by_phy(unsigned int base_phy, unsigned int phy_idx)
{
	unsigned int phy_block;
	unsigned int byte_num = ddrtrn_hal_get_phy_total_byte_num(phy_idx);
	unsigned int phy_block_num = byte_num / NUM_FOR_REMAINING;
	ddr_phy_dxnmiscctrl4_desc ddr_phy_dxnmiscctrl4_reg;

	for (phy_block = 0; phy_block < phy_block_num; phy_block++) {
		ddr_phy_dxnmiscctrl4_reg.val = ddrtrn_reg_read(base_phy + ddr_phy_dxnmiscctrl4(phy_block));
		ddr_phy_dxnmiscctrl4_reg.bits.dxctl_clkgated_mclk = 0;
		ddrtrn_reg_write(ddr_phy_dxnmiscctrl4_reg.val, base_phy + ddr_phy_dxnmiscctrl4(phy_block));
	}
}

/*
@brief    DDR disable dxctl clkgated mclk gating
 */
void ddrtrn_disable_clkgated_mclk(void)
{
	unsigned int i;
	unsigned int base_phy;
	unsigned int phy_num = ddrtrn_hal_get_phy_num();

	for (i = 0; i < phy_num; i++) {
		base_phy = ddrtrn_hal_get_phy_addr(i);
		ddrtrn_disable_clkgated_mclk_by_phy(base_phy, i);
	}
}

/**
@brief  DMC DDRPHY CKG_EN config.

@param      flag         flag to enable DMC DDRPHY CKG_EN.
                                    0: enable
                                    1: disable
**/
static void ddrtrn_dmc_ddrphy_ckg_en_cfg(unsigned int base_dmc, unsigned int flag)
{
	ddr_dmc_ddrphy_ckg_desc ddr_dmc_ddrphy_ckg_reg;

	ddr_dmc_ddrphy_ckg_reg.val = ddrtrn_reg_read(base_dmc + DDR_DMC_DDRPHY_CKG);
	ddr_dmc_ddrphy_ckg_reg.bits.acphy_cmd1t2t_ckg_en = flag;
	ddr_dmc_ddrphy_ckg_reg.bits.ddrphy_dficlk_ckg_en = flag;
	ddrtrn_reg_write(ddr_dmc_ddrphy_ckg_reg.val, base_dmc + DDR_DMC_DDRPHY_CKG);
}

/**
@brief  DMC DDRPHY AC_OE_EN config.

@param      flag         flag to enable DMC DDRPHY AC_OE_EN.
                                    0: enable
                                    1: disable
**/
static void ddrtrn_dmc_ddrphy_ac_io_oe_en_cfg(unsigned int base_dmc, unsigned int flag)
{
	ddr_dmc_ddrphy_ac_oe_desc ddr_dmc_ddrphy_ac_oe_reg;

	ddr_dmc_ddrphy_ac_oe_reg.val = ddrtrn_reg_read(base_dmc + DDR_DMC_DDRPHY_AC_OE);
	ddr_dmc_ddrphy_ac_oe_reg.bits.ddrphy_cs_io_oe_en = flag;
	ddr_dmc_ddrphy_ac_oe_reg.bits.ddrphy_cmd2t_io_oe_en = flag;
	ddrtrn_reg_write(ddr_dmc_ddrphy_ac_oe_reg.val, base_dmc + DDR_DMC_DDRPHY_AC_OE);
}

/**
@brief  DMC CKG_EN config.

@param      flag         flag to enable DMC DDRPHY AC_OE_EN.
                                    0: enable
                                    1: disable
**/
static void ddrtrn_dmc_ckg_en_cfg(unsigned int base_dmc, unsigned int flag)
{
	ddr_dmc_ckg_desc ddr_dmc_ckg_reg;

	ddr_dmc_ckg_reg.val = ddrtrn_reg_read(base_dmc + DDR_DMC_CKG);
	ddr_dmc_ckg_reg.bits.cmd_path_ckg_en = flag;
	ddr_dmc_ckg_reg.bits.dmc_aref_ckg_en = flag;
	ddr_dmc_ckg_reg.bits.dmc_pd_ckg_en = flag;
	ddrtrn_reg_write(ddr_dmc_ckg_reg.val, base_dmc + DDR_DMC_CKG);
}
/*
@brief    DDR enter low power mode
 */
void ddrtrn_enter_lp_mode(void)
{
	unsigned int i, j;
	unsigned int dmc_num, base_dmc;
	unsigned int phy_num = ddrtrn_hal_get_phy_num();

	for (i = 0; i < phy_num; i++) {
		ddrtrn_hal_set_phy_id(i);
		dmc_num = ddrtrn_hal_get_cur_phy_dmc_num();
		for (j = 0; j < dmc_num; j++) {
			base_dmc = ddrtrn_hal_get_cur_dmc_addr(j);
			/* DMC DDRPHY CKG disable */
			ddrtrn_dmc_ddrphy_ckg_en_cfg(base_dmc, DMC_DDRPHY_CKG_DISABLE);
			/* DMC DDRPHY AC IO OE disable */
			ddrtrn_dmc_ddrphy_ac_io_oe_en_cfg(base_dmc, DMC_DDRPHY_AC_IO_OE_DISABLE);
			/* DMC CKG disable */
			ddrtrn_dmc_ckg_en_cfg(base_dmc, DMC_CKG_DISABLE);
		}
	}
}

/*
@brief    DDR exit low power mode
 */
void ddrtrn_exit_lp_mode(void)
{
	unsigned int i, j;
	unsigned int dmc_num, base_dmc;
	unsigned int phy_num = ddrtrn_hal_get_phy_num();

	for (i = 0; i < phy_num; i++) {
		ddrtrn_hal_set_phy_id(i);
		dmc_num = ddrtrn_hal_get_cur_phy_dmc_num();
		for (j = 0; j < dmc_num; j++) {
			base_dmc = ddrtrn_hal_get_cur_dmc_addr(j);
			/* DMC DDRPHY CKG enable */
			ddrtrn_dmc_ddrphy_ckg_en_cfg(base_dmc, DMC_DDRPHY_CKG_ENABLE);
			/* DMC DDRPHY AC IO OE enable */
			ddrtrn_dmc_ddrphy_ac_io_oe_en_cfg(base_dmc, DMC_DDRPHY_AC_IO_OE_ENABLE);
			/* DMC CKG enable */
			ddrtrn_dmc_ckg_en_cfg(base_dmc, DMC_CKG_ENABLE);
		}
	}
}
#endif