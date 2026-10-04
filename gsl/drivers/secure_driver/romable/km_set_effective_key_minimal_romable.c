/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
*/
#include "cipher_romable.h"

#include "crypto_drv_common.h"

#define SALT_LENGTH_IN_WORD         7
#define SALT_LENGTH_IN_BYTE         ((SALT_LENGTH_IN_WORD) * (CRYPTO_WORD_WIDTH))

#define CRYPTO_U8_TO_U32_BIT_SHIFT(data, i) \
    ((td_u32)(data)[(i)*4]  | ((td_u32)(data)[(i)*4+1] << 8) | ((td_u32)(data)[(i)*4+2] << 16) | \
    ((td_u32)(data)[(i)*4+3]<< 24))

/* reg offset. */
#define KLAD_REG_OFFSET              (0x00001000)

#define KL_KEY_ADDR         (KLAD_REG_OFFSET + 0x010)
#define KL_KEY_CFG          (KLAD_REG_OFFSET + 0x014)
#define KL_KEY_SEC_CFG      (KLAD_REG_OFFSET + 0x018)
#define KL_ERROR            (KLAD_REG_OFFSET + 0x038)
#define KC_ERROR            (KLAD_REG_OFFSET + 0x03c)
#define KL_INT_RAW          (KLAD_REG_OFFSET + 0x044)
#define KL_LOCK_CTRL        (KLAD_REG_OFFSET + 0x074)
#define KL_UNLOCK_CTRL      (KLAD_REG_OFFSET + 0x078)
#define KL_COM_CTRL         (KLAD_REG_OFFSET + 0x084)
#define KL_COM_STATUS       (KLAD_REG_OFFSET + 0x088)

#define RKP_PBKDF2_DATA(a)      (0x100 + 4 * (a)) /* a 0~31 */

#define RKP_LOCK_GSL                (0x000)
#define RKP_CMD_CFG             (0x004)
#define KDF_ERROR               (0x008)
#define RKP_RAW_INT             (0x010)

/* Define the union U_RKP_CMD_CFG */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    sw_calc_req               : 1; /* [0]  */
        unsigned int    pbkdf2_alg_sel_cfg        : 3; /* [3..1]  */
        unsigned int    pbkdf2_key_sel_cfg        : 5; /* [8..4]  */
        unsigned int    reserved                  : 3; /* [11..9]  */
        unsigned int    master_key_sel            : 2; /* [13..12]  */
        unsigned int    pbkdf2_key_len            : 2; /* [15..14]  */
        unsigned int    rkp_pbkdf_calc_time       : 16; /* [31..16]  */
    } bits;
    /* Define an unsigned member */
    unsigned int    u32;
} rkp_cmd_cfg;

/* Define the union KL_COM_CTRL */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    kl_com_start          : 1; /* [0]  */
        unsigned int    kl_com_level_sel      : 3; /* [3..1]  */
        unsigned int    kl_com_alg_sel        : 2; /* [5..4]  */
        unsigned int    kl_com_key_size       : 2; /* [7..6]  */
        unsigned int    rk_choose             : 5; /* [12..8]  */
        unsigned int    reserved_1            : 19; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} kl_com_ctrl;

/* Define the union U_KL_COM_STATUS */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    kl_com_rk_rdy         : 1; /* [0]  */
        unsigned int    kl_com_lv1_rdy        : 1; /* [1]  */
        unsigned int    reserved_0            : 30; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} kl_com_status;

/* Define the union kl_int_raw */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    com_kl_int_raw               : 1; /* [0]  */
        unsigned int    reserved0                    : 3; /* [3..1]  */
        unsigned int    kl_int_num                   : 5; /* [8..4]  */
        unsigned int    reserved1                    : 5; /* [13..9]  */
        unsigned int    clr_kl_int_raw               : 1; /* [14]  */
        unsigned int    reserved2                    : 18; /* [31..15]  */
    } bits;
    /* Define an unsigned member */
    unsigned int    u32;
} kl_int_raw;

#define km_null_ptr_chk(ptr)    do {        \
    if ((ptr) == NULL) {                    \
        return ERROR_PARAM_IS_NULL;         \
    }                                       \
} while (0)

int32_t gsl_drv_cipher_create_keyslot_by_set_effective_key(unsigned int *keyslot_handle, ext_km_rootkey_type type,
    const uint8_t *salt, uint32_t salt_len, ext_symc_alg symc_alg)
{
    int32_t ret;
    uint32_t i;
    uint32_t salt_word;
    rkp_cmd_cfg cmd_cfg = { 0 };
    uint32_t kdf_err = 0;
    kl_com_ctrl com_ctrl = { 0 };
    kl_int_raw int_raw = {0};
    kl_com_status com_status = {0};
    uint32_t kl_err = 0;
    uint32_t kc_err = 0;

    km_null_ptr_chk(keyslot_handle);
    km_null_ptr_chk(salt);
    crypto_chk_return(type != EXT_KM_ROOTKEY_TYPE_ERK_REE && type != EXT_KM_ROOTKEY_TYPE_ERK_TEE, ERROR_INVALID_PARAM,
        "rootkey_type is invalid\n");
    crypto_chk_return(salt_len != SALT_LENGTH_IN_BYTE, ERROR_INVALID_PARAM, "salt_len is invalid\n");

    /* 1. klad lock. */
    km_reg_write(KL_LOCK_CTRL, 0x1);

    /* 2. rkp lock. */
    km_reg_write(RKP_LOCK_GSL, 0x2);    // 0x2: rkp locked by tee cpu.

    /* 3. set kl_key_addr, use default 0. */
    km_reg_write(KL_KEY_ADDR, 0);
    /* 4. set kl_key_cfg. */
    /*
     * 0x20201:
     * bit[17] is 1'b1, means can be used for decryption.
     * bit[16] is 0'b0, means can't be used for encryption.
     * bit[11:4] is 0x2- or 0x5-. 0x2- means dsc_code is AES, 0x5- means dsc_code is SM4.
     * bit[1:0] is 2'b01, means port_sel is Mcipher.
     */
     if (symc_alg == EXT_SYMC_ALG_AES) {
        km_reg_write(KL_KEY_CFG, 0x20201);  // 0x20201: refer to above.
     } else if (symc_alg == EXT_SYMC_ALG_SM4) {
        km_reg_write(KL_KEY_CFG, 0x20501);  // 0x20501: refer to above.
     } else {
        return ERROR_INVALID_PARAM;
     }

    /* 5. set kl_key_sec_cfg. */
    /*
     * 0x3f:
     * bit[5] is 1'b1, means tee_only is true.
     * bit[4] is 1'b1, means dest_sec is support.
     * bit[3] is 1'b1, means dest_nsec is support.
     * bit[2] is 1'b1, means src_sec is support.
     * bit[1] is 1'b1, means src_nsec is support.
     * bit[0] is 1'b1, means key_sec is secure.
     */
    km_reg_write(KL_KEY_SEC_CFG, 0x3f);  // 0x3f: refer to above.

    /* 6. set salt. */
    for (i = 0; i < SALT_LENGTH_IN_WORD; i++) {
        salt_word = CRYPTO_U8_TO_U32_BIT_SHIFT(salt, i);
        km_reg_write(RKP_PBKDF2_DATA(i), salt_word);
    }
    /* 7. set rkp_cmd_cfg. */
    /*
     * 0x6071 for SOC_ERK_REE and 0x6061 for SOC_ERK_TEE:
     * bit[15:14] is 2'b01, means pbkdf2_key_len is 128-bit
     * bit[13:12] is 2'b10, means master_key_sel is OEM_Rootkey0
     * bit[8:4] is 5'd7/5'b6, means pbkdf2_key_sel_cfg is SOC_ERK_REE/SOC_ERK_TEE
     * bit[3:1] is 3'b0, means pbkdf2_alg_sel_cfg is SHA256
     * bit[0] is 1'b1, means start calculation
     *
     */
    if (type == EXT_KM_ROOTKEY_TYPE_ERK_REE) {
        km_reg_write(RKP_CMD_CFG, 0x6071);  // 0x6071: refer to above.
    } else {
        km_reg_write(RKP_CMD_CFG, 0x6061);  // 0x6061: refer to above.
    }

    /* 8. wait rkp_done. */
    for (i = 0; i < CONFIG_RKP_WAIT_TIMEOUT_IN_US; i++) {
        cmd_cfg.u32 = km_reg_read(RKP_CMD_CFG);
        if (cmd_cfg.bits.sw_calc_req == 0) {
            km_reg_write(RKP_RAW_INT, 0x1); // 0x1: clear rkp raw interrupt.
            break;
        }
        crypto_udelay(1);
    }
    if (i >= CONFIG_RKP_WAIT_TIMEOUT_IN_US) {
        ret = ERROR_RKP_CALC_TIMEOUT;
        goto exit_unlock;
    }
    /* check kdf err. */
    kdf_err = km_reg_read(KDF_ERROR);
    if (kdf_err != 0) {
        ret = ERROR_KM_LOGIC;
        goto exit_unlock;
    }
    /* 9. set KL_COM_CTRL. */
    /*
     * 0x741 for SOC_ERK_REE and 0x601 for SOC_ERK_TEE:
     * bit[12:8] is 5'd7/5'd6, means rk_choose is SOC_ERK_REE/SOC_ERK_TEE
     * bit[7:6] is 2'b01, means kl_com_key_size is 128bit
     * bit[0] is 1'b1, means start common key ladder
     */
    if (type == EXT_KM_ROOTKEY_TYPE_ERK_REE) {
        km_reg_write(KL_COM_CTRL, 0x741);
    } else {
        km_reg_write(KL_COM_CTRL, 0x641);
    }
    /* 10. wait hkl_done. */
    for (i = 0; i < CONFIG_KLAD_COM_ROUTE_TIMEOUT_IN_US; i++) {
        com_ctrl.u32 = km_reg_read(KL_COM_CTRL);
        if (com_ctrl.bits.kl_com_start == 0) {
            int_raw.u32 = km_reg_read(KL_INT_RAW);
            int_raw.bits.com_kl_int_raw = 0x1;
            km_reg_write(KL_INT_RAW, int_raw.u32);
            break;
        }
        crypto_udelay(1);
    }
    if (i >= CONFIG_KLAD_COM_ROUTE_TIMEOUT_IN_US) {
        ret = ERROR_KLAD_COM_ROUTE_TIMEOUT;
        goto exit_unlock;
    }
    /* check rootkey ready status. */
    com_status.u32 = km_reg_read(KL_COM_STATUS);
    if (com_status.bits.kl_com_rk_rdy == 0) {
        ret = ERROR_KLAD_ROOTKEY_NOT_READY;
        goto exit_unlock;
    }
    /* check kl_err. */
    kl_err = km_reg_read(KL_ERROR);
    if (kl_err != 0) {
        ret = ERROR_KM_KL_LOGIC;
        crypto_log_err("kl_err is 0x%x\n", kl_err);
        goto exit_unlock;
    }
    kc_err = km_reg_read(KC_ERROR);
    /* check kc_err. */
    if (kc_err != 0) {
        ret = ERROR_KM_KC_LOGIC;
        crypto_log_err("kc_err is 0x%x\n", kc_err);
        goto exit_unlock;
    }

    *keyslot_handle = 0;
    ret = CRYPTO_SUCCESS;
exit_unlock:
    /* 11. rkp unlock. */
    km_reg_write(RKP_LOCK_GSL, 0);

    /* 12. klad unlock. */
    km_reg_write(KL_UNLOCK_CTRL, 0x1);
    return ret;
}

int32_t gsl_drv_cipher_destroy_keyslot(unsigned int keyslot_handle)
{
    crypto_unused(keyslot_handle);
    return CRYPTO_SUCCESS;
}