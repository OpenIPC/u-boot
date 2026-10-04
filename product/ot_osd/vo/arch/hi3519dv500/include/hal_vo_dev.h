/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#ifndef __HAL_VO_DEV_H__
#define __HAL_VO_DEV_H__

#include "hal_vo_dev_comm.h"
#include "inner_vo.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#if vo_desc("UBOOT_VO")

td_void hal_sys_set_outstanding(td_void);

#if vo_desc("dev intf")
#define VO_CVBS_DATE_PAL 0x628412dc
#define VO_CVBS_DATE_NTSC 0x108412dc
#define VO_CVBS_DATE_GAIN 0x8080958a
td_void vo_hal_intf_set_cvbs_dac_cfg(td_void);
td_void vo_hal_intf_set_date_cvbs_gain(td_u32 data);
td_void vo_hal_intf_set_date_cvbs_burst_start(td_void);
td_void vo_hal_intf_set_rgb_sync_inv(const hal_disp_syncinv *inv);
td_void vo_hal_intf_set_mipi_sync_inv(const hal_disp_syncinv *inv);
td_void vo_hal_intf_set_mux_sel(ot_vo_dev dev, ot_vo_intf_type intf);
td_void vo_hal_intf_set_csc_enable(ot_vo_intf_type intf, td_bool enable);
td_void vo_hal_intf_set_csc_cfg(ot_vo_intf_type intf, const csc_coef *csc_cfg);
td_void vo_hal_set_intf_rgb_component_order(td_bool component_inverse_en);
td_void vo_hal_set_intf_rgb_bit_inverse(td_bool bit_inverse_en);
td_void hal_disp_set_lcd_serial_perd(td_u32 serial_perd);
td_void vo_hal_set_intf_rgb_dummy_out(td_bool dummy_out_en);
td_void vo_hal_set_intf_ctrl(ot_vo_intf_type intf, const td_u32 *ctrl_info);
td_void vo_hal_set_dev_precharge_threshold(ot_vo_dev dev, td_bool te_enable);
td_void vo_hal_intf_set_lcd_dither(const vdp_dither_cfg *cfg);
#endif
#if vo_desc("dev")
td_void vo_hal_set_dev_full_frame_rate(ot_vo_dev dev, td_u32 frame_rate);
#ifdef CONFIG_OT_VO_INTF_GAMMA
td_void vo_hal_debug_get_gamma_para(td_void);
td_void vo_hal_set_gamma_disable(td_void);
td_void vo_hal_set_intf_gamma_para(td_bool gamma_enable);
#endif
#endif
#endif /* #if vo_desc("UBOOT_VO") */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* end of __HAL_VO_DEV_H__ */
