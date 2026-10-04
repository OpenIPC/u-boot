/*
 * Copyright (c) @CompanyNameMagicTag 2024-2024. All rights reserved.
 * Description: Secure Verify
 * Author: @CompanyNameTag
 * Create: 2024-03-21
 */

#ifndef VERIFY_BOOT_H
#define VERIFY_BOOT_H

#include <common.h>
#include "ot_mpi_cipher.h"

typedef uint32_t errcode_t;

#define ERRCODE_SUCC        0
#define ERRCODE_FAIL        0xFFFFFFFF

#define ROOT_PUBLIC_KEY_STRUCTURE_LENGTH 0x200
#define KEY_AREA_STRUCTURE_LENGTH        0x400
#define CODE_INFO_STRUCTURE_LENGTH       0x400
#define RSA_PUBLIC_KEY_LEN                   388    /* n:384 bytes, e:4 bytes */
#define RSA_SIG_LEN                          384

#define KEY_ALG_SM2                          0x2A13C823
#define SM2_PUBLIC_KEY_LEN                   32
#define SM2_SIG_LEN                          32

#define MAX_PUBLIC_KEY_LEN                   388
#define MAX_EXT_SIG_LEN                      384
#define MAX_SIG_LEN                          384

#define CODE_ENC_FLAG                   0x3C7896E1
#define DIE_ID_LEN                      16      /* DIEID has 160 bits, but we just use 128 bits */
#define HASH_LEN                        32
#define PROTECT_KEY_LEN                 16
#define IV_LEN                          16

typedef enum {
    LOADER_BOOT_TYPE = 0,
    PARAMS_BOOT_TYPE = 1,
    FLASH_BOOT_TYPE = 2,
    FLASH_BOOT_BACK_UP_TYPE = 3,
    APP_BOOT_TYPE = 4,
    INVALID_BOOT_TYPE
} image_type_t;

typedef enum {
    KEY_EREA_TYPE = 0,
    CODE_INFO_TYPE = 1,
    PARAMS_KEY_AREA_TYPE = 2,
    PARAMS_AREA_IOFO_TYPE = 3,
    INVALID_EREA_TYPE
} area_type_t;

/* root public key area, size is 0x80 */
typedef struct {
    uint32_t      image_id;
    uint32_t      structure_version;  /* currently version is 0x00010000 */
    uint32_t      structure_length;
    uint32_t      key_owner_id;
    uint32_t      key_id;
    uint32_t      key_alg;            /* 0x2A13C812: ECC256;  0x2A13C823: SM2 */
    uint32_t      ecc_curve_type;     /* 0x2A13C812: RFC 5639, BrainpoolP256r1 */
    uint32_t      key_length;
    uint8_t       reserved[ROOT_PUBLIC_KEY_STRUCTURE_LENGTH - 32 - MAX_PUBLIC_KEY_LEN]; /* 32 bytes above */
    uint8_t       root_key_area[MAX_PUBLIC_KEY_LEN];
} root_public_key_area_t;

/* Params key area structure, size is 0x100 */
typedef struct {
    uint32_t      image_id;
    uint32_t      structure_version;  /* currently version is 0x00010000 */
    uint32_t      structure_length;
    uint32_t      signature_length;
    uint32_t      key_owner_id;
    uint32_t      key_id;
    uint32_t      key_alg;            /* 0x2A13C812: ECC256;  0x2A13C823: SM2 */
    uint32_t      ecc_curve_type;     /* 0x2A13C812: RFC 5639, BrainpoolP256r1 */
    uint32_t      key_length;
    uint32_t      params_key_version_ext;
    uint32_t      mask_params_key_version_ext;
    uint32_t      msid_ext;
    uint32_t      mask_msid_ext;
    uint32_t      maintenance_mode;   /* 0x3C7896E1: enable */
    uint8_t       die_id[DIE_ID_LEN];
    uint32_t      params_info_addr;   /* 0 means followed Params Key Area */
    uint8_t       reserved[KEY_AREA_STRUCTURE_LENGTH - 76 - MAX_PUBLIC_KEY_LEN - MAX_SIG_LEN]; /* 76 bytes above */
    uint8_t       params_ext_key_area[MAX_PUBLIC_KEY_LEN];
    uint8_t       sig_params_key_area[MAX_SIG_LEN];
} params_key_area_t;

