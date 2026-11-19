/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef __CMN_H__
#define __CMN_H__

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed int s32;
typedef unsigned long long u64;
typedef unsigned int size_t;

#define NULL ((void *)0)
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

/* Out of line, as in the vendor code: one bus access per call. */
u32 readl(u32 addr);
void writel(u32 val, u32 addr);
/* Replace bits [shift + width - 1 : shift] of the register at addr. */
void reg_field_write(u32 addr, u32 width, u32 shift, u32 val);

void *memset(void *s, int c, size_t n);
size_t strlen(const char *s);
void info(const char *fmt, ...);

/* Boot ROM / first stage shared header (flash 0, copy at 0x400). */
struct boot_header {
	u32 check_sum;		/* word sum of bytes 8..0x3f8 */
	u32 magic;		/* IMG_HEADER_MAGIC */
	u32 capability;
	u32 img_size;
	u32 fw_size;
	u32 img_check_sum;
	u32 fw_check_sum;
	u32 flash_cfg;		/* NAND geometry, see spinand.c */
	u32 backup_addr[3];
};

#define IMG_HEADER_MAGIC	0x55543322
#define IMG_HEADER_SIZE		0x400
#define CAP_NAND_QUAD		(1 << 2)	/* capability, flash type 0/2 */
#define CAP_NOR_QUAD		(1 << 3)
#define CAP_ENCRYPTED		(1 << 8)
#define CAP_RSA_3072		(1 << 10)

#endif
