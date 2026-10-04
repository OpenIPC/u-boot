/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#ifndef DRV_VO_H
#define DRV_VO_H

#include "drv_vo_comm.h"
#include "drv_vo_dev.h"
#include "drv_vo_layer.h"
#include "drv_vo_video.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */
#if vo_desc("UBOOT_VO")

#define VO_ADDR_ALIGN        16 /* bytes */
#define VO_STRIDE_ALIGN      16

/* PLL FREF 24MHz */
#define VO_PLL_FREF 24

/* PLL frac precision */
#define VO_FRAC_SHIFT 24
#define VO_PLL_FRAC_PREC (1 << 24)
#define VO_PLL_FOUTVCO_MIN 800
#define VO_PLL_FOUTVCO_MAX 3200
#define VO_594000_KHZ_CLK 594000
#define VO_297000_KHZ_CLK 297000
#define VO_148500_KHZ_CLK 148500
#define VO_136000_KHZ_CLK 136000
#define VO_90000_KHZ_CLK 90000
#define VO_45000_KHZ_CLK 45000
#define VO_12500_KHZ_CLK 12500
#define VO_4200_KHZ_CLK 4200
#define VO_1200_KHZ_CLK 1200

/* LCDMCLKDIV max ((75 / 1188) * 2^27 )(max 75Mhz) */
#define VO_LCDMCLK_DIV_MAX 8473341
#define VO_LCDMCLK_BASE_CLK 1188
#define VO_LCDMCLK_SHIFT 27
#define VO_LCDMCLK_MAX_FREQENCY 75

#define VO_SD_SRC_CLK 54000
#define VO_CLK_MHZ_UNIT 1000
#define VO_SD_DEV_DIV 4

#define VO_DEFAULT_PRE_DIV 1
#define VO_DEFAULT_DEV_DIV 1

/* vou mixer prio id */
typedef enum {
    VOU_MIX_PRIO0 = 0,
    VOU_MIX_PRIO1,
    VOU_MIX_PRIO2,
    VOU_MIX_PRIO3,
    VOU_MIX_PRIO4,
    VOU_MIX_BUTT
} vou_mix_prio;

#define VO_CSC_DEF_VAL      50
#define VO_CSC_LUMA_MAX     100
#define VO_CSC_LUMA_MIN     0
#define VO_CSC_CONT_MAX     100
#define VO_CSC_CONT_MIN     0
#define VO_CSC_HUE_MAX      100
#define VO_CSC_HUE_MIN      0
#define VO_CSC_SAT_MAX      100
#define VO_CSC_SAT_MIN      0

#define VO_VGA_SHARPEN_STRENGTH_MIN  0
#define VO_VGA_SHARPEN_STRENGTH_MAX  0xFF
#define VO_VGA_SHARPEN_STRENGTH_DEF   128
#define VO_VGA_GAIN_MAX                 0x3F
#define VO_CVBS_DAC_GC_DEF              0x3C

#define VO_DAC_CABLE_CTR_DEF            3

typedef struct {
    vo_dev_type dev_type; /* dev type */
} vo_dev_capability;

#define VO_RESO_MIN_WDT      32 /* minimal width of display resolution */
#define VO_RESO_MIN_HGT      32 /* minimal height of display resolution */

/* desc : device context, which contains device public attribute. */
typedef struct {
    vo_dev_capability dev_cap;
    td_bool vo_enable;
    td_bool config;
    ot_vo_pub_attr vou_attr;

    td_u32 layer_num;

    td_u32 max_width;
    td_u32 max_height;
} vo_drv_dev;

typedef struct {
    td_bool video_enable;
    td_bool video_config;

    td_s32 bind_dev;
    ot_vo_video_layer_attr vo_video_attr;
    ot_vo_csc csc;
    csc_coef_param csc_param;
} vo_drv_layer;

#if vo_desc("ctx")
vo_drv_dev *vo_drv_get_dev_ctx(ot_vo_dev vo_dev);
vo_drv_layer *vo_drv_get_layer_ctx(ot_vo_layer layer);
#endif

#if vo_desc("pub")
td_void vo_drv_set_clk_reverse(ot_vo_dev dev, td_bool reverse);

td_void vou_drv_def_layer_bind_dev(td_void);
td_void vo_drv_default_setting(td_void);
#endif

#endif /* #if vo_desc("UBOOT_VO") */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* end of DRV_VO_H */
