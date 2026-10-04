/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#ifndef __OT_DEFINES_H__
#define __OT_DEFINES_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/* For VO */
#define OT_VO_MAX_PHYS_DEV_NUM             1   /* max physical dev num */
    /* max dev num */
#define OT_VO_MAX_DEV_NUM                 (OT_VO_MAX_PHYS_DEV_NUM)

#define OT_VO_MAX_PHYS_VIDEO_LAYER_NUM     1  /* max physical video layer num */
#define OT_VO_MAX_GFX_LAYER_NUM            1  /* max graphic layer num */
    /* max physical layer num */
#define OT_VO_MAX_PHYS_LAYER_NUM           (OT_VO_MAX_PHYS_VIDEO_LAYER_NUM + OT_VO_MAX_GFX_LAYER_NUM)
    /* max layer num */
#define OT_VO_MAX_LAYER_NUM                OT_VO_MAX_PHYS_LAYER_NUM
#define OT_VO_MAX_LAYER_IN_DEV             1 /* max video layer num of each dev */

#define OT_VO_LAYER_V0                     0  /* video layer 0 */
#define OT_VO_LAYER_G0                     1  /* graphics layer 0 */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __OT_DEFINES_H__ */

