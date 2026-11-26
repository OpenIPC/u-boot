/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * SigmaStar infinity2m display: HDMI + VGA at a fixed mode, one picture
 * (NV12) in MOP window 0 -- what a boot logo needs.
 */
#ifndef __MS_DISP_H
#define __MS_DISP_H

#include <linux/types.h>

/* Start the output, "1080p60" etc.; the mode's size comes back. */
int ms_disp_start(const char *mode, int *width, int *height);
/* Show an NV12 picture (luma, then interleaved chroma; pitch = width,
 * a multiple of 16) in the window, scaled by the display to fit, centred. */
int ms_disp_show(phys_addr_t nv12, int width, int height);
/* the modes ms_disp_start knows, NULL after the last */
const char *ms_disp_mode_name(int i);

#endif
