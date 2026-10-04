/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#ifndef JPEGD_ENTRY_H
#define JPEGD_ENTRY_H

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */
#define ENV_BUF_LEN 32
unsigned long get_ot_logo(void);
unsigned long get_jpeg_size_val(void);
unsigned long get_video_data_base(void);
unsigned long get_jpegd_emar_buf(void);
unsigned int get_output_format(void);

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif /* JPEGD_ENTRY_H */
