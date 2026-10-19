// SPDX-License-Identifier: GPL-2.0+
/*
 * SPI NOR behind SPI0 of the Anyka AK39 SoCs.
 *
 * SPI0 is a byte-wise PIO master. CON selects the chip and the direction:
 * with ARRM set it only sends (what comes back is dropped), with TGDM set it
 * only receives (it clocks out filler). Writing CNT starts a transfer of
 * that many bytes; they go through 8-byte FIFOs, OUT and IN, a
 * little-endian word at a time, and STA says when OUT has room, when IN
 * holds a word and when the count is done. Opcode and address are plain TX
 * bytes.
 *
 * After the vendor U-Boot's drivers/spi/ak39_spi.c (PIO path) and
 * spi_flash code. Like gk_sflash on openipc-gk710x, this driver provides
 * spi_flash_probe() itself, so mainline's non-DM spi_flash_*() wrappers,
 * the sf command and the environment in SPI flash work without the SPI and
 * spi-nor layers. Single lane, 3-byte addresses: parts up to 16 MiB.
 */

#include <common.h>
#include <spi_flash.h>
#include <watchdog.h>
#include <asm/io.h>
#include <asm/arch/anyka.h>
#include <linux/delay.h>
#include <linux/mtd/mtd.h>
#include <linux/sizes.h>

#define SPI_CON			(AK_SPI0_BASE + 0x00)
#define SPI_STA			(AK_SPI0_BASE + 0x04)
#define SPI_INT			(AK_SPI0_BASE + 0x08)
#define SPI_CNT			(AK_SPI0_BASE + 0x0c)
#define SPI_OUT			(AK_SPI0_BASE + 0x18)
#define SPI_IN			(AK_SPI0_BASE + 0x1c)

/* CON */
#define CON_TGDM		BIT(0)	/* receive only */
#define CON_ARRM		BIT(1)	/* send only */
#define CON_MASTER		BIT(4)
#define CON_CS			BIT(5)	/* chip selected */
#define CON_EN			BIT(6)
#define CON_DIV(d)		((d) << 8)
/* STA */
#define STA_TXHEMP		BIT(2)
#define STA_RXHFULL		BIT(6)
#define STA_TRANSF		BIT(8)

#define SPI_HZ			20000000
#define CON_DIV_MAX		0x7f
/* One CNT transfer, at most: the vendor U-Boot's MAX_XFER_LEN */
#define SPI_CHUNK		0x2000
/* Polls of STA before a transfer is given up */
#define SPI_SPINS		10000000

#define SZ_PAGE			256
#define SZ_SECTOR		SZ_64K

#define CMD_WRSR		0x01
#define CMD_PP			0x02
#define CMD_FAST_READ		0x0B
#define CMD_RDSR		0x05
#define CMD_WREN		0x06
#define CMD_RDSR2		0x35
#define CMD_RDID		0x9F
#define CMD_SE_64K		0xD8

#define SR1_WIP			BIT(0)
#define SR1_WEL			BIT(1)
#define SR1_BP			0x7C	/* GigaDevice/Winbond and alikes: BP0-4 */
#define SR1_BP_OTHER		0x3C	/* BP0-3 */

/* Longer than any erase or program of the parts on these boards */
#define SF_TIMEOUT_MS		3000

#define MFR_GIGADEVICE		0xc8
#define MFR_WINBOND		0xef
#define MFR_XMC			0x20
#define MFR_ZBIT		0x5e
#define MFR_BOYA		0x68

/* CON without the direction bits; set at probe */
static u32 spi_con = CON_EN | CON_MASTER;

static void spi_select(bool on)
{
	writel(on ? spi_con | CON_CS : spi_con, SPI_CON);
}

static int spi_wait(u32 bit)
{
	unsigned int spins = SPI_SPINS;

	while (!(readl(SPI_STA) & bit))
		if (!--spins)
			return -ETIMEDOUT;
	return 0;
}

static int spi_send(const u8 *buf, unsigned int len)
{
	writel(spi_con | CON_CS | CON_ARRM, SPI_CON);
	while (len) {
		unsigned int n = min_t(unsigned int, len, SPI_CHUNK);
		unsigned int i;

		writel(n, SPI_CNT);
		for (i = 0; i < n; i += 4) {
			unsigned int k = min(n - i, 4U);
			u32 w = 0;

			if (spi_wait(STA_TXHEMP))
				return -ETIMEDOUT;
			memcpy(&w, buf + i, k);
			writel(le32_to_cpu(w), SPI_OUT);
		}
		if (spi_wait(STA_TRANSF))
			return -ETIMEDOUT;
		buf += n;
		len -= n;
	}
	return 0;
}

static int spi_recv(u8 *buf, unsigned int len)
{
	writel(spi_con | CON_CS | CON_TGDM, SPI_CON);
	while (len) {
		unsigned int n = min_t(unsigned int, len, SPI_CHUNK);
		unsigned int got = 0;
		unsigned int spins = SPI_SPINS;

		writel(n, SPI_CNT);
		while (got < n) {
			u32 st = readl(SPI_STA);

			/* At the end, the last bytes may be less than a word */
			if (st & (STA_RXHFULL | STA_TRANSF)) {
				u32 w = cpu_to_le32(readl(SPI_IN));
				unsigned int k = min(n - got, 4U);

				memcpy(buf + got, &w, k);
				got += k;
				spins = SPI_SPINS;
			} else if (!--spins) {
				return -ETIMEDOUT;
			}
		}
		buf += n;
		len -= n;
	}
	return 0;
}

/*
 * One flash command: opcode and address out (and for a fast read the dummy
 * byte), then data out or in
 */
static int sf_cmd(u8 op, int addr, const u8 *out, u8 *in, unsigned int len)
{
	u8 cmd[5] = { op, addr >> 16, addr >> 8, addr, 0 };
	int ret;

	spi_select(true);
	ret = spi_send(cmd, addr < 0 ? 1 : op == CMD_FAST_READ ? 5 : 4);
	if (!ret && out && len)
		ret = spi_send(out, len);
	if (!ret && in && len)
		ret = spi_recv(in, len);
	spi_select(false);
	return ret;
}

static u8 sf_read_reg(u8 op)
{
	u8 v = 0xff;

	sf_cmd(op, -1, NULL, &v, 1);
	return v;
}

static int sf_wait_ready(void)
{
	ulong start = get_timer(0);

	while (sf_read_reg(CMD_RDSR) & SR1_WIP) {
		if (get_timer(start) > SF_TIMEOUT_MS) {
			printf("SF: timed out waiting for the flash\n");
			return -ETIMEDOUT;
		}
		WATCHDOG_RESET();
	}
	return 0;
}

/*
 * A program or erase the flash did not accept leaves it idle, which the
 * ready wait would take for success: make sure write enable latched.
 */
static int sf_write_enable(void)
{
	int ret = sf_cmd(CMD_WREN, -1, NULL, NULL, 0);

	if (ret)
		return ret;
	if (!(sf_read_reg(CMD_RDSR) & SR1_WEL)) {
		printf("SF: write enable did not latch\n");
		return -EIO;
	}
	return 0;
}

static int sf_write_sr(const u8 *sr, unsigned int len)
{
	int ret = sf_write_enable();

	if (!ret)
		ret = sf_cmd(CMD_WRSR, -1, sr, NULL, len);
	return ret ? ret : sf_wait_ready();
}

/* Clear the block protection a vendor loader may have left set */
static void sf_unprotect(u8 mfr)
{
	u8 sr[2] = { sf_read_reg(CMD_RDSR) };

	if (sr[0] == 0xff)
		return;
	if (mfr == MFR_GIGADEVICE || mfr == MFR_WINBOND || mfr == MFR_XMC ||
	    mfr == MFR_ZBIT || mfr == MFR_BOYA) {
		/* QE lives in SR2, written as WRSR's second byte: keep it */
		if (sr[0] & SR1_BP) {
			printf("SF: block protection (SR1 %02x), clearing\n", sr[0]);
			sr[0] &= ~SR1_BP;
			sr[1] = sf_read_reg(CMD_RDSR2);
			sf_write_sr(sr, 2);
		}
	} else if (sr[0] & SR1_BP_OTHER) {
		printf("SF: block protection (SR1 %02x), clearing\n", sr[0]);
		sr[0] &= ~SR1_BP_OTHER;
		sf_write_sr(sr, 1);
	}
}

static int ak_sf_read(struct mtd_info *mtd, loff_t from, size_t len,
		      size_t *retlen, u_char *buf)
{
	int ret;

	if (from > mtd->size || len > mtd->size - from)
		return -EINVAL;
	ret = sf_cmd(CMD_FAST_READ, from, NULL, buf, len);
	*retlen = ret ? 0 : len;
	return ret;
}

static int ak_sf_write(struct mtd_info *mtd, loff_t to, size_t len,
		       size_t *retlen, const u_char *buf)
{
	int ret;

	if (to > mtd->size || len > mtd->size - to)
		return -EINVAL;
	*retlen = 0;
	while (len) {
		unsigned int n = min_t(size_t, len,
				       SZ_PAGE - (to & (SZ_PAGE - 1)));

		ret = sf_write_enable();
		if (!ret)
			ret = sf_cmd(CMD_PP, to, buf, NULL, n);
		if (!ret)
			ret = sf_wait_ready();
		if (ret)
			return ret;
		to += n;
		buf += n;
		len -= n;
		*retlen += n;
	}
	return 0;
}

