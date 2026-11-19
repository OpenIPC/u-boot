/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Axera AX620U cameras.
 *
 * Boot chain: boot ROM -> first stage (spl partition, flash 0, 1 MiB: DDR,
 * PLLs; signed; openipc/ax620u-spl, or the cameras' own) -> U-Boot (boot partition, 0x100000,
 * behind the 1 KiB Axera image header; loaded at 0x40000000, entered at
 * 0x40000400 = SYS_TEXT_BASE) -> Linux from UBI.
 */
#ifndef __AX620U_H
#define __AX620U_H

#include <linux/sizes.h>

#define CONFIG_SYS_SDRAM_BASE		0x40000000
/* board_init_f()'s stack and global data: above the image the SPL
 * loaded, below baseaddr */
#define CONFIG_SYS_INIT_RAM_ADDR	0x40f00000
#define CONFIG_SYS_INIT_RAM_SIZE	0x100000

/*
 * The kernel gets mem=${osmem} (87 MiB) of the 256: keep the device tree
 * and an initrd where it maps them, not at the top of the RAM
 */
#define CONFIG_SYS_BOOTMAPSZ		SZ_32M

#ifdef CONFIG_ENV_IS_IN_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE	1
/* Unused with SYS_NAND_SELF_INIT, but nand.c wants one */
#define CONFIG_SYS_NAND_BASE		0x08000000
#endif

/* The OpenIPC environment (include/configs/openipc-common.h) */
#define OPENIPC_BASEADDR		0x42800000
/* The rest of the 256 MiB is the vendor media drivers' (CMM) */
#define OPENIPC_OSMEM			"87M"
/* The SPI NAND's MTD name in the AX620U kernels */
#define OPENIPC_SFC			"spi4.0_nand"
/* The first stage owns the first 1 MiB and loads U-Boot from 0x100000 */
#define OPENIPC_NAND_MTDPARTS		"1024k(spl),1536k(boot),512k(env),-(ubi)"
#define OPENIPC_NAND_UBI_MTD		3
#define OPENIPC_NAND_BOOT_SIZE		0x180000

#include "openipc-common.h"

#endif
