/**
    Global loader header file

    Global loader header file

    @file       loader.h
    @ingroup    mISYSUtil
    @note       Nothing
*/

#ifndef _LOADER_H
#define _LOADER_H

#include "constant.h"
#include "StorageDef.h"

/**
    @name Special key detect CallBack

    Special key detect Prototype

    Project layer can detect special key in this callback.
    If callback returns TRUE, loader will try to find files on SD card.

    @return
        - @b TRUE: special key is pressed
        - @b FALSE: special key is NOT pressed
*/
//@{
typedef BOOL (*LDR_SPECIAL_KEY_CB)(void);
//@}

/**
    @name Card detect CallBack

    Card detect Prototype

    Project layer can detect SD card existence in this callback.
    If callback returns TRUE, loader will consider SD is plugged

    @return
        - @b TRUE: SD exist
        - @b FALSE: SD NOT exist
*/
//@{
typedef BOOL (*LDR_CARD_DETECT_CB)(void);
//@}
/**
    @name Recovery Trigger CallBack

    Recovery trigger Prototype

    Project layer can detect if the recovery flow should started.
    If callback returns TRUE, recovery flow will start.

    @return
        - @b TRUE: Recovery flow is triggered.
        - @b FALSE: Recovery flow is NOT triggered.
*/
//@{
typedef BOOL (*LDR_RECOVERY_TRIGGER_CB)(void);
//@}

typedef enum _STORAGEINT {
	STORAGEINT_UNOKNOWN,
	STORAGEINT_SPI_NAND,
	STORAGEINT_SPI_NOR,
	STORAGEINT_EMMC,
} STORAGEINT;

extern void loader_setUpdateFwName(char* fileName);
extern void loader_setUpdateLdrName(char* fileName);
extern void loader_setRunFwName(char* fileName);
extern void loader_setRecoveryFwName(char *fileName);
extern void loader_setRecoveryPartitionID(UINT32 partition_id);
extern void loader_setVersion(UINT32 version);
extern void loader_setStorageIntType(STORAGEINT type, PSTORAGE_OBJ strg_obj);
extern void loader_installSpecialKeyCB(LDR_SPECIAL_KEY_CB callback);
extern void loader_installCardDetectCB(LDR_CARD_DETECT_CB callback);
extern void loader_installRecoveryTriggerCB(LDR_RECOVERY_TRIGGER_CB callback);

#endif

