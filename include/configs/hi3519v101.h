#ifndef __HI3519V101_H
#define __HI3519V101_H

#include <linux/sizes.h>
#include <asm/arch/platform.h>

#define CONFIG_HI3519V101
#define CONFIG_ARCH_HI3519V101

#define FMC_TEXT_ADRS               (FMC_MEM_BASE)
#define MEM_BASE_DDR                (DDR_MEM_BASE)

#define CONFIG_SYS_CACHELINE_SIZE   64

/* Physical Memory Map */
#define CONFIG_SYS_TEXT_BASE        0x88400000
#define CONFIG_SYS_TEXT_BASE_ORI    0x88300000

#define CONFIG_NR_DRAM_BANKS        1
#define PHYS_SDRAM_1                0x80000000
#define PHYS_SDRAM_1_SIZE           0x20000000
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
#define CFG_TIMER_CTRL              0xC2
#define CONFIG_SYS_TIMER_RATE       3000000
#define CONFIG_SYS_TIMER_COUNTER    (CFG_TIMERBASE + REG_TIMER_VALUE)
#define CONFIG_SYS_TIMER_COUNTS_DOWN

/* PL011 Serial Configuration */
#define CONFIG_PL011_CLOCK          24000000
#define CONFIG_PL01x_PORTS          {(void *)UART0_REG_BASE}
#define CONFIG_CUR_UART_BASE        UART0_REG_BASE

/* FMC SPI Flash Controller */
#ifdef CONFIG_FMC
#define CONFIG_FMC_REG_BASE         FMC_REG_BASE
#define CONFIG_FMC_BUFFER_BASE      FMC_MEM_BASE
#define CONFIG_FMC_MAX_CS_NUM       1
#endif
#ifdef CONFIG_FMC_SPI_NOR
#define CONFIG_SPI_NOR_MAX_CHIP_NUM 1
#define CONFIG_SPI_NOR_QUIET_TEST
#endif
#ifdef CONFIG_FMC_SPI_NAND
#define CONFIG_SPI_NAND_MAX_CHIP_NUM    1
#define CONFIG_SYS_MAX_NAND_DEVICE  CONFIG_SPI_NAND_MAX_CHIP_NUM
#define CONFIG_SYS_NAND_MAX_CHIPS   CONFIG_SPI_NAND_MAX_CHIP_NUM
#define CONFIG_SYS_NAND_BASE        FMC_MEM_BASE
#endif

/* Environment */
#define CONFIG_ENV_OFFSET           0x40000
#define CONFIG_ENV_SIZE             0x10000
#ifdef CONFIG_FMC_SPI_NOR
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_SECT_SIZE        0x10000
#endif

#define CONFIG_SYS_FAULT_ECHO_LINK_DOWN

/* HIGMAC gigabit ethernet */
#ifdef CONFIG_NET_HIGMACV300
#define CONFIG_GMAC_NUMS                    1
#define HIGMAC0_IOBASE                      0x10050000
#define CONFIG_HIGMAC_PHY0_ADDR             1
#define CONFIG_HIGMAC_PHY0_INTERFACE_MODE   2
#define CONFIG_HIGMAC_PHY1_ADDR             2
#define CONFIG_HIGMAC_PHY1_INTERFACE_MODE   2
#define CONFIG_HIGMAC_DESC_4_WORD
#endif

#ifdef CONFIG_MMC
#define CONFIG_EMMC_SUPPORT
#define CONFIG_HIMCI_V200
#define CONFIG_GENERIC_MMC
#define CONFIG_CMD_MMC
#define CONFIG_MMC_DEVID                    1
#define CONFIG_MMC_BOOT_ADDR                0
#define CONFIG_MMC_POWER_OFF_TIMEOUT        50
#define CONFIG_MMC_POWER_ON_TIMEROUT        50
#define CONFIG_MMC_RESET_LOW_TIMEOUT        10
#define CONFIG_MMC_RESET_HIGH_TIMEROUT      300
#endif

#include "openipc-common.h"

#endif /* __HI3519V101_H */
