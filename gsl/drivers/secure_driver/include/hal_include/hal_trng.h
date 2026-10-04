/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
*/
#ifndef HAL_TRNG_H
#define HAL_TRNG_H

#include "crypto_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

td_s32 hal_cipher_trng_get_random(td_u32 *randnum);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif