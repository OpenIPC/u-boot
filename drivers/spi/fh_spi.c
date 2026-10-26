// SPDX-License-Identifier: GPL-2.0+
/*
 * Fullhan SPI0, the flash controller of every Fullhan camera SoC: a
 * DesignWare APB SSI (with Fullhan's "SPIC" read-acceleration registers,
 * left in their standard mode). Non-DM, under the legacy spi_xfer() API,
 * so that the mainline spi-nor layer drives the flash.
 *
 * As in the vendor U-Boots and kernels, the chip select is a GPIO, not
 * the controller's SSI0_CSN pad: the controller's own select drops
 * whenever its TX FIFO runs dry. Transfers are full duplex in FIFO-sized
 * chunks, so a gap between chunks only pauses SCLK.
 */

#include <common.h>
#include <malloc.h>
#include <spi.h>
#include <asm/io.h>
#include <asm/arch/fullhan.h>
#include <linux/delay.h>

#define SSI_CTRLR0		0x00
#define SSI_CTRLR1		0x04
#define SSI_SSIENR		0x08
#define SSI_SER			0x10
#define SSI_BAUDR		0x14
#define SSI_TXFLR		0x20
#define SSI_RXFLR		0x24
#define SSI_SR			0x28
#define SSI_IMR			0x2C
#define SSI_DMACR		0x4C
#define SSI_DR			0x60
#define SSI_RX_SAMPLE_DLY	0xF0
#define SSI_CCFGR		0xF4	/* SPIC: data phase wires, XIP, swap */
#define SSI_OPCR		0xF8	/* SPIC: XIP opcodes */
#define SSI_TIMCR		0xFC
#define SSI_BBAR0		0x100
#define SSI_BBAR1		0x104

#define CTRLR0_DFS_8		0x7
#define CTRLR0_SCPH		BIT(6)
#define CTRLR0_SCPOL		BIT(7)

#define SR_BUSY			BIT(0)
#define SR_TFE			BIT(2)

/* The shallowest of the parts' FIFOs (32 on FH8632's U-Boot) */
#define FH_SPI_FIFO		32
#define FH_SPI_TIMEOUT_US	100000

struct fh_spi_slave {
	struct spi_slave slave;
	void __iomem *regs;
	unsigned int baudr;
	unsigned int mode;
};

static inline struct fh_spi_slave *to_fh_spi(struct spi_slave *slave)
{
	return container_of(slave, struct fh_spi_slave, slave);
}

static void fh_spi_cs(struct fh_spi_slave *fs, int active)
{
	if (active) {
		fh_gpio_output(FH_SPI0_CS_GPIO, 0);
		writel(1, fs->regs + SSI_SER);
	} else {
		writel(0, fs->regs + SSI_SER);
		fh_gpio_output(FH_SPI0_CS_GPIO, 1);
	}
}

static void fh_spi_hw_init(struct fh_spi_slave *fs)
{
	u32 ctrlr0 = CTRLR0_DFS_8;	/* Motorola SPI, TX and RX */

	if (fs->mode & SPI_CPHA)
		ctrlr0 |= CTRLR0_SCPH;
	if (fs->mode & SPI_CPOL)
		ctrlr0 |= CTRLR0_SCPOL;

	writel(0, fs->regs + SSI_SSIENR);
	writel(ctrlr0, fs->regs + SSI_CTRLR0);
	writel(fs->baudr, fs->regs + SSI_BAUDR);
	writel(0, fs->regs + SSI_IMR);
	writel(0, fs->regs + SSI_DMACR);
	writel(0, fs->regs + SSI_SER);
	/* One cycle of sample delay, as the vendor drivers set at 50 MHz */
	writel(1, fs->regs + SSI_RX_SAMPLE_DLY);
#ifdef FH_SPI0_SPIC_CCFGR
	/*
	 * SPIC, as the vendor's spic_wire_init() leaves it: single-wire data
	 * phase, no dummies, XIP off, 8-bit frames without the byte swap
	 */
	writel((readl(fs->regs + SSI_CCFGR) & ~0x1ffff) | FH_SPI0_SPIC_CCFGR,
	       fs->regs + SSI_CCFGR);
	writel((readl(fs->regs + SSI_OPCR) & ~(0xff << 20 | 0x30000)) |
	       0xff << 20, fs->regs + SSI_OPCR);
	writel(0, fs->regs + SSI_TIMCR);
	writel(0xb0000000, fs->regs + SSI_BBAR0);
	writel(0xb0000000, fs->regs + SSI_BBAR1);
#endif
	writel(1, fs->regs + SSI_SSIENR);
}

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return bus == 0 && cs == 0;
}

void spi_cs_activate(struct spi_slave *slave)
{
	fh_spi_cs(to_fh_spi(slave), 1);
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	fh_spi_cs(to_fh_spi(slave), 0);
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
				  unsigned int max_hz, unsigned int mode)
{
	struct fh_spi_slave *fs;
	unsigned int div;

	if (!spi_cs_is_valid(bus, cs))
		return NULL;

	fs = spi_alloc_slave(struct fh_spi_slave, bus, cs);
	if (!fs)
		return NULL;

	fs->regs = (void __iomem *)FH_SPI0_BASE;
	fs->mode = mode;
	/* SCLK = ssi_clk / BAUDR, BAUDR even and at least 2 */
	div = DIV_ROUND_UP(FH_SPI0_CLK, max_hz ? max_hz : 1);
	div = (div + 1) & ~1;
	fs->baudr = clamp(div, 2U, 0xfffeU);

	fh_spi0_setup();
	fh_gpio_output(FH_SPI0_CS_GPIO, 1);
	fh_spi_hw_init(fs);

	return &fs->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	free(to_fh_spi(slave));
}

int spi_claim_bus(struct spi_slave *slave)
{
	fh_spi_hw_init(to_fh_spi(slave));
	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
}

static int fh_spi_wait_idle(struct fh_spi_slave *fs)
{
	unsigned int t;

	for (t = 0; t < FH_SPI_TIMEOUT_US; t++) {
		u32 sr = readl(fs->regs + SSI_SR);

		if ((sr & SR_TFE) && !(sr & SR_BUSY))
			return 0;
		udelay(1);
	}
	return -ETIMEDOUT;
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen,
	     const void *dout, void *din, unsigned long flags)
{
	struct fh_spi_slave *fs = to_fh_spi(slave);
	const u8 *tx = dout;
	u8 *rx = din;
	unsigned int len = bitlen / 8;
	int ret = 0;

	if (bitlen % 8)
		return -EINVAL;

	if (flags & SPI_XFER_BEGIN)
		fh_spi_cs(fs, 1);

	while (len) {
		unsigned int n = min(len, (unsigned int)FH_SPI_FIFO);
		unsigned int i, got = 0, t = 0;

		for (i = 0; i < n; i++)
			writel(tx ? tx[i] : 0xff, fs->regs + SSI_DR);
		while (got < n) {
			u8 b;

			if (!readl(fs->regs + SSI_RXFLR)) {
				if (++t > FH_SPI_TIMEOUT_US) {
					ret = -ETIMEDOUT;
					goto out;
				}
				udelay(1);
				continue;
			}
			b = readl(fs->regs + SSI_DR);
			if (rx)
				rx[got] = b;
			got++;
		}
		if (tx)
			tx += n;
		if (rx)
			rx += n;
		len -= n;
	}
	ret = fh_spi_wait_idle(fs);
out:
	if (ret)
		printf("fh_spi: transfer timed out\n");
	if ((flags & SPI_XFER_END) || ret)
		fh_spi_cs(fs, 0);
	return ret;
}
