/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Allwinner camera SoCs with the OpenIPC environment: V3s, V831/V833,
 * V851s
 */
#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/sunxi-common.h>

#ifdef CONFIG_MACH_SUN8I_V831
/*
 * The nonsec switch finds the GIC through CBAR otherwise; name it, as
 * on the arndale: QEMU's Cortex-A7 reads CBAR as 0
 */
#define CONFIG_ARM_GIC_BASE_ADDRESS	SUNXI_GIC400_BASE

/* DRAM in the package: 64 MiB of DDR2 (V831) or 128 MiB of DDR3 (V833) */
#if CONFIG_DRAM_SUN8I_V831_TYPE == 3
#define PHYS_SDRAM_1_SIZE		0x08000000
#else
#define PHYS_SDRAM_1_SIZE		0x04000000
#endif
#endif

#ifdef CONFIG_MACH_SUN8I_V851S
#define CONFIG_ARM_GIC_BASE_ADDRESS SUNXI_GIC400_BASE
#endif

#if defined(CONFIG_MACH_SUN8I_V3S) || defined(CONFIG_MACH_SUN8I_V851S)
/* 64 MiB of DDR2 in the package */
#define PHYS_SDRAM_1_SIZE		0x04000000
#endif

/* The OpenIPC environment instead of sunxi-common's distro one */
#ifdef CONFIG_OPENIPC
#undef CONFIG_EXTRA_ENV_SETTINGS
#undef CONFIG_BOOTCOMMAND
#include "openipc-common.h"
#endif

#endif /* __CONFIG_H */
