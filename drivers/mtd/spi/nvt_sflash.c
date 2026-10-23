// SPDX-License-Identifier: GPL-2.0+
/*
 * SPI NOR behind the flash controller of the Novatek NA51089.
 *
 * The controller runs one operation at a time: software loads COMMAND,
 * ROWADDR (sent MSB first, as many bytes as MODULE0 says) and DATALEN,
 * then writes an operation code with OPER_EN into CTRL0; CTRL_STS bit 12
 * says when it is done. The operations used here: a bare command, a
 * command with a status byte back (in CTRL_STS[7:0]), the JEDEC ID, a
 * command with an address, a block erase that polls the status itself,
 * an automatic status poll, and N bytes in or out. Reads and programs
 * hold the chip selected by hand across the command-and-address operation
 * and the data one. Data goes through a 64-byte FIFO, a word at a time,
 * or by DMA.
 *
 * After the vendor U-Boot 2019.04 (drivers/mtd/nvt_flash_spi/ and
 * drivers/mtd/spiflash/spinor.c), single lane, as it reads at 48 MHz.
 * Like ak_sflash on openipc-anyka, this driver provides spi_flash_probe()
 * itself, so mainline's non-DM spi_flash_*() wrappers, the sf command and
 * the environment in SPI flash work without the SPI and spi-nor layers.
 * 3-byte addresses: parts up to 16 MiB.
 */

#include <common.h>
#include <cpu_func.h>
#include <spi_flash.h>
#include <watchdog.h>
#include <asm/cache.h>
#include <asm/io.h>
#include <asm/unaligned.h>
#include <asm/arch/na51089.h>
#include <linux/delay.h>
#include <linux/mtd/mtd.h>
#include <linux/sizes.h>

#define FL_MODULE0		(NVT_FLASH_BASE + 0x00)
#define FL_DLL1			(NVT_FLASH_BASE + 0x08)
#define FL_DLL2			(NVT_FLASH_BASE + 0x0C)
#define FL_SPI_CFG		(NVT_FLASH_BASE + 0x14)
#define FL_CTRL0		(NVT_FLASH_BASE + 0x20)
#define FL_TIME2		(NVT_FLASH_BASE + 0x24)
#define FL_TIME0		(NVT_FLASH_BASE + 0x2C)
#define FL_TIME1		(NVT_FLASH_BASE + 0x30)
#define FL_COMMAND		(NVT_FLASH_BASE + 0x34)
#define FL_COLADDR		(NVT_FLASH_BASE + 0x38)
#define FL_ROWADDR		(NVT_FLASH_BASE + 0x3C)
#define FL_INTMASK		(NVT_FLASH_BASE + 0x44)
#define FL_CTRL_STS		(NVT_FLASH_BASE + 0x48)
#define FL_STS_CHECK		(NVT_FLASH_BASE + 0xBC)
#define FL_DATAPORT		(NVT_FLASH_BASE + 0x100)
#define FL_DATALEN		(NVT_FLASH_BASE + 0x104)
#define FL_FIFO_STS		(NVT_FLASH_BASE + 0x108)
#define FL_FIFO_CTRL		(NVT_FLASH_BASE + 0x10C)
#define FL_DMASTART		(NVT_FLASH_BASE + 0x110)

