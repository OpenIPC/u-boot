// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 HiSilicon (Shanghai) Technologies CO., LIMITED.
 *
 */

#ifndef __CHIP_REGS_H__
#define __CHIP_REGS_H__

#define _HI3516C_V610           0x003516c610LL
#define _HI3516CV610_MASK       0xFFFFFFFFFFLL

/* -------------------------------------------------------------------- */
#define RAM_START_ADRS          0x04022000
#define STACK_TRAINING          0x0402A000

/* -------------------------------------------------------------------- */
#define FMC_REG_BASE            0x10000000

/* -------------------------------------------------------------------- */
#define REG_BASE_SF             0x10290000

/* -------------------------------------------------------------------- */
#define EMMC_REG_BASE           0x10030000
#define SDIO1_REG_BASE          0x10040000

/* -------------------------------------------------------------------- */
#define USB3_CTRL_REG_BASE      0x10300000

/* -------------------------------------------------------------------- */
#define HIUSB_OHCI_BASE         0x10300000

/* -------------------------------------------------------------------- */
#define DDRC0_REG_BASE          0x11330000

/* -------------------------------------------------------------------- */
#define TIMER0_REG_BASE         0x11000000

#define REG_TIMER_RELOAD        0x0
#define REG_TIMER_VALUE         0x4
#define REG_TIMER_CONTROL       0x8

#define CFG_TIMER_CLK           3000000
#define CFG_TIMERBASE           TIMER0_REG_BASE

/* enable timer.32bit, periodic,mask irq, 1 divider. */
#define CFG_TIMER_CTRL          0xC2

/* -------------------------------------------------------------------- */
/* Clock and Reset Generator REG */
/* -------------------------------------------------------------------- */
#define CRG_REG_BASE            0x11010000

#define REG_CRG3634         0x38c8
#define REG_CRG4048	    0x3f40
#define REG_CRG3571	    0x37cc
#define REG_CRG4192	    0x4180

/* -------------------------------------------------------------------- */
/* USB */
/* -------------------------------------------------------------------- */
#define USB2_PHY_BASE_ADDR                  0x17350000

/* USB CRG register offset */
#define USB3_CTRL_CRG           (CRG_REG_BASE + 0x3940)
#define USB2_PHY_CRG            (CRG_REG_BASE + 0x38c0)
#define USB3_PHY_CRG            (CRG_REG_BASE + 0x3944)

/* USB 2.0 CRG Control register offset */
#define REG_USB2_CTRL       REG_CRG3634

/* FMC CRG register offset */
#define REG_FMC_CRG         REG_CRG4048
#define FMC_SOFT_RST_REQ    BIT(0)
#define FMC_CLK_ENABLE      BIT(4)
#define FMC_CLK_SEL_MASK    (0x7 << 12)
#define FMC_CLK_SEL_SHIFT   12
/* SDR/DDR clock */
#define FMC_CLK_24M         0x0
#define FMC_CLK_100M        0x1
#define FMC_CLK_150M        0x2
#define FMC_CLK_200M        0x3
#define FMC_CLK_238M        0x4
#define FMC_CLK_264M        0x5

/* Only DDR clock */
#define FMC_CLK_297M        0x6
#define FMC_CLK_396M        0x7

#define fmc_clk_sel(_clk) \
	(((_clk) << FMC_CLK_SEL_SHIFT) & FMC_CLK_SEL_MASK)
#define get_fmc_clk_type(_reg) \
	(((_reg) & FMC_CLK_SEL_MASK) >> FMC_CLK_SEL_SHIFT)

/* Ethernet CRG register offset */
#define REG_ETH_CRG         REG_CRG3571
#define REG_ETH_MAC_IF      0x8c

/* Uart CRG register offset */
#define REG_UART0_CRG            REG_CRG4192
#define UART_CLK_SEL_MASK (0x3 << 12)
#define UART_CLK_SEL_24M (0x2 << 12)
#define UART_CLK_SEL_50M (0x1 << 12)
#define UART_CLK_SEL_100M (0x0 << 12)

/* SDIO0 CRG register offset */
#define REG_SDIO0_CRG           (CRG_REG_BASE + 0x238)

/* eMMC CRG register offset */
#define REG_EMMC_CRG            (CRG_REG_BASE + 0x1f4)
#define mmc_clk_sel(_clk)       (((_clk) & 0x7) << 24)
#define MMC_CLK_SEL_MASK        (0x7 << 24)
#define get_mmc_clk_type(_reg)      (((_reg) >> 24) & 0x7)

/* -------------------------------------------------------------------- */
/* System Control REG */
/* -------------------------------------------------------------------- */
#define SYS_CTRL_REG_BASE       0x11020000
#define REG_BASE_SCTL           SYS_CTRL_REG_BASE
#define REG_SC_SYSSTAT          0x18
#define spi_input_sle(x)        (((x) >> 16) & 0x1)

/* System Control register offset */
#define REG_SC_CTRL         0x0000
#define sc_ctrl_timer0_clk_sel(_clk)    (((_clk) & 0x1) << 16)
#define TIMER0_CLK_SEL_MASK     (0x1 << 16)
#define TIMER_CLK_3M            0
#define TIMER_CLK_BUS           1
#define SC_CTRL_REMAP_CLEAR     (0x1 << 8)

/* System soft reset register offset */
#define REG_SC_SYSRES           0x0004

/* System Status register offset */
#define REG_SYSSTAT         0x0018
#define SEC_BOOT_MODE       (1 << 31)
#define get_spi_nor_addr_mode(_reg)		(((_reg) >> 11) & 0x1)
#define get_spi_device_type(_reg)		(((_reg) >> 2) & 0x1)
#define get_sys_boot_mode(_reg)			(((_reg) >> 2) & 0x3)
#define BOOT_FROM_SPI				0
#define BOOT_FROM_SPI_NAND			1
#define BOOT_FROM_NAND				2
#define BOOT_FROM_EMMC				3
#define NF_BOOTBW_MASK          (1 << 11)


#define REG_SC_GEN0		0x0130
#define REG_SC_GEN1		0x0134
#define REG_SC_GEN2		0x0138
#define REG_SC_GEN3		0x013C
#define REG_SC_GEN4		0x0140
#define REG_SC_GEN5		0x0144
#define REG_SC_GEN6		0x0148
#define REG_SC_GEN7		0x014c
#define REG_SC_GEN8		0x0150
#define REG_SC_GEN9		0x0154
#define REG_SC_GEN10		0x0158
#define REG_SC_GEN11		0x015c

/********** Communication Register and flag used by bootrom *************/
#define REG_START_FLAG      (SYS_CTRL_REG_BASE + REG_SC_GEN3)

#define REG_SELF_BOOT_FALG		(SYS_CTRL_REG_BASE + 0x14)
#define SELF_BOOT_UART_NO_NUDE		0x01
#define SELF_BOOT_UART_NUDE		0x10

#define START_MAGIC         0x444f574e
#define SELF_BOOT_TYPE_USBDEV           0x2  /* debug */

/* -------------------------------------------------------------------- */
/* Peripheral Control REG */
/* -------------------------------------------------------------------- */
#define MISC_REG_BASE           0x11020000

#define MISC_CTRL17                     0x0044
#define MISC_CTRL18         0x48
#define MISC_CTRL7          0x001C
#define MISC_CTRL8          0x0020
#define MISC_CTRL9          0x0024

#define EMMC_ISO_EN         (0x1 << 16)
#define RG_EMMC_LHEN_IN         (0x3f << 17)

/* USB 2.0 MISC Control register offset */
#define REG_USB2_CTRL0          MISC_CTRL7
/* Open it as you need #define REG_USB2_CTRL1  MISC_CTRL9 */

/* FEPHY Control register offset */
#define REG_FEPHY_CTRL0               MISC_CTRL8
#define REG_FEPHY_CTRL1               MISC_CTRL9

/* -------------------------------------------------------------------- */
#define IO_CONFIG_REG_BASE      0x12050000

/* -------------------------------------------------------------------- */
#define UART0_REG_BASE          0x11040000
#define UART1_REG_BASE          0x11041000
#define UART2_REG_BASE          0x11042000

/* -------------------------------------------------------------------- */
#define GPIO0_REG_BASE          0x120B0000
#define GPIO1_REG_BASE          0x120B1000
#define GPIO2_REG_BASE          0x120B2000
#define GPIO3_REG_BASE          0x120B3000
#define GPIO4_REG_BASE          0x120B4000
#define GPIO5_REG_BASE          0x120B5000
#define GPIO6_REG_BASE          0x120B6000
#define GPIO7_REG_BASE          0x120B7000
#define GPIO8_REG_BASE          0x120B8000
#define GPIO9_REG_BASE          0x120B9000

#define FMC_MEM_BASE            0x0F000000
#define FMC_TEXT_ADRS           FMC_MEM_BASE
#define DDR_MEM_BASE            0x40000000
#define HW_DEC_INTR             86
/*-----------------------------------------------------------------------
 * EMMC / SD
 * ----------------------------------------------------------------------*/
/* SDIO0 REG */
#define SDIO0_BASE_REG          0x10030000
#define SDIO1_BASE_REG          0x10040000

/* EMMC REG */
#define EMMC_BASE_REG           0x10030000

#define REG_BASE_PERI_CTRL              REG_BASE_SCTL
#define REG_BASE_IO_CONFIG              IO_CONFIG_REG_BASE

#define MMC_IOMUX_START_ADDR            0xF8
#define MMC_IOMUX_END_ADDR              0x13C
#define MMC_IOMUX_CTRL_MASK             (1<<0 | 1<<1)
#define MMC_IOMUX_CTRL                  (1<<1)

#define SYSCNT_REG_BASE     0x12050000
#define SYSCNT_ENABLE_REG       0x0
#define SYSCNT_FREQ_REG         0x20
#define SYSCNT_FREQ         50000000

#define REG_BASE_SYSCNT SYSCNT_REG_BASE
#define CNTCR 0x0
#define CNTFID0 0x20

/* ---------------------------------------------------------*/
#define NUM_0					0
#define NUM_1					1
#define NUM_2					2
#define NUM_3					3
#define NUM_4					4
#define NUM_5					5
#define NUM_6					6
#define NUM_7					7
#define NUM_8					8
#define NUM_9					9

#define GZIP_REG_BASE 0x170F0000
#define ECO_REG0_0  0xf00

#define NFC_STATE_REG (SYS_CTRL_REG_BASE + ECO_REG0_0)
#define NFC_STATE_BIT_OFF	0
#define NFC_STATA_MASK		0x1
#define NFC_1_8_STATA		1
#define NFC_3_3_STATA		0

#endif /* End of __CHIP_REGS_H__ */

