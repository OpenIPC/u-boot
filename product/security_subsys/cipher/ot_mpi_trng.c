/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "ot_mpi_cipher.h"

#include "drv_trng.h"
#include "crypto_osal_lib.h"

td_s32 ot_mpi_cipher_trng_get_random(td_u32 *randnum)
{
    return drv_cipher_trng_get_random(randnum);
}

td_s32 ot_mpi_cipher_trng_get_multi_random(td_u32 size, td_u8 *randnum)
{
    return drv_cipher_trng_get_multi_random(size, randnum);
}