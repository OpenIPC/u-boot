/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include <common.h>
#include <cpu_func.h>
#include "crypto_drv_common.h"
#include "drv_trng.h"
#include "crypto_common_struct.h"
#include "crypto_osal_lib.h"

td_s32 crypto_get_random(td_u32 *randnum)
{
    return drv_cipher_trng_get_random(randnum);
}

#define OTP_BASE_ADDR 0x101E0000
#define OTP_REG_SIZE 0x100
#define OTP_SM2_BIT_OFFSET 25
#define OTP_SM3_BIT_OFFSET 26
#define OTP_SM4_BIT_OFFSET 27

void crypto_cache_flush(uintptr_t base_addr, td_u32 size)
{
    flush_cache(base_addr, size);
}

void crypto_dcache_enable(void)
{
    dcache_enable();
}

void crypto_dcache_disable(void)
{
    dcache_disable();
}

#define CPU_ID_STAT                     (0x0018)
#define CRYPTO_CPU_ID_SCPU              (0xa5)
#define CRYPTO_CPU_ID_ACPU              (0xaa)

crypto_cpu_type crypto_get_cpu_type(td_void)
{
    td_u32 cpu_id = ca_misc_reg_read(CPU_ID_STAT) & 0x00ff;
    if (cpu_id == CRYPTO_CPU_ID_SCPU) {
        return CRYPTO_CPU_TYPE_SCPU;
    } else if (cpu_id == CRYPTO_CPU_ID_ACPU) {
        return CRYPTO_CPU_TYPE_ACPU;
    }
    return CRYPTO_CPU_TYPE_INVALID;
}

td_s32 crypto_copy_from_phys_addr(td_u8 *dst, td_u32 dst_len, td_u64 src_phys_addr, td_u32 src_len)
{
    crypto_unused(dst_len);
    return crypto_phys_copy_to_virt(dst, src_phys_addr, src_len);
}

td_void *crypto_malloc_coherent(td_u32 size, td_char *buffer_name)
{
    void *ptr = NULL;
    crypto_unused(buffer_name);
    ptr = crypto_malloc(size);
    return ptr;
}

td_void *crypto_malloc_mmz(td_u32 size, td_char *buffer_name)
{
    return crypto_malloc_coherent(size, buffer_name);
}

td_void crypto_free_coherent(td_void *ptr)
{
    crypto_free(ptr);
}

td_phys_addr_t crypto_get_phys_addr(td_void *ptr)
{
    return (td_phys_addr_t)(uintptr_t)ptr;
}

#if defined(CONFIG_SYMC_CTR_NON_ALIGN_SUPPORT)
td_s32 crypto_virt_xor_phys_copy_to_phys(td_u64 dst_phys_addr, const td_u8 *a_virt_addr,
    td_u64 b_phys_addr, td_u32 length)
{
    td_s32 ret;
    td_u32 i;
    td_u8 *b_virt_addr = TD_NULL;
    td_u8 *dst_virt_addr = TD_NULL;

    b_virt_addr = crypto_ioremap_nocache(b_phys_addr, length);
    crypto_chk_goto_with_ret(ret, b_virt_addr == TD_NULL, exit, TD_FAILURE, "crypto_ioremap_nocache failed\n");

    dst_virt_addr = crypto_ioremap_nocache(dst_phys_addr, length);
    crypto_chk_goto_with_ret(ret, dst_virt_addr == TD_NULL, exit, TD_FAILURE, "crypto_ioremap_nocache failed\n");

#if defined(CRYPTO_CTR_TRACE_ENABLE)
    crypto_dump_data("a_virt_addr", a_virt_addr, length);
    crypto_dump_data("b_virt_addr", b_virt_addr, length);
#endif

    for (i = 0; i < length; i++) {
        dst_virt_addr[i] = a_virt_addr[i] ^ b_virt_addr[i];
    }

#if defined(CRYPTO_CTR_TRACE_ENABLE)
    crypto_dump_data("dst_virt_addr", dst_virt_addr, length);
#endif
    ret = TD_SUCCESS;
exit:
    if (b_virt_addr != TD_NULL) {
        crypto_iounmap(b_virt_addr, length);
    }
    if (dst_virt_addr != TD_NULL) {
        crypto_iounmap(dst_virt_addr, length);
    }
    return ret;
}
#endif

td_s32 crypto_phys_copy_to_virt(td_u8 *dst_virt_addr,
    td_phys_addr_t src_phys_addr, td_u32 length)
{
    td_s32 ret;
    td_u8 *src_virt_addr = TD_NULL;

    src_virt_addr = crypto_ioremap_nocache(src_phys_addr, length);
    crypto_chk_goto_with_ret(ret, src_virt_addr == TD_NULL, exit, TD_FAILURE, "crypto_ioremap_nocache failed\n");

    ret = memcpy_s(dst_virt_addr, length, src_virt_addr, length);
    crypto_chk_goto_with_ret(ret, ret != EOK, exit, TD_FAILURE, "memcpy_s failed\n");

    ret = TD_SUCCESS;
exit:
    if (src_virt_addr != TD_NULL) {
        crypto_iounmap(src_virt_addr, length);
    }
    return ret;
}