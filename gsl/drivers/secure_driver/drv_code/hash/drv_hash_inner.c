/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
*/
#include "drv_hash.h"
#include "crypto_hash_struct.h"
#include "crypto_drv_common.h"
#include "drv_hash_inner.h"

#if defined(CONFIG_HASH_SM3_SUPPORT)
// SM3, the initial hash value
static const td_u8 g_sm3_ival[32] = {
    0x73, 0x80, 0x16, 0x6F,
    0x49, 0x14, 0xB2, 0xB9,
    0x17, 0x24, 0x42, 0xD7,
    0xDA, 0x8A, 0x06, 0x00,
    0xA9, 0x6F, 0x30, 0xBC,
    0x16, 0x31, 0x38, 0xAA,
    0xE3, 0x8D, 0xEE, 0x4D,
    0xB0, 0xFB, 0x0E, 0x4E
};
#endif

td_s32 inner_hash_drv_handle_chk(td_handle hash_handle)
{
    td_u32 chn_num = (td_u32)hash_handle;
    crypto_chk_return(chn_num >= CONFIG_HASH_HARD_CHN_CNT, HASH_COMPAT_ERRNO(ERROR_INVALID_HANDLE),
        "hash_handle[%u] is invalid\n", hash_handle);
    crypto_chk_return(((1 << chn_num) & CONFIG_HASH_HARD_CHN_MASK) == 0, HASH_COMPAT_ERRNO(ERROR_INVALID_HANDLE),
        "hash_handle[%u] is invalid\n", hash_handle);

    return TD_SUCCESS;
}

td_void inner_drv_hash_length_add(td_u32 length[2], td_u32 addition)
{
    td_u32 diff = 0;
    if (length[1] > length[1] + addition) {
        length[0]++;
        diff = 0xFFFFFFFF - length[1] + 1;
        length[1] = addition - diff;
        return;
    }
    length[1] += addition;
}

typedef struct {
    crypto_hash_type hash_type;
    const td_u8 *state_val;
    td_u32 state_length;
} inner_drv_hash_state_iv_table_t;

static inner_drv_hash_state_iv_table_t g_state_iv_table[] = {
#if defined(CONFIG_HASH_SM3_SUPPORT)
    {
        .hash_type = CRYPTO_HASH_TYPE_SM3,
        .state_val = g_sm3_ival,
        .state_length = sizeof(g_sm3_ival)
    },
#endif
};

td_s32 drv_hash_get_state_iv(crypto_hash_type hash_type, td_u32 *iv_size, td_u32 *state_iv, td_u32 state_iv_len)
{
    const td_u8 *state_val = TD_NULL;
    td_s32 ret;
    td_u32 length = 0;
    td_u32 type = crypto_hash_remove_hmac_flag(hash_type);
    td_u32 i;

    crypto_chk_return(state_iv == TD_NULL, TD_FAILURE, "state_iv is NULL!\n");

    for (i = 0; i < crypto_array_size(g_state_iv_table); i++) {
        if (type == g_state_iv_table[i].hash_type) {
            state_val = g_state_iv_table[i].state_val;
            length = g_state_iv_table[i].state_length;
            break;
        }
    }
    if (state_val == TD_NULL) {
        crypto_log_err("Invalid Hash Mode!\n");
        return HASH_COMPAT_ERRNO(ERROR_INVALID_PARAM);
    }

    ret = memcpy_s(state_iv, state_iv_len, state_val, length);
    crypto_chk_return(ret != EOK, HASH_COMPAT_ERRNO(ERROR_MEMCPY_S), "memcpy_s failed\n");

    if (iv_size != TD_NULL) {
        *iv_size = length;
    }
    return TD_SUCCESS;
}
