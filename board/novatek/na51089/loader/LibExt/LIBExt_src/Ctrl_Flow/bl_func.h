/**
    @file       bl_func.h
    @brief      Header file for main function

    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.
*/
#ifndef _BL_MAIN_H
#define _BL_MAIN_H

#include "main.h"
#include "IOReg.h"
#include "GPIO.h"
#include "nvtpack.h"
#include "dram_partition_info.h"
#include "loader.h"
//------------------------------------------------------------------------------
// Compiler option
//
// These compiler option are define in MakeConfig.txt
//!!!!!!!!!!!!!!!!!!! DO NOT MODIFY COMPILER OPTION HERE !!!!!!!!!!!!!!!!!!!!
//------------------------------------------------------------------------------
#ifndef SECURE_DECRYPT_UBOOT
#define SECURE_DECRYPT_UBOOT      0    	//Decrypt uboot sample code
#endif									//Option move to ModelConfig_EMU_EVB_XXX.txt

#ifndef SECURE_DECRYPT_OPTEE_OS
#define SECURE_DECRYPT_OPTEE_OS   0    	//Decrypt optee_os sample code
#endif								   	//Option move to ModelConfig_EMU_EVB_XXX.txt
#ifndef  SECURE_SIGNATURE_BY_AES
#define SECURE_SIGNATURE_BY_AES   0    	//using aes to decrypt signature
#endif									//Option move to ModelConfig_EMU_EVB_XXX.txt


#if (SECURE_DECRYPT_UBOOT || SECURE_DECRYPT_OPTEE_OS)
#define SECURE_OFFSET			1024		//Decrypt uboot from which offset
#endif

// Define LED_FUNCTION == ENABLE will enable LED on/off in boot loader
#define LED_FUNCTION                ENABLE//DISABLE

// FW checking method, support checksum & CRC now
//#define FW_CHECK_NOCHECK            0
//#define FW_CHECK_CHECKSUM           1
//#define FW_CHECK_CRC                2
#ifndef FW_CHECK_METHOD
#define FW_CHECK_METHOD             FW_CHECK_CHECKSUM
#endif

///#define FW_CHECK_INSTRUCTION0       0xE59FF018

// Optional pseudo string for checksum & CRC checking
#define FW_CHECK_PSEUDO_STR_EN      DISABLE
#if (FW_CHECK_PSEUDO_STR_EN == ENABLE)
    // Must sync to MakeConfig.txt
    #define FW_CHECK_PSEUDO_STR     "DSC_FW"
#endif

// Determine if need to support compressable main code
#ifndef _MAINCODE_COMPRESS_
#define _MAINCODE_COMPRESS_ 0
#endif

#ifndef STORAGEINT_SPI
#define STORAGEINT_SPI      0
#endif

#define FW_EXCEED_16MB_TEST         DISABLE

//------------------------------------------------------------------------------
// Global definition
//------------------------------------------------------------------------------
#define LOADER_CODE_SIZE            0x8000      // Reserve 32 KB

#define RESERVED_DATA_SIZE          0x4000      // Fixed to 16 KB

//Once if optee exist need modify to 0x02800000
//#define SDRAM_Start_FW              0x02800000  // DRAM uITRON starting address
#define SDRAM_Start_FW              0x02000000
//#endif

#define SRAM_Start_FW               0xF07c0000  // SRAM FW starting address

#define SRAM_TAG                    0xF0000000  // SRAM tag

#define CORE2_JUMP_ADDR             0x00000000  // Core2 entry code should put on 0, because when core2 reset is released, core2 will jump to 0 and run

#define CPU_TIMER_SETTING           0x00B71B00 //0x016E3600

#define _BOARD_IPC_ADDR_            0x00002000 // must be the same with BOARD_IPC_ADDR on ModelConfig.


#define UPDATE_CFG_BUFADDR          0x05000000
#define UPDATE_CFG_TAGCODE          0xABCD0043
#define UPDATE_CFG_LOADER_OFS       0x00
#define UPDATE_CFG_FW_OFS           0x10

#define UPDATE_CFG_ITEM_LD          0xABCD0001
#define UPDATE_CFG_ITEM_FW          0xABCD0002

#define UPDATE_CFG_TAG              0
#define UPDATE_CFG_ITEM             1
#define UPDATE_CFG_ADDR             2
#define UPDATE_CFG_LEN              3

#define USB_RESULT_LOADER_INVALID_FW_INVALID        (0)
#define USB_RESULT_LOADER_INVALID_FW_OK             (1)
#define USB_RESULT_LOADER_INVALIE_FW_FAIL           (2)
#define USB_RESULT_LOADER_OK_FW_INVALID             (3)
#define USB_RESULT_LOADER_FAIL_FW_INVALID           (4)
#define USB_RESULT_LOADER_OK_FW_OK                  (5)
#define USB_RESULT_LOADER_FAIL_FW_OK                (6)
#define USB_RESULT_LOADER_OK_FW_FAIL                (7)
#define USB_RESULT_LOADER_FAIL_FW_FAIL              (8)
#define USB_RESULT_OTHER                            (9)

// Loader checking
#define LOADER_TAG_OFFSET           0x30
#define LOADER_TAG_VALUE            0xAA55

//#NT#2011/06/30#Steven Wang -begin
//#NT#Checksum offset
///#define CHECKSUM_OFFSET             0x1C
#define CHECKSUM_TAG                0xAA55
//#NT#2011/06/30#Steven Wang -end

// Config table tag value (for USB flow)
#define CFG_TABLE_TAG               (0x4B54564E)        // 'N' 'V' 'T' 'K'

#define COMPRESS_TAG_VALUE          (0x314C4342)        // use to distinguish full compressed main code

#define SPI_FLASH_IDENTIFY          (0x46495053)        //'S''P''I''F'

#define BOOTVER_SET(OutMaj,OutMin,InnMaj,InnMin)  (((OutMaj)<<24)|((OutMin)<<16)|((InnMaj)<<8)|((InnMin)))

#define BOOTVER_GETOuterMajor(Ver)  (((Ver)&0xFF000000)>>24)
#define BOOTVER_GETOuterMinor(Ver)  (((Ver)&0x00FF0000)>>16)
#define BOOTVER_GETInnerMajor(Ver)  (((Ver)&0x0000FF00)>>8)
#define BOOTVER_GETInnerMinor(Ver)  (((Ver)&0x000000FF))

// Define loader version (_LDR_VER_) in MakeConfig.txt
#define BOOTLOADER_OUTERMAJOR       (((_LDR_VER_)&0xFF000000)>>24)
#define BOOTLOADER_OUTERMINOR       (((_LDR_VER_)&0x00FF0000)>>16)
#define BOOTLOADER_INNERMAJOR       (((_LDR_VER_)&0x0000FF00)>>8)
#define BOOTLOADER_INNERMINOR       (((_LDR_VER_)&0x000000FF))

// Binary tag value
#define FW_BINARY_TAG               0xAA55544E  // 'N' 'T' 0x55 0xAA

// CAL FW size exit tag
#define CAL_SIZE_TAG                0x5A534C43  // CLSZ
// FW size exit tag
#define FW_SIZE_TAG                 0x5A535746  // FWSZ

// Reserved area FW size offset
#define RESERVED_AREA_FW_SIZE_OFFSET    0x00
// Reserved area CAL size offset
#define RESERVED_AREA_CAL_SIZE_OFFSET   0x04
// Reserved area FW size tag offset
#define RESERVED_AREA_FW_TAG_OFFSET     0x08
// Reserved area CAL size tag offset
#define RESERVED_AREA_CAL_TAG_OFFSET    0x0C

// Operation of boot loader
#define FUNC_UPDATE_FW              0x01        // Read FW code from card and update to NAND and run
#define FUNC_UPDATE_LOADER          0x02        // Read boot loader code from card and update to NAND
#define FUNC_RUN_CARD               0x04        // Read FW code from card and run
#define FUNC_RUN_FLASH              0x08        // Read FW code from internal flash and run
#define FUNC_UPDATE_CAL             0X10        // Update CAL data
#define FUNC_RUN_WRBIN              0X20        // RUN wrkey data
#define FUNC_RUN_UBOOT              0X40        // Read Uboot from card and run
#define FUNC_UPDATE_FW_DONE         0x0100      // update f/w to NAND/SPI done

// API to turn on / off LED
#if (LED_FUNCTION == ENABLE)
// Red LED (DGPIO 6)
// Green LED (DGPIO 9)
#define TurnOff_RedLED()            OUTREG32(CPE_GPIO_BASE + 0x4C, 0x00000040)
#define TurnOn_RedLED()             OUTREG32(CPE_GPIO_BASE + 0x38, 0x00000040)
#define TurnOff_GreenLED()          OUTREG32(CPE_GPIO_BASE + 0x4C, 0x00000200)
#define TurnOn_GreenLED()           OUTREG32(CPE_GPIO_BASE + 0x38, 0x00000200)
#else
#define TurnOff_RedLED()
#define TurnOn_RedLED()
#define TurnOff_GreenLED()
#define TurnOn_GreenLED()
#endif

//Configure build as USB write loader @ ModelConfig_EMU_EVB.txt
//UART & USB can enable at the same time.
#if (STORAGE_EXT_TYPE == STORAGE_EXT_USB) || (STORAGE_EXT_TYPE == STORAGE_EXT_UART)
#define USB_WRITELOADER                 ENABLE
#define UART_UPDATE                     ENABLE
#else
#define USB_WRITELOADER                 DISABLE
#define UART_UPDATE                     DISABLE
#endif
#define UART_UPDATE_                    DISABLE
#define UPDATE_EMU_CODE					DISABLE
#define REMOVED_FLASH                   DISABLE
#define NUTTX_SUPPORT                   DISABLE
#define UPDATE_SIM_CODE					DISABLE
#define UITRON_FW						DISABLE
#define FDT_SUPPORT                     ENABLE
#define DUAL_RTOS_SUPPORT               DISABLE   // only for rtos-uvc project
#define STAND_ALONE_LOADER_560			0

#define LOADER_TYPE                     STAND_ALONE_LOADER_560

#define COMBINATION_LOADER_SIZE			0x18000

#define EMUKIT							0
#define DEMOKIT 						0x1800000
#define RESUME_ADDR						DEMOKIT

//#NT#2018/03/07#Ben Wang -begin
//#NT#Support EMMC boot flow
#define EMMC_BLOCK_SIZE		0x200
#define FDT_OFFSET		0x40000
//#NT#2018/03/07#Ben Wang -end

#if (STORAGEINT_SPI == 1)
    #define StartNandBlkUpdateLoader    0
    #define StartNandBlkReserved        0
    //#NT#2012/12/17#HH Chuang -begin
    //#NT#Sync spi erase unit to best access size of f/w storage object
    #define StartNandBlkUpdateFW        16
    //#NT#2012/12/17#HH Chuang -end
#else
    #define StartNandBlkUpdateLoader    0
    #define StartNandBlkUpdateFW        2
#endif

#if (FW_CHECK_METHOD == FW_CHECK_CRC)
static UINT32 g_uiCRCTable[256];
#endif

//Define @prj_main.h
#if 0
// Card detect GPIO offset
#define DGPIO_CARD_DETECT           (P_GPIO_17)


#define CHECK_CARD_EXIST            (ENABLE)
#if (CHECK_CARD_EXIST == ENABLE)
// MC16 default is config as input and pull up
#define bl_checkCardExist()         (gpio_getPin(DGPIO_CARD_DETECT) == 0 ? TRUE : FALSE)
#else
#define bl_checkCardExist()         TRUE
#endif
#endif
/**
    @name Assert macros
*/
//@{
#define ASSERT_CONCAT_(a, b)    a##b
#define ASSERT_CONCAT(a, b)     ASSERT_CONCAT_(a, b)

#if defined(__COUNTER__)

#define STATIC_ASSERT(expr) \
enum { ASSERT_CONCAT(FAILED_STATIC_ASSERT_, __COUNTER__) = 1/(expr) }

#else

// This might cause compile error when writing STATIC_ASSERT at the same line
// in two (or more) files and one file include another one.
#define STATIC_ASSERT(expr) \
enum { ASSERT_CONCAT(FAILED_STATIC_ASSERT_, __LINE__) = 1/(expr) }

#endif
//@}

//#NT#2010/05/21#Steven Wang -begin
//#NT#Check if FW size > EXEC base address
//extern UINT32 _load_BOOT_EXEC_start_base;
//#NT#2010/05/21#Steven Wang -end

extern char _load_LOADER_CONFIGRAM_FREQ_PARAM_start_base[];

extern UINT32 LoaderInternalInfo[];

extern PSTORAGE_OBJ int_strg_obj;

//------------------------------------------------------------------------------
// internal functions
//------------------------------------------------------------------------------
extern int bl_chk_valid_all_in_one(NVTPACK_MEM *p_mem_all_in_one);
extern int bl_chk_uboot(unsigned int addr, unsigned int size);
extern unsigned int bl_process_all_in_one(UINT32 uiFwBuf, UINT32 uiFwBufSize, DRAM_PARTITION **pOut_dram_partition, UINT32 uiLoaderFunc, UINT32 *p_comp_addr, UINT32 *p_comp_size);
extern int bl_boot_uboot(unsigned char *p_fdt);
extern int bl_chk_fdt(unsigned int addr, unsigned int size);
extern int bl_copy_fdt_to_fdt_addr(unsigned char *p_fdt /*IN*/, unsigned char **pp_fdt /*OUT*/);
extern int bl_flash_open(void);
// Exported functions
//------------------------------------------------------------------------------
extern UINT32 bl_mainFlow(void);
extern UINT32 BaseOfStack;

// project(customer) main function
extern void prj_main(void);
extern void bl_read_rtos_addr(UINT32* pLoadAddr, UINT32* pTargetAddr, UINT32* pSize);
#endif
