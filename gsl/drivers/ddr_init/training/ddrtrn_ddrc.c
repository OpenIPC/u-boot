/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
 */

#include "ddrtrn_training.h"
#include "hal/ddrtrn_hal_context.h"

void ddrtrn_sref_cfg_save(struct ddrtrn_dmc_cfg_sref *cfg_sref)
{
	unsigned int i;
	for (i = 0; i < ddrtrn_hal_get_cur_phy_dmc_num(); i++)
		cfg_sref->val[i] = ddrtrn_reg_read(ddrtrn_hal_get_cur_dmc_addr(i) + DDR_DMC_CFG_SREF);
}

/* Restore DMC_CFG_SREF config */
void ddrtrn_sref_cfg_restore(const struct ddrtrn_dmc_cfg_sref *cfg_sref)
{
	unsigned int i;
	for (i = 0; i < ddrtrn_hal_get_cur_phy_dmc_num(); i++)
		ddrtrn_hal_dmc_set_sref_cfg(i, cfg_sref->val[i]);
}

#if defined(DDR_HW_TRAINING_CONFIG) || defined(DDR_DCC_TRAINING_CONFIG)
/* DDR hw/dcc training exit or enter auto self-refresh */
int ddrtrn_training_ctrl_easr(unsigned int sref_req)
{
	int result = 0;
	unsigned int i;
	if (ddrtrn_hal_get_cur_phy_dmc_num() > DDR_DMC_PER_PHY_NUM) {
		ddrtrn_error("loop upper limit cfg->dmc_num out of range");
		return -1;
	}
	for (i = 0; i < ddrtrn_hal_get_cur_phy_dmc_num(); i++)
		result += ddrtrn_hal_ddrc_easr(ddrtrn_hal_get_cur_dmc_addr(i), sref_req);
	return result;
}

void ddrtrn_training_restore_timing(struct ddrtrn_hal_timing *timing)
{
	unsigned int i;
	for (i = 0; i < ddrtrn_hal_get_cur_phy_dmc_num(); i++)
		ddrtrn_hal_set_timing(ddrtrn_hal_get_cur_dmc_addr(i), timing->val[i]);
}
#endif /* DDR_HW_TRAINING_CONFIG ||  DDR_DCC_TRAINING_CONFIG */