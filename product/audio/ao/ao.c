/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include <common.h>

#include "aiao_hal_common.h"
#include "ao.h"
#include "amp.h"

int start_ao(unsigned int addr, unsigned int size, audio_sample_rate sample_rate,
    unsigned int chn_cnt, unsigned int vol)
{
    td_u32 chn_id = 0;
    td_s32 ret;
    td_u32 trans_size;
    aio_attr attr;

    if (chn_cnt == 1) {
        attr.snd_mode = AUDIO_SOUND_MODE_MONO;
    } else if (chn_cnt == 2) { /* 2: chn count */
        attr.snd_mode = AUDIO_SOUND_MODE_STEREO;
    } else {
        printf("chn_cnt:%u is invalid!\n", chn_cnt);
        return TD_FAILURE;
    }
    attr.bit_width = AUDIO_BIT_WIDTH_16;
    attr.sample_rate = sample_rate;
    attr.chn_cnt = chn_cnt;
    attr.clk_share = 0;
    attr.expand_flag = 0;
    attr.frame_num = 30; /* 30: frame num */
    attr.point_num_per_frame = 320; /* 320: posint */
    attr.work_mode = AIO_MODE_I2S_MASTER;
    trans_size = attr.point_num_per_frame << attr.bit_width;

    ret = aiao_module_init();
    if (ret != TD_SUCCESS) {
        printf("failed to init aiao module!\n");
        return ret;
    }
    ret = aop_hal_set_dev_attr(chn_id, &attr);
    if (ret != TD_SUCCESS) {
        printf("failed to set aop dev attr!\n");
        return ret;
    }

    aop_hal_set_buffer_addr(chn_id, addr);

    aop_hal_set_buffer_size(chn_id, size);

    aop_hal_set_buff_wptr(chn_id, size - 32); /* 32 byte */

    aop_hal_set_buff_rptr(chn_id, 0);

    aop_hal_set_trans_size(chn_id, trans_size);

    ret = aop_hal_set_volume(chn_id, vol);
    if (ret != TD_SUCCESS) {
        printf("failed to set aop volume!\n");
        return ret;
    }
    aop_hal_dev_enable(chn_id);

    return TD_SUCCESS;
}

int stop_ao(void)
{
    td_u32 chn_id = 0;

    aop_hal_dev_disable(chn_id);

    aiao_module_exit();

    return TD_SUCCESS;
}