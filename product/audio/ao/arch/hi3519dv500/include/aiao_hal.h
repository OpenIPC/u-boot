/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#ifndef __AIAO_HAL_H__
#define __AIAO_HAL_H__

#include "audio_common_aio.h"
#include "aiao_hal_common.h"

#define AUDIO_DEFAULT_CLK_SELECT AUDIO_CLK_SELECT_BASE

td_void aop_hal_buffer_saddr_reg_h(td_ulong aio_base, td_u32 chn_id, td_u32 value);
td_void aop_set_mask_private(td_u32 chn_id, td_ulong aio_base);
td_void sys_hal_aio_reset_sel(td_bool reset);
td_void sys_hal_aio_clk_en(td_bool clk_en);
td_void sys_hal_acodec_apb_clk_en(td_bool clk_en);
td_void sys_hal_acodec_reset_sel(td_bool reset);
td_s32 aiao_hal_set_clk_sel_private(audio_clk_select clk_sel, aiao_clk_info *clk_info);
td_s32 aop_get_bclk_id_private(td_void);
td_u8 aop_get_tx_sd_private(td_void);

#endif