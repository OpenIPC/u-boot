/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Grain Media GM8126: the register map, from the GM8136 SDK's U-Boot
 * 2013.01 (board/gm/gm8126, arch-gm8126/gm8126.h) and Linux
 * (arch/arm/mach-GM/platform-GM8126).
 */

#ifndef __MACH_GM8126_H
#define __MACH_GM8126_H

#include <linux/bitops.h>

#define GM_DDR_BASE		0x00000000

#define GM_AHBC1_BASE		0x90100000
#define GM_AHBC4_BASE		0x90400000
#define GM_AHBC5_BASE		0x90500000
#define GM_MAC_BASE		0x92500000	/* FTMAC110 */
#define GM_SSP0_BASE		0x98200000	/* FTSSP010, the flash */
#define GM_UART0_BASE		0x98300000	/* FTUART010, a 16550 */
#define GM_PMU_BASE		0x99000000	/* FTPMU010 */
#define GM_TIMER_BASE		0x99100000	/* FTTMR010 */
#define GM_WDT_BASE		0x99200000	/* FTWDT010 */
#define GM_GPIO0_BASE		0x99400000	/* FTGPIO010 */

/* PMU */
#define GM_PMU_ID		(GM_PMU_BASE + 0x00)	/* 0x8126xxxx */
#define GM_PMU_REG(off)		(GM_PMU_BASE + (off))
#define GM_PMU_PLL1		GM_PMU_REG(0x30)
#define GM_PMU_PLL2		GM_PMU_REG(0x34)
#define GM_PMU_GATE_AHB		GM_PMU_REG(0x38)	/* 1 = gated */
#define GM_PMU_GATE_APB		GM_PMU_REG(0x3C)
#define GM_PMU_PINMUX3		GM_PMU_REG(0x5C)
#define GM_PMU_MAC		GM_PMU_REG(0x6C)
#define GM_PMU_CLK_DIV1		GM_PMU_REG(0x74)
#define GM_PMU_SSP_DIV		GM_PMU_REG(0x78)
#define GM_PMU_EPHY		GM_PMU_REG(0x84)

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

/* GPIO; the flash's chip select is GPIO0 pin 14 */
#define GM_GPIO_DIR		0x08	/* 1 = output */
#define GM_GPIO_SET		0x10
#define GM_GPIO_CLEAR		0x14
#define GM_SPI_CS		BIT(14)

#define GM_UART_CLK		(30000000 / 16)

#ifndef __ASSEMBLY__
unsigned long gm_pll1_hz(void);
unsigned long gm_pll2_hz(void);
unsigned long gm_ahb_hz(void);
unsigned long gm_apb_hz(void);
unsigned long gm_ssp_hz(void);
#endif

#endif
