// SPDX-License-Identifier: GPL-2.0+
/*
 * SPI NAND on the Xiongmai XM530/XM550 flash controller (xmsfc v2).
 *
 * The controller runs SPI NAND commands itself through four operations:
 * OP1 an addressed command without data (page read to the chip's cache,
 * program execute, block erase, the row in ADDR), OP2 a bare command
 * (reset), OP3 a read into the 4 KiB RAM at 0x100F6000 or R_SR_ID, OP4 a
 * write out of the RAM or SRW. A page and its spare area pass through the
 * RAM whole, at their column offsets.
 *
 * From the vendor U-Boot 2014.04 drivers/mtd/nand/xmsfc_v2_nand.c, as a
 * legacy raw NAND driver: cmdfunc instead of the vendor's byte-by-byte
 * cmd_ctrl, a full-ID table instead of a patch to nand_base.c, and the
 * part's on-die ECC status (feature C0) reported through ecc.read_page.
 */

#include <common.h>
#include <malloc.h>
#include <nand.h>
#include <asm/io.h>
#include <asm/unaligned.h>
#include <asm/arch/xm.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/mtd/rawnand.h>
#include <linux/sizes.h>

#define V2_CTRL			(XM_SFC_BASE + 0x00)
#define V2_CFG1			(XM_SFC_BASE + 0x04)
#define V2_CFG2			(XM_SFC_BASE + 0x08)
#define V2_ADDR			(XM_SFC_BASE + 0x0C)
#define V2_OP			(XM_SFC_BASE + 0x10)
#define V2_CMD			(XM_SFC_BASE + 0x14)
#define V2_SRW			(XM_SFC_BASE + 0x18)
#define V2_RW_NUM		(XM_SFC_BASE + 0x1C)
#define V2_TDEL			(XM_SFC_BASE + 0x28)
#define V2_ST			(XM_SFC_BASE + 0x2C)
#define V2_R_SR_ID		(XM_SFC_BASE + 0x30)
#define V2_RAM_SIZE		0x1000

/*
 * CFG1: bit 0 WREN first, bit 1 the reset delay, bit 2 address, bit 3
 * dummy byte. CFG2: bit 0 data through the RAM, bit 1 poll until ready.
 */
#define CFG1_WREN		BIT(0)
#define CFG1_TDEL		BIT(1)
#define CFG1_ADDR		BIT(2)
#define CFG1_DUMMY		BIT(3)
#define CFG2_RAM		BIT(0)
#define CFG2_POLL		BIT(1)

#define SPINAND_GET_FEATURE	0x0F
#define SPINAND_SET_FEATURE	0x1F
#define SPINAND_PROG_EXEC	0x10
#define SPINAND_PAGE_READ	0x13
#define SPINAND_PROG_LOAD	0x02
#define SPINAND_READ_CACHE	0x03
#define SPINAND_READ_ID		0x9F
#define SPINAND_BLOCK_ERASE	0xD8
#define SPINAND_RESET		0xFF

#define FEAT_PROT		0xA0
#define FEAT_STATUS		0xC0
#define STATUS_OIP		BIT(0)
#define STATUS_E_FAIL		BIT(2)
#define STATUS_P_FAIL		BIT(3)
#define STATUS_ECC_MASK		(3 << 4)
#define STATUS_ECC_FIXED	BIT(4)

/* The NAND bus: pads 93, 94 and 96-99 (NOR uses 9-14) */
static const u8 xmsfc_nand_pads[] = { 93, 94, 96, 97, 98, 99 };

/* Parts in the vendor table (xmsfc_nand_params.c), by their full ID */
static struct nand_flash_dev xmsfc_nand_ids[] = {
	{ "W25N01GV", { .id = { 0xef, 0xaa, 0x21 } },
	  SZ_2K, SZ_128, SZ_128K, 0, 3, 64 },
	{ "W25N02GV", { .id = { 0xef, 0xab, 0x21 } },
	  SZ_2K, SZ_256, SZ_128K, 0, 3, 64 },
	{ "GD5F1GQ4U", { .id = { 0xc8, 0xd1 } },
	  SZ_2K, SZ_128, SZ_128K, 0, 2, 128 },
	{ NULL }
};

struct xmsfc_nand {
	struct nand_chip chip;
	u8 buf[V2_RAM_SIZE];	/* the page and its spare area */
	unsigned int pos;	/* read_byte/read_buf/write_buf position */
	int page;		/* page of the pending program */
	u8 id[8];
	u8 status;		/* feature C0 after the last page read */
	unsigned int last_cmd;
};

static struct xmsfc_nand xmsfc_nand;

/*
 * Run one controller operation; a controller that never finishes is
 * reported, not waited on forever
 */
static int xmsfc_run(u32 op)
{
	ulong start = get_timer(0);

	writel(op, V2_OP);
	while (!(readl(V2_ST) & BIT(op - 1))) {
		if (get_timer(start) > 100) {
			printf("xmsfc-nand: operation %u timed out\n", op);
			return -ETIMEDOUT;
		}
	}
	return 0;
}

static void xmsfc_bus_mode(void)
{
	writel(CFG1_ADDR, V2_CFG1);
	writel(CFG2_POLL, V2_CFG2);
}

static u8 xmsfc_get_feature(u8 reg)
{
	writel(CFG1_ADDR, V2_CFG1);
	writel(0, V2_CFG2);
	writel(SPINAND_GET_FEATURE, V2_CMD);
	writel(reg, V2_ADDR);
	writel(0, V2_RW_NUM);
	xmsfc_run(3);
	return readl(V2_R_SR_ID) & 0xff;
}

static void xmsfc_set_feature(u8 reg, u8 val)
{
	writel(CFG1_ADDR, V2_CFG1);
	writel(0, V2_CFG2);
	writel(SPINAND_SET_FEATURE, V2_CMD);
	writel(reg, V2_ADDR);
	writel(val, V2_SRW);
	writel(0, V2_RW_NUM);
	xmsfc_run(4);
}

/*
 * Wait for the part to finish (feature C0 OIP clear), at most 100 ms --
 * a block erase takes a few
 */
static int xmsfc_wait_ready(void)
{
	ulong start = get_timer(0);

	do {
		if (!(xmsfc_get_feature(FEAT_STATUS) & STATUS_OIP))
			return 0;
		udelay(1);
	} while (get_timer(start) < 100);
	printf("xmsfc-nand: still busy after 100 ms\n");
	return -ETIMEDOUT;
}

/* An addressed command without data: page read, program execute, erase */
static int xmsfc_row_cmd(u8 cmd, u32 row, u32 cfg1)
{
	int ret;

	writel(cfg1, V2_CFG1);
	writel(CFG2_POLL, V2_CFG2);
	writel(cmd, V2_CMD);
	writel(row, V2_ADDR);
	ret = xmsfc_run(1);
	return ret ? ret : xmsfc_wait_ready();
}

static void xmsfc_read_page(struct mtd_info *mtd, int page)
{
	struct xmsfc_nand *host = &xmsfc_nand;
	const unsigned int len = mtd->writesize + mtd->oobsize;

	if (xmsfc_row_cmd(SPINAND_PAGE_READ, page, CFG1_ADDR)) {
		/* Report the page as uncorrectable rather than stale data */
		memset(host->buf, 0xff, len);
		host->status = STATUS_ECC_MASK;
		xmsfc_bus_mode();
		return;
	}
	host->status = xmsfc_get_feature(FEAT_STATUS);

	writel(CFG1_ADDR | CFG1_DUMMY, V2_CFG1);
	writel(CFG2_RAM, V2_CFG2);
	writel(SPINAND_READ_CACHE, V2_CMD);
	writel(0, V2_ADDR);
	writel(len - 1, V2_RW_NUM);
	if (xmsfc_run(3)) {
		memset(host->buf, 0xff, len);
		host->status = STATUS_ECC_MASK;
	} else {
		memcpy_fromio(host->buf, (void *)XM_SFC_RAM, len);
	}
	xmsfc_bus_mode();
}

static int xmsfc_program_page(struct mtd_info *mtd, int page)
{
	struct xmsfc_nand *host = &xmsfc_nand;
	const unsigned int len = mtd->writesize + mtd->oobsize;
	unsigned int i;
	u8 status;

	/* Unprotect, as the vendor does before every program and erase */
	xmsfc_set_feature(FEAT_PROT, 0);

	for (i = 0; i < len; i += 4)
		writel(get_unaligned_le32(host->buf + i), XM_SFC_RAM + i);
	writel(CFG1_WREN | CFG1_ADDR, V2_CFG1);
	writel(CFG2_RAM, V2_CFG2);
	writel(SPINAND_PROG_LOAD, V2_CMD);
	writel(0, V2_ADDR);
	writel(len - 1, V2_RW_NUM);
	if (xmsfc_run(4) ||
	    xmsfc_row_cmd(SPINAND_PROG_EXEC, page, CFG1_ADDR)) {
		xmsfc_bus_mode();
		return NAND_STATUS_FAIL;
	}
	status = xmsfc_get_feature(FEAT_STATUS);
	xmsfc_bus_mode();
	return status & STATUS_P_FAIL ? NAND_STATUS_FAIL : 0;
}

static int xmsfc_erase_block(int page)
{
	u8 status;

	xmsfc_set_feature(FEAT_PROT, 0);
	if (xmsfc_row_cmd(SPINAND_BLOCK_ERASE, page, CFG1_WREN | CFG1_ADDR)) {
		xmsfc_bus_mode();
		return NAND_STATUS_FAIL;
	}
	status = xmsfc_get_feature(FEAT_STATUS);
	xmsfc_bus_mode();
	return status & STATUS_E_FAIL ? NAND_STATUS_FAIL : 0;
}

static void xmsfc_read_id(void)
{
	struct xmsfc_nand *host = &xmsfc_nand;
	u32 id;

	/* 9F, one address (dummy) byte, three ID bytes: first byte highest */
	writel(CFG1_ADDR, V2_CFG1);
	writel(0, V2_CFG2);
	writel(SPINAND_READ_ID, V2_CMD);
	writel(0, V2_ADDR);
	writel(2, V2_RW_NUM);
	memset(host->id, 0, sizeof(host->id));
	if (xmsfc_run(3)) {
		/* No ID: nand_scan_ident() then finds no chip */
		xmsfc_bus_mode();
		return;
	}
	id = readl(V2_R_SR_ID);
	host->id[0] = id >> 16;
	host->id[1] = id >> 8;
	host->id[2] = id;
	xmsfc_bus_mode();
}

static void xmsfc_reset(void)
{
	writel(CFG1_TDEL, V2_CFG1);
	writel(0, V2_CFG2);
	writel(0x19700, V2_TDEL);
	writel(SPINAND_RESET, V2_CMD);
	xmsfc_run(2);
	xmsfc_wait_ready();
	xmsfc_bus_mode();
}

static int xmsfc_status;

static void xmsfc_cmdfunc(struct mtd_info *mtd, unsigned int command,
			  int column, int page)
{
	struct xmsfc_nand *host = &xmsfc_nand;

	host->last_cmd = command;
	switch (command) {
	case NAND_CMD_RESET:
		xmsfc_reset();
		break;
	case NAND_CMD_READID:
		xmsfc_read_id();
		host->pos = 0;
		break;
	case NAND_CMD_PARAM:
		/* No ONFI parameter page on SPI NAND */
		memset(host->buf, 0, 256);
		host->pos = 0;
		break;
	case NAND_CMD_READ0:
		xmsfc_read_page(mtd, page);
		host->pos = column;
		break;
	case NAND_CMD_READOOB:
		xmsfc_read_page(mtd, page);
		host->pos = mtd->writesize + column;
		break;
	case NAND_CMD_SEQIN:
		memset(host->buf, 0xff, mtd->writesize + mtd->oobsize);
		host->page = page;
		host->pos = column;
		break;
	case NAND_CMD_PAGEPROG:
		xmsfc_status = xmsfc_program_page(mtd, host->page);
		break;
	case NAND_CMD_ERASE1:
		xmsfc_status = xmsfc_erase_block(page);
		break;
	case NAND_CMD_ERASE2:
	case NAND_CMD_STATUS:
		break;
	default:
		printf("xmsfc-nand: command 0x%x not supported\n", command);
		break;
	}
}

static u8 xmsfc_read_byte(struct mtd_info *mtd)
{
	struct xmsfc_nand *host = &xmsfc_nand;

	switch (host->last_cmd) {
	case NAND_CMD_READID:
		return host->pos < sizeof(host->id) ? host->id[host->pos++] : 0;
	case NAND_CMD_STATUS:
		/* Write protect off, ready, and the last program/erase result */
		return NAND_STATUS_WP | NAND_STATUS_READY | xmsfc_status;
	default:
		return host->pos < sizeof(host->buf) ? host->buf[host->pos++] : 0xff;
	}
}

static void xmsfc_read_buf(struct mtd_info *mtd, u8 *buf, int len)
{
	struct xmsfc_nand *host = &xmsfc_nand;

	len = min_t(int, len, sizeof(host->buf) - host->pos);
	memcpy(buf, host->buf + host->pos, len);
	host->pos += len;
}

static void xmsfc_write_buf(struct mtd_info *mtd, const u8 *buf, int len)
{
	struct xmsfc_nand *host = &xmsfc_nand;

	len = min_t(int, len, sizeof(host->buf) - host->pos);
	memcpy(host->buf + host->pos, buf, len);
	host->pos += len;
}

static int xmsfc_waitfunc(struct mtd_info *mtd, struct nand_chip *chip)
{
	return NAND_STATUS_WP | NAND_STATUS_READY | xmsfc_status;
}

static void xmsfc_select_chip(struct mtd_info *mtd, int cs)
{
}

/* The part corrects on die; report what feature C0 said about the page */
static int xmsfc_ecc_read_page(struct mtd_info *mtd, struct nand_chip *chip,
			       u8 *buf, int oob_required, int page)
{
	struct xmsfc_nand *host = &xmsfc_nand;

	chip->read_buf(mtd, buf, mtd->writesize);
	if (oob_required)
		chip->read_buf(mtd, chip->oob_poi, mtd->oobsize);

	switch (host->status & STATUS_ECC_MASK) {
	case 0:
		return 0;
	case STATUS_ECC_FIXED:
		mtd->ecc_stats.corrected++;
		return 1;
	default:
		mtd->ecc_stats.failed++;
		return 0;
	}
}

static int xmsfc_ecc_write_page(struct mtd_info *mtd, struct nand_chip *chip,
				const u8 *buf, int oob_required, int page)
{
	chip->write_buf(mtd, buf, mtd->writesize);
	if (oob_required)
		chip->write_buf(mtd, chip->oob_poi, mtd->oobsize);
	return 0;
}

void board_nand_init(void)
{
	struct nand_chip *chip = &xmsfc_nand.chip;
	struct mtd_info *mtd = nand_to_mtd(chip);
	int i;

	for (i = 0; i < ARRAY_SIZE(xmsfc_nand_pads); i++)
		writel(XM_GPIO_DRIVE_8MA | XM_GPIO_MUX2_EN,
		       XM_GPIO_PAD(xmsfc_nand_pads[i]));
	writel(0x00, V2_CTRL);	/* NAND */

	chip->cmdfunc = xmsfc_cmdfunc;
	chip->read_byte = xmsfc_read_byte;
	chip->read_buf = xmsfc_read_buf;
	chip->write_buf = xmsfc_write_buf;
	chip->waitfunc = xmsfc_waitfunc;
	chip->select_chip = xmsfc_select_chip;
	chip->options = NAND_NO_SUBPAGE_WRITE;

	if (nand_scan_ident(mtd, 1, xmsfc_nand_ids))
		return;

	chip->ecc.mode = NAND_ECC_HW;
	chip->ecc.size = mtd->writesize;
	chip->ecc.bytes = 0;
	chip->ecc.strength = 1;
	chip->ecc.read_page = xmsfc_ecc_read_page;
	chip->ecc.write_page = xmsfc_ecc_write_page;

	if (nand_scan_tail(mtd))
		return;
	nand_register(0, mtd);
}
