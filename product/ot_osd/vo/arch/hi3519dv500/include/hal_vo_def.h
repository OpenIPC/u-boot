/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#ifndef __HAL_VO_DEF_H__
#define __HAL_VO_DEF_H__

#include "ot_type.h"
#include "ot_defines.h"
#include "hal_vo_def_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#if vo_desc("UBOOT_VO")

#define VHD_REGS_LEN        0x1000 /* len of V0's regs */
#define GFX_REGS_LEN        0x800
#define DHD_REGS_LEN        0x1000
#define INTF_REGS_LEN       0x100
#define VID_REGS_LEN        0x200 /* len of VID regs */
#define GRF_REGS_LEN        0x200 /* len of GFX regs */

/*
 * offset define
 * 0x200 bytes, 0x200/4 regs
 */
#define FDR_VID_OFFSET      (0x200 / 4)
#define ZME_HPREC           (1 << 20)
#define ZME_VPREC           (1 << 12)

#define VO_INPUT_BIT_WIDTH_10  10
#define VO_INPUT_BIT_WIDTH_8   8

#define VO_OUTPUT_BIT_WIDTH_10 10
#define VO_OUTPUT_BIT_WIDTH_8  8
#define VO_OUTPUT_BIT_WIDTH_6  6

typedef enum {
    VO_DEV_DHD0 = 0, /* ultra high definition device */
    VO_DEV_BUTT
} vo_hal_dev;

typedef enum {
    VO_HAL_LAYER_VHD0 = 0, /* V0 layer */

    VO_HAL_LAYER_BUTT
} vo_hal_layer;

typedef enum {
    VO_SW_LAYER_VHD0 = 0,

    VOU_SW_LAYER_BUTT
} vo_sw_layer;

typedef enum {
    HAL_DISP_LAYER_VHD0 = 0,

    HAL_DISP_LAYER_GFX0 = 1,
    HAL_DISP_LAYER_BUTT,
} hal_disp_layer;

#define DEV_PHY_START       VO_DEV_DHD0
#define DEV_PHY_END         VO_DEV_DHD0

#define LAYER_VID_START     HAL_DISP_LAYER_VHD0  /* VHD0 */
#define LAYER_VID_END       HAL_DISP_LAYER_VHD0  /* VHD0 */
#define LAYER_PHY_END       VO_SW_LAYER_VHD0

#define LAYER_GFX_START     HAL_DISP_LAYER_GFX0  /* GFX0 */
#define LAYER_GFX_END       HAL_DISP_LAYER_GFX0  /* GFX0 */

typedef struct {
    td_u32 dither_sed_y0;
    td_u32 dither_sed_u0;
    td_u32 dither_sed_v0;
    td_u32 dither_sed_w0;

    td_u32 dither_sed_y1;
    td_u32 dither_sed_u1;
    td_u32 dither_sed_v1;
    td_u32 dither_sed_w1;

    td_u32 dither_sed_y2;
    td_u32 dither_sed_u2;
    td_u32 dither_sed_v2;
    td_u32 dither_sed_w2;

    td_u32 dither_sed_y3;
    td_u32 dither_sed_u3;
    td_u32 dither_sed_v3;
    td_u32 dither_sed_w3;
} vo_dihter_sed;

typedef enum {
    DITHER_IO_MODE_12_10 = 1,
    DITHER_IO_MODE_12_8 = 2, /* for rgb24bit */
    DITHER_IO_MODE_10_8 = 3,
    DITHER_IO_MODE_10_6 = 4, /* for rgb16bit, rgb18bit */
    DITHER_IO_MODE_BUTT
} dither_io_mode;

typedef enum {
    DITHER_MODE_10BIT = 0,
    DITHER_MODE_8BIT = 1,
    DITHER_MODE_BUTT
} dither_mode;

typedef enum {
    DITHER_OWIDTH_MODE_5BIT = 0,
    DITHER_OWIDTH_MODE_6BIT = 1,
    DITHER_OWIDTH_MODE_7BIT = 2,
    DITHER_OWIDTH_MODE_8BIT = 3,
    DITHER_OWIDTH_MODE_9BIT = 4,
    DITHER_OWIDTH_MODE_10BIT = 5,
    DITHER_OWIDTH_MODE_BUTT
} dither_owidth_mode;

typedef enum {
    DITHER_IWIDTH_MODE_8BIT = 0,
    DITHER_IWIDTH_MODE_9BIT = 1,
    DITHER_IWIDTH_MODE_10BIT = 2,
    DITHER_IWIDTH_MODE_11BIT = 3,
    DITHER_IWIDTH_MODE_12BIT = 4,
    DITHER_IWIDTH_MODE_BUTT
} dither_iwidth_mode;

typedef struct {
    dither_io_mode io_mode;

    td_u32 dither_en;
    td_u32 dither_mode;
    td_u32 dither_round;
    td_u32 dither_round_unlim;
    td_u32 i_data_width_dither;
    td_u32 o_data_width_dither;
    td_u32 dither_domain_mode;
    td_u32 dither_tap_mode;
    vo_dihter_sed dither_sed;
    td_u32 dither_thr_max;
    td_u32 dither_thr_min;
} vdp_dither_cfg;

#endif /* #if vo_desc("UBOOT_VO") */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */
#endif /* end of __HAL_VO_DEF_H__ */
