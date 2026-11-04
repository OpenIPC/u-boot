// SPDX-License-Identifier: GPL-2.0+
/*
 * SPI NOR behind the Faraday FTSPI020 flash controller of the Novatek
 * NA51068 (NT98321, NT98323).
 *
 * Each command is four register writes: the flash address (CMD0), its
 * length with the dummy cycles and the instruction length (CMD1), the
 * data length (CMD2), then the instruction with its mode bits (CMD3),
 * which starts it. Data goes through the 32-byte FIFO at DATA, a word at
 * a time, gated by STATUS; INT_STS bit 0 says the command is done. A
 * status read either returns one SR byte in READ_STS, or, polled by the
 * controller, ends only once the busy bit (CTRL RD_STS_LOC, bit 0) is
 * clear, which is how a program or an erase is waited for.
 *
 * After the vendor U-Boot 2016.07 of the NT9832x SDK
 * (drivers/mtd/nvt_flash_spi/nvt_flash_spi020*.c, spiflash/spi020_nor.c),
 * single lane and PIO, as it runs (its DMA and FIFO checksum paths are
 * compiled out). Like nvt_sflash, this driver provides spi_flash_probe()
 * itself, so mainline's non-DM spi_flash_*() wrappers, the sf command and
 * the environment in SPI flash work without the SPI and spi-nor layers.
 * Parts over 16 MiB take the 4-byte address opcodes, as the vendor's do.
 */

#include <common.h>
#include <malloc.h>
#include <spi_flash.h>
#include <time.h>
#include <asm/io.h>
#include <asm/unaligned.h>
#include <asm/arch/na51068.h>
#include <linux/delay.h>
#include <linux/libfdt.h>
#include <linux/mtd/mtd.h>
#include <linux/sizes.h>

#define FC(off)			(NVT_FLASH_BASE + (off))
#define FC_CMD0			FC(0x00)	/* flash address */
#define FC_CMD1			FC(0x04)
#define CMD1_ALEN(n)		(n)
#define CMD1_DUMMY(n)		((n) << 16)
#define CMD1_INST_LEN1		(1 << 24)
#define FC_CMD2			FC(0x08)	/* data length */
#define FC_CMD3			FC(0x0C)	/* writing it starts the command */
#define CMD3_WRITE		BIT(1)
#define CMD3_RD_STS		BIT(2)
#define CMD3_RD_STS_SW		BIT(3)		/* one read, else polled */
#define CMD3_INST(c)		((u32)(c) << 24)
#define FC_CTRL			FC(0x10)
#define CTRL_CLK_DIV(n)		(n)		/* /2, /4, /6, /8 */
#define FC_STATUS		FC(0x18)
#define STATUS_TXFIFO_EMPTY	BIT(0)
#define STATUS_RXFIFO_READY	BIT(1)
#define FC_INT_CTRL		FC(0x20)
#define INT_CTRL_DMA_EN		BIT(0)
#define FC_INT_STS		FC(0x24)
#define INT_STS_DONE		BIT(0)
#define FC_READ_STS		FC(0x28)
#define FC_DLY			FC(0x60)
#define DLY_RX_SAMPLE_EDGE	BIT(17)
#define FC_DATA			FC(0x100)
#define FC_FIFO			32

/* CG: the controller's reset and clock gate */
#define CG_FLASH_RSTN		(NVT_CG_BASE + 0x54)
#define CG_FLASH_RSTN_BIT	BIT(11)
#define CG_FLASH_GATE		(NVT_CG_BASE + 0x68)
#define CG_FLASH_GATE_BIT	BIT(9)

#define SZ_PAGE			256
#define SZ_SECTOR		SZ_64K

#define CMD_WRSR		0x01
#define CMD_PP			0x02
#define CMD_READ		0x03
#define CMD_WRDI		0x04
#define CMD_RDSR		0x05
#define CMD_WREN		0x06
#define CMD_PP_4B		0x12
#define CMD_READ_4B		0x13
#define CMD_RDSR2		0x35
#define CMD_RDID		0x9F
#define CMD_SE_64K		0xD8
#define CMD_SE_64K_4B		0xDC

#define SR1_WEL			BIT(1)
#define SR1_BP			0x7C	/* GigaDevice/Winbond and alikes: BP0-4 */
#define SR1_BP_OTHER		0x3C	/* BP0-3 */

#define MFR_GIGADEVICE		0xc8
#define MFR_WINBOND		0xef
#define MFR_XMC			0x20
#define MFR_ZBIT		0x5e
#define MFR_BOYA		0x68

enum { XFER_NORMAL, XFER_STATUS, XFER_POLL };

static struct {
	struct spi_flash flash;
	u32 ctrl;		/* the clock divider */
	u8 alen;		/* 3, or 4 past 16 MiB */
	bool protect;		/* block protection stays: refuse writes */
	bool announced;
} sf;

static int fc_wait(ulong reg, u32 mask, ulong ms)
{
	ulong start = get_timer(0);

	while (!(readl(reg) & mask)) {
		if (get_timer(start) > ms)
			return -ETIMEDOUT;
	}
	return 0;
}

static int fc_pio_in(u8 *buf, unsigned int len)
{
	while (len) {
		unsigned int n = min(len, (unsigned int)FC_FIFO);

		if (fc_wait(FC_STATUS, STATUS_RXFIFO_READY, 1000))
			return -ETIMEDOUT;
		len -= n;
		while (n) {
			u32 w = readl(FC_DATA);
			unsigned int i, k = min(n, 4u);

			for (i = 0; i < k; i++, w >>= 8)
				*buf++ = w;
			n -= k;
		}
	}
	return 0;
}

/*
 * Whole words: the caller pads the data to them. The controller sends
 * only the data length (CMD2) of what the last word holds.
 */
static int fc_pio_out(const u8 *buf, unsigned int len)
{
	len = ALIGN(len, 4);
	while (len) {
		unsigned int n = min(len, (unsigned int)FC_FIFO);

		if (fc_wait(FC_STATUS, STATUS_TXFIFO_EMPTY, 1000))
			return -ETIMEDOUT;
		len -= n;
		for (; n; n -= 4, buf += 4)
			writel(get_unaligned_le32(buf), FC_DATA);
	}
	return 0;
}

/*
 * One command: @alen address bytes, @len data bytes in (@out NULL) or
 * out; XFER_STATUS returns SR1 in *@sts, XFER_POLL waits until the flash
 * is no longer busy, for up to @ms.
 */
static int fc_xfer(u8 cmd, u32 addr, unsigned int alen, void *in,
		   const void *out, unsigned int len, int mode, u8 *sts,
		   ulong ms)
{
	u32 cmd3 = CMD3_INST(cmd);
	int ret = 0;

	writel(sf.ctrl, FC_CTRL);
	writel(DLY_RX_SAMPLE_EDGE, FC_DLY);
	if (alen)
		writel(addr, FC_CMD0);
	if (mode == XFER_NORMAL) {
		writel(len, FC_CMD2);
		if (!in)
			cmd3 |= CMD3_WRITE;
	} else {
		writel(0, FC_CMD2);
		cmd3 |= CMD3_RD_STS | (mode == XFER_STATUS ? CMD3_RD_STS_SW : 0);
	}
	writel(CMD1_ALEN(alen) | CMD1_INST_LEN1, FC_CMD1);
	writel(cmd3, FC_CMD3);

	if (len && mode == XFER_NORMAL)
		ret = in ? fc_pio_in(in, len) : fc_pio_out(out, len);
	if (!ret)
		ret = fc_wait(FC_INT_STS, INT_STS_DONE, ms);
	writel(INT_STS_DONE, FC_INT_STS);
	if (ret)
		printf("SF: command %02x timed out\n", cmd);
	else if (sts)
		*sts = readl(FC_READ_STS);
	return ret;
}

/* A status register, or the transfer's error: *v is left alone on one */
static int sf_read_reg(u8 cmd, u8 *v)
{
	return fc_xfer(cmd, 0, 0, NULL, NULL, 0, XFER_STATUS, v, 100);
}

/* Until the flash is no longer busy: the controller polls SR1 */
static int sf_wait_ready(ulong ms)
{
	return fc_xfer(CMD_RDSR, 0, 0, NULL, NULL, 0, XFER_POLL, NULL, ms);
}

static int sf_write_enable(void)
{
	int ret = fc_xfer(CMD_WREN, 0, 0, NULL, NULL, 0, XFER_NORMAL, NULL, 100);
	u8 sr;

	if (!ret)
		ret = sf_read_reg(CMD_RDSR, &sr);
	if (ret)
		return ret;
	if (!(sr & SR1_WEL)) {
		printf("SF: write enable does not stick\n");
		return -EIO;
	}
	return 0;
}

static int sf_write_sr(const u8 *sr, unsigned int len)
{
	u8 buf[4] = { sr[0], len > 1 ? sr[1] : 0xff, 0xff, 0xff };
	int ret = sf_write_enable();

	if (!ret)
		ret = fc_xfer(CMD_WRSR, 0, 0, NULL, buf, len, XFER_NORMAL,
			      NULL, 100);
	return ret ? ret : sf_wait_ready(1000);
}

/*
 * Clear the block protection a vendor loader or firmware may have left
 * set, and check that it went: a program or erase of a protected block
 * is accepted and dropped, which would pass for success.
 */
static int sf_unprotect(u8 mfr)
{
	u8 sr[2];
	u8 bp = SR1_BP_OTHER;
	int ret = sf_read_reg(CMD_RDSR, &sr[0]);

	if (ret)
		return ret;
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
		ret = sf_read_reg(CMD_RDSR2, &sr[1]);
		if (!ret)
			ret = sf_write_sr(sr, 2);
	} else {
		ret = sf_write_sr(sr, 1);
	}
	if (!ret)
		ret = sf_read_reg(CMD_RDSR, &sr[0]);
	if (ret || sr[0] & bp) {
		printf("SF: block protection stays (SR1 %02x): writes refused\n",
		       sr[0]);
		return -EROFS;
	}
	return 0;
}

static int nvt_sf_read(struct mtd_info *mtd, loff_t from, size_t len,
		       size_t *retlen, u_char *buf)
{
	int ret = 0;

	*retlen = 0;
	if (from > mtd->size || len > mtd->size - from)
		return -EINVAL;
	/* In pieces, so that a timeout stays a timeout */
	while (len && !ret) {
		unsigned int n = min_t(size_t, len, SZ_1M);

		ret = fc_xfer(sf.alen == 4 ? CMD_READ_4B : CMD_READ, from,
			      sf.alen, buf, NULL, n, XFER_NORMAL, NULL, 1000);
		from += n;
		buf += n;
		len -= n;
		if (!ret)
			*retlen += n;
	}
	return ret;
}

/* PAGE PROGRAM within a page, from a copy padded to whole words */
static int sf_program(u32 addr, const u8 *buf, unsigned int len)
{
	u8 page[SZ_PAGE + 4] __aligned(4);
	int ret;

	memcpy(page, buf, len);
	memset(page + len, 0xff, 4);
	ret = sf_write_enable();
	if (!ret)
		ret = fc_xfer(sf.alen == 4 ? CMD_PP_4B : CMD_PP, addr, sf.alen,
			      NULL, page, len, XFER_NORMAL, NULL, 100);
	return ret ? ret : sf_wait_ready(100);
}

static int nvt_sf_write(struct mtd_info *mtd, loff_t to, size_t len,
			size_t *retlen, const u_char *buf)
{
	int ret;

	*retlen = 0;
	if (to > mtd->size || len > mtd->size - to)
		return -EINVAL;
	if (sf.protect)
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

static int nvt_sf_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	u32 addr, end;
	int ret;

	/* Checked in 64 bits, before anything can wrap */
	if (instr->addr % SZ_SECTOR || instr->len % SZ_SECTOR ||
	    instr->addr > mtd->size || instr->len > mtd->size - instr->addr)
		return -EINVAL;
	if (sf.protect) {
		instr->state = MTD_ERASE_FAILED;
		return -EROFS;
	}
	instr->state = MTD_ERASING;
	addr = instr->addr;
	end = instr->addr + instr->len;
	for (; addr < end; addr += SZ_SECTOR) {
		ret = sf_write_enable();
		if (!ret)
			ret = fc_xfer(sf.alen == 4 ? CMD_SE_64K_4B : CMD_SE_64K,
				      addr, sf.alen, NULL, NULL, 0, XFER_NORMAL,
				      NULL, 100);
		/* A 64 KiB erase takes up to 2 s on the usual parts */
		if (!ret)
			ret = sf_wait_ready(5000);
		if (ret) {
			instr->fail_addr = addr;
			instr->state = MTD_ERASE_FAILED;
			return ret;
		}
	}
	instr->state = MTD_ERASE_DONE;
	return 0;
}

/*
 * The controller's clock and reset, as the vendor's nvt_spinor_reset();
 * the pins are the loader's, which booted from this flash. The clock
 * divider follows the loader's device tree, /nor@fa900000
 * clock-frequency, 48 MHz on the SDK's boards.
 */
static void fc_hw_init(void)
{
	const void *fdt = nvt_loader_fdt();
	const fdt32_t *val = NULL;
	u32 hz = 12000000;
	int node;

	if (fdt) {
		node = fdt_path_offset(fdt, "/nor@fa900000");
		if (node >= 0)
			val = fdt_getprop(fdt, node, "clock-frequency", NULL);
		if (val)
			hz = fdt32_to_cpu(*val);
	}
	sf.ctrl = CTRL_CLK_DIV(hz < 25000000 ? 3 : hz < 37500000 ? 2 :
			       hz < 75000000 ? 1 : 0);

	setbits_le32(CG_FLASH_RSTN, CG_FLASH_RSTN_BIT);
	clrbits_le32(CG_FLASH_GATE, CG_FLASH_GATE_BIT);
	clrbits_le32(FC_INT_CTRL, INT_CTRL_DMA_EN);
	writel(INT_STS_DONE, FC_INT_STS);
}

struct spi_flash *spi_flash_probe(unsigned int bus, unsigned int cs,
				  unsigned int max_hz, unsigned int spi_mode)
{
	struct spi_flash *flash = &sf.flash;
	struct mtd_info *mtd = &flash->mtd;
	u8 id[4];
	u32 jedec, size;

	fc_hw_init();
	if (fc_xfer(CMD_RDID, 0, 0, id, NULL, sizeof(id), XFER_NORMAL, NULL,
		    100)) {
		printf("SF: the flash controller does not answer\n");
		return NULL;
	}
	jedec = id[0] << 16 | id[1] << 8 | id[2];
	if (!jedec || jedec == 0xffffff) {
		printf("SF: no flash (JEDEC id %06x)\n", jedec);
		return NULL;
	}
	/*
	 * The last ID byte is log2 of the size, as the vendor driver takes it
	 * (trace-stdtable in the SDK's trees)
	 */
	if (id[2] >= 0x10 && id[2] <= 0x1c) {
		size = 1 << id[2];
	} else {
		size = SZ_8M;
		if (!sf.announced)
			printf("SF: JEDEC %06x: size unknown, taking 8 MiB\n", jedec);
	}
	sf.alen = size > SZ_16M ? 4 : 3;
	sf.protect = sf_unprotect(id[0]) < 0;

	memset(flash, 0, sizeof(*flash));
	flash->name = "nvt-spi020";
	flash->size = size;
	flash->page_size = SZ_PAGE;
	flash->sector_size = SZ_SECTOR;
	flash->erase_size = SZ_SECTOR;
	flash->erase_opcode = sf.alen == 4 ? CMD_SE_64K_4B : CMD_SE_64K;
	flash->read_opcode = sf.alen == 4 ? CMD_READ_4B : CMD_READ;
	flash->program_opcode = sf.alen == 4 ? CMD_PP_4B : CMD_PP;

	mtd->name = "nvt-spi020";
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
	if (!sf.announced) {
		sf.announced = true;
		printf("SF: Detected JEDEC %06x with page size %u Bytes, erase size %u KiB, total %u MiB\n",
		       jedec, SZ_PAGE, SZ_SECTOR >> 10, size >> 20);
	}
	return flash;
}

void spi_flash_free(struct spi_flash *flash)
{
}
