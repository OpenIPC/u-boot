/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description: hal trng header. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-03-22, Create file. \n
 */
#ifndef HAL_TRNG_H
#define HAL_TRNG_H

#include "crypto_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

td_s32 hal_cipher_trng_init(td_void);

td_s32 hal_cipher_trng_get_random(td_u32 *randnum);

td_s32 hal_cipher_trng_deinit(td_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif