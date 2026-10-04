/*
 * Copyright (c) @CompanyNameMagicTag 2024-2024. All rights reserved.
 * Description: Secure Verify
 * Author: @CompanyNameTag
 * Create: 2024-03-21
 */
#include "verify_boot.h"
#include "ot_mpi_cipher.h"

errcode_t crypto_hash_calc(const unsigned char *data, unsigned int data_length, unsigned char *hash,
    unsigned int hash_length, crypto_hash_type hash_type)
{
    td_handle drv_hash_handle = 0;
    errcode_t ret;
    crypto_hash_attr hash_attr = {0};
    crypto_buf_attr src_buf = {0};
    hash_attr.hash_type = hash_type;

    ret = ot_mpi_cipher_hash_create(&drv_hash_handle, &hash_attr);
    if (ret != ERRCODE_SUCC) {
        printf("ot_mpi_cipher_hash_init fail, ret is 0x%x\n", ret);
        return ret;
    }

    src_buf.virt_addr = (unsigned char *)data;
    ret = ot_mpi_cipher_hash_update(drv_hash_handle, &src_buf, data_length);
    if (ret != ERRCODE_SUCC) {
        printf("ot_mpi_cipher_hash_update fail, ret is 0x%x\n", ret);
        goto exit;
    }

    ret = ot_mpi_cipher_hash_finish(drv_hash_handle, hash, hash_length, &hash_length);
    if (ret != ERRCODE_SUCC) {
        printf("ot_mpi_cipher_hash_finish fail, ret is 0x%x\n", ret);
        goto exit;
    }
    return ERRCODE_SUCC;
exit:
    ot_mpi_cipher_hash_destroy(drv_hash_handle);
    return ret;
}