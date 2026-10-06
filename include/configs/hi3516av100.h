#ifndef __HI3516AV100_H
#define __HI3516AV100_H

#include <linux/sizes.h>
#include <asm/arch/platform.h>

#define CONFIG_HI3516A
#define CONFIG_HI3516AV100

#define SF_TEXT_ADRS                (SFC_MEM_BASE)
#define SNAND_TEXT_ADRS             (SPI_NAND_MEM_BASE)
#define NAND_TEXT_ADRS              (NAND_MEM_BASE)
#define MEM_BASE_DDR                (DDR_MEM_BASE)

#define CONFIG_SYS_CACHELINE_SIZE   64

/* Physical Memory Map */
#define CONFIG_SYS_TEXT_BASE        0x80800000

#define CONFIG_NR_DRAM_BANKS        1
#define PHYS_SDRAM_1                0x80000000
#define PHYS_SDRAM_1_SIZE           0x40000000
#define CONFIG_SYS_SDRAM_BASE       PHYS_SDRAM_1

#define CONFIG_SYS_INIT_RAM_ADDR    PHYS_SDRAM_1
#define CONFIG_SYS_INIT_RAM_SIZE    0x100000
#define CONFIG_SYS_INIT_SP_ADDR     (CONFIG_SYS_TEXT_BASE - 0x100000)

#define CONFIG_SYS_LOAD_ADDR        0x82000000
#define CONFIG_SYS_GBL_DATA_SIZE    128
#define CONFIG_STACKSIZE            (128 * 1024)

#define CFG_BOOT_PARAMS             (PHYS_SDRAM_1 + 0x100)

/* Timer */
#define CFG_TIMERBASE               TIMER0_REG_BASE
#define CFG_TIMER_CTRL              0xCA
#define CONFIG_SYS_TIMER_RATE       195312
#define CONFIG_SYS_TIMER_COUNTER    (CFG_TIMERBASE + REG_TIMER_VALUE)
#define CONFIG_SYS_TIMER_COUNTS_DOWN

/* PL011 Serial Configuration */
#define CONFIG_PL011_CLOCK          50000000
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

/* Environment */
#define CONFIG_ENV_OFFSET           0x40000
#define CONFIG_ENV_SIZE             0x10000
#ifdef CONFIG_HISFC350_SPI_NOR
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_SECT_SIZE        0x10000
#endif

#define CONFIG_SYS_FAULT_ECHO_LINK_DOWN

/* HIGMAC gigabit ethernet */
#ifdef CONFIG_NET_HIGMACV300
#define CONFIG_GMAC_NUMS                    1
#define HIGMAC0_IOBASE                      0x10090000
#define CONFIG_HIGMAC_PHY0_ADDR             1
#define CONFIG_HIGMAC_PHY0_INTERFACE_MODE   2
#define CONFIG_HIGMAC_PHY1_ADDR             2
#define CONFIG_HIGMAC_PHY1_INTERFACE_MODE   2
#define CONFIG_HIGMAC_DESC_NUM              64
#endif

/* USB OHCI */
#ifdef CONFIG_USB_OHCI_NEW
#define CONFIG_SYS_USB_OHCI_REGS_BASE   0x100a0000
#define CONFIG_SYS_USB_OHCI_SLOT_NAME   "hiusb-ohci"
#endif

#ifdef CONFIG_MMC
#define CONFIG_HIMCI_HI3516a
#define CONFIG_HIMCI_V100
#define CONFIG_GENERIC_MMC
#define CONFIG_CMD_MMC
#define REG_BASE_MCI                0x10020000
#endif

#include "openipc-common.h"

#endif /* __HI3516AV100_H */
