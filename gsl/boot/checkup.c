/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
 */
#include "checkup.h"
#include "../drivers/otp/otp.h"
#include "../drivers/tzasc/tzasc.h"
#include "backup.h"
#include "cipher.h"
#include "common.h"
#include "err_print.h"
#include "flash_map.h"
#include "lib.h"
#include "lpds.h"
#include "platform.h"
#include "securecutil.h"
#include "share_drivers.h"
#include "soc_errno.h"
#include "td_type.h"
#include "types.h"

td_u8 g_sm_ree_root_public_key[64];
td_u8 g_sm_gsl_external_public_key[64];

static u32 count_bit_set(u32 input)
{
	u32 count = 0; /* count accumulates the total bits set in input */
	for (count = 0; input; count++) {
		input &= input - 1; /* clear the least significant bit set */
	}
	return count;
}

static ALWAYS_INLINE int ret_redun_check(int ret, int err)
{
	for (int i = 0; i < REDUN_CHECK_CNT; i++) {
		if (ret != EXT_SEC_SUCCESS) {
			err_print(err);
			return EXT_SEC_FAILURE;
		}
	}
	return EXT_SEC_SUCCESS;
}

u32 get_gsl_code_info_addr(void)
{
	return (u32)(uintptr_t)GSL_CODE_INFO_ADDR;
}

u32 get_gsl_code_addr(void)
{
	return get_gsl_code_info_addr() + GSL_CODE_INFO_SIZE;
}

u32 get_gsl_code_area_len(void)
{
	gsl_code_info *gsl_info = (gsl_code_info *)(uintptr_t)GSL_CODE_INFO_ADDR;
	return (u32)gsl_info->code_area_len;
}

u32 get_gsl_heap_addr(void)
{
	u32 addr = GSL_CODE_AREA_ADDR + get_gsl_code_area_len();
	if ((addr % SIZE_1K) != 0) {
		addr += SIZE_1K;
		addr &= ~(SIZE_1K - 1);
	}
	return addr;
}

u32 get_gsl_heap_len(void)
{
	return (u32)(uintptr_t)MALLOC_SIZE;
}

u32 get_ree_key_area_addr(void)
{
	return get_gsl_heap_addr() + get_gsl_heap_len();
}

u32 get_ddr_param_info_addr(void)
{
	return get_ree_key_area_addr() + REE_BOOT_KEY_AREA_SIZE;
}

u32 get_ddr_param_data_addr(void)
{
	return get_ddr_param_info_addr() + PARM_AREA_INFO_SIZE;
}

u32 get_ddr_param_data_end_addr(void)
{
	u32 addr = get_ddr_param_data_addr() + get_ddr_param_len();
	if ((addr % SIZE_1K) != 0) {
		addr += SIZE_1K;
		addr &= ~(SIZE_1K - 1);
	}
	return addr;
}

uintptr_t get_uboot_info_download_ddr_addr(void)
{
	return DDR_DOWNLOAD_ADDR;
}

uintptr_t get_uboot_code_ddr_addr(void)
{
	return get_uboot_info_download_ddr_addr() + UBOOT_CODE_INFO_SIZE;
}

uintptr_t get_uboot_code_size(void)
{
	uboot_code_info *uboot_info = (uboot_code_info *)(uintptr_t)get_uboot_info_download_ddr_addr();
	return uboot_info->code_area_len;
}

uintptr_t get_uboot_entrypoint_ddr_addr(void)
{
	uboot_code_info *uboot_info = (uboot_code_info *)(uintptr_t)get_uboot_info_download_ddr_addr();
	return uboot_info->uboot_entry_point;
}

static s32 check_img_id(u32 image_img_id, u32 define_img_id)
{
	if (image_img_id != define_img_id) {
		err_print(VERIFY_IMG_ID_ERR);
		return EXT_SEC_FAILURE;
	}
	return EXT_SEC_SUCCESS;
}

static s32 check_version(const uintptr_t addr, const u32 img_ver, const u32 img_mask)
{
	u32 otp_boot_ver = 0x0;
	if (ext_drv_cipher_otp_read_word(addr, &otp_boot_ver) != EXT_SEC_SUCCESS) {
		err_print(DRV_OTP_READ_WORD_ERR);
		return EXT_SEC_FAILURE;
	}
	if (count_bit_set(img_ver & img_mask) < count_bit_set(otp_boot_ver & img_mask)) {
		err_print(VERSION_CMP_BIT_ERR);
		return EXT_SEC_FAILURE;
	}
#ifdef CFG_DEBUG_INFO
	log_serial_puts((const s8 *)"check_version ok\n");
#endif
	return EXT_SEC_SUCCESS;
}

static s32 check_version_with_num(const u32 img_ver, const uintptr_t start_addr,
		const u32 start_mask, const uintptr_t end_addr, const u32 end_mask)
{
	u32 otp_boot_ver;
	uintptr_t addr;
	u32 count = 0;
	for (addr = start_addr; addr <= end_addr; addr += BYTE_NUMS) {
		if (ext_drv_cipher_otp_read_word(addr, &otp_boot_ver) != EXT_SEC_SUCCESS) {
			err_print(DRV_OTP_READ_WORD_ERR);
			return EXT_SEC_FAILURE;
		}
		if (addr == start_addr)
			otp_boot_ver &= start_mask;
		else if (addr == end_addr)
			otp_boot_ver &= end_mask;
		else
			otp_boot_ver &= 0xffffffff;
		count += count_bit_set(otp_boot_ver);
	}

	if (img_ver < count || img_ver > TEE_VERSION_BOUND) {
		err_print(VERSION_CMP_COUNT_ERR);
		return EXT_SEC_FAILURE;
	}
#ifdef CFG_DEBUG_INFO
	log_serial_puts((const s8 *)"check_version_with_num ok\n");
#endif
	return EXT_SEC_SUCCESS;
}

static u32 check_msid_ext(u32 msid_ext, u32 mask_msid_ext)
{
	u32 msid_otp;
	if (ext_drv_cipher_otp_read_word(OTP_MSID, &msid_otp) != EXT_SEC_SUCCESS) {
		err_print(DRV_OTP_READ_WORD_ERR);
		return EXT_SEC_FAILURE;
	}
	if ((msid_otp & mask_msid_ext) != (msid_ext & mask_msid_ext)) {
		err_print(VERIFY_MSID_ERR);
		return EXT_SEC_FAILURE;
	}
#ifdef CFG_DEBUG_INFO
	log_serial_puts((const s8 *)"check_msid_ext ok\n");
#endif
	return EXT_SEC_SUCCESS;
}

static s32 die_id_compare(const u8 *die_id)
{
	u8 otp_die_id[DIE_ID_LEN];
	volatile s32 ret = EXT_SEC_FAILURE;
	ret = memset_ss(otp_die_id, DIE_ID_LEN, 0, DIE_ID_LEN, NO_CHECK_WORD);
	if (ret != EXT_SEC_SUCCESS) {
		err_print(MEMSET_ERR);
		return EXT_SEC_FAILURE;
	}
	otp_get_die_id(otp_die_id, DIE_ID_LEN);
	if (ret != EXT_SEC_SUCCESS) {
		err_print(DRV_OTP_GET_DIE_ID_ERR);
		return EXT_SEC_FAILURE;
	}
	ret = memcmp_ss(die_id, otp_die_id, DIE_ID_LEN, NO_CHECK_WORD);
	if (ret != EXT_SEC_SUCCESS) {
		err_print(MAINTENACE_MODE_DIE_ID_CMP_FAIL);
		return EXT_SEC_FAILURE;
	}
#ifdef CFG_DEBUG_INFO
	log_serial_puts((const s8 *)"die_id_compare ok\n");
#endif
	return EXT_SEC_SUCCESS;
}

static s32 maintenance_mode_check(const u32 maintenance_mode, const u8 *die_id)
{
	if (maintenance_mode == MAINTENACE_MODE_ENABLE)
		return die_id_compare(die_id);
	return EXT_SEC_SUCCESS;
}

