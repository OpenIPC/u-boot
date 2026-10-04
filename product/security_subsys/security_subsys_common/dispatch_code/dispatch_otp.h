/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef DISPATCH_OTP_H
#define DISPATCH_OTP_H

#include "ioctl_otp.h"
#include "crypto_drv_common.h"

crypto_ioctl_cmd *get_otp_func_list(td_void);

td_u32 get_otp_func_num(td_void);

#endif