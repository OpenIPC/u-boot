/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#ifndef __AO_H__
#define __AO_H__

#include "audio_ao.h"

int start_ao(unsigned int addr, unsigned int size, audio_sample_rate sample_rate,
    unsigned int chn_cnt, unsigned int vol);
int stop_ao(void);

#endif