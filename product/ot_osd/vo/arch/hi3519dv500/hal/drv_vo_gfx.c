/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "drv_vo_gfx.h"
#include "ot_common_vo_dev.h"

#include "drv_vo.h"

#if vo_desc("UBOOT_VO")

td_s32 vo_drv_check_gfx_id(ot_vo_layer gfx_layer)
{
    ot_unused(gfx_layer);
    return TD_SUCCESS;
}

#define VO_GFX_DISP_RECT_W_MAX_OF_GFXX 3840
#define VO_GFX_DISP_RECT_H_MAX_OF_GFXX 3840

#if vo_desc("vo gfx")

td_s32 vo_drv_get_hal_gfx_layer(ot_vo_layer gfx_layer, hal_disp_layer *hal_layer)
{
    switch (gfx_layer) {
        case OT_VO_LAYER_G0:
            *hal_layer = HAL_DISP_LAYER_GFX0;
            break;
        default:
            return OT_ERR_VO_INVALID_LAYER_ID;
    }

    return TD_SUCCESS;
}

td_s32 vou_drv_get_gfx_bind_dev(ot_vo_layer layer)
{
    ot_vo_dev bind_dev;
    if (layer == HAL_DISP_LAYER_GFX0) {
        bind_dev = VO_DEV_DHD0;
    } else {
        bind_dev = OT_INVALID_DEV;
    }
    return bind_dev;
}
#endif

#if vo_desc("gfx")

static td_void vo_drv_get_gfx_attr_max_rect(ot_vo_layer layer, td_u32 *width, td_u32 *height)
{
    *width = VO_GFX_DISP_RECT_W_MAX_OF_GFXX;
    *height = VO_GFX_DISP_RECT_H_MAX_OF_GFXX;
}

td_s32 vo_drv_check_gfx_attr_display_rect(ot_vo_layer layer, const ot_rect *rect)
{
    td_u32 max_width;
    td_u32 max_height;

    if ((layer >= OT_VO_LAYER_G0) && (layer <= OT_VO_LAYER_G0)) {
        vo_drv_get_gfx_attr_max_rect(layer, &max_width, &max_height);
        if ((rect->width > max_width) || (rect->height > max_height)) {
            vo_err_trace("gfx layer (%d) disp rect width(%d) height(%d) can't be larger than %dx%d!\n",
                layer, rect->width, rect->height,
                max_width, max_height);
            return OT_ERR_VO_ILLEGAL_PARAM;
        }
    } else {
        return OT_ERR_VO_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}
#endif
#endif /* #if vo_desc("UBOOT_VO") */
