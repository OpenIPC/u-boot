/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
*/
#include "crypto_drv_common.h"

#include "cipher_romable.h"
#include "drv_hash.h"
#include "err_print.h"

#define hash_null_ptr_chk(ptr)    do {      \
    if ((ptr) == NULL) {                    \
        return ERROR_PARAM_IS_NULL;         \
    }                                       \
} while (0)

#define MAX_DATA_LEN        (128 * 1024 * 1024) // max length: 128 MB
#define SHA256_LEN          32

int32_t gsl_drv_cipher_sm3(const uint8_t *data, uint32_t data_len, uint8_t *hash, uint32_t hash_len)
{
    int ret;
    td_handle hash_handle;
    crypto_hash_attr hash_attr;
    crypto_buf_attr buf;
    uint32_t out_hash_len = hash_len;
    hash_null_ptr_chk(data);
    hash_null_ptr_chk(hash);
    crypto_chk_return(data_len > MAX_DATA_LEN, ERROR_INVALID_PARAM, "data_len is too large\n");
    crypto_chk_return(hash_len < SHA256_LEN, ERROR_INVALID_PARAM, "hash_len is invalid\n");
    buf.virt_addr = (uint8_t *)data;
    buf.phys_addr = (unsigned long long)(uintptr_t)data;
    hash_attr.hash_type = CRYPTO_HASH_TYPE_SM3;

    ret = drv_cipher_hash_start(&hash_handle, &hash_attr);

    crypto_chk_return(ret != CRYPTO_SUCCESS, ret, "drv_cipher_hash_start failed\n");

    ret = drv_cipher_hash_update(hash_handle, &buf, data_len);
    crypto_chk_goto(ret != CRYPTO_SUCCESS, exit_destroy, "drv_cipher_hash_update failed\n");
    ret = drv_cipher_hash_finish(hash_handle, hash, &out_hash_len);
    crypto_chk_goto(ret != CRYPTO_SUCCESS, exit_destroy, "drv_cipher_hash_finish failed\n");
exit_destroy:
    drv_cipher_hash_destroy(hash_handle);
    return ret;
}