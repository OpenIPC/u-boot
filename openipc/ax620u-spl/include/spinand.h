/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef __SPINAND_H__
#define __SPINAND_H__
#define UBOOT_FLASH_ADDR	0x100000
#define UBOOT_MAX_BLOCKS	16
void spinand_set_geometry(u32 flash_cfg);
int spinand_init(u32 hz, int buswidth);
int spinand_read(u32 addr, u32 len, void *buf);
#endif
