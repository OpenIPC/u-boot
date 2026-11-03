/**
    Header file for CC (Core Communicator) module.

    This file is the header file that define the API and data type
    for CC module.

    @file       CC.h
    @ingroup    mIDrvSys_CC
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2015.  All rights reserved.
*/

#ifndef _CC_H
#define _CC_H

#define ENUM_DUMMY4WORD(name)   E_##name = 0x10000000

#define CC_CORE_CA53_CORE1  0
#define CC_CORE_CA53_CORE2  1
#define CC_CORE_DSP         2
#define CC_CORE_DSP2        3

typedef enum
{
    DMA_ID_1,                           ///< DMA Controller
    DMA_ID_2,                           ///< DMA Controller 2

    DMA_ID_COUNT,                       //< DMA controller count

    ENUM_DUMMY4WORD(DMA_ID)
} DMA_ID;

typedef enum
{
    CC_RESOURCE_UART,

    CC_RESOURCE_COUNT,                       //< DMA controller count

    ENUM_DUMMY4WORD(CC_RESOURCE_ID)
} CC_RESOURCE_ID;

typedef enum {
	CC_CORE_CPU1       = 0x1,         ///< 1st Core CPU
	CC_CORE_CPU2       = 0x2,         ///< 2nd Core CPU
	CC_CORE_MCU        = 0x3,         ///< MCU
	CC_CORE_MAX        = 0x3,
	CC_CORE_DUMMY      = 0xFFFFFFFF,
} CC_CORE_ID;

typedef struct {
	UINT32           cmd_buf[3];
} CC_CMD, *PCC_CMD;

extern void     cc_setCore2Base(UINT32 uiBaseAddr);
extern ER       cc_startCore(UINT32 uiCoreID) __attribute__ ((section (".part1")));
extern void     cc_stopCore(UINT32 uiCoreID);
extern UINT32 	dma_get_dram_capacity(DMA_ID id);
extern int      cc_getCoreCount(void) __attribute__ ((section (".part1"))); // total cnt = cc_getCoreCount() + 1
extern int      cc_getCoreId(void) __attribute__ ((section (".part1")));
extern void     cc_lock_resource(UINT32 uiCoreID, UINT32 resource) __attribute__ ((section (".part1")));
extern void     cc_unlock_resource(UINT32 uiCoreID, UINT32 resource) __attribute__ ((section (".part1")));
extern int      cc_send_core_cmd(int core_id, PCC_CMD p_cmd);

#endif
