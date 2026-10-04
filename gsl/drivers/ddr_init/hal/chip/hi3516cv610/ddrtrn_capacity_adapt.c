/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
 */

#include "ddrtrn_capacity_adapt.h"
#include "ddrtrn_training.h"
#include "hal/ddrtrn_hal_context.h"

#ifdef DDR_CAPAT_ADAPT_CFG
static const struct ddrtrn_reg_val_conf chsel_remap_reg_val_phy0_lpddr4[] = {
	/* offset, val */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_MAP1, 0x1700},
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB1, 0x70050050},
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_MAP2, 0x01ff0040}, /* ddr start addr */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB2, 0x0},
	{DDR_REG_BASE_AXI + DDR_AXI_REGION1_MAP1, 0x0},
};

static const struct ddrtrn_reg_val_conf chsel_remap_reg_val_phy0_nonlpddr4[] = {
	/* offset, val */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_MAP1, 0x1700},
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB1, 0x70050088},
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_MAP2, 0x01ff0040}, /* ddr start addr */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB2, 0x0},
	{DDR_REG_BASE_AXI + DDR_AXI_REGION1_MAP1, 0x0},
};

static const struct ddrtrn_reg_val_conf chsel_remap_reg_val_phy1_lpddr4[] = {
	/* offset, val */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_MAP1, 0x1700},
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB1, 0x70050052},
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_MAP2, 0x01ff0010}, /* ddr start addr */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB2, 0x0},
	{DDR_REG_BASE_AXI + DDR_AXI_REGION1_MAP1, 0x0},
};

static const struct ddrtrn_reg_val_conf chsel_remap_reg_val_phy1_nonlpddr4[] = {
	/* offset, val */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_MAP1, 0x1700},
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB1, 0x70050089},
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_MAP2, 0x01ff0010}, /* ddr start addr */
	{DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB2, 0x0},
	{DDR_REG_BASE_AXI + DDR_AXI_REGION1_MAP1, 0x0},
};

void ddrtrn_chsel_remap_func(void)
{
#define PHY_ARR_NUM 2
	unsigned int num;
	unsigned int phy_idx = ddrtrn_hal_get_phy_id();
	unsigned int dram_type = ddrtrn_hal_get_cur_phy_dram_type();
	const struct ddrtrn_reg_val_conf *chsel_remap_reg;
	const struct ddrtrn_reg_val_conf *phy_lpddr4_arr[PHY_ARR_NUM];
	const struct ddrtrn_reg_val_conf *phy_nonlpddr4_arr[PHY_ARR_NUM];

	phy_lpddr4_arr[0] = &chsel_remap_reg_val_phy0_lpddr4[0];
	phy_lpddr4_arr[1] = &chsel_remap_reg_val_phy1_lpddr4[0];
	phy_nonlpddr4_arr[0] = &chsel_remap_reg_val_phy0_nonlpddr4[0];
	phy_nonlpddr4_arr[1] = &chsel_remap_reg_val_phy1_nonlpddr4[0];

	ddrtrn_hal_axi_special_intlv_en();

	num = sizeof(chsel_remap_reg_val_phy0_lpddr4) / sizeof(chsel_remap_reg_val_phy0_lpddr4[0]);
	chsel_remap_reg = (dram_type == PHY_DRAMCFG_TYPE_LPDDR4) ? phy_lpddr4_arr[phy_idx] : phy_nonlpddr4_arr[phy_idx];

	ddrtrn_reg_config(chsel_remap_reg, num);
}

#endif /* DDR_CAPAT_ADAPT_CFG */
