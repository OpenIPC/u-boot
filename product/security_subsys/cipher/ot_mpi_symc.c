/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "ot_mpi_cipher.h"
#include "crypto_common_macro.h"
#include "drv_symc.h"

td_s32 ot_mpi_cipher_symc_init(td_void)
{
    return TD_SUCCESS;
}

td_s32 ot_mpi_cipher_symc_deinit(td_void)
{
    return TD_SUCCESS;
}

td_s32 ot_mpi_cipher_symc_create(td_handle *symc_handle, const crypto_symc_attr *symc_attr)
{
    return drv_cipher_symc_create(symc_handle, symc_attr);
}

td_s32 ot_mpi_cipher_symc_destroy(td_handle symc_handle)
{
    return drv_cipher_symc_destroy(symc_handle);
}

td_s32 ot_mpi_cipher_symc_set_config(td_handle symc_handle, const crypto_symc_ctrl_t *symc_ctrl)
{
    return drv_cipher_symc_set_config(symc_handle, symc_ctrl);
}

td_s32 ot_mpi_cipher_symc_attach(td_handle symc_handle, td_handle keyslot_handle)
{
    return drv_cipher_symc_attach(symc_handle, keyslot_handle);
}

td_s32 ot_mpi_cipher_symc_encrypt(td_handle symc_handle, const crypto_buf_attr *src_buf,
    const crypto_buf_attr *dst_buf, td_u32 length)
{
    return drv_cipher_symc_encrypt(symc_handle, src_buf, dst_buf, length);
}

td_s32 ot_mpi_cipher_symc_decrypt(td_handle symc_handle, const crypto_buf_attr *src_buf,
    const crypto_buf_attr *dst_buf, td_u32 length)
{
    return drv_cipher_symc_decrypt(symc_handle, src_buf, dst_buf, length);
}

td_s32 ot_mpi_cipher_symc_get_tag(td_handle symc_handle, td_u8 *tag, td_u32 tag_length)
{
    return drv_cipher_symc_get_tag(symc_handle, tag, tag_length);
}

td_s32 ot_mpi_cipher_mac_start(td_handle *symc_handle, const crypto_symc_mac_attr *mac_attr)
{
    return drv_cipher_mac_start(symc_handle, mac_attr);
}

td_s32 ot_mpi_cipher_mac_update(td_handle symc_handle, const crypto_buf_attr *src_buf, td_u32 length)
{
    return drv_cipher_mac_update(symc_handle, src_buf, length);
}

td_s32 ot_mpi_cipher_mac_finish(td_handle symc_handle, td_u8 *mac, td_u32 *mac_length)
{
    return drv_cipher_mac_finish(symc_handle, mac, mac_length);
}

td_s32 ot_mpi_cipher_symc_encrypt_multi(td_handle symc_handle, const crypto_symc_ctrl_t *symc_ctrl,
    const crypto_symc_pack *src_buf_pack, const crypto_symc_pack *dst_buf_pack, td_u32 pack_num)
{
    crypto_unused(symc_handle);
    crypto_unused(symc_ctrl);
    crypto_unused(src_buf_pack);
    crypto_unused(dst_buf_pack);
    crypto_unused(pack_num);

    return ERROR_UNSUPPORT;
}

td_s32 ot_mpi_cipher_symc_decrypt_multi(td_handle symc_handle, const crypto_symc_ctrl_t *symc_ctrl,
    const crypto_symc_pack *src_buf_pack, const crypto_symc_pack *dst_buf_pack, td_u32 pack_num)
{
    crypto_unused(symc_handle);
    crypto_unused(symc_ctrl);
    crypto_unused(src_buf_pack);
    crypto_unused(dst_buf_pack);
    crypto_unused(pack_num);

    return ERROR_UNSUPPORT;
}