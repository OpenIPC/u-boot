/*
 * OpenIPC: LZMA for U-Boot's BCL1 container (algorithm 11). NA51089's
 * mask ROM has a decoder the loader calls; NA51055's has none, so the
 * loader carries its own, Igor Pavlov's public domain LzmaDec as U-Boot
 * has it (lib/lzma). The input is an .lzma stream: 5 bytes of properties,
 * the unpacked size as a 64-bit word, then the data.
 */
/* OpenIPC: small, in Thumb, to fit the loader's 32 KiB */
#pragma GCC target("thumb")
#pragma GCC optimize("Os")

#include "LzmaDec.h"
#include "bl_lzma.h"

/* lc + lp <= 4: at most 1846 + 768 << 4 probabilities of 16 bits */
static unsigned char lzma_heap[0x8000] __attribute__((aligned(8)));

static void *lzma_alloc(void *p, size_t size)
{
	(void)p;
	return size <= sizeof(lzma_heap) ? lzma_heap : NULL;
}

static void lzma_free(void *p, void *address)
{
	(void)p;
	(void)address;
}

int bl_lzma_inflate(unsigned char *in, unsigned int in_size,
		    unsigned char *out, unsigned int out_size)
{
	ISzAlloc alloc = { lzma_alloc, lzma_free };
	SizeT src_len, dst_len = out_size;
	ELzmaStatus status;
	unsigned int len;

	if (in_size < LZMA_PROPS_SIZE + 8)
		return -1;
	len = in[LZMA_PROPS_SIZE] | in[LZMA_PROPS_SIZE + 1] << 8 |
	      in[LZMA_PROPS_SIZE + 2] << 16 |
	      (unsigned int)in[LZMA_PROPS_SIZE + 3] << 24;
	if (len < dst_len)
		dst_len = len;
	src_len = in_size - LZMA_PROPS_SIZE - 8;
	if (LzmaDecode(out, &dst_len, in + LZMA_PROPS_SIZE + 8, &src_len, in,
		       LZMA_PROPS_SIZE, LZMA_FINISH_END, &status, &alloc) != SZ_OK)
		return -1;
	return (int)dst_len;
}
