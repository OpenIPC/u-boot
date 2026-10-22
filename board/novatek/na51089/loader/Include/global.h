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

#define BOOT_SOURCE_SPI                0x00
#define BOOT_SOURCE_CARD               0x01
#define BOOT_SOURCE_SPI_NAND_2K        0x02
#define BOOT_SOURCE_SPI_NAND_RS_2K     0x03
#define BOOT_SOURCE_ETHERNET           0x04
#define BOOT_SOURCE_USB                0x05
#define BOOT_SOURCE_SPI_NAND_4K        0x06
#define BOOT_SOURCE_RESERVED           0x07
#define BOOT_SOURCE_EMMC_4BIT          0x08
#define BOOT_SOURCE_EMMC_8BIT          0x09
#define BOOT_SOURCE_SPI_NAND_RS_4K     0x0A
#define BOOT_SOURCE_USB_FULL           0x0B
#define BOOT_SOURCE_UART               0x0C

#define CHIPVER_A                       0x0

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
extern UINT32   utl_get_bootsrc(void);
extern UINT32   utl_get_chipversion(void);

#if (_ROM_PUBLIC_API_ == 1)
extern void 	rom_debug_msg(char* str);
extern void 	rom_debug_msg_var(char *str, int var);
extern void 	rom_Dec2HexStr(UINT32 data);
#endif
extern void load_dram_scan(UINT32 addr, UINT32 size)   __attribute__ ((section (".dram_text")));
extern void timer_delay(UINT32 US)   __attribute__ ((section (".part1")));

//@}

#endif
