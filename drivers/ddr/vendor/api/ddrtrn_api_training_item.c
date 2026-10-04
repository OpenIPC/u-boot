/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "ddrtrn_api.h"
#include "ddrtrn_interface.h"
#include "ddrtrn_training.h"
#include "hal/ddrtrn_hal_context.h"

int bsp_ddrtrn_training_item(unsigned int hw_item_mask, unsigned int sw_item_mask, int low_freq_flag)
{
	struct ddrtrn_hal_context ctx;
	struct ddrtrn_hal_phy_all phy_all;
	ddrtrn_hal_set_cfg_addr((uintptr_t)&ctx, (uintptr_t)&phy_all);
	ddrtrn_hal_cfg_init();
	if (ddrtrn_hw_training_init(hw_item_mask, low_freq_flag) == -1 ||
		ddrtrn_sw_training_if(sw_item_mask) == -1) {
		return -1;
	}
	return 0;
}

int bsp_ddrtrn_post_config(void)
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
	return 0;
}
