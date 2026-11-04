// SPDX-License-Identifier: GPL-2.0+
/*
 * SPI NAND behind the flash controller of the Novatek NA51090 (and
 * NA51103, NA51089: the same controller).
 *
 * The controller is run as for SPI NOR (drivers/mtd/spi/nvt_sflash.c):
 * one operation at a time, a command with up to four address bytes, then
 * N bytes in or out, the chip held selected by hand across the two. The
 * SPI NAND commands go out that way, byte for byte: PAGE DATA READ (0x13)
 * into the chip's cache, GET FEATURE (0x0F) for its status, READ (0x03)
 * of the cache, PROGRAM LOAD (0x02) and PROGRAM EXECUTE (0x10), BLOCK
 * ERASE (0xD8). The controller's NAND mode, which the vendor U-Boot
 * uses (drivers/mtd/nvt_flash_spi), folds these into page operations of
 * its own; the plain path needs nothing chip-specific from it.
 *
 * The chip's on-die ECC is on, as the vendor runs it: a page read reports
 * corrected and uncorrectable errors in the status register (C0 [5:4]),
 * which this driver hands to the MTD layer. A whole page and its spare
 * area go through one buffer, by DMA. The raw NAND framework sees a
 * large-page chip with hardware ECC; bad blocks are marked as on any
 * large-page NAND (spare byte 0).
 *
 * Non-DM, SYS_NAND_SELF_INIT: board_nand_init() registers nand0.
 */

#include <common.h>
#include <cpu_func.h>
#include <malloc.h>
#include <nand.h>
#include <asm/cache.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/libfdt.h>
#include <linux/mtd/rawnand.h>
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
#define FL_DATAPORT		(NVT_FLASH_BASE + 0x100)
#define FL_DATALEN		(NVT_FLASH_BASE + 0x104)
#define FL_FIFO_STS		(NVT_FLASH_BASE + 0x108)
#define FL_FIFO_CTRL		(NVT_FLASH_BASE + 0x10C)
#define FL_DMASTART		(NVT_FLASH_BASE + 0x110)

#define MOD_CYC_MASK		0xf
#define MOD_ROW_CYC(n)		(((n) - 1) << 2)
#define MOD_COL_CYC(n)		((n) - 1)
#define MOD_NOR			(BIT(19) | BIT(20))	/* plain SPI */
#define DLL1_SW_RESET		BIT(0)
#define DLL1_SAMPCLK_INV	BIT(2)
#define DLL1_SRC_CLK_SEL	BIT(3)
#define DLL1_PAD_CLK_SEL	BIT(20)
#define DLL2_INDLY(n)		((n) << 24)
#define DLL2_INDLY_MASK		DLL2_INDLY(0x3f)
#define CFG_CS_MANUAL		BIT(1)
#define CFG_CS_SEL		BIT(3)
#define CFG_CS_HIGH		BIT(4)
#define CFG_BUS_WIDTH_MASK	(0x3 << 9)
#define CFG_PULL_WPHLD		BIT(12)
#define CTRL0_OPER_EN		BIT(12)
#define CTRL0_WP_EN		BIT(13)
#define STS_COMP		BIT(12)
#define FIFO_EN			BIT(0)
#define FIFO_DMA		BIT(1)
#define FIFO_WRITE		BIT(2)
#define FIFO_CNT_MASK		0x1f
#define FIFO_EMPTY		BIT(8)
#define FIFO_FULL		BIT(9)
#define FIFO_BYTES		64

#define OP_CMD			0x07
#define OP_CMD_ADDR		0x0C
#define OP_READ_N		0x18
#define OP_WRITE_N		0x19

#define OP_TIMEOUT_MS		4500

#ifdef CONFIG_NVT_NA511XX
#define NVT_CG_FLASH_CLKEN	(NVT_CG_BASE + 0x74)
#define CG_FLASH_CLKEN		BIT(10)
#define NVT_CG_FLASH_RSTN	(NVT_CG_BASE + 0x98)
#define CG_FLASH_RSTN		BIT(1)
#define NVT_CG_FLASH_DIV	(NVT_CG_BASE + 0x48)	/* [21:16] */
#define CG_FLASH_DIV_SHIFT	16
#define NVT_PAD_FLASH_DRV	(NVT_PAD_BASE + 0x50)
#define NVT_TOP_FLASH_EN	(NVT_TOP_BASE + 0x04)
#define NVT_TOP_GPIO_C		(NVT_TOP_BASE + 0xA0)
#endif
#define NVT_FLASH_SRC_HZ	480000000
/* The NT98633 NVR's tree: /nand clock-frequency 24 MHz */
#define SNAND_HZ		24000000

/* SPI NAND commands */
#define SN_RESET		0xFF
#define SN_READ_ID		0x9F
#define SN_GET_FEATURE		0x0F
#define SN_SET_FEATURE		0x1F
#define SN_WREN			0x06
#define SN_PAGE_READ		0x13
#define SN_READ_CACHE		0x03
#define SN_PROG_LOAD		0x02
#define SN_PROG_EXEC		0x10
#define SN_BLOCK_ERASE		0xD8

#define FEAT_PROT		0xA0
#define FEAT_CFG		0xB0
#define FEAT_STATUS		0xC0
#define CFG_ECC_EN		BIT(4)
#define CFG_BUF			BIT(3)		/* Winbond: buffer read mode */
#define STS_OIP			BIT(0)
#define STS_E_FAIL		BIT(2)
#define STS_P_FAIL		BIT(3)
#define STS_ECC_MASK		(3 << 4)
#define STS_ECC_CORRECTED	(1 << 4)
#define STS_ECC_FAILED		(2 << 4)

#define MFR_WINBOND		0xEF

/* The parts the NVRs and their SDK list, 2 KiB pages, one plane */
static struct nand_flash_dev nvt_spinand_ids[] = {
	{"W25N01GV", {.id = {0xef, 0xaa, 0x21} }, SZ_2K, 128, SZ_128K, 0, 3,
	 64, NAND_ECC_INFO(1, SZ_512) },
	{"W25N02KV", {.id = {0xef, 0xaa, 0x22} }, SZ_2K, 256, SZ_128K, 0, 3,
	 128, NAND_ECC_INFO(8, SZ_512) },
	{"GD5F1GQ4UB", {.id = {0xc8, 0xd1} }, SZ_2K, 128, SZ_128K, 0, 2,
	 128, NAND_ECC_INFO(8, SZ_512) },
	{"GD5F1GQ5UE", {.id = {0xc8, 0x51} }, SZ_2K, 128, SZ_128K, 0, 2,
	 128, NAND_ECC_INFO(4, SZ_512) },
	{"MX35LF1GE4AB", {.id = {0xc2, 0x12} }, SZ_2K, 128, SZ_128K, 0, 2,
	 64, NAND_ECC_INFO(4, SZ_512) },
	{"XT26G01C", {.id = {0x0b, 0x11} }, SZ_2K, 128, SZ_128K, 0, 2,
	 128, NAND_ECC_INFO(8, SZ_512) },
	{NULL}
};

static struct {
	struct nand_chip chip;
	u8 *buf;		/* a page and its spare area */
	unsigned int pos, len;	/* what read_byte/read_buf/write_buf use */
	int page;		/* in the chip's cache, or -1 */
	int prog_page;		/* SEQIN's page */
	u8 status;		/* NAND_CMD_STATUS: the last program/erase */
	u8 ecc;			/* STS_ECC_* of the last page read */
	u8 id[8];
} sn;

/* ---- the controller, as nvt_sflash.c runs it ---- */

static void fl_addr_cycles(unsigned int n)
{
	clrsetbits_le32(FL_MODULE0, MOD_CYC_MASK,
			MOD_ROW_CYC(n) | MOD_COL_CYC(1));
}

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

static void fl_start(u32 cmd)
{
	writel(cmd, FL_COMMAND);
	setbits_le32(FL_CTRL0, CTRL0_OPER_EN);
}

static int fl_wait(void)
{
	ulong start = get_timer(0);

	while (!(readl(FL_CTRL_STS) & STS_COMP)) {
		if (get_timer(start) > OP_TIMEOUT_MS) {
			printf("NAND: the flash controller timed out\n");
			return -ETIMEDOUT;
		}
	}
	writel(STS_COMP, FL_CTRL_STS);
	return 0;
}

/* A command and @n address bytes (MSB first), then nothing */
static int fl_cmd(u8 cmd, unsigned int n, u32 addr)
{
	if (n) {
		fl_addr_cycles(n);
		fl_setup(OP_CMD_ADDR, addr, 0, 0);
	} else {
		fl_setup(OP_CMD, 0, 0, 0);
	}
	fl_start(cmd);
	return fl_wait();
}

static int fl_fifo_wait(u32 bits, unsigned int words)
{
	ulong start = get_timer(0);
	u32 sts;

	for (;;) {
		sts = readl(FL_FIFO_STS);
		if (bits ? sts & bits : sts & FIFO_FULL ||
		    (sts & FIFO_CNT_MASK) >= words)
			return 0;
		if (get_timer(start) > OP_TIMEOUT_MS) {
			printf("NAND: the flash FIFO timed out\n");
			return -ETIMEDOUT;
		}
	}
}

static int fl_pio_in(u8 *buf, unsigned int len)
{
	while (len) {
		unsigned int n = min_t(unsigned int, len, FIFO_BYTES), i;

		if (fl_fifo_wait(0, DIV_ROUND_UP(n, 4)))
			return -ETIMEDOUT;
		for (i = 0; i < n; i += 4) {
			u32 w = readl(FL_DATAPORT);

			memcpy(buf + i, &w, min(4U, n - i));
		}
		buf += n;
		len -= n;
	}
	return fl_wait();
}

static int fl_pio_out(const u8 *buf, unsigned int len)
{
	bool tail = len % 4;

	while (len) {
		unsigned int n = min_t(unsigned int, len, FIFO_BYTES), i;

		if (fl_fifo_wait(FIFO_EMPTY, 0))
			return -ETIMEDOUT;
		for (i = 0; i < n; i += 4) {
			u32 w = 0;

			memcpy(&w, buf + i, min(4U, n - i));
			writel(w, FL_DATAPORT);
		}
		buf += n;
		len -= n;
	}
	if (tail) {
		mdelay(1);
		clrbits_le32(FL_FIFO_CTRL, FIFO_EN);
	}
	return fl_wait();
}

static void fl_cs(bool on)
{
	if (on) {
		setbits_le32(FL_SPI_CFG, CFG_CS_HIGH);
		setbits_le32(FL_SPI_CFG, CFG_CS_MANUAL);
		clrbits_le32(FL_SPI_CFG, CFG_CS_HIGH);
	} else {
		setbits_le32(FL_SPI_CFG, CFG_CS_HIGH);
		clrbits_le32(FL_SPI_CFG, CFG_CS_MANUAL);
	}
}

/*
 * A command and @n address bytes, then @len bytes in (@out NULL) or out,
 * the chip selected throughout. Whole cache lines go by DMA.
 */
static int fl_xfer(u8 cmd, unsigned int n, u32 addr, u8 *in, const u8 *out,
		   unsigned int len)
{
	bool dma = len >= ARCH_DMA_MINALIGN && !(len % ARCH_DMA_MINALIGN) &&
		   !((ulong)(in ? in : out) % ARCH_DMA_MINALIGN);
	ulong b = (ulong)(in ? in : out);
	int ret;

	fl_cs(true);
	ret = fl_cmd(cmd, n, addr);
	if (!ret && in) {
		if (dma) {
			invalidate_dcache_range(b, b + len);
			writel(b, FL_DMASTART);
			fl_setup(OP_READ_N, 0, len, FIFO_DMA);
			fl_start(cmd);
			ret = fl_wait();
			invalidate_dcache_range(b, b + len);
		} else {
			fl_setup(OP_READ_N, 0, len, 0);
			fl_start(cmd);
			ret = fl_pio_in(in, len);
		}
	} else if (!ret && out) {
		if (dma) {
			flush_dcache_range(b, b + len);
			writel(b, FL_DMASTART);
			fl_setup(OP_WRITE_N, 0, len, FIFO_DMA | FIFO_WRITE);
			fl_start(cmd);
			ret = fl_wait();
		} else {
			fl_setup(OP_WRITE_N, 0, len, FIFO_WRITE);
			fl_start(cmd);
			ret = fl_pio_out(out, len);
		}
	}
	fl_cs(false);
	return ret;
}

/* ---- the SPI NAND chip ---- */

static u8 sn_get_feature(u8 reg)
{
	u8 v;

	if (fl_xfer(SN_GET_FEATURE, 1, reg, &v, NULL, 1))
		return 0xff;
	return v;
}

static int sn_set_feature(u8 reg, u8 val)
{
	return fl_cmd(SN_SET_FEATURE, 2, reg << 8 | val);
}

/* Wait for the operation in progress; its status, or -ETIMEDOUT */
static int sn_wait(void)
{
	ulong start = get_timer(0);
	u8 sts;

	do {
		sts = sn_get_feature(FEAT_STATUS);
		if (sts != 0xff && !(sts & STS_OIP))
			return sts;
	} while (get_timer(start) < OP_TIMEOUT_MS);
	printf("NAND: the chip stays busy (status %02x)\n", sts);
	return -ETIMEDOUT;
}

/* PAGE DATA READ, then the cache: the page and its spare area */
static int sn_read_page(int page)
{
	struct mtd_info *mtd = nand_to_mtd(&sn.chip);
	unsigned int len = mtd->writesize + mtd->oobsize;
	int sts;

	sn.page = -1;
	if (fl_cmd(SN_PAGE_READ, 3, page))
		return -EIO;
	sts = sn_wait();
	if (sts < 0)
		return sts;
	sn.ecc = sts & STS_ECC_MASK;
	/* Column 0, then a dummy byte */
	if (fl_xfer(SN_READ_CACHE, 3, 0, sn.buf, NULL, len))
		return -EIO;
	sn.page = page;
	return 0;
}

/* WRITE ENABLE, PROGRAM LOAD of the whole buffer, PROGRAM EXECUTE */
static int sn_program(int page)
{
	struct mtd_info *mtd = nand_to_mtd(&sn.chip);
	unsigned int len = mtd->writesize + mtd->oobsize;
	int sts;

	sn.page = -1;
	if (fl_cmd(SN_WREN, 0, 0) ||
	    fl_xfer(SN_PROG_LOAD, 2, 0, NULL, sn.buf, len) ||
	    fl_cmd(SN_PROG_EXEC, 3, page))
		return -EIO;
	sts = sn_wait();
	if (sts < 0)
		return sts;
	return sts & STS_P_FAIL ? -EIO : 0;
}

static int sn_erase(int page)
{
	int sts;

	sn.page = -1;
	if (fl_cmd(SN_WREN, 0, 0) || fl_cmd(SN_BLOCK_ERASE, 3, page))
		return -EIO;
	sts = sn_wait();
	if (sts < 0)
		return sts;
	return sts & STS_E_FAIL ? -EIO : 0;
}

/* ---- the raw NAND framework's hooks ---- */

static void sn_cmdfunc(struct mtd_info *mtd, unsigned int cmd, int column,
		       int page)
{
	int ret;

	switch (cmd) {
	case NAND_CMD_RESET:
		fl_cmd(SN_RESET, 0, 0);
		mdelay(1);
		sn_wait();
		sn.page = -1;
		break;
	case NAND_CMD_READID:
		/* No ONFI or JEDEC parameter page here: 0x20, 0x40 read 0 */
		memset(sn.id, 0, sizeof(sn.id));
		if (column == 0)
			fl_xfer(SN_READ_ID, 1, 0, sn.id, NULL, sizeof(sn.id));
		sn.pos = 0;
		sn.len = sizeof(sn.id);
		break;
	case NAND_CMD_READ0:
	case NAND_CMD_READOOB:
		if (cmd == NAND_CMD_READOOB)
			column += mtd->writesize;
		ret = page == sn.page ? 0 : sn_read_page(page);
		if (ret) {
			/* What a failed read leaves: an erased-looking page */
			memset(sn.buf, 0xff, mtd->writesize + mtd->oobsize);
			sn.ecc = STS_ECC_FAILED;
		}
		sn.pos = column;
		sn.len = mtd->writesize + mtd->oobsize;
		break;
	case NAND_CMD_SEQIN:
		memset(sn.buf, 0xff, mtd->writesize + mtd->oobsize);
		sn.prog_page = page;
		sn.pos = column;
		sn.len = mtd->writesize + mtd->oobsize;
		break;
	case NAND_CMD_PAGEPROG:
		ret = sn_program(sn.prog_page);
		sn.status = NAND_STATUS_READY | NAND_STATUS_WP |
			    (ret ? NAND_STATUS_FAIL : 0);
		break;
	case NAND_CMD_ERASE1:
		ret = sn_erase(page);
		sn.status = NAND_STATUS_READY | NAND_STATUS_WP |
			    (ret ? NAND_STATUS_FAIL : 0);
		break;
	case NAND_CMD_ERASE2:
		break;
	case NAND_CMD_STATUS:
		sn.id[0] = sn.status;
		sn.pos = 0;
		sn.len = 1;
		break;
	default:
		printf("NAND: command %#x not handled\n", cmd);
	}
}

/* Bytes come from the ID or status, else from the page buffer */
static u8 *sn_src(void)
{
	return sn.len <= sizeof(sn.id) ? sn.id : sn.buf;
}

static u8 sn_read_byte(struct mtd_info *mtd)
{
	if (sn.pos >= sn.len)
		return 0xff;
	return sn_src()[sn.pos++];
}

static void sn_read_buf(struct mtd_info *mtd, u8 *buf, int len)
{
	int n = min_t(int, len, sn.len - sn.pos);

	if (n > 0) {
		memcpy(buf, sn_src() + sn.pos, n);
		sn.pos += n;
	} else {
		n = 0;
	}
	memset(buf + n, 0xff, len - n);
}

static void sn_write_buf(struct mtd_info *mtd, const u8 *buf, int len)
{
	int n = min_t(int, len, sn.len - sn.pos);

	if (n > 0) {
		memcpy(sn.buf + sn.pos, buf, n);
		sn.pos += n;
	}
}

static int sn_waitfunc(struct mtd_info *mtd, struct nand_chip *chip)
{
	return sn.status;
}

static void sn_select_chip(struct mtd_info *mtd, int cs)
{
}

/* The page is in the buffer: what the on-die ECC said of it */
static int sn_read_page_ecc(struct mtd_info *mtd, struct nand_chip *chip,
			    u8 *buf, int oob_required, int page)
{
	chip->read_buf(mtd, buf, mtd->writesize);
	if (oob_required)
		chip->read_buf(mtd, chip->oob_poi, mtd->oobsize);
	if (sn.ecc == STS_ECC_FAILED) {
		mtd->ecc_stats.failed++;
		return 0;
	}
	if (sn.ecc) {
		mtd->ecc_stats.corrected++;
		return chip->ecc.strength;
	}
	return 0;
}

static int sn_write_page_ecc(struct mtd_info *mtd, struct nand_chip *chip,
			     const u8 *buf, int oob_required, int page)
{
	chip->write_buf(mtd, buf, mtd->writesize);
	chip->write_buf(mtd, chip->oob_poi, mtd->oobsize);
	return 0;
}

/* A u32 property of the loader's device tree */
static u32 sn_fdt_u32(const char *path, const char *prop, u32 def)
{
	const void *fdt = nvt_loader_fdt();
	const fdt32_t *val = NULL;
	int node;

	if (fdt) {
		node = fdt_path_offset(fdt, path);
		if (node >= 0)
			val = fdt_getprop(fdt, node, prop, NULL);
	}
	return val ? fdt32_to_cpu(*val) : def;
}

/*
 * Clock, reset, pins and PHY, as the vendor's spinand.c drv_nand_reset()
 * and nand_phy_config() do on NA51090, at the tree's clock
 */
static void sn_hw_init(void)
{
	u32 hz = sn_fdt_u32("/nand", "clock-frequency", SNAND_HZ);
	u32 pins = 3, drv;

	hz = clamp_t(u32, hz, NVT_FLASH_SRC_HZ / 64, 80000000);
	setbits_le32(NVT_CG_FLASH_CLKEN, CG_FLASH_CLKEN);
	setbits_le32(NVT_CG_FLASH_RSTN, CG_FLASH_RSTN);
	writel(0x06002222, FL_TIME0);
	writel(0x7f0f, FL_TIME1);
	if (sn_fdt_u32("/top/spi", "pinmux", 0) & 0x20)
		pins |= BIT(2);
	setbits_le32(NVT_TOP_FLASH_EN, pins);
	clrbits_le32(NVT_TOP_GPIO_C, 0x3f | (pins & BIT(2) ? BIT(6) : 0));
	clrsetbits_le32(NVT_CG_FLASH_DIV, 0x3f << CG_FLASH_DIV_SHIFT,
			(NVT_FLASH_SRC_HZ / hz - 1) << CG_FLASH_DIV_SHIFT);
	drv = hz <= 24000000 ? 0x0111101 : hz <= 60000000 ? 0x0222202 :
	      0x0222203;
	clrsetbits_le32(NVT_PAD_FLASH_DRV, 0xfffffff, drv);
	setbits_le32(FL_MODULE0, MOD_NOR);
	writel(0x5f51, FL_TIME2);

	clrsetbits_le32(FL_DLL1, DLL1_SRC_CLK_SEL,
			DLL1_SAMPCLK_INV | DLL1_PAD_CLK_SEL);
	clrsetbits_le32(FL_DLL2, DLL2_INDLY_MASK, DLL2_INDLY(8));
	setbits_le32(FL_DLL1, DLL1_SW_RESET);
	while (readl(FL_DLL1) & DLL1_SW_RESET)
		;
	clrsetbits_le32(FL_SPI_CFG, CFG_BUS_WIDTH_MASK | CFG_CS_SEL,
			CFG_PULL_WPHLD);
}

void board_nand_init(void)
{
	struct nand_chip *chip = &sn.chip;
	struct mtd_info *mtd = nand_to_mtd(chip);
	u8 cfg;
	int ret;

	sn_hw_init();
	sn.page = -1;
	/* Nothing failed yet; the framework also reads WP from here */
	sn.status = NAND_STATUS_READY | NAND_STATUS_WP;
	sn.buf = memalign(ARCH_DMA_MINALIGN,
			  ALIGN(SZ_4K + 256, ARCH_DMA_MINALIGN));
	if (!sn.buf)
		return;

	chip->cmdfunc = sn_cmdfunc;
	chip->read_byte = sn_read_byte;
	chip->read_buf = sn_read_buf;
	chip->write_buf = sn_write_buf;
	chip->waitfunc = sn_waitfunc;
	chip->select_chip = sn_select_chip;
	chip->chip_delay = 0;
	chip->options = NAND_NO_SUBPAGE_WRITE;

	ret = nand_scan_ident(mtd, 1, nvt_spinand_ids);
	if (ret) {
		printf("NAND: no SPI NAND known here (ID %02x %02x %02x)\n",
		       sn.id[0], sn.id[1], sn.id[2]);
		return;
	}

	/* Writable everywhere; the on-die ECC on, Winbond's buffer mode */
	sn_set_feature(FEAT_PROT, 0);
	cfg = sn_get_feature(FEAT_CFG) | CFG_ECC_EN;
	if (sn.id[0] == MFR_WINBOND)
		cfg |= CFG_BUF;
	sn_set_feature(FEAT_CFG, cfg);

	chip->ecc.mode = NAND_ECC_HW;
	chip->ecc.size = mtd->writesize;
	chip->ecc.bytes = 0;
	chip->ecc.strength = chip->ecc_strength_ds ?: 1;
	chip->ecc.read_page = sn_read_page_ecc;
	chip->ecc.write_page = sn_write_page_ecc;

	ret = nand_scan_tail(mtd);
	if (!ret)
		ret = nand_register(0, mtd);
	if (ret)
		printf("NAND: cannot register (%d)\n", ret);
}
