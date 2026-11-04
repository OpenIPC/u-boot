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

extern void     cc_setCore2Base(UINT32 uiBaseAddr);
extern ER       cc_startCore(UINT32 uiCoreID);
extern void     cc_stopCore(UINT32 uiCoreID);
extern UINT32 	dma_get_dram_capacity(DMA_ID id);



#endif
