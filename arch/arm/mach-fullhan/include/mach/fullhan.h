/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Fullhan camera SoCs: the register map, from the vendor U-Boots 2010.06
 * and kernels of the FH8632, FH885x V100, FH885xV200 SDKs and the FH8626V100
 * and FH8652 cameras' flash.
 *
 * Every generation keeps one map: DDR at 0xA0000000, DesignWare
 * peripherals, and Fullhan's PMU (clocks, resets, pad control) at
 * 0xF0000000. They differ in the PMU's divider and reset fields, the flash
 * controller's chip select, and the Ethernet PHY.
 */

#ifndef __MACH_FULLHAN_H
#define __MACH_FULLHAN_H

#include <linux/bitops.h>

#define FH_DDR_BASE		0xA0000000

#define FH_PMU_BASE		0xF0000000
#define FH_GPIO0_BASE		0xF0300000	/* GPIO 0-31, DW APB GPIO */
#define FH_GPIO1_BASE		0xF4000000	/* GPIO 32-63 */
#define FH_UART0_BASE		0xF0700000	/* 16550, 32-bit registers */
#define FH_TIMER_BASE		0xF0C00000	/* DW APB timers */
#define FH_WDT_BASE		0xF0D00000	/* DW APB watchdog */
#define FH_GMAC_BASE		0xE0600000	/* Synopsys GMAC, DMA at +0x1000 */
#define FH_SDC0_BASE		0xE2000000	/* DW MMC */

#if defined(CONFIG_FH8626V100)
#define FH_SPI0_BASE		0xF0E00000
#else
#define FH_SPI0_BASE		0xF0500000	/* DW SSI */
#endif

/* PMU */
#define FH_PMU_CHIP_ID		(FH_PMU_BASE + 0x000)
#define FH_PMU_PACKAGE_ID	(FH_PMU_BASE + 0x004)
#define FH_PMU_SYS_CTRL		(FH_PMU_BASE + 0x00C)
#define FH_PMU_CLK_GATE		(FH_PMU_BASE + 0x01C)	/* 1 = gated */
#define FH_PMU_CLK_GATE1	(FH_PMU_BASE + 0x020)	/* CLK_SEL before V200 */
#define FH_PMU_CLK_DIV2		(FH_PMU_BASE + 0x02C)
#define FH_PMU_CLK_DIV3		(FH_PMU_BASE + 0x030)
#define FH_PMU_CLK_DIV4		(FH_PMU_BASE + 0x034)
#define FH_PMU_CLK_DIV5		(FH_PMU_BASE + 0x038)
#define FH_PMU_CLK_DIV6		(FH_PMU_BASE + 0x03C)
#define FH_PMU_PAD(n)		(FH_PMU_BASE + 0x080 + 4 * (n))

/*
 * Block resets: write the register with the block's bit clear, and it
 * reads back all ones once the block is out of reset. Clearing bit 31 of
 * the main one resets the whole chip.
 */
#if defined(CONFIG_FH8626V100)
#define FH_PMU_SWRST_MAIN	(FH_PMU_BASE + 0x04C)
#define FH_PMU_SWRST_AHB	(FH_PMU_BASE + 0x054)
#define FH_SWRST_GMAC		BIT(10)
#define FH_SWRST_SDC0		BIT(12)
#else
#define FH_PMU_SWRST_MAIN	(FH_PMU_BASE + 0x040)
#define FH_PMU_SWRST_AHB	(FH_PMU_BASE + 0x048)
#define FH_SWRST_GMAC		BIT(17)
#define FH_SWRST_SDC0		BIT(2)
#endif
#define FH_SWRST_CHIP		BIT(31)

/* Gates, the same bits on every part */
#define FH_GATE_SPI0		BIT(7)
#define FH_GATE_SDC0		BIT(9)
#define FH_GATE_UART0		BIT(13)
#define FH_GATE_TMR0		BIT(17)

/* SYS_CTRL: the MAC's speed and interface */
#define FH_SYS_CTRL_MAC_100M	BIT(24)

/* Timer channel 0, counting down at 1 MHz from the divider set up here */
#define FH_TIMER0_LOAD		(FH_TIMER_BASE + 0x00)
#define FH_TIMER0_VALUE		(FH_TIMER_BASE + 0x04)
#define FH_TIMER0_CTRL		(FH_TIMER_BASE + 0x08)
#define FH_TIMER_CTRL_EN	BIT(0)
#define FH_TIMER_CTRL_INTMASK	BIT(2)
#define FH_TIMER_RATE		1000000

/* Watchdog */
#define FH_WDT_CR		(FH_WDT_BASE + 0x00)
#define FH_WDT_TORR		(FH_WDT_BASE + 0x04)
#define FH_WDT_CRR		(FH_WDT_BASE + 0x0C)
#define FH_WDT_KICK		0x76

/* DW APB GPIO, port A of each block */
#define FH_GPIO_DR		0x00
#define FH_GPIO_DDR		0x04
#define FH_GPIO_EXT		0x50

/*
 * Per generation: the UART and flash controller input clocks, the flash's
 * chip-select GPIO, and the SPIC configuration the vendor drivers set.
 */
#if defined(CONFIG_FH885XV200) || defined(CONFIG_FH865X)
#define FH_UART_CLK		16666667	/* 100 MHz / 6 */
#define FH_SPI0_CLK		200000000
#define FH_SPI0_CS_GPIO		6
#define FH_SPI0_SPIC_CCFGR	(BIT(13) | BIT(15))
#elif defined(CONFIG_FH885XV100) || defined(CONFIG_FH8626V100)
#define FH_UART_CLK		16666667
#define FH_SPI0_CLK		100000000
#define FH_SPI0_CS_GPIO		54
#define FH_SPI0_SPIC_CCFGR	BIT(13)
#elif defined(CONFIG_FH8632)
#define FH_UART_CLK		30000000	/* PLL1 600 MHz / 10 / 2 */
#define FH_SPI0_CLK		50000000
#define FH_SPI0_CS_GPIO		54
#endif

#ifndef __ASSEMBLY__
void fh_gpio_output(unsigned int gpio, int value);
void fh_pmu_update(unsigned long reg, u32 mask, u32 val);
void fh_pmu_reset(unsigned long reg, u32 bits);
void fh_spi0_setup(void);
#endif

#endif
