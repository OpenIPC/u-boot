/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "ot_mpi_otp.h"
#include "drv_otp.h"
#include "crypto_osal_lib.h"

td_s32 ot_mpi_otp_init()
{
    return TD_SUCCESS;
}

td_s32 ot_mpi_otp_deinit(void)
{
    return TD_SUCCESS;
}

td_s32 ot_mpi_otp_read_word(td_u32 offset, td_u32 *data)
{
    return drv_otp_read_word(offset, data);
}

td_s32 ot_mpi_otp_read_byte(td_u32 offset, td_u8 *data)
{
    return drv_otp_read_byte(offset, data);
}

td_s32 ot_mpi_otp_write_byte(td_u32 offset, td_u8 data)
{
    return drv_otp_write_byte(offset, data);
}
