/**
    Storage module driver.

    This file is the driver of storage module.

    @file       StorageDef.h
    @ingroup    mIDrvStorage
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/

#ifndef _STORAGE_DEF_
#define _STORAGE_DEF_

#include "constant.h"

#define _STORAGE_OBJ_					1
/**
    @addtogroup mIDrvStorage
*/
//@{

/**
    @name External storage type

*/
//@{
#define EXT_STORAGE_TYPE_NONE           (0)
#define EXT_STORAGE_TYPE_SDIO1          (1)
#define EXT_STORAGE_TYPE_SDIO2          (2)
#define EXT_STORAGE_TYPE_USB            (3)
#define EXT_STORAGE_TYPE_ETH            (4)
//*}

/**
    @name SPI Quad read type

    @note for SPI_IDENTIFY
*/
//@{
#define     SPI_QUAD_NONE               0       ///< Not support
#define     SPI_QUAD_TYPE1              1       ///< Support quad read, QE(Quad Enable) bit is in Status Reigster[bit 6] and 0x6B command requires 8 dummy clocks.
#define     SPI_QUAD_TYPE2              2       ///< Support quad read, QE(Quad Enable) bit  is in Status Register[bit 9] and 0x6B command requires 8 dummy clocks.
#define     SPI_QUAD_TYPE3              3       ///< Support quad read, QE(Quad Enable) NOT exist and 0x6B command requires 8 dummy clocks
#define     SPI_QUAD_TYPE4              4       ///< Support quad read, QE(Quad Enable) bit  is in Status Register[bit 9] and 0x6B command requires 8 dummy clocks.
                                                ///< But QE should be modified by 0x31 command
//@}


/**
    @name SPI RDCR read type

    @note for SPI_IDENTIFY
*/
//@{
#define     SPI_RDCR_NONE               0       ///< Not support
#define     SPI_RDCR_TYPE1              1       ///< Support RDCR(0x15) bit[7..6] need config as 0x3

//@}

/**
    SPI flash identify descriptor

    @note For STRG_IDENTIFY_CB
*/
typedef struct
{
    BOOL    bSupportEWSR;                       ///< EWSR(0x50) command capability
                                                ///< - @b TRUE: support EWSR command
                                                ///< - @b FALSE: NOT support EWSR command
    BOOL    bSupportAAI;                        ///< AAI program(0xAD) command capability
                                                ///< - @b TRUE: ONLY support AAI program command
                                                ///< - @b FALSE: NOT support AAI program command, but support page program command
    BOOL    bDualRead;                          ///< Dual read (0x3B) command capability
                                                ///< - @b TRUE: support dual read comand. (Command value should be 0x3B.)
                                                ///< - @b FALSE: NOT support dual read command
    UINT32  uiQuadReadType;                     ///< Quad read type capability. (ONLY support quad command 0x6B)
                                                ///< - @b SPI_QUAD_NONE: NOT support quad read command
                                                ///< - @b SPI_QUAD_TYPE1: Support quad read command, QE in Status Register bit 6
                                                ///< - @b SPI_QUAD_TYPE2: Support quad read command, QE in Status Register bit 9
                                                ///< - @b SPI_QUAD_TYPE3: Support quad read command, QE NOT exist
                                                ///< - @b SPI_QUAD_TYPE4: Support quad read command, QE in Status Register bit 9 (but need 0x31 command)
    UINT32  uiFlashSize;                        ///< Flash total byte size. (unit: byte)
} SPI_IDENTIFY, *PSPI_IDENTIFY;

/**
    @name Storage Access CallBack

    Storage Access CallBack Prototype

    @return void
*/
//@{
typedef void (*STRG_ACCESS_CB)(void);   ///< Storage access call back prototype
//@}

/**
    @name Storage Identify CallBack

    Storage Identify CallBack Prototype

    @param[in] UINT32 uiMfgID   (JEDEC) manufacturer ID read from spi flash
    @param[in] UINT32 uiTypeID  (JEDEC) type ID read from spi flash
    @param[in] UINT32 uiCapacityID  (JEDEC) capacity ID read from spi flash
    @param[out] PSPI_IDENTIFY   flash identification returned to spi flash driver

    @return
        - @b TRUE: call back will handle identification of this flash. and PSPI_IDENTIFY will fill identifed information
        - @b FALSE: input ID is NOT supported/identified by call back
*/
//@{
typedef BOOL (*STRG_IDENTIFY_CB)(UINT32, UINT32, UINT32, PSPI_IDENTIFY);    ///< Storage identify call back prototype
//@}

