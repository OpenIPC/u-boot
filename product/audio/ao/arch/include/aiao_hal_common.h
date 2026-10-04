/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#ifndef __AIAO_HAL_COMMON_H__
#define __AIAO_HAL_COMMON_H__

#include "type.h"
#include "audio_common_aio.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define io_address(x) (x)

#define AIO_DATA_WIDTH_8BIT 0x0
#define AIO_DATA_WIDTH_16BIT 0x1
#define AIO_DATA_WIDTH_18BIT 0x2
#define AIO_DATA_WIDTH_20BIT 0x3
#define AIO_DATA_WIDTH_24BIT 0x4
#define AIO_DATA_WIDTH_32BIT 0x5

#define AIO_PCM_DATA_WIDTH_8BIT 0x0
#define AIO_PCM_DATA_WIDTH_16BIT 0x1
#define AIO_PCM_DATA_WIDTH_32BIT 0x2
#define AIO_PCM_DATA_WIDTH_64BIT 0x3
#define AIO_PCM_DATA_WIDTH_128BIT 0x4

#define AIO_CHN_NUM_2CHN 0x0
#define AIO_CHN_NUM_4CHN 0x1
#define AIO_CHN_NUM_8CHN 0x2
#define AIO_CHN_NUM_16CHN 0x3

#define AIO_ONE_FIFO_BITWIDTH 128
#define aio_get_bit_cnt(bit_width) (8 << (bit_width))

static inline bool aio_is_pcm_mode(aio_mode mode)
{
    return (mode == AIO_MODE_PCM_SLAVE_STD || mode == AIO_MODE_PCM_SLAVE_NSTD ||
        mode == AIO_MODE_PCM_MASTER_STD || mode == AIO_MODE_PCM_MASTER_NSTD);
}

static inline bool aio_is_i2s_mode(aio_mode mode)
{
    return (mode == AIO_MODE_I2S_MASTER || mode == AIO_MODE_I2S_SLAVE);
}

static inline bool aio_is_master_mode(aio_mode mode)
{
    return (mode == AIO_MODE_I2S_MASTER || mode == AIO_MODE_PCM_MASTER_STD || mode == AIO_MODE_PCM_MASTER_NSTD);
}

static inline bool aio_is_slave_mode(aio_mode mode)
{
    return (mode == AIO_MODE_I2S_SLAVE || mode == AIO_MODE_PCM_SLAVE_STD || mode == AIO_MODE_PCM_SLAVE_NSTD);
}

#define AIP0_INTMASK 1
#define AOP0_INTMASK 1
#define AOP1_INTMASK 1

#define AI_DEV_MAX_NUM 1
#define AO_DEV_MIN_NUM 0
#define AO_DEV_MAX_NUM 1

#define AIO_MAX_NUM 1
#define AIO_MAX_CHN_NUM 2

#define AIO_CHN_ID_0    0
#define AIO_CHN_ID_1    1
#define AIO_CHN_ID_2    2

#define MAX_AUDIO_FRAME_NUM 50  /* max count of audio frame in Buffer */
#define MAX_AUDIO_POINT_BYTES 4 /* max bytes of one sample point(now 32bit max) */

#define MAX_VOICE_POINT_NUM 480 /* max sample per frame for voice encode */

#define MAX_AUDIO_POINT_NUM 2048 /* max sample per frame for all encoder(aacplus:2048) */
#define MAX_AO_POINT_NUM 4096    /* from h3:support 4096 framelen */
#define MIN_AUDIO_POINT_NUM 80   /* min sample per frame */
#define MAX_AI_POINT_NUM 2048    /* max sample per frame for all encoder(aacplus:2048) */

/* max length of audio frame by bytes, one frame contain many sample point */
#define MAX_AUDIO_FRAME_LEN (MAX_AUDIO_POINT_BYTES * MAX_AO_POINT_NUM)

#define AIO_BCLK_TO_FSCLK_RATIO_16  16
#define AIO_BCLK_TO_FSCLK_RATIO_32  32
#define AIO_BCLK_TO_FSCLK_RATIO_48  48
#define AIO_BCLK_TO_FSCLK_RATIO_64  64
#define AIO_BCLK_TO_FSCLK_RATIO_128 128
#define AIO_BCLK_TO_FSCLK_RATIO_256 256
#define AIO_BCLK_TO_FSCLK_RATIO_320 320
#define AIO_BCLK_TO_FSCLK_RATIO_384 384

