/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
 */
#ifndef __TIME_STAMP_INDEX__
#define __TIME_STAMP_INDEX__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

enum {
	BOOTROM_START_TIME = 0x0,
	ENTER_SECURE_BOOT_TIME,
	EMMC_INIT_END_TIME,
	USB_TIMEOUT_END_TIME,
	SDIO_TIMEOUT_END_TIME,
	GET_DATA_CHANNEL_TYPE_END_TIME,
	LOAD_SEC_IMAGE_END_TIME,
	HANDLE_ROOT_PUBLIC_KEY_END_TIME,
	HANDLE_GSL_AREA_END_TIME,
	ENTRY_GSL_TIME,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __TIME_STAMP_INDEX__ */
