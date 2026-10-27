/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __UNXZ_H
#define __UNXZ_H

#include <linux/types.h>

/* The xz stream magic: an "lzma" uImage may hold xz data */
#define XZ_MAGIC		"\3757zXZ"
#define XZ_MAGIC_SIZE		6

/*
 * Decompress the xz stream in[in_size] into out; *out_size is the room
 * there on entry, the bytes written on return. 0, or a negative errno.
 */
int unxz(const void *in, size_t in_size, void *out, size_t *out_size);

#endif
