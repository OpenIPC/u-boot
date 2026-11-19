// SPDX-License-Identifier: GPL-2.0+
/*
 * AX620U LPDDR4/4x init, reconstructed from the vendor first stage.
 *
 * The controller (CTL, 0x00100000), its training engine (PI, 0x00102000)
 * and the PHY (0x00104000) are one register file of 32-bit words,
 * addressed here by index from 0x00100000. Their settings come from
 * ddr_tables.c; the code below writes them in the vendor's order, sets
 * the DDR PLL (0x00210000), starts CTL and PI and waits for the PI's
 * training (write leveling, gate, read/write eye, CA, as the tables
 * enable it), serving the controller's frequency change requests on the
 * way. Boards with DDR_CFG_MODE_MASK set run it twice: the first pass
 * records the CA training results (PHY CA slave delays, MR12 VREF-CA
 * per channel) and the second writes the delays back with CA training
 * off.
 */
#include "cmn.h"
#include "ddr.h"
#include "ddr_tables.h"
#include "timer.h"

#define DDR_REG_BASE		0x00100000
#define DDR_PI_BASE		0x00102000
#define DDR_PHY_BASE		0x00104000
#define DDR_REG(idx)		(DDR_REG_BASE + (idx) * 4)

#define DPLL_BASE		0x00210000
#define DPLL_CFG		(DPLL_BASE + 0x00)
#define DPLL_STATUS		(DPLL_BASE + 0x38)
#define DPLL_STATUS_DONE	(1 << 13)
#define DPLL_FREQ_REQ		(DPLL_BASE + 0x64)

#define SYS_PLL_BASE		0x02210000

/* CTL registers */
#define CTL_START		0x000
#define CTL_MRR_CMD		0x0b6
#define CTL_MRR_DATA0		0x0b7
#define CTL_MRR_DATA1		0x0b8
#define CTL_INT_STATUS_MR	0x134
#define CTL_INT_ACK_MR		0x13c
#define CTL_MRR_ERR		0x016
/* PI registers, by CTL index */
#define PI_START		0x800
/* PHY registers, by PHY index */
#define PHY_CA_SLAVE_DELAY	0x433	/* ten registers */
#define PHY_CA_UPDATE		0x503

static int ddr_pass;

/* CA training results, kept from the first pass of a two-pass init. */
static u32 phy_ca_regs[10];
static u32 phy_ca_delay[16];
static u32 vref_ca[4];
static u32 mr_data_extra;

#define DELAY(n) do { volatile int _i = (n); while (_i-- != 0) ; } while (0)

static void ddr_reg_field(u32 idx, u32 width, u32 shift, u32 val)
{
	reg_field_write(DDR_REG(idx), width, shift, val);
}

static u32 ddr_reg_read(u32 idx, int phy)
{
	return readl(phy == 1 ? DDR_PHY_BASE + idx * 4 : DDR_REG_BASE + idx * 4);
}

static void ddr_reg_write(u32 idx, u32 val, int phy, int pi)
{
	u32 addr;

	idx *= 4;
	if (phy == 1)
		addr = DDR_PHY_BASE + idx;
	else if (pi == 1)
		addr = DDR_PI_BASE + idx;
	else
		addr = DDR_REG_BASE + idx;
	writel(val, addr);
}

static u32 bits_get(u32 v, u32 hi, u32 lo)
{
	return (v << (31 - hi)) >> (31 - hi) >> lo;
}

static u32 bits_set(u32 val, u32 old, u32 hi, u32 lo)
{
	u32 w = 31 - (hi - lo);
	u32 mask = (((0xffffffffU >> lo) << lo) << (31 - hi)) >> (31 - hi);

	return ((val << w) >> w) << lo | (~mask & old);
}

static void write_pairs(const struct ddr_reg_val *t, u32 n)
{
	u32 i;

	for (i = 0; i < n; i++)
		writel(t[i].val, t[i].reg);
}

static void write_fields(const struct ddr_reg_field *t, u32 n)
{
	u32 i;

	for (i = 0; i < n; i++)
		reg_field_write(t[i].reg, t[i].width, t[i].shift, t[i].val);
}

/* DDR reference from the system PLL block. */
static void ddr_sys_pll_set(void)
{
	reg_field_write(SYS_PLL_BASE + 0x1c, 1, 2, 1);
	reg_field_write(SYS_PLL_BASE + 0x1c, 1, 0, 0);
	reg_field_write(SYS_PLL_BASE + 0x34, 1, 0, 0);
	reg_field_write(SYS_PLL_BASE + 0xc8, 9, 0, 0x5b);
	reg_field_write(SYS_PLL_BASE + 0xb8, 0x18, 0, 0);
	reg_field_write(SYS_PLL_BASE + 0x1c, 1, 2, 1);
	reg_field_write(SYS_PLL_BASE + 0x1c, 1, 0, 1);
	reg_field_write(SYS_PLL_BASE + 0x1c, 1, 2, 0);
}

static void ddr_sys_pll_wait_lock(void)
{
	int n = 0x10000;

	do {
		n--;
		if (readl(SYS_PLL_BASE + 0x18) & 1)
			return;
	} while (n);
}

static u32 dpll_ssc;	/* never set: spread spectrum off */

static void dpll_init(void)
{
	ddr_sys_pll_set();
	ddr_sys_pll_wait_lock();
	if (ddr_pass)
		writel(0xde, DPLL_BASE + 0x10);
	writel(0xff, DPLL_BASE + 0x180);
	writel(0, DPLL_BASE + 0x4);
	writel(0, DPLL_BASE + 0x8);
	writel(ddr_cfg_dpll, DPLL_CFG);
	reg_field_write(DPLL_BASE + 0x10, 1, 6, 0);
	reg_field_write(DPLL_BASE + 0x10, 4, 2, 0);
	reg_field_write(DPLL_BASE + 0x10, 1, 1, 0);
	writel(0, DPLL_BASE + 0x10);
	writel(0xffff, DPLL_BASE + 0x4);
	writel(0xffff, DPLL_BASE + 0x8);
	reg_field_write(DPLL_BASE + 0xc, 4, 0, dpll_ssc);
	reg_field_write(DPLL_BASE + 0xc, 1, 4, 1);
}

static void ddr_ctl_misc_set(void)
{
	ddr_reg_field(0x122, 2, 0x18, 1);
	reg_field_write(0x100470, 0x10, 0, 0xffff);
	reg_field_write(0x100470, 0x10, 0x10, 0xffff);
	reg_field_write(0x100474, 0x10, 8, 0xffff);
	reg_field_write(0x100468, 0x10, 0x10, 0x3fff);
	reg_field_write(0x10046c, 0x10, 8, 0xfff);
}

/* Drive strength, DQ/CA bit swizzle of this board. */
static void ddr_board_set(void)
{
	ddr_reg_field(0x80d, 4, 0x18, ddr_cfg_dq_drv);
	ddr_reg_field(0x80e, 4, 0, ddr_cfg_dq_drv);
	ddr_reg_field(0x82d, 4, 8, ddr_cfg_dq_drv);
	ddr_reg_field(0x82d, 4, 0x10, ddr_cfg_dq_drv);
	ddr_reg_field(0x842, 4, 8, ddr_cfg_dq_drv);
	ddr_reg_field(0x81d, 4, 0x18, ddr_cfg_dq_drv);
	ddr_reg_field(0x837, 4, 0x10, ddr_cfg_dq_drv);
	ddr_reg_field(0x8d5, 2, 8, 1);
	ddr_reg_field(0x8d5, 2, 0x10, 3);
	ddr_reg_field(0x8d5, 2, 0x18, 2);
	ddr_reg_field(0x8d6, 2, 0, 0);
	ddr_reg_field(0x8d5, 1, 0, 1);
	ddr_reg_field(0x1536, 0x10, 0, 0x1203);
	ddr_reg_field(0x1025, 1, 0x10, 0);
	ddr_reg_field(0x1125, 1, 0x10, 0);
	ddr_reg_field(0x1225, 1, 0x10, 0);
	ddr_reg_field(0x1325, 1, 0x10, 1);
	ddr_reg_field(0x123, 1, 0, 1);
	ddr_reg_field(0x106b, 0x20, 0, 0x01324657);	/* DQ swizzle, slice 0 */
	ddr_reg_field(0x106c, 4, 0, 8);
	ddr_reg_field(0x116b, 0x20, 0, 0x76543210);	/* slice 1 */
	ddr_reg_field(0x116c, 4, 0, 8);
	ddr_reg_field(0x126b, 0x20, 0, 0x76543210);	/* slice 2 */
	ddr_reg_field(0x126c, 4, 0, 8);
	ddr_reg_field(0x136b, 0x20, 0, 0x01324567);	/* slice 3 */
	ddr_reg_field(0x136c, 4, 0, 8);
	ddr_reg_field(0x8d2, 4, 0, 0);			/* CA swizzle */
	ddr_reg_field(0x8d2, 4, 8, 1);
	ddr_reg_field(0x8d2, 4, 0x10, 2);
	ddr_reg_field(0x8d2, 4, 0x18, 3);
	ddr_reg_field(0x8d3, 4, 0, 4);
	ddr_reg_field(0x8d3, 4, 8, 5);
	ddr_reg_field(0x999, 4, 0x18, 4);
	ddr_reg_field(0x99a, 4, 0, 5);
	ddr_reg_field(0x99a, 4, 8, 6);
	ddr_reg_field(0x99a, 4, 0x10, 7);
	ddr_reg_field(0x8d1, 1, 0x18, 1);
	reg_field_write(DDR_PHY_BASE + 0x014, 2, 0x18, 1);
	reg_field_write(DDR_PHY_BASE + 0x414, 2, 0x18, 1);
	reg_field_write(DDR_PHY_BASE + 0x814, 2, 0x18, 1);
	reg_field_write(DDR_PHY_BASE + 0xc14, 2, 0x18, 1);
}

static void ddr_pi_misc_set(void)
{
	ddr_reg_field(0x80a, 0x20, 0, 5);
	ddr_reg_field(0x80b, 5, 0, 2);
	ddr_reg_field(0xab, 2, 0, 2);
}

static int ddr_load_cfg(void)
{
	u32 i;

	write_pairs(ddr_ctl_regs, ARRAY_SIZE(ddr_ctl_regs));
	ddr_reg_field(0x16f, 0x20, 0, 0xffffffff);
	ddr_reg_field(0x178, 0x20, 0, 0xffffffff);
	ddr_reg_field(0x181, 0x20, 0, 0xffffffff);
	for (i = 0; i < ARRAY_SIZE(ddr_pi_regs); i++)
		writel(ddr_pi_regs[i], DDR_PI_BASE + i * 4);
	ddr_pi_misc_set();
	write_pairs(ddr_phy_regs, ARRAY_SIZE(ddr_phy_regs));
	write_pairs(ddr_phy_board_regs, ARRAY_SIZE(ddr_phy_board_regs));
	write_pairs(ddr_ctl_timing_regs, ARRAY_SIZE(ddr_ctl_timing_regs));
	write_pairs(ddr_pi_timing_regs, ARRAY_SIZE(ddr_pi_timing_regs));
	ddr_ctl_misc_set();
	ddr_board_set();
	ddr_reg_field(0x152a, 7, 0x18, 1);
	ddr_reg_field(0x1556, 3, 0, 0);
	ddr_reg_field(0x1524, 3, 8, 0);
	ddr_reg_field(0x1537, 3, 0x10, 0);
	ddr_reg_field(0x153f, 1, 0, 1);
	ddr_reg_field(0x9d, 6, 8, 0);
	write_fields(ddr_phy_fields, ARRAY_SIZE(ddr_phy_fields));
	if (ddr_cfg & DDR_CFG_EXTRA_FIELDS)
		write_fields(ddr_phy_fields_cfg100, ARRAY_SIZE(ddr_phy_fields_cfg100));
	switch (ddr_cfg & DDR_CFG_MODE_MASK) {
	case 0x200:
		write_fields(ddr_phy_fields_mode2, ARRAY_SIZE(ddr_phy_fields_mode2));
		break;
	case 0x000:
		write_fields(ddr_phy_fields_mode0, ARRAY_SIZE(ddr_phy_fields_mode0));
		break;
	case 0x300:
		write_fields(ddr_phy_fields_mode3, ARRAY_SIZE(ddr_phy_fields_mode3));
		break;
	case 0x100:
		write_fields(ddr_phy_fields_mode1, ARRAY_SIZE(ddr_phy_fields_mode1));
		break;
	}
	return 0;
}

/*
 * Wait for CTL/PI init and training to finish. Meanwhile the controller
 * may ask for a DDR clock change (DPLL_FREQ_REQ bit 1, the frequency
 * index in bits 6:2): set the PLL for it and acknowledge with bit 0.
 */
static void ddr_wait_init_done(void)
{
	int timeout = 0x1000000;
	u32 start = get_curr_time(SEC);
	u32 req, idx, done;

	do {
		req = readl(DPLL_FREQ_REQ);
		idx = req >> 2 & 0x1f;
		if (req >> 1 & 1) {
			if (idx == 0) {
				DELAY(200);
				writel(0, DPLL_CFG);
				DELAY(200);
				DELAY(200);
				writel(req | 1, DPLL_FREQ_REQ);
				DELAY(100);
				req &= ~1;
				writel(req, DPLL_FREQ_REQ);
			}
			if (idx == 1) {
				DELAY(200);
				writel(ddr_cfg_dpll, DPLL_CFG);
				DELAY(200);
				writel(req | 1, DPLL_FREQ_REQ);
				DELAY(100);
				req &= ~1;
				writel(req, DPLL_FREQ_REQ);
			}
			if (idx == 2) {
				DELAY(200);
				writel(ddr_cfg_dpll, DPLL_CFG);
				DELAY(100);
				DELAY(100);
				writel(req | 1, DPLL_FREQ_REQ);
				DELAY(100);
				writel(req & ~1, DPLL_FREQ_REQ);
			}
		}
		DELAY(100);
		done = readl(DPLL_STATUS) >> 13 & 1;
		if (start - get_curr_time(SEC) > 1)
			timeout = 0;
		if (done == 1)
			break;
	} while (timeout-- != 0);
	if (timeout < 0) {
		info("ddr init timeout!\r\n");
	} else {
		writel(done, DDR_REG_BASE + 0x4ec);
		info("ddr init success!\r\n");
	}
}

static void phy_ca_update(void)
{
	u32 v = ddr_reg_read(PHY_CA_UPDATE, 1);

	ddr_reg_write(PHY_CA_UPDATE, bits_set(1, v, 24, 24), 1, 0);
}

static void phy_ca_regs_save(void)
{
	int i;

	for (i = 0; i < 10; i++)
		phy_ca_regs[i] = ddr_reg_read(PHY_CA_SLAVE_DELAY + i, 1);
}

/* PHY CA slave delays: two per register in the first six, then one. */
static void phy_ca_delay_save(void)
{
	int i;

	phy_ca_regs_save();
	for (i = 0; i < 6; i++) {
		phy_ca_delay[2 * i] = bits_get(phy_ca_regs[i], 9, 0);
		phy_ca_delay[2 * i + 1] = bits_get(phy_ca_regs[i], 25, 16);
	}
	for (i = 6; i < 10; i++)
		phy_ca_delay[i + 6] = bits_get(phy_ca_regs[i], 9, 0);
}

static void phy_ca_delay_restore(void)
{
	int i;

	for (i = 0; i < 6; i++) {
		phy_ca_regs[i] = bits_set(phy_ca_delay[2 * i], phy_ca_regs[i], 9, 0);
		phy_ca_regs[i] = bits_set(phy_ca_delay[2 * i + 1], phy_ca_regs[i], 25, 16);
	}
	for (i = 6; i < 10; i++)
		phy_ca_regs[i] = bits_set(phy_ca_delay[i + 6], phy_ca_regs[i], 9, 0);
	for (i = 0; i < 10; i++)
		ddr_reg_write(PHY_CA_SLAVE_DELAY + i, phy_ca_regs[i], 1, 0);
	phy_ca_update();
}

/* Mode register read of mr on channel ch (both ranks' bytes). */
static int ddr_mrr(int ch, int mr)
{
	int tries = 0, ok = 1;
	u32 done, v;

	ddr_reg_write(CTL_MRR_CMD, mr * 0x100 + ch * 0x10000, 0, 0);
	ddr_reg_write(CTL_MRR_CMD, mr * 0x100 + ch * 0x10000 + 0x1000000, 0, 0);
	do {
		mdelay(100);
		done = bits_get(ddr_reg_read(CTL_INT_STATUS_MR, 0), 2, 2);
		if (++tries == 30) {
			ok = 0;
			break;
		}
	} while (!done);
	if (bits_get(ddr_reg_read(CTL_INT_STATUS_MR, 0), 0, 0) == 1) {
		v = bits_get(ddr_reg_read(CTL_MRR_ERR, 0), 0, 0);
		ok = 0;
		info("ERROR: MRR error bit is set %x\r\n", v);
	}
	if (done == 1) {
		v = ddr_reg_read(CTL_INT_ACK_MR, 0);
		ddr_reg_write(CTL_INT_ACK_MR, bits_set(1, v, 2, 2), 0, 0);
	} else {
		info("ERROR: MRR Operation Failed \r\n");
		ok = 0;
	}
	v = ddr_reg_read(CTL_MRR_DATA0, 0);
	vref_ca[0] = bits_get(v, 7, 0);		/* TrainedVREFCA_CH0 */
	vref_ca[3] = bits_get(v, 15, 8);
	vref_ca[2] = bits_get(v, 23, 16);	/* TrainedVREFCA_CH1 */
	vref_ca[1] = bits_get(v, 31, 24);
	mr_data_extra = bits_get(ddr_reg_read(CTL_MRR_DATA1, 0), 7, 0);
	return ok;
}

static void ddr_start(void)
{
	ddr_reg_field(0x81a, 1, 0x18, 0);
	ddr_reg_field(0x123, 1, 0, 1);
	reg_field_write(DDR_PHY_BASE + 0x1558, 4, 0, ddr_cfg_phy);
	reg_field_write(DDR_PHY_BASE + 0x0f4, 1, 8, 1);
	reg_field_write(DDR_PHY_BASE + 0x4f4, 1, 8, 1);
	reg_field_write(DDR_PHY_BASE + 0x8f4, 1, 8, 1);
	reg_field_write(DDR_PHY_BASE + 0xcf4, 1, 8, 1);
	ddr_reg_field(0x1400, 10, 0x10, 0x30);
	ddr_reg_field(0x1401, 10, 0x10, 0x30);
	ddr_reg_field(0x1061, 4, 0x10, 1);
	ddr_reg_field(0x1161, 4, 0x10, 1);
	ddr_reg_field(0x1261, 4, 0x10, 1);
	ddr_reg_field(0x1361, 4, 0x10, 1);
	ddr_reg_field(0x1020, 6, 8, 8);
	ddr_reg_field(0x1120, 6, 8, 8);
	ddr_reg_field(0x1220, 6, 8, 8);
	ddr_reg_field(0x1320, 6, 8, 8);
	ddr_reg_field(0x1064, 4, 8, 1);
	ddr_reg_field(0x1164, 4, 8, 1);
	ddr_reg_field(0x1264, 4, 8, 1);
	ddr_reg_field(0x1364, 4, 8, 1);
	ddr_reg_field(0x100e, 3, 8, 5);
	ddr_reg_field(0x110e, 3, 8, 5);
	ddr_reg_field(0x120e, 3, 8, 5);
	ddr_reg_field(0x130e, 3, 8, 5);
	ddr_reg_field(0x100d, 3, 0x18, 2);
	ddr_reg_field(0x110d, 3, 0x18, 2);
	ddr_reg_field(0x120d, 3, 0x18, 2);
	ddr_reg_field(0x130d, 3, 0x18, 2);
	ddr_reg_field(0x100d, 6, 0x10, 1);
	ddr_reg_field(0x110d, 6, 0x10, 1);
	ddr_reg_field(0x120d, 6, 0x10, 1);
	ddr_reg_field(0x130d, 6, 0x10, 1);
	ddr_reg_field(0x886, 3, 8, 5);
	ddr_reg_field(0x842, 5, 0x10, 1);
	ddr_reg_field(0x1024, 3, 8, 2);
	ddr_reg_field(0x1124, 3, 8, 2);
	ddr_reg_field(0x1224, 3, 8, 2);
	ddr_reg_field(0x1324, 3, 8, 2);
	ddr_reg_field(0x89e, 3, 0x18, 5);
	ddr_reg_field(0x83c, 4, 8, 1);
	ddr_reg_field(0x1401, 3, 0, 2);
	ddr_reg_field(0x107d, 5, 0x18, 9);
	ddr_reg_field(0x117d, 5, 0x18, 9);
	ddr_reg_field(0x127d, 5, 0x18, 9);
	ddr_reg_field(0x137d, 5, 0x18, 9);
	ddr_reg_field(0x1062, 10, 0, 0xc0);
	ddr_reg_field(0x1162, 10, 0, 0xc0);
	ddr_reg_field(0x1262, 10, 0, 0xc0);
	ddr_reg_field(0x1362, 10, 0, 0xc0);
	ddr_reg_field(0x1052, 9, 0x10, 0x20);
	ddr_reg_field(0x1053, 0x20, 0, 0x200020);
	ddr_reg_field(0x1054, 0x20, 0, 0x200020);
	ddr_reg_field(0x1055, 0x20, 0, 0x200020);
	ddr_reg_field(0x1056, 0x20, 0, 0x200020);
	ddr_reg_field(0x1152, 9, 0x10, 0x20);
	ddr_reg_field(0x1153, 0x20, 0, 0x200020);
	ddr_reg_field(0x1154, 0x20, 0, 0x200020);
	ddr_reg_field(0x1155, 0x20, 0, 0x200020);
	ddr_reg_field(0x1156, 0x20, 0, 0x200020);
	ddr_reg_field(0x1252, 9, 0x10, 0x20);
	ddr_reg_field(0x1253, 0x20, 0, 0x200020);
	ddr_reg_field(0x1254, 0x20, 0, 0x200020);
	ddr_reg_field(0x1255, 0x20, 0, 0x200020);
	ddr_reg_field(0x1256, 0x20, 0, 0x200020);
	ddr_reg_field(0x1352, 9, 0x10, 0x20);
	ddr_reg_field(0x1353, 0x20, 0, 0x200020);
	ddr_reg_field(0x1354, 0x20, 0, 0x200020);
	ddr_reg_field(0x1355, 0x20, 0, 0x200020);
	ddr_reg_field(0x1356, 0x20, 0, 0x200020);
	ddr_reg_field(0x95b, 0x18, 0, 0xfffff);
	ddr_reg_field(0xab, 2, 8, 2);
	ddr_reg_field(0x966, 4, 0x10, 0);
	ddr_reg_field(0x1428, 1, 8, 0);
	if (ddr_cfg & DDR_CFG_EXTRA_FIELDS)
		ddr_reg_field(0x1522, 4, 4, 7);
	if (ddr_pass == 0) {
		if ((ddr_cfg & DDR_CFG_MODE_MASK) == DDR_CFG_MODE_MASK) {
			ddr_reg_field(0x966, 2, 8, 0);
			ddr_reg_field(0x93b, 2, 0x18, 0);
			ddr_reg_field(0x93b, 2, 0x10, 0);
			ddr_reg_field(0x936, 2, 0x10, 0);
		}
	} else {
		/* second pass: CA training off, first pass results back */
		ddr_reg_field(0x946, 2, 0, 0);
		ddr_reg_field(0x1428, 1, 8, 0);
		ddr_reg_field(0x1428, 1, 0, 0);
		phy_ca_delay_restore();
	}
	ddr_reg_field(PI_START, 1, 0, 1);
	ddr_reg_field(CTL_START, 1, 0, 1);
	ddr_wait_init_done();
}

int ddr_init(void)
{
	int passes = 1;

	if ((ddr_cfg & DDR_CFG_MODE_MASK) == DDR_CFG_MODE_MASK)
		passes = 2;
	for (ddr_pass = 0; ddr_pass < passes; ddr_pass++) {
		dpll_init();
		if (ddr_load_cfg())
			return -1;
		ddr_start();
		if ((ddr_cfg & DDR_CFG_MODE_MASK) == DDR_CFG_MODE_MASK && ddr_pass == 0) {
			phy_ca_delay_save();
			if (!ddr_mrr(0, 12) || !ddr_mrr(1, 12))
				info("mr read fail\n");
		}
	}
	return 0;
}

/* The values the two-pass init trained, for a debug print. */
void ddr_print_ca_training(void)
{
	phy_ca_delay_save();
	ddr_mrr(0, 12);
	info("TrainedVREFCA_CH0 = 0x%x\r\n", vref_ca[0]);
	info("TrainedVREFCA_CH1 = 0x%x\r\n", vref_ca[2]);
	info("phy_CA0_slave_delay:%x\r\n", phy_ca_delay[0]);
	info("phy_CA1_slave_delay:%x\r\n", phy_ca_delay[1]);
	info("phy_CA2_slave_delay:%x\r\n", phy_ca_delay[4]);
	info("phy_CA3_slave_delay:%x\r\n", phy_ca_delay[5]);
	info("phy_CA4_slave_delay:%x\r\n", phy_ca_delay[6]);
	info("phy_CA5_slave_delay:%x\r\n", phy_ca_delay[7]);
}
