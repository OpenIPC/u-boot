/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
 */

#ifndef DDRTRN_CAPACITY_ADAPT_H
#define DDRTRN_CAPACITY_ADAPT_H

#include "ddrtrn_training.h"

struct ddr_capat_by_phy {
	unsigned int capat_low16bit;
	unsigned int capat_high16bit;
	unsigned int capacity;
};

struct ddr_capat_phy_all {
	struct ddr_capat_by_phy phy_capat[DDR_PHY_NUM];
	unsigned int ddr_capat_sum;
	unsigned int cur_phy_rank_num[DDR_PHY_NUM];
};

void ddrtrn_chsel_remap_func(void);
#endif /* DDRTRN_CAPACITY_ADAPT_H */
