/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
 */
#include <td_type.h>
#include <time_stamp_index.h>
#include <common.h>
#include "../drivers/otp/otp.h"

static td_u64 get_time_stamp(td_u32 index)
{
	td_u64 syscnt_save_addr =
		SYSCNT_AREA_START_ADDR + BYTE_NUMS * 2 + BYTE_NUMS * 2 * index;
	return ((td_u64)reg_get(syscnt_save_addr + BYTE_NUMS) << BITS_PER_LONG) +
	       reg_get(syscnt_save_addr);
}

void print_time_stamp(char *name, td_u32 index)
{
	if (get_time_stamp(index)) {
		log_serial_puts((const s8 *)"\n");
		log_serial_puts((const s8 *)name);
		log_serial_puts((const s8 *)": ");
		serial_put_hex(get_time_stamp(index));
		log_serial_puts((const s8 *)"\n");
	}
}

void print_boot_time(void)
{
	if (is_sec_dbg_enable() == AUTH_FAILURE)
		return;

	print_time_stamp("BOOTROM_START_TIME", BOOTROM_START_TIME);
	print_time_stamp("ENTER_SECURE_BOOT_TIME", ENTER_SECURE_BOOT_TIME);
	print_time_stamp("EMMC_INIT_END_TIME", EMMC_INIT_END_TIME);
	print_time_stamp("USB_TIMEOUT_END_TIME", USB_TIMEOUT_END_TIME);
	print_time_stamp("SDIO_TIMEOUT_END_TIME", SDIO_TIMEOUT_END_TIME);
	print_time_stamp("GET_DATA_CHANNEL_TYPE_END_TIME",
			 GET_DATA_CHANNEL_TYPE_END_TIME);
	print_time_stamp("LOAD_SEC_IMAGE_END_TIME", LOAD_SEC_IMAGE_END_TIME);
	print_time_stamp("HANDLE_ROOT_PUBLIC_KEY_END_TIME",
			 HANDLE_ROOT_PUBLIC_KEY_END_TIME);
	print_time_stamp("HANDLE_GSL_AREA_END_TIME", HANDLE_GSL_AREA_END_TIME);
	print_time_stamp("ENTRY_GSL_TIME", ENTRY_GSL_TIME);
	print_time_stamp("GSL_START_TIME", GSL_START_TIME);
	print_time_stamp("ENTRY_UBOOT_TIME", ENTRY_UBOOT_TIME);

	return;
}
