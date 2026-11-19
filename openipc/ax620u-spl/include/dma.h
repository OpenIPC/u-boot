/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef __DMA_H__
#define __DMA_H__
#define DMA_HS_NONE	0
#define DMA_HS_DST	1	/* hardware handshake on the destination */
#define DMA_HS_SRC	2
void dma_init(void);
int dma_xfer_hs(int ch, u32 src, u32 dst, u32 len, u32 width_shift,
		int burst_shift_unused, int burst, int hs, int hs_if);
int dma_wait_done(int ch);
#endif
