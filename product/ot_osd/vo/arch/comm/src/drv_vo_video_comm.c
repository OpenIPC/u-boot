/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include <common.h>
#include "drv_vo.h"
#include "drv_vo_video.h"
#include "drv_vo_coef_org_comm.h"
#include "hal_vo_layer_comm.h"
#include "hal_vo_video_comm.h"
#include "hal_vo.h"
#include "vo.h"
#include "ot_math.h"
#include "securec.h"
#include "drv_vo_video_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#if vo_desc("UBOOT_VO")
#if vo_desc("video")
td_void hal_video_set_layer_ck_gt_en(hal_disp_layer layer, td_bool ck_gt_en)
{
    volatile reg_vdp_regs *vo_reg = vo_hal_get_reg();
    volatile reg_voctrl voctrl;
    /* only v0 support */
    if (layer > HAL_DISP_LAYER_VHD0) {
        return;
    }
    voctrl.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)&(vo_reg->voctrl.u32));
    voctrl.bits.v0_ck_gt_en = ck_gt_en;
    hal_write_reg((td_u32 *)(td_uintptr_t)&(vo_reg->voctrl.u32), voctrl.u32);
}

td_void vou_drv_init_default_csc(ot_vo_layer layer)
{
    ot_vo_csc *csc = TD_NULL;
    vo_drv_layer *drv_layer_ctx = vo_drv_get_layer_ctx(layer);

    csc = &drv_layer_ctx->csc;
    csc->csc_matrix = OT_VO_CSC_MATRIX_BT601FULL_TO_BT601FULL;
    csc->luma = VO_CSC_DEF_VAL;
    csc->contrast = VO_CSC_DEF_VAL;
    csc->hue = VO_CSC_DEF_VAL;
    csc->saturation = VO_CSC_DEF_VAL;
    csc->ex_csc_en = TD_FALSE;
}

td_void vou_drv_init_default_csc_param(ot_vo_layer layer)
{
    vo_drv_layer *drv_layer_ctx = vo_drv_get_layer_ctx(layer);
    csc_coef_param *csc_param = &drv_layer_ctx->csc_param;

    csc_param->csc_scale2p = VO_LAYER_CSC_SCALE2P_DEF_VAL;
    csc_param->csc_clip_min = VO_LAYER_CSC_CLIP_MIN;
    csc_param->csc_clip_max = VO_LAYER_CSC_CLIP_MAX;
}

td_void vo_drv_set_layer_attr(ot_vo_layer layer, const ot_vo_video_layer_attr *vo_layer_attr)
{
    vo_drv_layer *drv_layer_ctx = TD_NULL;

    drv_layer_ctx = vo_drv_get_layer_ctx(layer);
    (td_void)memcpy_s(&drv_layer_ctx->vo_video_attr, sizeof(ot_vo_video_layer_attr), vo_layer_attr,
        sizeof(ot_vo_video_layer_attr));
    drv_layer_ctx->video_config = TD_TRUE;
}

td_void vou_drv_get_default_csc(ot_vo_layer layer, ot_vo_csc *csc)
{
    vo_drv_layer *drv_layer_ctx = vo_drv_get_layer_ctx(layer);
    (td_void)memcpy_s(csc, sizeof(ot_vo_csc), &drv_layer_ctx->csc, sizeof(ot_vo_csc));
}

td_void vo_drv_layer_set_zme_info(ot_vo_layer layer, td_u32 width, td_u32 height, hal_disp_zme_outfmt zme_out_fmt)
{
    hal_layer_set_layer_zme_info(layer, width, height, zme_out_fmt);
}

td_void vo_drv_disable_layer(ot_vo_layer layer)
{
    vo_drv_layer *drv_layer_ctx = vo_drv_get_layer_ctx(layer);

    drv_layer_ctx->video_enable = TD_FALSE;
}

td_s32 vou_drv_get_video_layer_bind_dev(ot_vo_layer layer)
{
    vo_drv_layer *layer_ctx = TD_NULL;
    layer_ctx = vo_drv_get_layer_ctx(layer);
    return layer_ctx->bind_dev;
}

#endif /* #if vo_desc("video") */

#if vo_desc("video open close")
static td_void video_layer_set_zme_cfg(unsigned int layer, const ot_rect *disp_rect)
{
    vo_video_layer_ip_cfg vid_cfg = {0};

    vid_cfg.csc_en = 0;
    vid_cfg.hfir_en = 1;
    vid_cfg.vid_iw = disp_rect->width;
    vid_cfg.vid_ih = disp_rect->height;

    vid_cfg.vid_ow = disp_rect->width;
    vid_cfg.vid_oh = disp_rect->height;
    vid_cfg.zme_en = TD_FALSE;
    vo_drv_video_set_zme_enable(layer, &vid_cfg);
}

td_void vo_drv_layer_open(ot_vo_layer layer_id, const ot_vo_video_layer_attr *video_attr)
{
    hal_disp_layer layer;
    ot_rect disp_rect = {0};
    td_ulong addr;
    td_u32 strd;

    disp_rect = video_attr->display_rect;
    addr = video_attr->address;
    strd = video_attr->stride;

    layer = vou_drv_convert_layer(layer_id);

    hal_layer_set_layer_data_fmt(layer, HAL_INPUTFMT_YCBCR_SEMIPLANAR_420);
    hal_video_set_layer_disp_rect(layer, &disp_rect);
    hal_video_set_layer_video_rect(layer, &disp_rect);
    hal_layer_set_layer_in_rect(layer, &disp_rect);
    hal_layer_set_layer_global_alpha(layer, VO_ALPHA_OPACITY);
    hal_layer_set_src_resolution(layer, &disp_rect);

    video_layer_set_zme_cfg(layer, &disp_rect);

    vo_drv_layer_set_zme_info(layer, disp_rect.width, disp_rect.height, HAL_DISP_ZME_OUTFMT422);

    hal_video_set_multi_area_lum_addr(layer, addr, strd);
    hal_video_set_multi_area_chm_addr(layer, addr + strd * vou_align(disp_rect.height, VO_STRIDE_ALIGN), strd);
    hal_layer_enable_layer(layer, TD_TRUE);
    hal_video_set_layer_ck_gt_en(layer, TD_TRUE);
    hal_layer_set_reg_up(layer);
}

#endif

#if vo_desc("video csc")

td_s32 vo_drv_get_csc_matrix(ot_vo_csc_matrix csc_matrix, const csc_coef **csc_tmp)
{
    *csc_tmp = vo_get_csc_coef(csc_matrix);
    if (*csc_tmp == TD_NULL) {
        return OT_ERR_VO_NULL_PTR;
    }
    return TD_SUCCESS;
}

td_void vo_drv_calculate_yuv2rgb(const hal_csc_value *csc_value, const csc_coef *csc_tmp, csc_coef *coef)
{
    td_s32 luma;
    td_s32 contrast;
    td_s32 hue;
    td_s32 satu;
    const td_s32 csc_value_times = 100;
    const td_s32 table_times = 1000;
    td_s32 square_cv_times = csc_value_times * csc_value_times;
    const td_s32 *cos_table = vo_get_cos_table();
    const td_s32 *sin_table = vo_get_sin_table();

    luma = csc_value->luma;
    contrast = csc_value->cont;
    hue = csc_value->hue;
    satu = csc_value->satu;

    coef->csc_coef00 = (contrast * csc_tmp->csc_coef00) / csc_value_times;
    coef->csc_coef01 = (contrast * satu * ((csc_tmp->csc_coef01 * cos_table[hue] - csc_tmp->csc_coef02 *
                                            sin_table[hue]) / table_times)) / square_cv_times;
    coef->csc_coef02 = (contrast * satu * ((csc_tmp->csc_coef01 * sin_table[hue] + csc_tmp->csc_coef02 *
                                            cos_table[hue]) / table_times)) / square_cv_times;
    coef->csc_coef10 = (contrast * csc_tmp->csc_coef10) / csc_value_times;
    coef->csc_coef11 = (contrast * satu * ((csc_tmp->csc_coef11 * cos_table[hue] - csc_tmp->csc_coef12 *
                                            sin_table[hue]) / table_times)) / square_cv_times;
    coef->csc_coef12 = (contrast * satu * ((csc_tmp->csc_coef11 * sin_table[hue] + csc_tmp->csc_coef12 *
                                            cos_table[hue]) / table_times)) / square_cv_times;
    coef->csc_coef20 = (contrast * csc_tmp->csc_coef20) / csc_value_times;
    coef->csc_coef21 = (contrast * satu * ((csc_tmp->csc_coef21 * cos_table[hue] - csc_tmp->csc_coef22 *
                                            sin_table[hue]) / table_times)) / square_cv_times;
    coef->csc_coef22 = (contrast * satu * ((csc_tmp->csc_coef21 * sin_table[hue] + csc_tmp->csc_coef22 *
                                            cos_table[hue]) / table_times)) / square_cv_times;
    coef->csc_in_dc0 += ((contrast != 0) ? (luma * 100 / contrast) : (luma * 100));  /* 100 : param */
}

td_void vo_drv_calculate_rgb2yuv(const hal_csc_value *csc_value, const csc_coef *csc_tmp, csc_coef *coef)
{
    td_s32 luma;
    td_s32 contrast;
    td_s32 hue;
    td_s32 satu;
    const td_s32 csc_value_times = 100;
    const td_s32 table_times = 1000;
    td_s32 square_cv_times = csc_value_times * csc_value_times;
    const td_s32 *cos_table = vo_get_cos_table();
    const td_s32 *sin_table = vo_get_sin_table();

    luma = csc_value->luma;
    contrast = csc_value->cont;
    hue = csc_value->hue;
    satu = csc_value->satu;

    coef->csc_coef00 = (contrast * csc_tmp->csc_coef00) / csc_value_times;
    coef->csc_coef01 = (contrast * csc_tmp->csc_coef01) / csc_value_times;
    coef->csc_coef02 = (contrast * csc_tmp->csc_coef02) / csc_value_times;
    coef->csc_coef10 = (contrast * satu * ((csc_tmp->csc_coef10 * cos_table[hue] + csc_tmp->csc_coef20 *
                                            sin_table[hue]) / table_times)) / square_cv_times;
    coef->csc_coef11 = (contrast * satu * ((csc_tmp->csc_coef11 * cos_table[hue] + csc_tmp->csc_coef21 *
                                            sin_table[hue]) / table_times)) / square_cv_times;
    coef->csc_coef12 = (contrast * satu * ((csc_tmp->csc_coef12 * cos_table[hue] + csc_tmp->csc_coef22 *
                                            sin_table[hue]) / table_times)) / square_cv_times;
    coef->csc_coef20 = (contrast * satu * ((csc_tmp->csc_coef20 * cos_table[hue] - csc_tmp->csc_coef10 *
                                            sin_table[hue]) / table_times)) / square_cv_times;
    coef->csc_coef21 = (contrast * satu * ((csc_tmp->csc_coef21 * cos_table[hue] - csc_tmp->csc_coef11 *
                                            sin_table[hue]) / table_times)) / square_cv_times;
    coef->csc_coef22 = (contrast * satu * ((csc_tmp->csc_coef22 * cos_table[hue] - csc_tmp->csc_coef12 *
                                            sin_table[hue]) / table_times)) / square_cv_times;
    coef->csc_out_dc0 += luma;
}

