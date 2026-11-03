/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * The memory map of the Novatek die U-Boot is built for: the drivers and
 * the board code include this rather than a die's own header.
 */

#ifndef __NVT_HARDWARE_H
#define __NVT_HARDWARE_H

#if defined(CONFIG_NVT_NA511XX)
#include <asm/arch/na511xx.h>
#elif defined(CONFIG_NVT_NA51068)
#include <asm/arch/na51068.h>
#else
#include <asm/arch/na51089.h>
#endif

#ifndef __ASSEMBLY__
/* The device tree the loader left (SHMINFO), or NULL */
const void *nvt_loader_fdt(void);
#endif

#endif
