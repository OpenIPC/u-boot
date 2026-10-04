/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2024. All rights reserved.
 */

#include "ddrtrn_api.h"
#include "ddrtrn_training.h"
#include "hal/ddrtrn_hal_context.h"
#ifdef DDR_LOW_POWER_CONFIG
void bsp_ddrtrn_enter_lp_mode(void)
{
	struct ddrtrn_hal_context ctx;
	struct ddrtrn_hal_phy_all phy_all;
	ddrtrn_hal_set_cfg_addr((uintptr_t)&ctx, (uintptr_t)&phy_all);
	ddrtrn_hal_cfg_init();
	ddrtrn_enter_lp_mode();
}

void bsp_ddrtrn_exit_lp_mode(void)
{
	struct ddrtrn_hal_context ctx;
	struct ddrtrn_hal_phy_all phy_all;
	ddrtrn_hal_set_cfg_addr((uintptr_t)&ctx, (uintptr_t)&phy_all);
	ddrtrn_hal_cfg_init();
	ddrtrn_exit_lp_mode();
}
#else
void bsp_ddrtrn_enter_lp_mode(void)
{
	return;
}

void bsp_ddrtrn_exit_lp_mode(void)
{
	return;
}
#endif
