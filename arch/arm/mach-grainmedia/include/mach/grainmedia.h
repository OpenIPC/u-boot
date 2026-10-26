/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Grain Media GM8135S / GM8136 / GM8136S: the register map, from the
 * GM8136 SDK's U-Boot 2013.01 (board/gm/gm8136), nsboot and Linux 3.3
 * (arch/arm/mach-GM/platform-GM8136).
 *
 * DDR at 0 once nsboot has remapped it, Faraday peripherals from
 * 0x90000000, and Faraday's FTPMU010 for clocks, gates and pin mux.
 */

#ifndef __MACH_GRAINMEDIA_H
#define __MACH_GRAINMEDIA_H

#include <linux/bitops.h>

#define GM_DDR_BASE		0x00000000

#define GM_UART0_BASE		0x90700000	/* FTUART010, a 16550 */
#define GM_PMU_BASE		0x90C00000	/* FTPMU010 */
#define GM_TIMER_BASE		0x90D00000	/* FTTMR010 */
#define GM_WDT_BASE		0x90E00000	/* FTWDT010 */
#define GM_GPIO0_BASE		0x91000000	/* FTGPIO010, GPIO 0-31 */
#define GM_GPIO1_BASE		0x91900000	/* GPIO 32-63 */
#define GM_SPI020_BASE		0x92300000	/* FTSPI020 flash controller */
#define GM_GMAC_BASE		0x92500000	/* FTGMAC100 */
#define GM_SDC0_BASE		0x92800000	/* FTSDC021, an SDHCI */
#define GM_DDRC_BASE		0x9A100000	/* FTDDRC010 */

/* PMU */
#define GM_PMU_ID		(GM_PMU_BASE + 0x00)	/* 0x081361xx */
#define GM_PMU_STRAP		(GM_PMU_BASE + 0x04)
#define GM_PMU_CLK_SEL		(GM_PMU_BASE + 0x28)
#define GM_PMU_PLL1		(GM_PMU_BASE + 0x30)
#define GM_PMU_PLL23		(GM_PMU_BASE + 0x34)
#define GM_PMU_DRIVE0		(GM_PMU_BASE + 0x40)
#define GM_PMU_DRIVE1		(GM_PMU_BASE + 0x44)
#define GM_PMU_PINMUX2		(GM_PMU_BASE + 0x58)
#define GM_PMU_PINMUX3		(GM_PMU_BASE + 0x5C)
#define GM_PMU_CLK_DIV		(GM_PMU_BASE + 0x70)
#define GM_PMU_MISC		(GM_PMU_BASE + 0x7C)
#define GM_PMU_GATE_PHY		(GM_PMU_BASE + 0xAC)	/* 1 = gated */
#define GM_PMU_GATE_AXI		(GM_PMU_BASE + 0xB0)
#define GM_PMU_GATE_AHB		(GM_PMU_BASE + 0xB4)
#define GM_PMU_GATE_APB		(GM_PMU_BASE + 0xB8)

#define GM_MISC_NO_AUTO_WDT	BIT(4)

#define GM_AHB_GMAC		BIT(11)
#define GM_AHB_SDC0		BIT(14)
#define GM_AHB_SPI020		BIT(15)
#define GM_APB_UART0		BIT(0)
#define GM_APB_WDT		BIT(10)
#define GM_APB_TIMER		BIT(11)
#define GM_APB_GPIO1		BIT(17)

/* Package: PMU ID bits 6:2, as the kernel's pmu_get_chip() reads them */
#define GM_PACKAGE(id)		(((id) >> 2) & 0x1f)
#define GM_PACKAGE_8136		0x00
#define GM_PACKAGE_8136S(p)	((p) == 0x14 || (p) == 0x1c)

/* The DRAM size code nsboot's table leaves in the DDR controller */
#define GM_DDRC_SIZE		(GM_DDRC_BASE + 0x10)

/* Timer channel 0 and the control register of all three */
#define GM_TIMER0_COUNTER	(GM_TIMER_BASE + 0x00)
#define GM_TIMER0_LOAD		(GM_TIMER_BASE + 0x04)
#define GM_TIMER0_MATCH1	(GM_TIMER_BASE + 0x08)
#define GM_TIMER0_MATCH2	(GM_TIMER_BASE + 0x0C)
#define GM_TIMER_CR		(GM_TIMER_BASE + 0x30)
#define GM_TIMER_CR_TM1_EN	BIT(0)

/* Watchdog */
#define GM_WDT_LOAD		(GM_WDT_BASE + 0x04)
#define GM_WDT_RESTART		(GM_WDT_BASE + 0x08)
#define GM_WDT_CR		(GM_WDT_BASE + 0x0C)
#define GM_WDT_KICK		0x5AB9
#define GM_WDT_CR_EN		BIT(0)
#define GM_WDT_CR_RST		BIT(1)

/* GPIO */
#define GM_GPIO_DOUT		0x00
#define GM_GPIO_DIR		0x08	/* 1 = output */

#define GM_UART_CLK		25000000	/* PLL2 / 2 / (div + 1), set by nsboot */

#ifndef __ASSEMBLY__
unsigned long gm_pll1_hz(void);
unsigned long gm_pll2_hz(void);
unsigned long gm_ahb_hz(void);
unsigned long gm_apb_hz(void);
#endif

#endif
