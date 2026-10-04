/*
 * Copyright (c) @CompanyNameMagicTag 2024-2024. All rights reserved.
 * Description: Secure Verify
 * Author: @CompanyNameTag
 * Create: 2024-03-21
 */
#include "securec.h"
#include "ot_mpi_cipher.h"
#include "ot_mpi_km.h"
#include "ot_mpi_otp.h"
#include "verify_boot.h"

#define SM4_KEY_LEN 16

#if defined(CONFIG_SM4_DECRYPT_SUPPORT)
#if defined(CONFIG_KM_CONTENT_KEY_SUPPORT)

static errcode_t cipher_set_rootkey(uint32_t keyslot_handle, const uint8_t *key)
{
    uint32_t ret = 0;
    uint32_t klad_handle = 0;
    uint32_t offset = 0x12;
    uint8_t tee_enable = 0;
    km_klad_attr klad_attr = {
        .klad_cfg = {
            .rootkey_type = KM_KLAD_KEY_TYPE_ABRK1
        },
        .key_cfg = {
            .engine = KM_CRYPTO_ALG_SM4,
            .decrypt_support = TD_TRUE,
            .encrypt_support = TD_TRUE
        }
    };
    km_klad_session_key klad_session_key = {
        .level = KM_KLAD_LEVEL1,
        .alg = KM_KLAD_ALG_TYPE_AES,
        .key_size = PROTECT_KEY_LEN,
        .key = (uint8_t *)key
    };

    km_klad_content_key klad_content_key = {
        .alg = KM_KLAD_ALG_TYPE_AES,
        .key_size = PROTECT_KEY_LEN,
        .key = (uint8_t *)(key + PROTECT_KEY_LEN)
    };

    (void)ot_mpi_otp_init();
    (void)ot_mpi_otp_read_byte(offset, &tee_enable);
    (void)ot_mpi_otp_deinit();
    if (tee_enable == 0x42) {
        klad_attr.key_sec_cfg.key_sec = KM_KLAD_SEC_ENABLE;
        klad_attr.key_sec_cfg.master_only_enable = TD_TRUE;
        klad_attr.key_sec_cfg.dest_buf_sec_support = TD_TRUE;
        klad_attr.key_sec_cfg.src_buf_sec_support = TD_TRUE;
        klad_attr.key_sec_cfg.src_buf_non_sec_support = TD_FALSE;
        klad_attr.key_sec_cfg.dest_buf_non_sec_support = TD_FALSE;
    } else {
        klad_attr.key_sec_cfg.key_sec = KM_KLAD_SEC_DISABLE;
        klad_attr.key_sec_cfg.master_only_enable = TD_FALSE;
        klad_attr.key_sec_cfg.dest_buf_sec_support = TD_FALSE;
        klad_attr.key_sec_cfg.dest_buf_non_sec_support = TD_TRUE;
        klad_attr.key_sec_cfg.src_buf_sec_support = TD_FALSE;
        klad_attr.key_sec_cfg.src_buf_non_sec_support = TD_TRUE;
    }

    ret = ot_mpi_klad_create(&klad_handle);
    if (ret != ERRCODE_SUCC) {
        printf("ot_mpi_klad_create fail, ret is 0x%x\n", ret);
        return ERRCODE_FAIL;
    }

    ret = ot_mpi_klad_set_attr(klad_handle, &klad_attr);
    if (ret != ERRCODE_SUCC) {
        printf("ot_mpi_klad_set_attr fail, ret is 0x%x\n", ret);
        goto klad_destory;
    }

    ret = ot_mpi_klad_attach(klad_handle, KM_KLAD_DEST_TYPE_MCIPHER, keyslot_handle);
    if (ret != ERRCODE_SUCC) {
        printf("ot_mpi_klad_attach fail, ret is 0x%x\n", ret);
        goto klad_destory;
    }

    ret = ot_mpi_klad_set_session_key(klad_handle, &klad_session_key);
    if (ret != ERRCODE_SUCC) {
        printf("ot_mpi_klad_set_session_key fail, ret is 0x%x\n", ret);
        goto klad_detach;
    }

    ret = ot_mpi_klad_set_content_key(klad_handle, &klad_content_key);
    if (ret != ERRCODE_SUCC) {
        printf("ot_mpi_klad_set_content_key fail, ret is 0x%x\n", ret);
        goto klad_detach;
    }
    return ERRCODE_SUCC;

klad_detach:
    ot_mpi_klad_detach(klad_handle, KM_KLAD_DEST_TYPE_MCIPHER, keyslot_handle);
klad_destory:
    ot_mpi_klad_destroy(klad_handle);
    return ret;
}

