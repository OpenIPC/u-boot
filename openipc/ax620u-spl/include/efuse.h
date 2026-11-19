/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef __EFUSE_H__
#define __EFUSE_H__
int efuse_ready(void);
int efuse_read(int blk, u32 *val);
#endif
