// SPDX-License-Identifier: GPL-2.0+
/*
 * Axera AX620E boot SPI flash controller: a DesignWare SSI in standard
 * (single line) SPI mode, polled.
 *
 * Written from the vendor SDK's U-Boot 2020.04 (drivers/spi/axera_spi.c)
 * and its first stage (boot/bl1/driver/spi/dw_spi_mst.c): every transfer
 * is either transmit only or receive only, never both, and the chip select
 * is a GPIO held across the command and data phases. A receive-only phase
 * is started by a dummy write to the data register and clocks in CTRLR1 + 1
 * frames, so it is kept to the receive FIFO's depth: the vendor drives
 * longer ones through the AXI DMA, which this driver does not.
 *
 * The flash runs on the clock and sample delay the first stage left, which
 * read the image this U-Boot came from with opcode 0x03. Operations with
 * dummy cycles are refused, so spi-nor reads with 0x03 as well.
 */

#include <common.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <errno.h>
#include <log.h>
#include <spi.h>
#include <spi-mem.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/iopoll.h>

#define DW_SPI_CTRLR0		0x00
#define DW_SPI_CTRLR1		0x04
#define DW_SPI_SSIENR		0x08
#define DW_SPI_SER		0x10
#define DW_SPI_TXFTLR		0x18
#define DW_SPI_RXFTLR		0x1c
#define DW_SPI_TXFLR		0x20
#define DW_SPI_RXFLR		0x24
#define DW_SPI_SR		0x28
#define DW_SPI_IMR		0x2c
#define DW_SPI_DMACR		0x4c
#define DW_SPI_DR		0x60

#define CTRLR0_DFS_8		7
#define CTRLR0_MODE_SHIFT	8
#define CTRLR0_TMOD_TO		(1 << 10)
#define CTRLR0_TMOD_RO		(2 << 10)

#define SR_BUSY			BIT(0)
#define SR_TF_EMPT		BIT(2)

#define AX_SPI_TIMEOUT_US	1000000

struct ax_spi_priv {
	void __iomem *regs;
	struct gpio_desc cs_gpio;
	unsigned int mode;
	unsigned int fifo_len;
};

static void ax_spi_enable(struct ax_spi_priv *priv, int on)
{
	writel(on ? 1 : 0, priv->regs + DW_SPI_SSIENR);
}

/*
 * Start a receive-only phase. x19 is cleared around the write: the
 * ax620e machine of OpenIPC's QEMU stores the data it is about to return
 * to [x19 + 120] as well, a stand-in for the vendor U-Boot's driver state
 * there, unless x19 is zero. Harmless on silicon.
 */
static void ax_spi_enable_rx(struct ax_spi_priv *priv)
{
	asm volatile("mov x19, xzr\n\tstr %w0, [%1]"
		     : : "r"(1), "r"(priv->regs + DW_SPI_SSIENR)
		     : "x19", "memory");
}

/* Depth of the transmit FIFO, which the receive FIFO matches */
static unsigned int ax_spi_fifo_len(struct ax_spi_priv *priv)
{
	unsigned int n;

	for (n = 1; n < 256; n++) {
		writel(n, priv->regs + DW_SPI_TXFTLR);
		if (readl(priv->regs + DW_SPI_TXFTLR) != n)
			break;
	}
	writel(0, priv->regs + DW_SPI_TXFTLR);
	return n;
}

static void ax_spi_cs(struct udevice *dev, int on)
{
	struct ax_spi_priv *priv = dev_get_priv(dev->parent);

	dm_gpio_set_value(&priv->cs_gpio, on);
}