s32 hash_compare(uintptr_t area_addr, u32 area_len, const u8 *hash_addr)
{
	volatile s32 ret = EXT_SEC_FAILURE;
	u8 hash_result[HASH_LEN] __attribute__((aligned(8))); /* aligned 8 bytes */
	ret = memset_ss(hash_result, HASH_LEN, 0x5a, HASH_LEN, NO_CHECK_WORD);
	if (ret != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;
	ret = EXT_SEC_FAILURE;
	ret = calc_hash((uintptr_t)area_addr, area_len, hash_result, HASH_LEN, NO_CHECK_WORD);
	if (ret != EXT_SEC_SUCCESS) {
		err_print(CALC_HASH_ERR);
		return EXT_SEC_FAILURE;
	}
	/* Check the hash result */
	ret = EXT_SEC_FAILURE;
	ret = memcmp_ss(hash_result, hash_addr, HASH_LEN, NO_CHECK_WORD);
	if (ret != EXT_SEC_SUCCESS) {
		err_print(HASH_CMP_FAIL);
		return EXT_SEC_FAILURE;
	}
	return ret;
}

static td_s32 secure_verify_area(const td_u8 *pub_key, const ext_data *data, const td_u8 *sign)
{
	volatile td_s32 ret = EXT_SEC_FAILURE;
	if ((pub_key == TD_NULL) || (data == TD_NULL) || (sign == TD_NULL)) {
		return EXT_SEC_FAILURE;
	}

	ret = verify_signature(pub_key, data, sign);
	for (int i = 0; i < REDUN_CHECK_CNT; i++) {
		if (ret != EXT_SEC_SUCCESS)
			return EXT_SEC_FAILURE;
	}
#ifdef CFG_DEBUG_INFO
	log_serial_puts((const s8 *)"\nsecure_verify_area ok\n");
#endif
	return ret;
}

static u8 get_ree_verify_enable_flag(void)
{
	return (u8)reg_getbits(REE_VER_EN_FLAG_ADDR, REE_VER_EN_FLAG_BIT_OFFSET, REE_VER_EN_FLAG_BIT_WIDTH);
}

static u8 get_tee_verify_enable_flag(void)
{
	return (u8)reg_getbits(TEE_VER_EN_FLAG_ADDR, TEE_VER_EN_FLAG_BIT_OFFSET, TEE_VER_EN_FLAG_BIT_WIDTH);
}

s32 handle_ree_key_area(void)
{
	s32 ret;
	ree_key_area_s *ree_key_area = NULL;

	td_u8 *root_pub_key = NULL;
	td_u8 *sign = NULL;
	ext_data data;
#ifndef CFG_SM_SUPPORT
	flash_root_public_key_s *flash_root_key = (flash_root_public_key_s *)(uintptr_t)REE_FLASH_ROOT_PUB_KEY_ADDR;
#endif
	ree_key_area = (ree_key_area_s *)(uintptr_t)get_ree_key_area_addr();

	if (get_ree_verify_enable_flag() == DISABLE) {
		if (get_ree_verify_enable_flag() != DISABLE)
			return EXT_SEC_FAILURE;
		return EXT_SEC_SUCCESS;
	}
	ret = check_img_id(ree_key_area->img_id, REE_KEY_AREA_IMG_ID);
	if (ret != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;
	for (int i = 0; i < REDUN_CHECK_CNT; i++) {
		if ((reg_get(REE_FLASH_ROOTKEY_STATUS_ADDR) & REE_FLASH_ROOTKEY_STATUS_MASK) !=
		    REE_FLASH_ROOTKEY_STATUS_VALID) {
			err_print(REE_FLASH_ROOTKEY_STATUS_INVALID);
			return EXT_SEC_FAILURE;
		}
	}
	data.data = (td_u8 *)ree_key_area;
#ifdef CFG_SM_SUPPORT
		root_pub_key = g_sm_ree_root_public_key;
		data.length = sizeof(ree_key_area_s) - SM_SIG_LEN;
		sign = ree_key_area->sm.signature;
#else
		root_pub_key = (td_u8 *)&flash_root_key->root_key_area;
		data.length = sizeof(ree_key_area_s) - RSA_SIG_LEN;
		sign = ree_key_area->rsa.signature;
#endif
	ret = secure_verify_area(root_pub_key, &data, sign);
	if (ret != EXT_SEC_SUCCESS) {
		err_print(SECURE_VERIFY_ERR);
		return EXT_SEC_FAILURE;
	}

	ret = check_version(OTP_REE_BOOT_VERSION, ree_key_area->version_ext, ree_key_area->mask_version_ext);
	if (ret != EXT_SEC_SUCCESS) {
		err_print(VERIFY_VERSION_ERR);
		return EXT_SEC_FAILURE;
	}

	ret = check_msid_ext(ree_key_area->msid_ext, ree_key_area->mask_msid_ext);
	if (ret != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;

	ret = maintenance_mode_check(ree_key_area->maintenance_mode, ree_key_area->die_id);
	if (ret != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;

	add_boot_img_size(REE_BOOT_KEY_AREA_SIZE);
#ifdef CFG_DEBUG_INFO
	log_serial_puts((const s8 *)"handle_ree_key_area ok\n");
#endif
	return EXT_SEC_SUCCESS;
}

static s32 params_info_check(u32 board_index)
{
	u32 para_cnt = get_ddr_param_cnt();
	u32 para_len = get_ddr_param_len();
	para_area_info *para_info = (para_area_info *)(uintptr_t)get_ddr_param_info_addr();
	if (para_cnt > MAX_DDR_PARA_NUM) {
		log_serial_puts((const s8 *)"\r\nerr0:max num 0x");
		serial_put_hex(MAX_DDR_PARA_NUM);
		err_print(PARA_AREA_NUM_ERR);
		return EXT_SEC_FAILURE;
	}
	if (para_info->board_index_hash_table[board_index] == INVALID_PAM_HASH_VAL) {
		log_serial_puts((const s8 *)"\r\nerr0:invalid param table image offset 0x");
		serial_put_hex(para_info->board_index_hash_table[board_index]);
		return EXT_SEC_FAILURE;
	}
	if (para_info->board_index_hash_table[board_index] >= para_cnt) {
		log_serial_puts((const s8 *)"\r\nerr1:max num 0x");
		serial_put_hex(para_cnt);
		err_print(PARA_AREA_NUM_ERR);
		return EXT_SEC_FAILURE;
	}
	if (para_len > SIZE_15K) {
		log_serial_puts((const s8 *)"\nerr2:max lenth 0x");
		serial_put_hex(SIZE_15K);
		err_print(PARA_AREA_LEN_ERR);
		return EXT_SEC_FAILURE;
	}
	if (para_len == 0) {
		err_print(PARA_AREA_LEN_ERR);
		return EXT_SEC_FAILURE;
	}

	return EXT_SEC_SUCCESS;
}

s32 handle_ddr_param_info(u32 board_index)
{
	s32 ret = EXT_SEC_FAILURE;
	td_u8 *pub_key = NULL;
	td_u8 *sign = NULL;
	ext_data data;
	ree_key_area_s *ree_key_area = (ree_key_area_s *)(uintptr_t)get_ree_key_area_addr();
	para_area_info *para_info = (para_area_info *)(uintptr_t)get_ddr_param_info_addr();

	if (params_info_check(board_index) != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;
	ret = check_img_id(para_info->img_id, PARAMS_INFO_IMG_ID);
	if (ret != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;

	if (get_ree_verify_enable_flag() == DISABLE) {
		if (get_ree_verify_enable_flag() != DISABLE)
			return EXT_SEC_FAILURE;
		return EXT_SEC_SUCCESS;
	}

	data.data = (td_u8 *)para_info;
#ifdef CFG_SM_SUPPORT
		pub_key = ree_key_area->sm.ext_pulic_key_area;
		data.length = sizeof(para_area_info) - SM_SIG_LEN;
		sign = para_info->sm.signature;
#else
		pub_key = ree_key_area->rsa.ext_pulic_key_area;
		data.length = sizeof(para_area_info) - RSA_SIG_LEN;
		sign = para_info->rsa.signature;
#endif
	ret = secure_verify_area(pub_key, &data, sign);
	if (ret != EXT_SEC_SUCCESS) {
#ifdef CFG_DEBUG_INFO
		log_serial_puts((const s8 *)"handle_ddr_param_info err\n");
#endif
	}
	if (ret_redun_check(ret, SECURE_VERIFY_ERR) != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;

	ret = check_version(OTP_PARAMS_VERSION, para_info->version_ext, para_info->mask_version_ext);
	if (ret != EXT_SEC_SUCCESS) {
		err_print(VERIFY_VERSION_ERR);
		return EXT_SEC_FAILURE;
	}

	ret = check_msid_ext(para_info->msid_ext, para_info->mask_msid_ext);
	if (ret != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;

	add_boot_img_size(PARM_AREA_INFO_SIZE + para_info->para_area_addr);
#ifdef CFG_DEBUG_INFO
	log_serial_puts((const s8 *)"handle_ddr_param_info ok\n");
#endif
	return EXT_SEC_SUCCESS;
}

u32 get_ddr_param_len()
{
	para_area_info *para_info = (para_area_info *)(uintptr_t)get_ddr_param_info_addr();
	return para_info->para_area_len;
}

u32 get_ddr_param_cnt()
{
	para_area_info *para_info = (para_area_info *)(uintptr_t)get_ddr_param_info_addr();
	return para_info->para_area_num;
}

s32 handle_ddr_param(u32 board_index)
{
	s32 ret;
	para_area_info *para_info = NULL;
	uintptr_t ddr_param_area;
	para_info = (para_area_info *)(uintptr_t)get_ddr_param_info_addr();
	ddr_param_area = (uintptr_t)get_ddr_param_data_addr();
	if (get_ree_verify_enable_flag() == DISABLE) {
		if (get_ree_verify_enable_flag() != DISABLE)
			return EXT_SEC_FAILURE;
		return EXT_SEC_SUCCESS;
	}
	if (para_info->para_area_num <= para_info->board_index_hash_table[board_index]) {
		err_print(PARA_AREA_NUM_ERR);
		return EXT_SEC_FAILURE;
	}
	for (int i = 0; i < REDUN_CHECK_CNT; i++) {
		ret = hash_compare(ddr_param_area, para_info->para_area_len, para_info->para_area_hash[board_index]);
		if (ret != EXT_SEC_SUCCESS) {
#ifdef CFG_DEBUG_INFO
			log_serial_puts((const s8 *)"handle_ddr_param err\n");
#endif
		}
		if (ret != EXT_SEC_SUCCESS)
			return EXT_SEC_FAILURE;
	}

	add_boot_img_size(para_info->para_area_num * para_info->para_area_len);
#ifdef CFG_DEBUG_INFO
	log_serial_puts((const s8 *)"handle_ddr_param ok\n");
#endif
	return EXT_SEC_SUCCESS;
}

s32 handle_uboot_info(void)
{
	s32 ret = EXT_SEC_FAILURE;
	td_u8 *pub_key = NULL;
	td_u8 *sign = NULL;
	ext_data data;
	ree_key_area_s *ree_key_area = (ree_key_area_s *)(uintptr_t)get_ree_key_area_addr();
	uboot_code_info *uboot_info = (uboot_code_info *)(uintptr_t)get_uboot_info_download_ddr_addr();
	ret = check_img_id(uboot_info->img_id, UBOOT_INFO_IMG_ID);
	if (ret != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;
	if (get_ree_verify_enable_flag() == DISABLE) {
		if (get_ree_verify_enable_flag() != DISABLE)
			return EXT_SEC_FAILURE;
		return EXT_SEC_SUCCESS;
	}

	data.data = (td_u8 *)uboot_info;
#ifdef CFG_SM_SUPPORT
		pub_key = ree_key_area->sm.ext_pulic_key_area;
		data.length = sizeof(uboot_code_info) - SM_SIG_LEN;
		sign = uboot_info->sm.signature;
#else
		pub_key = ree_key_area->rsa.ext_pulic_key_area;
		data.length = sizeof(uboot_code_info) - RSA_SIG_LEN;
		sign = uboot_info->rsa.signature;
#endif
	ret = secure_verify_area(pub_key, &data, sign);
	if (ret_redun_check(ret, SECURE_VERIFY_ERR) != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;

	ret = check_version(OTP_REE_BOOT_VERSION, uboot_info->version_ext, uboot_info->mask_version_ext);
	if (ret != EXT_SEC_SUCCESS) {
		err_print(VERIFY_VERSION_ERR);
		return EXT_SEC_FAILURE;
	}

	ret = check_msid_ext(uboot_info->msid_ext, uboot_info->mask_msid_ext);
	if (ret != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;

	add_boot_img_size(UBOOT_CODE_INFO_SIZE);
#ifdef CFG_DEBUG_INFO
	log_serial_puts((const s8 *)"handle_uboot_info ok\n");
#endif
	return EXT_SEC_SUCCESS;
}

s32 handle_uboot_code(void)
{
	s32 ret;
	uintptr_t boot_code_addr;
	unsigned char *uboot_dec_addr;
	uboot_code_info *uboot_info = NULL;
	u32 uboot_dec_len = 0;
	uboot_info = (uboot_code_info *)(uintptr_t)get_uboot_info_download_ddr_addr();
	boot_code_addr = (uintptr_t)(uboot_info->uboot_entry_point);
	uboot_dec_addr = (unsigned char *)(uintptr_t)(uboot_info->uboot_entry_point);
	uboot_dec_len = uboot_info->code_area_len;

	if (!(is_ree_boot_dec_en_enable() == AUTH_FAILURE && uboot_info->enc_info.code_enc_flag == CODE_ENC_DISABLE)) {
		ret = decrypt_data(REE, &(uboot_info->enc_info), uboot_dec_addr, uboot_dec_addr, uboot_dec_len, UBOOT_ROOTKEY);
		if (ret_redun_check(ret, DEC_CODE_ERR) != EXT_SEC_SUCCESS)
			return EXT_SEC_FAILURE;
	}
	if (get_ree_verify_enable_flag() == DISABLE) {
		if (get_ree_verify_enable_flag() != DISABLE)
			return EXT_SEC_FAILURE;
		return EXT_SEC_SUCCESS;
	}
	for (int i = 0; i < REDUN_CHECK_CNT; i++) {
		ret = hash_compare(boot_code_addr, uboot_dec_len, uboot_info->code_area_hash);
		if (ret != EXT_SEC_SUCCESS)
			return EXT_SEC_FAILURE;
	}
	add_boot_img_size(uboot_info->code_area_len);
#ifdef CFG_DEBUG_INFO
	log_serial_puts((const s8 *)"handle_uboot_code ok\n");
#endif
	return EXT_SEC_SUCCESS;
}

s32 handle_tee_key(uintptr_t tee_key_addr)
{
	s32 ret;
	td_u8 *pub_key = NULL;
	td_u8 *sign = NULL;
	ext_data data;
	tee_key_area_s *tee_key_area = NULL;
	tee_key_area = (tee_key_area_s *)(tee_key_addr);

#ifndef CFG_SM_SUPPORT
	gsl_key_area_s *gsl_key_area = (gsl_key_area_s *)(uintptr_t)(GSL_KEY_AREA_ADDR);
#endif
	
	if (is_soc_tee_enable() == AUTH_FAILURE || get_tee_verify_enable_flag() == DISABLE) {
		if (is_soc_tee_enable() == AUTH_FAILURE || get_tee_verify_enable_flag() == DISABLE)
			return EXT_SEC_SUCCESS;
	}
	ret = check_img_id(tee_key_area->img_id, TEE_KEY_AREA_IMG_ID);
	if (ret != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;
	data.data = (td_u8 *)tee_key_area;
#ifdef CFG_SM_SUPPORT
		pub_key = g_sm_gsl_external_public_key;
		data.length = sizeof(tee_key_area_s) - SM_SIG_LEN;
		sign = tee_key_area->sm.signature;
#else
		pub_key = gsl_key_area->ext_pulic_key_area;
		data.length = sizeof(tee_key_area_s) - RSA_SIG_LEN;
		sign = tee_key_area->rsa.signature;
#endif
	ret = secure_verify_area(pub_key, &data, sign);
	if (ret_redun_check(ret, SECURE_VERIFY_ERR) != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;
	ret = check_version(OTP_TEE_OS_VERSION, tee_key_area->version_ext, tee_key_area->mask_version_ext);
	if (ret != EXT_SEC_SUCCESS) {
		err_print(VERIFY_VERSION_ERR);
		return EXT_SEC_FAILURE;
	}
	ret = check_msid_ext(tee_key_area->msid_ext, tee_key_area->mask_msid_ext);
	if (ret != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;
	ret = maintenance_mode_check(tee_key_area->maintenance_mode, tee_key_area->die_id);
	if (ret != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;
#ifdef CFG_DEBUG_INFO
	log_serial_puts((const s8 *)"handle_tee_key ok\n");
#endif
	return EXT_SEC_SUCCESS;
}

s32 handle_tee_info(uintptr_t tee_key_addr, uintptr_t tee_info_addr)
{
	s32 ret;
	tee_key_area_s *tee_key_area = NULL;
	tee_code_info *tee_info = NULL;
	td_u8 *pub_key = NULL;
	td_u8 *sign = NULL;
	ext_data data;
	u32 mask_high;
	u32 mask_low;
	tee_info = (tee_code_info *)tee_info_addr;
	ret = check_img_id(tee_info->img_id, TEE_CODE_INFO_IMG_ID);
	if (ret != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;
	if (is_soc_tee_enable() == AUTH_FAILURE || get_tee_verify_enable_flag() == DISABLE) {
		if (is_soc_tee_enable() == AUTH_FAILURE || get_tee_verify_enable_flag() == DISABLE)
			return EXT_SEC_SUCCESS;
	}
	tee_key_area = (tee_key_area_s *)(tee_key_addr);
	data.data = (td_u8 *)tee_info;
#ifdef CFG_SM_SUPPORT
		pub_key = tee_key_area->sm.ext_pulic_key_area;
		data.length = sizeof(tee_code_info) - SM_SIG_LEN;
		sign = tee_info->sm.signature;
#else
		pub_key = tee_key_area->rsa.ext_pulic_key_area;
		data.length = sizeof(tee_code_info) - RSA_SIG_LEN;
		sign = tee_info->rsa.signature;
#endif
	ret = secure_verify_area(pub_key, &data, sign);
	if (ret_redun_check(ret, SECURE_VERIFY_ERR) != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;
	mask_high = ~(tee_key_area->mask_version_ext);
	mask_low = 0xffffffff;
	ret = check_version_with_num(tee_info->version_ext, OTP_TEE_OS_VERSION, mask_high,
			OTP_TEE_OS_VERSION_EX, mask_low);
	if (ret != EXT_SEC_SUCCESS) {
		err_print(VERIFY_VERSION_ERR);
		return EXT_SEC_FAILURE;
	}
	ret = check_msid_ext(tee_info->msid_ext, tee_info->mask_msid_ext);
	if (ret != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;
#ifdef CFG_DEBUG_INFO
	log_serial_puts((const s8 *)"handle_tee_info ok\n");
#endif
	return EXT_SEC_SUCCESS;
}

#define TEE_OS_HEAD 0x1c
s32 handle_tee_code(uintptr_t addr, uintptr_t tee_info_addr, uintptr_t tee_code_addr)
{
	s32 ret;
	unsigned char *tee_dec_dest_addr;
	tee_code_info *tee_info = NULL;
	u32 tee_dec_len = 0;
	tee_info = (tee_code_info *)tee_info_addr;
	tee_dec_len = tee_info->tee_code_area_len;
	void *src_bl32_code_addr = NULL;
	src_bl32_code_addr = (unsigned char *)(addr + TEE_KEY_SIZE + TEE_CODE_INFO_SIZE);
	tee_dec_dest_addr = (unsigned char *)(uintptr_t)BL32_LOAD_ADDR;
	if (tee_dec_len == ZERO || tee_dec_len > BL32_SIZE)
		return EXT_SEC_FAILURE;
	if (is_tee_dec_en_enable() == AUTH_FAILURE && tee_info->enc_info.code_enc_flag == CODE_ENC_DISABLE) {
		if (is_tee_dec_en_enable() == AUTH_FAILURE && tee_info->enc_info.code_enc_flag == CODE_ENC_DISABLE)
			ret = dma_copy(BL32_LOAD_ADDR, tee_dec_len, (uintptr_t)src_bl32_code_addr);
		else
			ret = decrypt_data(TEE, &(tee_info->enc_info), tee_dec_dest_addr, src_bl32_code_addr, tee_dec_len,
					   TEE_ROOTKEY);
	} else {
		ret = decrypt_data(TEE, &(tee_info->enc_info), tee_dec_dest_addr, src_bl32_code_addr, tee_dec_len, TEE_ROOTKEY);
	}

#ifdef CFG_DEBUG_INFO
	log_serial_puts((const s8 *)"handle_tee_code\n");
#endif
	if (ret_redun_check(ret, DEC_CODE_ERR) != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;
	if (is_soc_tee_enable() == AUTH_FAILURE || get_tee_verify_enable_flag() == DISABLE) {
		return EXT_SEC_SUCCESS;
	}
	for (int i = 0; i < REDUN_CHECK_CNT; i++) {
		ret = hash_compare(tee_code_addr, tee_info->tee_code_area_len, tee_info->tee_code_area_hash);
		if (ret != EXT_SEC_SUCCESS)
			return EXT_SEC_FAILURE;
	}
#ifdef CFG_DEBUG_INFO
	log_serial_puts((const s8 *)"handle_tee_code ok\n");
#endif
	ret = memset_ss((void *)BL32_LOAD_ADDR, TEE_OS_HEAD, 0, TEE_OS_HEAD, NO_CHECK_WORD);
	if (ret != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;
	return EXT_SEC_SUCCESS;
}

#define DDRC_LOCK_CTRL1 0x4144
#define DDRC_LOCK_CTRL2 0x4148
#define DDRC_LOCK_CTRL3 0x414c
#define DDRC_LOCK_CTRL4 0x4150
#define DDRC_LOCK_CTRL5 0x4154

static void ddrc_lock(void)
{
	reg_set(REG_BASE_MISC + DDRC_LOCK_CTRL1, 0x1);
	reg_set(REG_BASE_MISC + DDRC_LOCK_CTRL2, 0x1);
	reg_set(REG_BASE_MISC + DDRC_LOCK_CTRL3, 0x1);
	reg_set(REG_BASE_MISC + DDRC_LOCK_CTRL4, 0x1);
	reg_set(REG_BASE_MISC + DDRC_LOCK_CTRL5, 0x1);
}

static int config_ddr_security(size_t sec_rgn_size)
{
	int ret;
	u32 rgn_attr;
	const size_t ddr_region_size = DDR_END - DDR_BASE;
	rgn_attr = TZASC_ATTR_SEC_R | TZASC_ATTR_SEC_W | TZASC_ATTR_NOSEC_R | TZASC_ATTR_NOSEC_W | TZASC_ATTR_SEC_INV;
	config_tzasc(TZASC_RGN_1, rgn_attr, DDR_BASE, ddr_region_size);
	rgn_attr = TZASC_ATTR_SEC_R | TZASC_ATTR_SEC_W | TZASC_ATTR_SEC_INV;
	config_tzasc(TZASC_RGN_2, rgn_attr, SEC_DDR_BASE, sec_rgn_size);
	ret = tzasc_sec_config_read_back(TZASC_RGN_2, SEC_DDR_BASE, sec_rgn_size);
	if (ret != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;
	ddrc_lock();
	ret = tzasc_bypass_disable();
	if (ret != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;
	return EXT_SEC_SUCCESS;
}

int load_tee_img(uintptr_t addr)
{
	int ret;
	tee_code_info *tee_info = NULL;
	size_t sec_rgn_size = BL32_SIZE + TEE_OS_KEY_SIZE;

	ret = config_ddr_security(sec_rgn_size);
	if (ret != EXT_SEC_SUCCESS) {
		err_print(DDR_CONFIG_ERR);
		return EXT_SEC_FAILURE;
	}

	ret = dma_copy(TEE_OS_KEY_ADDR, TEE_KEY_SIZE + TEE_CODE_INFO_SIZE, addr);
	if (ret != EXT_SEC_SUCCESS) {
#ifdef CFG_DEBUG_INFO
		log_serial_puts((const s8 *)"load_tee_img failed\n");
#endif
		return EXT_SEC_FAILURE;
	}
	ret = handle_tee_key((uintptr_t)TEE_OS_KEY_ADDR);
	if (ret != EXT_SEC_SUCCESS) {
		err_print(HANDLE_TEE_KEY_ERR);
		return EXT_SEC_FAILURE;
	}
	ret = handle_tee_info((uintptr_t)TEE_OS_KEY_ADDR, (uintptr_t)(TEE_OS_KEY_ADDR + TEE_KEY_SIZE));
	if (ret != EXT_SEC_SUCCESS) {
		err_print(HANDLE_TEE_INFO_ERR);
		return EXT_SEC_FAILURE;
	}
	tee_info = (tee_code_info *)(uintptr_t)(TEE_OS_KEY_ADDR + TEE_KEY_SIZE);
	ret = handle_tee_code(addr, (uintptr_t)(tee_info), (uintptr_t)BL32_LOAD_ADDR);
	if (ret != EXT_SEC_SUCCESS) {
		err_print(HANDLE_TEE_CODE_ERR);
		return EXT_SEC_FAILURE;
	}
	return EXT_SEC_SUCCESS;
}
