/* SPDX-License-Identifier: 0BSD */
/*
 * U-Boot's environment for xz-embedded, in place of the upstream userspace
 * xz_config.h: single-call decoding, CRC32 and CRC64 computed here, no BCJ
 * filters. xz_private.h includes this when __UBOOT__ is defined.
 */

#ifndef XZ_CONFIG_H
#define XZ_CONFIG_H

#define XZ_DEC_SINGLE
#define XZ_USE_CRC64
#define XZ_INTERNAL_CRC32 1
#define XZ_INTERNAL_CRC64 1

#include <malloc.h>
#include <linux/compiler.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/xz.h>
#include <asm/unaligned.h>

#define kmalloc(size, flags)	malloc(size)
#define kfree(ptr)		free(ptr)
#define vmalloc(size)		malloc(size)
#define vfree(ptr)		free(ptr)

#define memeq(a, b, size)	(memcmp(a, b, size) == 0)
#define memzero(buf, size)	memset(buf, 0, size)

#define get_le32		get_unaligned_le32
#define get_be32		get_unaligned_be32

#endif