static int ak_sf_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	u32 addr, end;
	int ret;

	/* Checked in 64 bits, before anything can wrap */
	if (instr->addr % SZ_SECTOR || instr->len % SZ_SECTOR ||
	    instr->addr > mtd->size || instr->len > mtd->size - instr->addr)
		return -EINVAL;
	addr = instr->addr;
	end = instr->addr + instr->len;
	for (; addr < end; addr += SZ_SECTOR) {
		ret = sf_write_enable();
		if (!ret)
			ret = sf_cmd(CMD_SE_64K, addr, NULL, NULL, 0);
		if (!ret)
			ret = sf_wait_ready();
		if (ret) {
			instr->fail_addr = addr;
			return ret;
		}
	}
	return 0;
}

/*
 * SPI0's pads, as the vendor U-Boots set them before each use; the boot ROM
 * leaves enough for reading the boot image. The EV330's are left as the
 * ROM has them.
 */
#if defined(CONFIG_AK39_EV200)
#define SPI0_SHAREPIN0		BIT(25)
#define SPI0_SHAREPIN3		0x3003
#elif defined(CONFIG_AK39_EV300)
#define SPI0_SHAREPIN0		BIT(25)
#define SPI0_SHAREPIN3		(0x3 | 0x1f << 9)
#endif

static void spi_hw_init(void)
{
	setbits_le32(AK_SYS_SOFT_RST, AK_RST_SPI0);
	udelay(10);
	clrbits_le32(AK_SYS_SOFT_RST, AK_RST_SPI0);
#ifdef SPI0_SHAREPIN0
	setbits_le32(AK_SYS_SHAREPIN0, SPI0_SHAREPIN0);
	setbits_le32(AK_SYS_SHAREPIN3, SPI0_SHAREPIN3);
#endif
}

static struct spi_flash ak_flash;
static int ak_announced;

struct spi_flash *spi_flash_probe(unsigned int bus, unsigned int cs,
				  unsigned int max_hz, unsigned int spi_mode)
{
	struct spi_flash *flash = &ak_flash;
	struct mtd_info *mtd = &flash->mtd;
	unsigned long clk = ak_asic_clk();
	unsigned int div = clk ? DIV_ROUND_UP(clk, 2 * SPI_HZ) - 1 : 2;
	u8 id[3];
	u32 jedec, size;

	spi_hw_init();
	spi_con = CON_EN | CON_MASTER | CON_DIV(min(div, CON_DIV_MAX));
	spi_select(false);
	writel(0, SPI_INT);

	if (sf_cmd(CMD_RDID, -1, NULL, id, 3)) {
		printf("SF: SPI0 does not answer\n");
		return NULL;
	}
	jedec = id[0] << 16 | id[1] << 8 | id[2];
	if (!jedec || jedec == 0xffffff) {
		printf("SF: no flash (JEDEC id %06x)\n", jedec);
		return NULL;
	}
	/* The last ID byte is log2 of the size on the parts these boards carry */
	size = id[2] >= 0x14 && id[2] <= 0x18 ? 1 << id[2] : SZ_8M;

	sf_unprotect(id[0]);

	memset(flash, 0, sizeof(*flash));
	flash->name = "ak-sflash";
	flash->size = size;
	flash->page_size = SZ_PAGE;
	flash->sector_size = SZ_SECTOR;
	flash->erase_size = SZ_SECTOR;
	flash->erase_opcode = CMD_SE_64K;
	flash->read_opcode = CMD_FAST_READ;
	flash->program_opcode = CMD_PP;

	mtd->name = "ak-sflash";
	mtd->type = MTD_NORFLASH;
	mtd->flags = MTD_CAP_NORFLASH;
	mtd->size = size;
	mtd->erasesize = SZ_SECTOR;
	mtd->writesize = 1;
	mtd->writebufsize = SZ_PAGE;
	mtd->_read = ak_sf_read;
	mtd->_write = ak_sf_write;
	mtd->_erase = ak_sf_erase;
	mtd->priv = flash;

	/* Once: the environment and common/openipc.c probe again and again */
	if (!ak_announced++)
		printf("SF: Detected JEDEC %06x with page size %u Bytes, erase size %u KiB, total %u MiB\n",
		       jedec, SZ_PAGE, SZ_SECTOR >> 10, size >> 20);
	return flash;
}

void spi_flash_free(struct spi_flash *flash)
{
}
