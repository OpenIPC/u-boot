/**
    nand module driver.

    This file is the driver of storage module.

    @file       nand.h
    @ingroup    mIDrvStorage
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/

#ifndef _NAND_DEF_
#define _NAND_DEF_

#include "IOReg.h"

#define _SNAND_WINBOND_ID               (0xEF)
#define _SNAND_ESMT_ID                  (0xC8)
#define _SNAND_GD_ID                    (0xC8)
#define _SNAND_MICRON_ID                (0x2C)
#define _SNAND_ATO_ID                   (0x9B)
#define _SNAND_ETRON_ID                 (0xD5)
#define _SNAND_MXIC_ID     				(0xC2)
#define _SNAND_TOSHIBA_ID               (0x98)
#define _SNAND_DOSILICON_ID             (0xE5)



#define _GD_SPI_NAND_1Gb            	(0xF1)
#define _MXIC_SPI_NAND_1Gb            	(0x12)
#define _DOSILICON_NAND_1Gb				(0x71)
/**
    SPI NAND flash QE(quid enable) identify type

*/

//@{
typedef enum {
	SPINAND_QE_NONE = 0x0,             ///< SPI NAND flash not with QE bit(only by quad command)
	SPINAND_QE_FEATURE2_B0H_BIT0_TYPE1, ///< SPI NAND flash QE bit locate at feature field(0xB0H) bit[0]
} SPINAND_QE_TYPE;


/**
    SPI NAND flash twp plane identify type

*/
//@{
typedef enum {
	SPINAND_2_PLANE_NONE = 0x0,       				///< SPI NAND flash not 2 plane type
	SPINAND_2_PLANE_COLUMN_ADDR_BIT_12_AS_PLANE_SEL,///< SPI NAND flash use column addr bit[12] as plane select
} SPINAND_PLANE_TYPE;



/**
    SPI flash identification structure

    @note For SPIFLASH_IDENTIFY_CB
*/
typedef struct {
	UINT32	pagesize;
	UINT32	erasesize;
	UINT32	qe_opt;
	UINT32	plane_opt;
} SPINAND_IDENTIFY, *PSPINAND_IDENTIFY;

/**
    SPIFLASH identify callback

    Callback routine to be invoked after JEDEC ID is read from spi flash.
    Callback routine should check if read ID is supported.

    @note STRG_EXT_CMD_SPI_IDENTIFY_CB

    @param[in] first parameter  (JEDEC) manufacturer ID read from spi flash
    @param[in] second parameter (JEDEC) type ID read from spi flash
    @param[in] third parameter  (JEDEC) capacity ID read from spi flash
    @param[out] forth parameter flash identification returned to spi flash driver

    @return
        - @b TRUE: call back will handle identification of this flash. and PSPI_IDENTIFY will fill identifed information
        - @b FALSE: input ID is NOT supported/identified by call back
*/
typedef BOOL (*SPINAND_IDENTIFY_CB)(UINT32, UINT32, PSPINAND_IDENTIFY);




/**
 * struct nand_flash_dev - NAND Flash Device ID Structure
 * @mfr_id: manufecturer ID part of the full chip ID array (refers the same
 *          memory address as @id[0])
 * @dev_id: the device ID (the second byte of the full chip ID array)
 * @pagesize: size of the NAND page in bytes; if 0, then the real page size (as
 *            well as the eraseblock size) is determined from the extended NAND
 *            chip ID array)
 * @erasesize: NAND block size
 * @qe_opt: Quad enable type(please reference document to select this option)
 * @plane_opt: 2 plane nand option(please reference document to select this option)
 * @options: stores various chip bit options
 *
 */
typedef struct _nand_flash_dev {
	UINT32 	mfr_id;
	UINT32	dev_id;
	UINT32	pagesize;
	UINT32	erasesize;
	UINT32	qe_opt;
	UINT32	plane_opt;
} NAND_FLASH_DEV, *PNAND_FLASH_DEV;

#define SPI_ID_NAND(mfrid, devid, pagesz, erasesz, opts, plane_opt_type)\
	{ .mfr_id =(mfrid), .dev_id = (devid), .pagesize = (pagesz), \
	  .erasesize = (erasesz), .qe_opt = (opts), .plane_opt=(plane_opt_type) }

extern BOOL nand_identify(UINT32 uiMfgID, UINT32 uiTypeID, PSPINAND_IDENTIFY pIdentify);

#endif

