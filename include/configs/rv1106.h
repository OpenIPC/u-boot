/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Rockchip RV1106 / RV1103 cameras.
 *
 * Boot chain: mask ROM -> the ID block's DDR-init blob (rkbin, run from
 * SRAM; it leaves ATAGs at 0x1fe000) -> its SPL, loaded into DRAM at 0
 * and run there -> U-Boot, from the boot partition, at SYS_TEXT_BASE ->
 * Linux. DRAM starts at 0.
 */
#ifndef __RV1106_H
#define __RV1106_H

#include <linux/sizes.h>

#define CONFIG_SYS_SDRAM_BASE		0
/* board_init_f()'s stack and global data: above U-Boot, below baseaddr */
#define CONFIG_SYS_INIT_RAM_ADDR	0x00300000
#define CONFIG_SYS_INIT_RAM_SIZE	0x00100000
/* the kernel's mapping: keep the device tree and initrd inside 32 MiB */
#define CONFIG_SYS_BOOTMAPSZ		SZ_32M

#define CONFIG_SYS_NS16550_MEM32

/* System SRAM: the mask ROM's boot-source word is at +0x10 */
#define CONFIG_IRAM_BASE		0xff6c0000

/*
 * SPI NAND: the boot partition (OpenIPC's 768k(boot),256k(env),-(ubi))
 * holds the ID block in block 0 and its copy in block 1, for the mask ROM,
 * and u-boot.itb from block 2, which the SPL reads skipping bad blocks
 * (board/rockchip/rv11xx/spl_spi_nand.c)
 */
#define RV11XX_NAND_ITB			0x40000
#define RV11XX_NAND_BOOT_SIZE		0xc0000

/* The OpenIPC environment (include/configs/openipc-common.h) */
#define OPENIPC_BASEADDR		0x01000000
/* The vendor kernel's console is the FIQ debugger's, on UART2 */
#define OPENIPC_CONSOLE			"ttyFIQ0,115200"

#ifdef CONFIG_ENV_IS_IN_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE	1
/* Unused with SYS_NAND_SELF_INIT, but nand.c wants one */
#define CONFIG_SYS_NAND_BASE		0xffac0000
/* the SPI NAND's MTD name in the kernel */
#define OPENIPC_SFC			"spi-nand0"
#define OPENIPC_NAND_MTDPARTS		"768k(boot),256k(env),-(ubi)"
#define OPENIPC_NAND_UBI_MTD		2
#define OPENIPC_NAND_BOOT_SIZE		0xc0000
#else
/* the SPI NOR's MTD name in the kernel */
#define OPENIPC_SFC			"sfc_nor"
#endif

#include "openipc-common.h"

#endif
