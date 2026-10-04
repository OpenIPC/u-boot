/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description: Provides pke_alg get_multi_random. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-11-28, Create file. \n
 */

#include "hal_pke_alg.h"
#include "drv_trng.h"

td_s32 drv_cipher_pke_get_multi_random(td_u8 *random, td_u32 size)
{
    return drv_cipher_trng_get_multi_random(size, random);
}