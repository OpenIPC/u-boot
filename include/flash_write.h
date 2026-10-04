/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#ifndef __FLASH_WRITE_H
#define __FLASH_WRITE_H

#include "compiler.h"

#ifndef UPDATE_MEDIUM_SPINOR
#define UPDATE_MEDIUM_SPINOR 0
#endif
#ifndef UPDATE_MEDIUM_NAND
#define UPDATE_MEDIUM_NAND   1
#endif
#ifndef UPDATE_MEDIUM_EMMC
#define UPDATE_MEDIUM_EMMC   2
#endif

#ifndef EMMC_BLOCK_SHIFT
#define EMMC_BLOCK_SHIFT	9
#endif

#define	BOOT_MEDIUM_LEN		20

struct flash_interface {
	char name[BOOT_MEDIUM_LEN];
	int (*init)(void);
	int (*erase)(unsigned long offset, unsigned long len);
	int (*write)(unsigned long offset, unsigned long len, unsigned char *buf);
	int (*write_yaffs)(unsigned long offset, unsigned long len, unsigned char *buf);
	int (*write_ext4)(unsigned long offset, unsigned long len, unsigned char *buf, char is_sparse);
};

int do_flash_init(void);
int do_flash_write(unsigned long offset, uint8_t *src_addr, unsigned int size);
int do_flash_erase(unsigned long offset, size_t size);
#endif /* __FLASH_WRITE_H */
