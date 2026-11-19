// SPDX-License-Identifier: GPL-2.0+
/*
 * SPI NAND reads for the first stage: page read to cache, read from cache
 * (0x03, or 0x6b on four lines), ECC status, and bad block skipping for
 * the U-Boot area (UBOOT_MAX_BLOCKS good blocks from UBOOT_FLASH_ADDR).
 */
#include "cmn.h"
#include "spi.h"
#include "spinand.h"
#include "timer.h"

#define SPINAND_CMD_RESET		0xff
#define SPINAND_CMD_GET_FEATURE		0x0f
#define SPINAND_CMD_SET_FEATURE		0x1f
#define SPINAND_CMD_PAGE_READ		0x13
#define SPINAND_CMD_READ_CACHE		0x03
#define SPINAND_CMD_READ_CACHE_X4	0x6b
#define REG_CFG				0xb0
#define REG_STATUS			0xc0
#define CFG_QUAD_ENABLE			0x01
#define STATUS_BUSY			0x01
#define STATUS_ECC_MASK			0x30
#define STATUS_ECC_NONE			0x00
#define STATUS_ECC_CORRECTED		0x10
#define STATUS_ECC_UNCOR		0x20

/* Geometry, as log2; the boot header's flash_cfg overrides them. */
static int page_shift = 11;
static int block_shift = 17;
static int oob_shift = 6;
static int bbm_bytes = 1;
static int planes = 1;
static int good_blocks[UBOOT_MAX_BLOCKS] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};
static u32 cfg_byte1 = 0x35, cfg_byte2 = 0x31, cfg_byte3 = 1;
static u32 geometry_set;
static u32 quad_read;

/*
 * flash_cfg: [1:0] page size 2 KiB << n, [4:2] block size 64 KiB << n,
 * [6:5] OOB size 64 << n, [7] two bad block marker bytes, [31:8] three
 * more bytes the NAND path does not use.
 */
void spinand_set_geometry(u32 cfg)
{
	page_shift = (cfg & 3) + 11;
	block_shift = (cfg >> 2 & 7) + 16;
	oob_shift = (cfg >> 5 & 3) + 6;
	bbm_bytes = (cfg & 0x80) ? 2 : 1;
	cfg_byte1 = cfg >> 8 & 0xff;
	cfg_byte2 = cfg >> 16 & 0xff;
	cfg_byte3 = cfg >> 24;
	geometry_set = 1;
}

static int get_feature(u8 reg, u8 *val)
{
	struct spi_mem_op op = {
		.cmd = { 1, SPINAND_CMD_GET_FEATURE },
		.addr = { 1, 1, reg },
		.dummy = { 0, 0 },
		.data = { 1, SPI_MEM_DATA_IN, 1, val },
	};

	return spi_mem_exec_op(&op);
}

static int set_feature(u8 reg, u8 val)
{
	u8 buf[2] = { val, reg };
	struct spi_mem_op op = {
		.cmd = { 1, SPINAND_CMD_SET_FEATURE },
		.addr = { 1, 1, reg },
		.dummy = { 0, 0 },
		.data = { 1, SPI_MEM_DATA_OUT, 1, buf },
	};

	return spi_mem_exec_op(&op);
}

static int read_status(u8 *st)
{
	return get_feature(REG_STATUS, st);
}

static int update_cfg(u8 mask, u8 val)
{
	u8 cfg;
	int ret = get_feature(REG_CFG, &cfg);

	if (ret)
		return ret;
	cfg = val | (cfg & ~mask);
	ret = set_feature(REG_CFG, cfg);
	if (ret)
		return ret;
	return 0;
}

static int load_page(u32 page)
{
	struct spi_mem_op op = {
		.cmd = { 1, SPINAND_CMD_PAGE_READ },
		.addr = { 3, 1, page },
		.dummy = { 0, 0 },
		.data = { 0, SPI_MEM_DATA_OUT, 0, NULL },
	};

	return spi_mem_exec_op(&op);
}

static int read_from_cache(void *buf, u32 column, u32 len)
{
	struct spi_mem_op op = {
		.cmd = { 1, quad_read ? SPINAND_CMD_READ_CACHE_X4 : SPINAND_CMD_READ_CACHE },
		.addr = { 2, 1, 0 },
		.dummy = { 1, 1 },
		.data = { quad_read ? 4 : 1, SPI_MEM_DATA_IN, 0, NULL },
	};
	u8 *p = buf;
	int ret;

	op.addr.val = column;
	while (len) {
		op.data.buf = p;
		op.data.nbytes = len;
		ret = spi_mem_adjust_op_size(&op);
		if (ret)
			return ret;
		ret = spi_mem_exec_op(&op);
		if (ret)
			return ret;
		p += op.data.nbytes;
		len -= op.data.nbytes;
		op.addr.val += op.data.nbytes;
	}
	return 0;
}

static int wait_ready(u8 *status)
{
	int timeout = 10000;
	u8 st;
	int ret;

	do {
		ret = read_status(&st);
		if (ret)
			return ret;
		if (!(st & STATUS_BUSY))
			goto done;
	} while (timeout-- != 0);
	ret = read_status(&st);
	if (ret)
		return ret;
done:
	if (status)
		*status = st;
	return (st & STATUS_BUSY) ? -1 : 0;
}

static int spinand_reset(void)
{
	struct spi_mem_op op = {
		.cmd = { 1, SPINAND_CMD_RESET },
		.data = { 0, SPI_MEM_DATA_OUT, 0, NULL },
	};
	int ret = spi_mem_exec_op(&op);

	if (ret)
		return ret;
	udelay(1000);
	return wait_ready(NULL);
}

static int check_ecc_status(u8 st)
{
	switch (st & STATUS_ECC_MASK) {
	case STATUS_ECC_NONE:
	case STATUS_ECC_CORRECTED:
		return 0;
	default:
		return -1;
	}
}

static int fls(u32 x)
{
	int r = 32;

	if (!x)
		return 0;
	if (!(x & 0xffff0000)) {
		x <<= 16;
		r = 16;
	}
	if (!(x & 0xff000000)) {
		x <<= 8;
		r -= 8;
	}
	if (!(x & 0xf0000000)) {
		x <<= 4;
		r -= 4;
	}
	if (!(x & 0xc0000000)) {
		x <<= 2;
		r -= 2;
	}
	if (!(x & 0x80000000))
		r -= 1;
	return r;
}

/* Two-plane parts: the plane bit sits above the column. */
static void select_plane(u32 addr, u32 *column)
{
	if (planes > 1)
		*column |= ((addr >> block_shift) % planes) << fls(1 << page_shift);
}

static int read_page(u32 addr, void *buf, u32 len, int oob)
{
	u32 page = addr >> page_shift;
	u32 column;
	u8 st;
	int ret;

	if (!oob) {
		column = addr & ((1 << page_shift) - 1);
	} else {
		column = 1 << page_shift;
		if (len > (u32)(1 << oob_shift))
			return -1;
	}
	if (!oob && column + len > (u32)(1 << page_shift))
		return -1;
	ret = load_page(page);
	if (ret)
		return ret;
	ret = wait_ready(&st);
	if (ret < 0)
		return ret;
	select_plane(addr, &column);
	ret = read_from_cache(buf, column, len);
	if (ret)
		return ret;
	return check_ecc_status(st);
}

static int is_bad_block(u32 addr)
{
	char oob[64];
	int ret, i;

	ret = read_page(addr & -(1 << block_shift), oob, 1 << oob_shift, 1);
	if (ret)
		return ret;
	for (i = 0; i < bbm_bytes; i++)
		if (oob[i] != (char)0xff)
			return 1;
	return 0;
}

/* Logical U-Boot area address to the flash address, skipping bad blocks. */
static int map_addr(u32 addr)
{
	u32 blk = addr >> block_shift;
	u32 off = addr & ((1 << block_shift) - 1);
	u32 first, cur;
	int i;

	if (((1 << block_shift) - 1) & UBOOT_FLASH_ADDR) {
		info("\r\nuboot flash addr 0x%X is not blk size 0x%X aligned\n",
		     UBOOT_FLASH_ADDR, 1 << block_shift);
		return -1;
	}
	first = UBOOT_FLASH_ADDR >> block_shift;
	if (blk < first || blk >= first + UBOOT_MAX_BLOCKS)
		return -1;
	if (good_blocks[blk - first] != -1)
		return off + (good_blocks[blk - first] << block_shift);
	for (i = 0; i < UBOOT_MAX_BLOCKS && good_blocks[i] != -1; i++)
		;
	if (i == 0)
		cur = first << block_shift;
	else
		cur = (good_blocks[i - 1] + 1) << block_shift;
	while (first + i <= blk) {
		if (is_bad_block(cur) == 0)
			good_blocks[i++] = cur >> block_shift;
		else
			info("\r\neba %d is bad\n", cur >> block_shift);
		cur += 1 << block_shift;
	}
	return off + (good_blocks[blk - first] << block_shift);
}

static int read_in_block(u32 addr, u32 len, u8 *buf)
{
	u32 done = 0, n;
	int ret;

	while (done < len) {
		n = (1 << page_shift) - ((done + addr) & ((1 << page_shift) - 1));
		if (len - done <= n)
			n = len - done;
		ret = read_page(done + addr, buf + done, n, 0);
		if (ret < 0)
			return ret;
		done += n;
	}
	return 0;
}

int spinand_read(u32 addr, u32 len, void *buf)
{
	u32 done = 0, n;
	int phys, ret;

	while (done < len) {
		phys = map_addr(done + addr);
		if (phys < 0)
			return -1;
		n = (1 << block_shift) - (phys & ((1 << block_shift) - 1));
		if (len - done <= n)
			n = len - done;
		ret = read_in_block(phys, n, (u8 *)buf + done);
		if (ret < 0)
			return ret;
		done += n;
	}
	return 0;
}

static int enable_quad(int on)
{
	quad_read = on != 0;
	return update_cfg(CFG_QUAD_ENABLE, on != 0);
}

int spinand_init(u32 hz, int buswidth)
{
	int ret;

	spi_init(hz);
	ret = spinand_reset();
	if (ret)
		return ret;
	if (buswidth == 4) {
		ret = enable_quad(1);
		if (ret)
			return ret;
	}
	return 0;
}
