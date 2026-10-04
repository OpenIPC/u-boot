/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#include "jpegd_comm.h"

__inline unsigned int get_low_addr(unsigned long long phyaddr)
{
    return (unsigned int)phyaddr;
}

__inline unsigned int get_high_addr(unsigned long long phyaddr)
{
    return (unsigned int)(phyaddr >> 32); /* shift right 32 bits */
}