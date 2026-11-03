/**
    nand module driver.

    This file is the driver of storage module.

    @file       nand.h
    @ingroup    mIDrvStorage
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/

#ifndef _NOR_DEF_
#define _NOR_DEF_

#include "IOReg.h"
#include "StorageDef.h"


/**
 * struct nand_flash_dev - NAND Flash Device ID Structure
 * @name: a human-readable name of the NAND chip
 * @dev_id: the device ID (the second byte of the full chip ID array)
 * @mfr_id: manufecturer ID part of the full chip ID array (refers the same
 *          memory address as @id[0])
 * @dev_id: device ID part of the full chip ID array (refers the same memory
 *          address as @id[1])
 * @pagesize: size of the NAND page in bytes; if 0, then the real page size (as
 *            well as the eraseblock size) is determined from the extended NAND
 *            chip ID array)
 * @chipsize: total chip size in MiB
 * @erasesize: eraseblock size in bytes (determined from the extended ID if 0)
 * @options: stores various chip bit options
 *
 */
typedef struct _nor_flash_dev {
	UINT32 	mfr_id;
	UINT32	dev_id;
	UINT32	capacity_id;
	UINT32	bDualRead;
	UINT32	uiQuadReadType;
	UINT32	uiFlashSize;
} NOR_FLASH_DEV, *PNOR_FLASH_DEV;

#define SPI_ID_NOR(mfrid, devid, capacityid, dual_read, quad_read_type, flash_size)\
	{ .mfr_id =(mfrid), .dev_id = (devid), .capacity_id =(capacityid), .bDualRead=(dual_read), \
	  .uiQuadReadType=(quad_read_type), .uiFlashSize=(flash_size) }


extern BOOL nor_identify(UINT32 uiMfgID, UINT32 uiTypeID, UINT32 uiCapacityID, PSPI_IDENTIFY pIdentify);
#endif
