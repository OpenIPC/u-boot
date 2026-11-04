#ifndef __TOP_CTRL_H
#define __TOP_CTRL_H

#include "board.h"
#define TOP_CTRL_REG_BASE_ADDR		REG_TOP_BASE

#define TOP_CTRL_BOOTSRC            (TOP_CTRL_REG_BASE_ADDR + 0x00)

#define TOP_CTRL_PIN_MUX_CTRL_REG0  (TOP_CTRL_REG_BASE_ADDR + 0x20)
#define TOP_CTRL_PIN_MUX_CTRL_REG1  (TOP_CTRL_REG_BASE_ADDR + 0x24)

#define TOP_CTRL_PIN_MUX_CTRL_REG2  (TOP_CTRL_REG_BASE_ADDR + 0x08)
#define TOP_CTRL_PIN_MUX_CTRL_REG4  (TOP_CTRL_REG_BASE_ADDR + 0x10)
#define TOP_CTRL_PIN_MUX_CTRL_REG5  (TOP_CTRL_REG_BASE_ADDR + 0x14)
#define TOP_CTRL_PIN_MUX_CTRL_REG8  (TOP_CTRL_REG_BASE_ADDR + 0x20)
#define TOP_CTRL_PIN_MUX_CTRL_REG9  (TOP_CTRL_REG_BASE_ADDR + 0x24)
#define TOP_CTRL_PIN_MUX_CTRL_REG6  (TOP_CTRL_REG_BASE_ADDR + 0x44)

#define TOP_CTRL_ETH0_MUX_REG0		(TOP_CTRL_REG_BASE_ADDR + 0x70)
#define TOP_CTRL_ETH0_MUX_REG1		(TOP_CTRL_REG_BASE_ADDR + 0x74)
#define TOP_CTRL_ETH_MUX_SEL		(TOP_CTRL_REG_BASE_ADDR + 0xC8)



#define TOP_CTRL_CGPIO_MUX_REG0     (TOP_CTRL_REG_BASE_ADDR + 0xA0)
#define TOP_CTRL_CGPIO_MUX_REG1     (TOP_CTRL_REG_BASE_ADDR + 0xA4)
#define TOP_CTRL_PGPIO_MUX_REG0     (TOP_CTRL_REG_BASE_ADDR + 0xA8)
#define TOP_CTRL_PGPIO_MUX_REG1     (TOP_CTRL_REG_BASE_ADDR + 0xAC)
#define TOP_CTRL_SGPIO_MUX_REG0     (TOP_CTRL_REG_BASE_ADDR + 0xB0)
#define TOP_CTRL_LGPIO_MUX_REG0     (TOP_CTRL_REG_BASE_ADDR + 0xB8)
#define TOP_CTRL_DGPIO_MUX_REG0     (TOP_CTRL_REG_BASE_ADDR + 0xD0)
#define TOP_CTRL_VERSION_REG        (TOP_CTRL_REG_BASE_ADDR + 0xF0)


#define TOP_CTRL_SRAM_RESET_REG0    (TOP_CTRL_REG_BASE_ADDR + 0x1000)

#define TOP_CTRL_BS_SPI_NAND_2K_PAGE    (1<<3)
#define TOP_CTRL_BS_SPI_NAND_4K_PAGE    (0)

#define TOP_BOOTSRC_MSK              0x07

#define TOP_GETROMVER()         INREG32(0xE0007FFC)
//
// ROM code version number
//
#define TOP_ROMVER_A            0x00000103
#define TOP_ROMVER_B            0x00000106

//#define PINMUX_NAND_ACT             (1<<0)
//#define PINMUX_SPI_ACT              (1<<1)
//#define PINMUX_NAND_EXIST           (1<<12)
//#define PINMUX_SPI_EXIST            (1<<13)
#define PINMUX_SDIO_EXIST           (1<<14)
#define PINMUX_EXTROM_EXIST         (1<<16)
#define PINMUX_SDIO3_EXIST          (1<<17)
#define PINMUX_SDIO3_MUX_1          (0)
#define PINMUX_ICE_EXIST            (1<<20)
#define PINMUX_SDIO3_MUX_2          (1<<24)
#define PINMUX_SDIO3_4BITS          (0)
#define PINMUX_SDIO3_8BITS          (1<<31)
#define PINMUX_ROM_SRC              (1<<28)

