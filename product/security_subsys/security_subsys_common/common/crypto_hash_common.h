/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description: Provides cipher driver hash utils. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2024-01-22, Create file. \n
 */

#ifndef CRYPTO_HASH_COMMON_H
#define CRYPTO_HASH_COMMON_H

#include "crypto_hash_struct.h"
#include "crypto_drv_common.h"

td_s32 drv_hash_get_state_iv(crypto_hash_type hash_type, td_u32 *iv_size, td_u32 *state_iv, td_u32 state_iv_len);

td_s32 hash_mutex_init(td_void);

td_void hash_mutex_destroy(td_void);

td_void hash_common_lock(td_void);

td_void hash_common_unlock(td_void);

#endif