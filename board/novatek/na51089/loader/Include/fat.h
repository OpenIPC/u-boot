/**
    File system library

    This file is the library of file system

    @file       fat.h
    @ingroup    mISYSFile
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/

#ifndef _FAT_H
#define _FAT_H

#include "constant.h"
#include "StorageDef.h"

/**
    @addtogroup mISYSFile
*/
//@{

#define FAT_READ_TOTAL_FILE_LENGTH          0

typedef void (*TOGGLE_LED)(UINT8 WithCount);

extern BOOL     fat_initFAT(UINT32 uiBuf, UINT32 uiSize);
extern UINT32   fat_getPartitionCount(void);
extern BOOL     fat_mountPartition(UINT32 partition_id);
extern void     fat_closeFAT(void);
extern BOOL     fat_open_rootfile(UINT8 *pfilename);
extern UINT32   fat_read_rootfile(UINT8 *pbuf, UINT32 uiRdLen);
extern UINT32   fat_getRootfileSize(void);
extern void     fat_close_rootfile(void);
extern void     fat_regToggleLED(TOGGLE_LED pToggleLED);
extern void     fat_installAccessCB(STRG_ACCESS_CB accessCB);

// For SD registration
extern BOOL     fat_internal_initFAT(UINT32 uiBuf, UINT32 uiSize);
extern BOOL     fat_internal_open_rootfile(UINT8 *pfilename);
extern UINT32   fat_internal_read_rootfile(UINT8 *pbuf, UINT32 uiRdLen);
extern void     fat_internal_close_rootfile(void);

//@}

#endif

