// SPDX-License-Identifier: GPL-2.0+
/*
 * Faraday FTSSP010 as an SPI master, non-DM: the Grain Media GM8126's boot
 * flash.
 *
 * Frames are eight bits here, so data is in the flash's own byte order --
 * what Linux sees. The mask ROM and the vendor loaders use 32-bit frames,
 * which is why the first stage is stored with its words byte-swapped
 * (openipc/gm8126-image.py).
 */

#include <common.h>
#include <malloc.h>
#include <spi.h>
#include <asm/io.h>
#include <asm/arch/gm8126.h>
#include <linux/bitops.h>
#include <linux/delay.h>

#define SSP_CR0		(CONFIG_FTSSP010_BASE + 0x00)
#define SSP_CR1		(CONFIG_FTSSP010_BASE + 0x04)
#define SSP_CR2		(CONFIG_FTSSP010_BASE + 0x08)
#define SSP_STATUS	(CONFIG_FTSSP010_BASE + 0x0c)
#define SSP_ICR		(CONFIG_FTSSP010_BASE + 0x10)
#define SSP_DATA	(CONFIG_FTSSP010_BASE + 0x18)

#define CR0_SPI_MASTER	0x102c		/* Motorola SPI, master, mode 0 */
#define CR0_SCLKPH	BIT(0)
#define CR0_SCLKPO	BIT(1)
#define CR1_SDL(bits)	(((bits) - 1) << 16)
#define CR2_ENABLE	BIT(0)
#define CR2_TXDOE	BIT(1)
#define CR2_RXFCLR	BIT(2)
#define CR2_TXFCLR	BIT(3)
#define STATUS_TFNF	BIT(1)
#define STATUS_RFVE(s)	(((s) >> 4) & 0x1f)

/* at most this many frames in flight: the receive FIFO's depth */
#define SSP_FIFO	16
#define SSP_TIMEOUT_US	100000

static void ssp_cs(int active)
{
	writel(GM_SPI_CS, GM_GPIO0_BASE + (active ? GM_GPIO_CLEAR : GM_GPIO_SET));
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
				  unsigned int max_hz, unsigned int mode)
{
	struct spi_slave *slave;
	unsigned int div, clk = gm_ssp_hz();

	if (bus || cs)
		return NULL;
	slave = spi_alloc_slave_base(bus, cs);
	if (!slave)
		return NULL;

	setbits_le32(GM_GPIO0_BASE + GM_GPIO_DIR, GM_SPI_CS);
	ssp_cs(0);
	/* SCLK = clk / 2 / (div + 1) */
	div = max_hz ? DIV_ROUND_UP(clk, 2 * max_hz) : 1;
	div = div ? div - 1 : 0;
	writel(CR0_SPI_MASTER | (mode & SPI_CPHA ? CR0_SCLKPH : 0) |
	       (mode & SPI_CPOL ? CR0_SCLKPO : 0), SSP_CR0);
	writel(0, SSP_ICR);
	writel(CR1_SDL(8) | (div & 0xffff), SSP_CR1);
	return slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	free(slave);
}

int spi_claim_bus(struct spi_slave *slave)
{
	writel(CR2_ENABLE | CR2_TXDOE | CR2_RXFCLR | CR2_TXFCLR, SSP_CR2);
	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
	     void *din, unsigned long flags)
{
	unsigned int len = bitlen / 8, tx = 0, rx = 0, wait = 0;
	const u8 *out = dout;
	u8 *in = din;
	int ret = 0;

	if (bitlen % 8)
		return -EINVAL;
	if (flags & SPI_XFER_BEGIN)
		ssp_cs(1);
	while (rx < len) {
		u32 st = readl(SSP_STATUS);
		unsigned int n = STATUS_RFVE(st);

		if (tx < len && tx - rx < SSP_FIFO && (st & STATUS_TFNF)) {
			writel(out ? out[tx] : 0, SSP_DATA);
			tx++;
			wait = 0;
		} else if (!n) {
			if (++wait > SSP_TIMEOUT_US) {
				printf("ftssp010: transfer timed out\n");
				ret = -ETIMEDOUT;
				break;
			}
			udelay(1);
		}
		for (; n && rx < len; n--) {
			u8 v = readl(SSP_DATA);

			if (in)
				in[rx] = v;
			rx++;
			wait = 0;
		}
	}
	if (ret || (flags & SPI_XFER_END))
		ssp_cs(0);
	return ret;
}

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return !bus && !cs;
}

void spi_cs_activate(struct spi_slave *slave)
{
	ssp_cs(1);
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	ssp_cs(0);
}