#define AIO_MCLK_TO_BCLK_RATIO_1    1
#define AIO_MCLK_TO_BCLK_RATIO_2    2
#define AIO_MCLK_TO_BCLK_RATIO_3    3
#define AIO_MCLK_TO_BCLK_RATIO_4    4
#define AIO_MCLK_TO_BCLK_RATIO_6    6
#define AIO_MCLK_TO_BCLK_RATIO_8    8
#define AIO_MCLK_TO_BCLK_RATIO_12   12
#define AIO_MCLK_TO_BCLK_RATIO_16   16
#define AIO_MCLK_TO_BCLK_RATIO_24   24
#define AIO_MCLK_TO_BCLK_RATIO_32   32
#define AIO_MCLK_TO_BCLK_RATIO_48   48
#define AIO_MCLK_TO_BCLK_RATIO_64   64

#define SYS_AIO_SAMPLE_CLK16      0x0 /* 16 division */
#define SYS_AIO_SAMPLE_CLK32      0x01 /* 32 division */
#define SYS_AIO_SAMPLE_CLK48      0x02 /* 48 division */
#define SYS_AIO_SAMPLE_CLK64      0x03 /* 64 division */
#define SYS_AIO_SAMPLE_CLK128     0x04 /* 128 division */
#define SYS_AIO_SAMPLE_CLK256     0x05 /* 256 division */
#define SYS_AIO_SAMPLE_CLK320     0x06 /* 320 division */
#define SYS_AIO_SAMPLE_CLK384     0x07 /* 384 division */

/* For the following function
 * * typedef TD_S32 FN_SYS_AioBitStreamClkDivSel(TD_U32 u32DivSel);
 */
#define SYS_AIO_BS_CLK1 0x00  /* 1 division */
#define SYS_AIO_BS_CLK2 0x02  /* 2 division */
#define SYS_AIO_BS_CLK3 0x01  /* 3 division */
#define SYS_AIO_BS_CLK4 0x03  /* 4 division */
#define SYS_AIO_BS_CLK6 0x04  /* 6 division */
#define SYS_AIO_BS_CLK8 0x05  /* 8 division */
#define SYS_AIO_BS_CLK12 0x06 /* 12 division */
#define SYS_AIO_BS_CLK16 0x07 /* 16 division */
#define SYS_AIO_BS_CLK24 0x08 /* 24 division */
#define SYS_AIO_BS_CLK32 0x09 /* 32 division */
#define SYS_AIO_BS_CLK48 0x0a /* 48 division */
#define SYS_AIO_BS_CLK64 0x0b /* 64 division */

/* 2376M */
/* (MCLK0/AIO source clock freq) x 2^27 */
#define AIO_MCLK_48K_256FS_2376M    0x000A9778 /* 48k * 256 */
#define AIO_MCLK_44P1K_256FS_2376M  0x0009BB2A /* 44.1k * 256 */
#define AIO_MCLK_32K_256FS_2376M    0x00070FA5 /* 32k * 256 */

#define AIO_MCLK_48K_320FS_2376M    0x000D3D56 /* 48k * 320 */
#define AIO_MCLK_44P1K_320FS_2376M  0x000C29F4 /* 44.1k * 320 */
#define AIO_MCLK_32K_320FS_2376M    0x0008D38F /* 32k * 320 */

#define AIO_MCLK_96K_256FS_2376M    0x00152EF0 /* 96k * 256 */
#define AIO_MCLK_88P2K_256FS_2376M  0x00137653 /* 88.2k * 256 */
#define AIO_MCLK_64K_256FS_2376M    0x000E1F4B /* 64k * 256 */

#define AIO_MCLK_192K_256FS_2376M   0x002A5DE1 /* 192k * 256 */
#define AIO_MCLK_176P4K_256FS_2376M 0x0026ECA6 /* 176.4k * 256 */
#define AIO_MCLK_128K_256FS_2376M   0x001C3E96 /* 128k * 256 */

/* 1900M */
/* (MCLK0/AIO source clock freq) x 2^27 */
#define AIO_MCLK_48K_256FS_1900M     0x000D3EC3 /* 48k * 256 */
#define AIO_MCLK_44P1K_256FS_1900M   0x000C2B44 /* 44.1k * 256 */
#define AIO_MCLK_32K_256FS_1900M     0x0008D482 /* 32k * 256 */

#define AIO_MCLK_48K_320FS_1900M     0x00108E74 /* 48k * 320 */
#define AIO_MCLK_44P1K_320FS_1900M   0x000F3615 /* 44.1k * 320 */
#define AIO_MCLK_32K_320FS_1900M     0x000B09A3 /* 32k * 320 */

