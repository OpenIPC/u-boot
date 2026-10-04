/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
*/

#if defined(CONFIG_PKE_CAL_HASH_SUPPORT)

#include "hal_pke_alg.h"
#include "drv_pke_inner.h"
#include "drv_hash.h"

#include "crypto_drv_common.h"

static crypto_hash_type drv_pke_get_hash_type(drv_pke_hash_type hash_type)
{
    switch (hash_type) {
        case DRV_PKE_HASH_TYPE_SM3:
            return CRYPTO_HASH_TYPE_SM3;
        default:
            crypto_log_err("Invalid hash_type\n");
            return CRYPTO_HASH_TYPE_INVALID;
    }
}

#define INVALID_HANDLE  0xFFFF0000
td_s32 hal_pke_alg_calc_hash(const drv_pke_data *arr, td_u32 arr_len,
    const drv_pke_hash_type hash_type, drv_pke_data *hash)
{
    td_s32 ret = TD_FAILURE;
    td_handle hash_handle = INVALID_HANDLE;
    crypto_hash_attr hash_attr;
    if (memset_s(&hash_attr, sizeof(hash_attr), 0, sizeof(hash_attr)) != 0) {
        return TD_FAILURE;
    }
    td_u8 *buffer = NULL;
    td_u32 buffer_size = 256;
    crypto_buf_attr src_buf;
    uint32_t i = 0;
    uint32_t tail_len = 0;
    uint32_t processing_len = 0;
    uint32_t processed_len = 0;
    uint32_t left = 0;

    hash_attr.hash_type = drv_pke_get_hash_type(hash_type);
    if (hash_attr.hash_type == CRYPTO_HASH_TYPE_INVALID) {
        return PKE_COMPAT_ERRNO(ERROR_INVALID_PARAM);
    }

    buffer = crypto_malloc_coherent(buffer_size, "pke calc hash buffer");
    crypto_chk_return(buffer == NULL, PKE_COMPAT_ERRNO(ERROR_MALLOC), "crypto_malloc_coherent failed\n");

    ret = drv_cipher_hash_start(&hash_handle, &hash_attr);
    crypto_chk_goto(ret != CRYPTO_SUCCESS, exit_free, "drv_cipher_hash_start failed\n");

    src_buf.virt_addr = buffer;
    while (i < arr_len) {
        /* case 1. */
        if ((arr[i].length + tail_len) < buffer_size) {
            ret = memcpy_s(buffer + tail_len, buffer_size - tail_len, arr[i].data, arr[i].length);
            crypto_chk_goto_with_ret(ret, ret != EOK, hash_destroy, PKE_COMPAT_ERRNO(ERROR_MEMCPY_S),
                "memcpy_s failed\n");
            tail_len += arr[i].length;
            i++;
            continue;
        }
        /* case 2. */
        processing_len = buffer_size - tail_len;
        ret = memcpy_s(buffer + tail_len, processing_len, arr[i].data, processing_len);
        crypto_chk_goto_with_ret(ret, ret != EOK, hash_destroy, PKE_COMPAT_ERRNO(ERROR_MEMCPY_S), "memcpy_s failed\n");

        /* process. */
        ret = drv_cipher_hash_update(hash_handle, &src_buf, buffer_size);
        crypto_chk_goto(ret != CRYPTO_SUCCESS, hash_destroy, "drv_cipher_hash_update failed\n");
        processed_len = processing_len;

        left = arr[i].length - processing_len;
        while (left >= buffer_size) {
            ret = memcpy_s(buffer, buffer_size, arr[i].data + processed_len, buffer_size);
            crypto_chk_goto_with_ret(ret, ret != EOK, hash_destroy, PKE_COMPAT_ERRNO(ERROR_MEMCPY_S),
                "memcpy_s failed\n");
            ret = drv_cipher_hash_update(hash_handle, &src_buf, buffer_size);
            crypto_chk_goto(ret != CRYPTO_SUCCESS, hash_destroy, "drv_cipher_hash_update failed\n");
            left -= buffer_size;
            processed_len += buffer_size;
        }
        if (left != 0) {
            ret = memcpy_s(buffer, buffer_size, arr[i].data + processed_len, left);
            crypto_chk_goto_with_ret(ret, ret != EOK, hash_destroy, PKE_COMPAT_ERRNO(ERROR_MEMCPY_S),
                "memcpy_s failed\n");
        }
        tail_len = left;
        i++;
    }
    if (tail_len != 0) {
        ret = drv_cipher_hash_update(hash_handle, &src_buf, tail_len);
        crypto_chk_goto(ret != CRYPTO_SUCCESS, hash_destroy, "drv_cipher_hash_update failed\n");
    }

    ret = drv_cipher_hash_finish(hash_handle, hash->data, &hash->length);
    crypto_chk_goto(ret != CRYPTO_SUCCESS, hash_destroy, "drv_cipher_hash_finish failed\n");
    hash_handle = INVALID_HANDLE;

hash_destroy:
    if (hash_handle != INVALID_HANDLE) {
        drv_cipher_hash_destroy(hash_handle);
    }
exit_free:
    CRYPTO_FREE_COHERENT(buffer);
    return ret;
}
#endif