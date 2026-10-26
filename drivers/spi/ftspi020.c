// SPDX-License-Identifier: GPL-2.0+
/*
 * Faraday FTSPI020, the flash controller of Grain Media's GM813x. Non-DM.
 *
 * The controller does not shift raw bytes: software describes a whole
 * flash command -- address and its width, dummy cycles, opcode, data
 * count and direction -- and the controller runs it, chip select
 * included, moving the data through a FIFO. So this driver takes spi-mem
 * operations whole (spi_mem_exec_op(), in place of the generic non-DM one
 * that would split them into spi_xfer() calls) and the mainline spi-nor
 * layer drives the flash through it, single wire.
 *
 * After the GM8136 SDK U-Boot's drivers/mtd/spi/spi020.c, in its PIO
 * mode: data moves a FIFO's worth at a time when the controller reports
 * it ready, the command completes after the last byte.
 */

#include <common.h>
#include <malloc.h>
#include <spi.h>
#include <spi-mem.h>
#include <asm/io.h>
#include <asm/unaligned.h>
#include <linux/bitops.h>
#include <linux/delay.h>

#define SPI020_ADDR		0x00
#define SPI020_CMD1		0x04
#define SPI020_DATA_CNT		0x08
#define SPI020_CMD2		0x0C
#define SPI020_CONTROL		0x10
#define SPI020_STATUS		0x18
#define SPI020_INTR_CONTROL	0x20
#define SPI020_INTR_STATUS	0x24
#define SPI020_DATA		0x100

#define CMD1_ADDR_LEN(n)	((n) & 7)
#define CMD1_DUMMY_CYCLES(n)	(((n) & 0xff) << 16)
#define CMD1_INS_LEN_1		(1 << 24)

#define CMD2_OPCODE(op)		((op) << 24)
#define CMD2_WRITE		BIT(1)	/* no data or data out */
#define CMD2_INTR_EN		BIT(0)	/* report completion in INTR_STATUS */

#define STATUS_TX_READY		BIT(0)
#define STATUS_RX_READY		BIT(1)

#define INTR_CMD_COMPLETE	BIT(0)

#define SPI020_CONTROL_ABORT	BIT(8)

/* FIFO depth in words, fixed by the hardware */
#define SPI020_FIFO_WORDS	32
#define SPI020_FIFO_BYTES	(SPI020_FIFO_WORDS * 4)

#define SPI020_TIMEOUT_US	1000000

static void __iomem *const regs = (void __iomem *)CONFIG_FTSPI020_BASE;

static int ftspi020_wait(u32 reg, u32 bit)
{
	unsigned long start = timer_get_us();

	while (!(readl(regs + reg) & bit)) {
		if (timer_get_us() - start > SPI020_TIMEOUT_US)
			return -ETIMEDOUT;
	}
	return 0;
}

static int ftspi020_read_data(u8 *buf, unsigned int len)
{
	unsigned int i, n;
	int ret;

	while (len) {
		ret = ftspi020_wait(SPI020_STATUS, STATUS_RX_READY);
		if (ret)
			return ret;
		if (len >= SPI020_FIFO_BYTES) {
			/* a full FIFO, in words */
			for (i = 0; i < SPI020_FIFO_WORDS; i++, buf += 4)
				put_unaligned_le32(readl(regs + SPI020_DATA),
						   buf);
			n = SPI020_FIFO_BYTES;
		} else {
			/* the rest, a byte at a time */
			for (i = 0; i < len; i++)
				*buf++ = readb(regs + SPI020_DATA);
			n = len;
		}
		len -= n;
	}
	return 0;
}

static int ftspi020_write_data(const u8 *buf, unsigned int len)
{
	unsigned int i, n;
	int ret;

	while (len) {
		ret = ftspi020_wait(SPI020_STATUS, STATUS_TX_READY);
		if (ret)
			return ret;
		if (len >= SPI020_FIFO_BYTES) {
			for (i = 0; i < SPI020_FIFO_WORDS; i++, buf += 4)
				writel(get_unaligned_le32(buf),
				       regs + SPI020_DATA);
			n = SPI020_FIFO_BYTES;
		} else {
			for (i = 0; i < len; i++)
				writeb(*buf++, regs + SPI020_DATA);
			n = len;
		}
		len -= n;
	}
	return 0;
}

int spi_mem_exec_op(struct spi_slave *slave, const struct spi_mem_op *op)
{
	unsigned int len = op->data.nbytes;
	bool in = len && op->data.dir == SPI_MEM_DATA_IN;
	u32 cmd2;
	int ret;

	if (!spi_mem_supports_op(slave, op) || op->addr.nbytes > 4 ||
	    op->dummy.nbytes > 0xff / 8)
		return -EOPNOTSUPP;

	writel(INTR_CMD_COMPLETE, regs + SPI020_INTR_STATUS);

	writel(op->addr.val, regs + SPI020_ADDR);
	writel(CMD1_ADDR_LEN(op->addr.nbytes) |
	       CMD1_DUMMY_CYCLES(op->dummy.nbytes * 8) | CMD1_INS_LEN_1,
	       regs + SPI020_CMD1);
	writel(len, regs + SPI020_DATA_CNT);

	cmd2 = CMD2_OPCODE(op->cmd.opcode) | CMD2_INTR_EN;
	if (!in)
		cmd2 |= CMD2_WRITE;
	/* this write starts the command */
	writel(cmd2, regs + SPI020_CMD2);

	if (in)
		ret = ftspi020_read_data(op->data.buf.in, len);
	else if (len)
		ret = ftspi020_write_data(op->data.buf.out, len);
	else
		ret = 0;

	if (!ret)
		ret = ftspi020_wait(SPI020_INTR_STATUS, INTR_CMD_COMPLETE);
	writel(INTR_CMD_COMPLETE, regs + SPI020_INTR_STATUS);

	if (ret) {
		printf("ftspi020: opcode 0x%02x timed out\n", op->cmd.opcode);
		/* drop the command and the FIFOs */
		setbits_le32(regs + SPI020_CONTROL, SPI020_CONTROL_ABORT);
		udelay(10);
	}
	return ret;
}

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return bus == 0 && cs == 0;
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
				  unsigned int max_hz, unsigned int mode)
{
	struct spi_slave *slave;

	if (!spi_cs_is_valid(bus, cs))
		return NULL;

	slave = spi_alloc_slave_base(bus, cs);
	if (!slave)
		return NULL;

	/*
	 * The clock divider and mode stay as the boot ROM left them, as in
	 * the SDK U-Boot; completion is polled, not interrupted.
	 */
	slave->mode = mode;
	writel(0, regs + SPI020_INTR_CONTROL);

	return slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	free(slave);
}

int spi_claim_bus(struct spi_slave *slave)
{
	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
}

/* Only whole commands: see spi_mem_exec_op() */
int spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
	     void *din, unsigned long flags)
{
	return -EOPNOTSUPP;
}
