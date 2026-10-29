// SPDX-License-Identifier: GPL-2.0+
/*
 * SPI NOR on the SFC of the Ingenic T-series.
 *
 * The SFC runs one flash command per transfer: opcode, address, dummy
 * cycles and data, the data through a 32-bit FIFO. The sequences are the
 * vendor U-Boot's (drivers/spi/jz_sfc_nor.c of OpenIPC/u-boot-ingenic and,
 * for the T40's SFC, drivers/mtd/devices/jz_sfc_v1/jz_sfc_nor.c of
 * OpenIPC/u-boot-t40 and of the T41 SDK, all built without
 * CONFIG_SPI_QUAD): the controller at 50 MHz, single-bit
 * READ (0x03) and PP (0x02), WREN before and RDSR polled after every
 * program and erase, 4-byte addressing switched on around each operation
 * on parts that need it, a whole-chip erase as CHIP ERASE, and the chip
 * geometry from the vendor's table by JEDEC ID.
 *
 * Like ms_sflash on openipc-sigmastar, this driver provides
 * spi_flash_probe() itself, so mainline's non-DM spi_flash_*() wrappers,
 * the sf command and the environment in SPI flash work without the SPI
 * and spi-nor layers.
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 */

#include <common.h>
#include <spi_flash.h>
#include <linux/mtd/mtd.h>
#include <linux/sizes.h>
#include <asm/io.h>
#include <asm/arch/base.h>
#include <asm/arch/clk.h>
#include <asm/arch/sfc.h>
#include <asm/arch/spi.h>

/* The vendor's clock for the SFC in U-Boot */
#define JZ_SFC_RATE		50000000

#ifdef CONFIG_CPU_XBURST2
/*
 * The T40's SFC: the same controller with the transfer mode moved to
 * TRAN_CONF1 and the fields renamed (drivers/mtd/devices/jz_sfc_v1 of
 * OpenIPC/u-boot-t40)
 */
#ifdef CONFIG_T41
/* The T41's flash is on SFC0 (drivers/mtd/devices/jz_sfc_v1 of the T41 SDK) */
#define SFC_BASE		SFC0_BASE
#else
#define SFC_CLK_ID		SFC_CLK
#endif
#define SFC_DATA		SFC_RM_DR
#define G_TRAN_DIR		GLB_TRAN_DIR
#define G_OP_MODE		GLB_OP_MODE
#define G_THRESHOLD_MSK		GLB_THRESHOLD_MSK
#define G_THRESHOLD_OFFSET	GLB_THRESHOLD_OFFSET
#define C_ADDR_WIDTH_MSK	TRAN_CONF0_ADDR_WIDTH_MSK
#define C_ADDR_WIDTH_OFFSET	TRAN_CONF0_ADDR_WIDTH_OFFSET
#define C_CMD_EN		TRAN_CONF0_CMD_EN
#define C_DATEEN		TRAN_CONF0_DATEEN
#define C_CMD_MSK		TRAN_CONF0_CMD_MSK
#define C_DMYBITS_MSK		TRAN_CONF0_DMYBITS_MSK
#define C_PHASE_FORMAT		TRAN_CONF0_PHASE_FORMAT
#define SFC_MODE_REG(n)		SFC_TRAN_CONF1(n)
#define M_TRAN_MODE_MSK		TRAN_CONF1_TRAN_MODE_MSK
#define M_TRAN_MODE_OFFSET	TRAN_CONF1_TRAN_MODE_OFFSET
#define M_SPI_STANDARD		TRAN_CONF1_SPI_STANDARD
#define T_FLUSH			TRIG_FLUSH
#define T_START			TRIG_START
#define T_STOP			TRIG_STOP
#else
#define SFC_CLK_ID		SSI
#define SFC_DATA		SFC_DR
#define G_TRAN_DIR		TRAN_DIR
#define G_OP_MODE		OP_MODE
#define G_THRESHOLD_MSK		THRESHOLD_MSK
#define G_THRESHOLD_OFFSET	THRESHOLD_OFFSET
#define C_ADDR_WIDTH_MSK	ADDR_WIDTH_MSK
#define C_ADDR_WIDTH_OFFSET	ADDR_WIDTH_OFFSET
#define C_CMD_EN		CMD_EN
#define C_DATEEN		DATEEN
#define C_CMD_MSK		CMD_MSK
#define C_DMYBITS_MSK		TRAN_CONF_DMYBITS_MSK
#define C_PHASE_FORMAT		PHASE_FORMAT
#define SFC_MODE_REG(n)		SFC_TRAN_CONF(n)
#define M_TRAN_MODE_MSK		TRAN_MODE_MSK
#define M_TRAN_MODE_OFFSET	TRAN_MODE_OFFSET
#define M_SPI_STANDARD		TRAN_SPI_STANDARD
#define T_FLUSH			FLUSH
#define T_START			START
#define T_STOP			STOP
#endif

