/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "ot_mpi_cipher.h"

#include "drv_hash.h"
#include "crypto_osal_lib.h"
#include "crypto_drv_common.h"

td_s32 ot_mpi_cipher_hash_init(td_void)
{
    return TD_SUCCESS;
}

td_s32 ot_mpi_cipher_hash_deinit(td_void)
{
    return TD_SUCCESS;
}

td_s32 ot_mpi_cipher_hash_create(td_handle *mpi_hash_handle, const crypto_hash_attr *hash_attr)
{
    return drv_cipher_hash_start(mpi_hash_handle, hash_attr);
}

td_s32 ot_mpi_cipher_hash_update(td_handle mpi_hash_handle, const crypto_buf_attr *src_buf, const td_u32 len)
{
    return drv_cipher_hash_update(mpi_hash_handle, src_buf, len);
}

td_s32 ot_mpi_cipher_hash_finish(td_handle mpi_hash_handle, td_u8 *virt_addr, td_u32 buffer_len, td_u32 *result_len)
{
    td_s32 ret;
    crypto_chk_return(virt_addr == TD_NULL, UAPI_COMPAT_ERRNO(ERROR_MODULE_HASH, ERROR_PARAM_IS_NULL),
        "virt_addr is null\n");
    crypto_chk_return(result_len == TD_NULL, UAPI_COMPAT_ERRNO(ERROR_MODULE_HASH, ERROR_PARAM_IS_NULL),
        "result_len is null\n");
    *result_len = buffer_len;
    ret = drv_cipher_hash_finish(mpi_hash_handle, virt_addr, result_len);
    return ret;
}

td_s32 ot_mpi_cipher_hash_get(td_handle mpi_hash_handle, crypto_hash_clone_ctx *hash_clone_ctx)
{
    return drv_cipher_hash_get(mpi_hash_handle, hash_clone_ctx);
}

td_s32 ot_mpi_cipher_hash_set(td_handle mpi_hash_handle, const crypto_hash_clone_ctx *hash_clone_ctx)
{
    return drv_cipher_hash_set(mpi_hash_handle, hash_clone_ctx);
}

td_s32 ot_mpi_cipher_hash_destroy(td_handle mpi_hash_handle)
{
    return drv_cipher_hash_destroy(mpi_hash_handle);
}
