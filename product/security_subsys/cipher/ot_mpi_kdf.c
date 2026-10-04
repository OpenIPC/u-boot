/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "ot_mpi_cipher.h"

#include "drv_hash.h"
#include "crypto_osal_lib.h"
#include "crypto_drv_common.h"

td_s32 ot_mpi_cipher_pbkdf2(const crypto_kdf_pbkdf2_param *param, td_u8 *out, const td_u32 out_len)
{
    crypto_chk_return(param == TD_NULL, UAPI_COMPAT_ERRNO(ERROR_MODULE_HASH, ERROR_PARAM_IS_NULL), "param is null\n");
    crypto_chk_return(out_len == 0 || out_len > CRYPTO_PBKDF2_OUT_MAX_LENGTH,
        UAPI_COMPAT_ERRNO(ERROR_MODULE_HASH, ERROR_INVALID_PARAM), "out_len is Invalid\n");
    crypto_chk_return(param->plen > CRYPTO_PBKDF2_PASS_MAX_LENGTH,
        UAPI_COMPAT_ERRNO(ERROR_MODULE_HASH, ERROR_INVALID_PARAM), "plen is too long!\n");
    crypto_chk_return(param->slen > CRYPTO_PBKDF2_SALT_MAX_LENGTH,
        UAPI_COMPAT_ERRNO(ERROR_MODULE_HASH, ERROR_INVALID_PARAM), "slen is too long!\n");
    return drv_cipher_pbkdf2(param, out, out_len);
}