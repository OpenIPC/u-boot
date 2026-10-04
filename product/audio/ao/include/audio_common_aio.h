/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#ifndef __AUDIO_COMMON_AIO_H__
#define __AUDIO_COMMON_AIO_H__

#include <asm/io.h>

#include "audio_ao.h"
#include "type.h"

typedef enum {
    AUDIO_SOUND_MODE_MONO = 0,   /* mono */
    AUDIO_SOUND_MODE_STEREO = 1, /* stereo */
    AUDIO_SOUND_MODE_BUTT
} audio_sound_mode;

typedef enum {
    AIO_MODE_I2S_MASTER = 0,  /* AIO I2S master mode */
    AIO_MODE_I2S_SLAVE,       /* AIO I2S slave mode */
    AIO_MODE_PCM_SLAVE_STD,   /* AIO PCM slave standard mode */
    AIO_MODE_PCM_SLAVE_NSTD,  /* AIO PCM slave non-standard mode */
    AIO_MODE_PCM_MASTER_STD,  /* AIO PCM master standard mode */
    AIO_MODE_PCM_MASTER_NSTD, /* AIO PCM master non-standard mode */
    AIO_MODE_BUTT
} aio_mode;

typedef enum {
    AUDIO_BIT_WIDTH_8 = 0,  /* 8bit width */
    AUDIO_BIT_WIDTH_16 = 1, /* 16bit width */
    AUDIO_BIT_WIDTH_24 = 2, /* 24bit width */
    AUDIO_BIT_WIDTH_BUTT,
} audio_bit_width;

typedef struct {
    audio_sample_rate sample_rate; /* sample rate */
    audio_bit_width bit_width;     /* bit_width */
    aio_mode work_mode;            /* master or slave mode */
    audio_sound_mode snd_mode;     /* momo or steror */
    td_u32 expand_flag;               /* expand 8bit to 16bit,use AI_EXPAND(only valid for AI 8bit) */
    td_u32 frame_num;                 /* frame num in buf[2,MAX_AUDIO_FRAME_NUM] */
    td_u32 point_num_per_frame;       /* point num per frame (80/160/240/320/480/1024/2048)
                                       (ADPCM IMA should add 1 point, AMR only support 160) */
    td_u32 chn_cnt;                   /* channle number on FS, valid value:1/2/4/8 */
    td_u32 clk_share; /* 0: AI and AO clock is separate; 1: AI and AO clock is inseparate, AI use AO's clock */
} aio_attr;

/* Defines audio clksel type */
typedef enum {
    AUDIO_CLK_SELECT_BASE       = 0,  /* Audio base clk. */
    AUDIO_CLK_SELECT_SPARE,           /* Audio spare clk. */

    AUDIO_CLK_SELECT_BUTT,
} audio_clk_select;

typedef enum {
    AUDIO_FADE_RATE_1 = 0,
    AUDIO_FADE_RATE_2 = 1,
    AUDIO_FADE_RATE_4 = 2,
    AUDIO_FADE_RATE_8 = 3,
    AUDIO_FADE_RATE_16 = 4,
    AUDIO_FADE_RATE_32 = 5,
    AUDIO_FADE_RATE_64 = 6,
    AUDIO_FADE_RATE_128 = 7,
    AUDIO_FADE_RATE_BUTT
} audio_fade_rate;

typedef enum {
    AUDIO_TRACK_NORMAL = 0,
    AUDIO_TRACK_BOTH_LEFT = 1,
    AUDIO_TRACK_BOTH_RIGHT = 2,
    AUDIO_TRACK_EXCHANGE = 3,
    AUDIO_TRACK_MIX = 4,
    AUDIO_TRACK_LEFT_MUTE = 5,
    AUDIO_TRACK_RIGHT_MUTE = 6,
    AUDIO_TRACK_BOTH_MUTE = 7,
    AUDIO_TRACK_BUTT
} audio_track_mode;

static inline void ot_reg_set_bit(unsigned long value, unsigned long offset, unsigned long addr)
{
    unsigned long t;
    unsigned long mask;

    mask = 1 << offset;
    t = readl((td_u32 *)(td_uintptr_t)addr);
    t &= ~mask;
    t |= (value << offset) & mask;
    writel(t, (td_u32 *)(td_uintptr_t)addr);
}

#endif