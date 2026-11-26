// SPDX-License-Identifier: GPL-2.0+
/*
 * bootlogo: a picture on the SigmaStar infinity2m HDMI and VGA outputs (the
 * SSR621Q NVR die), in NV12 at the top of DRAM.
 *
 *   bootlogo [mode]          the OpenIPC logo, drawn at the mode's size
 *   bootlogo <addr> [mode]   a baseline JPEG at addr (loaded there by any
 *                            command: sf read, tftp, fatload ...), decoded
 *                            by TJpgDec; its width a multiple of 16
 *   bootlogo modes           the modes
 *
 * mode: 1080p60 (default, or $dispmode), 1080p50, 720p60, 720p50,
 * 1280x1024, 1024x768, 1440x900. A JPEG is scaled to fit and centred.
 */
#include <common.h>
#include <command.h>
#include <env.h>
#include <malloc.h>
#include <mapmem.h>
#include <cpu_func.h>
#include <linux/sizes.h>
#include <asm/global_data.h>
#include <mach/sigmastar.h>
#include <ms_disp.h>
#include "../lib/tjpgd/tjpgd.h"

DECLARE_GLOBAL_DATA_PTR;

#define LOGO_BUF_SIZE	SZ_8M	/* at the top of DRAM */
#define JPEG_MAX	SZ_1M
#define TJPGD_POOL	4096

struct logo {
	const u8 *src;
	size_t len, pos;
	u8 *y, *c;
	int w;
};

static size_t logo_in(JDEC *jd, u8 *buf, size_t n)
{
	struct logo *l = jd->device;

	if (n > l->len - l->pos)
		n = l->len - l->pos;
	if (buf)
		memcpy(buf, l->src + l->pos, n);
	l->pos += n;
	return n;
}

/*
 * One decoded block (RGB888, MCU-aligned: even coordinates) to NV12, BT.709
 * limited range -- the display's YUV-to-RGB matrix is the HDTV one.
 */
static int logo_out(JDEC *jd, void *bitmap, JRECT *r)
{
	struct logo *l = jd->device;
	int bw = r->right - r->left + 1, x, y;
	const u8 *p = bitmap;

	for (y = r->top; y <= r->bottom; y++) {
		for (x = r->left; x <= r->right; x++, p += 3) {
			int R = p[0], G = p[1], B = p[2];

			l->y[y * l->w + x] = 16 + ((47 * R + 157 * G + 16 * B) >> 8);
			if (!(x & 1) && !(y & 1)) {
				/* chroma from the 2x2 block's average */
				const u8 *q = p + 3, *d = p + bw * 3, *e = d + 3;
				int last_x = x == r->right, last_y = y == r->bottom;

				if (last_x)
					q = p;
				if (last_y)
					d = e = p;
				else if (last_x)
					e = d;
				R = (p[0] + q[0] + d[0] + e[0]) >> 2;
				G = (p[1] + q[1] + d[1] + e[1]) >> 2;
				B = (p[2] + q[2] + d[2] + e[2]) >> 2;
				u8 *c = l->c + (y / 2) * l->w + x;
				c[0] = 128 + ((-26 * R - 87 * G + 112 * B) >> 8);
				c[1] = 128 + ((112 * R - 102 * G - 10 * B) >> 8);
			}
		}
	}
	return 1;
}

/* Decode the JPEG at src into NV12 at buf; its size in *w, *h. */
static int logo_jpeg(const u8 *src, phys_addr_t buf, int *w, int *h)
{
	struct logo l = { .src = src, .len = JPEG_MAX };
	JDEC jd;
	JRESULT jr;
	void *pool;

	if (src[0] != 0xff || src[1] != 0xd8) {
		printf("bootlogo: no JPEG there\n");
		return -EINVAL;
	}
	pool = malloc(TJPGD_POOL);
	if (!pool)
		return -ENOMEM;
	jr = jd_prepare(&jd, logo_in, pool, TJPGD_POOL, &l);
	if (jr != JDR_OK || (jd.width & 15) || (jd.height & 1) ||
	    (u32)jd.width * jd.height * 3 / 2 > LOGO_BUF_SIZE) {
		printf("bootlogo: JPEG not usable (%d, %ux%u)\n", jr,
		       jd.width, jd.height);
		free(pool);
		return -EINVAL;
	}
	l.w = jd.width;
	l.y = map_sysmem(buf, LOGO_BUF_SIZE);
	l.c = l.y + jd.width * jd.height;
	jr = jd_decomp(&jd, logo_out, 0);
	free(pool);
	if (jr != JDR_OK) {
		printf("bootlogo: decode failed (%d)\n", jr);
		return -EINVAL;
	}
	*w = jd.width;
	*h = jd.height;
	return 0;
}

static int is_mode(const char *s)
{
	int w, h;

	return !ms_disp_mode_size(s, &w, &h);
}

static int do_bootlogo(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	const char *mode = NULL, *addr = NULL;
	phys_addr_t buf;
	int w, h, dw, dh, i;
	u8 *p;

	if (argc > 1 && !strcmp(argv[1], "modes")) {
		for (i = 0; ms_disp_mode_name(i); i++)
			printf("%s\n", ms_disp_mode_name(i));
		return 0;
	}
	for (i = 1; i < argc; i++) {
		if (is_mode(argv[i]))
			mode = argv[i];
		else if (!addr)
			addr = argv[i];
		else
			return CMD_RET_USAGE;
	}
	if (!mode)
		mode = env_get("dispmode");
	if (!mode)
		mode = "1080p60";
	if (ms_disp_mode_size(mode, &dw, &dh)) {
		printf("bootlogo: unknown mode %s (bootlogo modes)\n", mode);
		return CMD_RET_FAILURE;
	}

	buf = gd->bd->bi_dram[0].start + gd->bd->bi_dram[0].size - LOGO_BUF_SIZE;
	if (addr) {
		if (logo_jpeg(map_sysmem(hextoul(addr, NULL), JPEG_MAX), buf,
			      &w, &h))
			return CMD_RET_FAILURE;
	} else {
		w = dw;
		h = dh;
		p = map_sysmem(buf, LOGO_BUF_SIZE);
		if (ms_logo_draw(p, p + w * h, w, h)) {
			printf("bootlogo: drawing failed\n");
			return CMD_RET_FAILURE;
		}
	}
	flush_dcache_range(buf, buf + LOGO_BUF_SIZE);

	if (ms_disp_start(mode, &dw, &dh) || ms_disp_show(buf, w, h))
		return CMD_RET_FAILURE;
	printf("bootlogo: %s %ux%u on %s\n", addr ? "JPEG" : "OpenIPC logo",
	       w, h, mode);
	return 0;
}

U_BOOT_CMD(bootlogo, 3, 0, do_bootlogo,
	   "boot logo on HDMI and VGA (SigmaStar infinity2m)",
	   "[mode]         - the OpenIPC logo\n"
	   "bootlogo <addr> [mode]  - the baseline JPEG at addr\n"
	   "bootlogo modes          - the output modes (default $dispmode, else 1080p60)");