#define AIO_MCLK_96K_256FS_1900M     0x001A7D87 /* 96k * 256 */
#define AIO_MCLK_88P2K_256FS_1900M   0x00185687 /* 88.2k * 256 */
#define AIO_MCLK_64K_256FS_1900M     0x0011A905 /* 64k * 256 */

#define AIO_MCLK_192K_256FS_1900M    0x0034FB0E /* 192k * 256 */
#define AIO_MCLK_176P4K_256FS_1900M  0x0030AD0E /* 176.4k * 256 */
#define AIO_MCLK_128K_256FS_1900M    0x00235209 /* 128k * 256 */

/* 1800M */
/* (MCLK0/AIO source clock freq) x 2^27 */
#define AIO_MCLK_48K_256FS_1800M    0x000DFB24 /* 48k * 256 */
#define AIO_MCLK_44P1K_256FS_1800M  0x000CD856 /* 44.1k * 256 */
#define AIO_MCLK_32K_256FS_1800M    0x00095218 /* 32k * 256 */

#define AIO_MCLK_48K_320FS_1800M    0x001179ED /* 48k * 320 */
#define AIO_MCLK_44P1K_320FS_1800M  0x00100E6B /* 44.1k * 320 */
#define AIO_MCLK_32K_320FS_1800M    0x000BA69E /* 32k * 320 */

#define AIO_MCLK_96K_256FS_1800M    0x001BF647 /* 96k * 256 */
#define AIO_MCLK_88P2K_256FS_1800M  0x0019B0AB /* 88.2k * 256 */
#define AIO_MCLK_64K_256FS_1800M    0x0012A430 /* 64k * 256 */

#define AIO_MCLK_192K_256FS_1800M   0x0037EC8F /* 192k * 256 */
#define AIO_MCLK_176P4K_256FS_1800M 0x00336156 /* 176.4k * 256 */
#define AIO_MCLK_128K_256FS_1800M   0x0025485F /* 128k * 256 */

/* 1400M */
/* (MCLK0/AIO source clock freq) x 2^27 */
#define AIO_MCLK_48K_256FS_1400M    0x0011F9C0 /* 48k * 256 */
#define AIO_MCLK_44P1K_256FS_1400M  0x001083DC /* 44.1k * 256 */
#define AIO_MCLK_32K_256FS_1400M    0x000BFBD5 /* 32k * 256 */

#define AIO_MCLK_48K_320FS_1400M    0x00167830 /* 48k * 320 */
#define AIO_MCLK_44P1K_320FS_1400M  0x0014A4D3 /* 44.1k * 320 */
#define AIO_MCLK_32K_320FS_1400M    0x000EFACB /* 32k * 320 */

#define AIO_MCLK_96K_256FS_1400M    0x0023F380 /* 96k * 256 */
#define AIO_MCLK_88P2K_256FS_1400M  0x002107B8 /* 88.2k * 256 */
#define AIO_MCLK_64K_256FS_1400M    0x0017F7AB /* 64k * 256 */

#define AIO_MCLK_192K_256FS_1400M   0x0047E701 /* 192k * 256 */
#define AIO_MCLK_176P4K_256FS_1400M 0x00420F6F /* 176.4k * 256 */
#define AIO_MCLK_128K_256FS_1400M   0x002FEF56 /* 128k * 256 */

/* 1188M */
/* (MCLK0/AIO source clock freq) x 2^27 */
#define AIO_MCLK_48K_256FS_1188M    0x00152EF0 /* 48k * 256 */
#define AIO_MCLK_44P1K_256FS_1188M  0x00137653 /* 44.1k * 256 */
#define AIO_MCLK_32K_256FS_1188M    0x000E1F4B /* 32k * 256 */

#define AIO_MCLK_48K_320FS_1188M    0x001A7AAC /* 48k * 320 */
#define AIO_MCLK_44P1K_320FS_1188M  0x001853E8 /* 44.1k * 320 */
#define AIO_MCLK_32K_320FS_1188M    0x0011A71E /* 32k * 320 */

#define AIO_MCLK_96K_256FS_1188M    0x002A5DE1 /* 96k * 256 */
#define AIO_MCLK_88P2K_256FS_1188M  0x0026ECA6 /* 88.2k * 256 */
#define AIO_MCLK_64K_256FS_1188M    0x001C3E96 /* 64k * 256 */

