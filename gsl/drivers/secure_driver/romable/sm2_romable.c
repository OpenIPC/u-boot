/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
*/
#include "cipher_romable.h"

#include "drv_pke.h"

#define CRYPTO_PKE_MSG_MAX_SIZE (2 * 1024 * 1024) // max length: 2 MB

int32_t gsl_drv_cipher_pke_sm2_dsa_hash(const ext_pke_data *sm2_id, const ext_pke_ecc_point *pub_key,
    const ext_pke_data *msg, ext_pke_data *hash)
{
    drv_pke_msg sm2_msg = {0};
    if (msg == NULL || msg->data == NULL || msg->length > CRYPTO_PKE_MSG_MAX_SIZE) {
        return ERROR_INVALID_PARAM;
    }
    sm2_msg.data = msg->data;
    sm2_msg.length = msg->length;
    
    return drv_cipher_pke_sm2_dsa_hash(
        (const drv_pke_data *)sm2_id,
        (const drv_pke_ecc_point *)pub_key,
        (const drv_pke_msg *)&sm2_msg,
        (drv_pke_data *)hash
    );
}

int32_t gsl_drv_cipher_pke_sm2_verify(const ext_pke_ecc_point *pub_key, const ext_pke_data *hash,
    const ext_pke_ecc_sig *sig)
{
    return drv_cipher_pke_sm2_verify(
        (const drv_pke_ecc_point *)pub_key,
        (const drv_pke_data *)hash,
        (const drv_pke_ecc_sig *)sig
    );
}