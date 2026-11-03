/**
    @file       main.h
    @brief      Header file for main function

    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.
*/
#ifndef _MAIN_H
#define _MAIN_H

#include "constant.h"

// FW checking method, support checksum & CRC now
#define FW_CHECK_NOCHECK            	0
#define FW_CHECK_CHECKSUM           	1
#define FW_CHECK_CRC                	2
#define DRAM_RANGE_SCAN_EN              (DISABLE)
#define _LOADER_COMPRESSED_             (ENABLE)
#define _LOADER_DUTY_CALIBRATION_       (DISABLE)
#define _LOADER_DUTY_CALIBRATION_LOG_   (DISABLE)
#define FW_CHECK_METHOD                 (FW_CHECK_CHECKSUM)




#define LDC_HEADER_SIZE     16
#if (_LOADER_COMPRESSED_ == ENABLE)
// header size for compressed loader (16 bytes)
//#define LDC_HEADER_SIZE     16

#include "lz.h"
#endif

extern UINT32 main(void) __attribute__ ((section (".part1")));


#endif
