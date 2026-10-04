/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#ifndef JPEGD_ERROR_H
#define JPEGD_ERROR_H

#include <common.h>

/* for SOF error */
#define OT_ERR_NOT_BASELINE        0x00000001
#define OT_UNSUPPORT_GRAY_PIC      0x00000002
#define OT_UNSUPPORT_PIC_SIZE      0x00000004
#define OT_UNSUPPORT_PIC_STRUCT    0x00000008
#define OT_UNSUPPORT_YUV411        0x00000010

#define OT_ERR_QUANT_TABLE         0x00000020
#define OT_ERR_HUFFMAN_TABLE       0x00000040
#define OT_ERR_FRAME_HEADER        0x00000080
#define OT_ERR_SCAN_HEADER         0x00000100
#define OT_ERR_RESTART_ERROR       0x00000200
#define OT_ERR_UNSUPPORT_DNL       0x00000400

#define ot_trace(fmt...)                             \
    do {                                             \
        printf("[%s]-%d: ", __FUNCTION__, __LINE__); \
        printf(fmt);                                 \
    } while (0)

#endif /* End of JPEGD_ERROR_H */