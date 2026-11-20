/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef __SPINAND_H__
#define __SPINAND_H__
/*
 * U-Boot: OpenIPC's boot partition (768k(boot),256k(env),-(ubi)) holds
 * the first stage in its first two blocks and U-Boot from 0x40000 to the
 * environment at 0xc0000. The stock first stage loads it from 0x100000,
 * the cameras' own layout. The area is bounded by address; the map holds
 * its blocks at the smallest size the geometry word gives (64 KiB, also
 * what a header of zeros gives).
 */
#define UBOOT_FLASH_ADDR	0x40000
#define UBOOT_FLASH_END		0xc0000
#define UBOOT_MAX_BLOCKS	((UBOOT_FLASH_END - UBOOT_FLASH_ADDR) >> 16)
void spinand_set_geometry(u32 flash_cfg);
int spinand_init(u32 hz, int buswidth);
int spinand_read(u32 addr, u32 len, void *buf);
#endif
