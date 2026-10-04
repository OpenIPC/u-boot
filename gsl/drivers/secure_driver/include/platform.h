/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
 */
#ifndef __PLATFORM_HISILICON_H__
#define __PLATFORM_HISILICON_H__

/******************************************************************************
 *
 *               ON CHIP RAM
 *               -------------------------
 *               | heap                  |
 *               +-----------------------+
 *               | stack                 |
 *               +-----------------------+
 *               | global variable       |
 *               +-----------------------+
 *               | page table            |
 * RAM_BASE--->  -------------------------
 *
 *
 *               ON CHIP ROM
 *               -------------------------
 *               | rodata                |
 *               +-----------------------+
 *               | text                  |
 * ROM_BASE--->  -------------------------
 *
 *
 *
 ******************************************************************************/
#define BOOTRAM_ADDR_BEGIN	    RAM_BASE_ADDR
#define BOOTRAM_ADDR_END	    (BOOTRAM_ADDR_BEGIN + RAM_SIZE)

#define ACPU_RAM_ADDR		    0x40000
#define ACPU_RAM_SIZE		    0x18000

#define SYSCNT_AREA_SIZE	    0x200
#define SYSCNT_AREA_START_ADDR	    (BOOTRAM_ADDR_END - SYSCNT_AREA_SIZE)

#define SECTION_DDR_START	    0x40000000 /* 0x1000_0000 ~ , DDR */

/* Sys ctrl register */
#define REG_SYSCTRL_BASE	    0x11020000
#define GSL_BSS_SIZE		    0x400
/* 0x1000 + 0x600 + 0x400 = 0x1a00 */
#define CP_STEP1_ADDR		    (RAM_BASE_ADDR + STACK_SIZE + BSS_SIZE + GSL_BSS_SIZE)
#define CP_STEP1_SIZE		    (0xa000) /* reserved 40KB BOOT_IMAGE */

#define HEAP_ADDR		    (CP_STEP1_ADDR + CP_STEP1_SIZE)
#define BSS_BOOTROM_END_ADDR	    (BSS_ADDR + BSS_SIZE)
#define BSS_END_ADDR		    (BSS_BOOTROM_END_ADDR + GSL_BSS_SIZE)

#define SECURE_IMAGE_STEP1_SIZE	    CP_STEP1_SIZE /* copy 80KB to bootram */

#define UART_FIRST_SECT_ADDR	    (RAM_BASE_ADDR + 0x3000)
#define UART_FIRST_SECT_SIZE	    0x100

#define USB_FIRST_SECT_ADDR	    UART_FIRST_SECT_ADDR
#define USB_FIRST_SECT_SIZE	    UART_FIRST_SECT_SIZE

/* TEE/REE/TP KEY AREA offset:0x80 + 0x80 + 0x80 = 0x180; addr: CP_STEP1_ADDR */
#define FLASH_ROOT_PUB_KEY_SIZE	    0x200

#define REE_FLASH_ROOT_PUB_KEY_ADDR CP_STEP1_ADDR
#define REE_FLASH_ROOT_PUB_KEY_SIZE FLASH_ROOT_PUB_KEY_SIZE
#define TEE_FLASH_ROOT_PUB_KEY_ADDR \
	(REE_FLASH_ROOT_PUB_KEY_ADDR + REE_FLASH_ROOT_PUB_KEY_SIZE)
#define TEE_FLASH_ROOT_PUB_KEY_SIZE FLASH_ROOT_PUB_KEY_SIZE
#define TP_FLASH_ROOT_PUB_KEY_ADDR \
	(TEE_FLASH_ROOT_PUB_KEY_ADDR + TEE_FLASH_ROOT_PUB_KEY_SIZE)
#define TP_FLASH_ROOT_PUB_KEY_SIZE FLASH_ROOT_PUB_KEY_SIZE

/* root key image size + cfct image size: 0x180 + 0x800 + CFCT_TABLE_FILL_SIZE =
 * 4k */
