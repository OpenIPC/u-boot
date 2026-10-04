/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: otp_maps_h
 * Author: Hisilicon multimedia software group
 * Create: 2023-05-08
 */
#ifndef OTP_MAPS_H
#define OTP_MAPS_H

#include <cmd_otp.h>

#ifdef __cplusplus
extern "C" {
#endif
/*
 * obfu_rusk cannot be burnt in the U-Boot, but can be burned only in the teeos
 */
otp_val g_otp_fields_big_endian_num[] = {
	// { "tee_hash_flash_root_key", 256, LOCKABLE, 0x40, 0, "" },
	// { "ree_hash_flash_root_key", 256, LOCKABLE, 0x60, 0, "" },
	// { "tp_hash_flash_root_key", 256, LOCKABLE, 0x80, 0, "" },
	// { "obfu_mrk0", 128, LOCKABLE, 0xa0, 0, "" },
	// { "obfu_mrk1", 128, LOCKABLE, 0xb0, 0, "" },
	// { "obfu_mrk2", 128, LOCKABLE, 0xc0, 0, "" },
	// { "obfu_usk", 128, LOCKABLE, 0xd0, 0, "" },
	// { "scs_alg_sel", 1, LOCKABLE, 0x0, 0, "" },
	// { "rkp_deob_alg_sel", 1, LOCKABLE, 0x0, 2, "" },
	// { "wfi_wdg_rst_en", 1, LOCKABLE, 0x0, 5, "" },
	// { "dice_cdi_enable", 1, LOCKABLE, 0x0, 6, "" },
	// { "boot_backup_enable", 1, LOCKABLE, 0x0, 7, "" },
	// { "sm2_disable", 1, LOCKABLE, 0x3, 3, "" },
	// { "sm3_disable", 1, LOCKABLE, 0x3, 2, "" },
	// { "sm4_disable", 1, LOCKABLE, 0x3, 1, "" },
	// { "tee_dec_enable", 4, LOCKABLE, 0x6, 0, "" },
	// { "ree_dec_enable", 4, LOCKABLE, 0x6, 4, "" },
	// { "soc_tee_enable", 8, LOCKABLE, 0x12, 0, "" },
	// { "tp_verify_enable", 8, LOCKABLE, 0x15, 0, "" },
	// { "ree_verify_enable", 8, LOCKABLE, 0x16, 0, "" },
	// { "tee_verify_enable", 8, LOCKABLE, 0x17, 0, "" },
	// { "uart_halt_interval", 2, ONEWAY, 0x1e0, 2, "" },
	// { "sec_boot_dbg_lv", 2, ONEWAY, 0x1e1, 0, "" },
	// { "uart_selfboot_disable", 1, ONEWAY, 0x1e1, 4, "" },
	// { "sdio_selfboot_disable", 1, ONEWAY, 0x1e1, 5, "" },
	// { "usb_selfboot_disable", 1, ONEWAY, 0x1e1, 6, "" },
	// { "func_jtag_mode", 8, ONEWAY, 0x1e5, 0, "" },
	// { "dft_test_jtag_mode", 8, ONEWAY, 0x1e6, 0, "" },
	// { "tee_priv_dbg_mode", 2, ONEWAY, 0x1e7, 0, "" },
	// { "tee_priv_dbg_mode_ext", 2, ONEWAY, 0x1e8, 2, "" },
	// { "cw_crc_rd_disable", 8, ONEWAY, 0x1ed, 0, "" },
	// { "msid", 32, LOCKABLE, 0x30, 0, "" },
	// { "params_ver", 32, ONEWAY, 0x1cc, 0, "" },
	// { "ree_boot_ver", 32, ONEWAY, 0x1d0, 0, "" },
	// { "ree_app_ver", 32, ONEWAY, 0x1d4, 0, "" },
	// { "tp_key_ver", 4, ONEWAY, 0x1df, 0, "" },
	// { "tee_boot_ver", 16, ONEWAY, 0x1b0, 0, "" },
	// { "tee_os_ver", 64, ONEWAY, 0x1b8, 0, "" },
	// { "sec_storage_ver", 32, ONEWAY, 0x1c0, 0, "" },
	// { "nnie_model_ver", 32, ONEWAY, 0x1c4, 0, "" },
};

// char ree_user_data_buf[2560] = {};
// char tee_user_data_buf[1024] = {};

otp_raw g_otp_fields_raw[] = {
	// { "ree_user_data", 20480, ONEWAY, 0x600, 0, ree_user_data_buf },
	// { "tee_user_data",  8192, ONEWAY, 0x200, 0, tee_user_data_buf },
};

// rule divides the otp into different regions
// offset of otp field within the region must be aligned according to otp_corresponding_per_bit_locker
struct otp_rule g_locker_rule_list[] = {
	[0] = { .otp_range = { { 0, 0 }, { 3, 7 } },
		.locker_range = { { 0x1f0, 0 }, { 0x1f3, 7 } },
		.otp_corresponding_per_bit_locker = 1 },
	[1] = { .otp_range = { { 0x4, 0 }, { 0xf, 7 } },
		.locker_range = { { 0x1f4, 0 }, { 0x1f6, 7 } },
		.otp_corresponding_per_bit_locker = 4 },
	[2] = { .otp_range = { { 0x10, 0 }, { 0x2f, 7 } },
		.locker_range = { { 0x1f7, 0 }, { 0x1fa, 7 } },
		.otp_corresponding_per_bit_locker = 8 },
	[3] = { .otp_range = { { 0x30, 0 }, { 0x3f, 7 } },
		.locker_range = { { 0x1fb, 0 }, { 0x1fb, 3 } },
		.otp_corresponding_per_bit_locker = 32 },
	[4] = { .otp_range = { { 0x40, 0 }, { 0x9f, 7 } },
		.locker_range = { { 0x1fb, 4 }, { 0x1fc, 1 } },
		.otp_corresponding_per_bit_locker = 128 },
	[5] = { .otp_range = { { 0xa0, 0 }, { 0xef, 7 } },
		.locker_range = { { 0x1fc, 2 }, { 0x1fc, 6 } },
		.otp_corresponding_per_bit_locker = 128 },
	[6] = { .otp_range = { { 0xf0, 0 }, { 0x12f, 7 } },
		.locker_range = { { 0x1fc, 7 }, { 0x1fd, 2 } },
		.otp_corresponding_per_bit_locker = 128 },
	[7] = { .otp_range = { { 0x130, 0 }, { 0x18f, 7 } },
		.locker_range = { { 0x1fd, 3 }, { 0x1fe, 0 } },
		.otp_corresponding_per_bit_locker = 128 },
	[8] = { .otp_range = { { 0x190, 0 }, { 0x1af, 7 } },
		.locker_range = { { 0x1fe, 1 }, { 0x1fe, 2 } },
		.otp_corresponding_per_bit_locker = 1 },
};


#ifdef __cplusplus
}
#endif

#endif /* OTP_MAPS_H */
