/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Common header file for U-Boot
 *
 * This file still includes quite a few headers that should be included
 * individually as needed. Patches to remove things are welcome.
 *
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef __COMMON_H_
#define __COMMON_H_	1

#ifndef __ASSEMBLY__		/* put C only stuff in this section */
#include <config.h>
#include <errno.h>
#include <time.h>
#include <linux/types.h>
#include <linux/printk.h>
#include <linux/string.h>
#include <stdarg.h>
#include <stdio.h>
#include <linux/kernel.h>
#include <asm/u-boot.h> /* boot information for Linux kernel */
#include <vsprintf.h>
#endif	/* __ASSEMBLY__ */

/* Pull in stuff for the build system */
#ifdef DO_DEPS_ONLY
# include <env_internal.h>
#endif

#define BOOT_MEDIA_UNKNOWN        (0)
#define BOOT_MEDIA_UFS            (1)
#define BOOT_MEDIA_NAND           (2)
#define BOOT_MEDIA_SPIFLASH       (3)
#define BOOT_MEDIA_EMMC           (4)

#ifndef __ASSEMBLY__
void add_shutdown(void (*shutdown)(void));
void do_shutdown(void);

/* get uboot start media. */
int get_boot_media(void);
unsigned int get_ddr_size(void);
#endif

#endif	/* __COMMON_H_ */
