/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
*/
#include "drv_hash_inner.h"

#define CRYPTO_INVALID_CHN_NUM (0xFFFFFFFF)

static drv_hash_simple_context g_hash_ctx_list[CONFIG_HASH_HARD_CHN_CNT];

drv_hash_simple_context *inner_get_hash_ctx(td_handle hash_handle)
{
    td_s32 ret;

    ret = inner_hash_drv_handle_chk(hash_handle);
    if (ret != TD_SUCCESS) {
        return TD_NULL;
    }

    return &g_hash_ctx_list[hash_handle];
}

td_s32 drv_cipher_hash_init(td_void)
{
    td_s32 ret = TD_SUCCESS;

    crypto_drv_func_enter();

    ret = hal_cipher_hash_init();
    crypto_chk_return(ret != CRYPTO_SUCCESS, ret, "hal_cipher_hash_init failed\n");

    crypto_drv_func_exit();
    return ret;
}

td_s32 drv_cipher_hash_deinit(td_void)
{
    td_u32 i;
    drv_hash_simple_context *ctx = TD_NULL;
    crypto_drv_func_enter();

    for (i = 0; i < CONFIG_HASH_HARD_CHN_CNT; i++) {
        ctx = &g_hash_ctx_list[i];
        if (ctx->open == TD_FALSE) {
            continue;
        }
        hal_hash_unlock(i);
        (td_void) memset_s(ctx, sizeof(drv_hash_simple_context), 0, sizeof(drv_hash_simple_context));
    }

    hal_cipher_hash_deinit();
    crypto_drv_func_exit();
    return TD_SUCCESS;
}

static td_s32 drv_hash_lock_chn(td_u32 *chn_num)
{
    td_u32 i;
    td_s32 ret = TD_SUCCESS;
    *chn_num = CRYPTO_INVALID_CHN_NUM;
    /* Lock one free Hash Hard Channel. */
    for (i = 0; i < CONFIG_HASH_HARD_CHN_CNT; i++) {
        if (g_hash_ctx_list[i].open == TD_TRUE) {
            continue;
        }
        ret = hal_hash_lock(i);
        if (ret == TD_SUCCESS) {
            *chn_num = i;
            break;
        }
    }
    if (*chn_num == CRYPTO_INVALID_CHN_NUM) {
        crypto_log_err("Hash Channel is Busy\n");
        return HASH_COMPAT_ERRNO(ERROR_CHN_BUSY);
    }
    return TD_SUCCESS;
}

td_s32 drv_cipher_hash_start(td_handle *drv_hash_handle, const crypto_hash_attr *hash_attr)
{
    td_u32 chn_num = CRYPTO_INVALID_CHN_NUM;
    td_s32 ret = TD_FAILURE;
    drv_hash_simple_context *ctx = TD_NULL;
    crypto_hash_type type;
    crypto_drv_func_enter();

    hash_null_ptr_chk(drv_hash_handle);
    hash_null_ptr_chk(hash_attr);

    type = crypto_hash_remove_hmac_flag(hash_attr->hash_type);
    crypto_chk_return(type != CRYPTO_HASH_TYPE_SM3,
                      HASH_COMPAT_ERRNO(ERROR_INVALID_PARAM), "hash_type is invalid!\n");

    /* Lock one free Hash Hard Channel. */
    ret = drv_hash_lock_chn(&chn_num);
    if (ret != TD_SUCCESS) {
        crypto_log_err("drv_hash_lock_chn failed\n");
        return ret;
    }
    ctx = &g_hash_ctx_list[chn_num];
    (td_void) memset_s(ctx, sizeof(drv_hash_simple_context), 0, sizeof(drv_hash_simple_context));

    ctx->hash_type = hash_attr->hash_type;
    ctx->chn_num = chn_num;

    /* Set Config. */
    ret = drv_hash_get_state_iv(type, TD_NULL, ctx->state, sizeof(ctx->state));
    crypto_chk_goto(ret != CRYPTO_SUCCESS, error_hash_unlock, "drv_hash_get_state failed\n");

    ret = hal_cipher_hash_config(chn_num, type, ctx->state);
    crypto_chk_goto(ret != CRYPTO_SUCCESS, error_hash_unlock, "hal_cipher_hash_config failed\n");

    if (crypto_hash_is_hmac(hash_attr->hash_type) == TD_TRUE) {
        if (hash_attr->is_keyslot) {
#if defined(CONFIG_HASH_HMAC_KEYSLOT_SUPPORT)
            ret = hal_cipher_hash_attach(chn_num, hash_attr->drv_keyslot_handle);
            crypto_chk_goto(ret != CRYPTO_SUCCESS, error_hash_unlock, "hal_cipher_hash_attach failed\n");
#else
            crypto_log_err("hard hmac unsupport!\n");
            ret = HASH_COMPAT_ERRNO(ERROR_UNSUPPORT);
            goto error_hash_unlock;
#endif
        } else {
#if defined(CONFIG_HASH_SOFT_HMAC_SUPPORT)
            ret = inner_drv_hmac_start(ctx, hash_attr);
            crypto_chk_goto(ret != TD_SUCCESS, error_hash_unlock, "inner_drv_hmac_start failed\n");
#else
            crypto_log_err("soft hmac unsupport!\n");
            ret = HASH_COMPAT_ERRNO(ERROR_UNSUPPORT);
            goto error_hash_unlock;
#endif
        }
        ctx->is_keyslot = hash_attr->is_keyslot;
    }

    *drv_hash_handle = chn_num;
    ctx->open = TD_TRUE;
    crypto_drv_func_exit();
    return ret;
error_hash_unlock:
    (td_void) hal_hash_unlock(chn_num);
    return ret;
}

