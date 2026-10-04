/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "drv_vo.h"
#include "drv_vo_comm.h"
#include "drv_vo_dev.h"
#include "hal_vo_video.h"
#include "hal_vo_dev_comm.h"
#include "hal_vo_layer_comm.h"
#include "hal_vo_gfx_comm.h"
#include "hal_vo_dev.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#if vo_desc("UBOOT_VO")

#if vo_desc("pub")

static vo_drv_dev g_vo_drv_dev[OT_VO_MAX_DEV_NUM];
static vo_drv_layer g_vo_drv_layer[OT_VO_MAX_LAYER_NUM];

vo_drv_dev *vo_drv_get_dev_ctx(ot_vo_dev vo_dev)
{
    return &g_vo_drv_dev[vo_dev];
}

vo_drv_layer *vo_drv_get_layer_ctx(ot_vo_layer layer)
{
    return &g_vo_drv_layer[layer];
}

td_void vo_drv_set_clk_reverse(ot_vo_dev dev, td_bool reverse)
{
    ot_mpp_chn mpp_chn = {0};

    vo_drv_set_dev_mpp_chn(dev, &mpp_chn);
    call_sys_drv_ioctrl(&mpp_chn, SYS_VO_HD_CLKOUT_PHASIC_REVERSE_EN, &reverse);
}

td_void vou_drv_def_layer_bind_dev(td_void)
{
    vou_drv_set_layer_priority(VO_DEV_DHD0, OT_VO_LAYER_V0, VOU_MIX_PRIO0);
    vou_drv_set_layer_priority(VO_DEV_DHD0, OT_VO_LAYER_G0, VOU_MIX_PRIO1);

    hal_disp_set_reg_up(VO_DEV_DHD0);
    return;
}

td_void vo_drv_default_setting(td_void)
{
    td_u32 i;
    hfir_coef h_coef[LAYER_VID_END + 1] = {
        { 0x3f5, 0xf, 0x3ec, 0x1c, 0x3d8, 0x3d, 0x395, 0x14a },
    };

    /* set each video/graphic layer global alpha */
    for (i = LAYER_VID_START; i <= LAYER_GFX_END; i++) {
        hal_layer_set_layer_global_alpha(i, 255); /* global alpha max 255 */
        hal_video_set_layer_alpha(i, 255); /* alpha max 255 */
    }

    /* select graphic layer's alpha range(0: 0~128,1:0~255) */
    for (i = LAYER_GFX_START; i <= LAYER_GFX_END; i++) {
        hal_gfx_set_pixel_alpha_range(i, 1);
    }

    /* set video layer hfir enable */
    for (i = LAYER_VID_START; i <= LAYER_VID_END; i++) {
        hal_video_hfir_set_hfir_mode(i, HAL_HFIRMODE_COPY);
        hal_video_hfir_set_coef(i, &h_coef[i]);
        hal_video_hfir_set_mid_en(i, TD_TRUE);
    }

    /* set each cross bar default PRI */
    vou_drv_def_layer_bind_dev();

    /* outstanding */
    hal_sys_set_outstanding();

    hal_disp_clear_int_status(VO_INTREPORT_ALL);

    /* only set video layer */
    for (i = LAYER_VID_START; i <= LAYER_VID_END; i++) {
        vou_drv_layer_enable(i, TD_FALSE);
        vou_drv_set_layer_data_fmt(i, VOU_LAYER_PIXEL_FORMAT_SP_YCBCR_420);
    }
}

#endif
#endif /* #if vo_desc("UBOOT_VO") */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */
