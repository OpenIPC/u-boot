// SPDX-License-Identifier: GPL-2.0+
/*
 * Goke GK710x SD controller, for SD-card recovery (common/openipc.c).
 *
 * Underneath it is a standard SDHCI. On the GK7101/GK7102 it sits at
 * 0x60002000 as one. On the GK710xS the vendor drivers reach it with no
 * HAL in between, in their own layout (arch-gk7101s/gk7101_reg.h
 * REG_SDIO_*): registers in another order, 16 bits wide where SDHCI packs
 * bytes, some bits permuted. That layout is exactly what the GK7101/GK7102
 * HAL translates into the SDHCI one, so the table below is that HAL's
 * translation, recovered by running its hw_write/hw_read accessors of each
 * width under Unicorn (the read and write maps agree), with what the S
 * drivers change on top of it: the transfer-mode direction and multi-block
 * bits swapped (mach-gk710xs sd.h). The timeout, reset, host, power, gap
 * and wake-up registers are bytes with bit orders of their own; the same
 * addresses read as words are something else. The accessors put mainline's
 * SDHCI driver on top of this table.
 */

#include <common.h>
#include <malloc.h>
#include <mmc.h>
#include <sdhci.h>
#include <asm/io.h>
#include <asm/arch/gk710x.h>

#define GK_SD_FREQ_MAX		48000000

/*
 * The SD base clock: 24 MHz * fbdiv / (refdiv * pstdiv1 * pstdiv2) of the
 * core PLL, divided by the SD scaler (the vendor kernel's get_sd_freq_hz()).
 * The scaler is set here only if nothing has set it.
 */
static unsigned int gk_sd_clock(void)
{
	u32 pll = readl(GK_RCT_PLL_SD);
	u32 fbdiv, pst1, pst2, ref, div;
	ulong vco;

#ifdef CONFIG_GK710X_S
	fbdiv = pll & 0xfff;
	pst1 = (pll >> 12) & 7;
	pst2 = (pll >> 16) & 7;
	ref = (pll >> 20) & 0x3f;
#else
	/* The GK7101/GK7102 HAL moves the fields on the way in */
	fbdiv = (pll >> 8) & 0xfff;
	pst1 = (pll >> 20) & 7;
	pst2 = (pll >> 24) & 7;
	ref = pll & 0x3f;
#endif
	if (!fbdiv || !pst1 || !pst2 || !ref)
		return GK_SD_FREQ_MAX;
	vco = 24000000UL / ref * fbdiv / (pst1 * pst2);

	div = readl(GK_RCT_SCALER_SD);
	if (!div) {
		div = DIV_ROUND_UP(vco, GK_SD_FREQ_MAX);
		writel(div, GK_RCT_SCALER_SD);
	}
	return vco / div;
}

#ifdef CONFIG_GK710X_S
/*
 * One vendor register: its offset and width, the SDHCI register it is, and
 * for each of its bits the SDHCI bit (-1: the bit does not exist there).
 * A register not listed is the identity: SDHCI offsets from 0x44 up.
 */
struct gk_sd_reg {
	u8 off, width, std;
	s8 bit[32];
};

#define ID8	{ 0, 1, 2, 3, 4, 5, 6, 7 }
#define ID16	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }
#define ID32	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, \
		  16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 }
#define N	-1

static const struct gk_sd_reg gk_sd_regs[] = {
	{ 0x00, 32, SDHCI_DMA_ADDRESS, ID32 },
	{ 0x04, 16, SDHCI_TRANSFER_MODE,
	  { 1, 2, 0, N, 5, 4, N, N, N, N, N, N, N, N, N, N } },
	{ 0x06, 16, SDHCI_SIGNAL_ENABLE, ID16 },
	{ 0x08, 16, SDHCI_SIGNAL_ENABLE + 2,
	  { 0, 2, 3, 4, 1, 5, 6, 7, 8, N, N, N, N, N, 14, 15 } },
	{ 0x0a, 16, SDHCI_BLOCK_COUNT, ID16 },
	{ 0x0c, 16, SDHCI_BLOCK_SIZE, ID16 },
	{ 0x0e, 16, SDHCI_INT_ENABLE, ID16 },
	{ 0x10, 16, SDHCI_INT_ENABLE + 2,
	  { 0, 2, 1, 3, 5, 4, 6, 7, 8, N, N, N, N, N, 14, 15 } },
	{ 0x12, 16, SDHCI_INT_STATUS,
	  { 0, 2, 3, 1, 4, 6, 5, 7, 8, 9, N, N, N, N, N, 15 } },
	{ 0x14, 16, SDHCI_INT_STATUS + 2, ID16 },
	{ 0x16, 16, SDHCI_COMMAND,
	  { 0, 1, N, 3, 5, 4, 6, 7, 8, 9, 10, 11, 12, 13, N, N } },
	{ 0x18, 8, SDHCI_TIMEOUT_CONTROL, ID8 },
	{ 0x19, 8, SDHCI_SOFTWARE_RESET, { 1, 0, 2, 3, 4, 5, 6, 7 } },
	{ 0x1a, 16, SDHCI_CLOCK_CONTROL, ID16 },
	{ 0x1c, 32, SDHCI_RESPONSE, ID32 },
	{ 0x20, 32, SDHCI_RESPONSE + 4, ID32 },
	{ 0x24, 32, SDHCI_RESPONSE + 8, ID32 },
	{ 0x28, 32, SDHCI_RESPONSE + 12, ID32 },
	{ 0x2c, 8, SDHCI_HOST_CONTROL, { 0, 1, 3, 2, 4, 5, 6, 7 } },
	{ 0x2d, 8, SDHCI_POWER_CONTROL, ID8 },
	{ 0x2e, 8, SDHCI_BLOCK_GAP_CONTROL, { 0, 2, 1, 3, 7, 5, 6, 4 } },
	{ 0x2f, 8, SDHCI_WAKE_UP_CONTROL, { 1, 0, 2, 3, 4, 5, 6, 7 } },
	{ 0x30, 32, SDHCI_PRESENT_STATE,
	  { 0, 2, 1, N, N, N, N, N, 9, 10, 8, 11, N, N, N, N,
	    16, 18, 17, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, N, N, N } },
	{ 0x34, 32, SDHCI_ARGUMENT, ID32 },
	{ 0x38, 32, SDHCI_CAPABILITIES,
	  { 0, 1, 2, 3, 4, 5, N, 7, 8, 9, 10, 11, 12, 13, N, N,
	    16, 17, 18, N, N, 21, 23, 22, 24, 25, 26, 27, N, N, N, N } },
	{ 0x3c, 32, SDHCI_ACMD12_ERR,
	  { 1, 2, 3, 0, 4, N, N, 7, N, N, N, N, N, N, N, N,
	    N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N } },
	{ 0x40, 32, SDHCI_BUFFER, ID32 },
};

static const struct gk_sd_reg *gk_sd_find(int std)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(gk_sd_regs); i++) {
		const struct gk_sd_reg *r = &gk_sd_regs[i];

		if (std >= r->std && std < r->std + r->width / 8)
			return r;
	}
	return NULL;
}

/* A vendor register's value as SDHCI bits, and back */
static u32 gk_sd_to_std(const struct gk_sd_reg *r, u32 v)
{
	u32 s = 0;
	int i;

	for (i = 0; i < r->width; i++)
		if (r->bit[i] >= 0 && (v & BIT(i)))
			s |= BIT(r->bit[i]);
	return s;
}

static u32 gk_sd_from_std(const struct gk_sd_reg *r, u32 s)
{
	u32 v = 0;
	int i;

	for (i = 0; i < r->width; i++)
		if (r->bit[i] >= 0 && (s & BIT(r->bit[i])))
			v |= BIT(i);
	return v;
}

static u32 gk_sd_rd(struct sdhci_host *host, const struct gk_sd_reg *r)
{
	void *a = host->ioaddr + r->off;

	return gk_sd_to_std(r, r->width == 8 ? readb(a) :
			       r->width == 16 ? readw(a) : readl(a));
}

static void gk_sd_wr(struct sdhci_host *host, const struct gk_sd_reg *r,
		     u32 s)
{
	void *a = host->ioaddr + r->off;

	if (r->width == 8)
		writeb(gk_sd_from_std(r, s), a);
	else if (r->width == 16)
		writew(gk_sd_from_std(r, s), a);
	else
		writel(gk_sd_from_std(r, s), a);
}

/*
 * An access of `size` bytes at SDHCI offset `reg`: whole vendor registers
 * are read or written as they are; a narrower write into one is merged
 * with its current value (the control registers SDHCI writes bytewise).
 */
static u32 gk_sd_read(struct sdhci_host *host, int reg, int size)
{
	u32 v = 0;
	int i;

	/* No version register in this layout; the vendor clocks it as 2.0 */
	if (reg == SDHCI_HOST_VERSION)
		return SDHCI_SPEC_200;

	for (i = 0; i < size; ) {
		const struct gk_sd_reg *r = gk_sd_find(reg + i);
		int sh;

		if (!r) {
			v |= (size == 4 ? readl(host->ioaddr + reg) :
			      size == 2 ? readw(host->ioaddr + reg) :
			      readb(host->ioaddr + reg)) << (8 * i);
			break;
		}
		sh = 8 * (reg + i - r->std);
		v |= (gk_sd_rd(host, r) >> sh) << (8 * i);
		i += r->width / 8 - (reg + i - r->std);
	}
	return size == 4 ? v : v & ((1u << (8 * size)) - 1);
}

static void gk_sd_write(struct sdhci_host *host, u32 val, int reg, int size)
{
	int i;

	for (i = 0; i < size; ) {
		const struct gk_sd_reg *r = gk_sd_find(reg + i);
		int start, n;
		u32 mask, s;

		if (!r) {
			if (size == 4)
				writel(val, host->ioaddr + reg);
			else if (size == 2)
				writew(val, host->ioaddr + reg);
			else
				writeb(val, host->ioaddr + reg);
			return;
		}
		start = reg + i - r->std;
		n = min(size - i, r->width / 8 - start);
		mask = (n == 4 ? ~0u : (1u << (8 * n)) - 1) << (8 * start);
		s = (val >> (8 * i)) << (8 * start) & mask;
		if (n * 8 != r->width)
			s |= gk_sd_rd(host, r) & ~mask;
		gk_sd_wr(host, r, s);
		i += n;
	}
}

static u32 gk_sd_readl(struct sdhci_host *host, int reg)
{
	return gk_sd_read(host, reg, 4);
}

static u16 gk_sd_readw(struct sdhci_host *host, int reg)
{
	return gk_sd_read(host, reg, 2);
}

static u8 gk_sd_readb(struct sdhci_host *host, int reg)
{
	return gk_sd_read(host, reg, 1);
}

static void gk_sd_writel(struct sdhci_host *host, u32 val, int reg)
{
	gk_sd_write(host, val, reg, 4);
}

static void gk_sd_writew(struct sdhci_host *host, u16 val, int reg)
{
	gk_sd_write(host, val, reg, 2);
}

static void gk_sd_writeb(struct sdhci_host *host, u8 val, int reg)
{
	gk_sd_write(host, val, reg, 1);
}

static const struct sdhci_ops gk_sd_ops = {
	.read_l = gk_sd_readl,
	.read_w = gk_sd_readw,
	.read_b = gk_sd_readb,
	.write_l = gk_sd_writel,
	.write_w = gk_sd_writew,
	.write_b = gk_sd_writeb,
};
#endif /* CONFIG_GK710X_S */

int board_mmc_init(struct bd_info *bis)
{
	struct sdhci_host *host = calloc(1, sizeof(*host));
	unsigned int clk = gk_sd_clock();

	if (!host)
		return -ENOMEM;
	host->name = "gk-sdhci";
	host->ioaddr = (void *)GK_SDIO0_BASE;
	host->max_clk = clk;
	host->voltages = MMC_VDD_32_33 | MMC_VDD_33_34;
	/* The vendor layout has no power register: the slot is always on */
	host->quirks = SDHCI_QUIRK_BROKEN_VOLTAGE | SDHCI_QUIRK_NO_HISPD_BIT;
#ifdef CONFIG_GK710X_S
	host->ops = &gk_sd_ops;
#endif
	return add_sdhci(host, clk, 400000);
}
