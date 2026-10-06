#ifndef __HI3520DV100_H
#define __HI3520DV100_H

#include <linux/sizes.h>
#include <asm/arch/platform.h>

#define CONFIG_HI3520D

#define SF_TEXT_ADRS                (SFC_MEM_BASE)
#define MEM_BASE_DDR                (DDR_MEM_BASE)

#define CONFIG_SYS_CACHELINE_SIZE   32

/* mini-boot reg_info blank zones (two reg_info files, 2400 bytes each) */
#define ENABLE_HI3520D_BLANK
#define ENABLE_HI3515A_BLANK
#define REG_INFO_BLANK_SIZE         2400

/* Physical Memory Map */
#define CONFIG_SYS_TEXT_BASE        0x80800000
#define CONFIG_SYS_TEXT_BASE_ORI    0x80700000

#define CONFIG_NR_DRAM_BANKS        1
#define PHYS_SDRAM_1                0x80000000
#define PHYS_SDRAM_1_SIZE           0x10000000
#define CONFIG_SYS_SDRAM_BASE       PHYS_SDRAM_1

#define CONFIG_SYS_INIT_RAM_ADDR    PHYS_SDRAM_1
#define CONFIG_SYS_INIT_RAM_SIZE    0x100000
#define CONFIG_SYS_INIT_SP_ADDR     (CONFIG_SYS_TEXT_BASE - 0x100000)

#define CONFIG_SYS_LOAD_ADDR        0x82000000
#define CONFIG_SYS_GBL_DATA_SIZE    128
#define CONFIG_STACKSIZE            (128 * 1024)

#define CFG_BOOT_PARAMS             (PHYS_SDRAM_1 + 0x100)

/* bus clock from PLL registers (runtime) */
#define HW_REG(a) (*(volatile unsigned long *)(a))
#define A9_AXI_SCALE_REG            0x20030028
#define get_bus_clk() ({ \
	unsigned long fbdiv, pstdiv1, pstdiv2, refdiv; \
	unsigned long tmp_reg, foutvco, busclk; \
	tmp_reg = HW_REG(REG_CRG0_OFFSET); \
	pstdiv1 = (tmp_reg >> 24) & 0x7; \
	pstdiv2 = (tmp_reg >> 27) & 0x7; \
	tmp_reg = HW_REG(REG_CRG1_OFFSET); \
	refdiv = (tmp_reg >> 12) & 0x3f; \
	fbdiv = tmp_reg & 0xfff; \
	foutvco = 24000000 / refdiv; \
	foutvco *= fbdiv; \
	tmp_reg = HW_REG(A9_AXI_SCALE_REG); \
	if ((tmp_reg & 0xc) == 0xc) \
		busclk = foutvco / 2; \
	else \
		busclk = foutvco / 4; \
	busclk; \
})

/* Timer */
#define CFG_TIMERBASE               TIMER0_REG_BASE
#define CFG_TIMER_CTRL              0xCA
#define CONFIG_SYS_TIMER_RATE       (get_bus_clk() / 4 / 256)
#define CONFIG_SYS_TIMER_COUNTER    (CFG_TIMERBASE + REG_TIMER_VALUE)
#define CONFIG_SYS_TIMER_COUNTS_DOWN

/* PL011 Serial Configuration */
#define CONFIG_PL011_CLOCK          (get_bus_clk() / 4)
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

/* HIETH driver */
#ifdef CONFIG_NET_HISFV300
#define CONFIG_NET_HISFV300_HI3520D
#define HISFV_MII_MODE              0
#define HISFV_RMII_MODE             1
#define HIETH_MII_RMII_MODE_U       HISFV_MII_MODE
#define HIETH_MII_RMII_MODE_D       HISFV_MII_MODE
#define HISFV_PHY_U                 1
#define HISFV_PHY_D                 0
#endif

/* USB OHCI */
#ifdef CONFIG_USB_OHCI_NEW
#define CONFIG_SYS_USB_OHCI_REGS_BASE   0x100a0000
#define CONFIG_SYS_USB_OHCI_SLOT_NAME   "hiusb-ohci"
#endif

#include "openipc-common.h"

#endif /* __HI3520DV100_H */
