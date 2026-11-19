// SPDX-License-Identifier: GPL-2.0+
/* eFuse: two controllers, blocks 0-63 and 64-95. */
#include "cmn.h"
#include "efuse.h"

#define EFUSE0_BASE	0x02200000
#define EFUSE1_BASE	0x02200800
#define EFUSE_STATUS	0x14
#define EFUSE_CLR	0x28
#define EFUSE_KEY	0x2c
#define EFUSE_ADDR	0x34
#define EFUSE_START	0x38
#define EFUSE_DATA	0x40
#define EFUSE_KEY_READ	0x6d633230

static int efuse_busy(u32 base)
{
	return (readl(base + EFUSE_STATUS) & 4) ? -2 : 0;
}

int efuse_ready(void)
{
	int ret = efuse_busy(EFUSE0_BASE);

	if (!ret)
		ret = efuse_busy(EFUSE1_BASE);
	return ret;
}

static int efuse_ctl_read(u32 base, int blk, u32 *val)
{
	int ret = 0;
	int i;

	writel(EFUSE_KEY_READ, base + EFUSE_KEY);
	writel(blk << 3 | 1, base + EFUSE_ADDR);
	writel(1, base + EFUSE_START);
	writel(0, base + EFUSE_START);
	for (i = 0; i < 10000 && !(readl(base + EFUSE_STATUS) & 1); i++)
		;
	if (i >= 10000)
		ret = -4;
	*val = readl(base + EFUSE_DATA + blk * 4);
	writel(0xf, base + EFUSE_CLR);
	writel(0, base + EFUSE_KEY);
	writel(0, base + EFUSE_ADDR);
	return ret;
}

int efuse_read(int blk, u32 *val)
{
	if (blk < 64)
		return efuse_ctl_read(EFUSE0_BASE, blk, val);
	if (blk < 96)
		return efuse_ctl_read(EFUSE1_BASE, blk - 64, val);
	return -1;
}
