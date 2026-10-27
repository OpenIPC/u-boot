// SPDX-License-Identifier: GPL-2.0+
/*
 * SPI NOR behind the FSP of the SigmaStar infinity3.
 *
 * The FSP ("flash serial programmer") runs up to three flash commands in
 * one go from a 10-byte write buffer: sizes 0-2 say how many of its bytes
 * each command sends, read sizes 0-2 how many bytes each one reads back
 * into a 10-byte read buffer. With the status check on, the command picked
 * as RDSR is repeated until the flash's busy bit clears, and only then
 * does the FSP report done. A page program takes its data from BDMA
 * instead ("outside" write buffer): BDMA copies the page from DDR to the
 * FSP, which sends it in place of write-buffer byte 5.
 *
 * Reads go through the XIU window, the flash mapped at 0x14000000 in the
 * read mode the IPL set up, as the vendor kernel reads it.
 *
 * After the vendor U-Boot and kernel drivers/mstar/flash_isp (infinity3
 * halSERFLASH.c: FSP_READ_DIRECT, FSP_WRITE_BDMA). Like ak_sflash on
 * openipc-anyka, this driver provides spi_flash_probe() itself, so
 * mainline's non-DM spi_flash_*() wrappers, the sf command and the
 * environment in SPI flash work without the SPI and spi-nor layers.
 * 3-byte addresses: parts up to 16 MiB.
 */

#include <common.h>
#include <spi_flash.h>
#include <time.h>
#include <watchdog.h>
#include <asm/io.h>
#include <asm/arch/sigmastar.h>
#include <linux/delay.h>
#include <linux/mtd/mtd.h>
#include <linux/sizes.h>

#define FSP_REG(r)		SS_REG(SS_BANK_FSP, r)
#define FSP_WBF(i)		FSP_REG(0x60 + (i) / 2)	/* bytes 0-9 */
#define FSP_RBF(i)		FSP_REG(0x65 + (i) / 2)	/* bytes 0-9 */
#define FSP_WBF_SIZE		FSP_REG(0x6a)
#define FSP_RBF_SIZE		FSP_REG(0x6b)
#define FSP_CTRL		FSP_REG(0x6c)
#define FSP_TRIGGER		FSP_REG(0x6d)
#define FSP_DONE		FSP_REG(0x6e)
#define FSP_DONE_CLR		FSP_REG(0x6f)
#define FSP_OUT_SIZE		FSP_REG(0x78)
#define FSP_OUT			FSP_REG(0x79)

#define FSP_SIZES(a, b, c)	((a) | (b) << 4 | (c) << 8)

/* CTRL */
#define CTRL_EN			BIT(0)
#define CTRL_NRESET		BIT(1)
#define CTRL_INT		BIT(2)
#define CTRL_RDSR(n)		((n) << 11)	/* which command polls status */
#define CTRL_FSCHK		BIT(13)		/* poll it until not busy */
#define CTRL_CMD3		BIT(14)
#define CTRL_CMD2		BIT(15)
#define CTRL_BASE		(CTRL_EN | CTRL_NRESET | CTRL_INT)

/* OUT: write-buffer byte 5 is replaced by OUT_SIZE bytes from BDMA */
#define OUT_REPLACE(i)		(i)
#define OUT_MODE_BDMA		(1 << 8)
#define OUT_EN			BIT(12)

/* BDMA channel 0 */
#define BDMA_REG(r)		SS_REG(SS_BANK_BDMA, r)
#define BDMA_CTRL		BDMA_REG(0x00)
#define BDMA_STATUS		BDMA_REG(0x01)
#define BDMA_PATH		BDMA_REG(0x02)
#define BDMA_SRC_L		BDMA_REG(0x04)
#define BDMA_SRC_H		BDMA_REG(0x05)
#define BDMA_DST_L		BDMA_REG(0x06)
#define BDMA_DST_H		BDMA_REG(0x07)
#define BDMA_LEN_L		BDMA_REG(0x08)
#define BDMA_LEN_H		BDMA_REG(0x09)
#define BDMA_TRIGGER		BIT(0)
#define BDMA_DONE		BIT(3)
#define BDMA_PATH_MIU_FSP	0x2B40	/* MIU, 16 bytes wide, to the FSP */

#define SZ_PAGE			256
#define SZ_SECTOR		SZ_64K

#define CMD_WRSR		0x01
#define CMD_PP			0x02
#define CMD_RDSR		0x05
#define CMD_WREN		0x06
#define CMD_RDID		0x9F
#define CMD_SE_64K		0xD8

#define SR_WIP			BIT(0)
#define SR_BP			0x3C
#define SR_SRWD			BIT(7)

/* Longer than any 64 KiB erase or page program of the parts in use */
#define SF_TIMEOUT_MS		3000

static int fsp_wait(void)
{
	ulong start = get_timer(0);
	int ret = 0;

	while (!(readw(FSP_DONE) & BIT(0))) {
		if (get_timer(start) > SF_TIMEOUT_MS) {
			ret = -ETIMEDOUT;
			break;
		}
	}
	writew(BIT(0), FSP_DONE_CLR);
	return ret;
}

/*
 * One FSP run: wlen bytes of wbuf, split between the commands by wsizes,
 * and rbuf filled with what they read back (rsizes)
 */
static int fsp_run(const u8 *wbuf, unsigned int wlen, u16 wsizes, u16 rsizes,
		   u16 ctrl, u8 *rbuf, unsigned int rlen)
{
	unsigned int i;
	int ret;

	for (i = 0; i < wlen; i += 2)
		writew(wbuf[i] | (i + 1 < wlen ? wbuf[i + 1] << 8 : 0),
		       FSP_WBF(i));
	writew(wsizes, FSP_WBF_SIZE);
	writew(rsizes, FSP_RBF_SIZE);
	writew(CTRL_BASE | ctrl, FSP_CTRL);
	writew(BIT(0), FSP_TRIGGER);
	ret = fsp_wait();
	for (i = 0; !ret && i < rlen; i++)
		rbuf[i] = readw(FSP_RBF(i)) >> (i & 1 ? 8 : 0);
	return ret;
}

/* A command alone, reading len (up to 10) bytes back */
static int sf_read_cmd(u8 op, u8 *buf, unsigned int len)
{
	return fsp_run(&op, 1, FSP_SIZES(1, 0, 0), FSP_SIZES(len, 0, 0), 0,
		       buf, len);
}

static int sf_wait_ready(void)
{
	ulong start = get_timer(0);
	u8 sr;

	do {
		if (sf_read_cmd(CMD_RDSR, &sr, 1))
			return -EIO;
		if (!(sr & SR_WIP))
			return 0;
		WATCHDOG_RESET();
	} while (get_timer(start) < SF_TIMEOUT_MS);
	return -ETIMEDOUT;
}

/*
 * WREN, the command with its address, RDSR polled to the end: an erase,
 * and a page program whose data comes from BDMA (the dummy byte 5 is
 * replaced by it)
 */
static int sf_wren_cmd(u8 op, u32 addr, bool bdma)
{
	u8 wbuf[7] = { CMD_WREN, op, addr >> 16, addr >> 8, addr,
		       bdma ? 0 : CMD_RDSR, CMD_RDSR };

	return fsp_run(wbuf, bdma ? 7 : 6,
		       FSP_SIZES(1, bdma ? 5 : 4, 1), FSP_SIZES(0, 0, 1),
		       CTRL_CMD2 | CTRL_CMD3 | CTRL_RDSR(2) | CTRL_FSCHK,
		       NULL, 0);
}

static int ms_sf_read(struct mtd_info *mtd, loff_t from, size_t len,
		      size_t *retlen, u_char *buf)
{
	if (from > mtd->size || len > mtd->size - from)
		return -EINVAL;
	memcpy(buf, (void *)(SS_XIU_FLASH_BASE + (ulong)from), len);
	*retlen = len;
	return 0;
}

/* BDMA reads its source from DDR: a page goes through here */
static u8 sf_page[SZ_PAGE] __aligned(64);

static int sf_program_page(u32 to, const u8 *buf, unsigned int len)
{
	ulong start;
	int ret;

	memcpy(sf_page, buf, len);
	ss_miu_flush();

	writew(0, BDMA_CTRL);
	writew(BDMA_PATH_MIU_FSP, BDMA_PATH);
	writew(SS_MIU_ADDR(sf_page) & 0xffff, BDMA_SRC_L);
	writew(SS_MIU_ADDR(sf_page) >> 16 & 0x0fff, BDMA_SRC_H);
	writew(0, BDMA_DST_L);
	writew(0, BDMA_DST_H);
	writew(len, BDMA_LEN_L);
	writew(0, BDMA_LEN_H);
	writew(BDMA_TRIGGER, BDMA_CTRL);

	writew(len, FSP_OUT_SIZE);
	writew(OUT_REPLACE(5) | OUT_MODE_BDMA | OUT_EN, FSP_OUT);
	ret = sf_wren_cmd(CMD_PP, to, true);

	start = get_timer(0);
	while (!(readw(BDMA_STATUS) & BDMA_DONE)) {
		if (get_timer(start) > SF_TIMEOUT_MS) {
			ret = -ETIMEDOUT;
			break;
		}
	}
	writew(BDMA_DONE, BDMA_STATUS);
	writew(0, FSP_OUT_SIZE);
	writew(0, FSP_OUT);
	return ret ? ret : sf_wait_ready();
}

static int ms_sf_write(struct mtd_info *mtd, loff_t to, size_t len,
		       size_t *retlen, const u_char *buf)
{
	int ret;

	if (to > mtd->size || len > mtd->size - to)
		return -EINVAL;
	*retlen = 0;
	while (len) {
		unsigned int n = min_t(size_t, len,
				       SZ_PAGE - (to & (SZ_PAGE - 1)));

		ret = sf_program_page(to, buf, n);
		if (ret)
			return ret;
		to += n;
		buf += n;
		len -= n;
		*retlen += n;
	}
	return 0;
}

static int ms_sf_erase(struct mtd_info *mtd, struct erase_info *instr)
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
		ret = sf_wren_cmd(CMD_SE_64K, addr, false);
		if (!ret)
			ret = sf_wait_ready();
		if (ret) {
			instr->fail_addr = addr;
			return ret;
		}
		WATCHDOG_RESET();
	}
	return 0;
}

/*
 * Block protection left on by a vendor loader: cleared, status register 1
 * only. Not written when clear, so status register 2 (the quad enable bit
 * the IPL's read mode may rely on) is left alone.
 */
static void sf_unprotect(void)
{
	u8 wbuf[4] = { CMD_WREN, CMD_WRSR, 0, CMD_RDSR };
	u8 sr;

	if (sf_read_cmd(CMD_RDSR, &sr, 1) || !(sr & (SR_BP | SR_SRWD)))
		return;
	if (fsp_run(wbuf, 4, FSP_SIZES(1, 2, 1), FSP_SIZES(0, 0, 1),
		    CTRL_CMD2 | CTRL_CMD3 | CTRL_RDSR(2) | CTRL_FSCHK, NULL, 0) ||
	    sf_wait_ready())
		printf("SF: could not clear the block protection (SR %02x)\n",
		       sr);
}

static struct spi_flash ms_flash;
static int ms_announced;

struct spi_flash *spi_flash_probe(unsigned int bus, unsigned int cs,
				  unsigned int max_hz, unsigned int spi_mode)
{
	struct spi_flash *flash = &ms_flash;
	struct mtd_info *mtd = &flash->mtd;
	u8 id[3];
	u32 jedec, size;

	if (sf_read_cmd(CMD_RDID, id, 3)) {
		printf("SF: the FSP does not answer\n");
		return NULL;
	}
	jedec = id[0] << 16 | id[1] << 8 | id[2];
	if (!jedec || jedec == 0xffffff) {
		printf("SF: no flash (JEDEC id %06x)\n", jedec);
		return NULL;
	}
	/*
	 * The last ID byte is log2 of the size on the parts these boards
	 * carry; the XIU window and 3-byte addresses end at 16 MiB
	 */
	size = id[2] >= 0x14 && id[2] <= 0x18 ? 1 << id[2] : SZ_8M;

	sf_unprotect();

	memset(flash, 0, sizeof(*flash));
	flash->name = "ms-sflash";
	flash->size = size;
	flash->page_size = SZ_PAGE;
	flash->sector_size = SZ_SECTOR;
	flash->erase_size = SZ_SECTOR;
	flash->erase_opcode = CMD_SE_64K;
	flash->program_opcode = CMD_PP;

	mtd->name = "ms-sflash";
	mtd->type = MTD_NORFLASH;
	mtd->flags = MTD_CAP_NORFLASH;
	mtd->size = size;
	mtd->erasesize = SZ_SECTOR;
	mtd->writesize = 1;
	mtd->writebufsize = SZ_PAGE;
	mtd->_read = ms_sf_read;
	mtd->_write = ms_sf_write;
	mtd->_erase = ms_sf_erase;
	mtd->priv = flash;

	/* Once: the environment and common/openipc.c probe again and again */
	if (!ms_announced++)
		printf("SF: Detected JEDEC %06x with page size %u Bytes, erase size %u KiB, total %u MiB\n",
		       jedec, SZ_PAGE, SZ_SECTOR >> 10, size >> 20);
	return flash;
}

void spi_flash_free(struct spi_flash *flash)
{
}