/* MODULE0 */
#define MOD_COL_CYC(n)		((n) - 1)		/* address bytes */
#define MOD_ROW_CYC(n)		(((n) - 1) << 2)
#define MOD_CYC_MASK		0xf
#define MOD_NOR			(BIT(19) | BIT(20))	/* SPI, NOR */
/* DLL1 */
#define DLL1_SW_RESET		BIT(0)
#define DLL1_SAMPCLK_INV	BIT(2)
#define DLL1_SRC_CLK_SEL	BIT(3)
#define DLL1_PAD_CLK_SEL	BIT(20)
/* DLL2 */
#define DLL2_INDLY(n)		((n) << 24)
#define DLL2_INDLY_MASK		DLL2_INDLY(0x3f)
/* SPI_CFG */
#define CFG_CS_MANUAL		BIT(1)
#define CFG_CS_HIGH		BIT(4)
#define CFG_BUS_WIDTH_MASK	(0x3 << 9)
#define CFG_PULL_WPHLD		BIT(12)
/* CTRL0 */
#define CTRL0_OPER_EN		BIT(12)
#define CTRL0_WP_EN		BIT(13)
#define CTRL0_TIMEOUT_EN	BIT(19)
#define CTRL0_PROTECT		(BIT(20) | BIT(21))
/* CTRL_STS, INTMASK */
#define STS_COMP		BIT(12)
/* FIFO_CTRL */
#define FIFO_EN			BIT(0)
#define FIFO_DMA		BIT(1)
#define FIFO_WRITE		BIT(2)
/* FIFO_STS */
#define FIFO_CNT_MASK		0x1f			/* words */
#define FIFO_EMPTY		BIT(8)
#define FIFO_FULL		BIT(9)
#define FIFO_BYTES		64

/* Operations */
#define OP_BLK_ERSE		0x05
#define OP_READ_ID		0x06
#define OP_CMD			0x07
#define OP_CMD_STS		0x0B
#define OP_CMD_ADDR		0x0C
#define OP_READ_N		0x18
#define OP_WRITE_N		0x19
#define OP_AUTO_STATUS		0x1D

/* The clock gate that would stop the controller with the chip selected */
#define NVT_CG_AUTOGATE		(NVT_CG_BASE + 0xB0)
#define CG_AUTOGATE_FLASH	BIT(19)
#define NVT_CG_FLASH_DIV	(NVT_CG_BASE + 0x40)	/* [17:12] */
#define NVT_FLASH_SRC_HZ	480000000
#define NVT_TOP_FLASH_EN	(NVT_TOP_BASE + 0x04)
#define NVT_TOP_GPIO_C		(NVT_TOP_BASE + 0xA0)	/* pads as GPIO */
#define NVT_TOP_SRAM_SD		(NVT_TOP_BASE + 0x1000)	/* SRAM shut down */
#define NVT_PAD_FLASH_DRV	(NVT_PAD_BASE + 0x40)

#define SF_HZ			48000000
#define SZ_PAGE			256
#define SZ_SECTOR		SZ_64K
/* The vendor U-Boot gives every operation this long */
#define OP_TIMEOUT_MS		4500

#define CMD_WRSR		0x01
#define CMD_PP			0x02
#define CMD_READ		0x03
#define CMD_WRDI		0x04
#define CMD_RDSR		0x05
#define CMD_WREN		0x06
#define CMD_RDSR2		0x35
#define CMD_RDID		0x9F
#define CMD_SE_64K		0xD8

#define SR1_WIP			BIT(0)
#define SR1_WEL			BIT(1)
#define SR1_BP			0x7C	/* GigaDevice/Winbond and alikes: BP0-4 */
#define SR1_BP_OTHER		0x3C	/* BP0-3 */

#define MFR_GIGADEVICE		0xc8
#define MFR_WINBOND		0xef
#define MFR_XMC			0x20
#define MFR_ZBIT		0x5e
#define MFR_BOYA		0x68

static void fl_addr_cycles(unsigned int row, unsigned int col)
{
	clrsetbits_le32(FL_MODULE0, MOD_CYC_MASK,
			MOD_ROW_CYC(row) | MOD_COL_CYC(col));
}

/* Load an operation; the data, if any, goes through the FIFO */
static void fl_setup(u32 op, u32 row, u32 len, u32 fifo)
{
	clrbits_le32(FL_INTMASK, STS_COMP);
	writel(fifo, FL_FIFO_CTRL);
	writel(0, FL_CTRL0);
	writel(0, FL_COLADDR);
	writel(row, FL_ROWADDR);
	writel(len, FL_DATALEN);
	writel(op | (fifo & FIFO_WRITE ? 0 : CTRL0_WP_EN), FL_CTRL0);
	writel(STS_COMP, FL_CTRL_STS);
	if (len)
		writel(fifo | FIFO_EN, FL_FIFO_CTRL);
	setbits_le32(FL_INTMASK, STS_COMP);
}

static void fl_start(u32 cmd, bool timeout)
{
	writel(cmd, FL_COMMAND);
	setbits_le32(FL_CTRL0, CTRL0_OPER_EN | (timeout ? CTRL0_TIMEOUT_EN : 0));
}

static int fl_wait(void)
{
	ulong start = get_timer(0);

	while (!(readl(FL_CTRL_STS) & STS_COMP)) {
		if (get_timer(start) > OP_TIMEOUT_MS) {
			printf("SF: the flash controller timed out\n");
			return -ETIMEDOUT;
		}
		WATCHDOG_RESET();
	}
	writel(STS_COMP, FL_CTRL_STS);
	return 0;
}

static int fl_op(u32 op, u32 cmd, u32 row, bool timeout)
{
	fl_setup(op, row, 0, 0);
	fl_start(cmd, timeout);
	return fl_wait();
}

static int fl_wait_fifo(u32 bits, u32 val)
{
	ulong start = get_timer(0);

	while ((readl(FL_FIFO_STS) & bits) != val) {
		if (get_timer(start) > OP_TIMEOUT_MS) {
			printf("SF: the flash FIFO timed out\n");
			return -ETIMEDOUT;
		}
	}
	return 0;
}

/* The words of a PIO read, as the FIFO fills */
static int fl_pio_in(u8 *buf, unsigned int len)
{
	int ret;

	while (len) {
		unsigned int n = min_t(unsigned int, len, FIFO_BYTES);
		unsigned int i;

		if (n == FIFO_BYTES)
			ret = fl_wait_fifo(FIFO_FULL, FIFO_FULL);
		else
			ret = fl_wait_fifo(FIFO_CNT_MASK, DIV_ROUND_UP(n, 4));
		if (ret)
			return ret;
		for (i = 0; i < n; i += 4) {
			u32 w = readl(FL_DATAPORT);

			if (n - i >= 4)
				put_unaligned_le32(w, buf + i);
			else
				memcpy(buf + i, &w, n - i);
		}
		buf += n;
		len -= n;
	}
	return fl_wait();
}

/* The words of a PIO write, a FIFO at a time */
static int fl_pio_out(const u8 *buf, unsigned int len)
{
	bool tail = len % 4;
	int ret;

	while (len) {
		unsigned int n = min_t(unsigned int, len, FIFO_BYTES);
		unsigned int i;

		ret = fl_wait_fifo(FIFO_EMPTY, FIFO_EMPTY);
		if (ret)
			return ret;
		for (i = 0; i < n; i += 4) {
			u32 w = 0;

			if (n - i >= 4)
				w = get_unaligned_le32(buf + i);
			else
				memcpy(&w, buf + i, n - i);
			writel(w, FL_DATAPORT);
		}
		buf += n;
		len -= n;
	}
	/* As the vendor does after a word it only partly filled */
	if (tail) {
		mdelay(1);
		clrbits_le32(FL_FIFO_CTRL, FIFO_EN);
	}
	return fl_wait();
}

/* Chip select by hand, for an operation in two parts */
static void fl_cs_manual(bool on)
{
	if (on) {
		clrbits_le32(NVT_CG_AUTOGATE, CG_AUTOGATE_FLASH);
		setbits_le32(FL_SPI_CFG, CFG_CS_HIGH);
		setbits_le32(FL_SPI_CFG, CFG_CS_MANUAL);
		clrbits_le32(FL_SPI_CFG, CFG_CS_HIGH);
	} else {
		setbits_le32(FL_SPI_CFG, CFG_CS_HIGH);
		clrbits_le32(FL_SPI_CFG, CFG_CS_MANUAL);
		setbits_le32(NVT_CG_AUTOGATE, CG_AUTOGATE_FLASH);
	}
}

static int sf_wait_ready(void)
{
	/* The controller reads SR1 until WIP (mask bit 0) reads 0 */
	writel(SR1_WIP << 8, FL_STS_CHECK);
	return fl_op(OP_AUTO_STATUS, CMD_RDSR, 0, true);
}

static u8 sf_read_reg(u8 op)
{
	if (fl_op(OP_CMD_STS, op, 0, false))
		return 0xff;
	return readl(FL_CTRL_STS) & 0xff;
}

/*
 * A program or erase the flash did not accept leaves it idle, which the
 * ready wait would take for success: make sure write enable latched.
 */
static int sf_write_enable(void)
{
	int ret = fl_op(OP_CMD, CMD_WREN, 0, false);

	if (ret)
		return ret;
	if (!(sf_read_reg(CMD_RDSR) & SR1_WEL)) {
		printf("SF: write enable did not latch\n");
		return -EIO;
	}
	return 0;
}

/* The status register bytes go out as the "address" of WRSR */
static int sf_write_sr(const u8 *sr, unsigned int len)
{
	int ret = sf_write_enable();

	if (ret)
		return ret;
	fl_addr_cycles(len, 1);
	ret = fl_op(OP_CMD_ADDR, CMD_WRSR, len == 2 ? sr[0] << 8 | sr[1] : sr[0],
		    false);
	return ret ? ret : sf_wait_ready();
}

/* Set when the block protection could not be cleared: writes are refused */
static bool nvt_protected;

/*
 * Clear the block protection a vendor loader may have left set, and check
 * that it went: a program or erase of a protected block is accepted and
 * dropped, which would pass for success.
 */
static int sf_unprotect(u8 mfr)
{
	u8 sr[2] = { sf_read_reg(CMD_RDSR) };
	u8 bp = SR1_BP_OTHER;
	int ret;

	if (sr[0] == 0xff)
		return 0;
	if (mfr == MFR_GIGADEVICE || mfr == MFR_WINBOND || mfr == MFR_XMC ||
	    mfr == MFR_ZBIT || mfr == MFR_BOYA)
		bp = SR1_BP;
	if (!(sr[0] & bp))
		return 0;

	printf("SF: block protection (SR1 %02x), clearing\n", sr[0]);
	sr[0] &= ~bp;
	if (bp == SR1_BP) {
		/* QE lives in SR2, written as WRSR's second byte: keep it */
		sr[1] = sf_read_reg(CMD_RDSR2);
		ret = sf_write_sr(sr, 2);
	} else {
		ret = sf_write_sr(sr, 1);
	}
	sr[0] = sf_read_reg(CMD_RDSR);
	if (ret || sr[0] & bp) {
		printf("SF: block protection stays (SR1 %02x): writes refused\n",
		       sr[0]);
		return -EROFS;
	}
	return 0;
}

/*
 * READ (0x03) with the chip held selected: command and address, then the
 * data, by DMA into a word-aligned buffer, through the FIFO otherwise.
 */
static int sf_read_chunk(u32 addr, u8 *buf, unsigned int len, bool dma)
{
	int ret;

	fl_cs_manual(true);
	fl_addr_cycles(3, 1);
	ret = fl_op(OP_CMD_ADDR, CMD_READ, addr, false);
	if (!ret) {
		clrbits_le32(FL_SPI_CFG, CFG_BUS_WIDTH_MASK);
		if (dma) {
			/* No dirty line may land on the data afterwards */
			invalidate_dcache_range((ulong)buf, (ulong)buf + len);
			writel((ulong)buf, FL_DMASTART);
			fl_setup(OP_READ_N, 0, len, FIFO_DMA);
			fl_start(CMD_READ, false);
			ret = fl_wait();
			invalidate_dcache_range((ulong)buf, (ulong)buf + len);
		} else {
			fl_setup(OP_READ_N, 0, len, 0);
			fl_start(CMD_READ, false);
			ret = fl_pio_in(buf, len);
		}
	}
	fl_cs_manual(false);
	return ret;
}