//need to check (0xF0010018)
#define PINMUX_ETH_RMII             (1<<12)//((1<<16) | (1<<8))  SP clk need ??
#define PINMUX_ETH_REFCLK_INPUT     (1<<15)                    //RMII refclk input mode

#define PINMUX_ETH_EXTPHY_CLK_EN    (1<<28)


//Shut down SRAM SDIO1 mask
#define TOP_SRAM_SD_SDIO1           0x20000000
//Shut down SRAM RSA mask
#define TOP_SRAM_SD_RSA             0x02000000

// For storage pinmux
#define TOP_REG0_SDIO_PINMUX        (PINMUX_SDIO_EXIST)
#define TOP_REG0_SDIO3_PINMUX       (PINMUX_SDIO3_EXIST | PINMUX_SDIO3_MUX_1)
#define TOP_REG0_EMMC_PINMUX        (PINMUX_SDIO3_EXIST )
//#define TOP_REG1_NAND_PINMUX        (PINMUX_NAND_EXIST)
#define TOP_REG0_SPI_PINMUX         ((2<<28)|(2<<24)|(2<<20))
#define TOP_REG0_SPI_MASK           ((7<<28)|(7<<24)|(7<<20))
#define TOP_REG0_BMC_PINMUX         (PINMUX_EXTROM_EXIST)
#define TOP_REG0_ICE_PINMUX         (PINMUX_ICE_EXIST)
#define TOP_REG1_SPI_PINMUX         ((2<<12)|(2<<8)|(2<<4)|(2<<0))
#define TOP_REG1_SPI_MASK           ((7<<12)|(7<<8)|(7<<4)|(7<<0))
#if 0
#if ((USE_FPGA_EMULATION == 1) || (USE_BMC_EMULATION == 1))
#define TOP_CGPIO0_SDIO_PINMUX      (0xFFFFFFFF & ~((0x3F<<11)))
#define TOP_CGPIO0_SDIO3_PINMUX     (0xFFFFFFFF & ~((0xF<<28)|(0xF<<18)))
#define TOP_CGPIO1_SDIO3_PINMUX     (0)
#define TOP_CGPIO0_EMMC_PINMUX      (0xFFFFFFFF & ~((0xFF<<0)|(1<<8)|(1<<9)|(1<<10)))
#define TOP_CGPIO0_EMMC_4BIT_PINMUX      (0xFFFFFFFF & ~((0xF<<0)|(1<<8)|(1<<9)|(1<<10)|(0xF<<18)))
#define TOP_CGPIO0_NAND_PINMUX      (0xFFFFFFFF & ~((0xFFFF<<0)|(0xF<<18)))
//#define TOP_CGPIO0_SPI_PINMUX       (0xFFFFFFFF & ~((0xF<<0)|(1<<8)|(1<<10)))
#define TOP_SGPIO0_BMC_PINMUX       (0xFFFFFFFF & ~((1<<4)|(1<<6)|(1<<7)|(1<<8)))
#else
#define TOP_CGPIO0_SDIO_PINMUX      (0xFFFFFFFF & ~(0x3F << 11))
#define TOP_CGPIO0_SDIO3_PINMUX     (0xFFFFFFFF & ~(0xF<<28))
#define TOP_CGPIO1_SDIO3_PINMUX     (0)
#define TOP_CGPIO0_EMMC_PINMUX      (0xFFFFFFFF & ~((0xFF<<0)|(1<<8)|(1<<9)|(1<<10)))
#define TOP_CGPIO0_EMMC_4BIT_PINMUX      (0xFFFFFFFF & ~((0xF<<0)|(1<<8)|(1<<9)|(1<<10)))
#define TOP_CGPIO0_NAND_PINMUX      (0xFFFFFFFF & ~((0xFFFF<<0)))
//#define TOP_CGPIO0_SPI_PINMUX       (0xFFFFFFFF & ~((0xF<<0)|(1<<8)|(1<<10)))
#endif
#endif
//Below need to check again
#define TOP_LGPIO0_ETH_MII_PINMUX   (0xFFFFFFFF & ~(0xFF7FF))
#define TOP_LGPIO0_ETH_RMII_PINMUX  (0xFFFFFFFF & ~(0x7FF))
#define TOP_LGPIO0_ETH_EMBED_PINMUX   (0xFFFFFFFF & ~((0x1<<19)|(0x3<<9)))
#define TOP_PGPIO0_ETH_PINMUX       (0xFFFFFFFF & ~(1<<22))

// For eMMC configuration
#define TOP_REG0_EMMC_4BITS         (0)
#define TOP_REG0_EMMC_8BITS         (1 << 7)
#define TOP_REG0_EMMC_BUSWIDTH_MSK  (1 << 7)
#define TOP_REG0_EMMC_BOOT_SDR      (0)
#define TOP_REG0_EMMC_BOOT_DDR      (1 << 8)
#define TOP_REG0_EMMC_BOOTMODE_MSK  (1 << 8)
#define TOP_REG0_EMMC_DDR_ODD_1ST   (0)
#define TOP_REG0_EMMC_DDR_EVEN_1ST  (1 << 9)
#define TOP_REG0_EMMC_DDREDGE_MSK   (1 << 9)

//Below need to check again
// For ethernet configuration
#define TOP_REG0_ETH_PINMUX_MSK            (0x3 << 3)
#define TOP_REG0_ETH_PINMUX_RMII_OUTPUT    (0 << 3)
#define TOP_REG0_ETH_PINMUX_RMII_INPUT     (1 << 3)
#define TOP_REG0_ETH_PINMUX_RGMII          (2 << 3)
#define TOP_REG0_ETH_PINMUX_EMB            (3 << 3)

// ETH LED
#define TOP_REG6_ETH_LED_MSK               (0xFF0 )
#define TOP_REG6_ETH_LED_2                 (2 << 13)
#define TOP_REG6_ETH_LED_1                 (0x55 << 4)
#define TOP_REG6_ETH_LED_DISABLE           (0 << 13)
// WDT
#define TOP_REG0_WDT_PINMUX_MSK     (0x01 <<12)


#define TOP_DSP_IOP_CH_EN           (1 << 17)

// For UART pinmux
#define UART0_SWITCH                (0x01 << 0)

// For USB pinmux
#define TOP_USB_VBUSI               (0x03)          // Bit2: device mode, Bit1: VBUSI

//#define selectUART()            SETREG32(TOP_CTRL_PIN_MUX_CTRL_REG9, UART0_SWITCH)
#if 0
#if ((USE_FPGA_EMULATION == 1) || (USE_BMC_EMULATION == 1))
#define selectCardType(type)    OUTREG32(TOP_CTRL_PIN_MUX_CTRL_REG1, (INREG32(TOP_CTRL_PIN_MUX_CTRL_REG1)&(TOP_REG0_BMC_PINMUX|TOP_REG0_ICE_PINMUX))|(type))
#else
#define selectCardType(type)    OUTREG32(TOP_CTRL_PIN_MUX_CTRL_REG1, (INREG32(TOP_CTRL_PIN_MUX_CTRL_REG1)&(TOP_REG0_BMC_PINMUX|TOP_REG0_ICE_PINMUX))|(type))
#endif
#endif

#define selectUART0()			CLRREG32(TOP_CTRL_PIN_MUX_CTRL_REG1, ((0x3 << 20) | (0x3 << 24)));\
								SETREG32(TOP_CTRL_PIN_MUX_CTRL_REG1, ((0x1 << 20) | (0x1 << 24)));

#endif
