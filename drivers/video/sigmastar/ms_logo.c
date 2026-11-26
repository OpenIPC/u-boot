// SPDX-License-Identifier: GPL-2.0+
/*
 * The OpenIPC logo drawn into an NV12 frame of any size: white outlines
 * (ms_logo_paths.h, from the logo's SVG) filled with 4x4 anti-aliasing,
 * centred at 54% of the width on the navy gradient of OpenIPC's test
 * scenes. No picture file, no flash partition: the default boot logo.
 *
 * Integer only: the outlines are flattened (cubics into a fixed number of
 * lines) and filled scanline by scanline, nonzero rule, four sub-rows per
 * pixel row and 1/16 pixel horizontally.
 */
#include <common.h>
#include <malloc.h>
#include <linux/math64.h>
#include <ms_disp.h>

#include "ms_logo_paths.h"

#define FIX		8		/* device coordinates: 1/256 pixel */
#define SUB		4		/* sub-rows per pixel row */
#define CUBIC_STEPS	12
/* the navy, top and bottom of the screen (RGB) */
static const u8 navy_top[3] = { 18, 29, 102 };
static const u8 navy_bot[3] = { 11, 18, 66 };

struct edge {
	s32 x0, y0, x1, y1;		/* y0 < y1 */
	s8 dir;
};

struct raster {
	struct edge *e;
	int n, max;
	s64 scale_num;			/* path units to device FIX units */
	s32 scale_den;
	s32 ox, oy;
	s32 cx, cy, sx, sy;		/* current point, subpath start */
};

static s32 dev_x(struct raster *r, s32 v)
{
	return r->ox + (s32)div_s64(v * r->scale_num, r->scale_den);
}

static s32 dev_y(struct raster *r, s32 v)
{
	return r->oy + (s32)div_s64(v * r->scale_num, r->scale_den);
}

static void edge_add(struct raster *r, s32 x0, s32 y0, s32 x1, s32 y1)
{
	struct edge *e;

	if (y0 == y1 || r->n == r->max)
		return;
	e = &r->e[r->n++];
	if (y0 < y1) {
		*e = (struct edge){ x0, y0, x1, y1, 1 };
	} else {
		*e = (struct edge){ x1, y1, x0, y0, -1 };
	}
}

static void line_to(struct raster *r, s32 x, s32 y)
{
	edge_add(r, r->cx, r->cy, x, y);
	r->cx = x;
	r->cy = y;
}

/* B(t) for t = k / N, as integers: sum of the Bernstein terms over N^3 */
static s32 bez(s32 p0, s32 p1, s32 p2, s32 p3, int k)
{
	s64 t = k, u = CUBIC_STEPS - k;

	return (s32)div_s64(u * u * u * p0 + 3 * u * u * t * p1 +
			    3 * u * t * t * p2 + t * t * t * p3,
			    CUBIC_STEPS * CUBIC_STEPS * CUBIC_STEPS);
}

static int raster_build(struct raster *r)
{
	const s16 *p = ms_logo_path;
	int k;

	while (*p != LOGO_END) {
		switch (*p++) {
		case LOGO_MOVE:
			line_to(r, r->sx, r->sy);	/* close the last one */
			r->cx = r->sx = dev_x(r, p[0]);
			r->cy = r->sy = dev_y(r, p[1]);
			p += 2;
			break;
		case LOGO_LINE:
			line_to(r, dev_x(r, p[0]), dev_y(r, p[1]));
			p += 2;
			break;
		case LOGO_CUBIC: {
			s32 x0 = r->cx, y0 = r->cy;
			s32 x1 = dev_x(r, p[0]), y1 = dev_y(r, p[1]);
			s32 x2 = dev_x(r, p[2]), y2 = dev_y(r, p[3]);
			s32 x3 = dev_x(r, p[4]), y3 = dev_y(r, p[5]);

			for (k = 1; k <= CUBIC_STEPS; k++)
				line_to(r, bez(x0, x1, x2, x3, k),
					bez(y0, y1, y2, y3, k));
			p += 6;
			break;
		}
		case LOGO_CLOSE:
			line_to(r, r->sx, r->sy);
			break;
		default:
			return -EINVAL;
		}
	}
	line_to(r, r->sx, r->sy);
	return r->n == r->max ? -ENOSPC : 0;
}

/* Add the coverage of one sub-row's span [xa, xb) (FIX units) to acc. */
static void span(u16 *acc, int w, s32 xa, s32 xb)
{
	int a = xa >> (FIX - 4), b = xb >> (FIX - 4);	/* 1/16 pixel */
	int i = a >> 4, j = b >> 4;

	if (a < 0)
		a = 0, i = 0;
	if (b > w << 4)
		b = w << 4, j = w;
	if (a >= b)
		return;
	if (i == j) {
		acc[i] += b - a;
		return;
	}
	acc[i] += 16 - (a & 15);
	for (i++; i < j; i++)
		acc[i] += 16;
	if (j < w)
		acc[j] += b & 15;
}

/* Coverage of pixel row py into acc (0..16*SUB per pixel). */
static void raster_row(struct raster *r, int py, u16 *acc, int w,
		       s32 *xs, s8 *ds)
{
	int s, i, n, wind;
	s32 start = 0;

	memset(acc, 0, w * sizeof(*acc));
	for (s = 0; s < SUB; s++) {
		s32 ys = (py << FIX) + ((2 * s + 1) << FIX) / (2 * SUB);

		for (n = 0, i = 0; i < r->n; i++) {
			const struct edge *e = &r->e[i];
			s32 x;
			int k;

			if (ys < e->y0 || ys >= e->y1)
				continue;
			x = e->x0 + (s32)div_s64((s64)(e->x1 - e->x0) *
						 (ys - e->y0), e->y1 - e->y0);
			/* insertion sort by x */
			for (k = n; k > 0 && xs[k - 1] > x; k--) {
				xs[k] = xs[k - 1];
				ds[k] = ds[k - 1];
			}
			xs[k] = x;
			ds[k] = e->dir;
			n++;
		}
		for (wind = 0, i = 0; i < n; i++) {
			if (!wind)
				start = xs[i];
			wind += ds[i];
			if (!wind)
				span(acc, w, start, xs[i]);
		}
	}
}

static u8 lim_y(int r, int g, int b)
{
	return 16 + ((47 * r + 157 * g + 16 * b) >> 8);
}

static void lim_uv(int r, int g, int b, u8 *c)
{
	c[0] = 128 + ((-26 * r - 87 * g + 112 * b) >> 8);
	c[1] = 128 + ((112 * r - 102 * g - 10 * b) >> 8);
}

static void navy(int y, int h, int *rgb)
{
	int i;

	for (i = 0; i < 3; i++)
		rgb[i] = navy_top[i] + (navy_bot[i] - navy_top[i]) * y / (h - 1);
}

int ms_logo_draw(u8 *luma, u8 *chroma, int w, int h)
{
	struct raster r = { 0 };
	int lw = w * 1040 / 1920, lh = lw * MS_LOGO_H / MS_LOGO_W;
	int x0 = (w - lw) / 2, y0 = (h - lh) / 2, y, x, ret = -ENOMEM;
	u16 *acc0, *acc1;
	s32 *xs;
	s8 *ds;

	r.max = 4096;
	r.e = malloc(r.max * sizeof(*r.e));
	acc0 = malloc(2 * w * sizeof(*acc0));
	xs = malloc(r.max * sizeof(*xs));
	ds = malloc(r.max);
	if (!r.e || !acc0 || !xs || !ds)
		goto out;
	acc1 = acc0 + w;
	r.scale_num = (s64)lw << FIX;
	r.scale_den = MS_LOGO_W * MS_LOGO_SCALE;
	r.ox = x0 << FIX;
	r.oy = y0 << FIX;
	ret = raster_build(&r);
	if (ret)
		goto out;

	for (y = 0; y < h; y += 2) {
		int c0[3], c1[3], i;
		int inside = y + 1 >= y0 && y <= y0 + lh;
		u8 *l0 = luma + y * w, *l1 = l0 + w, *cr = chroma + y / 2 * w;
		u8 uv[2];

		navy(y, h, c0);
		navy(y + 1, h, c1);
		if (!inside) {
			memset(l0, lim_y(c0[0], c0[1], c0[2]), w);
			memset(l1, lim_y(c1[0], c1[1], c1[2]), w);
			lim_uv(c0[0], c0[1], c0[2], uv);
			for (x = 0; x < w; x += 2) {
				cr[x] = uv[0];
				cr[x + 1] = uv[1];
			}
			continue;
		}
		raster_row(&r, y, acc0, w, xs, ds);
		raster_row(&r, y + 1, acc1, w, xs, ds);
		for (x = 0; x < w; x += 2) {
			int a[4] = { acc0[x], acc0[x + 1], acc1[x], acc1[x + 1] };
			int sum = 0, rgb[3];

			for (i = 0; i < 4; i++) {
				int *c = i < 2 ? c0 : c1, k = a[i] * 255 / (16 * SUB);

				sum += k;
				(i < 2 ? l0 : l1)[x + (i & 1)] =
					lim_y(c[0] + (255 - c[0]) * k / 255,
					      c[1] + (255 - c[1]) * k / 255,
					      c[2] + (255 - c[2]) * k / 255);
			}
			sum /= 4;
			for (i = 0; i < 3; i++)
				rgb[i] = c0[i] + (255 - c0[i]) * sum / 255;
			lim_uv(rgb[0], rgb[1], rgb[2], cr + x);
		}
	}
	ret = 0;
out:
	free(ds);
	free(xs);
	free(acc0);
	free(r.e);
	return ret;
}