static int ax_spi_xfer(struct udevice *dev, unsigned int bitlen,
		       const void *dout, void *din, unsigned long flags)
{
	struct ax_spi_priv *priv = dev_get_priv(dev->parent);
	unsigned int len = bitlen / 8, done = 0, n;
	const u8 *tx = dout;
	u8 *rx = din;
	u32 sr;
	int ret = 0;

	if (bitlen % 8 || (tx && rx))
		return -EINVAL;

	if (flags & SPI_XFER_BEGIN)
		ax_spi_cs(dev, 1);

	if (len) {
		ax_spi_enable(priv, 0);
		writel(CTRLR0_DFS_8 | (priv->mode << CTRLR0_MODE_SHIFT) |
		       (rx ? CTRLR0_TMOD_RO : CTRLR0_TMOD_TO),
		       priv->regs + DW_SPI_CTRLR0);
		writel(0, priv->regs + DW_SPI_DMACR);
		writel(0, priv->regs + DW_SPI_TXFTLR);
		writel(0, priv->regs + DW_SPI_RXFTLR);
		writel(BIT(spi_chip_select(dev)), priv->regs + DW_SPI_SER);

		if (rx) {
			writel(len - 1, priv->regs + DW_SPI_CTRLR1);
			ax_spi_enable_rx(priv);
			writel(0xffffffff, priv->regs + DW_SPI_DR);
			while (done < len) {
				ret = readl_poll_timeout(priv->regs + DW_SPI_RXFLR,
							 n, n, AX_SPI_TIMEOUT_US);
				if (ret)
					break;
				while (n-- && done < len)
					rx[done++] = readl(priv->regs + DW_SPI_DR);
			}
		} else {
			ax_spi_enable(priv, 1);
			while (done < len) {
				n = priv->fifo_len -
				    readl(priv->regs + DW_SPI_TXFLR);
				while (n-- && done < len)
					writel(tx[done++], priv->regs + DW_SPI_DR);
			}
		}

		if (!ret)
			ret = readl_poll_timeout(priv->regs + DW_SPI_SR, sr,
						 (sr & SR_TF_EMPT) &&
						 !(sr & SR_BUSY),
						 AX_SPI_TIMEOUT_US);
	}

	if ((flags & SPI_XFER_END) || ret)
		ax_spi_cs(dev, 0);

	if (ret)
		log_debug("%s: %s of %u bytes timed out\n", dev->name,
			  rx ? "read" : "write", len);
	return ret;
}

static bool ax_spi_supports_op(struct spi_slave *slave,
			       const struct spi_mem_op *op)
{
	if (op->dummy.nbytes)
		return false;
	return spi_mem_default_supports_op(slave, op);
}

static const struct spi_controller_mem_ops ax_spi_mem_ops = {
	.supports_op = ax_spi_supports_op,
};

/* The first stage's clock and sample delay stay */
static int ax_spi_set_speed(struct udevice *bus, uint speed)
{
	return 0;
}

static int ax_spi_set_mode(struct udevice *bus, uint mode)
{
	struct ax_spi_priv *priv = dev_get_priv(bus);

	priv->mode = mode & (SPI_CPHA | SPI_CPOL);
	return 0;
}

static int ax_spi_child_pre_probe(struct udevice *dev)
{
	struct ax_spi_priv *priv = dev_get_priv(dev->parent);
	struct spi_slave *slave = dev_get_parent_priv(dev);

	slave->max_read_size = priv->fifo_len;
	return 0;
}

static int ax_spi_probe(struct udevice *bus)
{
	struct ax_spi_priv *priv = dev_get_priv(bus);
	int ret;

	priv->regs = dev_read_addr_ptr(bus);
	if (!priv->regs)
		return -EINVAL;

	ret = gpio_request_by_name(bus, "cs-gpios", 0, &priv->cs_gpio,
				   GPIOD_IS_OUT);
	if (ret) {
		dev_err(bus, "no chip select GPIO: %d\n", ret);
		return ret;
	}

	ax_spi_enable(priv, 0);
	writel(0, priv->regs + DW_SPI_IMR);
	priv->fifo_len = ax_spi_fifo_len(priv);
	ax_spi_enable(priv, 1);

	return 0;
}

static const struct dm_spi_ops ax_spi_ops = {
	.xfer		= ax_spi_xfer,
	.set_speed	= ax_spi_set_speed,
	.set_mode	= ax_spi_set_mode,
	.mem_ops	= &ax_spi_mem_ops,
};

static const struct udevice_id ax_spi_ids[] = {
	{ .compatible = "axera,ax620e-spi" },
	{ }
};

U_BOOT_DRIVER(axera_spi) = {
	.name		= "axera_spi",
	.id		= UCLASS_SPI,
	.of_match	= ax_spi_ids,
	.ops		= &ax_spi_ops,
	.probe		= ax_spi_probe,
	.child_pre_probe = ax_spi_child_pre_probe,
	.priv_auto	= sizeof(struct ax_spi_priv),
};
