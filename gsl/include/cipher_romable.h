/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
 */
#ifndef CIPHER_ROMABLE_H
#define CIPHER_ROMABLE_H

#include "td_type.h"
#include "types.h"

typedef enum {
	EXT_SYMC_ALG_TDES = 0x0,
	EXT_SYMC_ALG_AES = 0x1,
	EXT_SYMC_ALG_SM4 = 0x2,
	EXT_SYMC_ALG_LEA = 0x3,
	EXT_SYMC_ALG_DMA = 0x4,
	EXT_SYMC_ALG_MAX,
	EXT_SYMC_ALG_INVALID = 0xffffffff,
} ext_symc_alg;

typedef struct {
	unsigned char *x;
	unsigned char *y;
	unsigned int length;
} ext_pke_ecc_point;

typedef struct {
	unsigned int length;
	unsigned char *data;
} ext_pke_data;

typedef struct {
	unsigned char *r;
	unsigned char *s;
	unsigned int length;
} ext_pke_ecc_sig;

typedef struct {
	unsigned char *n;
	unsigned char *e;
	unsigned short len;
} ext_pke_rsa_pub_key;

typedef enum {
	EXT_PKE_RSA_SCHEME_PKCS1_V15 = 0x00, /* PKCS#1 V15 */
	EXT_PKE_RSA_SCHEME_PKCS1_V21, /* PKCS#1 V21, PSS for signning, OAEP for encryption */
	EXT_PKE_RSA_SCHEME_MAX,
	EXT_PKE_RSA_SCHEME_INVALID = 0xffffffff,
} ext_pke_rsa_scheme;

typedef enum {
	EXT_PKE_HASH_TYPE_SHA1 = 0x00, /* Suggest Not to use */
	EXT_PKE_HASH_TYPE_SHA224,
	EXT_PKE_HASH_TYPE_SHA256,
	EXT_PKE_HASH_TYPE_SHA384,
	EXT_PKE_HASH_TYPE_SHA512,
	EXT_PKE_HASH_TYPE_SM3,
	EXT_PKE_HASH_TYPE_MAX,
	EXT_PKE_HASH_TYPE_INVALID = 0xffffffff,
} ext_pke_hash_type;

typedef enum {
	EXT_KM_ROOTKEY_TYPE_ERK_TEE = 0x00,
	EXT_KM_ROOTKEY_TYPE_ERK_REE,
	EXT_KM_ROOTKEY_TYPE_MAX,
	EXT_KM_ROOTKEY_TYPE_INVALID = 0xffffffff,
} ext_km_rootkey_type;

typedef void *(*ext_drv_func_malloc)(unsigned int size);
typedef void (*ext_drv_func_free)(const void *ptr);
typedef void (*ext_drv_func_udelay)(unsigned int us);

typedef struct {
	ext_drv_func_malloc malloc;
	ext_drv_func_free free;
	ext_drv_func_udelay udelay;
} ext_drv_func;

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

int32_t ext_drv_cipher_func_register(const ext_drv_func *func);

int32_t ext_drv_cipher_init(void);

int32_t ext_drv_cipher_deinit(void);

int32_t ext_drv_cipher_create_keyslot_by_set_effective_key(
	unsigned int *keyslot_handle, ext_km_rootkey_type type,
	const uint8_t *salt, uint32_t salt_len);

int32_t ext_drv_cipher_destroy_keyslot(unsigned int keyslot_handle);

int32_t ext_drv_cipher_otp_read_word(const uint32_t addr, uint32_t *data);

int32_t ext_drv_cipher_trng_get_random(uint32_t *random);

int32_t ext_drv_cipher_dma_copy(uint64_t src, uint64_t dst, uint32_t length);

int32_t ext_drv_cipher_symc_decrypt(ext_symc_alg symc_alg,
				    uint32_t keyslot_chn_num, uint8_t *iv,
				    uint32_t iv_length, uint64_t src,
				    uint64_t dst, uint32_t length);

int32_t ext_drv_cipher_sha256(const uint8_t *data, uint32_t data_len,
			      uint8_t *hash, uint32_t hash_len);

int32_t ext_drv_cipher_sm3(const uint8_t *data, uint32_t data_len,
			   uint8_t *hash, uint32_t hash_len);

int32_t ext_drv_cipher_pke_sm2_dsa_hash(const ext_pke_data *sm2_id,
					const ext_pke_ecc_point *pub_key,
					const ext_pke_data *msg,
					ext_pke_data *hash);

int32_t ext_drv_cipher_pke_sm2_verify(const ext_pke_ecc_point *pub_key,
				      const ext_pke_data *hash,
				      const ext_pke_ecc_sig *sig);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
