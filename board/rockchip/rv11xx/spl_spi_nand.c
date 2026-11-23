// SPDX-License-Identifier: GPL-2.0+
/*
 * The SPL's loader for SPI NAND: u-boot.itb from the boot partition, read
 * page by page through the SFC's spi-mem operations, skipping bad blocks.
 * Mainline v2022.10 has no SPI-NAND loader for the SPL, and its spi-nand
 * driver needs the whole MTD stack.
 *
 * The commands are the ones every SPI NAND shares (JEDEC-style, as
 * mainline's drivers/mtd/nand/spi/core.c issues them): PAGE READ (0x13,
 * a 24-bit row) into the cache, GET FEATURE (0x0f) on the status register
 * (0xc0: bit 0 busy, bits 5:4 ECC: 2 uncorrectable), READ FROM CACHE
 * (0x03, a 16-bit column, a dummy byte). The geometry is the 2 KiB page,
 * 64-page block of the RV11xx cameras' parts (Winbond W25N01GV, GigaDevice,
 * XTX, ...). A block is bad when the first byte of its first page's spare
 * area (column 2048) is not 0xff, the factory marker.
 */
#include <common.h>
#include <dm.h>
#include <image.h>
#include <spi.h>
#include <spi-mem.h>
#include <spl.h>
#include <config.h>

#define SPINAND_PAGE		2048
#define SPINAND_BLOCK		(64 * SPINAND_PAGE)

#define CMD_PAGE_READ		0x13
#define CMD_READ_CACHE		0x03
#define CMD_GET_FEATURE		0x0f
#define REG_STATUS		0xc0
#define STATUS_BUSY		BIT(0)
#define STATUS_ECC_MASK		(3 << 4)
#define STATUS_ECC_UNCOR	(2 << 4)

static struct spi_slave *spinand;

static int spinand_status(u8 *sr)
{
	struct spi_mem_op op =
		SPI_MEM_OP(SPI_MEM_OP_CMD(CMD_GET_FEATURE, 1),
			   SPI_MEM_OP_ADDR(1, REG_STATUS, 1),
			   SPI_MEM_OP_NO_DUMMY,
			   SPI_MEM_OP_DATA_IN(1, sr, 1));

	return spi_mem_exec_op(spinand, &op);
}

/* Page @row into the cache; 0, or -EBADMSG past the ECC */
static int spinand_load_page(u32 row)
{
	struct spi_mem_op op =
		SPI_MEM_OP(SPI_MEM_OP_CMD(CMD_PAGE_READ, 1),
			   SPI_MEM_OP_ADDR(3, row, 1),
			   SPI_MEM_OP_NO_DUMMY,
			   SPI_MEM_OP_NO_DATA);
	ulong start = get_timer(0);
	u8 sr;
	int ret;

	ret = spi_mem_exec_op(spinand, &op);
	if (ret)
		return ret;
	do {
		ret = spinand_status(&sr);
		if (ret)
			return ret;
		if (get_timer(start) > 10)
			return -ETIMEDOUT;
	} while (sr & STATUS_BUSY);

	return (sr & STATUS_ECC_MASK) == STATUS_ECC_UNCOR ? -EBADMSG : 0;
}

static int spinand_read_cache(u32 col, void *buf, u32 len)
{
	struct spi_mem_op op =
		SPI_MEM_OP(SPI_MEM_OP_CMD(CMD_READ_CACHE, 1),
			   SPI_MEM_OP_ADDR(2, col, 1),
			   SPI_MEM_OP_DUMMY(1, 1),
			   SPI_MEM_OP_DATA_IN(len, buf, 1));

	while (len) {
		u32 n = len;
		int ret;

		/* the controller may take less in one operation */
		op.data.nbytes = n;
		ret = spi_mem_adjust_op_size(spinand, &op);
		if (ret)
			return ret;
		n = op.data.nbytes;
		ret = spi_mem_exec_op(spinand, &op);
		if (ret)
			return ret;
		op.addr.val += n;
		op.data.buf.in += n;
		len -= n;
	}
	return 0;
}

static bool spinand_block_bad(u32 block)
{
	u8 marker;

	if (spinand_load_page(block * (SPINAND_BLOCK / SPINAND_PAGE)) ||
	    spinand_read_cache(SPINAND_PAGE, &marker, 1))
		return true;
	return marker != 0xff;
}

/* Bytes [@from, @from + @len) of the image that starts at RV11XX_NAND_ITB,
 * the bad blocks before and in it skipped */
static ulong spinand_read(struct spl_load_info *load, ulong from, ulong len,
			  void *buf)
{
	u32 block = RV11XX_NAND_ITB / SPINAND_BLOCK;
	u32 skip = from / SPINAND_BLOCK;
	u8 *p = buf;
	ulong left = len;

	/* the good block that holds byte @from */
	for (;;) {
		if (block * SPINAND_BLOCK >= RV11XX_NAND_BOOT_SIZE)
			return 0;
		if (!spinand_block_bad(block)) {
			if (!skip)
				break;
			skip--;
		}
		block++;
	}
	from %= SPINAND_BLOCK;
	while (left) {
		u32 page = from / SPINAND_PAGE;
		u32 col = from % SPINAND_PAGE;
		u32 n = min_t(ulong, left, SPINAND_PAGE - col);

		if (spinand_load_page(block * (SPINAND_BLOCK / SPINAND_PAGE) +
				      page) ||
		    spinand_read_cache(col, p, n))
			return 0;
		p += n;
		left -= n;
		from += n;
		if (from == SPINAND_BLOCK && left) {
			do {
				block++;
				if (block * SPINAND_BLOCK >= RV11XX_NAND_BOOT_SIZE)
					return 0;
			} while (spinand_block_bad(block));
			from = 0;
		}
	}
	return len;
}

static int spl_spi_nand_load_image(struct spl_image_info *spl_image,
				   struct spl_boot_device *bootdev)
{
	struct spl_load_info load = {
		.bl_len = 1,
		.read = spinand_read,
	};
	struct image_header *header;
	struct udevice *bus;
	int ret;

	ret = _spi_get_bus_and_cs(0, 0, 50000000, SPI_MODE_0,
				  "spi_generic_drv", "spinand", &bus,
				  &spinand);
	if (ret) {
		printf("SPI NAND: no SPI slave (%d)\n", ret);
		return ret;
	}
	header = spl_get_load_buffer(-sizeof(*header), SPINAND_PAGE);
	if (spinand_read(&load, 0, SPINAND_PAGE, header) != SPINAND_PAGE)
		return -EIO;
	if (image_get_magic(header) != FDT_MAGIC) {
		printf("SPI NAND: no u-boot.itb at 0x%x\n", RV11XX_NAND_ITB);
		return -ENOENT;
	}
	return spl_load_simple_fit(spl_image, &load, 0, header);
}
SPL_LOAD_IMAGE_METHOD("SPI NAND", 0, BOOT_DEVICE_SPI, spl_spi_nand_load_image);
