/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include <asm/io.h>
#include <config.h>

#define PERI_CRG_GZIP 0x2b80
#define GZIP_CLKEN (1<<4)
#define GZIP_SRST  (1<<0)

#ifndef GZIP_REG_BASE
#define GZIP_REG_BASE		0x170F0000
#endif

#define HW_DEC_REG_BASE_ADDR  (GZIP_REG_BASE)

static void decompress_reset(void)
{
	unsigned int regval;

	regval = readl(CRG_REG_BASE + PERI_CRG_GZIP);
	regval |= GZIP_SRST;
	writel(regval, CRG_REG_BASE + PERI_CRG_GZIP);
}

static void disable_decompress_clock(void)
{
	unsigned int regval;

	regval = readl(CRG_REG_BASE + PERI_CRG_GZIP);
	regval &= ~GZIP_CLKEN;
	writel(regval, CRG_REG_BASE + PERI_CRG_GZIP);
}

static void enable_decompress_clock(void)
{
	unsigned int regval;

	regval = readl(CRG_REG_BASE + PERI_CRG_GZIP);
	regval |= GZIP_CLKEN;
	writel(regval, CRG_REG_BASE + PERI_CRG_GZIP);
}

static void decompress_unreset(void)
{
	unsigned int regval;

	regval = readl(CRG_REG_BASE + PERI_CRG_GZIP);
	regval &= ~GZIP_SRST;
	writel(regval, CRG_REG_BASE + PERI_CRG_GZIP);
}
