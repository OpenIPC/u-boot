/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
*/
#ifndef CRYPTO_OSAL_LIB_H
#define CRYPTO_OSAL_LIB_H

#include "crypto_platform.h"
#include "crypto_common_struct.h"
#include "common.h"
#include "soc_errno.h"
#include "lib.h"

#define crypto_print(...)

#define crypto_get_cpu_type(...)        CRYPTO_CPU_TYPE_SCPU

#define CRYPTO_ERROR_ENV        ERROR_ENV_NOOS

int crypto_drv_func_register(const crypto_romable_drv_func *drv_func);

/* Register Read&Write. */
#define spacc_reg_read(offset)          crypto_reg_read(SPACC_REG_BASE_ADDR + (offset))
#define spacc_reg_write(offset, value)  crypto_reg_write(SPACC_REG_BASE_ADDR + (offset), value)

#define trng_reg_read(offset)           crypto_reg_read(TRNG_REG_BASE_ADDR + (offset))
#define trng_reg_write(offset, value)   crypto_reg_write(TRNG_REG_BASE_ADDR + (offset), value)

#define pke_reg_read(offset)            crypto_reg_read(PKE_REG_BASE_ADDR + (offset))
#define pke_reg_write(offset, value)    crypto_reg_write(PKE_REG_BASE_ADDR + (offset), value)

#define km_reg_read(offset)             crypto_reg_read(KM_REG_BASE_ADDR + (offset))
#define km_reg_write(offset, value)     crypto_reg_write(KM_REG_BASE_ADDR + (offset), value)

#define otpc_reg_read(offset)           crypto_reg_read(OTPC_BASE_ADDR + (offset))
#define otpc_reg_write(offset, value)   crypto_reg_write(OTPC_BASE_ADDR + (offset), value)

#define ca_misc_reg_read(offset)            crypto_reg_read(CA_MISC_REG_BASE_ADDR + (offset))
#define ca_misc_reg_write(offset, value)    crypto_reg_write(CA_MISC_REG_BASE_ADDR + (offset), value)

/* udelay. */
void crypto_udelay(unsigned int us);

#define crypto_get_phys_addr(addr)      (unsigned long long)(uintptr_t)(addr)

/* cache. */
#define crypto_cache_flush(...)
#define crypto_cache_all(...)

/* Memory. */
void *crypto_malloc(unsigned int size);
void crypto_free(const void *ptr);

#define crypto_malloc_coherent(size, name)      crypto_malloc(size)
#define CRYPTO_FREE_COHERENT                    crypto_free
#define crypto_malloc_mmz(size, name)           crypto_malloc(size)

#define crypto_ioremap_nocache(phys_addr, size)     (void *)(uintptr_t)(phys_addr)
#define crypto_iounmap(virt_addr, size)

#define crypto_sm3_support()            TD_TRUE
#define crypto_rsa_support(...)         TD_TRUE

#endif
