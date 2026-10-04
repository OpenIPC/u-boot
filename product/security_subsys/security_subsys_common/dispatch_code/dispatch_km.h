/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef DISPATCH_KM_H
#define DISPATCH_KM_H

#include "ioctl_km.h"
#include "crypto_drv_common.h"

crypto_ioctl_cmd *get_km_func_list(td_void);

td_u32 get_km_func_num(td_void);

#endif