// SPDX-License-Identifier: GPL-2.0+
/*
 * Single-call xz decompression for U-Boot, over xz-embedded.
 */

#include <errno.h>
#include <unxz.h>
#include "xz_private.h"

int unxz(const void *in, size_t in_size, void *out, size_t *out_size)
{
	struct xz_buf b = {
		.in = in,
		.in_size = in_size,
		.out = out,
		.out_size = *out_size,
	};
	struct xz_dec *s;
	enum xz_ret ret;

	xz_crc32_init();
	xz_crc64_init();
	s = xz_dec_init(XZ_SINGLE, 0);
	if (!s)
		return -ENOMEM;
	ret = xz_dec_run(s, &b);
	xz_dec_end(s);
	*out_size = b.out_pos;
	if (ret == XZ_STREAM_END)
		return 0;
	return ret == XZ_MEM_ERROR || ret == XZ_BUF_ERROR ? -ENOSPC : -EINVAL;
}
