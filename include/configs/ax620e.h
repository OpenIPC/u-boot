/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Axera AX620E cameras.
 *
 * Boot chain: boot ROM -> vendor first stage (flash 0, 64 KiB: DDR; signed,
 * kept as shipped) -> U-Boot (flash 0x10000, behind the 1 KiB Axera image
 * header, packed; unpacked to 0x5C000400, which branches to SYS_TEXT_BASE)
 * -> Linux from the OpenIPC NOR layout.
 */
#ifndef __AX620E_H
#define __AX620E_H

#include <linux/sizes.h>

#define CONFIG_SYS_SDRAM_BASE		0x40000000
/* board_init_f()'s stack and global data, in DDR the first stage leaves
 * alone */
#define CONFIG_SYS_INIT_RAM_ADDR	0x40f00000
#define CONFIG_SYS_INIT_RAM_SIZE	0x100000

/*
 * The kernel gets mem=${osmem} (120 MiB) of the 256: keep the device tree
 * and an initrd where it maps them, not at the top of the RAM, which is
 * the media pool's
 */
#define CONFIG_SYS_BOOTMAPSZ		SZ_64M

/* The OpenIPC environment (include/configs/openipc-common.h) */
#define OPENIPC_BASEADDR		0x45000000
/* The rest of the 256 MiB is the vendor media drivers' (CMM) */
#define OPENIPC_OSMEM			"120M"
/* The SPI NOR's MTD name in the AX620E kernel (its devicetree's label) */
#define OPENIPC_SFC			"spi_nor.0"

#include "openipc-common.h"

#endif
