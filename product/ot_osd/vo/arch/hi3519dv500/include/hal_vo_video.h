/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#ifndef __HAL_VO_VIDEO_H__
#define __HAL_VO_VIDEO_H__

#include "hal_vo_video_comm.h"
#include "inner_vo.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#if vo_desc("UBOOT_VO")
td_void vo_hal_layer_set_cvfir_cfg(hal_disp_layer layer, td_u32 vratio, const vdp_v1_cvfir_cfg *cfg,
    const vo_zme_comm_pq_cfg *pq_cfg);
td_void vo_hal_layer_set_cvfir_coef(hal_disp_layer layer, const cvfir_coef *coef);
td_void hal_cbm_set_cbm_attr(hal_disp_layer layer, ot_vo_dev dev);
td_void hal_cbm_set_cbm_bkg(hal_cbmmix mixer, const hal_disp_bkcolor *bkg);
td_void hal_cbm_set_cbm_mixer_prio(ot_vo_layer layer, td_u8 prio, td_u8 mixer_id);
td_void hal_layer_set_layer_zme_info(hal_disp_layer layer, td_u32 width, td_u32 height,
    hal_disp_zme_outfmt zme_out_fmt);
#endif /* #if vo_desc("UBOOT_VO") */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* end of __HAL_VO_VIDEO_H__ */
