/**
    Global utility header file

    Global utility header file

    @file       global.h
    @ingroup    mISYSUtil
    @note       Nothing
*/

#ifndef _GLOBAL_H
#define _GLOBAL_H

#include "constant.h"

/**
    @addtogroup mISYSUtil
*/
//@{

extern UINT32   bitCount(UINT32 data);
extern void     UTL_setDrvTmpBufferAddress(UINT32 addr);
extern UINT32   UTL_getDrvTmpBufferAddress(void);
extern char*    Dec2HexStr(UINT32 data);
//extern void     PrintDec2Hex(UINT32 data) __attribute__ ((section (".part1")));
extern void     rom_Dec2HexStr(UINT32 data) __attribute__ ((section (".part1")));
#define PrintDec2Hex(m)	rom_Dec2HexStr(m)
extern char*    Dec2HexStr2Bytes(UINT32 data);
extern void     PrintDec2HexStr2Bytes(UINT32 data) __attribute__ ((section (".part1")));
extern UINT32   DecStr2Int(char* str);
extern BOOL     UTL_canUpdateSecKey(void);
extern BOOL     utl_is_sram_fw(UINT32 fw_addr);
extern void     utl_test_checksum(UINT32 addr, UINT32 size);
extern void     utl_dram_protect_enable(UINT32 addr, UINT32 size);
extern void     utl_dram_protect_disable(void);
extern UINT32   utl_dram_protect_check(void);

extern void load_dram_scan(UINT32 addr, UINT32 size)   __attribute__ ((section (".dram_text")));
extern void timer_delay(UINT32 US)   __attribute__ ((section (".part1")));

//@}

#endif
