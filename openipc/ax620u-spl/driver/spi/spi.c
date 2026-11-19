// SPDX-License-Identifier: GPL-2.0+
/*
 * DesignWare SSI flash controller at 0x08000000 (standard and quad SPI),
 * PIO for short transfers, the system DMA for 32 or more frames.
 */
#include "cmn.h"
#include "dma.h"
#include "spi.h"

#define SPI_BASE	0x08000000
#define SSI_CTRLR0	0x00
#define SSI_CTRLR1	0x04
#define SSI_SSIENR	0x08
#define SSI_BAUDR	0x14
#define SSI_TXFTLR	0x18
#define SSI_RXFTLR	0x1c
#define SSI_TXFLR	0x20
#define SSI_RXFLR	0x24
#define SSI_SR		0x28
#define SSI_IMR		0x2c
#define SSI_DMACR	0x4c
#define SSI_DMATDLR	0x50
#define SSI_DMARDLR	0x54
#define SSI_DR		0x60
#define SSI_RX_SAMPLE	0xf0
#define SSI_SPI_CTRLR0	0xf4
#define SR_BUSY		1
#define SR_TFE		4
#define FIFO_BYTES	0x40
#define SPI_REF_HZ	250000000
#define DMA_MIN_FRAMES	0x20
#define DMA_CH		1

#define XFER_CS_ASSERT	1
#define XFER_CS_RELEASE	2

struct pio_xfer {
	u32 len;	/* frames left */
	u8 *tx;
	u8 *rx;
	u32 bits;
};

static u32 ssi_read(u32 off)
{
	return readl(SPI_BASE + off);
}

static void ssi_write(u32 off, u32 val)
{
	writel(val, SPI_BASE + off);
}

static void spi_pinmux(void)
{
	writel(0x10013, 0x1002000c);
	writel(0x10093, 0x10020018);
	writel(0x10093, 0x10020024);
	writel(0x10093, 0x10020030);
	writel(0x10093, 0x1002003c);
	writel(0x60093, 0x10020048);
	writel(0x60093, 0x10020054);
	writel(readl(0x10010008) & 0xffffffdf, 0x10010008);
	writel(readl(0x10010004) | 0x20, 0x10010004);	/* CS as GPIO output */
	writel(readl(0x10010000) | 0x20, 0x10010000);	/* high */
}

static void spi_enable(int on)
{
	ssi_write(SSI_SSIENR, on != 0);
}

static void tx_fill(struct pio_xfer *x)
{
	u32 bits = x->bits;
	u32 left = x->len;
	u32 room = FIFO_BYTES / (bits >> 3) - ssi_read(SSI_TXFLR);
	u32 n = left <= room ? left : room;

	x->len = left - n;
	while (n) {
		if (x->bits == 8)
			ssi_write(SSI_DR, *x->tx);
		else
			ssi_write(SSI_DR, *(u32 *)x->tx);
		x->tx += bits >> 3;
		n--;
	}
}

static void rx_drain(struct pio_xfer *x)
{
	u32 bits = x->bits;
	u32 left = x->len;
	u32 n = ssi_read(SSI_RXFLR);

	if (left <= n)
		n = left;
	x->len = left - n;
	while (n) {
		if (x->bits == 8)
			*x->rx = ssi_read(SSI_DR);
		else
			*(u32 *)x->rx = ssi_read(SSI_DR);
		x->rx += bits >> 3;
		n--;
	}
}

static int pio_xfer(struct pio_xfer *x)
{
	if (x->rx)
		ssi_write(SSI_DR, 0xffffffff);	/* start receive-only */
	do {
		if (x->tx) {
			tx_fill(x);
		} else {
			if (!x->rx)
				return -1;
			rx_drain(x);
		}
	} while (x->len);
	return 0;
}

static void spi_cs(int high)
{
	if (!high)
		writel(readl(0x10010000) & 0xffffffdf, 0x10010000);
	else
		writel(readl(0x10010000) | 0x20, 0x10010000);
}

static int spi_xfer(u32 bitlen, void *tx, void *rx, u32 flags, u32 bits, int quad)
{
	int ret = 0;
	u32 ctrl0, spi_ctrl0 = 0, txftl = 0, rxftl = 0;
	int timeout = 10000;
	int tmod = 0;
	u32 width = 0;
	u32 nbytes, frames, sr;
	struct pio_xfer x;

	if (bits == 32)
		width = 2;
	if ((bitlen & 7) || (bits & 7) || bitlen % bits)
		return -1;
	nbytes = bitlen >> 3;
	frames = nbytes / (bits >> 3);
	if (flags & XFER_CS_ASSERT)
		spi_cs(0);
	ctrl0 = bits - 1;
	if (quad) {
		ctrl0 |= 0x800000;		/* SPI_FRF: quad */
		spi_ctrl0 = 0x40000000;
		rxftl = 0x21;
		if (tx) {
			spi_ctrl0 = bits == 32 ? 0x40000022 : 0x4000000a;
			txftl = 0x10000;
		}
	}
	if (tx)
		tmod = 1;		/* transmit only */
	else if (rx)
		tmod = 2;		/* receive only */
	ctrl0 = tmod << 10 | (ctrl0 & 0xfffff3ff);
	spi_enable(0);
	if (ctrl0 != ssi_read(SSI_CTRLR0))
		ssi_write(SSI_CTRLR0, ctrl0);
	if (quad && ssi_read(SSI_SPI_CTRLR0) != spi_ctrl0)
		ssi_write(SSI_SPI_CTRLR0, spi_ctrl0);
	ssi_write(SSI_TXFTLR, txftl);
	ssi_write(SSI_RXFTLR, rxftl);
	if (rx)
		ssi_write(SSI_CTRLR1, frames - 1);
	spi_enable(1);
	if (frames < DMA_MIN_FRAMES) {
		x.len = frames;
		x.tx = tx;
		x.rx = rx;
		x.bits = bits;
		ret = pio_xfer(&x);
	} else {
		if (!tx) {
			dma_xfer_hs(DMA_CH, SPI_BASE + SSI_DR, (u32)rx, nbytes, width,
				    width, 4, DMA_HS_SRC, 1);
			ssi_write(SSI_DMARDLR, DMA_MIN_FRAMES - 1);
			ssi_write(SSI_DMACR, 1);
			ssi_write(SSI_DR, 0xffffffff);
		} else {
			dma_xfer_hs(DMA_CH, (u32)tx, SPI_BASE + SSI_DR, nbytes, width,
				    width, 4, DMA_HS_DST, 0);
			ssi_write(SSI_DMATDLR, DMA_MIN_FRAMES);
			ssi_write(SSI_DMACR, 2);
		}
		ret = dma_wait_done(DMA_CH);
		ssi_write(SSI_DMACR, 0);
	}
	do {
		do {
			sr = ssi_read(SSI_SR);
		} while (!(sr & SR_TFE));
	} while ((sr & SR_BUSY) && timeout-- != 0);
	if (flags & XFER_CS_RELEASE)
		spi_cs(1);
	return ret;
}

int spi_mem_adjust_op_size(struct spi_mem_op *op)
{
	if (op->data.dir == SPI_MEM_DATA_IN) {
		if (op->data.nbytes >= 0x8000)
			op->data.nbytes = 0x8000;
	} else {
		if (op->data.nbytes >= 0x8000)
			op->data.nbytes = 0x8000;
	}
	return op->data.nbytes ? 0 : -1;
}

int spi_mem_exec_op(struct spi_mem_op *op)
{
	u8 *tx = NULL, *rx = NULL;
	int n = op->addr.nbytes + op->dummy.nbytes;
	u8 buf[(n + 8) & ~7];
	int pos = 1, i, ret;
	u32 flags;

	if (op->data.nbytes) {
		if (op->data.dir == SPI_MEM_DATA_IN)
			rx = op->data.buf;
		else
			tx = op->data.buf;
	}
	buf[0] = op->cmd.opcode;
	if (op->addr.nbytes) {
		for (i = 0; i < op->addr.nbytes; i++)
			buf[i + 1] = op->addr.val >> ((op->addr.nbytes - i - 1) * 8);
		pos = op->addr.nbytes + 1;
	}
	if (op->dummy.nbytes)
		for (i = 0; i < op->dummy.nbytes; i++)
			buf[pos + i] = 0xff;
	flags = XFER_CS_ASSERT;
	if (!tx && !rx)
		flags = XFER_CS_ASSERT | XFER_CS_RELEASE;
	ret = spi_xfer((n + 1) * 8, buf, NULL, flags, 8, 0);
	if (ret)
		return ret;
	if (tx || rx) {
		ret = spi_xfer(op->data.nbytes << 3, tx, rx, XFER_CS_RELEASE, 8,
			       op->data.buswidth == 4);
		if (ret)
			return ret;
	}
	return 0;
}

void spi_init(u32 hz)
{
	u32 div = (SPI_REF_HZ / hz + 1) & 0xfffe;

	spi_pinmux();
	spi_enable(0);
	ssi_write(SSI_IMR, 0);
	ssi_write(SSI_BAUDR, div);
	ssi_write(SSI_RX_SAMPLE, (div >> 1) - 1);
	spi_enable(1);
}
