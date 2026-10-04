/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */


#ifndef OT_COMMON_VO_H
#define OT_COMMON_VO_H

#include "ot_errno.h"
#include "ot_common_video.h"
#include "ot_common_vo_dev.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

typedef struct {
    ot_rect display_rect; /* RW; display resolution */
    td_phys_addr_t address;
    td_u32 stride;
    ot_pixel_format pixel_format;
} ot_vo_video_layer_attr;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* end of #ifndef OT_COMMON_VO_H */
