// SPDX-License-Identifier: GPL-2.0+
/*
 * Allwinner V831/V833 (sun8iw19p1) DRAM initialisation
 *
 * There is no documentation for this memory controller. This is a
 * translation of the vendor's boot0 DRAM library (libdram) for the
 * sun8iw19p1, function for function and register write for register write;
 * the names are the library's own. The controller is the one of the later
 * D1/T113 (see drivers/ram/sunxi/dram_sun20i_d1.c in mainline), at
 * 0x04002000 (MC_COM) and 0x04003000 (MCTL + PHY), but this library is an
 * older revision: the timing values are computed in auto_set_timing_para()
 * and stored back into the parameters, and it can tell DDR2 from DDR3 by
 * itself (auto_scan_dram_type()).
 *
 * Left out: the vendor's chip binding check (bond_id_check(), which makes
 * the library refuse to run on a chip whose efuse does not match), the
 * PMIC DRAM supply setting (dram_vol_set(); these packages have no PMIC
 * on the reference boards) and the super standby resume path, which only
 * runs when the PRCM says the SoC is waking from it.
 */

#include <common.h>
#include <init.h>
#include <log.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/dram.h>
#include <asm/arch/cpu.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/sizes.h>

#define MC_COM			0x04002000
#define MCTL			0x04003000
#define CCM			SUNXI_CCM_BASE

#define CCM_PLL_DDR		(CCM + 0x010)
#define CCM_PLL_DDR_PAT		(CCM + 0x110)
#define CCM_MBUS		(CCM + 0x540)
#define CCM_DRAM_CLK		(CCM + 0x800)
#define CCM_DRAM_BGR		(CCM + 0x80c)

/* the vendor's parameter block, as in boot0's header (dram_para[32]) */
struct dram_para {
	u32 clk;
	u32 type;
	u32 zq;
	u32 odt_en;
	u32 para1;
	u32 para2;
	u32 mr0;
	u32 mr1;
	u32 mr2;
	u32 mr3;
	u32 tpr0;
	u32 tpr1;
	u32 tpr2;
	u32 tpr3;	/* the DRAM supply voltages, for dram_vol_set() */
	u32 tpr4;
	u32 tpr5;
	u32 tpr6;
	u32 tpr7;
	u32 tpr8;
	u32 tpr9;
	u32 tpr10;
	u32 tpr11;
	u32 tpr12;
	u32 tpr13;
};

static void dram_enable_all_master(void)
{
	writel(0xffffffff, MC_COM + 0x20);
	writel(0x7f, MC_COM + 0x24);
	writel(0xffff, MC_COM + 0x28);
}

static void dram_disable_all_master(void)
{
	writel(1, MC_COM + 0x20);
	writel(0, MC_COM + 0x24);
	writel(0, MC_COM + 0x28);
}

static void eye_delay_compensation(struct dram_para *para)
{
	u32 delay;
	ulong ptr;

	/* DATn0IOCR, n = 0..7 */
	delay = ((para->tpr11 << 9) & 0x1e00) | ((para->tpr12 << 1) & 0x1e);
	for (ptr = MCTL + 0x310; ptr != MCTL + 0x334; ptr += 4)
		setbits_le32(ptr, delay);

	/* DATn1IOCR, n = 0..7 */
	delay = ((para->tpr11 << 5) & 0x1e00) | ((para->tpr12 >> 3) & 0x1e);
	for (ptr = MCTL + 0x390; ptr != MCTL + 0x3b4; ptr += 4)
		setbits_le32(ptr, delay);

	/* PGCR0: assert the AC loopback FIFO reset */
	clrbits_le32(MCTL + 0x100, BIT(26));

	delay = (((para->tpr11 >> 16) << 9) & 0x1e00) |
		((para->tpr12 >> 15) & 0x1e);
	setbits_le32(MCTL + 0x334, delay);
	setbits_le32(MCTL + 0x338, delay);

	delay = (((para->tpr11 >> 20) << 9) & 0x1e00) |
		((para->tpr12 >> 19) & 0x1e);
	setbits_le32(MCTL + 0x3b4, delay);
	setbits_le32(MCTL + 0x3b8, delay);

	setbits_le32(MCTL + 0x33c, ((para->tpr11 >> 16) << 25) & 0x1e000000);
	setbits_le32(MCTL + 0x3bc, ((para->tpr11 >> 20) << 25) & 0x1e000000);

	/* PGCR0: release the AC loopback FIFO reset */
	setbits_le32(MCTL + 0x100, BIT(26));

	delay = (para->tpr10 << 4) & 0xf00;
	for (ptr = MCTL + 0x240; ptr != MCTL + 0x27c; ptr += 4)
		setbits_le32(ptr, delay);

	setbits_le32(MCTL + 0x218, (para->tpr10 << 8) & 0xf00);
	setbits_le32(MCTL + 0x21c, para->tpr10 & 0xf00);
	setbits_le32(MCTL + 0x280, (para->tpr10 >> 4) & 0xf00);
}

/* cycles of the controller clock (DRAM clock / 2) in ns, rounded up */
static u32 auto_cal_timing(u32 time_ns, u32 clk)
{
	u32 value = time_ns * clk;

	return value / 1000 + (value % 1000 ? 1 : 0);
}

static u32 refresh_interval(u32 clk, u32 ns)
{
	u32 t = auto_cal_timing(ns, clk);

	return (t >> 5) + ((t & 0x1f) ? 1 : 0);
}

static void auto_set_timing_para(struct dram_para *para)
{
	u32 clk = para->clk;
	u32 ctrl_freq = clk / 2;
	u32 type = para->type;
	u32 tccd, trcd, tfaw, trrd, trc;
	u32 txp, twtr, twr, trtp, trp, tras;
	u32 trfc, trefi;
	u32 tcksrx, tckesr, tcke, trd2wr, twr2rd, trasmax, twtp;
	u32 tmod, tmrd, tmrw, tcl, tcwl, t_rdata_en, wr_latency;
	u32 mr0, mr1, mr2, mr3;
	u32 tdinit0, tdinit1, tdinit2, tdinit3;
	u32 val;

	if (para->tpr13 & BIT(1)) {
		/* timings given in tpr0..tpr2 */
		tccd = (para->tpr0 >> 21) & 0x7;
		trcd = (para->tpr0 >> 6) & 0x1f;
		tfaw = (para->tpr0 >> 15) & 0x3f;
		trrd = (para->tpr0 >> 11) & 0xf;
		trc = para->tpr0 & 0x3f;
		txp = (para->tpr1 >> 23) & 0x1f;
		twtr = (para->tpr1 >> 20) & 0x7;
		twr = (para->tpr1 >> 11) & 0xf;
		trtp = (para->tpr1 >> 15) & 0x1f;
		trp = (para->tpr1 >> 6) & 0x1f;
		tras = para->tpr1 & 0x3f;
		trfc = (para->tpr2 >> 12) & 0x1ff;
		trefi = para->tpr2 & 0xfff;
	} else {
		switch (type) {
		case SUNXI_DRAM_V831_DDR3:
			twtr = max(auto_cal_timing(8, ctrl_freq), 2U);
			trcd = auto_cal_timing(15, ctrl_freq);
			twr = max(trcd, 2U);
			trefi = refresh_interval(ctrl_freq, 7800);
			trfc = auto_cal_timing(350, ctrl_freq);
			if (clk <= 800) {
				tfaw = auto_cal_timing(50, ctrl_freq);
				trrd = max(auto_cal_timing(10, ctrl_freq), 2U);
				trc = auto_cal_timing(53, ctrl_freq);
				tras = auto_cal_timing(38, ctrl_freq);
				txp = twtr;
			} else {
				tfaw = auto_cal_timing(35, ctrl_freq);
				trrd = max(auto_cal_timing(6, ctrl_freq), 2U);
				trcd = auto_cal_timing(14, ctrl_freq);
				trc = auto_cal_timing(48, ctrl_freq);
				tras = auto_cal_timing(34, ctrl_freq);
				txp = trrd;
			}
			trp = trcd;
			break;
		case SUNXI_DRAM_V831_DDR2:
			tfaw = auto_cal_timing(50, ctrl_freq);
			trrd = auto_cal_timing(10, ctrl_freq);
			trcd = auto_cal_timing(20, ctrl_freq);
			trc = auto_cal_timing(65, ctrl_freq);
			twtr = auto_cal_timing(8, ctrl_freq);
			trp = auto_cal_timing(15, ctrl_freq);
			twr = trp;
			tras = auto_cal_timing(45, ctrl_freq);
			trefi = refresh_interval(ctrl_freq, 7800);
			trfc = auto_cal_timing(328, ctrl_freq);
			txp = 2;
			break;
		case SUNXI_DRAM_V831_LPDDR2:
			tfaw = max(auto_cal_timing(50, ctrl_freq), 4U);
			trrd = auto_cal_timing(10, ctrl_freq);
			if (trrd == 0)
				trrd = 1;
			trcd = max(auto_cal_timing(24, ctrl_freq), 2U);
			trc = auto_cal_timing(70, ctrl_freq);
			txp = auto_cal_timing(8, ctrl_freq);
			if (txp == 0) {
				txp = 1;
				twtr = 2;
			} else {
				twtr = max(txp, 2U);
			}
			twr = max(auto_cal_timing(15, ctrl_freq), 2U);
			trp = auto_cal_timing(17, ctrl_freq);
			tras = auto_cal_timing(42, ctrl_freq);
			trefi = refresh_interval(ctrl_freq, 3900);
			trfc = auto_cal_timing(210, ctrl_freq);
			break;
		case SUNXI_DRAM_V831_LPDDR3:
			tfaw = max(auto_cal_timing(50, ctrl_freq), 4U);
			trrd = auto_cal_timing(10, ctrl_freq);
			if (trrd == 0)
				trrd = 1;
			trcd = max(auto_cal_timing(24, ctrl_freq), 2U);
			trc = auto_cal_timing(70, ctrl_freq);
			twtr = max(auto_cal_timing(8, ctrl_freq), 2U);
			twr = max(auto_cal_timing(15, ctrl_freq), 2U);
			trp = auto_cal_timing(17, ctrl_freq);
			tras = auto_cal_timing(42, ctrl_freq);
			trefi = refresh_interval(ctrl_freq, 3900);
			trfc = auto_cal_timing(210, ctrl_freq);
			txp = twtr;
			break;
		default:
			trfc = 128;
			trefi = 98;
			txp = 10;
			tfaw = 16;
			tras = 14;
			twtr = 3;
			trp = 6;
			twr = 8;
			trc = 20;
			trcd = 6;
			trrd = 3;
			break;
		}
		tccd = 2;
		trtp = twtr;

		/* the library stores what it computed */
		para->tpr0 = trc | (trcd << 6) | (trrd << 11) | (tfaw << 15) |
			     (tccd << 21);
		para->tpr1 = tras | (trp << 6) | (twr << 11) | (trtp << 15) |
			     (twtr << 20) | (txp << 23);
		para->tpr2 = trefi | (trfc << 12);
	}

	switch (type) {
	case SUNXI_DRAM_V831_DDR2:
		tmrd = 2;
		tcwl = 3;
		tcke = 3;
		tckesr = 4;
		trd2wr = 4;
		tcksrx = 5;
		tmrw = 0;
		tmod = 12;
		wr_latency = 1;
		trasmax = clk / 30;
		if (ctrl_freq < 205) {
			t_rdata_en = 1;
			tcl = 3;
			mr0 = 0xa63;
		} else {
			t_rdata_en = 2;
			tcl = 4;
			mr0 = 0xe73;
		}
		mr1 = para->mr1;
		mr2 = 0;
		mr3 = 0;
		tdinit0 = 400 * clk + 1;
		tdinit1 = 500 * clk / 1000 + 1;
		tdinit2 = 200 * clk + 1;
		tdinit3 = clk + 1;
		twtp = twr + 5;
		twr2rd = twtr + 5;
		break;
	case SUNXI_DRAM_V831_DDR3:
		tcwl = clk > 800 ? 5 : 4;
		t_rdata_en = tcwl;
		twr += 2;
		twtr += 2;
		tckesr = 4;
		tcke = 3;
		trasmax = clk / 30;
		tcl = clk > 800 ? 7 : 6;
		wr_latency = clk > 800 ? 3 : 2;
		mr2 = clk > 800 ? 32 : 24;
		mr0 = clk > 800 ? 0x1e14 : 0x1c70;
		tdinit0 = 500 * clk + 1;
		tdinit1 = 360 * clk / 1000 + 1;
		tdinit2 = 200 * clk + 1;
		tdinit3 = clk + 1;
		twtp = twr + tcwl;
		twr2rd = twtr + tcwl;
		tcksrx = 5;
		mr1 = para->mr1;
		if (((para->tpr13 >> 2) & 0x3) == 1 || clk <= 912)
			trd2wr = 5;
		else
			trd2wr = 6;
		tmrd = 4;
		tmod = 12;
		mr3 = 0;
		tmrw = 0;
		break;
	case SUNXI_DRAM_V831_LPDDR2:
		mr2 = 6;
		tcke = 2;
		trd2wr = 10;
		tcwl = 2;
		tcl = 4;
		wr_latency = 1;
		trasmax = clk / 60;
		tdinit0 = 200 * clk + 1;
		tdinit1 = 100 * clk / 1000 + 1;
		tcksrx = 5;
		tckesr = 5;
		tmrd = 5;
		tmod = 5;
		tdinit2 = 11 * clk + 1;
		tdinit3 = clk + 1;
		twtp = twr + 5;
		twr2rd = twtr + 5;
		mr3 = para->mr3;
		tmrw = 3;
		t_rdata_en = 3;
		mr1 = 0xc3;
		mr0 = 0;
		break;
	case SUNXI_DRAM_V831_LPDDR3:
		tcwl = clk > 800 ? 4 : 3;
		t_rdata_en = clk > 800 ? 6 : 5;
		twr += 5;
		twtr += 5;
		trd2wr = 13;
		trasmax = clk / 60;
		wr_latency = clk > 800 ? 3 : 2;
		mr2 = clk > 800 ? 12 : 10;
		tdinit0 = 200 * clk + 1;
		tdinit1 = 100 * clk / 1000 + 1;
		tcksrx = 5;
		tckesr = 5;
		tdinit2 = 11 * clk + 1;
		tdinit3 = clk + 1;
		twtp = twr + tcwl;
		twr2rd = twtr + tcwl;
		tcl = clk > 800 ? 7 : 6;
		tcke = 3;
		mr3 = para->mr3;
		tmrd = 5;
		tmod = 12;
		tmrw = 5;
		mr1 = 0xc3;
		mr0 = 0;
		break;
	default:
		trasmax = 27;
		twr2rd = 8;
		twtp = 12;
		tcke = 2;
		tmod = 6;
		tckesr = 3;
		t_rdata_en = 1;
		tcksrx = 4;
		tmrd = 2;
		tmrw = 0;
		trd2wr = 4;
		tcwl = 3;
		tcl = 3;
		wr_latency = 1;
		mr0 = 0;
		mr1 = 0;
		mr2 = 0;
		mr3 = 0;
		tdinit0 = 0;
		tdinit1 = 0;
		tdinit2 = 0;
		tdinit3 = 0;
		break;
	}

	if (trtp + trp < tcl + 2)
		trtp = tcl + 2 - trp;

	/* a value given with bits set in the upper half wins */
	if (!(para->mr0 >> 16))
		para->mr0 = mr0;
	if (!(para->mr1 >> 16))
		para->mr1 = mr1;
	if (!(para->mr2 >> 16))
		para->mr2 = mr2;
	if (!(para->mr3 >> 16))
		para->mr3 = mr3;

	writel(para->mr0 & 0xffff, MCTL + 0x030);
	writel(para->mr1 & 0xffff, MCTL + 0x034);
	writel(para->mr2 & 0xffff, MCTL + 0x038);
	writel(para->mr3 & 0xffff, MCTL + 0x03c);
	writel((para->odt_en >> 4) & 0x3, MCTL + 0x02c);

	/* DRAMTMG0..5 */
	writel(tras | (tfaw << 16) | (twtp << 24) | (trasmax << 8),
	       MCTL + 0x058);
	writel(trc | (txp << 16) | (trtp << 8), MCTL + 0x05c);
	writel(twr2rd | (tcl << 16) | (tcwl << 24) | (trd2wr << 8),
	       MCTL + 0x060);
	writel(tmod | (tmrd << 12) | (tmrw << 16), MCTL + 0x064);
	writel(trp | (trrd << 8) | (tccd << 16) | (trcd << 24), MCTL + 0x068);
	writel(tcke | (tckesr << 8) | (tcksrx << 16) | (tcksrx << 24),
	       MCTL + 0x06c);

	/* dual rank timing */
	val = (readl(MCTL + 0x078) & 0x0fff0000) | 0xf0000010;
	val |= clk <= 800 ? 0x6600 : 0x7600;
	writel(val, MCTL + 0x078);

	/* PITMG0, PTR3, PTR4 */
	writel(wr_latency | BIT(8) | (t_rdata_en << 16) | (2 << 24),
	       MCTL + 0x080);
	writel(tdinit0 | (tdinit1 << 20), MCTL + 0x050);
	writel(tdinit2 | (tdinit3 << 20), MCTL + 0x054);

	/* refresh */
	writel(trfc | (trefi << 16), MCTL + 0x090);
	writel((trefi << 15) & 0x0fff0000, MCTL + 0x094);
}

static void ccm_set_pll_ddr_sscg(struct dram_para *para)
{
	u32 pat;

	if (readl(CCM_PLL_DDR) & BIT(1))
		setbits_le32(CCM_PLL_DDR_PAT, BIT(19));

	switch ((para->tpr13 >> 20) & 0x7) {
	case 1:
		pat = 0xe486cccc;
		break;
	case 2:
		pat = 0xe9069999;
		break;
	case 3:
		pat = 0xed866666;
		break;
	case 5:
		pat = 0xf5860000;
		break;
	default:
		pat = 0xf2063333;
		break;
	}
	writel(pat, CCM_PLL_DDR_PAT);
	setbits_le32(CCM_PLL_DDR, BIT(24));
}

static u32 ccm_set_pll_ddr_clk(struct dram_para *para)
{
	u32 clk, n, val;

	clk = (para->tpr13 & BIT(6)) ? para->tpr9 : para->clk;
	n = clk * 2 / 24;

	val = readl(CCM_PLL_DDR) & ~0x7f03;
	val |= (max(n, 12U) - 1) << 8;
	writel(val | BIT(31), CCM_PLL_DDR);
	val &= ~BIT(29);
	writel(val | BIT(31), CCM_PLL_DDR);
	writel(val | BIT(31) | BIT(29), CCM_PLL_DDR);
	while (!(readl(CCM_PLL_DDR) & BIT(28)))
		;

	if (para->tpr13 & BIT(23))
		ccm_set_pll_ddr_sscg(para);

	return n * 24;
}

static void mctl_sys_init(struct dram_para *para)
{
	/* MBUS reset and gate */
	clrbits_le32(CCM_MBUS, BIT(31));
	clrbits_le32(CCM_MBUS, BIT(30));
	/* DRAM bus gate and reset */
	clrbits_le32(CCM_DRAM_BGR, BIT(0));
	clrbits_le32(CCM_DRAM_BGR, BIT(16));
	/* PLL_DDR off, its LDO on */
	clrsetbits_le32(CCM_PLL_DDR, BIT(31), BIT(30));
	clrbits_le32(CCM_DRAM_CLK, BIT(30));

	para->clk = ccm_set_pll_ddr_clk(para) / 2;

	/* DRAM clock from PLL_DDR, M = 1, update */
	clrbits_le32(CCM_DRAM_CLK, 0x3000000);
	clrbits_le32(CCM_DRAM_CLK, 0x3);
	setbits_le32(CCM_DRAM_CLK, BIT(27));

	setbits_le32(CCM_DRAM_BGR, BIT(16));
	setbits_le32(CCM_DRAM_BGR, BIT(0));

	dram_disable_all_master();

	setbits_le32(CCM_MBUS, BIT(30));
	setbits_le32(CCM_MBUS, BIT(31));
	setbits_le32(CCM_DRAM_CLK, BIT(30));

	writel(0x8000, MCTL + 0x00c);
}

static void mctl_com_init(struct dram_para *para)
{
	u32 val, n, i;

	clrsetbits_le32(MC_COM + 0x008, 0x3f00, 0x2000);

	val = readl(MC_COM + 0x000) & ~0x00fff000;
	val |= BIT(22);
	val |= (para->type << 16) & 0x70000;
	if (!(para->para2 & BIT(0)))
		val |= BIT(12);
	if (para->type == SUNXI_DRAM_V831_LPDDR2 ||
	    para->type == SUNXI_DRAM_V831_LPDDR3)
		val |= BIT(19);
	else
		val |= (para->tpr13 << 14) & BIT(19);
	writel(val, MC_COM + 0x000);

	/* rank0 and, with two different ranks, rank1 */
	if ((para->para2 & BIT(8)) && ((para->para2 >> 12) & 0xf) == 1)
		n = 2;
	else
		n = 1;

	for (i = 0; i < n; i++) {
		u32 sh = i * 16;
		ulong reg = MC_COM + 0x000 + i * 4;

		val = readl(reg) & 0xfffff000;
		val |= (para->para2 >> 12) & 0x3;
		val |= ((para->para1 >> (sh + 12)) << 2) & 0x4;
		val |= (((para->para1 >> (sh + 4)) - 1) << 4) & 0xff;
		switch ((para->para1 >> sh) & 0xf) {
		case 1:
			val |= 0x700;
			break;
		case 2:
			val |= 0x800;
			break;
		case 4:
			val |= 0x900;
			break;
		case 8:
			val |= 0xa00;
			break;
		default:
			val |= 0x600;
			break;
		}
		writel(val, reg);
	}

	writel((readl(MC_COM + 0x000) & BIT(0)) ? 0x303 : 0x201,
	       MCTL + 0x120);

	if (para->para2 & BIT(0))
		writel(0, MCTL + 0x3c4);

	if (para->tpr4) {
		setbits_le32(MC_COM + 0x000, (para->tpr4 << 25) & 0x06000000);
		setbits_le32(MC_COM + 0x004, (para->tpr4 << 10) & 0x001ff000);
	}
}

static void mctl_phy_ac_remapping(struct dram_para *para)
{
	if (para->type != SUNXI_DRAM_V831_DDR3)
		return;

	writel(0x00f4b0c0, MC_COM + 0x500);
	writel(0x04e50267, MC_COM + 0x504);
	writel(0x00ba5461, MC_COM + 0x508);
	writel(0x24db4080, MC_COM + 0x50c);
	writel(0x00f4b0c1, MC_COM + 0x500);
}

static void mctl_vrefzq_init(struct dram_para *para)
{
	if (para->tpr13 & BIT(17))
		return;

	clrsetbits_le32(MCTL + 0x110, 0x7f7f7f7f, para->tpr5);
	if (!(para->tpr13 & BIT(16)))
		clrsetbits_le32(MCTL + 0x114, 0x7f, para->tpr6 & 0x7f);
}

static int mctl_channel_init(struct dram_para *para)
{
	u32 dqs_gating_mode = (para->tpr13 >> 2) & 0x3;
	u32 odt, val;
	int ret;

	clrsetbits_le32(MC_COM + 0x00c, 0xfff, para->clk >> 1);
	clrsetbits_le32(MCTL + 0x108, 0xf00, 0x300);

	odt = (~para->odt_en << 5) & BIT(5);

	/* DX0GCR0, DX1GCR0 */
	val = (readl(MCTL + 0x344) & ~0x30) | odt;
	if (para->clk > 672)
		val = (val & ~0xf61e) | 0x400;
	else
		val &= ~0xf01e;
	writel(val, MCTL + 0x344);

	val = (readl(MCTL + 0x3c4) & ~0x30) | odt;
	if (para->clk > 672)
		val = (val & ~0xf61e) | 0x400;
	else
		val &= ~0xf01e;
	writel(val, MCTL + 0x3c4);

	setbits_le32(MCTL + 0x208, BIT(1));

	eye_delay_compensation(para);

	if (dqs_gating_mode == 1) {
		clrbits_le32(MCTL + 0x108, 0xc0);
		clrbits_le32(MCTL + 0x0bc, 0x107);
	} else if (dqs_gating_mode == 2) {
		clrsetbits_le32(MCTL + 0x108, 0xc0, 0x80);
		clrsetbits_le32(MCTL + 0x0bc, 0x107,
				(((readl(MCTL + 0x060) >> 16) & 0x1f) - 2) |
				0x100);
		clrsetbits_le32(MCTL + 0x11c, BIT(31), BIT(27));
	} else {
		clrbits_le32(MCTL + 0x108, 0x40);
		setbits_le32(MCTL + 0x108, 0xc0);
	}

	if (para->type == SUNXI_DRAM_V831_LPDDR2 ||
	    para->type == SUNXI_DRAM_V831_LPDDR3) {
		if (dqs_gating_mode == 1)
			clrsetbits_le32(MCTL + 0x11c, 0x080000c0, BIT(31));
		else
			clrsetbits_le32(MCTL + 0x11c, 0x77000000, 0x22000000);
	}

	clrsetbits_le32(MCTL + 0x0c0, 0x0fffffff,
			((para->para2 & BIT(12)) ? 0x03000000 : 0x01000000) |
			1);

	/* ZQ */
	clrsetbits_le32(MCTL + 0x140, 0x03ffffff,
			BIT(25) | (para->zq & 0x00ffffff));

	if (dqs_gating_mode == 1) {
		writel(0x52, MCTL + 0x000);
		writel(0x53, MCTL + 0x000);
		while (!(readl(MCTL + 0x010) & BIT(0)))
			;
		val = para->type == SUNXI_DRAM_V831_DDR3 ? 0x5a0 : 0x520;
	} else {
		val = para->type == SUNXI_DRAM_V831_DDR3 ? 0x1f2 : 0x172;
	}
	/* PIR: the training steps, then go */
	writel(val, MCTL + 0x000);
	writel(val | BIT(0), MCTL + 0x000);
	while (!(readl(MCTL + 0x010) & BIT(0)))
		;

	/* PGSR0 error bits; ZQ calibration failing is fatal at once */
	val = readl(MCTL + 0x010);
	if (val & BIT(20))
		return 0;
	ret = !(val & 0x0ff00000);

	while (!(readl(MCTL + 0x018) & BIT(0)))
		;

	setbits_le32(MCTL + 0x08c, BIT(31));
	clrbits_le32(MCTL + 0x08c, BIT(31));
	setbits_le32(MC_COM + 0x014, BIT(31));
	clrbits_le32(MCTL + 0x10c, 0x06000000);
	if (dqs_gating_mode == 1)
		clrsetbits_le32(MCTL + 0x11c, 0xc0, 0x40);

	return ret;
}

static int mctl_core_init(struct dram_para *para)
{
	mctl_sys_init(para);
	mctl_vrefzq_init(para);
	mctl_com_init(para);
	mctl_phy_ac_remapping(para);
	auto_set_timing_para(para);

	return mctl_channel_init(para);
}

static u32 rank_size_mb(u32 reg)
{
	return 1U << (((reg >> 8) & 0xf) + ((reg >> 4) & 0xf) +
		      ((reg >> 2) & 0x3) - 14);
}

static u32 dramc_get_dram_size(void)
{
	u32 reg0 = readl(MC_COM + 0x000);
	u32 reg1;
	u32 size0 = rank_size_mb(reg0);

	if (!(reg0 & 0x3))
		return size0;

	reg1 = readl(MC_COM + 0x004);
	if (!(reg1 & 0x3))
		return size0 * 2;

	return size0 + rank_size_mb(reg1);
}

/* after a training run in DQS probe mode: ranks and DQ width */
static int dqs_gate_detect(struct dram_para *para)
{
	u32 dx0, dx1;

	if (!(readl(MCTL + 0x010) & BIT(22))) {
		/* dual rank, full DQ */
		para->para2 = (para->para2 & ~0xf) | BIT(12);
		return 1;
	}

	dx0 = (readl(MCTL + 0x348) >> 24) & 0x3;
	dx1 = (readl(MCTL + 0x3c8) >> 24) & 0x3;

	if (dx0 == 2) {
		/* single rank, full or half DQ */
		para->para2 &= ~0xf00f;
		if (dx1 != 2)
			para->para2 |= BIT(0);
		return 1;
	}

	if (dx0 == 0) {
		/* dual rank, half DQ */
		para->para2 = (para->para2 & ~0xf) | 0x1001;
		return 1;
	}

	return 0;
}

static int dramc_simple_wr_test(u32 mem_mb, u32 len)
{
	ulong half = (mem_mb >> 1) << 20;
	ulong base = CONFIG_SYS_SDRAM_BASE;
	u32 i;

	for (i = 0; i < len; i++) {
		writel(0x01234567 + i, base + i * 4);
		writel(0xfedcba98 + i, base + half + i * 4);
	}

	for (i = 0; i < len; i++) {
		if (readl(base + half + i * 4) != 0xfedcba98 + i ||
		    readl(base + i * 4) != 0x01234567 + i) {
			printf("DRAM: simple test FAIL at %lx\n", base + i * 4);
			return 1;
		}
	}

	return 0;
}

static u32 scan_pattern(int odd, ulong addr)
{
	return odd ? addr : ~addr;
}

/* 64 words at base + off read back as the pattern written at pat */
static int scan_match(ulong addr, ulong pat)
{
	int i;

	for (i = 0; i < 64; i++) {
		if (readl(addr + i * 4) != scan_pattern(i & 1, pat + i * 4))
			return 0;
	}

	return 1;
}

/*
 * Rows, banks and page size per rank: set the address mapping wide, then
 * see where the pattern at the start of the rank shows up again.
 */
static int auto_scan_dram_size(struct dram_para *para)
{
	u32 para2 = para->para2;
	u32 maxrank = ((para2 >> 12) & 0xf) == 1 ? 2 : 1;
	ulong pat = CONFIG_SYS_SDRAM_BASE, base = CONFIG_SYS_SDRAM_BASE;
	u32 shift = 12, rank, i, pg;
	ulong reg;

	if (!mctl_core_init(para))
		return 0;

	for (rank = 0; rank < maxrank; rank++) {
		reg = MC_COM + 0x000 + rank * 4;

		for (i = 0; i < 64; i++)
			writel(scan_pattern(i & 1, base + i * 4),
			       base + i * 4);

		/* row mode */
		clrsetbits_le32(reg, 0xf0c, 0x6f0);
		for (i = 11; i < 17; i++) {
			if (scan_match(base + (1UL << (i + 11)), pat))
				break;
		}
		if (i == 17)
			i = 16;
		para->para1 = (para->para1 & ~(0xff << (shift - 8))) |
			      (i << (shift - 8));

		/* bank mode */
		if (rank == 1) {
			clrsetbits_le32(MC_COM + 0x000, 0xffc, 0x6a4);
			base = CONFIG_SYS_SDRAM_BASE + 0x800000;
		}
		clrsetbits_le32(reg, 0xffc, 0x6a4);
		i = scan_match(base + 0x800, pat) ? 0 : 1;
		para->para1 = (para->para1 & ~(0xf << shift)) | (i << shift);

		/* page mode */
		if (rank == 1) {
			clrsetbits_le32(MC_COM + 0x000, 0xffc, 0xaa0);
			base = CONFIG_SYS_SDRAM_BASE + 0x4000000;
		}
		clrsetbits_le32(reg, 0xffc, 0xaa0);
		for (i = 9; i < 14; i++) {
			if (scan_match(base + (1UL << i), pat))
				break;
		}
		if (i == 14)
			i = 13;
		pg = i == 9 ? 0 : 1 << (i - 10);
		para->para1 = (para->para1 & ~(0xf << (rank * 16))) |
			      (pg << (rank * 16));

		if (rank + 1 == maxrank)
			break;

		/* on to rank1, at 128 MiB */
		clrsetbits_le32(MC_COM + 0x000, 0xffc, 0x6f0);
		clrsetbits_le32(MC_COM + 0x004, 0xffc, 0x6f1);
		pat = base = CONFIG_SYS_SDRAM_BASE + 0x8000000;
		shift += 16;
	}

	if (maxrank == 2) {
		para2 &= ~0xf00;
		if ((para->para1 & 0xffff) != (para->para1 >> 16))
			para2 |= BIT(8);
		para->para2 = para2;
	}

	return 1;
}

static int auto_scan_dram_rank_width(struct dram_para *para)
{
	u32 tpr13 = para->tpr13;
	u32 para1 = para->para1;

	para->para1 = 0x00b000b0;
	para->para2 = (para->para2 & ~0xf) | BIT(12);
	/* DQS probe mode */
	para->tpr13 = (tpr13 & ~BIT(3)) | BIT(2) | BIT(0);

	mctl_core_init(para);
	if (readl(MCTL + 0x010) & BIT(20))
		return 0;
	if (!dqs_gate_detect(para))
		return 0;

	para->tpr13 = tpr13;
	para->para1 = para1;

	return 1;
}

/* DDR3 first, then DDR2; the clocks for either are in tpr7 */
static int auto_scan_dram_type(struct dram_para *para)
{
	u32 tpr13 = para->tpr13;
	u32 para1 = para->para1;
	u32 para2 = para->para2;
	u32 clk = para->clk;

	para->para1 = 0x00b000b0;
	para->para2 = 0;
	para->tpr13 = (tpr13 & ~BIT(3)) | BIT(2) | BIT(0);

	if (!clk)
		para->clk = 24 * ((para->tpr7 >> 16) & 0xff);
	para->type = SUNXI_DRAM_V831_DDR3;
	para->zq = 0x3bfb;
	mctl_core_init(para);
	if (!dqs_gate_detect(para)) {
		if (!clk)
			para->clk = 24 * ((para->tpr7 >> 24) & 0xff);
		para->type = SUNXI_DRAM_V831_DDR2;
		para->zq = 0x3bf9;
		mctl_core_init(para);
		if (!dqs_gate_detect(para))
			return 0;
	}

	para->tpr13 = tpr13;
	para->para1 = para1;
	para->para2 = para2;

	return 1;
}

static int auto_scan_dram_config(struct dram_para *para)
{
	if (!(para->tpr13 & BIT(13)) && !auto_scan_dram_type(para))
		return 0;
	if (!(para->tpr13 & BIT(14)) && !auto_scan_dram_rank_width(para))
		return 0;
	if (!(para->tpr13 & BIT(0)) && !auto_scan_dram_size(para))
		return 0;
	if (!(para->tpr13 & BIT(15)))
		para->tpr13 |= BIT(14) | BIT(13) | BIT(1) | BIT(0);

	return 1;
}

static u32 init_dram(struct dram_para *para)
{
	u32 size_mb;

	if (!(para->tpr13 & BIT(16))) {
		/* external ZQ resistor */
		writel(0x10, SUNXI_PRCM_BASE + 0x250);
		writel(0x330000, SUNXI_PRCM_BASE + 0x310);
		writel(0x330003, SUNXI_PRCM_BASE + 0x310);
	} else {
		setbits_le32(SUNXI_PRCM_BASE + 0x310, BIT(8));
		writel(0, SUNXI_PRCM_BASE + 0x318);
	}

	if (!(para->tpr13 & BIT(0)) && !auto_scan_dram_config(para)) {
		printf("DRAM: auto scan failed\n");
		return 0;
	}

	if (!mctl_core_init(para)) {
		printf("DRAM: initialisation failed\n");
		return 0;
	}

	if (para->para2 & BIT(31)) {
		size_mb = (para->para2 >> 16) & 0x7fff;
	} else {
		size_mb = dramc_get_dram_size();
		para->para2 = (para->para2 & 0xffff) | (size_mb << 16);
	}

	/* auto self refresh */
	if (para->tpr13 & BIT(30)) {
		writel(para->tpr8 ? para->tpr8 : 0x10000200, MCTL + 0x0a0);
		writel(0x40a, MCTL + 0x09c);
		setbits_le32(MCTL + 0x004, BIT(0));
	} else {
		clrbits_le32(MCTL + 0x0a0, 0xffff);
		clrbits_le32(MCTL + 0x004, BIT(0));
	}

	if ((para->tpr13 & BIT(9)) || para->type == SUNXI_DRAM_V831_LPDDR2)
		clrsetbits_le32(MCTL + 0x100, 0xf000, 0x5000);
	else
		clrbits_le32(MCTL + 0x100, 0xf000);

	setbits_le32(MCTL + 0x140, BIT(31));
	if (para->tpr13 & BIT(8))
		setbits_le32(MCTL + 0x0b8, 0x300);

	if (para->tpr13 & BIT(26))
		clrbits_le32(MCTL + 0x108, BIT(13));
	else
		setbits_le32(MCTL + 0x108, BIT(13));

	if (para->type == SUNXI_DRAM_V831_LPDDR3)
		clrsetbits_le32(MCTL + 0x07c, 0xf0000, 0x10000);

	dram_enable_all_master();

	if ((para->tpr13 & BIT(28)) && dramc_simple_wr_test(size_mb, 4096))
		return 0;

	return size_mb;
}

unsigned long sunxi_dram_init(void)
{
	struct dram_para para = {
		.clk	= CONFIG_DRAM_CLK,
		.type	= CONFIG_DRAM_SUN8I_V831_TYPE,
		.zq	= CONFIG_DRAM_ZQ,
		.odt_en	= CONFIG_DRAM_SUN8I_V831_ODT_EN,
		.para1	= CONFIG_DRAM_SUN8I_V831_PARA1,
		.para2	= CONFIG_DRAM_SUN8I_V831_PARA2,
		.mr0	= CONFIG_DRAM_SUN8I_V831_MR0,
		.mr1	= CONFIG_DRAM_SUN8I_V831_MR1,
		.mr2	= CONFIG_DRAM_SUN8I_V831_MR2,
		.mr3	= CONFIG_DRAM_SUN8I_V831_MR3,
		.tpr0	= CONFIG_DRAM_SUN8I_V831_TPR0,
		.tpr1	= CONFIG_DRAM_SUN8I_V831_TPR1,
		.tpr2	= CONFIG_DRAM_SUN8I_V831_TPR2,
		.tpr5	= CONFIG_DRAM_SUN8I_V831_TPR5,
		.tpr6	= CONFIG_DRAM_SUN8I_V831_TPR6,
		.tpr7	= CONFIG_DRAM_SUN8I_V831_TPR7,
		.tpr11	= CONFIG_DRAM_SUN8I_V831_TPR11,
		.tpr12	= CONFIG_DRAM_SUN8I_V831_TPR12,
		.tpr13	= CONFIG_DRAM_SUN8I_V831_TPR13,
	};

	unsigned long size_mb = init_dram(&para);

	if (!size_mb)
		return 0;

	/*
	 * The geometry the library found, checked against the DRAM itself:
	 * past the part's size the addresses alias. (Under QEMU, which does
	 * not model the controller's address mapping, the library's scan,
	 * like the vendor's boot0, reports 4160 MiB.)
	 */
	size_mb = min(size_mb, (unsigned long)(SZ_2G >> 20));

	return get_ram_size((long *)CONFIG_SYS_SDRAM_BASE, size_mb << 20);
}