td_s32 inner_drv_hash_process(td_u32 chn_num, drv_hash_simple_context *ctx, td_u8 *data_buffer, td_u32 data_length,
                              td_u32 node_type)
{
    td_s32 ret;

    crypto_cache_flush((uintptr_t)data_buffer, data_length);
    ret = hal_cipher_hash_add_in_node(chn_num, crypto_get_phys_addr(data_buffer), data_length, node_type);
    crypto_chk_return(ret != TD_SUCCESS, ret, "hal_cipher_hash_add_in_node failed\n");

    ret = hal_cipher_hash_start(chn_num, TD_FALSE);
    crypto_chk_return(ret != TD_SUCCESS, ret, "hal_cipher_hash_start failed\n");

    ret = hal_cipher_hash_wait_done(chn_num, ctx->state, sizeof(ctx->state));
    crypto_chk_return(ret != TD_SUCCESS, ret, "hal_cipher_hash_wait_done failed\n");

    return ret;
}

td_s32 drv_cipher_hash_update(td_handle drv_hash_handle, const crypto_buf_attr *src_buf, const td_u32 len)
{
    td_s32 ret = TD_SUCCESS;
    td_u32 chn_num = (td_u32)drv_hash_handle;
    drv_hash_simple_context *ctx = TD_NULL;
    td_u32 block_size;
    td_u32 processing_len = 0;
    td_u32 left = 0;
    in_node_type_e node_type = IN_NODE_TYPE_FIRST;
    crypto_drv_func_enter();

    hash_null_ptr_chk(src_buf);
    hash_null_ptr_chk(src_buf->virt_addr);
    crypto_chk_return(len == 0 || len > CRYPTO_HASH_UPDATE_MAX_LEN, HASH_COMPAT_ERRNO(ERROR_INVALID_PARAM),
                      "len is invalid\n");

    ret = inner_hash_drv_handle_chk(drv_hash_handle);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ctx = &g_hash_ctx_list[chn_num];
    crypto_chk_return(ctx->open == TD_FALSE, HASH_COMPAT_ERRNO(ERROR_CTX_CLOSED), "ctx is closed!\n");

    if (ctx->is_keyslot == TD_FALSE) {
        node_type |= IN_NODE_TYPE_LAST;
    }

    block_size = crypto_hash_get_block_size(ctx->hash_type) / CRYPTO_BITS_IN_BYTE;
    processing_len = len - len % block_size;
    left = len - processing_len;

    if (processing_len != 0) {
        ret = inner_drv_hash_process(chn_num, ctx, src_buf->virt_addr, processing_len, node_type);
        crypto_chk_return(ret != TD_SUCCESS, ret, "inner_drv_hash_process failed\n");
    }

    if (left != 0) {
        ret = memcpy_s(ctx->tail, sizeof(ctx->tail), (td_u8 *)src_buf->virt_addr + processing_len, left);
        crypto_chk_return(ret != EOK, HASH_COMPAT_ERRNO(ERROR_MEMCPY_S), "memcpy_s failed\n");
    }
    ctx->tail_length = left;
    inner_drv_hash_length_add(ctx->length, len * CRYPTO_BITS_IN_BYTE);
    crypto_drv_func_exit();
    return TD_SUCCESS;
}

