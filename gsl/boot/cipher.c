/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
 */
#include "types.h"
#include "platform.h"
#include "lib.h"
#include "../drivers/otp/otp.h"
#include "share_drivers.h"
#include "flash_map.h"
#include "common.h"
#include "securecutil.h"
#include "err_print.h"
#include "soc_errno.h"
#include "soc_drv_common.h"
#include "checkup.h"

td_u8 hash_verify_buf[SHA_256_LEN];

#define KEYSALT_LEN 28
#define KEY_MATERIAL 16
#define SYM_KEY_TYPE_OFF 24
#define IV_LEN 16

int decrypt_data(u32 type, para_enc_info *enc_info, u8 *code_dest, u8 *code_src,
		 u32 code_len, u32 sym_key_type)
{
	volatile td_s32 ret = EXT_SEC_FAILURE;
	unsigned int keyslot_handle;
	ext_symc_alg symc_alg;

	ext_km_rootkey_type rootkey_type = 0;

	unsigned char keysalt[KEYSALT_LEN];
	ret = memset_ss(keysalt, KEYSALT_LEN, 0, KEYSALT_LEN, NO_CHECK_WORD);
	if (ret != EXT_SEC_SUCCESS) {
		return EXT_SEC_FAILURE;
	}

	ret = memcpy_ss(keysalt, KEY_MATERIAL, enc_info->key_material, KEY_MATERIAL, NO_CHECK_WORD);
	if (ret != EXT_SEC_SUCCESS) {
		return EXT_SEC_FAILURE;
	}

	*(td_u32 *)(keysalt + SYM_KEY_TYPE_OFF) = sym_key_type;

	if (type == REE) {
		rootkey_type = EXT_KM_ROOTKEY_TYPE_ERK_REE;
	} else if (type == TEE) {
		rootkey_type = EXT_KM_ROOTKEY_TYPE_ERK_TEE;
	} else {
		return EXT_SEC_FAILURE;
	}
	/* alg_sel */
#ifdef CFG_SM_SUPPORT
	symc_alg = EXT_SYMC_ALG_SM4;
	ret = gsl_drv_cipher_create_keyslot_by_set_effective_key(
		&keyslot_handle, rootkey_type, keysalt, KEYSALT_LEN, symc_alg);
#else
	symc_alg = EXT_SYMC_ALG_AES;
	ret = ext_drv_cipher_create_keyslot_by_set_effective_key(
		&keyslot_handle, rootkey_type, keysalt, KEYSALT_LEN);
#endif
	if (ret != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;
#ifdef CFG_SM_SUPPORT
	ret = gsl_drv_cipher_symc_decrypt(symc_alg, keyslot_handle,
					enc_info->iv, IV_LEN, (uint32_t)code_src,
					(uint32_t)code_dest, code_len);
#else
	ret = ext_drv_cipher_symc_decrypt(symc_alg, keyslot_handle,
					enc_info->iv, IV_LEN, (uint32_t)code_src,
					(uint32_t)code_dest, code_len);
#endif
	if (ret != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;
#ifdef CFG_DEBUG_INFO
	log_serial_puts((const s8 *)"decrypt_data ok\n");
#endif
	return EXT_SEC_SUCCESS;
}

td_s32 calc_hash(td_u32 src_addr, td_u32 src_len, td_u8 *data_sha,
		 td_u32 data_sha_len, td_u32 check_word)
{
	volatile td_s32 ret = EXT_SEC_FAILURE;
#ifdef CFG_SM_SUPPORT
	ret = gsl_drv_cipher_sm3((td_u8 *)(uintptr_t)src_addr, src_len,
				    data_sha, data_sha_len);
#else
	ret = ext_drv_cipher_sha256((td_u8 *)(uintptr_t)src_addr, src_len,
					data_sha, data_sha_len);
#endif
	if (ret != EXT_SEC_SUCCESS) {
		return EXT_SEC_FAILURE;
	}
	return ret;
}

#ifndef CFG_SM_SUPPORT
static td_s32 store_to_hash_buf(td_u8 *hash_buf, td_u8 *data_hash)
{
	td_s32 ret = EXT_SEC_FAILURE;

	ret = memcpy_ss(hash_buf, SHA_256_LEN, data_hash, SHA_256_LEN,
			NO_CHECK_WORD);
	if (ret != EXT_SEC_SUCCESS) {
		return EXT_SEC_FAILURE;
	}

	return EXT_SEC_SUCCESS;
}
#endif

td_s32 verify_signature(const td_u8 *pub_key, const ext_data *data, const td_u8 *sign)
{
	td_s32 ret = EXT_SEC_FAILURE;
	ext_pke_data input_hash = {0};
	td_u8 hash_buf[HASH_LEN];
	if (memset_s(hash_buf, sizeof(hash_buf), 0, HASH_LEN) != EOK)
		return EXT_SEC_FAILURE;
	input_hash.data = hash_buf;
	input_hash.length = SHA_256_LEN;
#ifdef CFG_SM_SUPPORT
	ext_pke_ecc_point sm_pub_key = {0};
	ext_pke_ecc_sig sm_sig = {0};
	ext_pke_data sm2_id = {0};
	ext_pke_data msg = {0};
	uint8_t sm2_id_buf[16];
	if (memcpy_s(sm2_id_buf, sizeof(sm2_id_buf), "1234567812345678", 16) != EOK)
		return EXT_SEC_FAILURE;
	sm2_id.data = sm2_id_buf;
	sm2_id.length = 16;
	msg.data = data->data;
	msg.length = data->length;
	sm_pub_key.x = (td_u8 *)pub_key;
	sm_pub_key.y = (td_u8 *)(pub_key + 32);
	sm_pub_key.length = 32;
	sm_sig.r = (td_u8 *)sign;
	sm_sig.s = (td_u8 *)(sign + 32);
	sm_sig.length = 32;
	gsl_drv_cipher_pke_sm2_dsa_hash(&sm2_id, &sm_pub_key, &msg, &input_hash);
	ret = gsl_drv_cipher_pke_sm2_verify(&sm_pub_key, &input_hash, &sm_sig);
#else
	td_u8 data_hash[SHA_256_LEN];
	ext_pke_rsa_pub_key rsa_pub_key;
	ext_pke_data rsa_sign;
	td_u8 exp[RSA_KEY_OFFSET];
	if (memset_s(exp, sizeof(exp), 0, RSA_KEY_OFFSET) != EOK)
		return EXT_SEC_FAILURE;
	exp[RSA_KEY_OFFSET - 4] = *(pub_key + RSA_KEY_OFFSET);
	exp[RSA_KEY_OFFSET - 3] = *(pub_key + RSA_KEY_OFFSET + 1);
	exp[RSA_KEY_OFFSET - 2] = *(pub_key + RSA_KEY_OFFSET + 2);
	exp[RSA_KEY_OFFSET - 1] = *(pub_key + RSA_KEY_OFFSET + 3);
	rsa_pub_key.n = (td_u8 *)pub_key;
	rsa_pub_key.e = (td_u8 *)exp;
	rsa_pub_key.len = RSA_KEY_LEN;
	rsa_sign.length = RSA_SIGN_LEN;
	rsa_sign.data = (td_u8 *)sign;
	/* Initialise hash arrays and structs */
	if (memset_s(data_hash, sizeof(data_hash), 0x5a, SHA_256_LEN) != EOK)
		return EXT_SEC_FAILURE;
	(void)calc_hash((uintptr_t)data->data, data->length, data_hash,
		SHA_256_LEN, 0);
	if (store_to_hash_buf(hash_buf, data_hash) != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;
	ret = ext_drv_cipher_pke_rsa_verify(&rsa_pub_key,
					EXT_PKE_RSA_SCHEME_PKCS1_V21,
					EXT_PKE_HASH_TYPE_SHA256,
					&input_hash, &rsa_sign);
#endif
	return ret;
}


void lock_ree_bootkey(void)
{
	reg_set(RKP_LOCK, RKP_REE_LOCK);
	reg_set(RKP_ONEWAY, LOCK_REE_KEY);
	reg_set(RKP_LOCK, RKP_UNLOCK);
}


void lock_tee_bootkey(void)
{
	reg_set(RKP_LOCK, RKP_TEE_LOCK);
	reg_set(RKP_ONEWAY, LOCK_TEE_REE_KEY);
	reg_set(RKP_LOCK, RKP_UNLOCK);
}


int dma_copy(uintptr_t dest, u32 count, uintptr_t src)
{
#ifdef CFG_SM_SUPPORT
	return gsl_drv_cipher_dma_copy(src, dest, count);
#else
	return ext_drv_cipher_dma_copy(src, dest, count);
#endif
}
