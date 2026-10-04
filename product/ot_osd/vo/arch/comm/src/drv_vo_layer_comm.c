/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "drv_vo_layer_comm.h"
#include "drv_vo.h"
#include "hal_vo_layer_comm.h"
#include "hal_vo_video_comm.h"
#include "drv_vo_coef_org_comm.h"
#include "vo.h"
#include "ot_math.h"
#include "securec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#if vo_desc("UBOOT_VO")


#if vo_desc("layer cfg")

ot_vo_layer vo_drv_get_hw_layer(ot_vo_layer layer)
{
    return layer;
}

td_void vou_drv_layer_enable(ot_vo_layer layer, td_bool enable)
{
    ot_vo_layer hw_layer = vo_drv_get_hw_layer(layer);

    hal_video_hfir_set_ck_gt_en(hw_layer, enable);
    hal_layer_enable_layer(hw_layer, enable);
    hal_video_set_layer_ck_gt_en(layer, enable);
}

td_void vou_drv_set_layer_reg_up(ot_vo_layer layer)
{
    ot_vo_layer hw_layer = vo_drv_get_hw_layer(layer);
    hal_layer_set_reg_up(hw_layer);
}

td_void vou_drv_set_layer_data_fmt(ot_vo_layer layer, vou_layer_pixel_format data_fmt)
{
    ot_vo_layer hw_layer = vo_drv_get_hw_layer(layer);
    hal_disp_layer hal_layer;
    hal_disp_pixel_format disp_data_fmt;

    hal_layer = vou_drv_convert_layer(hw_layer);
    disp_data_fmt = vo_drv_convert_data_format(data_fmt);
    hal_layer_set_layer_data_fmt(hal_layer, disp_data_fmt);
}
#endif
#endif /* #if vo_desc("UBOOT_VO") */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */
