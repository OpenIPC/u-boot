/*
 * Copyright (c) @CompanyNameMagicTag 2024-2024. All rights reserved.
 * Description: Secure Verify
 * Author: @CompanyNameTag
 * Create: 2024-03-21
 */
#include "verify_boot.h"
#include "ot_mpi_cipher.h"

#define SM2_ID_LEN 16
static uint8_t sm2_id_buf[SM2_ID_LEN] = "1234567812345678";

errcode_t crypto_sm2_verify(const uint8_t *key, uint32_t key_length,
    const uint8_t *data, uint32_t data_length, const uint8_t *sign_buff, uint32_t sign_length)
{
    uint32_t ret = 0;
    uint8_t hash_result[HASH_LEN];
    drv_pke_ecc_point input_pub_key = {0};
    drv_pke_data input_hash = {0};
    drv_pke_ecc_sig input_sig = {0};
    drv_pke_data sm2_id = {0};
    drv_pke_msg msg = {0};

    if (key_length != SM2_PUBLIC_KEY_LEN) {
        printf("key_length err!");
        return ERRCODE_FAIL;
    }

    sm2_id.data = sm2_id_buf;
    sm2_id.length = SM2_ID_LEN;
    input_pub_key.x = (uint8_t *)key;
    input_pub_key.y = (uint8_t *)(key + SM2_PUBLIC_KEY_LEN);
    input_pub_key.length = SM2_PUBLIC_KEY_LEN;
    msg.data = (uint8_t *)data;
    msg.length = data_length;
    input_hash.data = hash_result;
    input_hash.length = HASH_LEN;

    ret = ot_mpi_cipher_pke_sm2_dsa_hash(&sm2_id, &input_pub_key, &msg, &input_hash);
    if (ret != ERRCODE_SUCC) {
        printf("secure_authenticate crypto_hash_sha256 fail!");
        return ERRCODE_FAIL;
    }

    input_sig.r = (uint8_t *)sign_buff;
    input_sig.s = (uint8_t *)(sign_buff + SM2_PUBLIC_KEY_LEN);
    input_sig.length = sign_length;

    ret = ot_mpi_cipher_pke_ecdsa_verify(DRV_PKE_ECC_TYPE_SM2, &input_pub_key, &input_hash, &input_sig);
    
    return ret;
}