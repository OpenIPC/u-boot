/*
 *      nand_ids.c
 *
 *      nand id table
 *
 *      Copyright (c) 2018 Novatek, ALL Rights Reserve
 */
#include "main.h"
#include "nor.h"
#include "constant.h"
#include "compiler.h"
#include "fuart.h"

#define _THUMB2 __attribute__((target("thumb")))


NOR_FLASH_DEV nvt_nor_ids[] = {
    //     manufactor ID   device id   capacity id   flash size   bus width
	SPI_ID_NOR(0xEF,       0x40,       0x18,         (16<<20),    (SPI_BUSWIDTH_DUAL|SPI_BUSWIDTH_QUAD_TYPE2)),
	SPI_ID_NOR(0xC8,       0x48,       0x1B,         (16<<23),    SPI_BUSWIDTH_OCTAL_TYPE1),  // GD55X01GE, 8-bit
};

_THUMB2 BOOL nor_identify(UINT32 uiMfgID, UINT32 uiTypeID, UINT32 uiCapacityID, PSPI_IDENTIFY pIdentify)
{
    BOOL    nor_ids_found = FALSE;
    UINT32	ui_nor_ids;
    for (ui_nor_ids = 0; ui_nor_ids < (sizeof(nvt_nor_ids) / sizeof(NOR_FLASH_DEV)); ui_nor_ids++) {
		if (uiMfgID == nvt_nor_ids[ui_nor_ids].mfr_id && uiTypeID == nvt_nor_ids[ui_nor_ids].dev_id && uiCapacityID == nvt_nor_ids[ui_nor_ids].capacity_id) {
			// For SPI Nor flash only
            pIdentify->uiBusWidth 		= nvt_nor_ids[ui_nor_ids].uiBusWidth;
			pIdentify->uiFlashSize		= nvt_nor_ids[ui_nor_ids].uiFlashSize;
			nor_ids_found      			= TRUE;
            break;
		}
    }

    return nor_ids_found;
}
