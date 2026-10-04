/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
*/

#ifndef CRYPTO_DRV_COMMON_H
#define CRYPTO_DRV_COMMON_H

#include "crypto_type.h"
#include "crypto_osal_lib.h"
#include "crypto_log.h"
#include "crypto_security.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#if defined(CONFIG_CRYPTO_PERF_STATISTICS)
td_u64 crypto_timer_end(td_u32 timer_id, const td_char *item_name);
#else
#define crypto_timer_end(...)
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif