// SPDX-License-Identifier: GPL-2.0+
/*
 * Goke GK7101/GK7102 serial flash controller ("sflash") with a SPI NOR
 * behind it.
 *
 * A register-driven SPI master with no memory window. COMMAND describes one
 * transaction -- opcode, address and dummy byte counts, direction, lane
 * widths, 1-4 data bytes per DATA access -- and writing it starts the
 * transaction. With address bytes, the first DATA write is the address; a
 * command without data (an erase) runs then, reads and programs stream
 * through DATA from there on.
 *
 * Bytes are packed MSB first: a four-byte read returns the first byte from
 * flash in bits 31:24. Like the boot ROM and the vendor kernel, this driver
 * moves whole little-endian words between memory and DATA, so the flash
 * holds every 32-bit word of an image byte-swapped, and reads it back as it
 * was written. A camera's flash dump reads as big-endian for that reason.
 *
 * After the vendor U-Boot's drivers/mtd/spi/gk7101_sflash.c and kernel
 * gk_sflash_v1_00.c, single-lane only; like them it provides
 * spi_flash_probe() itself, so mainline's non-DM spi_flash_*() wrappers, the
 * sf command and the environment in SPI flash work without the SPI and
 * spi-nor layers. Parts up to 16 MiB: the 4-byte/bank-register path of the
 * vendor code is left out.
 */

#include <common.h>
#include <spi_flash.h>
#include <watchdog.h>
#include <asm/io.h>
#include <asm/unaligned.h>
#include <asm/arch/gk710x.h>
#include <linux/mtd/mtd.h>
#include <linux/sizes.h>
#include <linux/byteorder/swab.h>

#define SF_DATA			(GK_SFLASH_BASE + 0x00)
#define SF_COMMAND		(GK_SFLASH_BASE + 0x04)
#define SF_CE			(GK_SFLASH_BASE + 0x08)
#define SF_SPEED		(GK_SFLASH_BASE + 0x0C)

/* COMMAND */
#define SF_SEND_CMD		BIT(8)
#define SF_ADDR3		(3 << 9)
#define SF_RWN_READ		(0 << 15)
#define SF_RWN_WRITE		(1 << 15)
#define SF_RWN_NOTHING		(3 << 15)
#define SF_BYTES(n)		((n) << 23)
#define SF_BYTES_4		SF_BYTES(0x1f)

/* CE: chip select 0, WP and HOLD inactive (vendor value) */
#define SF_CE_DEFAULT		0x0E
#define SF_SPEED_DIV2		0

#define SZ_PAGE			256
#define SZ_SECTOR		SZ_64K

#define CMD_WRSR		0x01
#define CMD_PP			0x02
#define CMD_READ		0x03
#define CMD_RDSR		0x05
#define CMD_WREN		0x06
#define CMD_RDSR2		0x35
#define CMD_RDID		0x9F
#define CMD_SE_64K		0xD8

#define SR1_WIP			BIT(0)
#define SR1_BP			0x7C	/* GigaDevice/Winbond: BP0-4 */
#define SR1_BP_OTHER		0x3C	/* BP0-3 */

/* Longer than any erase or program of the parts on these boards */
#define SF_TIMEOUT_MS		3000

#define MFR_GIGADEVICE		0xc8
#define MFR_WINBOND		0xef
#define MFR_XMC			0x20

static u32 sf_reg_read(u8 op, unsigned int nbytes)
{
	writel(op | SF_SEND_CMD | SF_RWN_READ | SF_BYTES(nbytes), SF_COMMAND);
	return readl(SF_DATA);
}

/* A command without address or data starts with the DATA access */
static void sf_cmd(u8 op)
{
	writel(op | SF_SEND_CMD | SF_RWN_NOTHING, SF_COMMAND);
	readl(SF_DATA);
}

static int sf_wait_ready(void)
{
	ulong start = get_timer(0);

	while (sf_reg_read(CMD_RDSR, 1) & SR1_WIP) {
		if (get_timer(start) > SF_TIMEOUT_MS) {
			printf("SF: timed out waiting for the flash\n");
			return -ETIMEDOUT;
		}
		WATCHDOG_RESET();
	}
	return 0;
}

static int sf_write_sr(u32 val, unsigned int nbytes)
{
	sf_cmd(CMD_WREN);
	writel(CMD_WRSR | SF_SEND_CMD | SF_RWN_WRITE | SF_BYTES(nbytes),
	       SF_COMMAND);
	writel(val, SF_DATA);
	return sf_wait_ready();
}

/* Clear the block protection a vendor loader may have left set */
static void sf_unprotect(u32 jedec)
{
	const u8 mfr = jedec >> 16;
	u32 sr1 = sf_reg_read(CMD_RDSR, 1) & 0xff;

	if (sr1 == 0xff)
		return;
	if (mfr == MFR_GIGADEVICE || mfr == MFR_WINBOND || mfr == MFR_XMC) {
		/* QE lives in SR2, written as WRSR's second byte: keep it */
		if (sr1 & SR1_BP) {
			u32 sr2 = sf_reg_read(CMD_RDSR2, 1) & 0xff;

			printf("SF: block protection (SR1 %02x), clearing\n", sr1);
			sf_write_sr((sr1 & ~SR1_BP) << 8 | sr2, 2);
		}
	} else if (sr1 & SR1_BP_OTHER) {
		printf("SF: block protection (SR1 %02x), clearing\n", sr1);
		sf_write_sr(sr1 & ~SR1_BP_OTHER, 1);
	}
}

static int sf_erase_sector(u32 addr)
{
	sf_cmd(CMD_WREN);
	writel(CMD_SE_64K | SF_SEND_CMD | SF_ADDR3 | SF_RWN_NOTHING,
	       SF_COMMAND);
	writel(addr, SF_DATA);
	return sf_wait_ready();
}

/* Whole words, within one page */
static int sf_program(u32 addr, const u8 *buf, unsigned int words)
{
	sf_cmd(CMD_WREN);
	writel(CMD_PP | SF_SEND_CMD | SF_ADDR3 | SF_RWN_WRITE | SF_BYTES_4,
	       SF_COMMAND);
	writel(addr, SF_DATA);
	while (words--) {
		writel(get_unaligned_le32(buf), SF_DATA);
		buf += 4;
	}
	return sf_wait_ready();
}

/* The boot ROM header's first word, "GK71" */
#define GK_BOOT_MAGIC		0x474b3731

/*
 * The controller packs bytes MSB first: every 32-bit word goes to the chip
 * byte-swapped and comes back swapped again, so through this driver the
 * flash reads and writes in CPU order, the order of every published file
 * (U-Boot, kernel, rootfs) and of what tftp, mmc or usb load. The chip
 * itself, which the boot ROM and a programmer see, holds every word
 * swapped; a programmer image is the files laid out and every word
 * swapped, and a dump swapped back is in CPU order.
 *
 * A programmer image may still be written through here (sf write, SD
 * recovery). A write at 0 that starts with the swapped boot magic is
 * taken for one and swapped back: over sf_swap_end -- header table plus
 * the U-Boot length the header gives -- however the caller splits the
 * rest, and over its whole length when that one write is longer (a
 * whole-chip image). Any other write at 0 closes the window. The vendor
 * U-Boot's flash write does the same (check_magic_number() in its
 * gk7101_sflash.c). Reads are never swapped.
 */
static u32 sf_swap_end;

/* The length of a chip-order boot image starting at buf, or 0 */
static u32 sf_boot_len(const u_char *buf, size_t len)
{
	u32 nwords, i;

	if (len < 32 || get_unaligned_le32(buf) != swab32(GK_BOOT_MAGIC))
		return 0;
	/*
	 * Header: 7 words, then the table -- <register, value> writes,
	 * 0xFFFFFFFE <register, value> polls, 0xFFFFFFFF <count> delays --
	 * ended by a 0 word; then nwords of U-Boot. A value can be 0, so the
	 * table is walked entry by entry.
	 */
	nwords = swab32(get_unaligned_le32(buf + 8));
	for (i = 7 * 4; i + 4 <= len; ) {
		u32 w = swab32(get_unaligned_le32(buf + i));

		if (!w)
			break;
		i += 4 * (w == 0xfffffffe ? 3 : 2);
	}
	if (i + 4 > len)
		return 0;
	return i + 4 + 4 * nwords;
}

static void sf_swap_detect(loff_t to, size_t len, const u_char *buf)
{
	if (to)
		return;
	sf_swap_end = sf_boot_len(buf, len);
	if (sf_swap_end)
		printf("SF: boot image in the chip's byte order, swapping it back\n");
}

static int gk_sf_read(struct mtd_info *mtd, loff_t from, size_t len,
		      size_t *retlen, u_char *buf)
{
	u32 addr = from & ~3;
	unsigned int skip = from & 3;

	if (from + len > mtd->size)
		return -EINVAL;
	*retlen = len;
	if (!len)
		return 0;

	writel(CMD_READ | SF_SEND_CMD | SF_ADDR3 | SF_RWN_READ | SF_BYTES_4,
	       SF_COMMAND);
	writel(addr, SF_DATA);
	while (len) {
		u32 w = readl(SF_DATA);
		unsigned int n = min_t(size_t, len, 4 - skip);

		if (!skip && n == 4) {
			put_unaligned_le32(w, buf);
		} else {
			u8 b[4];

			put_unaligned_le32(w, b);
			memcpy(buf, b + skip, n);
			skip = 0;
		}
		buf += n;
		len -= n;
	}
	return 0;
}

/*
 * The controller programs whole words, so a write that starts or ends
 * inside one pads it with 0xff, which leaves those flash bytes as they are.
 */
static int gk_sf_write(struct mtd_info *mtd, loff_t to, size_t len,
		       size_t *retlen, const u_char *buf)
{
	u8 page[SZ_PAGE];
	u32 addr = to & ~3;
	unsigned int head = to & 3;
	unsigned int i;
	u32 end;
	int ret;

	if (to + len > mtd->size)
		return -EINVAL;
	sf_swap_detect(to, len, buf);
	end = !to && sf_swap_end ? max_t(u32, sf_swap_end, len) : sf_swap_end;
	*retlen = 0;
	while (len) {
		unsigned int room = SZ_PAGE - (addr & (SZ_PAGE - 1));
		unsigned int n = min_t(size_t, len, room - head);
		unsigned int span = ALIGN(head + n, 4);

		memset(page, 0xff, span);
		memcpy(page + head, buf, n);
		for (i = 0; i < span && addr + i < end; i += 4)
			put_unaligned_le32(swab32(get_unaligned_le32(page + i)),
					   page + i);
		ret = sf_program(addr, page, span / 4);
		if (ret)
			return ret;
		addr += span;
		buf += n;
		len -= n;
		*retlen += n;
		head = 0;
	}
	return 0;
}

static int gk_sf_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	u32 addr = instr->addr;
	u32 end = instr->addr + instr->len;
	int ret;

	if (addr % SZ_SECTOR || instr->len % SZ_SECTOR || end > mtd->size)
		return -EINVAL;
	for (; addr < end; addr += SZ_SECTOR) {
		ret = sf_erase_sector(addr);
		if (ret) {
			instr->fail_addr = addr;
			return ret;
		}
	}
	return 0;
}

static struct spi_flash gk_flash;
static int gk_announced;

struct spi_flash *spi_flash_probe(unsigned int bus, unsigned int cs,
				  unsigned int max_hz, unsigned int spi_mode)
{
	struct spi_flash *flash = &gk_flash;
	struct mtd_info *mtd = &flash->mtd;
	u32 jedec, size;

	writel(SF_CE_DEFAULT, SF_CE);
	writel(SF_SPEED_DIV2, SF_SPEED);
	jedec = sf_reg_read(CMD_RDID, 3) & 0xffffff;
	if (!jedec || jedec == 0xffffff) {
		printf("SF: no flash (JEDEC id %06x)\n", jedec);
		return NULL;
	}
	/* The last ID byte is log2 of the size on the parts these boards carry */
	size = (jedec & 0xff) >= 0x14 && (jedec & 0xff) <= 0x18 ?
		1 << (jedec & 0xff) : SZ_8M;

	sf_unprotect(jedec);

	memset(flash, 0, sizeof(*flash));
	flash->name = "gk-sflash";
	flash->size = size;
	flash->page_size = SZ_PAGE;
	flash->sector_size = SZ_SECTOR;
	flash->erase_size = SZ_SECTOR;
	flash->erase_opcode = CMD_SE_64K;
	flash->read_opcode = CMD_READ;
	flash->program_opcode = CMD_PP;

	mtd->name = "gk-sflash";
	mtd->type = MTD_NORFLASH;
	mtd->flags = MTD_CAP_NORFLASH;
	mtd->size = size;
	mtd->erasesize = SZ_SECTOR;
	mtd->writesize = 1;
	mtd->writebufsize = SZ_PAGE;
	mtd->_read = gk_sf_read;
	mtd->_write = gk_sf_write;
	mtd->_erase = gk_sf_erase;
	mtd->priv = flash;

	/* Once: the environment and common/openipc.c probe again and again */
	if (!gk_announced++)
		printf("SF: Detected JEDEC %06x with page size %u Bytes, erase size %u KiB, total %u MiB\n",
		       jedec, SZ_PAGE, SZ_SECTOR >> 10, size >> 20);
	return flash;
}

void spi_flash_free(struct spi_flash *flash)
{
}
