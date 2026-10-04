/*
 * Copyright (c) @CompanyNameMagicTag 2024-2024. All rights reserved.
 * Description: Secure Verify
 * Author: @CompanyNameTag
 * Create: 2024-03-21
 */
#include "verify_boot.h"
#include "securec.h"
#include "ot_mpi_cipher.h"

#define E_LEN 4

errcode_t crypto_rsa_verify(const uint8_t *key, uint32_t key_length,
    const uint8_t *data, uint32_t data_length, const uint8_t *sign_buff, uint32_t sign_length)
{
    uint32_t ret = 0;
    uint8_t hash_result[HASH_LEN];
    uint8_t exponent[RSA_PUBLIC_KEY_LEN - E_LEN] = {0};    /* 388 - 4 */
    drv_pke_rsa_pub_key input_pub_key = { 0 };
    drv_pke_data input_hash = { 0 };
    drv_pke_data input_sig = { 0 };
    
    if (key_length != RSA_PUBLIC_KEY_LEN) {
        printf("key_length err!");
        return ERRCODE_FAIL;
    }

    ret = crypto_hash_calc(data, data_length, hash_result, (uint32_t)HASH_LEN, CRYPTO_HASH_TYPE_SHA256);
    if (ret != ERRCODE_SUCC) {
        printf("secure_authenticate crypto_hash_sha256 fail!");
        return ERRCODE_FAIL;
    }

    ret = memcpy_s(exponent + RSA_PUBLIC_KEY_LEN - E_LEN - E_LEN, E_LEN, key + RSA_PUBLIC_KEY_LEN - E_LEN, E_LEN);
    if (ret != ERRCODE_SUCC) {
        printf("memcpy_s public key fail!");
        return ERRCODE_FAIL;
    }

    input_pub_key.n = (uint8_t *)key;
    input_pub_key.e = (uint8_t *)exponent;
    input_pub_key.len = RSA_PUBLIC_KEY_LEN - E_LEN;
    input_hash.data = hash_result;
    input_hash.length = HASH_LEN;
    input_sig.data = (uint8_t *)sign_buff;
    input_sig.length = sign_length;

    ret = ot_mpi_cipher_pke_rsa_verify(&input_pub_key, DRV_PKE_RSA_SCHEME_PKCS1_V21, DRV_PKE_HASH_TYPE_SHA256,
        &input_hash, &input_sig);
        
    return ret;
}