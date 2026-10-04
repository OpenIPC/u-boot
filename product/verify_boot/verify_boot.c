/*
 * Copyright (c) @CompanyNameMagicTag 2024-2024. All rights reserved.
 * Description: Secure Verify
 * Author: @CompanyNameTag
 * Create: 2024-03-21
 */

#include "verify_boot.h"
#include "securec.h"
#include "ot_mpi_cipher.h"

/* Public Key goes here. */
const unsigned char g_root_pub_key[MAX_PUBLIC_KEY_LEN] = {
    /* RSA3072: N(384 Bytes) + E(4 Bytes). */
    /* SM2: X(32 Bytes) + Y(32 Bytes). */
};

errcode_t secure_boot_verify(const image_key_area_t *app_key_area,
    const image_code_info_t *app_code_info, const uint8_t *app_code)
{
    int8_t ret;
    uint8_t code_hash[HASH_LEN] = {0};
    const uint8_t *ext_pubkey = app_key_area->ext_pulic_key_area;

#if defined(CONFIG_VERIFY_KEY_AREA_SUPPORT)
    /* verify key area. */
    if (app_key_area->key_alg == KEY_ALG_SM2) {
        ret = crypto_sm2_verify(g_root_pub_key, SM2_PUBLIC_KEY_LEN,
            (uint8_t *)app_key_area, KEY_AREA_STRUCTURE_LENGTH - MAX_SIG_LEN, app_key_area->sig_key_area, SM2_SIG_LEN);
    } else {
        ret = crypto_rsa_verify(g_root_pub_key, RSA_PUBLIC_KEY_LEN,
            (uint8_t *)app_key_area, KEY_AREA_STRUCTURE_LENGTH - MAX_SIG_LEN, app_key_area->sig_key_area, RSA_SIG_LEN);
    }
    if (ret != ERRCODE_SUCC) {
        printf("secure_authenticate for key area fail!\n");
        return ret;
    }
    printf("Verify key area success!\n");
#else
    ext_pubkey = g_root_pub_key;
#endif

#if defined(CONFIG_VERIFY_KEY_AREA_SUPPORT)
    /* verify code info area. */
    if (app_key_area->key_alg == KEY_ALG_SM2) {
        ret = crypto_sm2_verify(ext_pubkey, SM2_PUBLIC_KEY_LEN,
            (uint8_t *)(uintptr_t)app_code_info, CODE_INFO_STRUCTURE_LENGTH - MAX_SIG_LEN - MAX_EXT_SIG_LEN,
            app_code_info->sig_code_info, SM2_SIG_LEN);
    } else {
        ret = crypto_rsa_verify(ext_pubkey, RSA_PUBLIC_KEY_LEN,
            (uint8_t *)(uintptr_t)app_code_info, CODE_INFO_STRUCTURE_LENGTH - MAX_SIG_LEN - MAX_EXT_SIG_LEN,
            app_code_info->sig_code_info, RSA_SIG_LEN);
    }
#else
        ret = crypto_sm2_verify(ext_pubkey, SM2_PUBLIC_KEY_LEN,
            (uint8_t *)(uintptr_t)app_code_info, CODE_INFO_STRUCTURE_LENGTH - MAX_SIG_LEN - MAX_EXT_SIG_LEN,
            app_code_info->sig_code_info, SM2_SIG_LEN);
#endif
    if (ret != ERRCODE_SUCC) {
        printf("secure_authenticate for code info area fail!\n");
        return ret;
    }
    printf("Verify code info area success!\n");

    /* decrypt code area data */
    if (app_code_info->code_enc_flag == CODE_ENC_FLAG) {
        ret = crypto_sm4_dec(app_code_info->protection_key_l1, app_code_info->iv,
            app_code, (uint8_t *)app_code, app_code_info->code_area_len);
        if (ret != ERRCODE_SUCC) {
            printf("app code decrypt fail!\n");
            return ret;
        }
        printf("app code decrypt success!\n");
    }

    /* compare code area hash */
    if (app_key_area->key_alg == KEY_ALG_SM2) {
        ret = crypto_hash_calc(app_code, app_code_info->code_area_len, code_hash, HASH_LEN, CRYPTO_HASH_TYPE_SM3);
    } else {
        ret = crypto_hash_calc(app_code, app_code_info->code_area_len, code_hash, HASH_LEN, CRYPTO_HASH_TYPE_SHA256);
    }
    if (ret != ERRCODE_SUCC) {
        printf("crypto_hash_calc fail!\n");
        return ret;
    }

    ret = memcmp(code_hash, app_code_info->code_area_hash, HASH_LEN);
    if (ret != ERRCODE_SUCC) {
        printf("app code hash compare fail!\n");
        return ret;
    }
    printf("Verify code area success!\n");



    return ERRCODE_SUCC;
}