/**
    @name Storage return value

    @note for flash_open(), flash_readSectors(), and flash_writeSectors()
*/
//@{
#define     E_OK                         0      ///< Success
#define     E_NAND_IDENTIFY_ERR         -1      ///< Read NAND ID (information) error
#define     E_NAND_INSUFFICINET_BLK     -2      ///< Bad block is too much to write data
#define     E_NAND_BLK_NOT_FOUND        -3      ///< Logic block number not found in physical block
#define     E_NAND_WRITE_PAGEDATA_ERR   -4      ///< Logic block number not found in physical block
#define     E_NAND_ERASEE_BLK_ERR       -5      ///< NAND erase block error
#define     E_NAND_READ_PAGE_ERR        -6      ///< NAND read page data error
#define     E_NAND_READ_BLK_ERR         -7      ///< NAND read block data error
#define     E_NAND_ID_NOT_SUPPORT       -8      ///< Not support NAND ID
#define     E_IDENTIFY_ERR              -9      ///< SPI IDENTIFY error
#define     E_SYS                       -10     ///< System Error
#define     E_NAND_ALIGN_ERR            -11     ///< Not block alignment
#define     E_NAND_BOUNDARY_ERR         -12     ///< boundary < write buffer
#define     E_OK_TABLE_FOUND             1      ///< Success & NAND table find
#define     E_OK_TABLE_NOT_FOUND         2      ///< Success & NAND table not find
//@}

/**
    @name Internal Storage Update Region

    @note for flash_readSectors(), and flash_writeSectors()
*/
//@{
#define     NAND_RW_LOADER               0      ///< Update Region is Loader (Update size is assumed 16 KB)
#define     NAND_RW_RESERVED             1      ///< Update Region is reserved area (Update size is assumed 16 KB)
#define     NAND_RW_FIRMWARE             2      ///< Update Region is FW (Update size is variable)
#define     NAND_RW_FIRMWARE_2           3      ///< Update Region is FW from 2-5(Update size is variable)
#define     NAND_RW_FIRMWARE_3           4      ///< Update Region is FW from 2-5(Update size is variable)
#define     NAND_RW_FIRMWARE_4           5      ///< Update Region is FW from 2-5(Update size is variable)
#define     NAND_RW_FIRMWARE_5           6      ///< Update Region is FW from 2-5(Update size is variable)
//@}

/**
    @name Internal Storage Configuration ID

    @note for flash_setConfig()
*/
//@{
#define     FLASH_CFG_ID_NONE                   0
#define     FLASH_CFG_ID_SPI_SUPPORT_4BITS      1   ///< Enable your project to support 4 bit SPI flash. ONLY enable when you ENSURE SPI_D0~SPI_D3 are connected to SPI flash in your PCB!!!
#define     FLASH_CFG_ID_EMMC_SUPPORT_8BITS     2   ///< Enable your project to support 8 bit EMMC. ONLY enable when you ENSURE EMMC_D0~SPI_D7 are connected to EMMC flash in your PCB!!!
#define	    FLASH_CFG_ID_CS_SELECT				3   ///< Select this flash (nand/nor) is connected with CS0 or CS1
#define 	FLASH_CFG_ID_SPI_IDENTIFY_CB		4	///< Install flash identify call back
//@}

#if (_STORAGE_OBJ_== 1)


#define BOOT_SOURCE_SPI                0x00
#define BOOT_SOURCE_SPI_NAND_2K        0x02
#define BOOT_SOURCE_SPI_NAND_4K        0x06
#define BOOT_SOURCE_EMMC_4BIT          0x08
#define BOOT_SOURCE_EMMC_8BIT          0x09

/**
    @name New storage object interface

    For new version storage device driver
*/
//@{
typedef struct {
	INT32(*flash_open)(void);												///< Flash Open
  	 void(*flash_close)(void);												///< Flash Close
	INT32(*flash_readSectors)(UINT32, UINT32, UINT8 *,UINT32);				///< Flash Read Sector
	INT32(*flash_writeSectors)(UINT32, UINT32, UINT8 *, UINT32);			///< Flash Write Sector
	INT32(*flash_writePartition)(UINT32, UINT32, UINT32, UINT8 *, UINT32);	///< Flash Write Sector, and remain space need erase
   UINT32(*flash_getBlockSize)(void);										///< Flash Get Block Size
     void(*flash_setReservedBadBlockNumber)(UINT32);						///< Flash Set Reserved Bad Block Number
	 void(*flash_setReservedAreaMaxBlockNumber)(UINT32);					///< Flash Set Reserved Area Block Number
   UINT64(*flash_getTotalSize)(void);
     void(*flash_installAccessCB)(STRG_ACCESS_CB);
	 void(*flash_installIdentifyCB)(STRG_IDENTIFY_CB);
	 void(*flash_setFrequency)(UINT32);
	INT32(*flash_setConfig)(UINT32, UINT32);
	INT32(*flash_get_spare_data)(UINT32, UINT32 *);
} STORAGE_OBJ, *PSTORAGE_OBJ;

extern PSTORAGE_OBJ emmc_get_storage_object(void);
extern PSTORAGE_OBJ nand_get_storage_object(void);
extern PSTORAGE_OBJ nor_get_storage_object(void);

//@}
#else
#define	FLASH_OPEN(m)						flash_open()
#define	FLASH_CLOSE(m)						flash_close()
#define FLASH_READSECTOR(m, n, o, x, y)		flash_readSectors(n,o,x,y)	///< Flash Read Sector
#define FLASH_WRITESECTOR(m, n, o, x, y)	flash_writeSectors(n,o,x,y)	///< Flash Write Sector
#define FLASH_GETBLKSIZE(m)					flash_getBlockSize()

     void(*flash_setReservedBadBlockNumber)(UINT32);				///< Flash Set Reserved Bad Block Number
	 void(*flash_setReservedAreaMaxBlockNumber)(UINT32);			///< Flash Set Reserved Area Block Number
   UINT64(*flash_getTotalSize)(void);
     void(*flash_installAccessCB)(STRG_ACCESS_CB);
	 void(*flash_installIdentifyCB)(STRG_IDENTIFY_CB);
	 void(*flash_setFrequency)(UINT32);
	INT32(*flash_setConfig)(UINT32, UINT32);
} STORAGE_OBJ, *PSTORAGE_OBJ;

/**
    Flash Open

    This function will initialize internal flash driver

    @note Internal flash is selected by STORAGEINT in MakeConfig.txt

    @return
        - @b E_OK: open success
        - @b E_IDENTIFY_ERR: identify fail (SPI path)
        - @b E_NAND_IDENTIFY_ERR: identify fail (NAND path)
        - @b E_NAND_ID_NOT_SUPPORT: not supported NAND flash
*/
extern INT32    flash_open(void);
/**
    Flash Close

    This function will close internal flash driver

    @note Internal flash is selected by STORAGEINT in MakeConfig.txt

    @return void
*/
extern void     flash_close(void);

/**
    Flash Read Sector

    This function will read data from internal flash

    @note Internal flash is selected by STORAGEINT in MakeConfig.txt

    @param[in] startblk         Start block of this read operation (unit: flash block)
    @param[in] length           Length of read operation (unit: byte)
    @param[out] buffer          Pointer to DRAM buffer to receive read data (word alignment)
    @param[in] updateType       Operation Access Region
                                - @b NAND_RW_LOADER: loader region
                                - @b NAND_RW_RESERVED: reserved area
                                - @b NAND_RW_FIRMWARE: f/w region

    @return
        - @b E_OK: read success
        - @b E_NAND_BLK_NOT_FOUND: can not find available physical block
        - @b E_NAND_READ_BLK_ERR: NAND access block error
        - @b E_NAND_READ_PAGE_ERR: NAND access page error
*/
extern INT32    flash_readSectors(UINT32 startblk, UINT32 length, UINT8 * buffer, UINT32 updateType);

/**
    Flash Write Sector

    This function will write data to internal flash

    @note Internal flash is selected by STORAGEINT in MakeConfig.txt

    @param[in] startblk         Start block of this write operation (unit: flash block)
    @param[in] length           Length of write operation (unit: byte)
    @param[in] buffer           Pointer to DRAM buffer of written data (word alignment)
    @param[in] updateType       Operation Access Region
                                - @b NAND_RW_LOADER: loader region
                                - @b NAND_RW_RESERVED: reserved area
                                - @b NAND_RW_FIRMWARE: f/w region

    @return
        - @b E_OK: read success
        - @b E_NAND_BLK_NOT_FOUND: can not find available physical block
        - @b E_NAND_ERASEE_BLK_ERR: erase block error
        - @b E_NAND_WRITE_PAGEDATA_ERR: page program error
        - @b E_NAND_INSUFFICINET_BLK: insufficient good block
*/
extern INT32    flash_writeSectors(UINT32 startblk, UINT32 length, UINT8 * buffer, UINT32 updateType);

/**
    Flash Get Block Size

    This function will return block size of attached internal flash

    @note Internal flash is selected by STORAGEINT in MakeConfig.txt

    @return block size (unit: byte)
*/
extern UINT32   flash_getBlockSize(void);

/**
    Flash Set Reserved Bad Block Number

    This function will set block count reserved to do bad block replacement

    @note Internal flash is selected by STORAGEINT in MakeConfig.txt

    @param[in] badBlkNum        reserved bad block count (unit: block)

    @return void
*/
extern void     flash_setReservedBadBlockNumber(UINT32 badBlkNum);

//#NT#2010/04/20#Steven Wang -begin
//#NT#Autumn reserved area from 2 ~ 23
/**
    Flash Set Reserved Area Block Number

    This function will set reserved area max block count

    @note Internal flash is selected by STORAGEINT in MakeConfig.txt

    @param[in] RsvMaxBlkNum     max reserved area block count (unit: block)

    @return void
*/
extern void     flash_setReservedAreaMaxBlockNumber(UINT32 RsvMaxBlkNum);
//#NT#2010/04/20#Steven Wang -end

/**
    Flash Get Total Size

    This function will return total size of attached internal flash

    @note Internal flash is selected by STORAGEINT in MakeConfig.txt

    @return total size (unit: byte)
*/
extern UINT64   flash_getTotalSize(void);

/**
    Install Flash Access CallBack

    This function will install access callback.
    The installed callback will be invoked when one block/page is programmed

    @note card is selected by STORAGEEXT in MakeConfig.txt

    @param[in] accessCB     installed callback

    @return void
*/
extern void     flash_installAccessCB(STRG_ACCESS_CB accessCB);

/**
    Install Flash Identify CallBack

    This function will install identification callback.
    The installed callback will be invoked when spi flash driver is opened.

    @note NAND driver will NOT invoke the installed call back

    @param[in] identifyCB   installed callback

    @return void
*/
extern void     flash_installIdentifyCB(STRG_IDENTIFY_CB identifyCB);

/**
    Flash Set Bus Frequency

    This function will set operating frequency of attached internal flash

    @note Internal flash is selected by STORAGEINT in MakeConfig.txt

    @param[in] uiFreq       Operating frequency (unit: MHz)

    @return void
*/
extern void     flash_setFrequency(UINT32 uiFreq);

/**
    Flash Set Configuration

    This function will set specific configuration for internal flash

    @note Internal flash is selected by STORAGEINT in MakeConfig.txt

    @param[in] uiConfigId   Configuration ID, can be:
                            - @b FLASH_CFG_ID_SPI_SUPPORT_4BITS: configure to support 4 bit SPI flash
    @param[in] uiContext    Configuration context for uiConfigId
                            When uiConfigId == FLASH_CFG_ID_SPI_SUPPORT_4BITS:
                            - @b TRUE: use 4 bit read when attached SPI flash support 4 bit read
                            - @b FALSE: won't use 4 bit read

    @return
        - @b E_OK: success
        - @b Else: fail
*/
extern INT32    flash_setConfig(UINT32 uiConfigId, UINT32 uiContext);
#endif
// External storage public API ( SDIO/MS/xD )
/**
    Open card driver

    This function will initialize card driver and register it to file system

    @note card is selected by STORAGEEXT in MakeConfig.txt

    @return
        - @b TRUE: open success
        - @b FALSE: open fail
*/
extern INT32    card_open(void);

/**
    Close card driver

    This function will close card driver

    @note card is selected by STORAGEEXT in MakeConfig.txt

    @return void
*/
extern void     card_close(void);

/**
    Get card type

    This function is used to get card storage type

    @note card is selected by STORAGEEXT in MakeConfig.txt

    @return void
*/

extern UINT32   card_get_type(void);

//@}

extern void  USBOTGReset(void);
//extern INT32 USB3Download(void);
extern INT32 USBDownload(void);
extern void  USBSetResult(UINT32 uiResult);
extern INT32 USBStateMachine(void);
#endif
