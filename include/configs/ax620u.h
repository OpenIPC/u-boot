/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Axera AX620U cameras.
 *
 * Boot chain: boot ROM -> first stage (flash 0 and its backup at 0x20000:
 * DDR, PLLs; signed; openipc/ax620u-spl, or the cameras' own changed to
 * this layout) -> U-Boot (0x40000, behind the 1 KiB Axera image header;
 * loaded at 0x40000000, entered at 0x40000400 = SYS_TEXT_BASE) -> Linux
 * from UBI. Both are in the boot partition of OpenIPC's NAND layout.
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
/*
 * OpenIPC's NAND layout. The first stage has the boot partition's first two
 * blocks, at places the boot ROM fixes, and loads U-Boot from 0x40000. The
 * image covers both from flash 0 (openipc/ax620u-image.py), and ubwrite
 * writes it so (OPENIPC_NAND_SPL_SIZE)
 */
#define OPENIPC_NAND_MTDPARTS		"768k(boot),256k(env),-(ubi)"
#define OPENIPC_NAND_SPL_SIZE		0x40000
#define OPENIPC_NAND_UBI_MTD		2
#define OPENIPC_NAND_BOOT_SIZE		0xc0000

#include "openipc-common.h"

#endif