static int nvt_sf_read(struct mtd_info *mtd, loff_t from, size_t len,
		       size_t *retlen, u_char *buf)
{
	unsigned int bulk = 0;
	int ret = 0;

	if (from > mtd->size || len > mtd->size - from)
		return -EINVAL;
	/* The cache-line multiple by DMA, as the vendor does, the rest by PIO */
	if (!((ulong)buf & (ARCH_DMA_MINALIGN - 1)))
		bulk = len & ~(ARCH_DMA_MINALIGN - 1);
	if (bulk)
		ret = sf_read_chunk(from, buf, bulk, true);
	if (!ret && len > bulk)
		ret = sf_read_chunk(from + bulk, buf + bulk, len - bulk, false);
	*retlen = ret ? 0 : len;
	return ret;
}

/* PAGE PROGRAM: a whole aligned page by DMA, as the vendor does, or PIO */
static int sf_program(u32 addr, const u8 *buf, unsigned int len)
{
	bool dma = len == SZ_PAGE && !((ulong)buf & 3);
	int ret;

	ret = sf_write_enable();
	if (ret)
		return ret;
	fl_cs_manual(true);
	fl_addr_cycles(3, 1);
	ret = fl_op(OP_CMD_ADDR, CMD_PP, addr, false);
	if (!ret) {
		clrbits_le32(FL_SPI_CFG, CFG_BUS_WIDTH_MASK);
		if (dma) {
			flush_dcache_range((ulong)buf, (ulong)buf + len);
			writel((ulong)buf, FL_DMASTART);
			fl_setup(OP_WRITE_N, 0, len, FIFO_DMA | FIFO_WRITE);
			fl_start(CMD_PP, false);
			ret = fl_wait();
		} else {
			fl_setup(OP_WRITE_N, 0, len, FIFO_WRITE);
			fl_start(CMD_PP, false);
			ret = fl_pio_out(buf, len);
		}
	}
	fl_cs_manual(false);
	return ret ? ret : sf_wait_ready();
}

static int nvt_sf_write(struct mtd_info *mtd, loff_t to, size_t len,
			size_t *retlen, const u_char *buf)
{
	int ret;

	if (to > mtd->size || len > mtd->size - to)
		return -EINVAL;
	*retlen = 0;
	if (nvt_protected)
		return -EROFS;
	while (len) {
		unsigned int n = min_t(size_t, len,
				       SZ_PAGE - (to & (SZ_PAGE - 1)));

		ret = sf_program(to, buf, n);
		if (ret)
			return ret;
		to += n;
		buf += n;
		len -= n;
		*retlen += n;
	}
	return 0;
}

/* The controller sends the erase, then polls SR1 itself until WIP clears */
static int sf_erase_sector(u32 addr)
{
	int ret = sf_write_enable();

	if (ret)
		return ret;
	fl_addr_cycles(3, 1);
	writel(SR1_WIP << 8, FL_STS_CHECK);
	ret = fl_op(OP_BLK_ERSE, CMD_SE_64K | CMD_RDSR << 8, addr, true);
	fl_op(OP_CMD, CMD_WRDI, 0, false);
	return ret;
}

static int nvt_sf_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	u32 addr, end;
	int ret;

	/* Checked in 64 bits, before anything can wrap */
	if (instr->addr % SZ_SECTOR || instr->len % SZ_SECTOR ||
	    instr->addr > mtd->size || instr->len > mtd->size - instr->addr)
		return -EINVAL;
	if (nvt_protected) {
		instr->state = MTD_ERASE_FAILED;
		return -EROFS;
	}
	instr->state = MTD_ERASING;
	addr = instr->addr;
	end = instr->addr + instr->len;
	for (; addr < end; addr += SZ_SECTOR) {
		ret = sf_erase_sector(addr);
		if (ret) {
			instr->fail_addr = addr;
			instr->state = MTD_ERASE_FAILED;
			return ret;
		}
	}
	instr->state = MTD_ERASE_DONE;
	return 0;
}

