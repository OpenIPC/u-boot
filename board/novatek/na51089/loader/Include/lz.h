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
#if (_ROM_PUBLIC_API_ == 1)
#define LZ_Un_compress(a,b,c)	rom_LZ_Uncompress(a,b,c)
extern unsigned int rom_LZ_Uncompress(UINT8 *in, UINT8 *out, UINT32 insize)__attribute__ ((section (".part1")));
extern unsigned int LZ_Uncompress(UINT8 *in, UINT8 *out, UINT32 insize);
#else
extern unsigned int LZ_Uncompress(UINT8 *in, UINT8 *out, UINT32 insize)__attribute__ ((section (".part1")));
#define LZ_Un_compress(a,b,c)	LZ_Uncompress(a,b,c)
#endif
//extern void LZ_Uncompress(UINT8 *in, UINT8 *out, UINT32 insize)  __attribute__ ((section (".part1"), far));

//@}

#endif
