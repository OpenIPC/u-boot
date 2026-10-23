/*
    Project layer Main control function

    This file is implement by user mode

    @file       prj_main.c

    Copyright   Novatek Microelectronics Corp. 2015.  All rights reserved.
*/

#include "fuart.h"
#include "fat.h"
#include "rtc.h"
#include "timer.h"
#include "StorageDef.h"
#include "global.h"
#include "Clock.h"
#include "string.h"
#include "prj_main.h"
#include "debug.h"

#define _THUMB2 __attribute__((target("thumb")))

#if defined(_STORAGEINT_EMMC_NAND_COMBO_)
#if (CHECK_CARD_EXIST == ENABLE)
#undef CHECK_CARD_EXIST
#define CHECK_CARD_EXIST DISABLE
#endif

#endif

extern UINT32 LoaderInternalInfo[];

#if (DEBUG_MSG == DISABLE)
//weak function mechanism for debug msg output
extern void debug_disable_msg(void);
void debug_disable_msg(void) {}
#endif

#if (SPECIAL_KEY_UPDATE == ENABLE)
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

#if (CHECK_CARD_EXIST == ENABLE)
// MC16 default is config as input and pull up
_THUMB2 static BOOL prj_checkCardExist(void)
{
#if (STORAGE_EXT_TYPE == STORAGE_EXT_ETH)
	return TRUE;        // force ethernet probe DHCP server
#else
	return (gpio_getPin(DGPIO_CARD_DETECT) == 0 ? TRUE : FALSE);
#endif
}
#endif

#if 0
static BOOL bl_spiIdentify(UINT32 uiMfgID, UINT32 uiTypeID, UINT32 uiCapacityID, PSPI_IDENTIFY pIdentify)
{
	// Sample to support SST25VF032
	if ((uiMfgID == 0xBF) &&
		(uiCapacityID == 0x4A)) {
		pIdentify->bDualRead = FALSE;
		pIdentify->bSupportAAI = TRUE;
		pIdentify->bSupportEWSR = TRUE;
		pIdentify->uiFlashSize = 4 * 1024 * 1024;
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
_THUMB2 void prj_main(void)
{
	PSTORAGE_OBJ strg_obj = NULL;
	loader_setVersion(_LDR_VER_);

	debug_msg("\r\n");
	debug_msg("LD_VER ");
	debug_msg(Dec2HexStr2Bytes((LoaderInternalInfo[1] >> 28) & 0xF));
	debug_msg(".");
	debug_msg(Dec2HexStr2Bytes((LoaderInternalInfo[1] >> 24) & 0xF));
	debug_msg(".");
	debug_msg(Dec2HexStr2Bytes((LoaderInternalInfo[1] >> 16) & 0xFF));
	debug_msg("\r\n");

	// Print project layer info
	if((LoaderInternalInfo[6] & 0x100000) == 0x100000) {
		if(gpio_getPin((((LoaderInternalInfo[6]>>12)&0x7)*32) + ((LoaderInternalInfo[6]>>15)&0x1f))) {
			debug_msg(_LOADER_VERSION_STR2_);
		} else {
			debug_msg(_LOADER_VERSION_STR_);
		}
	} else {
		debug_msg(_LOADER_VERSION_STR_);
	}

	// Setup loader/fw file names
	loader_setUpdateFwName(_UPDATE_FW_NAME_);
	loader_setUpdateLdrName(_UPDATE_LOADER_NAME_);
	loader_setRunFwName(_RUN_FW_NAME_);
	// Setup FW_MAC_CODE_SIZE (i.e. FWResvSize in RTOS)

#if 0
	// Sample to hook spi flash extending function
	flash_installIdentifyCB(bl_spiIdentify);
#endif

#if 0
	// Sample to hook flash programming LED callback
	flash_installAccessCB(bl_flashLED);
#endif

#if (SPECIAL_KEY_UPDATE == ENABLE)
	loader_installSpecialKeyCB(prj_isSpecialKeyPressed);
#endif

#if (CHECK_CARD_EXIST == ENABLE)
	loader_installCardDetectCB(prj_checkCardExist);
#endif


#if defined(_STORAGEINT_NandSpi_)
	strg_obj = nand_get_storage_object();
#if (SPI_PINMUX_SETTING == SPI_PINMUX_4BITS)
	strg_obj->flash_setConfig(FLASH_CFG_ID_SPI_SUPPORT_4BITS, TRUE);
#endif
	loader_setStorageIntType(STORAGEINT_SPI_NAND, strg_obj);

	// set Nand Flash speed to 96MHz
	//strg_obj->flash_setFrequency(96);
#elif defined(_STORAGEINT_SpiFlash_)
	strg_obj = nor_get_storage_object();
#if (SPI_PINMUX_SETTING == SPI_PINMUX_4BITS)
	strg_obj->flash_setConfig(FLASH_CFG_ID_SPI_SUPPORT_4BITS, TRUE);
#endif
	loader_setStorageIntType(STORAGEINT_SPI_NOR, strg_obj);
#elif defined(_STORAGEINT_EMMC_)
	strg_obj = emmc_get_storage_object();
	loader_setStorageIntType(STORAGEINT_EMMC, strg_obj);
#elif defined(_STORAGEINT_EMMC_NAND_COMBO_)
	if (top_get_bs() == BOOT_SOURCE_SPI_NAND_2K || top_get_bs() == BOOT_SOURCE_SPI_NAND_4K) {
		debug_msg_var("NAND,BS=", top_get_bs());
		strg_obj = nand_get_storage_object();
#if (SPI_PINMUX_SETTING == SPI_PINMUX_4BITS)
		strg_obj->flash_setConfig(FLASH_CFG_ID_SPI_SUPPORT_4BITS, TRUE);
#endif
		loader_setStorageIntType(STORAGEINT_SPI_NAND, strg_obj);
	} else if (top_get_bs() == BOOT_SOURCE_EMMC_4BIT || top_get_bs() == BOOT_SOURCE_EMMC_8BIT) {
		debug_msg("eMMC");
		strg_obj = emmc_get_storage_object();
		if (top_get_bs() == BOOT_SOURCE_EMMC_8BIT) {
			strg_obj->flash_setConfig(FLASH_CFG_ID_EMMC_SUPPORT_8BITS, TRUE);
			debug_msg("[8bit]\r\n");
		} else {
			debug_msg("[4bit]\r\n");
		}
		loader_setStorageIntType(STORAGEINT_EMMC, strg_obj);
	} else if (top_get_bs() ==  BOOT_SOURCE_SPI) {
		debug_msg("NOR\r\n");
		strg_obj = nor_get_storage_object();
#if (SPI_PINMUX_SETTING == SPI_PINMUX_4BITS)
		strg_obj->flash_setConfig(FLASH_CFG_ID_SPI_SUPPORT_4BITS, TRUE);
#endif
		loader_setStorageIntType(STORAGEINT_SPI_NOR, strg_obj);
	} else {
		debug_msg_var("unknow BS=", top_get_bs());
	}
#endif
}

