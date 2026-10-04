/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description: Provides drv_pke common api. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-11-28, Create file. \n
 */

#include "drv_pke.h"
#include "hal_pke_v5.h"
#include "crypto_drv_common.h"

td_s32 drv_cipher_pke_init(void)
{
    int ret;
    crypto_drv_func_enter();
    ret = hal_pke_init();
    crypto_drv_func_exit();
    return ret;
}

td_s32 drv_cipher_pke_deinit(void)
{
    int ret;
    crypto_drv_func_enter();
    ret = hal_pke_deinit();
    crypto_drv_func_exit();
    return ret;
}