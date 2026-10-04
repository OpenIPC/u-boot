/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
 */

#ifndef DRIVER_CIPHER_COMMON_DRV_COMMON_H
#define DRIVER_CIPHER_COMMON_DRV_COMMON_H
#include "soc_errno.h"
#include "common.h"
#include "platform.h"

#define ALWAYS_INLINE inline __attribute__((always_inline))

/* Define the maximum number of HMAC keyslot. */
#define EXT_DRV_KEYSLOT_HMAC_MAX 2

#define ZERO			 0
#define WORD_INDEX_0		 0
#define WORD_INDEX_1		 1
#define WORD_INDEX_2		 2
#define WORD_INDEX_3		 3

/* Boundary value 1. */
#define BOUND_VALUE_1 1

/* multiple value */
#define MULTIPLE_VALUE_1	1
#define MULTIPLE_VALUE_2	2

#define SHIFT_4BITS		4
#define SHIFT_8BITS		8
#define SHIFT_16BITS		16
#define SHIFT_24BITS		24
#define MAX_LOW_2BITS		3
#define MAX_LOW_3BITS		7
#define MAX_LOW_4BITS		0xF
#define MAX_LOW_8BITS		0xFF
#define BYTE_BITS		8

#define SYM_LOCAL_STEP_CNT_INIT 0x96C3785A
#define SYM_LOCAL_STEP_AUTH	11

#define SOC_TEE_ENABLE_REG	(OTPC_BASE_ADDR + 0x0010)

#define TEE_NOT_ENABLE		0x42
/* Define the union U_OTP_BYTE_ALIGNED_LOCKABLE_0 */
void drv_serial_putc(const char c);
void drv_serial_puts(const char *s);
void drv_serial_put_hex(unsigned int hex);
td_u32 drv_msleep(td_u32 ms);

void *drv_malloc(const td_u32 size);
void drv_free(void *ptr);

void drv_cache_flush(uintptr_t base_addr, td_u32 size);
void drv_cache_inv(uintptr_t base_addr, td_u32 size);

typedef struct {
	unsigned int length;
	unsigned char *data;
} ext_data;

#define RSA_KEY_LEN    384
#define RSA_KEY_OFFSET 384
#define RSA_SIGN_LEN   384
#define SHA_256_LEN    32
#endif