errcode_t crypto_sm4_dec(const uint8_t *key, const uint8_t *iv,
    const uint8_t *code_src, uint8_t *code_dst, uint32_t data_length)
{
    uint32_t ret = 0;
    uint32_t symc_handle = 0;
    uint32_t keyslot_handle = 0;
    crypto_buf_attr src_buf = {0};
    crypto_buf_attr dst_buf = {0};
    crypto_symc_attr symc_attr;
    crypto_symc_ctrl_t symc_ctrl;

    symc_attr.symc_type = CRYPTO_SYMC_TYPE_NORMAL;
    symc_attr.is_long_term = TD_FALSE;
    symc_ctrl.symc_alg = CRYPTO_SYMC_ALG_SM4;
    symc_ctrl.work_mode = CRYPTO_SYMC_WORK_MODE_CBC;
    symc_ctrl.symc_key_length = CRYPTO_SYMC_KEY_128BIT;
    symc_ctrl.symc_bit_width = CRYPTO_SYMC_BIT_WIDTH_128BIT;
    symc_ctrl.iv_length = IV_LEN;

    ret = memcpy_s(symc_ctrl.iv, IV_LEN, iv, IV_LEN);
    if (ret != ERRCODE_SUCC) {
        printf("memcpy_s iv fail!");
        return ERRCODE_FAIL;
    }

    ret = ot_mpi_cipher_symc_create(&symc_handle, &symc_attr);
    if (ret != ERRCODE_SUCC) {
        printf("ot_mpi_cipher_symc_create fail, ret is 0x%x\n", ret);
        return ret;
    }

    ret = ot_mpi_keyslot_create(&keyslot_handle, KM_KEYSLOT_TYPE_MCIPHER);
    if (ret != ERRCODE_SUCC) {
        printf("ot_mpi_keyslot_create fail, ret is 0x%x\n", ret);
        goto cipher_destory;
    }

    ret = ot_mpi_cipher_symc_attach(symc_handle, keyslot_handle);
    if (ret != ERRCODE_SUCC) {
        printf("ot_mpi_cipher_symc_attach fail, ret is 0x%x\n", ret);
        goto keyslot_destory;
    }

    ret = cipher_set_rootkey(keyslot_handle, key);
    if (ret != ERRCODE_SUCC) {
        printf("cipher_set_rootkey fail, ret is 0x%x\n", ret);
        goto keyslot_destory;
    }

    ret = ot_mpi_cipher_symc_set_config(symc_handle, &symc_ctrl);
    if (ret != ERRCODE_SUCC) {
        printf("ot_mpi_cipher_symc_set_config fail, ret is 0x%x\n", ret);
        goto keyslot_destory;
    }

    src_buf.phys_addr = (uintptr_t)code_src;
    dst_buf.phys_addr = (uintptr_t)code_dst;
    ret = ot_mpi_cipher_symc_decrypt(symc_handle, &src_buf, &dst_buf, data_length);
    if (ret != ERRCODE_SUCC) {
        printf("ot_mpi_cipher_symc_decrypt fail, ret is 0x%x\n", ret);
        goto keyslot_destory;
    }

keyslot_destory:
    ot_mpi_keyslot_destroy(keyslot_handle);
cipher_destory:
    ot_mpi_cipher_symc_destroy(symc_handle);
    return ret;
}
#endif

#if defined(CONFIG_KM_EFFECTIVE_KEY_SUPPORT)
static errcode_t cipher_set_rootkey(uint32_t keyslot_handle, const uint8_t *salt)
{
    uint32_t ret = 0;
    uint32_t klad_handle = 0;
    uint32_t offset = 0x12;
    uint8_t tee_enable = 0;
    km_klad_attr klad_attr = {
        .klad_cfg = {
            .rootkey_type = CRYPTO_KDF_HARD_KEY_TYPE_ERK_REE
        },
        .key_cfg = {
            .engine = KM_CRYPTO_ALG_SM4,
            .decrypt_support = TD_TRUE,
            .encrypt_support = TD_TRUE
        }
    };

    km_klad_effective_key klad_effective_key = {
        .salt = salt,
        .salt_length = 28,
        .kdf_hard_alg = CRYPTO_KDF_HARD_ALG_SHA256,
        .key_size = CRYPTO_KLAD_KEY_SIZE_128BIT,
        .key_parity = 0,
        .oneway = 0,
    };

    (void)ot_mpi_otp_init();
    (void)ot_mpi_otp_read_byte(offset, &tee_enable);
    (void)ot_mpi_otp_deinit();
    if (tee_enable == 0x42) {
        klad_attr.key_sec_cfg.key_sec = KM_KLAD_SEC_ENABLE;
        klad_attr.key_sec_cfg.master_only_enable = TD_TRUE;
        klad_attr.key_sec_cfg.dest_buf_sec_support = TD_TRUE;
        klad_attr.key_sec_cfg.src_buf_sec_support = TD_TRUE;
        klad_attr.key_sec_cfg.src_buf_non_sec_support = TD_FALSE;
        klad_attr.key_sec_cfg.dest_buf_non_sec_support = TD_FALSE;
    } else {
        klad_attr.key_sec_cfg.key_sec = KM_KLAD_SEC_DISABLE;
        klad_attr.key_sec_cfg.master_only_enable = TD_FALSE;
        klad_attr.key_sec_cfg.dest_buf_sec_support = TD_FALSE;
        klad_attr.key_sec_cfg.dest_buf_non_sec_support = TD_TRUE;
        klad_attr.key_sec_cfg.src_buf_sec_support = TD_FALSE;
        klad_attr.key_sec_cfg.src_buf_non_sec_support = TD_TRUE;
    }

    ret = ot_mpi_klad_create(&klad_handle);
    if (ret != ERRCODE_SUCC) {
        printf("ot_mpi_klad_create fail, ret is 0x%x\n", ret);
        return ERRCODE_FAIL;
    }

    ret = ot_mpi_klad_set_attr(klad_handle, &klad_attr);
    if (ret != ERRCODE_SUCC) {
        printf("ot_mpi_klad_set_attr fail, ret is 0x%x\n", ret);
        goto klad_destory;
    }

    ret = ot_mpi_klad_attach(klad_handle, KM_KLAD_DEST_TYPE_MCIPHER, keyslot_handle);
    if (ret != ERRCODE_SUCC) {
        printf("ot_mpi_klad_attach fail, ret is 0x%x\n", ret);
        goto klad_destory;
    }

    ret = ot_mpi_klad_set_effective_key(klad_handle, &klad_effective_key);
    if (ret != ERRCODE_SUCC) {
        printf("hi_mpi_klad_set_effective_key fail, ret is 0x%x\n", ret);
        goto klad_detach;
    }

    return ERRCODE_SUCC;

klad_detach:
    ot_mpi_klad_detach(klad_handle, KM_KLAD_DEST_TYPE_MCIPHER, keyslot_handle);
klad_destory:
    ot_mpi_klad_destroy(klad_handle);
    return ret;
}

