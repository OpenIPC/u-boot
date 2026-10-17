// SPDX-License-Identifier: GPL-2.0+
/*
 * Xiongmai SPI NOR flash controller ("xmsfc"): v1 on the XM510, v2 on the
 * XM530/XM550.
 *
 * The controller runs whole SPI transactions itself: software loads the
 * opcode, address and data, writes an operation number to OP and polls ST
 * for that operation's done bit. The array is also mapped read-only through
 * a window (0x0 on v1, 0x60000000 on v2) that U-Boot executes from before
 * relocation, and reads here are a copy out of it.
 *
 * Ported from the vendor U-Boot 2014.04 drivers/mtd/spi/sf_probe.c, which
 * replaced spi_flash_probe() with this controller, and from the vendor
 * kernel's drivers/mtd/devices/xmsfc. Unlike the vendor code it:
 *  - writes the v2 data RAM and the v1 buffer in whole words only, padding
 *    a partial last word with 0xff (the vendor U-Boot corrupts the tail of
 *    a write whose length is not 64K-aligned);
 *  - polls the status register after a v1 erase or program, as the kernel
 *    driver does;
 *  - undoes the per-block write protection that Xiongmai bootloaders leave
 *    enabled (SR3.WPS), which otherwise makes erase and program silently do
 *    nothing.
 */

#include <common.h>
#include <malloc.h>
#include <spi_flash.h>
#include <watchdog.h>
#include <asm/io.h>
#include <asm/arch/xm.h>
#include <linux/delay.h>
#include <linux/mtd/mtd.h>
#include <linux/sizes.h>

#define SZ_PAGE			256
#define SZ_SECTOR		0x10000

#define CMD_WRSR		0x01
#define CMD_PP			0x02
#define CMD_READ		0x03
#define CMD_RDSR		0x05
#define CMD_WRSR3		0x11
#define CMD_RDSR3		0x15
#define CMD_RDSR2		0x35
#define CMD_GBULK		0x98	/* global block/sector unlock */
#define CMD_RDID		0x9F
#define CMD_SE_64K		0xD8

#define SR1_WIP			BIT(0)
#define SR1_BP			0x7C	/* Winbond-style: BP0-2, TB, SEC */
#define SR1_BP_MXIC		0x3C	/* Macronix and others: BP0-3, bit 6 is QE */
#define SR3_WPS			BIT(2)

#define MFR_WINBOND		0xef
#define MFR_XMC			0x20
#define MFR_GIGADEVICE		0xc8

#define OP_DONE(op)		BIT((op) - 1)

#ifdef CONFIG_XM_SFC_V2

#define V2_CTRL			(XM_SFC_BASE + 0x00)
#define V2_CFG1			(XM_SFC_BASE + 0x04)
#define V2_CFG2			(XM_SFC_BASE + 0x08)
#define V2_ADDR			(XM_SFC_BASE + 0x0C)
#define V2_OP			(XM_SFC_BASE + 0x10)
#define V2_CMD			(XM_SFC_BASE + 0x14)
#define V2_SRW			(XM_SFC_BASE + 0x18)
#define V2_RW_NUM		(XM_SFC_BASE + 0x1C)
#define V2_PST_CMD		(XM_SFC_BASE + 0x24)
#define V2_ST			(XM_SFC_BASE + 0x2C)
#define V2_R_SR_ID		(XM_SFC_BASE + 0x30)

/*
 * CFG1 shapes the transaction: bit 0 sends WREN first, bit 2 an address,
 * bit 3 a dummy byte. CFG2 = 2 makes the controller poll the status
 * register (PST_CMD) until the flash is no longer busy.
 */
#define CFG1_WREN		BIT(0)
#define CFG1_ADDR		BIT(2)
#define CFG2_POLL		0x02

/* GPIO pads 9-14 carry the flash bus */
#define PAD_FIRST		9
#define PAD_LAST		14

static void xmsfc_run(u32 op)
{
	writel(op, V2_OP);
	while (!(readl(V2_ST) & OP_DONE(op)))
		;
}

/* Back to bus reads: the window, and so instruction fetch, works again */
static void xmsfc_bus_mode(void)
{
	writel(CFG1_ADDR, V2_CFG1);
	writel(CFG2_POLL, V2_CFG2);
}