#define ROOT_PUB_KEY_AREA_SIZE	   (FLASH_ROOT_PUB_KEY_SIZE * 2)
/* GSL offset:0x1000 + 0x100 + 0x100 + 0x200 = 0x1400 */
#define GSL_KEY_AREA_ADDR	   (REE_FLASH_ROOT_PUB_KEY_ADDR + ROOT_PUB_KEY_AREA_SIZE)
#define GSL_KEY_AREA_SIZE	   0x400
#define GSL_CODE_INFO_ADDR	   (GSL_KEY_AREA_ADDR + GSL_KEY_AREA_SIZE)
#define GSL_CODE_INFO_SIZE	   0x400
#define GSL_CODE_AREA_ADDR	   (GSL_CODE_INFO_ADDR + GSL_CODE_INFO_SIZE)
/* GSL key area + info area size: 0x400 */
#define GSL_AREA_INFO_SIZE	   (GSL_KEY_AREA_SIZE + GSL_CODE_INFO_SIZE)

#define GSL_CODE_INFO_OFFSET	   (ROOT_PUB_KEY_AREA_SIZE + GSL_KEY_AREA_SIZE)
#define BOOT_IMAGE_FIRST_LOAD_SIZE (ROOT_PUB_KEY_AREA_SIZE + GSL_AREA_INFO_SIZE)

#define PUBLIC_IV_LEN		   0x10 /* 4 word size */
#define UBOOT_FLAG_OFST		   (0x3c)
#define UBOOT_FLAG_VAL		   (0x12345678)

/*-----------------------------------------------------------------
 * sysctrl register
 ------------------------------------------------------------------ */
#define REG_BASE_SCTL		   0x11020000
#define REG_SC_SYSRES		   0x0004
#define REG_SC_SYSSTAT		   (REG_BASE_SCTL + 0x18)
#define NON_SLAVE_BOOT_FLAG(x)	   ((((x) >> 5) & 0x1) == 0x0)
#define UART_BOOT_FLAG(x)	   ((((x) >> 4) & 0x3) == 0x1)
#define PCIE_SLAVE_BOOT_FLAG(x)	   ((((x) >> 4) & 0x3) == 0x2)
#define USB_SLAVE_BOOT_FLAG(x)	   ((((x) >> 4) & 0x3) == 0x3)

#define SYSCNT_UPDATA0		   (REG_BASE_SCTL + 0x104C)
#define SYSCNT_REG0		   (REG_BASE_SCTL + 0x1050)
#define SYSCNT_REG1		   (REG_BASE_SCTL + 0x1054)

#define SELF_BOOT_DEV_REG_OFFSET \
	(0x14) /* sys_ctrl, SC_PLLCTRL 0x1102_0000+0x0014, b[7:0] */
#define SELF_BOOT_DEV_REG_ADDR	     (REG_BASE_SCTL + SELF_BOOT_DEV_REG_OFFSET)

/* SELF_BOOT_DEV_REG_ADDR [3:0] */
#define BURN_MODE_BIT_OFFSET	     (0x0)
#define BURN_MODE_BIT_WIDTH	     (0x4)

#define UART_NO_NUDE		     0x1 /* [3:0] */
#define USB_DEVICE_NO_NUDE	     0x2 /* [3:0] */
#define USB_HOST_NO_NUDE	     0x3 /* [3:0] */
#define SD_CARD_NO_NUDE		     0x4 /* [3:0] */

#define UART_DDR		     0x1 /* [3:0] */
#define USB_DDR			     0x1 /* [3:0] */

/* SELF_BOOT_DEV_REG_ADDR [7:4] */
#define SELF_BOOT_DEV_REG_BIT_OFFSET (0x4)
#define SELF_BOOT_DEV_REG_BIT_WIDTH  (0x4)

#define SELF_BOOT_FLASH		     0x0 /* [7:4] */
#define SELF_BOOT_UART		     0x1 /* [7:4] */
#define SELF_BOOT_USB_DEV_SLAVE	     0x2 /* [7:4] */
#define SELF_BOOT_USB_DEV_BURN	     0x3 /* [7:4] */

#define REG_SC_GEN0		     0x0130
#define REG_SC_GEN1		     0x0134
#define REG_SC_GEN3		     0x013C
#define REG_SC_GEN4		     0x0140
#define REG_SC_GEN5		     0x0144
#define REG_SC_GEN6		     0x0148
#define REG_SC_GEN7		     0x014c
#define REG_SC_GEN8		     0x0150
#define REG_SC_GEN9		     0x0154
#define REG_SC_GEN10		     0x0158
#define REG_SC_GEN11		     0x015c

#define REG_TEE_GEN_REG0	     0x500

#define REG_START_WARM_ENTRYPOINT    (REG_SYSCTRL_BASE + REG_TEE_GEN_REG0)

#define REG_UPS_UPDATE_ENABLE	     0x0410
#define GET_UPS_UPDATE_FLAG(val)     (((val) >> 1) & 0x1)
#define REG_UPS_CONFIG_VAL_START     0x0418
#define REG_UPS_CONFIG_VAL_END	     0x0434
#define CONFIG_VAL_REG_NUM	     0x8
#define UPS_MODE_MASK		     (0x7 << 16)
#define GET_UPS_MODE(val)	     (((val) & (UPS_MODE_MASK)) >> 16)

#define PCIE_SLAVE_BOOT_CTL_REG	     REG_SC_GEN1
#define DDR_INIT_DOWNLOAD_OK_FLAG \
	0xDCDFF001 /* step1:Ddrinit Code Download Finished Flag:  DCDFF001 */
#define DDR_INIT_EXCUTE_OK_FLAG \
	0xDCEFF002 /* step2:Ddrinit Code Excute Finished Flag:    DCEFF002 */
#define UBOOT_DOWNLOAD_OK_FLAG \
	0xBCDFF003 /* step3:Boot Code Download Finished Flag:     BCDFF003 */
#define KEY_AREA_DOWNLOAD_OK_FLAG \
	0xDCDFF001 /* step1:Key Area Download Finished Flag:      DCDFF001 */

#define CHIP_FEATURE_CFG_LOCK	    (0x1800)
#define CFCT_LOACK_REG_ADDR	    (REG_BASE_SCTL + CHIP_FEATURE_CFG_LOCK)
#define CHIP_FEATURE_CFG_LOCK_MASK  (0x1)
#define CFCT_LOCK		    (0x1)
#define CFCT_UNLOCK		    (0x0)
/*-----------------------------------------------------------------
 * CRG register
 ------------------------------------------------------------------ */
#define REG_BASE_CRG		    0x11010000

#define PERI_CRG_PLL0		    0x0
#define PERI_CRG_PLL1		    0x4
#define PERI_CRG_PLL33		    0x84
#define PERI_CRG_PLL129		    0x204
#define PERI_CRG_PLL193		    0x304
#define PERI_CRG_PLL673		    0xa84

#define REG_GPIO0_CLK	    	(REG_BASE_CRG + 0x4768)

#define REG_PERI_CRG2064	    0x2040
#define REG_PERI_CRG2048	    0x2000

#define REG_PERI_CRG4048	    0x3f40
#define FMC_CLK_ENABLE		    (0x1 << 4)
#define FMC_SRST_REQ		    (0x1 << 0)

#define REG_PERI_CRG4192	    0x4180
#define UART0_CLK_ENABLE	    (0x1 << 4)
#define UART0_CLK_MASK		    (0x3 << 12)
#define UART0_CLK_3M		    (0x3 << 12)
#define UART0_CLK_24M		    (0x2 << 12)
#define UART0_CLK_50M		    (0x1 << 12)
#define UART0_CLK_100M		    (0x0 << 12)
#define UART0_SRST_REQ		    (0x1 << 0)

#define APLL_LOCK_REG		    0x38
#define BPLL_LOCK_REG		    0xb8
#define DPLL_LOCK_REG		    0x1b8
#define EPLL_LOCK_REG		    0x238
#define FPLL_LOCK_REG		    0x2b8
#define PPLL_LOCK_REG		    0x7b8
#define SPLL_LOCK_REG		    0x938
#define VPLL_LOCK_REG		    0xab8

#define PLL_LOCK		    (0x1 << 0)

#define PLL_ISO			    (0x1 << 0)

/*-----------------------------------------------------------------
 * VO_AIAO_SUBSYS_MISC_REG register
 ------------------------------------------------------------------ */
#define VO_AIAO_SUBSYS_MISC_REG	    0x17c90000
#define VIVO_AIAO_ECO_REG0	    0xf00

/*-----------------------------------------------------------------
 * MISC register
 ------------------------------------------------------------------ */
#define REG_BASE_HP_SUBSYS_MISC_REG 0x10fe0000
#define HP_UPS_PHY_CTRL3	    0x88
#define HP_UPS_PHY_CTRL4	    0x8C
#define REG_BASE_MISC		    0x11020000
#define MISC_SYS_CTRL		    0x10
#define DDR_CA0_OFST		    0x24
#define DDR_CA1_OFST		    0x28
#define DDR_CA2_OFST		    0x2c

#define REG_AHB_MISC_BASE	    0x10270000

#define ROM_HIDE_EN_REG_OFFSET	    (0xc)
#define ROM_HIDE_EN_REG_ADDR \
	(REG_AHB_MISC_BASE + ROM_HIDE_EN_REG_OFFSET) /* b[3:0]) */
#define ROM_HIDE_EN_REG_BIT_OFFSET (0x0)
#define ROM_HIDE_EN_REG_BIT_WIDTH  (0x4)
#define ROM_HIDE_ACCESSIBLE	   (0xA)
#define ROM_HIDE_NOT_ACCESSIBLE	   (0x5)

#define ROM_HIDE_CFG_REG_OFFSET	   (0x8)
#define ROM_HIDE_CFG_REG_ADDR \
	(REG_AHB_MISC_BASE + ROM_HIDE_CFG_REG_OFFSET) /* b[7:0]) */

#define ROM_HIDE_GRANULARITY		    256

#define ROM_HIDE_SIZE_REG_BIT_OFFSET	    0x0
#define ROM_HIDE_SIZE_REG_BIT_WIDTH	    (0x8)

/*-----------------------------------------------------------------
 * CA_MISC register
 *------------------------------------------------------------------ */
#define REG_BASE_CA_MISC		    0x101E8000
#define TEE_CRG_CTRL_REG		    0x304
#define SCS_CTRL			    0x10

#define SCS_FINISH_ADDR			    (REG_BASE_CA_MISC + SCS_CTRL) /* b[3:0]) */
#define SCS_FINISH_ADDR_BIT_OFFSET	    (0x0)
#define SCS_FINISH_ADDR_BIT_WIDTH	    (0x4)
#define SCS_IS_FINISH			    (0x5)
#define SCS_IS_NOT_FINISH		    (0xA)

#define TEE_FAILURE_SOURCE_ADDR		    (REG_BASE_CA_MISC + 0x170) /* b[3:0]) */
#define TEE_FAILURE_SOURCE_BIT_OFFSET	    (0x0)
#define TEE_FAILURE_SOURCE_BIT_WIDTH	    (0x4)

#define REE_FLASH_ROOTKEY_STATUS_ADDR	    (REG_BASE_CA_MISC + 0x150) /* b[3:0] */
#define REE_FLASH_ROOTKEY_STATUS_BIT_OFFSET (0)
#define REE_FLASH_ROOTKEY_STATUS_BIT_WIDTH  (4)

#define TP_FLASH_ROOTKEY_STATUS_ADDR	    (REG_BASE_CA_MISC + 0x150) /* b[11:8] */
#define TP_FLASH_ROOTKEY_STATUS_BIT_OFFSET  (8)
#define TP_FLASH_ROOTKEY_STATUS_BIT_WIDTH   (4)

#define TEE_FLASH_ROOTKEY_STATUS_ADDR	    (REG_BASE_CA_MISC + 0x130) /* b[3:0] */
#define TEE_FLASH_ROOTKEY_STATUS_BIT_OFFSET (0)
#define TEE_FLASH_ROOTKEY_STATUS_BIT_WIDTH  (4)

#define VALID				    (0x5)
#define INVALID				    (0xA)

#define TEE_VERIFY_ENABLE_FLAG_ADDR	    (REG_BASE_CA_MISC + 0x154)
#define TEE_VERIFY_ENABLE_FLAG_BIT_OFFSET   (0)
#define TEE_VERIFY_ENABLE_FLAG_BIT_WIDTH    (8)

#define REE_VERIFY_ENABLE_FLAG_ADDR	    (TEE_VERIFY_ENABLE_FLAG_ADDR)
#define REE_VERIFY_ENABLE_FLAG_BIT_OFFSET   (8)
#define REE_VERIFY_ENABLE_FLAG_BIT_WIDTH    (8)

#define TP_VERIFY_ENABLE_FLAG_ADDR	    (TEE_VERIFY_ENABLE_FLAG_ADDR)
#define TP_VERIFY_ENABLE_FLAG_BIT_OFFSET    (16)
#define TP_VERIFY_ENABLE_FLAG_BIT_WIDTH	    (8)

/*-----------------------------------------------------------------
 * serial base address and clock
 ------------------------------------------------------------------ */
#define REG_BASE_SERIAL0		    0x11040000
#define CONFIG_PL011_CLOCK		    24000000 /* Serial need 24M clock input */
#define CONFIG_CONS_INDEX		    0 /* select the default console */

/*-----------------------------------------------------------------
 * timer0 register
 ------------------------------------------------------------------ */
#define REG_BASE_TIMER0			    0x11000000
#define TIMER_LOAD			    0x0000
#define TIMER_VALUE			    0x0004
#define TIMER_CONTROL			    0x0008
#define TIMER_EN			    (1 << 7)
#define TIMER_MODE			    (1 << 6)
#define TIMER_PRE			    (0 << 2)
#define TIMER_SIZE			    (1 << 1)

#define CFG_TIMERBASE			    REG_BASE_TIMER0
#define READ_TIMER			    (*(volatile unsigned int *)(CFG_TIMERBASE + TIMER_VALUE))
#define TIMER_FEQ			    (3000000)
/* how many ticks per second. show the precision of timer. */
#define TIMER_DIV			    (1)
#define CONFIG_SYS_HZ			    (TIMER_FEQ / TIMER_DIV)

/*-----------------------------------------------------------------
 * boot   Configuration
 ------------------------------------------------------------------ */
#define BOOTROM_STAT_STACK		    0x0
#define BOOTROM_STAT_START		    0x1
#define BOOTROM_STAT_MAIN		    0x2
#define BOOTROM_STAT_SLOW		    0x3
#define BOOTROM_STAT_NORMAL		    0x4
#define BOOTROM_STAT_SELFBOOT		    0x5
#define BOOTROM_STAT_UART		    0x6
#define BOOTROM_STAT_SPI		    0x7
#define BOOTROM_STAT_SPI_NAND		    0x8
#define BOOTROM_STAT_NAND		    0x9
#define BOOTROM_STAT_BOOT		    0xa
#define BOOTROM_STAT_ERR		    0xb
#define BOOTROM_STAT_INTR		    0xc
#define BOOTROM_STAT_EMMC		    0xd
#define BOOTROM_STAT_SDIO		    0xe
#define BOOTROM_STAT_USB		    0xf

#define SPI_BASE_ADDR			    0x0f000000
#define DDR_BASE_ADDR			    0x40000000

#define SRAM_DOWNLOAD_ADDR		    HEAP_ADDR
#define DDR_DOWNLOAD_ADDR		    0x41000000
#define DDR_DECRYPT_UBOOT_ADDR		    0x41800000

#define SRAM_DOWNLOAD_SIZE		    CP_STEP1_SIZE
#define DDR_DOWNLOAD_SIZE		    0x80000 /* 512K */

#define CAL_UBOOT_DDR_START_ADDR(ddr_init_len, n_len)                    \
	(DDR_DOWNLOAD_ADDR + 4 * sizeof(unsigned int) + (ddr_init_len) + \
	 3 * (n_len))

#define SELF_BOOT_START_MAGIC		     (0x7a696a75)
#define SELF_BOOT_DOWNLOAD_MAGIC	     (0x444f574e)
#define SELF_BOOT_EMMC_NO_NUDE		     (0x454d4d43)
#define SELF_BOOT_TYPE_UART		     0x1
#define SELF_BOOT_TYPE_USBDEV		     0x2

#define SECURE_UBOOT_IMAGE_MAGIC	     (0x4253424D) /* Bvt Secure Boot Magic */
#define BACKUP_IMAGE_FLG_REG		     (0xC8)
#define BACKUP_IMAGE_OFF_REG		     (0xCC)
#define BACKUP_IMAGE_TIMES_REG		     (0x314)
#define BACKUP_IMAGE_ADDR_REG		     (0x318)
#define DATA_CHANNEL_TYPE_REG		     (0x31c)
#define BACKUP_IMAGE_ENABLE		     (0x424945) /* Backup Image Enable */
#define GET_BACKUP_IMAGE_FLAG(val)	     (((val) >> 4) & 0xffffff)
#define GET_BACKUP_OFFSET_TIMES(val)	     (((val) >> 4) & 0xfff)
#define OFFSET_SIZE			     (64 * 1024)
#define FLASH_MAX_BACKUP_IMAGE_OFFSET	     (2 * 1024 * 1024)
#define GET_BACKUP_IMAGE_OTP_FLAG(val)	     (((val) >> 11) & 0x1)

/*
 * AHB_IOCFG
 */
#define REG_CORE_IO_CFG_BASE		     0x110A0000

#define UART0_RXD_IOCFG_OFST		     0x68
#define UART0_TXD_IOCFG_OFST		     0x6C

/* SFC_IOCFG */
#define REG_UPS_IO_CFG_BASE		     0x10260000
#define SFC_MOSI_IO_0_REG		     0x30
#define SFC_CLK_REG			     0x38
#define SFC_HOLD_IO_3_REG		     0x40
#define SFC_CS_0_N_REG			     0x44
#define SFC_MOSI_IO_1_REG		     0x3C
#define SFC_WP_IO_2_REG			     0x34

#define READ_DATA_BY_CPU		     0x01
#define READ_DATA_BY_DMA		     0x02
/*************************************************************
 *		Secure boot
 *************************************************************/
#define TOTAL_LEN_OFST			     (0x4)
#define RSA_N_PARA_OFST			     (0x8)
#define RSA_E1_PARA_OFST		     (0xC)
#define KEY_ADDR_OFST			     (0x10)
#define AES_IV_LEN			     (16)
#define GET_DDR_INIT_ADDR_OFST(n_len, e_len) ((n_len) + (e_len) + 40)

#define KEY_BUF_SIZE			     (0x800)
#define SHA_DATA_SIZE			     (0x20)
#define BACKUP_DATA_SIZE		     (0x78)
#define OAEP_CHECK_BUF_SIZE		     (0x200)
#define MAX_IMAGE_SIZE			     (0x100000)

/*
 * AES CBC decryptiong, use key 0, which is configured by cpu,
 * key length is 128 bit.
 */
#define CIPHER_CTRL_SEL			     ((0x2 << 4) | (0x1 << 1) | 0x1)

/*------------------------------------------------------------------------
 * ddrc register
 *------------------------------------------------------------------------*/
#define REG_BASE_DDRC			     0x11130000

#define DDRC_CTRL_SREF_OFST		     (0x8000 + 0x0)
#define DDRC_CFG_DDRMODE_OFST		     (0x8000 + 0x50)
#define DDRC_CURR_FUNC_OFST		     (0x8000 + 0x294)

#define DDRC1_CTRL_SREF_OFST		     (0x9000 + 0x0)
#define DDRC1_CFG_DDRMODE_OFST		     (0x9000 + 0x50)
#define DDRC1_CURR_FUNC_OFST		     (0x9000 + 0x294)

#define DDRC2_CTRL_SREF_OFST		     (0xa000 + 0x0)
#define DDRC2_CFG_DDRMODE_OFST		     (0xa000 + 0x50)
#define DDRC2_CURR_FUNC_OFST		     (0xa000 + 0x294)

#define DDRC3_CTRL_SREF_OFST		     (0xb000 + 0x0)
#define DDRC3_CFG_DDRMODE_OFST		     (0xb000 + 0x50)
#define DDRC3_CURR_FUNC_OFST		     (0xb000 + 0x294)

#define DDRC_CHANNEL_VALID_MASK		     (0xf)
#define DDRC_SELF_REFURBISH_MASK	     (0x1)

#define DDRC_SELF_REFURBISH_EN		     0x1
#define DDRC_SELF_REFURBISH_EXIT	     (0x1 << 1)

#define UPDATE_FROM_USBDEV_DISABLE	     (9)
#define UPDATE_FROM_SDIO_DISABLE	     (8)

/*-----------------------------------------------------------------------------------
 * gpio register
 *-----------------------------------------------------------------------------------*/
#define REG_BASE_GPIO0			     0x11090000
#define GPIO0_0_DATA_OFST		     0x4
#define GPIO_DIR_OFST			     0x400

#define USB_REG_BASE_ADDR				0x10500000

/*-----------------------------------------------------------------------------------
 * iocfg register
 *-----------------------------------------------------------------------------------*/
#define REG_VDP_AHB0_BASE_IOCFG		     0x10230000
#define IOCFG_REG0			     0x0
#define IOCFG_REG1			     0x4
#define IOCFG_REG2			     0x8
#define IOCFG_REG3			     0xc
#define IOCFG_REG4			     0x10
#define IOCFG_REG5			     0x14
#define IOCFG_REG6			     0x18
#define IOCFG_REG7			     0x1c
#define IOCFG_REG8			     0x20
#define IOCFG_REG9			     0x24
#define IOCFG_REG10			     0x28
#define IOCFG_REG11			     0x2c
#define IOCFG_REG12			     0x30
#define IOCFG_REG13			     0x34

/*-----------------------------------------------------------------------------------
 * otp user interface register
 *-----------------------------------------------------------------------------------*/
#define SCPU_OTPC_BASE_ADDR		     0x101E0000

#define OTP_SHADOW_BASE				SCPU_OTPC_BASE_ADDR
#define OTP_BIT_ALIGNED_LOCKABLE		(OTP_SHADOW_BASE + 0x0000)
#define OTP_4BIT_ALIGNED_LOCKABLE		(OTP_SHADOW_BASE + 0x0004)
#define OTP_4BIT_ALIGNED_LOCKABLE_1		(OTP_SHADOW_BASE + 0x0008)
#define OTP_H10CC_REV_VALUE1 	                (OTP_SHADOW_BASE + 0x000C)
#define OTP_BYTE_ALIGNED_LOCKABLE_0		(OTP_SHADOW_BASE + 0x0010)
#define OTP_BYTE_ALIGNED_LOCKABLE_1		(OTP_SHADOW_BASE + 0x0014)
#define OTP_SOC_FUSE_REG0	  		(OTP_SHADOW_BASE + 0x001C)
#define OTP_SOC_FUSE_REG1	  		(OTP_SHADOW_BASE + 0x0020)
#define OTP_H10CC_REV_VALUE2 	                (OTP_SHADOW_BASE + 0x0024)
#define OTP_SOC_FUSE_REG2	  		(OTP_SHADOW_BASE + 0x0028)
#define OTP_SOC_FUSE_REG3	  		(OTP_SHADOW_BASE + 0x002C)
#define OTP_ATE_DATA_REG0			(OTP_SHADOW_BASE + 0x0110)
#define OTP_ATE_DATA_REG1			(OTP_SHADOW_BASE + 0x0118)
#define OTP_ATE_DATA_REG2			(OTP_SHADOW_BASE + 0x011c)
#define OTP_SHADOWED_ONEWAY_0			(OTP_SHADOW_BASE + 0x01E0)
#define OTP_SHADOWED_ONEWAY_1			(OTP_SHADOW_BASE + 0x01E4)
#define OTP_SHADOWED_ONEWAY_2			(OTP_SHADOW_BASE + 0x01E8)
#define OTP_SHADOWED_ONEWAY_3			(OTP_SHADOW_BASE + 0x01EC)
#define OTP_BIT_ALIGNED_LOCKER			(OTP_SHADOW_BASE + 0x01F0)
#define OTP_4BIT_ALIGNED_LOCKER			(OTP_SHADOW_BASE + 0x01F4)

#define SEC_BOOT_DBG_ENABLE		     (0x42)
#define SEC_BOOT_DBG_DIABLE_MASK	     (0xFF0000)
#define ROM_DBG_ENABLE			     (0x5)
#define ROM_DBG_NOT_RESET		     (0xa)
#define ROM_DBG_DISABLE_MASK		     (0xF0000)

#define OTP_TEE_HASH_FLASH_ROOTKEY	     (0x40)
#define OTP_REE_HASH_FLASH_ROOTKEY	     (0x60)
#define OTP_SOC_HASH_FLASH_ROOTKEY	     (0x80)
#define OTP_SOC_SM3_FLASH_ROOTKEY	     (0x150)

#define BOOT_IMAGE_ADDR_REG_ADDR	     (REG_SYSCTRL_BASE + 0x0180)
#define BOOT_IMAGE_SIZE_REG_ADDR	     (REG_SYSCTRL_BASE + 0x0184)
#define VERIFY_BACKUP_IMAGE_REG_ADDR	     (REG_SYSCTRL_BASE + 0x0188) /* b[9:8] */
#define VERIFY_BACKUP_IMAGE_FLAG_BIT_OFFSET  (0x0)
#define VERIFY_BACKUP_IMAGE_FLAG_BIT_WIDTH   (0x2)

#define BOOT_FROM_PRIME			     (0x0)
#define BOOT_FROM_BACKUP		     (0x1)
#define BOOT_FROM_IMAGE_FAIL		     (0x2)

#define PRIME_IMAGE_FLASH_OFFSET	     (0x0)
#define BACKUP_IMAGE_FLASH_OFFSET	     (64 * 1024)
#define BACKUP_ENABLE			     (1)

/*-----------------------------------------------------------------------------------
 * otp read only register
 *-----------------------------------------------------------------------------------*/
#define REG_BASE_OTP_READ_ONLY		     0x11020000
#define OTP_PO_INFO_32			     (REG_BASE_OTP_READ_ONLY + 0x1550)

/*-----------------------------------------------------------------------------------
 * dbc_apb register
 *-----------------------------------------------------------------------------------*/
#define REG_BASE_DBC_APB		     0x10113000
#define DBC_APB_JTAG_DBG_INFO		     (REG_BASE_DBC_APB + 0x0d4)

/*-----------------------------------------------------------------------------------
 * boot start mode
 *-----------------------------------------------------------------------------------*/
#define SECURE_START_MODE		     0x01
#define FAST_START_MODE			     0x02

/*-----------------------------------------------------------------------------------
 * boot sel type
 *-----------------------------------------------------------------------------------*/
#define BOOT_SEL_PCIE			     0x965a4b87
#define BOOT_SEL_UART			     0x69a5b478
#define BOOT_SEL_USB			     0x965ab487
#define BOOT_SEL_SDIO			     0x69a54b87
#define BOOT_SEL_FLASH			     0x96a54b78
#define BOOT_SEL_EMMC			     0x695ab487
#define BOOT_SEL_UNKNOW			     0x965a4b78

/* watchdog register */
#define REG_WDG_BASE_ADDR		     0x11030000

/* DBC register */
#define REG_DBC_BASE_ADDR		     0x101E2000
#define REG_DBC_STR_OFF			     0x50
#define REG_DBC_LENTH			     16
#define REG_DBC_STR_ADDR		     (REG_DBC_BASE_ADDR + REG_DBC_STR_OFF)

#define MAX_DBGINFO_LV			     2
#define DBGINFO_TO_REG_LV		     1
#define DBGINFO_TO_REG_UART		     0

#define BITWIDTH1		    	1
#define BITWIDTH2		    	2
#define BITWIDTH3	      		3
#define BITWIDTH4		    	4
#define BITWIDTH5		    	5
#define BITWIDTH6		    	6
#define BITWIDTH7		    	7
#define BITWIDTH8		    	8
#define BITWIDTH9		    	9
#define BITWIDTH10		    	10
#define BITWIDTH11		    	11
#define BITWIDTH12		    	12
#define BITWIDTH13		    	13
#define BITWIDTH14		    	14
#define BITWIDTH15		    	15
#define BITWIDTH16		    	16
#define BITWIDTH18		    	18
#define BITWIDTH20		    	20
#define BITWIDTH22		    	22
#define BITWIDTH24		    	24

#endif /* __PLATFORM_HISILICON_H__ */
