/**
    Decompress library header file

    Decompress library header file

    @file       lz.h
    @ingroup    mISYSUtil
    @note       Nothing
*/
#ifndef __NVT_LZ_DEF__
#define __NVT_LZ_DEF__

#include "constant.h"

/**
    @addtogroup mISYSUtil
*/
//@{

extern unsigned int LZ_Uncompress(UINT8 *in, UINT8 *out, UINT32 insize)  __attribute__ ((section (".part1")));
//extern void LZ_Uncompress(UINT8 *in, UINT8 *out, UINT32 insize)  __attribute__ ((section (".part1"), far));
extern int lzma_inflate(UINT8 *in_ptr, UINT32 in_size, UINT8 *out_ptr, UINT32  out_size, UINT8 *p_tmp, UINT32 tmp_size);

//@}

#endif