td_void vou_drv_calc_csc_matrix(const ot_vo_csc *csc, ot_vo_csc_matrix csc_matrix, csc_coef *coef)
{
    td_s32 ret;
    const csc_coef *csc_tmp = TD_NULL;
    hal_csc_value csc_value;

    if (csc->ex_csc_en == TD_FALSE) {
        csc_value.luma = (td_s32)csc->luma * 64 / 100 - 32; /* 64: -32~32 100  */
    } else {
        csc_value.luma = (td_s32)csc->luma * 256 / 100 - 128; /* 256: -128~128 128 100  */
    }

    csc_value.cont = ((td_s32)csc->contrast - 50) * 2 + 100; /* 50 2 100 */
    csc_value.hue = (td_s32)csc->hue * 60 / 100;             /* 60 100  */
    csc_value.satu = ((td_s32)csc->saturation - 50) * 2 + 100; /* 50 2 100  */

    ret = vo_drv_get_csc_matrix(csc_matrix, &csc_tmp);
    if (ret != TD_SUCCESS) {
        return;
    }

    coef->csc_in_dc0 = csc_tmp->csc_in_dc0;
    coef->csc_in_dc1 = csc_tmp->csc_in_dc1;
    coef->csc_in_dc2 = csc_tmp->csc_in_dc2;
    coef->csc_out_dc0 = csc_tmp->csc_out_dc0;
    coef->csc_out_dc1 = csc_tmp->csc_out_dc1;
    coef->csc_out_dc2 = csc_tmp->csc_out_dc2;

    if ((csc_matrix >= OT_VO_CSC_MATRIX_BT601LIMIT_TO_RGBFULL) &&
        (csc_matrix <= OT_VO_CSC_MATRIX_BT709FULL_TO_RGBLIMIT)) {
        vo_drv_calculate_yuv2rgb(&csc_value, csc_tmp, coef);
    } else {
        vo_drv_calculate_rgb2yuv(&csc_value, csc_tmp, coef);
    }
}

td_void vo_drv_clip_layer_csc_coef(csc_coef *coef)
{
    td_s32 min_coef = 0;
    td_s32 max_coef = 0;

    vo_drv_get_layer_csc_coef_range(&min_coef, &max_coef);
    coef->csc_coef00 = clip3(coef->csc_coef00, min_coef, max_coef);
    coef->csc_coef01 = clip3(coef->csc_coef01, min_coef, max_coef);
    coef->csc_coef02 = clip3(coef->csc_coef02, min_coef, max_coef);

    coef->csc_coef10 = clip3(coef->csc_coef10, min_coef, max_coef);
    coef->csc_coef11 = clip3(coef->csc_coef11, min_coef, max_coef);
    coef->csc_coef12 = clip3(coef->csc_coef12, min_coef, max_coef);

    coef->csc_coef20 = clip3(coef->csc_coef20, min_coef, max_coef);
    coef->csc_coef21 = clip3(coef->csc_coef21, min_coef, max_coef);
    coef->csc_coef22 = clip3(coef->csc_coef22, min_coef, max_coef);
}

td_void vo_drv_layer_get_csc_param(ot_vo_layer layer, csc_coef_param *csc_param)
{
    vo_drv_layer *drv_layer_ctx = vo_drv_get_layer_ctx(layer);
    (td_void)memcpy_s(csc_param, sizeof(csc_coef_param), &(drv_layer_ctx->csc_param),
        sizeof(csc_coef_param));
}

td_void vou_drv_layer_csc_config(ot_vo_layer layer, const ot_vo_csc *csc)
{
    csc_coef coef = {0};
    csc_coef_param csc_param = {0};

    vou_drv_calc_csc_matrix(csc, csc->csc_matrix, &coef);

    vo_drv_layer_get_csc_param(layer, &csc_param);

    vo_drv_csc_trans_to_register(&coef);

    vo_drv_clip_layer_csc_coef(&coef);
    hal_layer_set_csc_coef(layer, &coef, &csc_param);
}

#endif

#endif /* #if vo_desc("UBOOT_VO") */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */
