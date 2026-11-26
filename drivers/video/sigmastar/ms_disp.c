// SPDX-License-Identifier: GPL-2.0+
/*
 * SigmaStar infinity2m display for a boot logo: the display pipe (timing
 * generator, OP2, colour, VGA DAC) and the HDMI transmitter started at one
 * output mode, and MOP window 0 showing one NV12 picture.
 *
 * The output modes are register programs (ms_disp_tbl.h) generated from
 * SigmaStar's own display and HDMI TX drivers (the SSR621 boot release,
 * drivers/mstar/{disp,hdmitx}, run through openipc/tools/disp-regdump as
 * its cmd_bootlogo.c drives them): carrying those drivers would not fit in
 * U-Boot's 128 KiB. The MOP window follows hal_disp_mop.c: every register
 * goes through its double buffer (bank 0x1405 reg 0x7F bit 8).
 */
#include <common.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <mach/sigmastar.h>
#include <ms_disp.h>

struct ms_disp_op {
	u16 bank;
	u8 reg;
	u8 type;		/* 0 write, 1 per-mode write, 2 delay */
	u16 mask;
	u16 val;		/* type 1: index into ms_disp_vars[mode] */
};

struct ms_disp_mode {
	const char *name;
	u16 width, height;
};

#define MS_DISP_W(b, r, m, v)	{ (b), (r), 0, (m), (v) }
#define MS_DISP_V(b, r, m, i)	{ (b), (r), 1, (m), (i) }
#define MS_DISP_DELAY(us)	{ 0, 0, 2, (us) >> 16, (us) & 0xffff }

#include "ms_disp_tbl.h"

#define MOP_CTRL	0x1405	/* reg 0x7F bit 8: double-buffer write */
#define MOP_WIN0	0x1406

static int cur_w, cur_h;

static void reg_write(u16 bank, u8 reg, u16 mask, u16 val)
{
	void __iomem *a = (void __iomem *)SS_REG(bank, reg);

	if (mask == 0xffff)
		writew(val, a);
	else
		writew((readw(a) & ~mask) | (val & mask), a);
}

const char *ms_disp_mode_name(int i)
{
	return i < ARRAY_SIZE(ms_disp_modes) ? ms_disp_modes[i].name : NULL;
}

int ms_disp_start(const char *mode, int *width, int *height)
{
	const struct ms_disp_op *op;
	int m;

	for (m = 0; m < ARRAY_SIZE(ms_disp_modes); m++)
		if (!strcmp(mode, ms_disp_modes[m].name))
			break;
	if (m == ARRAY_SIZE(ms_disp_modes))
		return -EINVAL;

	for (op = ms_disp_prog; op < ms_disp_prog + ARRAY_SIZE(ms_disp_prog);
	     op++) {
		switch (op->type) {
		case 0:
			reg_write(op->bank, op->reg, op->mask, op->val);
			break;
		case 1:
			reg_write(op->bank, op->reg, op->mask,
				  ms_disp_vars[m][op->val]);
			break;
		default:
			udelay((u32)op->mask << 16 | op->val);
			break;
		}
	}
	cur_w = ms_disp_modes[m].width;
	cur_h = ms_disp_modes[m].height;
	*width = cur_w;
	*height = cur_h;
	return 0;
}

static void mop_write(u8 reg, u16 mask, u16 val)
{
	reg_write(MOP_WIN0, reg, mask, val);
	reg_write(MOP_CTRL, 0x7f, 0x0100, 0x0100);
	reg_write(MOP_CTRL, 0x7f, 0x0100, 0x0000);
}

int ms_disp_show(phys_addr_t nv12, int w, int h)
{
	int dw, dh, x, y;
	u32 y16 = nv12 >> 4, c16 = (nv12 + w * h) >> 4;

	if (!cur_w || (w & 15) || (h & 1) || nv12 & 15)
		return -EINVAL;
	/* fit, keeping the aspect: the MOP scales (4.12, source/destination) */
	if ((u64)w * cur_h > (u64)h * cur_w) {
		dw = cur_w;
		dh = (h * cur_w / w) & ~1;
	} else {
		dh = cur_h;
		dw = (w * cur_h / h) & ~1;
	}
	x = (cur_w - dw) / 2;
	y = (cur_h - dh) / 2;

	mop_write(0x0b, 0x1fff, w - 1);			/* source size */
	mop_write(0x0c, 0x1fff, h - 1);
	mop_write(0x06, 0x1fff, x);			/* the window, inclusive */
	mop_write(0x07, 0x1fff, x + dw - 1);
	mop_write(0x08, 0x1fff, y);
	mop_write(0x09, 0x1fff, y + dh - 1);
	mop_write(0x0d, 0x1fff, dw == w ? 0x1000 : w * 4096 / dw);
	mop_write(0x0e, 0x1fff, dh == h ? 0x1000 : h * 4096 / dh);
	mop_write(0x02, 0xffff, y16 & 0xffff);		/* luma, 16 B units */
	mop_write(0x03, 0x0fff, y16 >> 16);
	mop_write(0x04, 0xffff, c16 & 0xffff);		/* chroma */
	mop_write(0x05, 0x0fff, c16 >> 16);
	mop_write(0x0a, 0x1fff, w >> 4);		/* pitch, 16 B units */
	mop_write(0x01, 0xffff, 0);
	mop_write(0x00, 0x0001, 1);			/* on */
	return 0;
}