static void xmsfc_hw_init(void)
{
	int pad;

	for (pad = PAD_FIRST; pad <= PAD_LAST; pad++)
		writel(XM_GPIO_DRIVE_8MA | XM_GPIO_MUX2_EN, XM_GPIO_PAD(pad));
	writel(0x80, V2_CTRL);	/* NOR */
}

static u32 xmsfc_read_reg(u8 cmd, unsigned int nbytes)
{
	u32 v;

	writel(0, V2_CFG1);
	writel(0, V2_CFG2);
	writel(cmd, V2_CMD);
	writel(nbytes - 1, V2_RW_NUM);
	xmsfc_run(3);
	v = readl(V2_R_SR_ID);
	xmsfc_bus_mode();
	return v;
}

static void xmsfc_write_reg(u8 cmd, u32 val, unsigned int nbytes)
{
	writel(CFG1_WREN, V2_CFG1);
	writel(0, V2_CFG2);
	writel(val, V2_SRW);
	writel(cmd, V2_CMD);
	writel(nbytes - 1, V2_RW_NUM);
	xmsfc_run(4);
	/* A status write is a program cycle: wait it out like one */
	while (xmsfc_read_reg(CMD_RDSR, 1) & SR1_WIP)
		;
}

static void xmsfc_cmd(u8 cmd)
{
	writel(CFG1_WREN, V2_CFG1);
	writel(0, V2_CFG2);
	writel(cmd, V2_CMD);
	xmsfc_run(1);
	xmsfc_bus_mode();
}

static void xmsfc_erase_sector(u32 addr)
{
	writel(CFG1_WREN | CFG1_ADDR, V2_CFG1);
	writel(CFG2_POLL, V2_CFG2);
	writel(CMD_RDSR, V2_PST_CMD);
	writel(CMD_SE_64K, V2_CMD);
	writel(addr, V2_ADDR);
	xmsfc_run(1);
	xmsfc_bus_mode();
}

/* Program up to one page, which must not cross a page boundary */
static void xmsfc_program(u32 addr, const u8 *buf, unsigned int len)
{
	unsigned int i;

	for (i = 0; i < len; i += 4) {
		u32 w = 0xffffffff;

		memcpy(&w, buf + i, min(4u, len - i));
		writel(w, XM_SFC_RAM + i);
	}
	writel(CFG1_WREN | CFG1_ADDR, V2_CFG1);
	writel(CFG2_POLL, V2_CFG2);
	writel(CMD_RDSR, V2_PST_CMD);
	writel(CMD_PP, V2_CMD);
	writel(addr, V2_ADDR);
	writel(len - 1, V2_RW_NUM);
	xmsfc_run(4);
	xmsfc_bus_mode();
}

#else /* v1 */

#define V1_ADDR			(XM_SFC_BASE + 0x04)
#define V1_OP			(XM_SFC_BASE + 0x08)
#define V1_CMD			(XM_SFC_BASE + 0x0C)
#define V1_SRW			(XM_SFC_BASE + 0x10)
#define V1_WRNUM		(XM_SFC_BASE + 0x14)
#define V1_BSEL			(XM_SFC_BASE + 0x18)
#define V1_ST			(XM_SFC_BASE + 0x38)
#define V1_SRR			(XM_SFC_BASE + 0x3C)
#define V1_BUF			(XM_SFC_BASE + 0x400)	/* one byte per word */

static void xmsfc_run(u32 op)
{
	writel(op, V1_OP);
	while (!(readl(V1_ST) & OP_DONE(op)))
		;
}

static void xmsfc_bus_mode(void)
{
}

static void xmsfc_hw_init(void)
{
}

static u32 xmsfc_read_reg(u8 cmd, unsigned int nbytes)
{
	writel(cmd, V1_CMD);
	writel(nbytes - 1, V1_BSEL);
	xmsfc_run(3);
	return readl(V1_SRR);
}

