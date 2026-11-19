// SPDX-License-Identifier: GPL-2.0+
/* Axera system DMA at 0x01920000: polled single-block transfers. */
#include "cmn.h"
#include "dma.h"

#define DMA_BASE		0x01920000
#define DMA_CTRL		(DMA_BASE + 0x10)
#define DMA_CH_EN		(DMA_BASE + 0x18)
#define DMA_RESET		(DMA_BASE + 0x58)
#define DMA_CH(ch, off)		(DMA_BASE + (ch) * 0x100 + (off))
#define CH_SRC			0x00
#define CH_SRC_HI		0x04
#define CH_DST			0x08
#define CH_DST_HI		0x0c
#define CH_BLOCK_TS		0x10
#define CH_CTL			0x18
#define CH_CTL_HI		0x1c
#define CH_CFG			0x20
#define CH_CFG_HI		0x24
#define CH_INT_EN		0x80
#define CH_INT_STATUS		0x88
#define CH_INT_SIG_EN		0x90
#define CH_INT_CLR		0x98
#define CH_INT_DONE		0x2
#define CH_INT_ERR		0x3f7fe0

static u32 dma_inited;

static void dma_reset(void)
{
	writel(1, DMA_RESET);
	while (readl(DMA_RESET) & 1)
		;
}

static void dma_enable(void)
{
	writel(readl(DMA_CTRL) | 1, DMA_CTRL);
}

static void dma_ch_enable(int ch)
{
	u32 bit = 1 << (ch - 1);

	writel(bit << 8 | bit | readl(DMA_CH_EN), DMA_CH_EN);
}

static void dma_ch_disable(int ch)
{
	u32 bit = 1 << (ch - 1);

	writel(bit << 8 | (~bit & readl(DMA_CH_EN)), DMA_CH_EN);
}

static u32 dma_ch_busy(int ch)
{
	return readl(DMA_CH_EN) & (1 << (ch - 1));
}

static void dma_irq_disable(void)
{
	writel(readl(DMA_CTRL) & 0xfffffffd, DMA_CTRL);
}

void dma_init(void)
{
	int ch;

	if (dma_inited)
		return;
	writel(readl(0x01900098) | 1, 0x01900098);	/* DMA clock */
	dma_reset();
	dma_enable();
	dma_irq_disable();
	for (ch = 1; ch < 3; ch++) {
		dma_ch_disable(ch);
		writel(0xffffffff, DMA_CH(ch, CH_INT_CLR));
		writel(0, DMA_CH(ch, CH_INT_SIG_EN));
		writel(CH_INT_DONE | CH_INT_ERR, DMA_CH(ch, CH_INT_EN));
	}
	dma_inited = 1;
}

static int dma_ch_setup(int ch, u32 src, u32 dst, u32 len, u32 width_shift,
			int src_msize, int dst_msize, int hs, int hs_if)
{
	u32 ctl, cfg_hi_if = 0, cfg_hi;

	writel(src, DMA_CH(ch, CH_SRC));
	writel(0, DMA_CH(ch, CH_SRC_HI));
	writel(dst, DMA_CH(ch, CH_DST));
	writel(0, DMA_CH(ch, CH_DST_HI));
	len >>= width_shift;
	if (len > 0x400000)
		return -1;
	writel(len - 1, DMA_CH(ch, CH_BLOCK_TS));
	ctl = width_shift << 8 | dst_msize << 18 | dst_msize << 14 | src_msize << 11;
	if (hs == DMA_HS_DST) {
		ctl |= 0x40;
		cfg_hi_if = hs_if << 11;
		cfg_hi = 1;
	} else if (hs == DMA_HS_SRC) {
		ctl |= 0x10;
		cfg_hi_if = hs_if << 4;
		cfg_hi = 2;
	} else {
		cfg_hi = 0;
	}
	writel(ctl, DMA_CH(ch, CH_CTL));
	writel(0, DMA_CH(ch, CH_CTL_HI));
	writel(cfg_hi_if, DMA_CH(ch, CH_CFG));
	writel(cfg_hi, DMA_CH(ch, CH_CFG_HI));
	return 0;
}

int dma_xfer_hs(int ch, u32 src, u32 dst, u32 len, u32 width_shift,
		int src_msize, int dst_msize, int hs, int hs_if)
{
	if (dma_ch_busy(ch))
		return -1;
	if (dma_ch_setup(ch, src, dst, len, width_shift, src_msize, dst_msize, hs, hs_if) < 0)
		return -1;
	dma_ch_enable(ch);
	return 0;
}

int dma_wait_done(int ch)
{
	u32 st;

	do {
		st = readl(DMA_CH(ch, CH_INT_STATUS));
	} while (!(st & (CH_INT_DONE | CH_INT_ERR)));
	writel(st, DMA_CH(ch, CH_INT_CLR));
	if (!(st & CH_INT_ERR)) {
		dma_ch_disable(ch);
		return 0;
	}
	dma_ch_disable(ch);
	return -1;
}