/* Params area info, size is 0x200 */
typedef struct {
    uint32_t      image_id;
    uint32_t      structure_version;  /* currently version is 0x00010000 */
    uint32_t      structure_length;  /* ecc/sm2 is 0x200, rsa3072 is 0x400 */
    uint32_t      signature_length;
    uint32_t      params_version_ext;
    uint32_t      mask_params_version_ext;
    uint32_t      msid_ext;
    uint32_t      mask_msid_ext;
    uint32_t      params_area_addr;   /* 0 means followed Params Area info */
    uint32_t      params_area_len;
    uint8_t       params_area_hash[HASH_LEN];
    uint8_t       reserved[CODE_INFO_STRUCTURE_LENGTH - 72 - MAX_SIG_LEN - MAX_EXT_SIG_LEN];      /* 72 bytes above */
    uint8_t       sig_params_info[MAX_SIG_LEN];
    uint8_t       sig_params_info_ext[MAX_EXT_SIG_LEN];
} params_area_info_t;

/* Params area structure, size is depend on params_area_len */
typedef struct {
    uint32_t      image_id;
    uint32_t      structure_version;
    uint32_t      version;
    uint16_t      item_offset;
    uint8_t       item_count;
} params_area_head_t;

typedef struct {
    uint32_t      item_addr;
    uint32_t      item_size;
} item_structure_t;

/* Key area, size is 0x100 */
typedef struct {
    uint32_t      image_id;
    uint32_t      structure_version;
    uint32_t      structure_length;
    uint32_t      signature_length;
    uint32_t      key_owner_id;
    uint32_t      key_id;
    uint32_t      key_alg;            /* 0x2A13C812: ECC256;  0x2A13C823: SM2 ; 0x2A13C834: RSA3072 */
    uint32_t      ecc_curve_type;     /* 0x2A13C812: RFC 5639, BrainpoolP256r1 */
    uint32_t      key_length;
    uint32_t      key_version_ext;
    uint32_t      mask_key_version_ext;
    uint32_t      msid_ext;
    uint32_t      mask_msid_ext;
    uint32_t      maintenance_mode;   /* 0x3C7896E1: enable */
    uint8_t       die_id[DIE_ID_LEN];
    uint32_t      code_info_addr; /* 0 means followed image Key Area */
    uint8_t       reserved[KEY_AREA_STRUCTURE_LENGTH - 76 - MAX_PUBLIC_KEY_LEN - MAX_SIG_LEN]; /* 76 bytes above */
    uint8_t       ext_pulic_key_area[MAX_PUBLIC_KEY_LEN];
    uint8_t       sig_key_area[MAX_SIG_LEN];
} image_key_area_t;

/* Code area info, size is 0x200 */
typedef struct {
    uint32_t      image_id;
    uint32_t      structure_version;
    uint32_t      structure_length;
    uint32_t      signature_length;
    uint32_t      version_ext;
    uint32_t      mask_version_ext;
    uint32_t      msid_ext;
    uint32_t      mask_msid_ext;
    uint32_t      code_area_addr;
    uint32_t      code_area_len;
    uint8_t       code_area_hash[HASH_LEN];
    uint32_t      code_enc_flag;   /* 0x3C7896E1 code area is encrypt */
    uint8_t       protection_key_l1[PROTECT_KEY_LEN];
    uint8_t       protection_key_l2[PROTECT_KEY_LEN];
    uint8_t       iv[IV_LEN];
    uint32_t      code_compress_flag; /* 0x3C7896E1: is compressed */
    uint32_t      code_uncompress_len;
    uint32_t      text_segment_size;
    uint8_t       reserved[CODE_INFO_STRUCTURE_LENGTH - 136 - MAX_SIG_LEN - MAX_EXT_SIG_LEN];  /* 136 bytes above */
    uint8_t       sig_code_info[MAX_SIG_LEN];
    uint8_t       sig_code_info_ext[MAX_EXT_SIG_LEN];
} image_code_info_t;

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

errcode_t secure_boot_verify(const image_key_area_t *app_key_area,
    const image_code_info_t *app_code_info, const uint8_t *app_code);

errcode_t crypto_rsa_verify(const uint8_t *key, uint32_t key_length,
    const uint8_t *data, uint32_t data_length, const uint8_t *sign_buff, uint32_t sign_length);

errcode_t crypto_sm2_verify(const uint8_t *key, uint32_t key_length,
    const uint8_t *data, uint32_t data_length, const uint8_t *sign_buff, uint32_t sign_length);

errcode_t crypto_hash_calc(const unsigned char *data, unsigned int data_length, unsigned char *hash,
    unsigned int hash_length, crypto_hash_type hash_type);

errcode_t crypto_sm4_dec(const uint8_t *key, const uint8_t *iv,
    const uint8_t *code_src, uint8_t *code_dst, uint32_t data_length);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif