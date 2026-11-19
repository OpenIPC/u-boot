/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef __BOOT_H__
#define __BOOT_H__
#define UBOOT_HEADER_ADDR	0x40000000
#define UBOOT_ENTRY		(UBOOT_HEADER_ADDR + IMG_HEADER_SIZE)
#define SPL_HEADER_ADDR		0x400	/* the boot ROM's copy of our header */
int boot_from_flash(int flash_type);
#endif
