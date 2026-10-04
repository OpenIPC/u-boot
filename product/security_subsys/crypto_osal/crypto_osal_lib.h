/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef CRYPTO_OSAL_LIB_H
#define CRYPTO_OSAL_LIB_H

#include <common.h>
#include "crypto_type.h"
#include "crypto_platform.h"
#include "crypto_security.h"
#include "crypto_common_struct.h"
#include "linux/delay.h"
#include "securec.h"

td_s32 crypto_get_random(td_u32 *randnum);

#define crypto_udelay               udelay
#define crypto_msleep(ms)           crypto_udelay(1000 * (ms))

#define crypto_print            printf
#define crypto_log_err(fmt, ...)  crypto_log_fmt("ERROR", fmt, ##__VA_ARGS__)

#define crypto_symc_support(...)            TD_TRUE
#define crypto_rsa_support(...)             TD_TRUE
#define crypto_sm3_support(...)             TD_TRUE
#define crypto_sm4_support(...)             TD_TRUE

#define crypto_mutex                        td_handle
#define crypto_mutex_init(x)                TD_SUCCESS
#define crypto_mutex_destroy(x)             crypto_unused(x)
#define crypto_mutex_lock(x)                crypto_unused(x)
#define crypto_mutex_unlock(x)              crypto_unused(x)

#define crypto_ioremap_nocache(addr, size)  (td_void *)(addr)
#define crypto_iounmap(addr, size)

#define crypto_cache_all(...)

#define crypto_malloc(x)          (((x) > 0) ? malloc(x) : TD_NULL)
#define crypto_free(x)            {if (((x) != TD_NULL)) free(x);}

#define crypto_getpid()                         osal_get_current_pid()

#define spacc_reg_read(offset)          crypto_reg_read(SPACC_REG_BASE_ADDR + (offset))
#define spacc_reg_write(offset, value)  crypto_reg_write(SPACC_REG_BASE_ADDR + (offset), value)

#define trng_reg_read(offset)           crypto_reg_read(TRNG_REG_BASE_ADDR + (offset))
#define trng_reg_write(offset, value)   crypto_reg_write(TRNG_REG_BASE_ADDR + (offset), value)

#define pke_reg_read(offset)            crypto_reg_read(PKE_REG_BASE_ADDR + (offset))
#define pke_reg_write(offset, value)    crypto_reg_write(PKE_REG_BASE_ADDR + (offset), value)

#define km_reg_read(offset)            crypto_reg_read(KM_REG_BASE_ADDR + (offset))
#define km_reg_write(offset, value)    crypto_reg_write(KM_REG_BASE_ADDR + (offset), value)

#define otpc_reg_read(offset)              crypto_reg_read(OTPC_BASE_ADDR + (offset))
#define otpc_reg_write(offset, value)      crypto_reg_write(OTPC_BASE_ADDR + (offset), value)

#define ca_misc_reg_read(offset)            crypto_reg_read(CA_MISC_REG_BASE_ADDR + (offset))
#define ca_misc_reg_write(offset, value)    crypto_reg_write(CA_MISC_REG_BASE_ADDR + (offset), value)

#define CRYPTO_ERROR_ENV                            ERROR_ENV_NOOS

td_void *crypto_malloc_coherent(td_u32 size, td_char *buffer_name);

td_void *crypto_malloc_mmz(td_u32 size, td_char *buffer_name);

td_void crypto_free_coherent(td_void *ptr);

td_phys_addr_t crypto_get_phys_addr(td_void *ptr);

td_s32 crypto_phys_copy_to_virt(td_u8 *dst_virt_addr, td_phys_addr_t src_phys_addr, td_u32 length);

crypto_cpu_type crypto_get_cpu_type(td_void);

void crypto_cache_flush(uintptr_t base_addr, td_u32 size);

typedef void crypto_wait_t;

#endif