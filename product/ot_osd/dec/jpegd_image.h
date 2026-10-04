/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#ifndef __JPEGD_IMAGE_H__
#define __JPEGD_IMAGE_H__
#include "jpegd.h"

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

int decode_sof0(motion_jpeg_obj *mjpeg, const unsigned char *stream);
int decode_sos(motion_jpeg_obj *mjpeg, const unsigned char *stream);
int decode_dht(motion_jpeg_obj *mjpeg, const unsigned char *stream);
int decode_dqt(motion_jpeg_obj *mjpeg, const unsigned char *stream);
int decode_dri(motion_jpeg_obj *mjpeg, const unsigned char *stream);
void init_default_huffman_table(motion_jpeg_obj *mjpeg);
void init_default_quant_table(motion_jpeg_obj *mjpeg);

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif
