/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#ifndef __HI3519DV500_H
#define __HI3519DV500_H

#include <asm/arch/platform.h>
#include <linux/sizes.h>

#define CONFIG_REMAKE_ELF

#define CONFIG_SUPPORT_RAW_INITRD

#define CONFIG_BOARD_EARLY_INIT_F

#define CONFIG_TFTP_PORT

/* Physical Memory Map */

/* CONFIG_SYS_TEXT_BASE needs to align with where ATF loads bl33.bin */
#define CONFIG_SYS_TEXT_BASE		0x48800000
#define CONFIG_SYS_TEXT_BASE_ORI	0x48700000


#define PHYS_SDRAM_1			0x40000000
#define PHYS_SDRAM_1_SIZE		0x20000000

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1

#define CONFIG_SYS_INIT_SP_ADDR		0x48700000

#define CONFIG_SYS_GBL_DATA_SIZE    128

/* Generic Timer Definitions */
#define COUNTER_FREQUENCY		24000000

#define CONFIG_SYS_TIMER_RATE		CFG_TIMER_CLK
#define CONFIG_SYS_TIMER_COUNTER	(CFG_TIMERBASE + REG_TIMER_VALUE)
#define CONFIG_SYS_TIMER_COUNTS_DOWN


/* Generic Interrupt Controller Definitions */

/* Size of malloc() pool */

/* PL011 Serial Configuration */

#define CONFIG_PL011_CLOCK		24000000

#define CONFIG_PL01x_PORTS  \
{(void *)UART0_REG_BASE, (void *)UART1_REG_BASE, \
	(void *)UART2_REG_BASE, (void *)UART3_REG_BASE}

#define CONFIG_CUR_UART_BASE	UART0_REG_BASE

#define CONFIG_64BIT

/* Network Configuration */

#define CONFIG_PHY_GIGE
#ifdef CONFIG_GMACV300_ETH
#define CONFIG_GMAC_NUMS        1
#define CONFIG_GMAC_PHY0_ADDR     1
#define CONFIG_GMAC_PHY0_INTERFACE_MODE	2 /* rgmii 2, rmii 1, mii 0 */
#define CONFIG_GMAC_PHY1_ADDR     3
#define CONFIG_GMAC_PHY1_INTERFACE_MODE	2 /* rgmii 2, rmii 1, mii 0 */
#define CONFIG_GMAC_DESC_4_WORD
#define CONFIG_SYS_FAULT_ECHO_LINK_DOWN 1
#endif

/* Flash Memory Configuration v100 */
#ifdef CONFIG_FMC
#define CONFIG_FMC_REG_BASE		FMC_REG_BASE
#define CONFIG_FMC_BUFFER_BASE	FMC_MEM_BASE
#define CONFIG_FMC_MAX_CS_NUM		1
#endif

#ifdef CONFIG_FMC_SPI_NOR
#define CONFIG_CMD_SF
#define CONFIG_SPI_NOR_MAX_CHIP_NUM	1
#define CONFIG_SPI_NOR_QUIET_TEST
#endif

#ifdef CONFIG_FMC_SPI_NAND
#define CONFIG_CMD_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE	CONFIG_SPI_NAND_MAX_CHIP_NUM
#define CONFIG_SYS_NAND_BASE		FMC_MEM_BASE
#endif

#ifdef CONFIG_FMC_NAND
#define CONFIG_CMD_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE  CONFIG_NAND_MAX_CHIP_NUM
#define CONFIG_SYS_NAND_BASE        FMC_MEM_BASE
#endif

/*---------------------------------------------------------------------
 * sdcard system updae
e* ---------------------------------------------------------------------*/

#if (CONFIG_AUTO_SD_UPDATE == 1)
#ifndef CONFIG_MMC
#define CONFIG_MMC      1
#define CONFIG_MMC_WRITE  1
#define CONFIG_MMC_QUIRKS  1
#define CONFIG_MMC_HW_PARTITIONING  1
#define CONFIG_MMC_HS400_ES_SUPPORT  1
#define CONFIG_MMC_HS400_SUPPORT  1
#define CONFIG_MMC_HS200_SUPPORT  1
#define CONFIG_MMC_VERBOSE  1
#define CONFIG_MMC_SDHCI  1
#define CONFIG_MMC_SDHCI_ADMA  1
#define CONFIG_MMC_SDHCI_ADMA_HELPERS 1
#endif
#endif

/* SD/MMC configuration */

#ifdef CONFIG_MMC
#define CONFIG_EMMC
#define CONFIG_SUPPORT_EMMC_BOOT
#define CONFIG_GENERIC_MMC
#define CONFIG_CMD_MMC
#define CONFIG_SYS_MMC_ENV_DEV	0
#define CONFIG_EXT4_SPARSE
#define CONFIG_SDHCI
#define CONFIG_BSP_SDHCI
#define CONFIG_BSP_SDHCI_MAX_FREQ  150000000
#define CONFIG_FS_EXT4
#define CONFIG_SDHCI_ADMA
#define CONFIG_SUPPORT_EMMC_RPMB
#define CONFIG_MMC_QUICKBOOT
#endif

#if (CONFIG_AUTO_USB_UPDATE == 1)
#ifndef CONFIG_USB_HOST
#define CONFIG_USB_HOST 1
#define CONFIG_USB_XHCI_HCD 1
#define CONFIG_USB_STORAGE 1
#define CONFIG_CMD_USB 1
#endif
#endif

#if defined(CONFIG_FMC) || defined(CONFIG_FMC_NAND)
#undef CONFIG_EMMC
#endif


#define CONFIG_MISC_INIT_R

/* Command line configuration */
#define CONFIG_MENU
/* Open it as you need  #define CONFIG_CMD_UNZIP */
#define CONFIG_CMD_ENV

/* BOOTP options */
#define CONFIG_BOOTP_BOOTFILESIZE


/* Initial environment variables */

/*
 * Defines where the kernel and FDT will be put in RAM
 */

/* Assume we boot with root on the seventh partition of eMMC */
#define CONFIG_BOOTCOMMAND "bootm 0x42000000"
#define CONFIG_SYS_USB_XHCI_MAX_ROOT_PORTS 2
#define CONFIG_USB_MAX_CONTROLLER_COUNT 1
#define BOOT_TARGET_DEVICES(func) \
	func(USB, usb, 0) \
func(MMC, mmc, 1) \
func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>

/* allow change env */
#define  CONFIG_ENV_OVERWRITE

#define CONFIG_COMMAND_HISTORY

/* env in flash instead of CFG_ENV_IS_NOWHERE */

#define CONFIG_ENV_VARS_UBOOT_CONFIG

/* kernel parameter list phy addr */
#define CFG_BOOT_PARAMS			(CONFIG_SYS_SDRAM_BASE + 0x0100)

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_SYS_MAXARGS		64	/* max command args */

#define CONFIG_SYS_NO_FLASH

#define TEGRA_SMC_GSL_FUNCID  0xc2fffa00


#define CONFIG_DDR_TRAINING_V2
/* Open it as you need #define CONFIG_I2C_BSP */

/* Osd enable */
#define CONFIG_OSD_ENABLE
/* Open it as you need  #define CONFIG_CIPHER_ENABLE */
/* Open it as you need  #define CONFIG_KLAD_ENABLE */
/* Open it as you need  #define CONFIG_OTP_ENABLE */
#define CONFIG_PRODUCTNAME "hi3519dv500"

/* Open it as you need #define CONFIG_PCI_CONFIG_HOST_BRIDGE */

/* Open it as you need #define CONFIG_AUDIO_ENABLE */

/*-----------------------------------------------------------------------------------
 * npu pi defense
 *-----------------------------------------------------------------------------------*/
#define SOC_CRG_BASE_ADDR	0x11010000
#define CRG_NPU_CPM_CLK_OFFSET		0x4C84
#define CRG_NPU_FFS_CONFIG_OFFSET	0x4E80
#define CRG_NPU_FFS_CLK_OFFSET		0x4E84
#define CRG_NPU_FFS_STATE_OFFSET	0x4E88
#define CRG_NPU_CLK_CTRL_OFFSET		0x6680

#define SOC_SYS_BASE_ADDR	0x11020000
#define SYS_NPU_VOL_OFFSET			0x9204
#define SYS_NPU_CPM_CONFIG_OFFSET	0xB220
#define SYS_NPU_POWER_CPM_OFFSET	0xB224
#define SYS_NPU_CPM_MA_VAL_OFFSET	0xB230

#define SOC_NPU_TOP_BASE_ADDR	0x14000000
#define NPU_TOP_DROP_FLAG_OFFSET	0x28

#define NPU_CPM_POWER_MAX_VAL	0xFFFF
#define NPU_CPM_MA_MASK	0x1F
#define NPU_CPM_POWER_STEP	4

#define NPU_CPM_MA_MIN_VAL	21
#define NPU_CPM_MA_MIN_MAX	30
#define NPU_CPM_THRESHOLD_DIFF	20
#define NPU_CPM_THRESHOLD_BIT	12

#define NPU_VOL_OFFSET_VAL	70
#define NPU_DELAY_TIME_US	10000
#define NPU_CRG_DELAY_TIME	4
#define NPU_GET_MA_TIMES	5

#define NPU_FFS_RESET_VAL	0x00001001
#define NPU_FFS_UNRESET_VAL	0x00001000
#define NPU_CPM_RESET_VAL	0x00000011
#define NPU_CPM_UNRESET_VAL	0x00000010
#define NPU_CLK_CTRL_VAL	0x01000010
#define NPU_CLK_DISABLE_VAL	0x01000000
#define NPU_DROP_FLAG_VAL	0x00000100
#define NPU_FFS_DEF_VAL		0x000c1090
#define NPU_FFS_CLK_VAL		0x000c1290
#define NPU_CPM_DEF_VAL		0x00800000
#define NPU_FFS_STATE_MASK_VAL	0x1000

/*-----------------------------------------------------------------------------------
 * otp user interface register
 *-----------------------------------------------------------------------------------*/
#define REG_BASE_OTP_USER_IF      0x10120000
#define OTP_USER_LOCKABLE0        (REG_BASE_OTP_USER_IF + 0x2058)
#define OTP_SOC_TEE_DISABLE_FLAG  0x42
#define NUM_FF  0xff
#define EXCEPTION_LEVEL1        1
#define EXCEPTION_LEVEL2        2
#define EXCEPTION_LEVEL3        3

#define OTP_REE_CMD_MODE    0
#define OTP_TEE_CMD_MODE    1

#define OTP_TEE_DISABLE    0
#define OTP_TEE_ENABLE     1

/*-----------------------------------------------------------------------------------
 * CA_MISC interface register
 *-----------------------------------------------------------------------------------*/
#define REG_BASE_CA_MISC        0x10115000
#define REG_SCS_CTRL            (REG_BASE_CA_MISC + 0x400)
#define SCS_FINISH_FLAG         0x5
#define SCS_FINISH_MASK         0xF

/*-----------------------------------------------------------------------------------
 * WATCH DOG interface register
 *-----------------------------------------------------------------------------------*/
#define REG_BASE_SOC_MISC     0x11020000
#define WATCH_DOG_MODE        0x1
#define REG_BASE_WATCH_DOG    0x11030000
#define WATCH_DOG_LOAD_VAL    0x30
#define WATCH_DOG_CONTROL     (REG_BASE_WATCH_DOG + 0x8)
#define WATCH_DOG_ENABLE      (0x3)

/*-----------------------------------------------------------------------------------
 * MPIDR
 *-----------------------------------------------------------------------------------*/
#define MPIDR_AFFINITY_BITS	8
#define MPIDR_AFFLVL_MASK	0xff
#define MPIDR_CLUSTER_MASK	(MPIDR_AFFLVL_MASK << MPIDR_AFFINITY_BITS)

/*-----------------------------------------------------------------------------------
 * pmc register
 *-----------------------------------------------------------------------------------*/
#define PMC_REG_BASE				(0x11120000) /* pmc base addr */
#define BOOT_IMAGE_ADDR_REG_ADDR		(PMC_REG_BASE + 0x014)
#define BOOT_IMAGE_SIZE_REG_ADDR		(PMC_REG_BASE + 0x018)
#define VERIFY_BACKUP_IMAGE_REG_ADDR		(PMC_REG_BASE + 0x01C) /* b[9:8] */
#define VERIFY_BACKUP_IMAGE_FLAG_BIT_OFFSET	(8)
#define VERIFY_BACKUP_IMAGE_FLAG_BIT_MASK	(0x3 << VERIFY_BACKUP_IMAGE_FLAG_BIT_OFFSET)

/*-----------------------------------------------------------------------------------
 * otp user interface register
 *-----------------------------------------------------------------------------------*/
#define SCPU_OTPC_BASE_ADDR             	0x101E0000
#define OTP_SHADOW_BASE                 	SCPU_OTPC_BASE_ADDR
#define OTP_BIT_ALIGNED_LOCKABLE        	(OTP_SHADOW_BASE + 0x0000)
#define OTP_4BIT_ALIGNED_LOCKABLE       	(OTP_SHADOW_BASE + 0x0004)
#define OTP_BYTE_ALIGNED_LOCKABLE_0     	(OTP_SHADOW_BASE + 0x0010)
#define OTP_BYTE_ALIGNED_LOCKABLE_1     	(OTP_SHADOW_BASE + 0x0014)

/*-----------------------------------------------------------------------------------
 * timestamp address info
 *-----------------------------------------------------------------------------------*/
#define SRAM_BASE				0x04020000
#define SRAM_SIZE				0x20000
#define SRAM_END				(SRAM_BASE + SRAM_SIZE)
#define SYSCNT_AREA_SIZE			0x200
#define SYSCNT_AREA_START_ADDR			(SRAM_END - SYSCNT_AREA_SIZE)
#define SYS_CNT_UNIT_HIGH_OFFSET		(4)
#define SYS_CNT_UNIT_LOW_OFFSET			(0)

/* syscnt time offset numbers */
#define SYS_CNT_BOOTROM_START_OFFSET            (1) /* bootrom start time */
#define SYS_CNT_BOOTROM_END_OFFSET              (2) /* bootrom end time */
#define SYS_CNT_GSL_START_OFFSET		(3) /* gsl start time */
#define SYS_CNT_UBOOT_START_OFFSET	        (4) /* entry uboot time */

#define SYS_CNT_FREQ_24MHZ		        (24)
#define SYS_CNT_RAM_UNIT			(8)
#define SYS_CNT_UNIT_LOW_OFFSET			(0)

#define SYS_CTRL_REG_BASE			0x11020000
#define SYSCNT_UPDATE_OFFSET			0x104C
#define SYSCNT_REG0_OFFSET			0x1050
#define SYSCNT_REG1_OFFSET			0x1054
#define HIGH_BIT_SHIFT				32
#define SYSCNT_FREQ_24MHZ			24

#define TIMESTAMP_MAGIC_VALUE			0x55aa55aa
#define TIMESTAMP_MAGIC_OFFSET			0
#define TIMESTAMP_COUNT_OFFSET			4
#define TIMESTAMP_ITEM_OFFSET			8
#define TIMESTAMP_NAME_LEN				64
#define TIMESTAMP_COUNT_MAX				100

#define FDR_ADDR_BASE					0x40000000
#define TIMESTAMP_MIN_ADDR				0x4000C800
#define TIMESTAMP_MAX_ADDR				0x40010000
#if defined(CONFIG_TIME_ADDR_OFFSET)
#define TIME_RECORD_ADDR (FDR_ADDR_BASE + CONFIG_TIME_ADDR_OFFSET)
#else
#define TIME_RECORD_ADDR (FDR_ADDR_BASE + 0xC800)
#endif

#endif /* __HI3519DV500_H */
