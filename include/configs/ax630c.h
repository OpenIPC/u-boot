/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Axera AX630C boards with eMMC and an SD slot (Sipeed MaixCAM2).
 *
 * Boot chain, for now: boot ROM -> the board's own first stage, ATF and
 * U-Boot from the eMMC -> this U-Boot, loaded from the card's first
 * partition and started with go (or the OpenIPC first stage, later) ->
 * Linux from the card (FAT p1 Image and devicetree, ext4 p2 root), else
 * from the eMMC in OpenIPC's layout.
 *
 * DRAM: 0.5 to 4 GiB at 0x40000000, sized from the board id the first
 * stage leaves in misc_info (arch/arm/mach-axera/ax620e/soc.c). The
 * kernel gets mem=${osmem}, a quarter of it by default (misc_init_r()
 * sets osmem when the environment has none); the rest is the vendor
 * media drivers' CMM pool.
 */
#ifndef __AX630C_H
#define __AX630C_H

#include <linux/sizes.h>

#define CONFIG_SYS_SDRAM_BASE		0x40000000
/* board_init_f()'s stack and global data, in DDR nothing else holds then:
 * past the board's ATF (0x40040000, 256 KiB) and below its OP-TEE
 * (0x44200000, 32 MiB) */
#define CONFIG_SYS_INIT_RAM_ADDR	0x40f00000
#define CONFIG_SYS_INIT_RAM_SIZE	0x100000

/* The device tree and an initrd stay in the first 64 MiB, inside any
 * mem= the kernel gets (at least 128 MiB) and below OP-TEE's area */
#define CONFIG_SYS_BOOTMAPSZ		SZ_64M

/* The OpenIPC environment (include/configs/openipc-common.h) */
/* Where images are loaded: past OP-TEE's area */
#define OPENIPC_BASEADDR		0x48000000
/* The arm64 Image (it runs where it is loaded, 2 MiB aligned) and its
 * device tree, below OP-TEE's area and past the ATF's */
#define OPENIPC_KERNEL_ADDR		0x40200000
#define OPENIPC_FDT_ADDR		0x40100000
#define OPENIPC_FDTFILE			"ax630c-maixcam2.dtb"

/*
 * MMC: the eMMC is mmc 0, the card mmc 1, in U-Boot as in Linux. The
 * card is the firmware's SD image: FAT p1 with Image and the device
 * tree, ext4 p2 the root. The eMMC has OpenIPC's layout from 0, no
 * partition table (the boot ROM reads the first stage from 0): boot
 * (the first stage and U-Boot), env (CONFIG_ENV_OFFSET), kernel (a
 * uImage or FIT, bootm) and the ext4 root, given to the kernel as
 * blkdevparts=.
 */
#define OPENIPC_EMMC_DEV		0
#define OPENIPC_SD_DEV			1
#define OPENIPC_MMC_PARTS		"768k(boot),256k(env),16384k(kernel),-(rootfs)"
/* The kernel partition, in 512-byte blocks, and in bytes */
#define OPENIPC_MMC_KERNEL_BLK		0x800
#define OPENIPC_MMC_KERNEL_BLKS		0x8000
#define OPENIPC_MMC_KERNEL_SIZE		0x1000000
/* The root partition: its number in that list, its first block */
#define OPENIPC_MMC_ROOT_PART		4
#define OPENIPC_MMC_ROOT_BLK		0x8800

#include "openipc-common.h"

#endif
