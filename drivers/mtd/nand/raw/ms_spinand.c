// SPDX-License-Identifier: GPL-2.0+
/*
 * SPI NAND behind the FSP of the SigmaStar infinity6 family.
 *
 * After the vendor U-Boot's drivers/mstar/spinand (hal/common and the
 * per-die HAL_SPINAND_Init()). The FSP sends a flash command and reads the
 * answer back; the page cache comes over BDMA, from the flash's read-from-
 * cache in QSPI's fast read mode, and goes back the same way. The ECC is the
 * chip's own: it reports its verdict in the status register.
 *
 * Geometry comes from the SNI the ROM boots from, not from the chip's ID:
 * the published images carry one generic SNI for every part. The bad-block
 * table stays in memory, built from the marks in the spare area, since the
 * vendor's U-Boot writes none and UBI owns those last blocks.
 *
 * infinity6c differs as its NOR does: the chip select stays in software, each
 * flash command framed by hand (flash/nand/drvSPINAND.c, fsp_qspi), the cache
 * read in QSPI's normal mode and the FSP fired before BDMA.
 */

#include <common.h>
#include <malloc.h>
#include <memalign.h>
#include <nand.h>
#include <time.h>
#include <wait_bit.h>
#include <watchdog.h>
#include <asm/io.h>
#include <asm/unaligned.h>
#include <asm/arch/sigmastar.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/mtd/rawnand.h>
#include <linux/sizes.h>

#define FSP_REG(r)		SS_REG(ss_fsp_bank(), r)
#define FSP_WBF(i)		FSP_REG(0x60 + (i) / 2)
#define FSP_RBF(i)		FSP_REG(0x65 + (i) / 2)
#define FSP_WBF_SIZE		FSP_REG(0x6a)
#define FSP_RBF_SIZE		FSP_REG(0x6b)
#define FSP_CTRL		FSP_REG(0x6c)
#define FSP_TRIGGER		FSP_REG(0x6d)
#define FSP_DONE		FSP_REG(0x6e)
#define FSP_DONE_CLR		FSP_REG(0x6f)
#define FSP_CTRL2		FSP_REG(0x75)
#define FSP_CTRL4		FSP_REG(0x76)
#define FSP_OUT_SIZE		FSP_REG(0x78)
#define FSP_OUT			FSP_REG(0x79)

#define CTRL_BASE		(BIT(0) | BIT(1) | BIT(2))
#define OUT_EN			BIT(12)
#define FSP_BUF_SIZE		10

/* QSPI: the next bank up from the FSP, PM or not */
#define QSPI_REG(r)		SS_REG(ss_fsp_bank() + 1, r)
#define QSPI_BURST		QSPI_REG(0x0a)
#define QSPI_BURST_ON		0x01
#define QSPI_BURST_OFF		0x02
#define QSPI_CS_SW		BIT(0)
#define QSPI_CS_HIGH		BIT(1)
#define QSPI_TIMEOUT_H		QSPI_REG(0x67)
#define QSPI_TIMEOUT_EN		BIT(15)
#define QSPI_TIMEOUT_OFF	0x4000
#define QSPI_CKG		QSPI_REG(0x70)
#define QSPI_CKG_DUMMY		0x17		/* 8 cycles */
#define QSPI_MODE		QSPI_REG(0x72)
#define QSPI_MODE_FAST		0x01
#define QSPI_MODE_NORMAL	0x00
#define QSPI_CS_SEL		QSPI_REG(0x7a)
#define QSPI_FUNC		QSPI_REG(0x7d)
#define QSPI_FUNC_CACHE		0x1800		/* 2-byte address, dummy */

#define PM_SPI_CZ		SS_REG(SS_BANK_PMSLEEP, 0x35)
#define CHIP_ALL_PAD_IN		SS_REG(SS_BANK_CHIPTOP, 0x50)

#define BDMA_REG(r)		SS_REG(SS_BANK_BDMA, r)
#define BDMA_CTRL		BDMA_REG(0x00)
#define BDMA_STATUS		BDMA_REG(0x01)
#define BDMA_PATH		BDMA_REG(0x02)
#define BDMA_PATH2		BDMA_REG(0x03)
#define BDMA_SRC_L		BDMA_REG(0x04)
#define BDMA_SRC_H		BDMA_REG(0x05)
#define BDMA_DST_L		BDMA_REG(0x06)
#define BDMA_DST_H		BDMA_REG(0x07)
#define BDMA_LEN_L		BDMA_REG(0x08)
#define BDMA_LEN_H		BDMA_REG(0x09)
#define BDMA_TRIGGER		BIT(0)
#define BDMA_DONE		BIT(3)
#ifdef CONFIG_SS_IFORD
#define BDMA_PATH_MIU_FSP	0x2B41
#else
#define BDMA_PATH_MIU_FSP	0x2B40
#endif
#ifdef CONFIG_SS_I6C_FAMILY
#define BDMA_PATH_SPI_MIU	0x4135
#else
#define BDMA_PATH_SPI_MIU	0x4035
#endif
/* One outside-buffer run takes this much, as the vendor's driver splits it */
#define BDMA_CHUNK		2048

#define CMD_RDID		0x9F
#define CMD_WREN		0x06
#define CMD_RESET		0xFF
#define CMD_GET_FEATURE		0x0F
#define CMD_SET_FEATURE		0x1F
#define CMD_PAGE_READ		0x13
#define CMD_PROGRAM_LOAD	0x02
#define CMD_PROGRAM_EXEC	0x10
#define CMD_BLOCK_ERASE		0xD8
#define CMD_DIE_SELECT		0xC2

#define REG_PROTECT		0xA0
#define REG_STATUS		0xC0
#define REG_CONFIG		0xB0
#define CFG_BUF			BIT(3)
#define ST_BUSY			BIT(0)
#define ST_E_FAIL		BIT(2)
#define ST_P_FAIL		BIT(3)
#define ST_ECC			(3 << 4)
#define ST_ECC_ERR		(2 << 4)

#define SF_TIMEOUT_MS		1000

/* The SNI, at the start of block 0, or of one of its backups */
#define SNI_TAG			"MSTARSEMIUSFDCIS"
#define SNI_TAG_LEN		16
#define SNI_COPIES		5
#define SNI_DEFAULT_BLK_PAGES	64

struct ms_sni {
	u8 id_len;
	u8 id[15];
	__le16 spare;
	__le16 page;
	__le16 blk_pages;
	__le16 blocks;
	__le16 sector;
	u8 planes;
} __packed;

struct ms_spinand {
	struct nand_chip chip;
	struct nand_flash_dev ids[2];
	u8 *buf;		/* page and spare */
	unsigned int pos;
	unsigned int page_size, spare_size, blk_pages, blocks;
	u8 planes;
	bool dies;		/* two dies behind one chip select */
	int page;		/* of the write in progress */
	u8 status;		/* of the last read, program or erase */
	bool failed;		/* the last program or erase */
	bool show_status;
	u8 id[8];
};

static struct ms_spinand ms_nand;

static int fsp_wait(void)
{
	int ret = wait_for_bit_le16((void *)FSP_DONE, BIT(0), true,
				    SF_TIMEOUT_MS, false);

	writew(BIT(0), FSP_DONE_CLR);
	return ret;
}

static void fsp_setup(void)
{
	writew(CTRL_BASE, FSP_CTRL);
	writew(0, FSP_CTRL2);
	writew(0, FSP_CTRL4);
}

static void fsp_load(const u8 *wbuf, unsigned int wlen)
{
	unsigned int i;

	for (i = 0; i < wlen; i += 2)
		writew(wbuf[i] | (i + 1 < wlen ? wbuf[i + 1] << 8 : 0),
		       FSP_WBF(i));
}

#ifdef CONFIG_SS_I6C_FAMILY
static void sn_cs(bool high)
{
	clrsetbits_le16(QSPI_BURST, QSPI_CS_SW | QSPI_CS_HIGH,
			QSPI_CS_SW | (high ? QSPI_CS_HIGH : 0));
}
#else
static void sn_cs(bool high)
{
}
#endif

/* One FSP run: wlen bytes out, then rlen back, up to 10 of each */
static int fsp_run(const u8 *wbuf, unsigned int wlen, u8 *rbuf,
		   unsigned int rlen)
{
	unsigned int i;
	int ret;

	fsp_setup();
	fsp_load(wbuf, wlen);
	writew(wlen, FSP_WBF_SIZE);
	writew(rlen, FSP_RBF_SIZE);
	writew(BIT(0), FSP_TRIGGER);
	ret = fsp_wait();
	for (i = 0; !ret && i < rlen; i++)
		rbuf[i] = readw(FSP_RBF(i)) >> (i & 1 ? 8 : 0);
	return ret;
}

/* One flash command */
static int fsp_cmd(const u8 *wbuf, unsigned int wlen, u8 *rbuf,
		   unsigned int rlen)
{
	int ret;

	sn_cs(false);
	ret = fsp_run(wbuf, wlen, rbuf, rlen);
	sn_cs(true);
	return ret;
}

static int sn_cmd_addr(u8 op, u32 row)
{
	u8 cmd[4] = { op, row >> 16, row >> 8, row };

	return fsp_cmd(cmd, sizeof(cmd), NULL, 0);
}

static int sn_get_status(u8 *status)
{
	u8 cmd[2] = { CMD_GET_FEATURE, REG_STATUS };
	ulong start = get_timer(0);
	int ret;

	do {
		ret = fsp_cmd(cmd, sizeof(cmd), status, 1);
		if (ret)
			return ret;
		if (!(*status & ST_BUSY))
			return 0;
		WATCHDOG_RESET();
	} while (get_timer(start) < SF_TIMEOUT_MS);
	return -ETIMEDOUT;
}

static int sn_unlock(void)
{
	u8 op = CMD_WREN;
	u8 cmd[3] = { CMD_SET_FEATURE, REG_PROTECT, 0 };
	int ret = fsp_cmd(&op, 1, NULL, 0);

	return ret ? ret : fsp_cmd(cmd, sizeof(cmd), NULL, 0);
}

static void sn_die_select(struct ms_spinand *ms, u32 row)
{
	u8 cmd[2] = { CMD_DIE_SELECT, row >= ms->blocks * ms->blk_pages / 2 };

	if (ms->dies)
		fsp_cmd(cmd, sizeof(cmd), NULL, 0);
}

/* On two-plane parts odd blocks are in plane 1, a column address bit */
static u16 sn_column(struct ms_spinand *ms, u32 row)
{
	return ms->planes && (row / ms->blk_pages & 1) ? BIT(12) : 0;
}

static void bdma_start(u16 path, u32 src, u32 dst, u32 len)
{
	writew(0, BDMA_CTRL);
	writew(path, BDMA_PATH);
	writew(0, BDMA_PATH2);
	writew(src & 0xffff, BDMA_SRC_L);
	writew(src >> 16, BDMA_SRC_H);
	writew(dst & 0xffff, BDMA_DST_L);
	writew(dst >> 16, BDMA_DST_H);
	writew(len & 0xffff, BDMA_LEN_L);
	writew(len >> 16, BDMA_LEN_H);
	writew(BDMA_TRIGGER, BDMA_CTRL);
}

static int bdma_wait(void)
{
	int ret = wait_for_bit_le16((void *)BDMA_STATUS, BDMA_DONE, true,
				    SF_TIMEOUT_MS, false);

	writew(BDMA_DONE, BDMA_STATUS);
	return ret;
}

/* Read from the page cache, the buffer aligned for BDMA */
static int sn_cache_read(u16 column, u8 *buf, unsigned int len)
{
	int ret;

	writew(QSPI_FUNC_CACHE, QSPI_FUNC);
	writew(QSPI_CKG_DUMMY, QSPI_CKG);
	writew(IS_ENABLED(CONFIG_SS_I6C_FAMILY) ? QSPI_MODE_NORMAL :
						  QSPI_MODE_FAST, QSPI_MODE);

	sn_cs(false);
	bdma_start(BDMA_PATH_SPI_MIU, column, SS_MIU_ADDR(buf), len);
	ret = bdma_wait();
	sn_cs(true);
	return ret;
}

static int sn_page_read(struct ms_spinand *ms, u32 row, bool main_area)
{
	u16 column = sn_column(ms, row);
	int ret;

	sn_die_select(ms, row);
	ret = sn_cmd_addr(CMD_PAGE_READ, row);
	if (!ret)
		ret = sn_get_status(&ms->status);
	if (!ret && main_area)
		ret = sn_cache_read(column, ms->buf, ms->page_size);
	if (!ret)
		ret = sn_cache_read(column + ms->page_size,
				      ms->buf + ms->page_size, ms->spare_size);
	return ret;
}

#ifdef CONFIG_SS_I6C_FAMILY
/* A page load, the chip select held by hand around it; the FSP fires first */
static int sn_program_load(u16 column, const u8 *buf, unsigned int len)
{
	u8 cmd[3] = { CMD_PROGRAM_LOAD, column >> 8, column };
	unsigned int off = 0;
	int ret;

	sn_cs(false);
	ret = fsp_run(cmd, sizeof(cmd), NULL, 0);

	while (!ret && off < len) {
		unsigned int n = min_t(unsigned int, len - off, BDMA_CHUNK);
		int bdma;

		writew(OUT_EN, FSP_OUT);
		writew(n + 1, FSP_OUT_SIZE);
		fsp_setup();
		writew(0, FSP_WBF_SIZE);
		writew(0, FSP_RBF_SIZE);
		writew(BIT(0), FSP_TRIGGER);

		bdma_start(BDMA_PATH_MIU_FSP, SS_MIU_ADDR(buf + off), 0, n);
		bdma = bdma_wait();
		ret = fsp_wait();
		if (!ret)
			ret = bdma;
		off += n;
	}

	writew(0, FSP_OUT);
	writew(0, FSP_OUT_SIZE);
	sn_cs(true);
	return ret;
}
#else
/*
 * A page load: the command in one FSP run, the data in as many outside-buffer
 * runs from BDMA, the load held open across them by QSPI's burst bit. QSPI's
 * chip select timeout is off meanwhile. OUT_SIZE counts one more than BDMA
 * sends here, where it replaces the whole of an empty command.
 */
static int sn_program_load(u16 column, const u8 *buf, unsigned int len)
{
	u8 cmd[3] = { CMD_PROGRAM_LOAD, column >> 8, column };
	u16 timeout = readw(QSPI_TIMEOUT_H);
	unsigned int off = 0;
	int ret;

	if (timeout & QSPI_TIMEOUT_EN)
		writew(QSPI_TIMEOUT_OFF, QSPI_TIMEOUT_H);

	writew(0, FSP_OUT_SIZE);
	writew(0, FSP_OUT);
	ret = fsp_run(cmd, sizeof(cmd), NULL, 0);

	while (!ret && off < len) {
		unsigned int n = min_t(unsigned int, len - off, BDMA_CHUNK);
		int bdma;

		writew(n + 1, FSP_OUT_SIZE);
		writew(OUT_EN, FSP_OUT);
		writew(QSPI_BURST_ON, QSPI_BURST);
		writew(0, FSP_WBF_SIZE);
		writew(0, FSP_RBF_SIZE);
		writew(0, FSP_CTRL2);

		bdma_start(BDMA_PATH_MIU_FSP, SS_MIU_ADDR(buf + off), 0, n);
		writew(BIT(0), FSP_TRIGGER);

		bdma = bdma_wait();
		ret = fsp_wait();
		if (!ret)
			ret = bdma;
		off += n;
	}

	writew(0, FSP_OUT_SIZE);
	writew(0, FSP_OUT);
	writew(QSPI_BURST_OFF, QSPI_BURST);
	if (timeout & QSPI_TIMEOUT_EN)
		writew(timeout, QSPI_TIMEOUT_H);
	return ret;
}

#endif

/* The page and, unless it is blank, its spare area */
static int sn_page_program(struct ms_spinand *ms, u32 row)
{
	unsigned int len = ms->page_size + ms->spare_size;
	u16 column = sn_column(ms, row);
	u8 op = CMD_WREN;
	int ret;

	if (memchr_inv(ms->buf + ms->page_size, 0xff, ms->spare_size) == NULL)
		len = ms->page_size;

	ss_miu_flush();
	sn_die_select(ms, row);
	ret = sn_unlock();
	if (!ret)
		ret = sn_program_load(column, ms->buf, len);
	if (!ret)
		ret = fsp_cmd(&op, 1, NULL, 0);
	if (!ret)
		ret = sn_cmd_addr(CMD_PROGRAM_EXEC, row);
	if (!ret)
		ret = sn_get_status(&ms->status);
	return ret ? ret : ms->status & ST_P_FAIL ? -EIO : 0;
}

static int sn_block_erase(struct ms_spinand *ms, u32 row)
{
	u8 op = CMD_WREN;
	int ret;

	sn_die_select(ms, row);
	ret = sn_unlock();
	if (!ret)
		ret = fsp_cmd(&op, 1, NULL, 0);
	if (!ret)
		ret = sn_cmd_addr(CMD_BLOCK_ERASE, row);
	if (!ret)
		ret = sn_get_status(&ms->status);
	return ret ? ret : ms->status & ST_E_FAIL ? -EIO : 0;
}

static void ms_cmdfunc(struct mtd_info *mtd, unsigned int command, int column,
		       int page)
{
	struct ms_spinand *ms = nand_get_controller_data(mtd_to_nand(mtd));
	u8 cmd[2] = { CMD_RDID, 0 };

	ms->show_status = false;
	switch (command) {
	case NAND_CMD_RESET:
		cmd[0] = CMD_RESET;
		fsp_cmd(cmd, 1, NULL, 0);
		sn_get_status(&ms->status);
		break;
	case NAND_CMD_READID:
		memset(ms->buf, 0, sizeof(ms->id));
		if (!column)
			memcpy(ms->buf, ms->id, sizeof(ms->id));
		ms->pos = 0;
		break;
	case NAND_CMD_READ0:
		ms->pos = column;
		if (sn_page_read(ms, page, true))
			ms->status = ST_ECC_ERR;
		break;
	case NAND_CMD_READOOB:
		ms->pos = ms->page_size + column;
		if (sn_page_read(ms, page, false))
			ms->status = ST_ECC_ERR;
		break;
	case NAND_CMD_RNDOUT:
		ms->pos = column;
		break;
	case NAND_CMD_SEQIN:
		memset(ms->buf, 0xff, ms->page_size + ms->spare_size);
		ms->page = page;
		ms->pos = column;
		break;
	case NAND_CMD_RNDIN:
		ms->pos = column;
		break;
	case NAND_CMD_PAGEPROG:
		ms->failed = !!sn_page_program(ms, ms->page);
		break;
	case NAND_CMD_ERASE1:
		ms->failed = !!sn_block_erase(ms, page);
		break;
	case NAND_CMD_ERASE2:
		break;
	case NAND_CMD_STATUS:
		ms->show_status = true;
		break;
	}
}

static u8 ms_status(struct ms_spinand *ms)
{
	return NAND_STATUS_READY | NAND_STATUS_WP |
	       (ms->failed ? NAND_STATUS_FAIL : 0);
}

static u8 ms_read_byte(struct mtd_info *mtd)
{
	struct ms_spinand *ms = nand_get_controller_data(mtd_to_nand(mtd));

	if (ms->show_status)
		return ms_status(ms);
	if (ms->pos >= ms->page_size + ms->spare_size)
		return 0xff;
	return ms->buf[ms->pos++];
}

static unsigned int ms_clamp(struct ms_spinand *ms, unsigned int len)
{
	unsigned int size = ms->page_size + ms->spare_size;

	return ms->pos >= size ? 0 : min(len, size - ms->pos);
}

static void ms_read_buf(struct mtd_info *mtd, u8 *buf, int len)
{
	struct ms_spinand *ms = nand_get_controller_data(mtd_to_nand(mtd));
	unsigned int n = ms_clamp(ms, len);

	memcpy(buf, ms->buf + ms->pos, n);
	memset(buf + n, 0xff, len - n);
	ms->pos += n;
}

static void ms_write_buf(struct mtd_info *mtd, const u8 *buf, int len)
{
	struct ms_spinand *ms = nand_get_controller_data(mtd_to_nand(mtd));
	unsigned int n = ms_clamp(ms, len);

	memcpy(ms->buf + ms->pos, buf, n);
	ms->pos += n;
}

static void ms_select_chip(struct mtd_info *mtd, int chip)
{
}

static int ms_waitfunc(struct mtd_info *mtd, struct nand_chip *chip)
{
	return ms_status(nand_get_controller_data(chip));
}

/*
 * The core's READ0 has brought the page in: hand it over with what the
 * chip's ECC said of it. A corrected read is counted without looking like
 * wear, like the vendor's driver; the chip does not say how many bits.
 */
static int ms_read_page(struct mtd_info *mtd, struct nand_chip *chip, u8 *buf,
			int oob_required, int page)
{
	struct ms_spinand *ms = nand_get_controller_data(chip);
	u8 ecc = ms->status & ST_ECC;

	memcpy(buf, ms->buf, ms->page_size);
	if (oob_required)
		memcpy(chip->oob_poi, ms->buf + ms->page_size, ms->spare_size);

	if (ecc == ST_ECC_ERR) {
		mtd->ecc_stats.failed++;
		return 0;
	}
	if (ecc) {
		mtd->ecc_stats.corrected++;
		return 1;
	}
	return 0;
}

static int ms_read_page_raw(struct mtd_info *mtd, struct nand_chip *chip,
			    u8 *buf, int oob_required, int page)
{
	struct ms_spinand *ms = nand_get_controller_data(chip);

	memcpy(buf, ms->buf, ms->page_size);
	if (oob_required)
		memcpy(chip->oob_poi, ms->buf + ms->page_size, ms->spare_size);
	return 0;
}

static int ms_write_page(struct mtd_info *mtd, struct nand_chip *chip,
			 const u8 *buf, int oob_required, int page)
{
	struct ms_spinand *ms = nand_get_controller_data(chip);

	memcpy(ms->buf, buf, ms->page_size);
	memcpy(ms->buf + ms->page_size, chip->oob_poi, ms->spare_size);
	return 0;
}

/* The chip's on-die ECC, in the spare area */
static struct nand_ecclayout ms_ecclayout = {
	.eccbytes = 32,
	.eccpos = { 8, 9, 10, 11, 12, 13, 14, 15,
		    24, 25, 26, 27, 28, 29, 30, 31,
		    40, 41, 42, 43, 44, 45, 46, 47,
		    56, 57, 58, 59, 60, 61, 62, 63 },
	.oobfree = { { 2, 6 }, { 16, 8 }, { 32, 8 }, { 48, 8 } },
};

/* BDMA needs its buffer here, so the SNI's page goes through ms->buf */
static int ms_read_sni(struct ms_spinand *ms)
{
	const struct ms_sni *sni = (void *)(ms->buf + SNI_TAG_LEN);
	unsigned int i;

	ms->blk_pages = SNI_DEFAULT_BLK_PAGES;
	ms->blocks = 1;
	for (i = 0; i < SNI_COPIES; i++) {
		u32 row = i * 2 * ms->blk_pages;
		u8 cmd[4] = { CMD_PAGE_READ, row >> 16, row >> 8, row };

		if (fsp_cmd(cmd, sizeof(cmd), NULL, 0) ||
		    sn_get_status(&ms->status) ||
		    (ms->status & ST_ECC) == ST_ECC_ERR ||
		    sn_cache_read(0, ms->buf, 512))
			continue;
		if (!memcmp(ms->buf, "SNIB", 4)) {
			/* Iford's CIS: the boot header, then the SNIF profile */
			ms->spare_size = get_unaligned_le16(ms->buf + 0x34);
			ms->page_size = get_unaligned_le16(ms->buf + 0x36);
			ms->blk_pages = get_unaligned_le16(ms->buf + 0x3A);
			ms->blocks = get_unaligned_le16(ms->buf + 0x3C);
			ms->planes = 0;
			return 0;
		}
		if (memcmp(ms->buf, SNI_TAG, SNI_TAG_LEN))
			continue;

		ms->page_size = le16_to_cpu(sni->page);
		ms->spare_size = le16_to_cpu(sni->spare);
		ms->blk_pages = le16_to_cpu(sni->blk_pages);
		ms->blocks = le16_to_cpu(sni->blocks);
		ms->planes = sni->planes;
		return 0;
	}
	return -ENODEV;
}

static int ms_nand_probe(struct ms_spinand *ms)
{
	static const u8 rdid[2] = { CMD_RDID, 0 };
	u8 op = CMD_RESET;
	int ret;

	if (!IS_ENABLED(CONFIG_SS_I6C_FAMILY)) {
		writew(0, CHIP_ALL_PAD_IN);
		writew(0, QSPI_CS_SEL);
		writew(0x08, PM_SPI_CZ);
	}

	ret = fsp_cmd(&op, 1, NULL, 0);
	if (!ret)
		ret = sn_get_status(&ms->status);
	if (!ret)
		ret = fsp_cmd(rdid, sizeof(rdid), ms->id, 3);
	if (ret)
		return ret;

	/*
	 * Winbond W25N: an IPL may leave the buffer mode off (feature 0xB0
	 * bit 3), and then a cache read streams on and ignores the column
	 */
	if (ms->id[0] == 0xEF) {
		u8 get[2] = { CMD_GET_FEATURE, REG_CONFIG };
		u8 set[3] = { CMD_SET_FEATURE, REG_CONFIG, 0 };
		u8 cfg;

		if (!fsp_cmd(get, sizeof(get), &cfg, 1) && !(cfg & CFG_BUF)) {
			set[2] = cfg | CFG_BUF;
			fsp_cmd(set, sizeof(set), NULL, 0);
		}
	}

	/* Winbond W25M and GigaDevice's stacked parts: the vendor's test */
	ms->dies = (ms->id[0] == 0xEF && ms->id[1] == 0xAB &&
		    ms->id[2] == 0x21) ||
		   (ms->id[0] == 0xC8 && ms->id[1] == 0x0A);
	return ms_read_sni(ms);
}

void board_nand_init(void)
{
	struct ms_spinand *ms = &ms_nand;
	struct nand_chip *chip = &ms->chip;
	struct mtd_info *mtd = nand_to_mtd(chip);
	struct nand_flash_dev *dev = &ms->ids[0];

	ms->buf = memalign(ARCH_DMA_MINALIGN < 16 ? 16 : ARCH_DMA_MINALIGN,
			   SZ_16K);
	if (!ms->buf)
		return;
	if (ms_nand_probe(ms)) {
		printf("NAND: no SPI NAND with an SNI\n");
		return;
	}
	if (ms->page_size + ms->spare_size > SZ_16K ||
	    !ms->page_size || !ms->blk_pages || !ms->blocks) {
		printf("NAND: SNI says %u+%u bytes, %u pages, %u blocks\n",
		       ms->page_size, ms->spare_size, ms->blk_pages,
		       ms->blocks);
		return;
	}

	dev->name = "SPI NAND";
	dev->mfr_id = ms->id[0];
	dev->dev_id = ms->id[1];
	dev->id_len = 2;
	dev->pagesize = ms->page_size;
	dev->chipsize = ms->page_size * ms->blk_pages / 1024 * ms->blocks /
			1024;
	dev->erasesize = ms->page_size * ms->blk_pages;
	dev->oobsize = ms->spare_size;

	nand_set_controller_data(chip, ms);
	chip->cmdfunc = ms_cmdfunc;
	chip->read_byte = ms_read_byte;
	chip->read_buf = ms_read_buf;
	chip->write_buf = ms_write_buf;
	chip->select_chip = ms_select_chip;
	chip->waitfunc = ms_waitfunc;
	chip->options = NAND_NO_SUBPAGE_WRITE;

	if (nand_scan_ident(mtd, 1, ms->ids))
		return;

	/* The ID's third byte is the part's, not an ONFI cell type */
	chip->bits_per_cell = 1;
	chip->bbt_options &= ~NAND_BBT_SCANLASTPAGE;

	chip->ecc.mode = NAND_ECC_HW;
	chip->ecc.size = ms->page_size;
	chip->ecc.bytes = ms_ecclayout.eccbytes;
	chip->ecc.strength = 4;
	if (ms->spare_size == 64)
		chip->ecc.layout = &ms_ecclayout;
	chip->ecc.read_page = ms_read_page;
	chip->ecc.read_page_raw = ms_read_page_raw;
	chip->ecc.write_page = ms_write_page;
	chip->ecc.write_page_raw = ms_write_page;

	if (nand_scan_tail(mtd))
		return;
	nand_register(0, mtd);
}
