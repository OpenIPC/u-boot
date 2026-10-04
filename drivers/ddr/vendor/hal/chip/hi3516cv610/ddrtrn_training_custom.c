/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_training.h"

/*
 * Do some prepare before copy code from DDR to SRAM.
 * Keep empty when nothing to do.
 */
void ddrtrn_hal_cmd_prepare_copy(void)
{
	return;
}

/*
 * Save site before DDR training command execute .
 * Keep empty when nothing to do.
 */
void ddrtrn_hal_cmd_site_save(void)
{
	return;
}

/*
 * Restore site after DDR training command execute.
 * Keep empty when nothing to do.
 */
void ddrtrn_hal_cmd_site_restore(void)
{
	return;
}

static void ddrtrn_hal_anti_aging_disable(struct ddrtrn_hal_training_custom_reg *custom_reg,
	unsigned int base_phy, unsigned int phy_idx)
{
	/* disable rdqs anti-aging */
	custom_reg->age_compst_en[phy_idx] =
		ddrtrn_reg_read(base_phy + DDR_PHY_PHYRSCTRL);
	ddrtrn_reg_write(custom_reg->age_compst_en[phy_idx] &
		(~((unsigned int)0x1 << DDR_CFG_RX_AGE_COMPST_EN_BIT)),
		base_phy + DDR_PHY_PHYRSCTRL);
}

/*
 * Save site before DDR training:include boot and command execute.
 * Keep empty when nothing to do.
 */
int ddrtrn_hal_boot_cmd_save(struct ddrtrn_hal_training_custom_reg *custom_reg)
{
	if (custom_reg == NULL)
		return -1;

	custom_reg->ddrt_ctrl = ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + MISC_DDRT_CTRL);
	ddrtrn_reg_write(custom_reg->ddrt_ctrl | (0x1 << DDRT_MST_SEL_BIT), DDR_REG_BASE_SYSCTRL + MISC_DDRT_CTRL);
	/* turn on ddrt clock */
	custom_reg->ddrt_clk_reg = ddrtrn_reg_read(CRG_REG_BASE_ADDR + CRG_DDRT);
	/* enable ddrt0 clock */
	ddrtrn_reg_write(custom_reg->ddrt_clk_reg | (0x1 << DDR_TEST0_CKEN_BIT), CRG_REG_BASE_ADDR + CRG_DDRT);
	ddr_asm_nop();
	/* disable ddrt0 soft reset */
	ddrtrn_reg_write(ddrtrn_reg_read(CRG_REG_BASE_ADDR + CRG_DDRT) & (~(0x1 << DDR_TEST0_SRST_REQ_BIT)),
		CRG_REG_BASE_ADDR + CRG_DDRT);

	/* disable rdqs anti-aging */
	ddrtrn_hal_anti_aging_disable(custom_reg, DDR_REG_BASE_PHY0, 0); /* 0: phy0 */

	return 0;
}

/*
 * Restore site after DDR training:include boot and command execute.
 * Keep empty when nothing to do.
 */
void ddrtrn_hal_boot_cmd_restore(const struct ddrtrn_hal_training_custom_reg *custom_reg)
{
	if (custom_reg == NULL)
		return;

	ddrtrn_reg_write(custom_reg->ddrt_ctrl, DDR_REG_BASE_SYSCTRL + MISC_DDRT_CTRL);

	/* restore ddrt clock */
	ddrtrn_reg_write(custom_reg->ddrt_clk_reg, CRG_REG_BASE_ADDR + CRG_DDRT);

	/* restore rdqs anti-aging */
	ddrtrn_reg_write(custom_reg->age_compst_en[0], DDR_REG_BASE_PHY0 + DDR_PHY_PHYRSCTRL);
}
