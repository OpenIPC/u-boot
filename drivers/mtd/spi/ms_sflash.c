// SPDX-License-Identifier: GPL-2.0+
/*
 * SPI NOR behind the FSP of the SigmaStar SoCs.
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
 * After the vendor U-Boot and kernel drivers/mstar/flash_isp (<die>
 * halSERFLASH.c: FSP_READ_DIRECT, FSP_WRITE_BDMA; the same FSP and BDMA
 * registers on every die, the FSP's bank apart on infinity6e). Like ak_sflash on
 * openipc-anyka, this driver provides spi_flash_probe() itself, so
 * mainline's non-DM spi_flash_*() wrappers, the sf command and the
 * environment in SPI flash work without the SPI and spi-nor layers.
 * 3-byte addresses: parts up to 16 MiB.
 *
 * infinity6c differs: its IPL hands over with the chip select in software,
 * so one FSP run is no longer one flash command. After the vendor's
 * flash/nor/drvSPINOR.c and fsp_qspi, what its published U-Boot runs (its
 * flash_isp/infinity6c does not describe that binary): each flash command
 * is framed by hand, reads go by BDMA or the FSP since the XIU window would
 * start a command per burst, and OUT_SIZE counts one more than BDMA sends.
 */

#include <common.h>
#include <spi_flash.h>
#include <time.h>
#include <wait_bit.h>
#include <watchdog.h>
#include <asm/io.h>
#include <asm/arch/sigmastar.h>
#include <linux/delay.h>
#include <linux/mtd/mtd.h>
#include <linux/sizes.h>

#define FSP_REG(r)		SS_REG(ss_fsp_bank(), r)
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

/* infinity6c: the QSPI bank next to the FSP, the chip select in software */
#define QSPI_REG(r)		SS_REG(ss_fsp_bank() + 1, r)
#define QSPI_CS			QSPI_REG(0x0a)
#define QSPI_CS_SW		BIT(0)
#define QSPI_CS_HIGH		BIT(1)
#define QSPI_CKG		QSPI_REG(0x70)
#define QSPI_CKG_DUMMY(n)	(BIT(4) | ((n) - 1))
#define QSPI_MODE		QSPI_REG(0x72)
#define QSPI_MODE_FAST		0x01
#define QSPI_FUNC		QSPI_REG(0x7d)
#define QSPI_FUNC_DUMMY		0x1000

/* BDMA channel 0 */
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
#ifdef CONFIG_SS_IFORD_FAMILY
#define BDMA_PATH_MIU_FSP	0x2B41	/* MIU by channel 1, to the FSP */
#else
#define BDMA_PATH_MIU_FSP	0x2B40	/* MIU, 16 bytes wide, to the FSP */
#endif
#define BDMA_PATH_SPI_MIU	0x4135	/* flash, 8 wide, to MIU, 16 wide */
#define BDMA_PATH2_CH0_MIU	0x8000	/* infinity6*: channel 0 to the MIU */

#define SZ_PAGE			256
#define SZ_SECTOR		SZ_64K

#define CMD_WRSR		0x01
#define CMD_PP			0x02
#define CMD_RDSR		0x05
#define CMD_RDSR2		0x35
#define CMD_FAST_READ		0x0B
#define CMD_WREN		0x06
#define CMD_RDID		0x9F
#define CMD_SE_64K		0xD8

#define SR_WIP			BIT(0)
#define SR_BP			0x3C
#define SR_SRWD			BIT(7)

/* Longer than any 64 KiB erase or page program of the parts in use */
#define SF_TIMEOUT_MS		3000
/* A BDMA read of a whole 16 MiB part, at the slowest clock an IPL sets */
#define SF_READ_TIMEOUT_MS	30000

#define FSP_BUF_SIZE		10

static int fsp_wait(void)
{
	int ret = wait_for_bit_le16((void *)FSP_DONE, BIT(0), true,
				    SF_TIMEOUT_MS, false);

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

/* BDMA channel 0; the DDR's address is the MIU's */
static void bdma_start(u16 path, u32 src, u32 dst, u32 len)
{
	writew(0, BDMA_CTRL);
	writew(path, BDMA_PATH);
#if defined(CONFIG_SS_INFINITY2M) || defined(CONFIG_SS_IFADO) || \
    defined(CONFIG_SS_I6C_FAMILY)
	writew(0, BDMA_PATH2);
#elif !defined(CONFIG_SS_INFINITY3)
	writew(BDMA_PATH2_CH0_MIU, BDMA_PATH2);
#endif
	writew(src & 0xffff, BDMA_SRC_L);
	writew(src >> 16, BDMA_SRC_H);
	writew(dst & 0xffff, BDMA_DST_L);
	writew(dst >> 16, BDMA_DST_H);
	writew(len & 0xffff, BDMA_LEN_L);
	writew(len >> 16, BDMA_LEN_H);
	writew(BDMA_TRIGGER, BDMA_CTRL);
}

static int bdma_wait(unsigned int timeout_ms)
{
	int ret = wait_for_bit_le16((void *)BDMA_STATUS, BDMA_DONE, true,
				    timeout_ms, false);

	writew(BDMA_DONE, BDMA_STATUS);
	return ret;
}

#ifdef CONFIG_SS_FSP_SW_CS
static void sf_cs(bool high)
{
	clrsetbits_le16(QSPI_CS, QSPI_CS_SW | QSPI_CS_HIGH,
			QSPI_CS_SW | (high ? QSPI_CS_HIGH : 0));
}

/* With the chip select held: bytes out, as many FSP runs as they take */
static int sf_send(const u8 *buf, unsigned int len)
{
	while (len) {
		unsigned int n = min_t(unsigned int, len, FSP_BUF_SIZE);
		int ret = fsp_run(buf, n, FSP_SIZES(n, 0, 0), 0, 0, NULL, 0);

		if (ret)
			return ret;
		buf += n;
		len -= n;
	}
	return 0;
}

static int sf_recv(u8 *buf, size_t len)
{
	while (len) {
		unsigned int n = min_t(size_t, len, FSP_BUF_SIZE);
		int ret = fsp_run(NULL, 0, 0, FSP_SIZES(n, 0, 0), 0, buf, n);

		if (ret)
			return ret;
		WATCHDOG_RESET();
		buf += n;
		len -= n;
	}
	return 0;
}

/* One flash command, framed by the chip select */
static int sf_cmd(const u8 *tx, unsigned int txlen, u8 *rx, size_t rxlen)
{
	int ret;

	sf_cs(false);
	ret = sf_send(tx, txlen);
	if (!ret)
		ret = sf_recv(rx, rxlen);
	sf_cs(true);
	return ret;
}

static int sf_wren(void)
{
	u8 op = CMD_WREN;

	return sf_cmd(&op, 1, NULL, 0);
}

static int sf_read_cmd(u8 op, u8 *buf, unsigned int len)
{
	return sf_cmd(&op, 1, buf, len);
}
#else
/* A command alone, reading len (up to 10) bytes back */
static int sf_read_cmd(u8 op, u8 *buf, unsigned int len)
{
	return fsp_run(&op, 1, FSP_SIZES(1, 0, 0), FSP_SIZES(len, 0, 0), 0,
		       buf, len);
}
#endif

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

/* BDMA reads its source from DDR: a page goes through here */
static u8 sf_page[SZ_PAGE] __aligned(64);

#ifdef CONFIG_SS_FSP_SW_CS
/* Short of this, or unaligned, the vendor's driver goes by the FSP */
#define SF_BDMA_MIN		64
#define SF_BDMA_ALIGN		16

static int sf_erase_sector(u32 addr)
{
	u8 cmd[4] = { CMD_SE_64K, addr >> 16, addr >> 8, addr };
	int ret = sf_wren();

	return ret ? ret : sf_cmd(cmd, sizeof(cmd), NULL, 0);
}

/* The rest of a page program, chip select held; sf_page is aligned */
static int sf_program_bdma(unsigned int len)
{
	int ret, bdma;

	ss_miu_flush();
	writew(CTRL_BASE, FSP_CTRL);
	writew(0, FSP_WBF_SIZE);
	writew(0, FSP_RBF_SIZE);
	writew(OUT_EN, FSP_OUT);
	writew(len + 1, FSP_OUT_SIZE);
	writew(BIT(0), FSP_TRIGGER);

	bdma_start(BDMA_PATH_MIU_FSP, SS_MIU_ADDR(sf_page), 0, len);
	bdma = bdma_wait(SF_TIMEOUT_MS);
	ret = fsp_wait();

	writew(0, FSP_OUT);
	writew(0, FSP_OUT_SIZE);
	return ret ? ret : bdma;
}

static int sf_program_page(u32 to, const u8 *buf, unsigned int len)
{
	u8 cmd[4] = { CMD_PP, to >> 16, to >> 8, to };
	int ret;

	memcpy(sf_page, buf, len);
	ret = sf_wren();
	if (ret)
		return ret;

	sf_cs(false);
	ret = sf_send(cmd, sizeof(cmd));
	if (!ret)
		ret = len < SF_BDMA_MIN ? sf_send(sf_page, len) :
					  sf_program_bdma(len);
	sf_cs(true);
	return ret ? ret : sf_wait_ready();
}

static int sf_read_fsp(u32 from, u8 *buf, size_t len)
{
	u8 cmd[5] = { CMD_FAST_READ, from >> 16, from >> 8, from, 0 };

	return sf_cmd(cmd, sizeof(cmd), buf, len);
}

/* QSPI's fast read, 8 dummy cycles: what the vendor's driver reads with */
static int sf_read_bdma(u32 from, u8 *buf, size_t len)
{
	int ret;

	writew(QSPI_FUNC_DUMMY, QSPI_FUNC);
	writew(QSPI_CKG_DUMMY(8), QSPI_CKG);
	writew(QSPI_MODE_FAST, QSPI_MODE);

	sf_cs(false);
	bdma_start(BDMA_PATH_SPI_MIU, from, SS_MIU_ADDR(buf), len);
	ret = bdma_wait(SF_READ_TIMEOUT_MS);
	sf_cs(true);
	return ret;
}

static int ms_sf_read(struct mtd_info *mtd, loff_t from, size_t len,
		      size_t *retlen, u_char *buf)
{
	int ret;

	if (from > mtd->size || len > mtd->size - from)
		return -EINVAL;
	if (len < SF_BDMA_MIN || from % SF_BDMA_ALIGN ||
	    (ulong)buf % SF_BDMA_ALIGN)
		ret = sf_read_fsp(from, buf, len);
	else
		ret = sf_read_bdma(from, buf, len);
	if (ret)
		return ret;
	*retlen = len;
	return 0;
}
#else
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

static int sf_erase_sector(u32 addr)
{
	return sf_wren_cmd(CMD_SE_64K, addr, false);
}

static int sf_program_page(u32 to, const u8 *buf, unsigned int len)
{
	int ret, bdma;

	memcpy(sf_page, buf, len);
	ss_miu_flush();

	bdma_start(BDMA_PATH_MIU_FSP, SS_MIU_ADDR(sf_page), 0, len);

	writew(len, FSP_OUT_SIZE);
	writew(OUT_REPLACE(5) | OUT_MODE_BDMA | OUT_EN, FSP_OUT);
	ret = sf_wren_cmd(CMD_PP, to, true);

	bdma = bdma_wait(SF_TIMEOUT_MS);
	writew(0, FSP_OUT_SIZE);
	writew(0, FSP_OUT);
	return ret ? ret : bdma ? bdma : sf_wait_ready();
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
#endif

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
		ret = sf_erase_sector(addr);
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

#define MFR_GIGADEVICE		0xc8
#define MFR_WINBOND		0xef

/* wbuf: WREN, WRSR, n status bytes, RDSR */
static int sf_write_status(const u8 *wbuf, unsigned int n)
{
#ifdef CONFIG_SS_FSP_SW_CS
	int ret = sf_wren();

	return ret ? ret : sf_cmd(wbuf + 1, 1 + n, NULL, 0);
#else
	return fsp_run(wbuf, 3 + n, FSP_SIZES(1, 1 + n, 1), FSP_SIZES(0, 0, 1),
		       CTRL_CMD2 | CTRL_CMD3 | CTRL_RDSR(2) | CTRL_FSCHK,
		       NULL, 0);
#endif
}

/*
 * Block protection left on by a vendor loader: the BP and SRWD bits
 * cleared, every other status bit kept (Macronix's quad enable is in
 * status register 1). Winbond and GigaDevice parts get status register 2
 * written back with it: on some of them a one-byte WRSR clears it, and
 * with it the quad enable the IPL's read mode may need. Nothing is
 * written when no protection is set.
 */
static void sf_unprotect(u8 mfr)
{
	u8 wbuf[5] = { CMD_WREN, CMD_WRSR, 0, 0, CMD_RDSR };
	unsigned int n = 1;
	u8 sr, sr2;

	if (sf_read_cmd(CMD_RDSR, &sr, 1) || !(sr & (SR_BP | SR_SRWD)))
		return;
	wbuf[2] = sr & ~(SR_BP | SR_SRWD);
	if ((mfr == MFR_WINBOND || mfr == MFR_GIGADEVICE) &&
	    !sf_read_cmd(CMD_RDSR2, &sr2, 1)) {
		wbuf[3] = sr2;
		n = 2;
	}
	wbuf[2 + n] = CMD_RDSR;
	if (sf_write_status(wbuf, n) || sf_wait_ready())
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

	sf_unprotect(id[0]);

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
