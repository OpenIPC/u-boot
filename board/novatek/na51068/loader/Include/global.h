/**
    Global utility header file

    Global utility header file

    @file       global.h
    @ingroup    mISYSUtil
    @note       Nothing
*/

#ifndef _GLOBAL_H
#define _GLOBAL_H

#include "board.h"


#define BOOT_SOURCE_SPI                 0x00
#define BOOT_SOURCE_SPI_NAND            0x01
#define BOOT_SOURCE_USB                 0x02
#define BOOT_SOURCE_ETHERNET            0x03
#define BOOT_SOURCE_UART                0x04
#define BOOT_SOURCE_CARD                0x05

#define BOOT_SOURCE_MSK                 0x07

#define CHIPVER_A                       0x0
#define CHIPVER_B                       0x1

/**
    @addtogroup mISYSUtil
*/
//@{
extern char*    Dec2HexStr(UINT32 data);
extern void     PrintDec2Hex(UINT32 data) __attribute__ ((section (".part1")));
extern char*    Dec2HexStr2Bytes(UINT32 data);
extern void     PrintDec2HexStr2Bytes(UINT32 data) __attribute__ ((section (".part1")));
extern UINT32   DecStr2Int(char* str);
extern UINT32   bitCount(UINT32 data);
extern void 	exit(int num);

extern int	    printf(char const *, ...) __attribute__ ((section (".part1")));;
extern BOOL     utl_is_sram_fw(UINT32 fw_addr);
extern UINT32   utl_get_bootsrc(void);
extern UINT32   utl_get_chipversion(void);
extern void     UTL_setDrvTmpBufferAddress(UINT32 addr);
extern UINT32   UTL_getDrvTmpBufferAddress(void);

#if 0
extern BOOL     UTL_canUpdateSecKey(void);
extern BOOL     utl_is_sram_fw(UINT32 fw_addr);
extern void     utl_test_checksum(UINT32 addr, UINT32 size);
extern void     utl_dram_protect_enable(UINT32 addr, UINT32 size);
extern void     utl_dram_protect_disable(void);
extern UINT32   utl_dram_protect_check(void);
extern UINT32   utl_dram_get_size(void);

extern void load_dram_scan(UINT32 addr, UINT32 size)   __attribute__ ((section (".dram_text")));

extern UINT32   dma_get_capacity(void);
#endif
extern __attribute__((target("thumb2"))) void udelay(UINT32 US)   __attribute__ ((section (".part1")));
extern __attribute__((target("thumb2"))) void timer0_tm1_init(void)	 __attribute__ ((section (".part1")));

#define timer_getSysTick()		((*(UINT32 volatile *)(REG_TMR010_BASE + 0x10)) / 16)
//@}

#endif