#define AIO_MCLK_192K_256FS_1188M   0x0054BBC1 /* 192k * 256 */
#define AIO_MCLK_176P4K_256FS_1188M 0x004DD94D /* 176.4k * 256 */
#define AIO_MCLK_128K_256FS_1188M   0x00387D2B /* 128k * 256 */

/* 786.432M */
/* (MCLK0/AIO source clock freq) x 2^27 */
#define AIO_MCLK_48K_256FS_786M     0x00200000 /* 48k * 256 */
#define AIO_MCLK_44P1K_256FS_786M   0x001D6666 /* 44.1k * 256 */
#define AIO_MCLK_32K_256FS_786M     0x00155555 /* 32k * 256 */

#define AIO_MCLK_48K_320FS_786M     0x00280000 /* 48k * 320 */
#define AIO_MCLK_44P1K_320FS_786M   0x0024C000 /* 44.1k * 320 */
#define AIO_MCLK_32K_320FS_786M     0x001AAAAB /* 32k * 320 */

#define AIO_MCLK_96K_256FS_786M     0x00400000 /* 96k * 256 */
#define AIO_MCLK_88P2K_256FS_786M   0x003ACCCD /* 88.2k * 256 */
#define AIO_MCLK_64K_256FS_786M     0x002AAAAB /* 64k * 256 */

#define AIO_MCLK_192K_256FS_786M    0x00800000 /* 192k * 256 */
#define AIO_MCLK_176P4K_256FS_786M  0x0075999A /* 176.4k * 256 */
#define AIO_MCLK_128K_256FS_786M    0x00555556 /* 128k * 256 */

#define AIO_FIFO_BIT_WIDTH_256  256
#define AIO_FIFO_BIT_WIDTH_320  320
#define AIO_FIFO_BIT_WIDTH_384  384

#define ot_unused(x)             ((td_void)x)

#define AIO_CRG_NUM 16
#define AIO_CRG_ID_INVALID (-1)
/* RX */
#define AIO_CRG_ID_0    0
#define AIO_CRG_ID_1    1
#define AIO_CRG_ID_2    2
/* TX */
#define AIO_CRG_ID_8    8
#define AIO_CRG_ID_9    9
#define AIO_CRG_ID_10   10

/* TX SD */
#define TX_SD_ID_0  0x0
#define TX_SD_ID_1  0x1
#define TX_SD_ID_2  0x2
/* RX SD */
#define RX_SD_ID_0  0x8
#define RX_SD_ID_1  0x9
#define RX_SD_ID_2  0xa

typedef td_s32 audio_dev;

typedef enum {
    AIO_TYPE_AI = 0,
    AIO_TYPE_AO,
    AIO_TYPE_BUTT
} aio_type;

typedef struct {
    td_bool fade;
    audio_fade_rate fade_in_rate;
    audio_fade_rate fade_out_rate;
} audio_fade;

typedef struct {
    /* 256fs for 48kHz */
    td_u32 mclk_div_48k_256fs;
    td_u32 mclk_div_44p1k_256fs;
    td_u32 mclk_div_32k_256fs;

    /* 320fs for 48kHz */
    td_u32 mclk_div_48k_320fs;
    td_u32 mclk_div_44p1k_320fs;
    td_u32 mclk_div_32k_320fs;

    /* 256fs for 96kHz */
    td_u32 mclk_div_96k_256fs;
    td_u32 mclk_div_88p2k_256fs;
    td_u32 mclk_div_64k_256fs;

    /* 256fs for 192kHz */
    td_u32 mclk_div_192k_256fs;
    td_u32 mclk_div_176p4k_256fs;
    td_u32 mclk_div_128k_256fs;
}aiao_clk_info;

/* Description : BSPAIO IP Driver API */

td_void aop_hal_set_buffer_addr(td_u32 chn_id, td_u32 value);
td_void aop_hal_set_buffer_size(td_u32 chn_id, td_u32 value);
td_void aop_hal_set_buff_wptr(td_u32 chn_id, td_u32 value);
td_void aop_hal_set_buff_rptr(td_u32 chn_id, td_u32 value);
td_void aop_hal_set_trans_size(td_u32 chn_id, td_u32 value);
td_s32 aop_hal_set_dev_attr(td_u32 chn_id, const aio_attr *attr);
td_s32 aop_hal_set_volume(td_u32 chn_id, td_s32 volume_db);
td_void aop_hal_dev_enable(td_u32 chn_id);
td_void aop_hal_dev_disable(td_u32 chn_id);
td_s32 aiao_module_init(td_void);
td_void aiao_module_exit(td_void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif