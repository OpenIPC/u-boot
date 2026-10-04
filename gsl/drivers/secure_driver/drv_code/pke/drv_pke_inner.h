/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
*/

#ifndef DRV_PKE_INNER_H
#define DRV_PKE_INNER_H

#include "drv_pke.h"
#include "hal_pke_alg.h"

#define PKE_COMPAT_ERRNO(err_code)      DRV_COMPAT_ERRNO(ERROR_MODULE_PKE, err_code)
#define pke_null_ptr_chk(ptr)   \
    crypto_chk_return((ptr) == TD_NULL, PKE_COMPAT_ERRNO(ERROR_PARAM_IS_NULL), #ptr" is NULL\n")

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/* Common. */
td_bool inner_drv_is_zero(const td_u8 *val, td_u32 length);

td_bool inner_drv_is_in_range(const uint8_t *value, const uint8_t *range, uint32_t len);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif