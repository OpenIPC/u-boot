#ifndef __HI3516CV200_H
#define __HI3516CV200_H

#include <linux/sizes.h>
#include <asm/arch/platform.h>

#define CONFIG_HI3518EV200

#define FMC_TEXT_ADRS               (FMC_MEM_BASE)
#define MEM_BASE_DDR                (DDR_MEM_BASE)

#define CONFIG_SYS_CACHELINE_SIZE   32

/* Physical Memory Map */
#define CONFIG_SYS_TEXT_BASE        0x80800000
#define CONFIG_SYS_TEXT_BASE_ORI    0x80700000

#define CONFIG_NR_DRAM_BANKS        1
#define PHYS_SDRAM_1                0x80000000
#define PHYS_SDRAM_1_SIZE           0x20000000
#define CONFIG_SYS_SDRAM_BASE       PHYS_SDRAM_1

#define CONFIG_SYS_INIT_RAM_ADDR    0x04010000
#define CONFIG_SYS_INIT_RAM_SIZE    0x4000
#define CONFIG_SYS_INIT_SP_ADDR     0x04014000

#define CONFIG_SYS_LOAD_ADDR        (PHYS_SDRAM_1 + 0x8000)
#define CONFIG_SYS_GBL_DATA_SIZE    128

#define CFG_BOOT_PARAMS             (PHYS_SDRAM_1 + 0x100)

/* Timer */
#define CFG_TIMERBASE               TIMER0_REG_BASE
#define CFG_TIMER_CTRL              0xCA
#define CONFIG_SYS_TIMER_RATE       193359
#define CONFIG_SYS_TIMER_COUNTER    (CFG_TIMERBASE + REG_TIMER_VALUE)
#define CONFIG_SYS_TIMER_COUNTS_DOWN

/* PL011 Serial Configuration */
#define CONFIG_PL011_CLOCK          24000000
#define CONFIG_PL01x_PORTS  \
	{(void *)UART0_REG_BASE, (void *)UART1_REG_BASE, \
	(void *)UART2_REG_BASE}
#define CONFIG_CUR_UART_BASE        UART0_REG_BASE

/* Flash Memory Controller */
#ifdef CONFIG_FMC
#define CONFIG_FMC_REG_BASE         FMC_REG_BASE
#define CONFIG_FMC_BUFFER_BASE      FMC_MEM_BASE
#define CONFIG_FMC_MAX_CS_NUM       2
#endif
#ifdef CONFIG_FMC_SPI_NOR
#define CONFIG_SPI_NOR_MAX_CHIP_NUM 2
#define CONFIG_SPI_NOR_QUIET_TEST
#define CONFIG_CLOSE_SPI_8PIN_4IO
#endif
#ifdef CONFIG_FMC_SPI_NAND
#define CONFIG_SPI_NAND_MAX_CHIP_NUM    1
#define CONFIG_SYS_MAX_NAND_DEVICE  CONFIG_SPI_NAND_MAX_CHIP_NUM
#define CONFIG_SYS_NAND_MAX_CHIPS   CONFIG_SPI_NAND_MAX_CHIP_NUM
#define CONFIG_SYS_NAND_BASE        FMC_MEM_BASE
#endif

#define CONFIG_SYS_FAULT_ECHO_LINK_DOWN

/* HIETH driver */
#ifdef CONFIG_NET_HISFV300
#define HISFV_RESET_PHY_BY_CRG
#define HISFV_MII_MODE              0
#define HISFV_RMII_MODE             1
#define HIETH_MII_RMII_MODE_U       HISFV_RMII_MODE
#define HIETH_MII_RMII_MODE_D       HISFV_RMII_MODE
#define HISFV_PHY_U                 1
#define HISFV_PHY_D                 2
#endif


#ifdef CONFIG_MMC
#define CONFIG_HIMCI_V200
#define CONFIG_GENERIC_MMC
#define CONFIG_CMD_MMC
#define CONFIG_MMC_DEVID		0
#define CONFIG_MMC_BOOT_ADDR		0
#define CONFIG_MMC_POWER_OFF_TIMEOUT	5
#define CONFIG_MMC_POWER_ON_TIMEROUT	40
#define CONFIG_MMC_RESET_LOW_TIMEOUT	10
#define CONFIG_MMC_RESET_HIGH_TIMEROUT	300
#endif

#include "openipc-common.h"

#endif /* __HI3516CV200_H */
