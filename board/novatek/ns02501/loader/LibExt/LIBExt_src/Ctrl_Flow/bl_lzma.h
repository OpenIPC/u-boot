/* OpenIPC: LZMA for U-Boot's BCL1 container (bl_lzma.c) */
#ifndef _BL_LZMA_H
#define _BL_LZMA_H

/* Returns the unpacked length, or -1 */
int bl_lzma_inflate(unsigned char *in, unsigned int in_size,
		    unsigned char *out, unsigned int out_size);

#endif