static void xmsfc_write_reg(u8 cmd, u32 val, unsigned int nbytes)
{
	writel(val, V1_SRW);
	writel(cmd, V1_CMD);
	writel(nbytes - 1, V1_BSEL);
	xmsfc_run(4);
	/* A status write is a program cycle: wait it out like one */
	while (xmsfc_read_reg(CMD_RDSR, 1) & SR1_WIP)
		;
}

/* v1 has no command-only operation that the vendor code documents */
static void xmsfc_cmd(u8 cmd)
{
}

static void xmsfc_wait_ready(void)
{
	while (xmsfc_read_reg(CMD_RDSR, 1) & SR1_WIP)
		;
}

static void xmsfc_erase_sector(u32 addr)
{
	writel(addr, V1_ADDR);
	writel(CMD_SE_64K, V1_CMD);
	xmsfc_run(2);
	xmsfc_wait_ready();
}

static void xmsfc_program(u32 addr, const u8 *buf, unsigned int len)
{
	unsigned int i;

	for (i = 0; i < len; i++)
		writel(buf[i], V1_BUF + 4 * i);
	writel(addr, V1_ADDR);
	writel(len - 1, V1_WRNUM);
	writel(CMD_PP, V1_CMD);
	xmsfc_run(6);
	xmsfc_wait_ready();
}

#endif

/* Parts on Xiongmai boards, from the vendor sf_params.c tables */
static const struct {
	const char *name;
	u32 jedec;
} xmsfc_ids[] = {
	{ "W25Q64",	0xef4017 },
	{ "W25Q128",	0xef4018 },
	{ "W25Q64DW",	0xef6017 },
	{ "W25Q128FW",	0xef6018 },
	{ "MX25L64",	0xc22017 },
	{ "MX25L128",	0xc22018 },
	{ "XM25Q64",	0x207017 },
	{ "XM25QH128",	0x207018 },
	{ "GD25Q64",	0xc84017 },
	{ "GD25Q128",	0xc84018 },
	{ "EN25QH64",	0x1c7017 },
	{ "EN25QH128",	0x1c7018 },
	{ "FS25Q64",	0xa14017 },
	{ "XT25F64",	0x0b4017 },
	{ "XT25F128",	0x0b4018 },
};

static struct spi_flash xmsfc_flash;
static int xmsfc_announced;

/*
 * Xiongmai bootloaders set SR3.WPS: protection then comes from per-block
 * lock bits, all set at power-up, instead of the BP bits, and every erase
 * and program is silently dropped. Unlock the blocks, return the part to
 * BP protection, and clear the BP bits too.
 *
 * Status layouts differ by maker. Winbond and XMC have SR3 and WPS; they
 * and GigaDevice keep QE in SR2, written as the second byte of WRSR. On
 * the others (Macronix, Eon, ...) bit 6 of SR1 is QE, 0x35 means
 * something else (EQIO on Macronix), and a second WRSR byte reaches a
 * configuration register: clear BP0-3 alone, with a one-byte WRSR.
 */
static void xmsfc_unprotect(u32 jedec)
{
	const u8 mfr = jedec >> 16;
	const bool sr2_qe = mfr == MFR_WINBOND || mfr == MFR_XMC ||
			    mfr == MFR_GIGADEVICE;
	u32 sr1, sr2, sr3;

	if (mfr == MFR_WINBOND || mfr == MFR_XMC) {
		sr3 = xmsfc_read_reg(CMD_RDSR3, 1) & 0xff;
		if (sr3 != 0xff && (sr3 & SR3_WPS)) {
			printf("SF: per-block protection (SR3 %02x), unlocking\n",
			       sr3);
			xmsfc_cmd(CMD_GBULK);
			xmsfc_write_reg(CMD_WRSR3, sr3 & ~SR3_WPS, 1);
		}
	}

	sr1 = xmsfc_read_reg(CMD_RDSR, 1) & 0xff;
	if (sr1 == 0xff)
		return;
	if (sr2_qe && (sr1 & SR1_BP)) {
		sr2 = xmsfc_read_reg(CMD_RDSR2, 1) & 0xff;
		printf("SF: block protection (SR1 %02x), clearing\n", sr1);
		xmsfc_write_reg(CMD_WRSR, (sr1 & ~SR1_BP) << 8 | sr2, 2);
	} else if (!sr2_qe && (sr1 & SR1_BP_MXIC)) {
		printf("SF: block protection (SR1 %02x), clearing\n", sr1);
		xmsfc_write_reg(CMD_WRSR, sr1 & ~SR1_BP_MXIC, 1);
	}
}

static int xmsfc_mtd_read(struct mtd_info *mtd, loff_t from, size_t len,
			  size_t *retlen, u_char *buf)
{
	if (from + len > mtd->size)
		return -EINVAL;
	xmsfc_bus_mode();
	memcpy(buf, (void *)XM_FLASH_BASE + (ulong)from, len);
	*retlen = len;
	return 0;
}

static int xmsfc_mtd_write(struct mtd_info *mtd, loff_t to, size_t len,
			   size_t *retlen, const u_char *buf)
{
	u32 addr = to;

	if (to + len > mtd->size)
		return -EINVAL;
	*retlen = 0;
	while (len) {
		unsigned int n = min_t(size_t, len,
				       SZ_PAGE - (addr & (SZ_PAGE - 1)));

		xmsfc_program(addr, buf, n);
		addr += n;
		buf += n;
		len -= n;
		*retlen += n;
	}
	return 0;
}

static int xmsfc_mtd_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	u32 addr = instr->addr;
	u32 end = instr->addr + instr->len;

	if (addr % SZ_SECTOR || instr->len % SZ_SECTOR || end > mtd->size)
		return -EINVAL;
	for (; addr < end; addr += SZ_SECTOR) {
		xmsfc_erase_sector(addr);
		WATCHDOG_RESET();
	}
	return 0;
}

struct spi_flash *spi_flash_probe(unsigned int bus, unsigned int cs,
				  unsigned int max_hz, unsigned int spi_mode)
{
	struct spi_flash *flash = &xmsfc_flash;
	struct mtd_info *mtd = &flash->mtd;
	const char *name = NULL;
	u32 jedec, size;
	int i;

	xmsfc_hw_init();
	jedec = xmsfc_read_reg(CMD_RDID, 3) & 0xffffff;
	if (!jedec || jedec == 0xffffff) {
		printf("SF: no flash (JEDEC id %06x)\n", jedec);
		return NULL;
	}

	for (i = 0; i < ARRAY_SIZE(xmsfc_ids); i++)
		if (xmsfc_ids[i].jedec == jedec)
			name = xmsfc_ids[i].name;
	/* The last ID byte is log2 of the size on every part listed */
	size = (jedec & 0xff) >= 0x14 && (jedec & 0xff) <= 0x19 ?
		1 << (jedec & 0xff) : SZ_8M;
	if (!name) {
		printf("SF: unknown JEDEC id %06x, assuming %u MiB\n", jedec,
		       size >> 20);
		name = "unknown";
	}

	xmsfc_unprotect(jedec);

	memset(flash, 0, sizeof(*flash));
	flash->name = name;
	flash->size = size;
	flash->page_size = SZ_PAGE;
	flash->sector_size = SZ_SECTOR;
	flash->erase_size = SZ_SECTOR;
	flash->erase_opcode = CMD_SE_64K;
	flash->read_opcode = CMD_READ;
	flash->program_opcode = CMD_PP;

	mtd->name = "xmsfc";
	mtd->type = MTD_NORFLASH;
	mtd->flags = MTD_CAP_NORFLASH;
	mtd->size = size;
	mtd->erasesize = SZ_SECTOR;
	mtd->writesize = 1;
	mtd->writebufsize = SZ_PAGE;
	mtd->_read = xmsfc_mtd_read;
	mtd->_write = xmsfc_mtd_write;
	mtd->_erase = xmsfc_mtd_erase;
	mtd->priv = flash;

	xmsfc_bus_mode();
	/* Once: the environment and common/openipc.c probe again and again */
	if (!xmsfc_announced++)
		printf("SF: Detected %s (%06x) with page size %u Bytes, erase size %u KiB, total %u MiB\n",
	       name, jedec, SZ_PAGE, SZ_SECTOR >> 10, size >> 20);
	return flash;
}

void spi_flash_free(struct spi_flash *flash)
{
}
