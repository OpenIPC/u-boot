/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
 */

#include "ddrtrn_api.h"
#include "ddrtrn_interface.h"
#include "ddrtrn_training.h"
#include "hal/ddrtrn_hal_context.h"

/**
  @brief DDR training.

  @param hw_item_mask    hw item mask, select hardware training type to be done.
  @param sw_item_mask    sw item mask, select software training type to be done.
  @param low_freq_flag   Whether to enable low frequency start.
  @param is_dvfs         Whether to enable dvfs.

  @retval -1    Error occur.
  @retval  0    Normally return.
**/
int ddrtrn_training(unsigned int hw_item_mask, unsigned int sw_item_mask, int low_freq_flag, unsigned int is_dvfs)
{
	unused(is_dvfs);
#ifdef DDR_ZQ_TRIM_CONFIG
	ddrtrn_zq_res_trim();
#endif
	ddrtrn_hal_sw_item_cfg(sw_item_mask);
	if (ddrtrn_hw_training_init(hw_item_mask, low_freq_flag) == -1 || ddrtrn_dcc_training_if() == -1) {
		return -1;
	}
	return 0;
}

/**
  @brief DDR post config flow.

  @param[out] ddr_size  DDR capacity, unit is MB.
**/
static void ddrtrn_post_cfg(unsigned int *ddr_size)
{
	struct ddrtrn_hal_context ctx;
	struct ddrtrn_hal_phy_all phy_all;
	ddrtrn_hal_set_cfg_addr((uintptr_t)&ctx, (uintptr_t)&phy_all);
	ddrtrn_hal_cfg_init();
#ifdef DDR_RDQBDL_ADJUST_CONFIG
	/* Subtract the smallest of rdqbdl, rdqsbdl, rdmbdl */
	ddrtrn_rdqbdl_adjust_func();
#endif
#ifdef DDR_LOW_POWER_CONFIG
	ddrtrn_disable_clkgated_mclk();
#endif
#ifdef DDR_ANTI_AGING_CONFIG
	ddrtrn_anti_aging_enable();
#endif
#ifdef DDR_RETRAIN_CONFIG
	ddrtrn_retrain_enable();
#endif
#ifdef DDR_AUTO_PD_CONFIG
	ddrtrn_dmc_auto_power_down_cfg();
#endif
	*ddr_size = 0;
#ifdef DDR_CAPAT_ADAPT_CFG
	*ddr_size = ddrtrn_capat_adapt();
#endif /* DDR_CAPAT_ADAPT_CFG */
}

int ddrtrn_start(unsigned int *ddr_size)
{
	int ret;
	struct ddrtrn_hal_context ctx;
	struct ddrtrn_hal_phy_all phy_all;

	ddrtrn_hal_set_cfg_addr((uintptr_t)&ctx, (uintptr_t)&phy_all);
	ddrtrn_hal_cfg_init();

	ret = ddrtrn_training(0xffffffff, 0x0, 0x1, 0x0);
	ddrtrn_post_cfg(ddr_size);

	return ret;
}
