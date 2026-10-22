/**
    @file       main.h
    @brief      Header file for main function

    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.
*/
#ifndef _MAIN_H
#define _MAIN_H

#include "constant.h"
#include "fuart.h"

// FW checking method, support checksum & CRC now
#define FW_CHECK_NOCHECK            0
#define FW_CHECK_CHECKSUM           1
#define FW_CHECK_CRC                2

#define _LOADER_COMPRESSED_             (ENABLE)
#define _LOADER_DUTY_CALIBRATION_       (DISABLE)
#define _LOADER_DUTY_CALIBRATION_LOG_   (DISABLE)
#define FW_CHECK_METHOD                 (FW_CHECK_CHECKSUM)
#define DRAM_RANGE_SCAN_EN              (ENABLE)
#define _MAINCODE_COMPRESS_             (ENABLE)
#define LDC_HEADER_SIZE     16
#if (_LOADER_COMPRESSED_ == ENABLE)
// header size for compressed loader (16 bytes)
//#define LDC_HEADER_SIZE     16

#include "lz.h"
#endif

#define DDR_PARAM_LEVEL_0_ADDR      (BOOT_ROM_RAM_TEMP_ADDR)

#define DDR_PARAM_LEVEL_1_ADDR      (DDR_PARAM_LEVEL_0_ADDR + 7 * 4)

#define DDR_PARAM_LEVEL_2_ADDR      (DDR_PARAM_LEVEL_0_ADDR + 7 * 4 * 2)


// Represent
// SPI : 0x46495053     //'S''P''I''F'
//NAND : (1)            // Table found
//       (2)            // Table not found

#define NAND_TABLE_FLAG_ADDR        (DDR_PARAM_LEVEL_0_ADDR + 7 * 4 * 3)

#define NAND_TABLE_VERSION_ADDR     (NAND_TABLE_FLAG_ADDR + 4)

#define DDR_LEVEL0_OFFSET           0x10

#define DDR_LEVEL1_OFFSET           0x200

extern UINT32 main(void) __attribute__ ((section (".part1")));
extern void BL_PostHandle(void);

#endif