static td_s32 drv_process_tail(td_u32 chn_num, drv_hash_simple_context *ctx)
{
    td_s32 ret = TD_SUCCESS;
    td_u32 max_message_len = crypto_hash_get_message_len(ctx->hash_type) / CRYPTO_BITS_IN_BYTE;
    td_u32 block_size = crypto_hash_get_block_size(ctx->hash_type) / CRYPTO_BITS_IN_BYTE;
    td_u32 processing_len = 0;
    td_u32 tail_length = ctx->tail_length;
    td_u32 *length = ctx->length;
    td_u32 tail_max_len = block_size * 2;  // 2: 2 blocks max
    td_u8 *tail_buf = crypto_malloc_coherent(tail_max_len, "tail_buf");
    crypto_chk_return(tail_buf == NULL, HASH_COMPAT_ERRNO(ERROR_MALLOC), "crypto_malloc_coherent failed\n");

    (void)memset_s(tail_buf, tail_max_len, 0, tail_max_len);

    ret = memcpy_s(tail_buf, tail_max_len, ctx->tail, tail_length);
    crypto_chk_goto_with_ret(ret, ret != EOK, exit_free, HASH_COMPAT_ERRNO(ERROR_MEMCPY_S), "memcpy_s failed\n");

    if (tail_length + 1 + max_message_len > block_size) {
        processing_len = tail_max_len;
    } else {
        processing_len = block_size;
    }
    tail_buf[tail_length] = 0x80;
    if (ctx->is_keyslot) {
        inner_drv_hash_length_add(length, block_size * CRYPTO_BITS_IN_BYTE);
    }
    length[1] = crypto_cpu_to_be32(length[1]);
    length[0] = crypto_cpu_to_be32(length[0]);

    ret = memcpy_s(tail_buf + processing_len - sizeof(ctx->length), sizeof(ctx->length), length, sizeof(ctx->length));
    crypto_chk_goto_with_ret(ret, ret != EOK, exit_free, HASH_COMPAT_ERRNO(ERROR_MEMCPY_S), "memcpy_s failed\n");

    ret = inner_drv_hash_process(chn_num, ctx, tail_buf, processing_len, IN_NODE_TYPE_FIRST | IN_NODE_TYPE_LAST);
    crypto_chk_goto(ret != TD_SUCCESS, exit_free, "inner_drv_hash_process failed\n");

exit_free:
    CRYPTO_FREE_COHERENT(tail_buf);
    return ret;
}

td_s32 drv_cipher_hash_finish(td_handle drv_hash_handle, td_u8 *out, td_u32 *out_len)
{
    td_s32 ret = TD_FAILURE;
    td_u32 chn_num = (td_u32)drv_hash_handle;
    drv_hash_simple_context *ctx = TD_NULL;
    td_u32 result_size;
    crypto_drv_func_enter();

    hash_null_ptr_chk(out);
    hash_null_ptr_chk(out_len);

    ret = inner_hash_drv_handle_chk(drv_hash_handle);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ctx = &g_hash_ctx_list[chn_num];
    crypto_chk_return(ctx->open == TD_FALSE, HASH_COMPAT_ERRNO(ERROR_CTX_CLOSED), "ctx is closed!\n");

    result_size = crypto_hash_get_result_size(ctx->hash_type) / CRYPTO_BITS_IN_BYTE;
    crypto_chk_return(*out_len < result_size, HASH_COMPAT_ERRNO(ERROR_INVALID_PARAM), "out's size is not enough\n");

    /* Process the Tail Data. */
    ret = drv_process_tail(chn_num, ctx);
    crypto_chk_goto(ret != TD_SUCCESS, exit_hash_unlock, "drv_process_tail failed\n");

    if (crypto_hash_is_hmac(ctx->hash_type) && ctx->is_keyslot == TD_FALSE) {
#if defined(CONFIG_HASH_SOFT_HMAC_SUPPORT)
        ret = inner_drv_hmac_finish(ctx, (uint8_t *)ctx->state, result_size);
        crypto_chk_goto(ret != TD_SUCCESS, exit_hash_unlock, "inner_drv_hmac_finish failed\n");
#else
        crypto_log_err("hmac without keyslot is unsupport\n");
        ret = HASH_COMPAT_ERRNO(ERROR_UNSUPPORT);
        goto exit_hash_unlock;
#endif
    }

    ret = memcpy_s(out, *out_len, ctx->state, *out_len);
    crypto_chk_goto_with_ret(ret, ret != EOK, exit_hash_unlock, HASH_COMPAT_ERRNO(ERROR_MEMCPY_S), "memcpy_s failed\n");

    *out_len = result_size;
    ret = CRYPTO_SUCCESS;
exit_hash_unlock:
    hal_hash_unlock(chn_num);
    (td_void) memset_s(ctx, sizeof(drv_hash_simple_context), 0, sizeof(drv_hash_simple_context));

    crypto_drv_func_exit();
    return ret;
}

td_s32 drv_cipher_hash_destroy(td_handle drv_hash_handle)
{
    td_s32 ret = TD_SUCCESS;
    td_u32 chn_num = (td_u32)drv_hash_handle;
    drv_hash_simple_context *ctx = TD_NULL;

    crypto_drv_func_enter();

    ret = inner_hash_drv_handle_chk(drv_hash_handle);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ctx = &g_hash_ctx_list[chn_num];
    crypto_chk_return(ctx->open == TD_FALSE, HASH_COMPAT_ERRNO(ERROR_CTX_CLOSED), "ctx is closed!\n");

    hal_hash_unlock(chn_num);

    (td_void) memset_s(ctx, sizeof(drv_hash_simple_context), 0, sizeof(drv_hash_simple_context));
    crypto_drv_func_exit();
    return ret;
}
