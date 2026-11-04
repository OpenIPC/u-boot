/*
    Project layer Main control function

    This file is implement by user mode

    @file       prj_main.c

    Copyright   Novatek Microelectronics Corp. 2015.  All rights reserved.
*/
/*
#include "fat.h"
#include "rtc.h"
#include "timer.h"
#include "StorageDef.h"
#include "global.h"
#include "Clock.h"
#include "string.h"
*/
#include "board.h"
#include "debug.h"
#include "mmc_api.h"
#include "fuart.h"
#include "prj_main.h"

extern UINT32 LoaderInternalInfo[];

#if (DEBUG_MSG == ENABLE)
//weak function mechanism for debug msg output
extern void config_dma_log(void);
void config_dma_log(void) {}
#endif

#if (SPECIAL_KEY_UPDATE == 1)
/**
    Check special key1(s) is(are) pressed.

    Check special key1(s) is(are) pressed or not.

    @return Key pressed status
        - @b TRUE: Special key(s) is(are) pressed
        - @b FALSE: Otherwise
*/
static BOOL prj_isSpecialKeyPressed(void)
{
    // Detect key example, must press shutter2 and playback mode keys
    // OUT_0: DGPIO 0
    // OUT_1: DGPIO 14
    // OUT_2: DGPIO 2
    // OUT_3: DGPIO 3
    // OUT_4: DGPIO 4
    // IN_0 : DGPIO 5
    // IN_1 : DGPIO 6
    // IN_2 : DGPIO 7
    // IN_3 : DGPIO 8
    //              KEY_IN_0    KEY_IN_1    KEY_IN_2    KEY_IN_3
    // KEY_OUT_0    MENU        SET         PLAYBACK    DISPLAY
    // KEY_OUT_1    WIDE        LEFT        MOVIE
    // KEY_OUT_2    TELE        DOWN                    TRASH
    // KEY_OUT_3    SHUTTER2    RIGHT       CAPTURE
    // KEY_OUT_4    SHUTTER1    UP
    // Add the necessary code..
    return TRUE;
}
#endif

#if (CHECK_CARD_EXIST == 1)
// MC16 default is config as input and pull up
static BOOL prj_checkCardExist(void)
{
#if (STORAGE_EXT_TYPE == STORAGE_EXT_ETH)
    return TRUE;        // force ethernet probe DHCP server
#else
    return (gpio_getPin(DGPIO_CARD_DETECT) == 0 ? TRUE : FALSE);
#endif
}
#endif

#if (CHECK_RECOVERY_TRIGGER == 1)
static BOOL prj_isRecoveryTriggered(void)
{
	#if 1
    return FALSE;
    #else
    //just a sample for using P_GPIO_0 high active to trigger recovery flow
    return (gpio_getPin(P_GPIO_0) == 1 ? TRUE : FALSE);
    #endif
}
#endif

#if 0
static BOOL bl_spiIdentify(UINT32 uiMfgID, UINT32 uiTypeID, UINT32 uiCapacityID, PSPI_IDENTIFY pIdentify)
{
    // Sample to support SST25VF032
    if ((uiMfgID == 0xBF) &&
        (uiCapacityID == 0x4A))
    {
        pIdentify->bDualRead = FALSE;
        pIdentify->bSupportAAI = TRUE;
        pIdentify->bSupportEWSR = TRUE;
        pIdentify->uiFlashSize = 4*1024*1024;
        return TRUE;
    }

    return FALSE;
}

static void bl_flashLED(void)
{
    // use GPIO to control LED
}
#endif

// main entry of project layer code
void prj_main(void)
{
	PSTORAGE_OBJ strg_obj = NULL;
	PSTORAGE_OBJ strg2_obj = NULL;
	loader_setVersion(_LDR_VER_);

	// Print project layer info

	if ((LoaderInternalInfo[6] & 0x100000) == 0x100000) {
		debug_msg(_LOADER_VERSION_STR2_);
	} else {
		debug_msg(_LOADER_VERSION_STR_);
	}

	// Setup loader/fw file names
	loader_setUpdateFwName(_UPDATE_FW_NAME_);
	loader_setUpdateLdrName(_UPDATE_LOADER_NAME_);
	loader_setRunFwName(_RUN_FW_NAME_);
	// Setup FW_MAC_CODE_SIZE (i.e. FWResvSize in RTOS)

#if defined(_STORAGEINT_NandSpi_)
	strg_obj = nand_get_storage_object();
	loader_setStorageIntType(STORAGEINT_SPI_NAND, strg_obj);
	strg2_obj = nor_get_storage_object();
	strg2_obj->flash_setConfig(FLASH_CFG_ID_CS_SELECT, 1);
#elif defined(_STORAGEINT_SpiFlash_)
	strg_obj = nor_get_storage_object();
	loader_setStorageIntType(STORAGEINT_SPI_NOR, strg_obj);
	strg2_obj = nand_get_storage_object();
	strg2_obj->flash_setConfig(FLASH_CFG_ID_CS_SELECT, 1);
#else
	#error "Internal storage not configured !!!";
#endif


#if 0
#if (SPI_PINMUX_SETTING == SPI_PINMUX_4BITS)
    flash_setConfig(FLASH_CFG_ID_SPI_SUPPORT_4BITS, TRUE);
#endif

#if 0
    // Sample to hook spi flash extending function
    flash_installIdentifyCB(bl_spiIdentify);
#endif

#if 0
    // Sample to hook flash programming LED callback
    flash_installAccessCB(bl_flashLED);
#endif

#if 0
    // Sample to enable eMMC 8 bits boot
    // *Note*: MUST ensure your PCB has BS[7] = HIGH
    if (INREG32(0xF0010000) & (1<<7)) {
        // If bootstrap pin[7] is pulled high
        // enable eMMC 8 bit
        debug_msg("Enable eMMC 8 bit mode\r\n");
        flash_setConfig(FLASH_CFG_ID_EMMC_SUPPORT_8BITS, TRUE);
    }
#endif

#if (SPECIAL_KEY_UPDATE == ENABLE)
    loader_installSpecialKeyCB(prj_isSpecialKeyPressed);
#endif

#if (CHECK_CARD_EXIST == ENABLE)
    loader_installCardDetectCB(prj_checkCardExist);
#endif

#if (CHECK_RECOVERY_TRIGGER == ENABLE)
	loader_setRecoveryPartitionID(2);
	loader_setRecoveryFwName(_RECOVERY_FW_NAME_);
    loader_installRecoveryTriggerCB(prj_isRecoveryTriggered);
#endif
#endif
}

