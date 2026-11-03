/*
 *      nand_ids.c
 *
 *      nand id table
 *
 *      Copyright (c) 2018 Novatek, ALL Rights Reserve
 */
#include "main.h"
#include "nand.h"
#include "constant.h"
#include "compiler.h"
#include "mmc_api.h"
#include "spi020.h"
#include "fuart.h"
#include "timer.h"
#include "debug.h"
#include "pll.h"

#define _THUMB2 __attribute__((target("thumb2")))


NAND_FLASH_DEV nvt_nand_ids[] = {
	//			manufactor ID	device id		  	page size(unit:bytes)	erase unit(block size, unit:bytes)	QE type								plane type
	SPI_ID_NAND(_SNAND_MXIC_ID, 	_MXIC_SPI_NAND_1Gb, 	2048, 					0x20000, 					SPINAND_QE_FEATURE2_B0H_BIT0_TYPE1,	SPINAND_2_PLANE_NONE),
	SPI_ID_NAND(_SNAND_TOSHIBA_ID, 	0xCD, 					4096, 					0x40000, 					SPINAND_QE_NONE,					SPINAND_2_PLANE_NONE),
	//DS35Q2GAxxx
	SPI_ID_NAND(0xE5, 				0x72, 					2048, 					0x20000, 					SPINAND_QE_NONE,					SPINAND_2_PLANE_COLUMN_ADDR_BIT_12_AS_PLANE_SEL),
	//MX35LFxGE4AB
	SPI_ID_NAND(_SNAND_MXIC_ID,		0x22, 					2048, 					0x20000, 					SPINAND_QE_NONE,					SPINAND_2_PLANE_COLUMN_ADDR_BIT_12_AS_PLANE_SEL),

};

_THUMB2 BOOL nand_identify(UINT32 uiMfgID, UINT32 uiTypeID, PSPINAND_IDENTIFY pIdentify)
{
    UINT32  ui_nand_ids;
    BOOL    nand_ids_found = FALSE;
    for (ui_nand_ids = 0; ui_nand_ids < (sizeof(nvt_nand_ids) / sizeof(NAND_FLASH_DEV)); ui_nand_ids++) {
		if (uiMfgID == nvt_nand_ids[ui_nand_ids].mfr_id && uiTypeID == nvt_nand_ids[ui_nand_ids].dev_id) {
			// For SPI Nor flash only
            pIdentify->pagesize	= nvt_nand_ids[ui_nand_ids].pagesize;
            pIdentify->erasesize= nvt_nand_ids[ui_nand_ids].erasesize;
			pIdentify->qe_opt   = nvt_nand_ids[ui_nand_ids].qe_opt;
			pIdentify->plane_opt= nvt_nand_ids[ui_nand_ids].plane_opt;
			nand_ids_found      = TRUE;
            break;
		}
    }
    return nand_ids_found;
}
