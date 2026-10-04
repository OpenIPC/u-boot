/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
*/
#include "cipher_romable.h"
#include "cipher_romable_inner.h"
#include "drv_hash.h"
#include "hal_trng.h"
#include "hal_hash.h"

#include "crypto_drv_common.h"
#include "crypto_pke_common.h"

int32_t gsl_drv_cipher_init(void)
{
#if defined(CONFIG_SYMC_ROMABLE_API_SUPPORT)
    inner_symc_romable_init();
#endif
    drv_cipher_hash_init();
    crypto_curve_param_init();
    return CRYPTO_SUCCESS;
}

int32_t gsl_drv_cipher_deinit(void)
{
#if defined(CONFIG_SYMC_ROMABLE_API_SUPPORT)
    inner_symc_romable_deinit();
#endif
    drv_cipher_hash_deinit();
    return CRYPTO_SUCCESS;
}

int32_t gsl_drv_cipher_func_register(const ext_drv_func *func)
{
    return crypto_drv_func_register(
        (const crypto_romable_drv_func *)func
    );
}
