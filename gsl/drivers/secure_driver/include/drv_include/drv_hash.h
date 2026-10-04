/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
*/

#ifndef DRV_HASH_H
#define DRV_HASH_H

#include "crypto_type.h"
#include "crypto_hash_struct.h"
#include "crypto_kdf_struct.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

td_s32 drv_cipher_hash_init(td_void);

td_s32 drv_cipher_hash_deinit(td_void);

td_s32 drv_cipher_hash_start(td_handle *drv_hash_handle, const crypto_hash_attr *hash_attr);

td_s32 drv_cipher_hash_update(td_handle drv_hash_handle,  const crypto_buf_attr *src_buf, const td_u32 len);

td_s32 drv_cipher_hash_finish(td_handle drv_hash_handle, td_u8 *out, td_u32 *out_len);

td_s32 drv_cipher_hash_destroy(td_handle drv_hash_handle);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif