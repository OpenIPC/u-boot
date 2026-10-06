#ifndef __HI3518EV100_H
#define __HI3518EV100_H

#include <linux/sizes.h>
#include <asm/arch/platform.h>

#define CONFIG_HI3518EV100
#define CONFIG_HI3518

#define SF_TEXT_ADRS                (SFC_MEM_BASE)
#define NAND_TEXT_ADRS              (NAND_MEM_BASE)
#define MEM_BASE_DDR                (DDR_MEM_BASE)

#define CONFIG_SYS_CACHELINE_SIZE   32

/* Physical Memory Map */
#define CONFIG_SYS_TEXT_BASE        0x80800000

#define CONFIG_NR_DRAM_BANKS        1
#define PHYS_SDRAM_1                0x80000000
#define PHYS_SDRAM_1_SIZE           0x08000000
#define CONFIG_SYS_SDRAM_BASE       PHYS_SDRAM_1

#define CONFIG_SYS_INIT_RAM_ADDR    PHYS_SDRAM_1
#define CONFIG_SYS_INIT_RAM_SIZE    0x100000
#define CONFIG_SYS_INIT_SP_ADDR     (CONFIG_SYS_TEXT_BASE - 0x100000)

#define CONFIG_SYS_LOAD_ADDR        0x82000000
#define CONFIG_SYS_GBL_DATA_SIZE    128

#define CFG_BOOT_PARAMS             (PHYS_SDRAM_1 + 0x100)
#define CONFIG_STACKSIZE            (128 * 1024)

/* Timer */
#define CFG_TIMERBASE               TIMER0_REG_BASE
#define CFG_TIMER_CTRL              0xCA
#define CONFIG_SYS_TIMER_RATE       390625
#define CONFIG_SYS_TIMER_COUNTER    (CFG_TIMERBASE + REG_TIMER_VALUE)
#define CONFIG_SYS_TIMER_COUNTS_DOWN

/* PL011 Serial Configuration */
#define CONFIG_PL011_CLOCK          3000000
#define CONFIG_PL01x_PORTS          {(void *)UART0_REG_BASE}
#define CONFIG_CUR_UART_BASE        UART0_REG_BASE

/* SFC350 SPI Flash Controller */
#ifdef CONFIG_HISFC350_SPI_NOR
#define CONFIG_HISFC350_REG_BASE_ADDRESS    SFC_REG_BASE
#define CONFIG_HISFC350_BUFFER_BASE_ADDRESS SFC_MEM_BASE
#define CONFIG_HISFC350_PERIPHERY_REGBASE   CRG_REG_BASE
#define CONFIG_HISFC350_CHIP_NUM            1
#define SFC_ADDR_MODE_REG                   0x8c
#define CONFIG_SPI_NOR_MAX_CHIP_NUM         1
#define CONFIG_SPI_NOR_QUIET_TEST
#endif

/* NAND (HINFC301) */
#ifdef CONFIG_NAND_HINFC301
#define CONFIG_SYS_NAND_BASE            NAND_MEM_BASE
#define CONFIG_SYS_MAX_NAND_DEVICE      1
#define CONFIG_SYS_NAND_MAX_CHIPS       2
#define CONFIG_HINFC301_REG_BASE_ADDRESS    NANDC_REG_BASE
#define CONFIG_HINFC301_BUFFER_BASE_ADDRESS NAND_MEM_BASE
#define CONFIG_HINFC301_MAX_CHIP        2
#define CONFIG_HINFC301_HARDWARE_PAGESIZE_ECC
#define CONFIG_HINFC301_W_LATCH         0xa
#define CONFIG_HINFC301_R_LATCH         0xa
#define CONFIG_HINFC301_RW_LATCH        0xa
#endif

/* USB OHCI */
#ifdef CONFIG_USB_OHCI_NEW
#define CONFIG_SYS_USB_OHCI_REGS_BASE   0x100a0000
#define CONFIG_SYS_USB_OHCI_SLOT_NAME   "hiusb-ohci"
#endif

/* Environment */
#define CONFIG_ENV_OFFSET           0x40000
#define CONFIG_ENV_SIZE             0x10000
#ifdef CONFIG_HISFC350_SPI_NOR
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_SECT_SIZE        0x10000
#endif

#define CONFIG_SYS_FAULT_ECHO_LINK_DOWN

/* HIETH driver */
#ifdef CONFIG_NET_HISFV300
#define CONFIG_NET_HISFV300_3518
#define HISFV_MII_MODE              0
#define HISFV_RMII_MODE             1
#define HIETH_MII_RMII_MODE_U       HISFV_MII_MODE
#define HIETH_MII_RMII_MODE_D       HISFV_MII_MODE
#define HISFV_PHY_U                 1
#define HISFV_PHY_D                 2
#endif

#ifdef CONFIG_MMC
#define CONFIG_HIMCI_HI3518
#define CONFIG_HIMCI_V100
#define CONFIG_GENERIC_MMC
#define CONFIG_CMD_MMC
#define REG_BASE_MCI                0x10020000
#endif

#include "openipc-common.h"

#endif /* __HI3518EV100_H */
