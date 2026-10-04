/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef UAPI_KDF_H
#define UAPI_KDF_H

#include "crypto_type.h"
#include "crypto_kdf_struct.h"

td_s32 unify_uapi_cipher_pbkdf2(const crypto_kdf_pbkdf2_param *param, td_u8 *out, const td_u32 out_len);

#endif