struct jz_nor_info {
	const char *name;
	u32 id;
	u32 page_size;
	u32 sector_size;
	u32 addr_size;
	u32 size;
};

/*
 * jz_spi_support_table of the vendor's drivers/spi/jz_spi.h: the XBurst
 * one (u-boot-ingenic) or the XBurst2 one (u-boot-t40), which differ
 */
static const struct jz_nor_info jz_nor_table[] = {
#ifdef CONFIG_CPU_XBURST2
	{ "MX25L128**E",       0xc22018, 256, (64*1024), 3, (16*1024*1024) },
	{ "MX25L64**E",        0xc22017, 256, (64*1024), 3, (8*1024*1024) },
	{ "MX25L25645G/35F",   0xc22019, 256, (32*1024), 4, (32*1024*1024) },
	{ "MX25L51245G",       0xc2201a, 256, (32*1024), 4, (64*1024*1024) },
	{ "GD25Q64C",          0xc84017, 256, (64*1024), 3, (8*1024*1024) },
	{ "GD25Q128C",         0xc84018, 256, (64*1024), 3, (16*1024*1024) },
	{ "GD25Q256D/E",       0xc84019, 256, (64*1024), 4, (32*1024*1024) },
	{ "W25Q64",            0xef4017, 256, (64*1024), 3, (8*1024*1024) },
	{ "W25Q128",           0xef4018, 256, (64*1024), 3, (16*1024*1024) },
	{ "W25Q256JV",         0xef4019, 256, (32*1024), 4, (32*1024*1024) },
	{ "W25Q512JV",         0xef4020, 256, (32*1024), 4, (64*1024*1024) },
	{ "XM25QH128A",        0x207018, 256, (64*1024), 3, (16*1024*1024) },
	{ "XM25QH64A",         0x207017, 256, (64*1024), 3, (8*1024*1024) },
	{ "XM25QH128B",        0x206018, 256, (64*1024), 3, (16*1024*1024) },
	{ "XM25QH64B",         0x206017, 256, (64*1024), 3, (8*1024*1024) },
	{ "XM25QH64C",         0x204017, 256, (32*1024), 3, (8*1024*1024) },
	{ "XM25QH128C",        0x204018, 256, (32*1024), 3, (16*1024*1024) },
	{ "XM25QH256C",        0x204019, 256, (32*1024), 4, (32*1024*1024) },
	{ "XM25QU128C",        0x204118, 256, (32*1024), 3, (16*1024*1024) },
	{ "XM25QU256C",        0x204119, 256, (32*1024), 4, (32*1024*1024) },
	{ "EN25QH64",          0x1c7017, 256, (32*1024), 3, (8*1024*1024) },
	{ "EN25QH128A",        0x1c7018, 256, (32*1024), 3, (16*1024*1024) },
	{ "EN25QH256A",        0x1c7019, 256, (32*1024), 4, (32*1024*1024) },
	{ "EN25QX128A",        0x1c7118, 256, (32*1024), 3, (16*1024*1024) },
	{ "FM25Q64",           0xf83217, 256, (64*1024), 3, (8*1024*1024) },
	{ "FM25W128",          0xa12818, 256, (64*1024), 3, (16*1024*1024) },
	{ "BY25Q64AS",         0x684017, 256, (64*1024), 3, (8*1024*1024) },
	{ "BY25Q128AS",        0x684018, 256, (64*1024), 3, (16*1024*1024) },
	{ "ZB25VQ64",          0x5e4017, 256, (64*1024), 3, (8*1024*1024) },
	{ "ZB25VQ128",         0x5e4018, 256, (64*1024), 3, (16*1024*1024) },
	{ "XT25F64B",          0x0b4017, 256, (32*1024), 3, (8*1024*1024) },
	{ "XT25F128B",         0x0b4018, 256, (32*1024), 3, (16*1024*1024) },
	{ "FS25Q064F2TFI1",    0xa14017, 256, (32*1024), 3, (8*1024*1024) },
	{ "FS25Q128F2TFI",     0xa14018, 256, (64*1024), 3, (16*1024*1024) },
	{ "P25Q64H",           0x856017, 256, (32*1024), 3, (8*1024*1024) },
	{ "P25Q128H",          0x856018, 256, (64*1024), 3, (16*1024*1024) },
	{ "ZD25Q64B",          0xba3217, 256, (32*1024), 3, (8*1024*1024) },
	{ "IS25LP064D",        0x9d6017, 256, (32*1024), 3, (8*1024*1024) },
	{ "IS25LP128F",        0x9d6018, 256, (64*1024), 3, (16*1024*1024) },
	{ "SK25P128",          0x256018, 256, (64*1024), 3, (16*1024*1024) },
	{ "NM25Q128EVB",       0x522118, 256, (64*1024), 3, (16*1024*1024) },
	{ "GM25Q64A",          0x1c4017, 256, (64*1024), 3, (8*1024*1024) },
	{ "GM25Q128A",         0x1c4018, 256, (64*1024), 3, (16*1024*1024) },
	{ "PY25Q128HA",        0x852018, 256, (64*1024), 3, (16*1024*1024) },
#ifdef CONFIG_T41
	/* the T41 SDK's table has these two more */
	{ "XT25Q128D",         0x0b6018, 256, (32*1024), 3, (16*1024*1024) },
	{ "NM25Q64EVD",        0x522217, 256, (32*1024), 3, (8*1024*1024) },
#endif
#else
	{ "MX25L128**E",       0xc22018, 256, (64*1024), 3, (16*1024*1024) },
	{ "MX25L64**E",        0xc22017, 256, (64*1024), 3, (8*1024*1024) },
	{ "MX25L25645G/35F",   0xc22019, 256, (32*1024), 4, (32*1024*1024) },
	{ "MX25L51245G",       0xc2201a, 256, (32*1024), 4, (64*1024*1024) },
	{ "GD25Q64C",          0xc84017, 256, (64*1024), 3, (8*1024*1024) },
	{ "GD25Q128",          0xc84018, 256, (64*1024), 3, (16*1024*1024) },
	{ "GD25Q256D/E",       0xc84019, 256, (64*1024), 4, (32*1024*1024) },
	{ "W25Q64",            0xef4017, 256, (64*1024), 3, (8*1024*1024) },
	{ "W25Q128",           0xef4018, 256, (64*1024), 3, (16*1024*1024) },
	{ "W25Q256JV",         0xef4019, 256, (32*1024), 4, (32*1024*1024) },
	{ "XM25QH128A",        0x207018, 256, (64*1024), 3, (16*1024*1024) },
	{ "XM25QH64A",         0x207017, 256, (64*1024), 3, (8*1024*1024) },
	{ "XM25QH128B",        0x206018, 256, (64*1024), 3, (16*1024*1024) },
	{ "XM25QH64B",         0x206017, 256, (64*1024), 3, (8*1024*1024) },
	{ "XM25QH128C",        0x204018, 256, (64*1024), 3, (16*1024*1024) },
	{ "XM25QH256C",        0x204019, 256, (32*1024), 4, (32*1024*1024) },
	{ "EN25QH64",          0x1c7017, 256, (32*1024), 3, (8*1024*1024) },
	{ "EN25QH128A",        0x1c7018, 256, (32*1024), 3, (16*1024*1024) },
	{ "EN25QX128A",        0x1c7118, 256, (32*1024), 3, (16*1024*1024) },
	{ "EN25QH256A",        0x1c7019, 256, (64*1024), 4, (32*1024*1024) },
	{ "FM25Q64",           0xf83217, 256, (64*1024), 3, (8*1024*1024) },
	{ "BY25Q64AS",         0x684017, 256, (64*1024), 3, (8*1024*1024) },
	{ "BY25Q128AS",        0x684018, 256, (64*1024), 3, (16*1024*1024) },
	{ "ZB25VQ64",          0x5e4017, 256, (64*1024), 3, (8*1024*1024) },
	{ "ZB25VQ128",         0x5e4018, 256, (64*1024), 3, (16*1024*1024) },
	{ "XT25F64B",          0x0b4017, 256, (32*1024), 3, (8*1024*1024) },
	{ "XT25F128B",         0x0b4018, 256, (32*1024), 3, (16*1024*1024) },
	{ "XT25F256",          0x0b4019, 256, (64*1024), 4, (32*1024*1024) },
	{ "FS25Q064F2TFI1",    0xa14017, 256, (32*1024), 3, (8*1024*1024) },
	{ "FM25Q128A",         0xa14018, 256, (64*1024), 3, (16*1024*1024) },
	{ "FM25W128",          0xa12818, 256, (64*1024), 3, (16*1024*1024) },
	{ "P25Q64H",           0x856017, 256, (32*1024), 3, (8*1024*1024) },
	{ "P25Q128H",          0x856018, 256, (64*1024), 3, (16*1024*1024) },
	{ "GM25Q64A",          0x1c4017, 256, (64*1024), 3, (8*1024*1024) },
	{ "GM25Q128A",         0x1c4018, 256, (64*1024), 3, (16*1024*1024) },
	{ "NM25Q64EVB",        0x522217, 256, (32*1024), 3, (8*1024*1024) },
	{ "NM25Q128EVB",       0x522118, 256, (64*1024), 3, (16*1024*1024) },
	{ "ZD25Q64B",          0xba3217, 256, (64*1024), 3, (8*1024*1024) },
	{ "IS25LP064D",        0x9d6017, 256, (64*1024), 3, (8*1024*1024) },
	{ "IS25LP128F",        0x9d6018, 256, (64*1024), 3, (16*1024*1024) },
	{ "DQ25Q128AL",        0x546018, 256, (32*1024), 3, (16*1024*1024) },
	{ "XM25QH64C",         0x204017, 256, (64*1024), 3, (8*1024*1024) },
	{ "XD25Q128C",         0xd84018, 256, (32*1024), 3, (16*1024*1024) },
	{ "PY25Q128HA",        0x852018, 256, (64*1024), 3, (16*1024*1024) },
#endif
};

static u32 jz_sfc_readl(unsigned int offset)
{
	return readl((void __iomem *)SFC_BASE + offset);
}

static void jz_sfc_writel(u32 value, unsigned int offset)
{
	writel(value, (void __iomem *)SFC_BASE + offset);
}

static void jz_sfc_clrsetbits(unsigned int offset, u32 clr, u32 set)
{
	jz_sfc_writel((jz_sfc_readl(offset) & ~clr) | set, offset);
}

/* One transfer on channel 0, as the vendor's sfc_set_transfer() sets it */
static void sfc_send_cmd(u8 cmd, unsigned int len, u32 addr,
			 unsigned int addr_len, unsigned int daten, int write)
{
	if (write)
		jz_sfc_clrsetbits(SFC_GLB, 0, G_TRAN_DIR);
	else
		jz_sfc_clrsetbits(SFC_GLB, G_TRAN_DIR, 0);
	jz_sfc_writel(len, SFC_TRAN_LEN);
	jz_sfc_clrsetbits(SFC_TRAN_CONF(0), C_ADDR_WIDTH_MSK,
			  addr_len << C_ADDR_WIDTH_OFFSET);
	jz_sfc_clrsetbits(SFC_TRAN_CONF(0), 0, C_CMD_EN);
	jz_sfc_clrsetbits(SFC_TRAN_CONF(0), C_DATEEN, daten ? C_DATEEN : 0);
	jz_sfc_clrsetbits(SFC_TRAN_CONF(0), C_CMD_MSK, cmd);
	jz_sfc_writel(addr, SFC_DEV_ADDR(0));
	jz_sfc_writel(0, SFC_DEV_ADDR_PLUS(0));
	jz_sfc_clrsetbits(SFC_TRAN_CONF(0), C_DMYBITS_MSK, 0);
	jz_sfc_clrsetbits(SFC_MODE_REG(0), M_TRAN_MODE_MSK,
			  M_SPI_STANDARD << M_TRAN_MODE_OFFSET);

	jz_sfc_writel(T_FLUSH, SFC_TRIG);
	jz_sfc_writel(T_START, SFC_TRIG);

	/* Without data the transfer is over at END */
	if (!daten) {
		while (!(jz_sfc_readl(SFC_SR) & END))
			;
		jz_sfc_writel(CLR_END, SFC_SCR);
	}
}

static void sfc_wait_end(void)
{
	while (!(jz_sfc_readl(SFC_SR) & END))
		;
	jz_sfc_writel(CLR_END, SFC_SCR);
}

/* words: the FIFO moves 32 bits at a time; buf is word-aligned */
static void sfc_read_data(u32 *buf, unsigned int words)
{
	unsigned int done = 0, n, i;

	while (done < words) {
		if (!(jz_sfc_readl(SFC_SR) & RECE_REQ))
			continue;
		jz_sfc_writel(CLR_RREQ, SFC_SCR);
		n = min(words - done, (unsigned int)THRESHOLD);
		for (i = 0; i < n; i++)
			buf[done++] = jz_sfc_readl(SFC_DATA);
	}
	sfc_wait_end();
}

static void sfc_write_data(const u32 *buf, unsigned int words)
{
	unsigned int done = 0, n, i;

	while (done < words) {
		if (!(jz_sfc_readl(SFC_SR) & TRAN_REQ))
			continue;
		jz_sfc_writel(CLR_TREQ, SFC_SCR);
		n = min(words - done, (unsigned int)THRESHOLD);
		for (i = 0; i < n; i++)
			jz_sfc_writel(buf[done++], SFC_DATA);
	}
	sfc_wait_end();
}

static void sfc_wait_ready(void)
{
	u32 sr;

	do {
		sfc_send_cmd(CMD_RDSR, 1, 0, 0, 1, 0);
		sfc_read_data(&sr, 1);
	} while (sr & CMD_SR_WIP);
}

/* The vendor's sfc_init(), run at every probe */
static void sfc_init(void)
{
	int i;

#ifdef CONFIG_T41
	/* both SFCs, as the T41 SDK with SFC0_NOR and SFC1_NOR */
	clk_set_rate(SFC0, JZ_SFC_RATE);
	clk_set_rate(SFC1, JZ_SFC_RATE);
#else
	clk_set_rate(SFC_CLK_ID, JZ_SFC_RATE);
#endif

#ifdef CONFIG_CPU_XBURST2
	/* The T40's: write protect off, no sample delay, no burst mode */
	jz_sfc_clrsetbits(SFC_GLB, G_TRAN_DIR | G_OP_MODE | GLB_WP_EN, 0);
	jz_sfc_clrsetbits(SFC_DEV_CONF, DEV_CONF_CMD_TYPE | DEV_CONF_CPHA |
			  DEV_CONF_CPOL | DEV_CONF_SMP_DELAY_MSK |
			  DEV_CONF_THOLD_MSK | DEV_CONF_TSETUP_MSK |
			  DEV_CONF_TSH_MSK,
			  DEV_CONF_CEDL | DEV_CONF_HOLDDL | DEV_CONF_WPDL);
	for (i = 0; i < 6; i++) {
		jz_sfc_clrsetbits(SFC_TRAN_CONF(i), C_PHASE_FORMAT, 0);
		jz_sfc_clrsetbits(SFC_TRAN_CONF1(i), M_TRAN_MODE_MSK, 0);
	}
	jz_sfc_writel(CLR_END | CLR_TREQ | CLR_RREQ | CLR_OVER | CLR_UNDR,
		      SFC_INTC);
	jz_sfc_writel(0, SFC_CGE);
	jz_sfc_clrsetbits(SFC_GLB, G_THRESHOLD_MSK | GLB_BURST_MD_MSK,
			  THRESHOLD << G_THRESHOLD_OFFSET);
#else
	jz_sfc_clrsetbits(SFC_GLB, TRAN_DIR | OP_MODE, WP_EN);
	jz_sfc_clrsetbits(SFC_DEV_CONF, CMD_TYPE | CPHA | CPOL |
			  SMP_DELAY_MSK | THOLD_MSK | TSETUP_MSK | TSH_MSK,
			  CEDL | HOLDDL | WPDL | 1 << SMP_DELAY_OFFSET);
	for (i = 0; i < 6; i++)
		jz_sfc_clrsetbits(SFC_TRAN_CONF(i),
				  TRAN_MODE_MSK | PHASE_FORMAT, 0);
	jz_sfc_writel(CLR_END | CLR_TREQ | CLR_RREQ | CLR_OVER | CLR_UNDR,
		      SFC_INTC);
	jz_sfc_writel(0, SFC_CGE);
	jz_sfc_clrsetbits(SFC_GLB, THRESHOLD_MSK, THRESHOLD << THRESHOLD_OFFSET);
#endif
}

/* 4-byte addressing on, or off, around each operation on 32 MiB parts */
static void sfc_address_mode(struct spi_flash *flash, int on)
{
	if (flash->addr_width != 4)
		return;
	sfc_send_cmd(CMD_WREN, 0, 0, 0, 0, 1);
	sfc_send_cmd(on ? CMD_EN4B : CMD_EX4B, 0, 0, 0, 0, 1);
	sfc_wait_ready();
}

/*
 * The vendor read the whole length in one transfer into the buffer word by
 * word, which wrote up to 3 bytes past a buffer of odd length and faulted
 * on an unaligned one. Whole words go straight into an aligned buffer; the
 * rest through a bounce buffer, a chunk per transfer.
 */
static u32 sfc_bounce[SZ_4K / 4];

static void sfc_read_chunk(struct spi_flash *flash, u32 from, size_t len,
			   u32 *buf)
{
	sfc_send_cmd(CMD_READ, len, from, flash->addr_width, 1, 0);
	sfc_read_data(buf, DIV_ROUND_UP(len, 4));
}

static int jz_sfc_read(struct mtd_info *mtd, loff_t from, size_t len,
		       size_t *retlen, u_char *buf)
{
	struct spi_flash *flash = mtd->priv;
	size_t whole, n;

	if (from + len > flash->size)
		return -EINVAL;

	jz_sfc_writel(T_FLUSH, SFC_TRIG);
	sfc_address_mode(flash, 1);

	whole = IS_ALIGNED((ulong)buf, 4) ? len & ~3 : 0;
	if (whole)
		sfc_read_chunk(flash, from, whole, (u32 *)buf);
	for (n = whole; n < len; n += sizeof(sfc_bounce)) {
		size_t chunk = min(len - n, sizeof(sfc_bounce));

		sfc_read_chunk(flash, from + n, chunk, sfc_bounce);
		memcpy(buf + n, sfc_bounce, chunk);
	}

	sfc_address_mode(flash, 0);
	*retlen = len;
	return 0;
}

static int jz_sfc_write(struct mtd_info *mtd, loff_t to, size_t len,
			size_t *retlen, const u_char *buf)
{
	struct spi_flash *flash = mtd->priv;
	size_t n;

	if (to + len > flash->size) {
		printf("Data write overflow this chip\n");
		return -EINVAL;
	}

	jz_sfc_writel(T_FLUSH, SFC_TRIG);
	sfc_address_mode(flash, 1);

	*retlen = 0;
	while (len) {
		/* up to the end of the page, as the vendor splits it */
		n = min(len, (size_t)(flash->page_size - (u32)to % flash->page_size));
		memcpy(sfc_bounce, buf, n);

		sfc_send_cmd(CMD_WREN, 0, 0, 0, 0, 1);
		sfc_send_cmd(CMD_PP, n, to, flash->addr_width, 1, 1);
		sfc_write_data(sfc_bounce, DIV_ROUND_UP(n, 4));
		sfc_wait_ready();

		to += n;
		buf += n;
		len -= n;
		*retlen += n;
	}

	sfc_address_mode(flash, 0);
	return 0;
}

static int jz_sfc_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	struct spi_flash *flash = mtd->priv;
	u32 offset = instr->addr, len = instr->len;
	u32 erase_size = flash->sector_size;
	u8 op;

	if (offset % CONFIG_SFC_MIN_ALIGN || len % CONFIG_SFC_MIN_ALIGN) {
		printf("SF: erase must be aligned to 0x%x\n",
		       CONFIG_SFC_MIN_ALIGN);
		return -EINVAL;
	}
	if (offset + len > flash->size) {
		printf("len plus offset more than flash size!\n");
		return -EINVAL;
	}

	jz_sfc_writel(T_FLUSH, SFC_TRIG);

	if (len == flash->size) {
		sfc_send_cmd(CMD_WREN, 0, 0, 0, 0, 1);
		sfc_send_cmd(CMD_ERASE_CE, 0, 0, 0, 0, 1);
		sfc_wait_ready();
		return 0;
	}

	sfc_address_mode(flash, 1);
	if (len < SZ_32K)
		erase_size = SZ_4K;
	while (len) {
		switch (erase_size) {
		case SZ_4K:
			op = CMD_ERASE_4K;
			break;
		case SZ_32K:
			op = CMD_ERASE_32K;
			break;
		default:
			op = CMD_ERASE_64K;
			break;
		}
		sfc_send_cmd(CMD_WREN, 0, 0, 0, 0, 1);
		sfc_send_cmd(op, 0, offset, flash->addr_width, 0, 1);
		sfc_wait_ready();

		offset += erase_size;
		len -= erase_size;
		if (erase_size != SZ_4K && len < erase_size)
			erase_size = SZ_4K;
	}
	sfc_address_mode(flash, 0);
	return 0;
}

static struct spi_flash jz_flash;
static int jz_announced;

struct spi_flash *spi_flash_probe(unsigned int bus, unsigned int cs,
				  unsigned int max_hz, unsigned int spi_mode)
{
	struct spi_flash *flash = &jz_flash;
	struct mtd_info *mtd = &flash->mtd;
	const struct jz_nor_info *info = NULL;
	static struct jz_nor_info unknown;
	u32 id = 0;
	int i;

	sfc_init();

	jz_sfc_writel(T_STOP, SFC_TRIG);
	jz_sfc_writel(T_FLUSH, SFC_TRIG);
	sfc_send_cmd(CMD_RDID, 3, 0, 0, 1, 0);
	sfc_read_data(&id, 1);
	/* the FIFO word holds the ID bytes as they came, first in bits 7:0 */
	id = (id & 0xff) << 16 | (id & 0xff00) | (id >> 16 & 0xff);
	if (!id || id == 0xffffff) {
		printf("SF: no flash (JEDEC id %06x)\n", id);
		return NULL;
	}

	for (i = 0; i < ARRAY_SIZE(jz_nor_table); i++)
		if (jz_nor_table[i].id == id) {
			info = &jz_nor_table[i];
			break;
		}
	if (!info) {
		/*
		 * Not in the table. The vendor driver then kept the last
		 * entry's geometry (16 MiB); take the size from the JEDEC
		 * capacity byte instead.
		 */
		u8 cap = id & 0xff;

		unknown.name = "unknown";
		unknown.id = id;
		unknown.page_size = 256;
		unknown.sector_size = SZ_64K;
		unknown.size = cap >= 0x14 && cap <= 0x1a ? 1 << cap : SZ_8M;
		unknown.addr_size = unknown.size > SZ_16M ? 4 : 3;
		info = &unknown;
	}

	memset(flash, 0, sizeof(*flash));
	flash->name = info->name;
	flash->size = info->size;
	flash->page_size = info->page_size;
	flash->sector_size = info->sector_size;
	flash->erase_size = CONFIG_SFC_MIN_ALIGN;
	flash->addr_width = info->addr_size;

	mtd->name = "jz_sfc";
	mtd->type = MTD_NORFLASH;
	mtd->flags = MTD_CAP_NORFLASH;
	mtd->size = info->size;
	mtd->erasesize = CONFIG_SFC_MIN_ALIGN;
	mtd->writesize = 1;
	mtd->writebufsize = info->page_size;
	mtd->_read = jz_sfc_read;
	mtd->_write = jz_sfc_write;
	mtd->_erase = jz_sfc_erase;
	mtd->priv = flash;

	/* Once: the environment and common/openipc.c probe again and again */
	if (!jz_announced++)
		printf("SF: Detected %s (JEDEC %06x) with page size %u Bytes, erase size %u KiB, total %u MiB\n",
		       info->name, id, info->page_size,
		       CONFIG_SFC_MIN_ALIGN >> 10, info->size >> 20);
	return flash;
}

void spi_flash_free(struct spi_flash *flash)
{
}
