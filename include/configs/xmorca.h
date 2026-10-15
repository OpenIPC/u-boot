/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef __XMORCA_H
#define __XMORCA_H

#include <linux/sizes.h>
#include <asm/arch/platform.h>
#include <linux/kconfig.h>

/* ======================== DEBUG begin ==============================*/
#define CP15_NO_INIT_IN_BOOTROM
#define GZIP_ENABLE
/*#define SYSCTRL_STAT_NOINIT_NAND */
/* ======================== DEBUG end ==============================*/

#define CONFIG_SYS_CACHELINE_SIZE   64

/* Open it as you need #define CONFIG_REMAKE_ELF */

#define CONFIG_SUPPORT_RAW_INITRD

#define CONFIG_BOARD_EARLY_INIT_F

/* Physical Memory Map */

/* CONFIG_SYS_TEXT_BASE needs to align with where ATF loads bl33.bin */
#define CONFIG_SYS_TEXT_BASE        0x40800000
#define CONFIG_SYS_TEXT_BASE_ORI        0x40700000

#define SEC_UBOOT_DATA_ADDR     0x41000000

#define PHYS_SDRAM_1            0x40000000
#define PHYS_SDRAM_1_SIZE       0x4000000
/* RAM detection probe size (the GK7206V1 dies carry 64M..256M) */
#define OPENIPC_RAM_MAX_SIZE    0x10000000

#define CONFIG_SYS_SDRAM_BASE       PHYS_SDRAM_1

#define CONFIG_SYS_INIT_SP_ADDR     0x04013000

#define CONFIG_SYS_GBL_DATA_SIZE    128

/* memtest works on */
#define CONFIG_SYS_MEMTEST_START	(CONFIG_SYS_SDRAM_BASE)
#define CONFIG_SYS_MEMTEST_END		(gd->start_addr_sp - 0x100000)
#define CONFIG_SYS_MEMTEST_SCRATCH	(SYS_CTRL_REG_BASE + 0x0c4)

/* Generic Timer Definitions */
#define COUNTER_FREQUENCY       24000000

#define CONFIG_SYS_TIMER_RATE       CFG_TIMER_CLK
#define CONFIG_SYS_TIMER_COUNTER    (CFG_TIMERBASE + REG_TIMER_VALUE)
#define CONFIG_SYS_TIMER_COUNTS_DOWN

/* Generic Interrupt Controller Definitions */
#define GICD_BASE           0xf6801000
#define GICC_BASE           0xf6802000

/* Size of malloc() pool */
#define CONFIG_SYS_NONCACHED_MEMORY SZ_1M

/* PL011 Serial Configuration */
#define CONFIG_PL011_CLOCK      24000000

#define CONFIG_PL01x_PORTS  \
	{(void *)UART0_REG_BASE, (void *)UART1_REG_BASE, \
	(void *)UART2_REG_BASE}

#define CONFIG_CUR_UART_BASE    UART0_REG_BASE

/* Flash Memory Configuration v100 */
#ifdef CONFIG_FMC
#define CONFIG_FMC_REG_BASE       (FMC_REG_BASE + 0x10)
#define CONFIG_FMC_BUFFER_BASE    FMC_MEM_BASE
#define CONFIG_FMC_MAX_CS_NUM     1
#endif

#ifdef CONFIG_FMC_SPI_NOR
#define CONFIG_SPI_NOR_MAX_CHIP_NUM 1
#define CONFIG_SPI_NOR_QUIET_TEST
#endif

#ifdef CONFIG_FMC_SPI_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE  CONFIG_SPI_NAND_MAX_CHIP_NUM
#define CONFIG_SYS_NAND_BASE        FMC_MEM_BASE
#endif

#ifndef CONFIG_SYS_NAND_MAX_CHIPS
#define CONFIG_SYS_NAND_MAX_CHIPS   1
#endif

#define CONFIG_SYS_FAULT_ECHO_LINK_DOWN

/*
*-----------------------------------------------------------------------
* SD/MMC configuration
*-----------------------------------------------------------------------
*/
#define CONFIG_MISC_INIT_R

/* Command line configuration */
#define CONFIG_MENU
/* #define CONFIG_CMD_UNZIP */
#define CONFIG_CMD_ENV

/* BOOTP options */
#define CONFIG_BOOTP_BOOTFILESIZE

/* Initial environment variables */

/*
 * Defines where the kernel and FDT will be put in RAM
 */

/* allow change env */
#define  CONFIG_ENV_OVERWRITE

#define CONFIG_COMMAND_HISTORY

#define CONFIG_ENV_VARS_UBOOT_CONFIG

/* kernel parameter list phy addr */
#define CFG_BOOT_PARAMS         (CONFIG_SYS_SDRAM_BASE + 0x0100)

/* Monitor Command Prompt */
/*
 * SYS_BOOTM_LEN, SYS_LOAD_ADDR, SYS_MALLOC_LEN, SYS_MAXARGS, SYS_PBSIZE,
 * SYS_RX_ETH_BUFFER and CMD_MMC are Kconfig symbols in U-Boot 2022.10;
 * openipc/gk7206v100_config sets them.
 */
#define CONFIG_SYS_CBSIZE       1024    /* Console I/O Buffer Size */
#define CONFIG_SYS_BARGSIZE     CONFIG_SYS_CBSIZE

#define CONFIG_SYS_NO_FLASH

/* Open it as you need #define DDR_SCRAMB_ENABLE */

#define CONFIG_PRODUCTNAME "xmorca"

#define HW_GZIP_HEAD_SIZE		0x10
/* HEAD_MAGIC_NUM* and the size offsets come from openipc-common.h */

/* Open it as you need #define CONFIG_OSD_ENABLE */ /* For VO */
/* Open it as you need #define CONFIG_CIPHER_ENABLE */

/* Open it as you need #define CONFIG_AUDIO_ENABLE */

/* Open it as you need #definee CONFIG_EDMA_PLL_TRAINNING */

#define SECUREBOOT_OTP_REG_BASE_ADDR_PHY            (0x10090000)

/******* Head area ***************/
/*  |   relative base:0x0
 *  |		root rsa pub key n (512 bytes)
 *  |   	root rsa pub key e (512 bytes)
 *  |       		508 bytes fill 0
 *  |	    		4 bytes: value of e
 *  |   relative base:0x400
 *  |   	params area (0x1bc0)
 *  |			val of auxcode_area_len (4 bytes)
 *  |                   val of boot_code_area_len (4 bytes)
 *  |                   val of total_boot_area_len (4 bytes)
 *  |                   val of auxcode_enc_flag  (4 bytes)
 *  |                   val of boot_enc_flag  (4 bytes)
 *  |			val of boot_entry_piont (4 bytes)
 *  |			val of auxcode_aes_iv (16 bytes)
 *  |			val of boot_aes_iv (16 bytes)
 *  |			config_param (6144 bytes)
 *  |			reserved_0 (392 bytes)
 *  |			sig_params (512 bytes)
 *
 */
/* key area    */
#define CONFIG_KEY_AREA_POS	0
#define CONFIG_KEY_AREA_LEN	0x2800
#define CONFIG_ROOT_RSA_PUB_KEY_N_POS	CONFIG_KEY_AREA_POS
#define CONFIG_ROOT_RSA_PUB_KEY_N_LEN	512
#define CONFIG_ROOT_RSA_PUB_KEY_E_POS   (CONFIG_KEY_AREA_POS + CONFIG_ROOT_RSA_PUB_KEY_N_LEN)
#define CONFIG_ROOT_RSA_PUB_KEY_E_ZERO_LEN   508
#define CONFIG_ROOT_RSA_PUB_KEY_E_V	0x0
/* params area    */
#define CONFIG_PARAMS_AREA_POS     0x400 /* fixed, and equal to (CONFIG_KEY_AREA_POS + CONFIG_ROOT_RSA_PUB_KEY_N_LEN + CONFIG_ROOT_RSA_PUB_KEY_E_LEN) */
#define CONFIG_PARAMS_AREA_LEN     0x23C0 /* fixed and not be changed   */
#define CONFIG_AUXCODE_AREA_LEN_POS	   (CONFIG_PARAMS_AREA_POS)
#define CONFIG_BOOT_CODE_AREA_LEN_POS      (CONFIG_AUXCODE_AREA_LEN_POS + 0x4)
#define CONFIG_TOTAL_BOOT_AREA_LEN_POS	   (CONFIG_BOOT_CODE_AREA_LEN_POS + 0x4)
#define CONFIG_AUXCODE_ENC_FLAG_POS	   (CONFIG_TOTAL_BOOT_AREA_LEN_POS + 0x4)
#define CONFIG_BOOT_ENC_FLAG_POS	   (CONFIG_AUXCODE_ENC_FLAG_POS + 0x4)
#define CONFIG_BOOT_ENTRY_PIONT_POS	   (CONFIG_BOOT_ENC_FLAG_POS + 0x4)
#define CONFIG_AUXCODE_AES_IV_POS	   (CONFIG_BOOT_ENTRY_PIONT_POS + 0x4)
#define CONFIG_BOOT_AES_IV_POS		   (CONFIG_AUXCODE_AES_IV_POS + 0x10)
#define CONFIG_PARAMS_FORM_POS		   (CONFIG_BOOT_AES_IV_POS + 0x10)
#define CONFIG_PARAMS_FORM_LEN		   (0x2000) /* fixed   */
#define CONFIG_PARAMS_FORM_LEN_POS         (CONFIG_TOTAL_BOOT_AREA_LEN_POS + 0x4)
#define CONFIG_LOAD_MCU_FW_FLAG_POS        (CONFIG_PARAMS_FORM_POS + CONFIG_PARAMS_FORM_LEN)
#define CONFIG_MCU_FW_AREA_LEN_POS         (CONFIG_LOAD_MCU_FW_FLAG_POS + 0x4)
#define CONFIG_MCU_FW_BUF_ADDR_POS         (CONFIG_MCU_FW_AREA_LEN_POS + 0x4)
#define CONFIG_DDR_PARAM_OFFSET_POS	   (CONFIG_MCU_FW_BUF_ADDR_POS + 0x4)
#define CONFIG_DDR_PARAM_SIZE_POS	   (CONFIG_DDR_PARAM_OFFSET_POS + 0x4)
#define CONFIG_RESERVED0_POS               (CONFIG_DDR_PARAM_SIZE_POS + 0x4)

/* Auxcode-2048 mode, as the SDK's first stage always builds it */
#define CONFIG_AUXCODE_2048_FLAG           0x20485AA5

/* No RISC-V MCU firmware is carried */
#define CONFIG_LOAD_MCU_FW_FLAG            0
#define CONFIG_MCU_FW_AREA_LEN             0

#define CONFIG_RESERVED0_LEN               (372)

#define CONFIG_SIG_PARAMS_POS		   (CONFIG_RESERVED0_POS + CONFIG_RESERVED0_LEN)
#define CONFIG_SIG_PARAMS_LEN		   (0x200)

/* auxcode.bin in arch/arm/cpu/armv7/xmorca/hw_compressed is 12K */
#define CONFIG_AUXAREA_LEN              0x3000

#define CONFIG_AUXCODE_ENC_FLAG         0x12345678  /* crypto:0x2A13C812  nocrypto:other val   */
#define CONFIG_BOOT_ENC_FLAG		0x12345678  /* crypto:0x2A13C812  nocrypto:other val  */
#define CONFIG_BOOT_ENTRY_PIONT         (CONFIG_SYS_TEXT_BASE_ORI + CONFIG_KEY_AREA_LEN + CONFIG_AUXAREA_LEN + CONFIG_MCU_FW_AREA_LEN)

/* unchecked area   */
#define CONFIG_SCS_SIMULATE_FLAG_POS	(CONFIG_PARAMS_AREA_POS + CONFIG_PARAMS_AREA_LEN)
#define CONFIG_BOOT_STORE_ADDR_POS	(CONFIG_SCS_SIMULATE_FLAG_POS + 0x4)
#define CONFIG_FLASH_EMPYT_FLAG_POS	(CONFIG_BOOT_STORE_ADDR_POS + 0x4)
#define CONFIG_AUXCODE_LOG_CTRL_POS	(CONFIG_FLASH_EMPYT_FLAG_POS + 0x4)
#define CONFIG_AUXCODE_SVB_CTRL_POS	(CONFIG_AUXCODE_LOG_CTRL_POS + 0x4)
#define CONFIG_RESERVED1_POS		(CONFIG_AUXCODE_SVB_CTRL_POS + 0x4)
#define CONFIG_RESERVED1_LEN		(0x2C)

/*
 * Values the boot header hands the auxcode (Kconfig options in the SDK):
 * log level 1 = as the bootrom OTP says; SVB (adaptive voltage) on; and
 * where the SDK's ddr_param partition sits, read on resume from suspend.
 */
#define CONFIG_AUXCODE_LOG_CTRL		1
#define CONFIG_AUXCODE_SVB_CTRL		0
#define CONFIG_DDR_PARAM_OFFSET		0xC0000
#define CONFIG_DDR_PARAM_SIZE		0x80000

#define CONFIG_SCS_SIMULATE_FLAG	0x69875AB4 /* no verify   */
#define CONFIG_FLASH_EMPYT_FLAG		0x435A590D /* not empty  */
#define CONFIG_BOOT_STORE_ADDR		0x40700000 /* same with burn tool  */

/* auxcode area   */
#define CONFIG_IMAGE_INFO_POS		(CONFIG_KEY_AREA_POS + CONFIG_KEY_AREA_LEN)
#define CONFIG_AUX_IDENTITY_INFO_POS	(CONFIG_IMAGE_INFO_POS + 0x20)
#define CONFIG_AUX_CODE_OFFSET_POS	(CONFIG_AUX_IDENTITY_INFO_POS + 0x10)
#define CONFIG_AUX_CODE_OFFSET		(0x40)
#define CONFIG_AUX_CODE_LEN_POS		(CONFIG_AUX_CODE_OFFSET_POS + 0x4)
#define CONFIG_RESERVED2_POS		(CONFIG_AUX_CODE_LEN_POS + 0x4)
#define CONFIG_AUXILIARY_CODE_POS	(CONFIG_RESERVED2_POS + 0x8)
#define CONFIE_AUXILIART_CODE_LEN	(CONFIG_AUXAREA_LEN - 0x240)
#define CONFIG_SIG_AUXCODE_POS		(CONFIG_AUXILIARY_CODE_POS + CONFIE_AUXILIART_CODE_LEN)
#define CONFIG_SIG_AUXCODE_LEN		(0x200)

/* mcu fw area */
#define CONFIG_MCU_FW_AREA_POS		(CONFIG_KEY_AREA_POS + CONFIG_KEY_AREA_LEN + CONFIG_AUXAREA_LEN)

/* boot code area */
#define CONFIG_BOOT_CODE_AREA_POS	(CONFIG_KEY_AREA_POS + CONFIG_KEY_AREA_LEN + CONFIG_AUXAREA_LEN + CONFIG_MCU_FW_AREA_LEN)

#define CONFIG_BOOT_HEAD_SIZE		(CONFIG_KEY_AREA_LEN + CONFIG_AUXAREA_LEN + CONFIG_MCU_FW_AREA_LEN)

/* Magic code for boot code area */
#define CONFIG_BOOT_CODE_AREA_MAGIC      0x96A5784B

#define MCU_FW_ALIGNED_SIZE (0x2000) /* 8K */

#define CONFIG_MCU_FW_BUF_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x48000)
#define CONFIG_MCU_FW_BUF_SIZE		(CONFIG_MCU_FW_AREA_LEN + 2 * MCU_FW_ALIGNED_SIZE)

#define CONFIG_DTB_BASE			(CONFIG_MCU_FW_BUF_ADDR + CONFIG_MCU_FW_BUF_SIZE)

#define CONFIG_DTB_MAX_SIZE             (0x20000)
/*
 * openipc-common.h sizes the malloc pool itself for NAND; drop Kconfig's
 * value first rather than redefine it.
 */
#ifdef CONFIG_FMC_SPI_NAND
#undef CONFIG_SYS_MALLOC_LEN
#endif
#include "openipc-common.h"

#endif /* __XMORCA_H */
