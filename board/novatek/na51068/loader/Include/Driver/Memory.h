# ifndef _MEMORY_680_H
# define _MEMORY_680_H


# define DRAM_BASE_ADDR             (0x0000000)
# define DRAM_EXP_BASE_ADDR         (DRAM_BASE_ADDR + 0x180)
# define DRAM_INT_BASE_ADDR         (DRAM_BASE_ADDR + 0x200)

# define StackSize_KNL              0x1000       /* 4KB */
# define StackSize_INT              0x400        /* 1KB */

# define FAT_HEAP_BUFFER_SIZE       0xA0000
# define LOADER_TMP_BUFFER_SIZE     0x80000

# define BOOT_ROM_SP_LIMIT_ADDR     0xFE0F8000
# define BOOT_ROM_RAM_TEMP_ADDR     0xF0800200
# define BOOT_ROM_CFG_TEMP_ADDR     0xF0800248
# define BOOT_ROM_RAM_BASE_ADDR     0x00000000
# define DMA2_CTRL_REG_BASE_ADDR    0xF0100000
# define DMA2_PHY_REG_BASE_ADDR     0xF0101000
# define USB_OTG_REG_BASE_ADDR      0xF0600000
# define DDR_CTRL_REG_BASE_ADDR     0xFC400000      /* DDR phy base address     */
# define DDR_PHY_REG_BASE_ADDR      0xFC408000
# define DDR_ARB_REG_BASE_ADDR      0xF0008000
# define DMA2_ARB_REG_BASE_ADDR     0xF0108000

# define GPIO_CTRL_REG_BASE_ADDR    0xF0070000
# define PMC_REG_BASE               0xF00A0000



# define UART2_REG_BASE_ADDR        0xF0300000
# define SM_HOST_REG_BASE_ADDR      0xF0400000
# define MS_HOST_REG_BASE_ADDR      0xF0410000
# define SD_HOST1_REG_BASE_ADDR     0xF0420000
# define SD_HOST2_REG_BASE_ADDR     0xF0500000
# define SD_HOST3_REG_BASE_ADDR     0xF0510000
# define SPI_CTRL_REG_BASE_ADDR     0xF0230000
# define ETH_CTRL_REG_BASE_ADDR     0xFCD00000
# define IDE_SRAM_BASE_ADDR         0xF07F0000
# define IDE_CTRL_REG_BASE_ADDR     0xF0800000
# define TIMER_REG_BASE_ADDR        0xF0040000
# define WDT_REG_BASE_ADDR          0xF0050000

# define BOOT_LOADER_TMP_BUFFER     0x00001000
# define CHIP_REMAP_REG_OFFSET      0x00000088
# define CPE_RTC_BASE               0xF0060000      /* RTC controller           */
# define CPE_GPIO_BASE              0xF0070000      /* GPIO controller          */
# define INTERRUPT_BASE_ADDR        0xF0080000      /* Interrupt controller     */
# define CC_BASE_ADDR               0xF0090000      /* CC controller     */
# define IOADDR_GIC_REG_BASE        0xF1500000      /* GIC Interrupt     */
# define PAD_REG_BASE				0xF0030000

#define EFUSE_CTRL_REG_BASE_ADDR    0xFECE0000
#define CRYPTO_REG_BASE_ADDR	    0xFEC60000
#define HASH_REG_BASE_ADDR	        0xFEC80000
#define RSA_REG_BASE_ADDR	        0xFECA0000

#define BASE_CA9_TIMER  0xF9C00000
#define BASE_PMU		0xFE000000  //SCU


#define CC_CPU2_CPU1_CMDBUF_REG1 	0xFE0B9000						//CC_CPU2_CPU1_CMDBUF_REG1 to store uboot starting address
#define CC_CPU2_CPU1_CMDBUF_REG2 	0xFE0B9004 						//CC_CPU2_CPU1_CMDBUF_REG1 has used in core2_entry.s
#define NVT_CORE2_START 			CC_CPU2_CPU1_CMDBUF_REG2		//core2_entry.S 's entry point

// Crypto controller register address
#define CRYPTO_CONFIG_REG        (CRYPTO_REG_BASE_ADDR + 0x00)
#define CRYPTO_CONTROL_REG       (CRYPTO_REG_BASE_ADDR + 0x04)
#define CRYPTO_OUT_DATA_REG      (CRYPTO_REG_BASE_ADDR + 0x08)
#define CRYPTO_STATUS_REG        (CRYPTO_REG_BASE_ADDR + 0x0C)

#define CRYPTO_PIO_KEY0_REG      (CRYPTO_REG_BASE_ADDR + 0x10)
#define CRYPTO_PIO_KEY1_REG      (CRYPTO_REG_BASE_ADDR + 0x14)
#define CRYPTO_PIO_KEY2_REG      (CRYPTO_REG_BASE_ADDR + 0x18)
#define CRYPTO_PIO_KEY3_REG      (CRYPTO_REG_BASE_ADDR + 0x1C)

#define CRYPTO_PIO_INPUT0_REG    (CRYPTO_REG_BASE_ADDR + 0x30)
#define CRYPTO_PIO_INPUT1_REG    (CRYPTO_REG_BASE_ADDR + 0x34)
#define CRYPTO_PIO_INPUT2_REG    (CRYPTO_REG_BASE_ADDR + 0x38)
#define CRYPTO_PIO_INPUT3_REG    (CRYPTO_REG_BASE_ADDR + 0x3C)
#define CRYPTO_PIO_OUTPUT0_REG   (CRYPTO_REG_BASE_ADDR + 0x40)
#define CRYPTO_PIO_OUTPUT1_REG   (CRYPTO_REG_BASE_ADDR + 0x44)
#define CRYPTO_PIO_OUTPUT2_REG   (CRYPTO_REG_BASE_ADDR + 0x48)
#define CRYPTO_PIO_OUTPUT3_REG   (CRYPTO_REG_BASE_ADDR + 0x4C)

// Hash
#define HASH_CONFIG_REG         (HASH_REG_BASE_ADDR + 0x00)
#define HASH_STATUS_REG         (HASH_REG_BASE_ADDR + 0x0C)
#define HASH_OUTPUT_DATA1_REG   (HASH_REG_BASE_ADDR + 0x70)
#define HASH_INPUT_DATA_REG     (HASH_REG_BASE_ADDR + 0x90)

//RSA
#define RSA_CONFIG_REG               (RSA_REG_BASE_ADDR + 0x00)
#define RSA_CONTROL_REG              (RSA_REG_BASE_ADDR + 0x04)
#define RSA_STATUS_REG               (RSA_REG_BASE_ADDR + 0x0C)
#define RSA_KEY_N_REG                (RSA_REG_BASE_ADDR + 0x10)
#define RSA_KEY_N_ADDR_REG           (RSA_REG_BASE_ADDR + 0x14)
#define RSA_KEY_ED_REG               (RSA_REG_BASE_ADDR + 0x18)
#define RSA_KEY_ED_ADDR_REG          (RSA_REG_BASE_ADDR + 0x1C)
#define RSA_DATA_REG                 (RSA_REG_BASE_ADDR + 0x20)
#define RSA_DATA_ADDR_REG            (RSA_REG_BASE_ADDR + 0x24)
#define RSA_CRC32_DEFAULT_REG_OFS    (RSA_REG_BASE_ADDR + 0x30)
#define RSA_CRC32_POLY_REG_OFS       (RSA_REG_BASE_ADDR + 0x34)
#define RSA_CRC32_OUTPUT_REG_OFS     (RSA_REG_BASE_ADDR + 0x38)

// Crypto
#define CRYPTO_CONFREG_SWRST          (1<<0)
#define CRYPTO_CONFREG_CRYPTO_EN      (1<<1)
#define CRYPTO_CONFREG_AES128         (2<<4)
#define CRYPTO_CONFREG_DECRYPT        (1<<8)

#define CRYPTO_PIO_DONE               (1<<0)


// Hash
#define HASH_CONFREG_INITSTATE        (1<<20)
#define HASH_CONFREG_SHA256           (1<<4)
#define HASH_CONFREG_ENABLE           (1<<1)

//RSA
#define RSA_ENABLE                    (1<<0)

#define RSA_KEYWIDTH_256              (0x0<<1)
#define RSA_KEYWIDTH_512              (0x1<<1)
#define RSA_KEYWIDTH_1024             (0x2<<1)
#define RSA_KEYWIDTH_2048             (0x3<<1)
#define RSA_KEYWIDTH_4096             (0x4<<1)

#define RSA_NORMAL_MODE               (0x0<<4)
#define RSA_MODE_CRC_KEY_N            (0x1<<4)
#define RSA_MODE_CRC_KEY_ED           (0x2<<4)


#define RSA_TRANSFER_END              (0x1<<0)
#define RSA_BUSY                      (0x1<<1)

#define HASH_TRANSFER_END             (0x1<<0)


// DDR phy duty configuration register bit
#define DDR_PHY_INCREASE_DUTY       0x40            // P < N
#define DDR_PHY_DECREASE_DUTY       0x00            // P > N
#define DDR_PHY_DUTY_TYPE_MSK       0x40            // bit mask for increase/decrease duty
#define DDR_PHY_CLK_ADJ_EN          0x80

// DDR phy duty calibration control register bit
#define DDR_PHY_CLK_OP_ONGOING      0x10
#define DDR_PHY_CLK_OP_DONE         0x00
#define DDR_PHY_DQS_OP_ONGOING      0x20
#define DDR_PHY_DQS_OP_DONE         0x00



# define DDR_PHY_CAL_CLK_ADJ_OFS    0x000           /* 0x00                     */
# define DDR_PHY_CAL_CMD_ADJ_OFS    0x01C           /* 0x1C                     */
# define DDR_PHY_CAL_CTRL_OFS       0x150           /* 0x54                     */
# define DDR_PHY_CAL_CLK_CFG_OFS    0x154           /* 0x55                     */
# define DDR_PHY_CAL_DQS_CFG_OFS    0x158           /* 0x56                     */
# define DDR_PHY_CAL_DTY_CNT_LB_OFS 0x15C           /* 0x57 duty cnt low byte   */
# define DDR_PHY_CAL_DTY_CNT_HB_OFS 0x160           /* 0x58 duty cnt high byte  */
# define DDR_PHY_CAL_DQS0_ADJ_OFS   0x1D0           /* 0x1D0                    */
# define DDR_PHY_CAL_DQ_ADJ_OFS     0x1D4           /* 0x1D0                    */


# define IDENTIFY_ERR       2
# define MBR_ERR            3
# define PBR_ERR            4
# define READ512_ERR        5
# define READ_ERR           7
# define HPA_ERR            8
# define CHECKSUM_ERR       9

# endif
