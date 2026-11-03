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

#ifndef SECURE_DECRYPT_BL31
#define SECURE_DECRYPT_BL31       0     //Decrypt bl31 sample code
#endif                                  //Option move to ModelConfig_EMU_EVB_XXX.txt

#ifndef  SECURE_SIGNATURE_BY_AES
#define SECURE_SIGNATURE_BY_AES   0    	//using aes to decrypt signature
#endif									//Option move to ModelConfig_EMU_EVB_XXX.txt


#if (SECURE_DECRYPT_UBOOT || SECURE_DECRYPT_OPTEE_OS || SECURE_DECRYPT_BL31)
#define SECURE_OFFSET           1024        //Decrypt uboot from which offset
#endif

#if (_FPGA_EMULATION_ == 1)
#define IS_SECURE() (SECURE_DECRYPT_UBOOT || SECURE_DECRYPT_OPTEE_OS || SECURE_DECRYPT_BL31)
#define PCIE_SUPPORT DISABLE
#define AT80C51_SUPPORT DISABLE
#else
#define IS_SECURE() is_secure_enable()
#define PCIE_SUPPORT DISABLE
#define AT80C51_SUPPORT ENABLE
#endif

#if (PCIE_SUPPORT)
#define IS_RC()  utl_is_boot_as_rc()
#else
#define IS_RC()  (1)
#endif

#if (AT80C51_SUPPORT)
#define IS_AT80C51_OPENED()  at80c51_is_opened()
#else
#define IS_AT80C51_OPENED()  (0)
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
#define SDRAM_Start_FW              0x02800000  // DRAM uITRON starting address
//#define SDRAM_Start_FW              0x02000000
//#endif

#define SRAM_Start_FW               0xF07c0000  // SRAM FW starting address

#define SRAM_TAG                    0xF0000000  // SRAM tag

#define CORE2_JUMP_ADDR             0x00000000  // Core2 entry code should put on 0, because when core2 reset is released, core2 will jump to 0 and run

#define CPU_TIMER_FREQ              3000000 // 3MHz

// Loader checking
#define LOADER_TAG_OFFSET           0x30
#define LOADER_TAG_VALUE            0xAA55
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

// spi-nand, spi-nor DTR tunning pattern
#define DTR_TUNNING_PATTERN_DATA1  0x0F0F0F0F //4bit
#define DTR_TUNNING_PATTERN_DATA2  0x00FF00FF //8bit
#define DTR_TUNNING_PATTERN_SIZE  0x1000

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
#define UPDATE_SIM_CODE					DISABLE
#define UITRON_FW						DISABLE
#define CRYPTO_ENGINE_SAMPLE            DISABLE
#define RUN_UITRON_FW                   DISABLE  // for FPGA, boot rtos bypass u-boot
#define FDT_SUPPORT                     ENABLE
#define DUAL_RTOS_SUPPORT               DISABLE  // only for rtos-uvc project (no longer support)
#define MULTI_CODE_FLOW                 ENABLE
//#define _FLASH_FREQ_MHZ_              120      // uncomment it to set flash frequency
#define DUMP_FLOW_TIMESTAMP             DISABLE  // shows flow timestamp for debugging fastboot

#define NVT_CC_CORE2_START              CC_CPU2_CPU1_CMDBUF_REG2
#define CC_CPU1_WARM_RESET_REG2         CC_CORE2_WARM_RST_REG       //Core1 warm reset entry
#define AARCH64_MODE                    DISABLE

#define DDR_SCAN_REDUCE_CODE              DISABLE

#define EMUKIT							0
#define DEMOKIT 						0x1800000
#define RESUME_ADDR						DEMOKIT

//#NT#2018/03/07#Ben Wang -begin
//#NT#Support EMMC boot flow
#define EMMC_BLOCK_SIZE		0x200
//#NT#2018/03/07#Ben Wang -end

/**
 * @brief FdtFast Flash Offset
 */
#if defined(_STORAGEINT_NandSpi_) \
    || defined(_STORAGEINT_NandEccBCH_) \
    || defined(_STORAGEINT_EMMC_) \
    || defined(_STORAGEINT_EMMC_NAND_COMBO_)
#define FDT_OFFSET         0x40000
#define FDTFAST_OFFSET     0x80000
#elif defined(_STORAGEINT_SpiFlash_)
#define FDT_OFFSET         0x10000
#define FDTFAST_OFFSET     0x50000
#elif defined(_STORAGEINT_NONE_)
#define FDT_OFFSET         0x40000
#define FDTFAST_OFFSET     0x80000
#endif


#if defined(_DIRECT_BOOT_)
#define BOOT_LINUX_SUPPORT ENABLE
#else
#define BOOT_LINUX_SUPPORT DISABLE
#endif

#if defined(_FASTBOOT_)
#define FDTFAST_SENSOR    ENABLE
#else
#define FDTFAST_SENSOR    DISABLE
#endif

#if (BOOT_LINUX_SUPPORT) && (!FDT_SUPPORT)
/**
 * because boot linux need fdt to pass chosen,
 * ramdisk and other parameters to kernel,
 */
#error "BOOT_LINUX_SUPPORT need FDT_SUPPORT"
#endif

/**
 * if nvt-fastboot.bin is too large (> 128KB),
 * nvt-fastboot.bin cannot be loaded to SRAM
 * for this case, nvt-fastboot.bin only can be loaded to DRAM
 * it will cause fastboot speed has a little down.
 */
#define FDTFAST_FROM_DRAM ENABLE

#define StartNandBlkUpdateLoader    0

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
extern void bl_read_rtos_addr(UINT32 *pLoadAddr, UINT32 *pTargetAddr, UINT32 *pSize);
extern void *bl_get_kernel_partition(unsigned char *p_fdt /*IN*/, int *p_id /*OUT*/);
extern void *bl_get_rootfs_partition(unsigned char *p_fdt /*IN*/, int *p_id /*OUT*/);

// decrypt
int do_decrypt_aes(unsigned int input_data, unsigned int output_data, unsigned int *output_size, unsigned int partition_size);

#endif
