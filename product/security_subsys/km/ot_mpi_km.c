/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "ot_mpi_km.h"

#include <securec.h>

#include "drv_keyslot.h"
#include "drv_klad.h"
#include "crypto_osal_lib.h"
#include "crypto_drv_common.h"

td_s32 ot_mpi_km_init()
{
    return TD_SUCCESS;
}

td_s32 ot_mpi_km_deinit()
{
    return TD_SUCCESS;
}

/* Keyslot. */
td_s32 ot_mpi_keyslot_create(crypto_handle *mpi_keyslot_handle, km_keyslot_type keyslot_type)
{
    return drv_keyslot_create((td_handle *)mpi_keyslot_handle, (crypto_keyslot_type)keyslot_type);
}

td_s32 ot_mpi_keyslot_destroy(crypto_handle mpi_keyslot_handle)
{
    return drv_keyslot_destroy((td_handle)mpi_keyslot_handle);
}

/* Klad. */
td_s32 ot_mpi_klad_create(crypto_handle *mpi_klad_handle)
{
    return drv_klad_create((td_handle *)mpi_klad_handle);
}

td_s32 ot_mpi_klad_destroy(crypto_handle mpi_klad_handle)
{
    return drv_klad_destroy((td_handle)mpi_klad_handle);
}

td_s32 ot_mpi_klad_attach(crypto_handle mpi_klad_handle, km_klad_dest_type klad_type,
    crypto_handle mpi_keyslot_handle)
{
    return drv_klad_attach((td_handle)mpi_klad_handle, (crypto_klad_dest)klad_type, (td_handle)mpi_keyslot_handle);
}

td_s32 ot_mpi_klad_detach(crypto_handle mpi_klad_handle, km_klad_dest_type klad_type,
    crypto_handle mpi_keyslot_handle)
{
    return drv_klad_detach((td_handle)mpi_klad_handle, (crypto_klad_dest)klad_type, (td_handle)mpi_keyslot_handle);
}

td_s32 ot_mpi_klad_set_attr(crypto_handle mpi_klad_handle, const km_klad_attr *attr)
{
    crypto_klad_attr klad_attr = {0};
    crypto_chk_return(attr == TD_NULL, UAPI_COMPAT_ERRNO(ERROR_MODULE_KM, ERROR_PARAM_IS_NULL), "attr is null");
    crypto_chk_return(memcpy_s(&klad_attr, sizeof(crypto_klad_attr), attr, sizeof(km_klad_attr)) != EOK,
        UAPI_COMPAT_ERRNO(ERROR_MODULE_KM, ERROR_MEMCPY_S), "memcpy_s failed\n");
    return drv_klad_set_attr((td_handle)mpi_klad_handle, &klad_attr);
}

td_s32 ot_mpi_klad_get_attr(crypto_handle mpi_klad_handle, km_klad_attr *attr)
{
    td_s32 ret;
    crypto_klad_attr klad_attr = {0};
    ret = drv_klad_get_attr((td_handle)mpi_klad_handle, &klad_attr);
    if (ret == TD_SUCCESS) {
        crypto_chk_return(memcpy_s(attr, sizeof(km_klad_attr), &klad_attr, sizeof(crypto_klad_attr)) != EOK,
            UAPI_COMPAT_ERRNO(ERROR_MODULE_KM, ERROR_MEMCPY_S), "memcpy_s failed\n");
    }
    return ret;
}

td_s32 ot_mpi_klad_set_session_key(crypto_handle mpi_klad_handle, const km_klad_session_key *key)
{
    crypto_klad_session_key session_key = {0};
    crypto_chk_return(memcpy_s(session_key.key, sizeof(session_key.key), key->key, key->key_size) != EOK,
        UAPI_COMPAT_ERRNO(ERROR_MODULE_KM, ERROR_MEMCPY_S), "memcpy_s failed\n");
    session_key.level = (crypto_klad_level_sel)key->level;
    session_key.alg = (crypto_klad_alg_sel)key->alg;
    session_key.key_length = key->key_size;
    return drv_klad_set_session_key((td_handle)mpi_klad_handle, &session_key);
}

td_s32 ot_mpi_klad_set_content_key(crypto_handle mpi_klad_handle, const km_klad_content_key *key)
{
    crypto_klad_content_key content_key = {0};
    crypto_chk_return(key == TD_NULL, UAPI_COMPAT_ERRNO(ERROR_MODULE_KM, ERROR_PARAM_IS_NULL), "key is null");
    crypto_chk_return(key->key_parity >= KM_KLAD_KEY_PARITY_MAX, UAPI_COMPAT_ERRNO(ERROR_MODULE_KM,
        ERROR_INVALID_PARAM), "content_key.key_parity >= KM_KLAD_KEY_PARITY_MAX\n");
    crypto_chk_return(memcpy_s(content_key.key, sizeof(content_key.key), key->key, key->key_size) != EOK,
        UAPI_COMPAT_ERRNO(ERROR_MODULE_KM, ERROR_MEMCPY_S), "memcpy_s failed\n");
    content_key.key_length = key->key_size;
    content_key.alg = (crypto_klad_alg_sel)key->alg;
    content_key.key_parity = (td_bool)key->key_parity;
    return drv_klad_set_content_key((td_handle)mpi_klad_handle, &content_key);
}

td_s32 ot_mpi_klad_set_clear_key(crypto_handle mpi_klad_handle, const km_klad_clear_key *key)
{
    crypto_klad_clear_key clear_key = {0};
    crypto_chk_return(key == TD_NULL, UAPI_COMPAT_ERRNO(ERROR_MODULE_KM, ERROR_PARAM_IS_NULL), "key is null");
    crypto_chk_return(key->key_parity >= KM_KLAD_KEY_PARITY_MAX,  UAPI_COMPAT_ERRNO(ERROR_MODULE_KM,
        ERROR_INVALID_PARAM), "klad_clear_key.key_parity >= KM_KLAD_KEY_PARITY_MAX\n");
    clear_key.key = key->key;
    clear_key.key_parity = (td_bool)key->key_parity;
    clear_key.key_length = key->key_size;
    clear_key.hmac_type = (crypto_klad_hmac_type)key->hmac_type;
    return drv_klad_set_clear_key((td_handle)mpi_klad_handle, &clear_key);
}

td_s32 ot_mpi_klad_set_effective_key(crypto_handle mpi_klad_handle, const km_klad_effective_key *key)
{
    crypto_klad_effective_key effective_key = {0};
    crypto_chk_return(key == TD_NULL, UAPI_COMPAT_ERRNO(ERROR_MODULE_KM, ERROR_PARAM_IS_NULL), "key is null");
    crypto_chk_return(key->key_parity != TD_FALSE && key->key_parity != TD_TRUE,
        UAPI_COMPAT_ERRNO(ERROR_MODULE_KM, ERROR_INVALID_PARAM), "klad_effective_key.key_parity is invalid\n");
    effective_key.kdf_hard_alg = key->kdf_hard_alg;
    effective_key.key_parity = (td_bool)key->key_parity;
    effective_key.key_size = key->key_size;
    effective_key.salt = key->salt;
    effective_key.salt_length = key->salt_length;
    effective_key.oneway = key->oneway;
    return drv_klad_set_effective_key((td_handle)mpi_klad_handle, &effective_key);
}