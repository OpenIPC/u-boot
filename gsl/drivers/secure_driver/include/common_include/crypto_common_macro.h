/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
*/

#ifndef CRYPTO_COMMON_MACRO_H
#define CRYPTO_COMMON_MACRO_H

#ifndef CRYPTO_STATIC
#define CRYPTO_STATIC static
#endif

#define crypto_addr_h(addr)     (unsigned int)(((addr) & 0xFFFFFFFF00000000) >> 32)
#define crypto_addr_l(addr)     (unsigned int)((addr) & 0xFFFFFFFF)

#define crypto_cpu_to_be32(v) ((((td_u32)(v)) >> 24) | ((((td_u32)(v)) >> 8) & 0xff00) \
    | ((((td_u32)(v)) << 8) & 0xff0000) | (((td_u32)(v)) << 24))

#define crypto_max(a, b)    ((a) > (b)) ? (a) : (b)
#define crypto_min(a, b)    ((a) > (b)) ? (b) : (a)

#ifndef crypto_unused
#define crypto_unused(x)    (td_void)(x)
#endif

#define crypto_array_size(arr)  (sizeof(arr) / sizeof((arr)[0]))

#define crypto_reg_read(reg_addr)               (*(volatile td_u32 *)(uintptr_t)(reg_addr))
#define crypto_reg_write(reg_addr, value)       (*(volatile td_u32 *)(uintptr_t)(reg_addr) = (value))
#define crypto_reg_read_u16(reg_addr)           (*(volatile td_u16 *)(uintptr_t)(reg_addr))

#define crypto_set_bits(src, bit)   ((src) |= (1 << (bit)))
#define crypto_clear_bits(src, bit) ((src) &= ~(1 << (bit)))

/* Assert Check. */
#if defined(CONFIG_CRYPTO_ASSERT_ENABLE)
#define crypto_assert_eq(a, b) do { \
    if ((a) != (b)) {   \
        crypto_log_err("ASSERT_EQ Failure. \nExpected: %s == %s\nReal: %s is 0x%x, %s is 0x%x\n",   \
            #a, #b, #a, a, #b, b);  \
    }   \
} while (0)
#define crypto_assert_neq(a, b) do { \
    if ((a) == (b)) {   \
        crypto_log_err("ASSERT_EQ Failure. \nExpected: %s != %s\nReal: %s is 0x%x, %s is 0x%x\n",   \
            #a, #b, #a, a, #b, b);  \
    }   \
} while (0)
#else
#define crypto_assert_eq(a, b)
#define crypto_assert_neq(a, b)
#endif

/* Error Check. */
#ifndef crypto_chk_print
#define crypto_chk_print(cond, ...) do {      \
    if (cond) {                                         \
        crypto_log_err(__VA_ARGS__);                       \
    }                                                   \
} while (0)
#endif

#ifndef crypto_chk_return
#define crypto_chk_return(cond, err_ret, ...) do {      \
    if (cond) {                                         \
        crypto_log_err(__VA_ARGS__);                       \
        return err_ret;                                 \
    }                                                   \
} while (0)
#endif

#ifndef crypto_chk_return_only
#define crypto_chk_return_only(cond, err_ret) do {      \
    if (cond) {                                         \
        return err_ret;                                 \
    }                                                   \
} while (0)
#endif

#ifndef crypto_chk_return_void
#define crypto_chk_return_void(cond, ...) do { \
    if (cond) { \
        crypto_log_err(__VA_ARGS__);                       \
        return; \
    }   \
} while (0)
#endif

#ifndef crypto_chk_goto
#define crypto_chk_goto(cond, label, ...) do {          \
    if (cond) {                                         \
        crypto_log_err(__VA_ARGS__);                       \
        goto label;                                     \
    }                                                   \
} while (0)
#endif

/* Input Params Check. */
#ifndef crypto_param_require
#define crypto_param_require(cond)  do {                \
    if (!(cond)) {                                      \
        crypto_log_err("Param Check %s failed\n", #cond);  \
        return CRYPTO_FAILURE;                          \
    }                                                   \
} while (0)
#endif

#ifndef crypto_param_check
#define crypto_param_check(cond)  do {                  \
    if (cond) {                                         \
        crypto_log_err("Param Check %s failed\n", #cond);  \
        return CRYPTO_FAILURE;                          \
    }                                                   \
} while (0)
#endif

#ifndef crypto_chk_goto_with_ret
#define crypto_chk_goto_with_ret(ret, cond, label, err_ret, fmt, ...) do {           \
    if (cond) {                                                                 \
        crypto_log_err(fmt, ##__VA_ARGS__);                                        \
        ret = err_ret;                                                          \
        goto label;                                                             \
    }                                                                           \
} while (0)
#endif

#ifndef crypto_print_func_err
#define crypto_print_func_err(_func, _err_code) do { \
    crypto_log_err("%s failed! error code: 0x%x \r\n", #_func, _err_code); \
} while (0)
#endif

#ifndef crypto_chk_func_return
#define crypto_chk_func_return(func, ret) do { \
    if ((ret) != TD_SUCCESS) { \
        crypto_print_func_err(func, ret);   \
        return ret; \
    }   \
} while (0)
#endif

#ifndef drv_crypto_pke_check_param
#define drv_crypto_pke_check_param(cond) do {       \
    if (cond) {         \
        crypto_log_err("%s invalid\n", #cond);  \
        return ERROR_INVALID_PARAM;     \
    }       \
} while (0)
#endif

#ifndef hal_crypto_pke_check_param
#define hal_crypto_pke_check_param(cond) do {       \
    if (cond) {         \
        return ERROR_INVALID_PARAM;     \
    }       \
} while (0)
#endif

#if defined(CRYPTO_HAL_FUNC_TRACE_ENABLE)
#define crypto_hal_func_enter()        crypto_print("########%s===>Enter\n", __func__)
#define crypto_hal_func_exit()         crypto_print("########%s<===Exit\n", __func__)
#else
#define crypto_hal_func_enter()
#define crypto_hal_func_exit()
#endif

#if defined(CRYPTO_KAPI_FUNC_TRACE_ENABLE)
#define crypto_kapi_func_enter()        crypto_print("%s===>Enter\n", __func__)
#define crypto_kapi_func_exit()         crypto_print("%s<===Exit\n", __func__)
#else
#define crypto_kapi_func_enter()
#define crypto_kapi_func_exit()
#endif

#if defined(CRYPTO_DRV_FUNC_TRACE_ENABLE)
#define crypto_drv_func_enter()        crypto_print("####%s===>Enter\n", __func__)
#define crypto_drv_func_exit()         crypto_print("####%s<===Exit\n", __func__)
#else
#define crypto_drv_func_enter()
#define crypto_drv_func_exit()
#endif

#if defined(CRYPTO_DISPATCH_FUNC_TRACE_ENABLE)
#define crypto_dispatch_func_enter()        crypto_print("%s===>Enter\n", __func__)
#define crypto_dispatch_func_exit()         crypto_print("%s<===Exit\n", __func__)
#else
#define crypto_dispatch_func_enter()
#define crypto_dispatch_func_exit()
#endif

#if defined(CRYPTO_UAPI_FUNC_TRACE_ENABLE)
#define crypto_uapi_func_enter()    crypto_print("%s ===>Enter\n", __func__)
#define crypto_uapi_func_exit()     crypto_print("%s <===Exit\n", __func__)
#else
#define crypto_uapi_func_enter()
#define crypto_uapi_func_exit()
#endif

#endif
