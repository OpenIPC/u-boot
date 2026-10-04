/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
 */

#ifndef DDR_API_H
#define DDR_API_H

enum ddrtrn_training_item {
	DDR_HW_TRAINING = 0,
	DDR_SW_TRAINING,
	DDR_PCODE_TRAINING,
	DDR_AC_TRAINING,
	DDR_ADJUST_TRAINING,
	DDR_DATAEYE_TRAINING,
	DDR_DCC_TRAINING,
	DDR_GATE_TRAINING,
	DDR_MPR_TRAINING,
	DDR_LPCA_TRAINING,
	DDR_VREF_TRAINING,
	DDR_WL_TRAINING,
	DDR_MAX_TRAINING
};

int bsp_ddrtrn_resume(void);
int bsp_ddrtrn_suspend(void);
int ddrtrn_training(unsigned int hw_item_mask, unsigned int sw_item_mask, int low_freq_flag, unsigned int is_dvfs);
int ddrtrn_start(unsigned int *ddr_size);
void bsp_ddrtrn_enter_lp_mode(void);
void bsp_ddrtrn_exit_lp_mode(void);
#endif
