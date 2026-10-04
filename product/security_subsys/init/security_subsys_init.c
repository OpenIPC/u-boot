/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "security_subsys_init.h"
#include "crypto_osal_lib.h"
#include "crypto_drv_common.h"
#include "crypto_pke_common.h"
#include "drv_pke.h"
#include "drv_hash.h"
#include "drv_symc.h"
#include "drv_trng.h"
#include "drv_klad.h"
#include "drv_keyslot.h"

#include <common.h>
#include <malloc.h>

int crypto_cipher_init(td_void)
{
    drv_cipher_pke_init();
    drv_cipher_trng_init();
    drv_cipher_hash_init();
    drv_cipher_symc_init();
    drv_keyslot_init();
    crypto_curve_param_init();
    return 0;
}

int crypto_cipher_deinit(td_void)
{
    drv_cipher_pke_deinit();
    drv_cipher_trng_deinit();
    drv_cipher_hash_deinit();
    drv_cipher_symc_deinit();
    drv_keyslot_deinit();
    return 0;
}