errcode_t crypto_sm4_dec(const uint8_t *salt, const uint8_t *iv,
    const uint8_t *code_src, uint8_t *code_dst, uint32_t data_length)
{
    uint32_t ret = 0;
    uint32_t symc_handle = 0;
    uint32_t keyslot_handle = 0;
    crypto_buf_attr src_buf = {0};
    crypto_buf_attr dst_buf = {0};
    crypto_symc_attr symc_attr = {0};
    crypto_symc_ctrl_t symc_ctrl = {0};

    symc_attr.symc_type = CRYPTO_SYMC_TYPE_NORMAL;
    symc_attr.is_long_term = TD_FALSE;
    symc_ctrl.symc_alg = CRYPTO_SYMC_ALG_SM4;
    symc_ctrl.work_mode = CRYPTO_SYMC_WORK_MODE_CBC;
    symc_ctrl.symc_key_length = CRYPTO_SYMC_KEY_128BIT;
    symc_ctrl.symc_bit_width = CRYPTO_SYMC_BIT_WIDTH_128BIT;
    symc_ctrl.iv_length = IV_LEN;

    ret = memcpy_s(symc_ctrl.iv, IV_LEN, iv, IV_LEN);
    if (ret != ERRCODE_SUCC) {
        printf("memcpy_s iv fail!");
        return ERRCODE_FAIL;
    }

    ret = ot_mpi_cipher_symc_create(&symc_handle, &symc_attr);
    if (ret != ERRCODE_SUCC) {
        printf("ot_mpi_cipher_symc_create fail, ret is 0x%x\n", ret);
        return ret;
    }

    ret = ot_mpi_keyslot_create(&keyslot_handle, KM_KEYSLOT_TYPE_MCIPHER);
    if (ret != ERRCODE_SUCC) {
        printf("ot_mpi_keyslot_create fail, ret is 0x%x\n", ret);
        goto cipher_destory;
    }

    ret = ot_mpi_cipher_symc_attach(symc_handle, keyslot_handle);
    if (ret != ERRCODE_SUCC) {
        printf("ot_mpi_cipher_symc_attach fail, ret is 0x%x\n", ret);
        goto keyslot_destory;
    }

    ret = cipher_set_rootkey(keyslot_handle, salt);
    if (ret != ERRCODE_SUCC) {
        printf("cipher_set_rootkey fail, ret is 0x%x\n", ret);
        goto keyslot_destory;
    }

    ret = ot_mpi_cipher_symc_set_config(symc_handle, &symc_ctrl);
    if (ret != ERRCODE_SUCC) {
        printf("ot_mpi_cipher_symc_set_config fail, ret is 0x%x\n", ret);
        goto keyslot_destory;
    }

    src_buf.phys_addr = (uintptr_t)code_src;
    dst_buf.phys_addr = (uintptr_t)code_dst;
    ret = ot_mpi_cipher_symc_decrypt(symc_handle, &src_buf, &dst_buf, data_length);
    if (ret != ERRCODE_SUCC) {
        printf("ot_mpi_cipher_symc_decrypt fail, ret is 0x%x\n", ret);
        goto keyslot_destory;
    }

keyslot_destory:
    ot_mpi_keyslot_destroy(keyslot_handle);
cipher_destory:
    ot_mpi_cipher_symc_destroy(symc_handle);
    return ret;
}
#endif
#else
errcode_t crypto_sm4_dec(const uint8_t *key, const uint8_t *iv,
    const uint8_t *code_src, uint8_t *code_dst, uint32_t data_length)
{
    (td_void)key;
    (td_void)iv;
    (td_void)code_src;
    (td_void)code_dst;
    (td_void)data_length;
    return ERRCODE_FAIL;
}
#endif
