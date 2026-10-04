/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#ifndef OT_COMMON_VO_GFX_H
#define OT_COMMON_VO_GFX_H

#include "ot_type.h"
#include "ot_common.h"
#include "ot_errno.h"
#include "ot_common_video.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

typedef enum {
    OT_VO_GFX_TYPE_ARGB1555 = 0,
    OT_VO_GFX_TYPE_BMP1555 = 1,

    OT_VO_GFX_TYPE_BUTT
} ot_vo_gfx_type;

typedef struct {
    ot_rect display_rect;
    td_phys_addr_t address;
    td_u32 stride;
    ot_vo_gfx_type type;
} ot_vo_gfx_attr;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* end of #ifndef OT_COMMON_VO_GFX_H */