/* Clock, reset, pins, timing and PHY, as the vendor's nvt_spinor_reset() */
static void fl_hw_init(void)
{
	setbits_le32(NVT_CG_CLKEN0, NVT_CG_FLASH);
	setbits_le32(NVT_CG_RSTN0, NVT_CG_FLASH);
	writel(0x06002222, FL_TIME0);
	writel(0x7f0f, FL_TIME1);
	setbits_le32(NVT_TOP_FLASH_EN, BIT(13));
	clrbits_le32(NVT_TOP_GPIO_C, 0x50f);
	clrsetbits_le32(NVT_CG_FLASH_DIV, 0x3f << 12,
			(NVT_FLASH_SRC_HZ / SF_HZ - 1) << 12);
	clrbits_le32(NVT_TOP_SRAM_SD, BIT(17));
	clrsetbits_le32(NVT_PAD_FLASH_DRV, 0x3f00ff, 0x150055);
	setbits_le32(FL_MODULE0, MOD_NOR);
	writel(0x5f51, FL_TIME2);

	clrsetbits_le32(FL_DLL1, DLL1_SRC_CLK_SEL,
			DLL1_SAMPCLK_INV | DLL1_PAD_CLK_SEL);
	clrsetbits_le32(FL_DLL2, DLL2_INDLY_MASK, DLL2_INDLY(5));
	setbits_le32(FL_DLL1, DLL1_SW_RESET);
	while (readl(FL_DLL1) & DLL1_SW_RESET)
		;
	clrsetbits_le32(FL_SPI_CFG, CFG_BUS_WIDTH_MASK, CFG_PULL_WPHLD);
}

static struct spi_flash nvt_flash;
static int nvt_announced;

struct spi_flash *spi_flash_probe(unsigned int bus, unsigned int cs,
				  unsigned int max_hz, unsigned int spi_mode)
{
	struct spi_flash *flash = &nvt_flash;
	struct mtd_info *mtd = &flash->mtd;
	u8 id[4];
	u32 jedec, size;

	fl_hw_init();

	fl_addr_cycles(1, 1);
	fl_setup(OP_READ_ID, 0, sizeof(id), 0);
	fl_start(CMD_RDID, false);
	if (fl_pio_in(id, sizeof(id))) {
		printf("SF: the flash controller does not answer\n");
		return NULL;
	}
	jedec = id[0] << 16 | id[1] << 8 | id[2];
	if (!jedec || jedec == 0xffffff) {
		printf("SF: no flash (JEDEC id %06x)\n", jedec);
		return NULL;
	}
	/*
	 * The last ID byte is log2 of the size on the parts these boards
	 * carry. Addresses are 3 bytes: past 16 MiB, only the first 16 MiB.
	 */
	if (id[2] > 0x18) {
		size = SZ_16M;
		if (!nvt_announced)
			printf("SF: JEDEC %06x is over 16 MiB, using its first 16 MiB\n",
			       jedec);
	} else if (id[2] >= 0x10) {
		size = 1 << id[2];
	} else {
		size = SZ_8M;
		if (!nvt_announced)
			printf("SF: JEDEC %06x: size unknown, taking 8 MiB\n", jedec);
	}

	nvt_protected = sf_unprotect(id[0]) < 0;

	memset(flash, 0, sizeof(*flash));
	flash->name = "nvt-sflash";
	flash->size = size;
	flash->page_size = SZ_PAGE;
	flash->sector_size = SZ_SECTOR;
	flash->erase_size = SZ_SECTOR;
	flash->erase_opcode = CMD_SE_64K;
	flash->read_opcode = CMD_READ;
	flash->program_opcode = CMD_PP;

	mtd->name = "nvt-sflash";
	mtd->type = MTD_NORFLASH;
	mtd->flags = MTD_CAP_NORFLASH;
	mtd->size = size;
	mtd->erasesize = SZ_SECTOR;
	mtd->writesize = 1;
	mtd->writebufsize = SZ_PAGE;
	mtd->_read = nvt_sf_read;
	mtd->_write = nvt_sf_write;
	mtd->_erase = nvt_sf_erase;
	mtd->priv = flash;

	/* Once: the environment and common/openipc.c probe again and again */
	if (!nvt_announced++)
		printf("SF: Detected JEDEC %06x with page size %u Bytes, erase size %u KiB, total %u MiB\n",
		       jedec, SZ_PAGE, SZ_SECTOR >> 10, size >> 20);
	return flash;
}

void spi_flash_free(struct spi_flash *flash)
{
}
