/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Xiongmai XM510 / XM530 / XM550 register map, from the vendor U-Boot
 * 2014.04 trees (arch/arm/include/asm/arch-xm/xm5x0.h).
 */

#ifndef __MACH_XM_H
#define __MACH_XM_H

/* 4 KiB on-chip SRAM: the only RAM before DDR is up */
#define XM_SRAM_BASE		0x04000000
#define XM_SRAM_TOP		(XM_SRAM_BASE + 0x1000)

#define XM_DDR_BASE		0x80000000

#define XM_UART0_BASE		0x10030000
#define XM_TIMER_BASE		0x100C0000	/* SP804 */
#define XM_DDRC_BASE		0x100B0000
#define XM_GPIO_BASE		0x10020000
#define XM_SYS_BASE		0x20000000
#define XM_SDIO0_BASE		0x50000000

#define XM_TIMER_LOAD		(XM_TIMER_BASE + 0x00)
#define XM_TIMER_VALUE		(XM_TIMER_BASE + 0x04)
#define XM_TIMER_CTRL		(XM_TIMER_BASE + 0x08)
/* Enable, periodic, prescale /256, 32 bit: a free-running down counter */
#define XM_TIMER_CTRL_RUN	0xCA
#define XM_TIMER_PRESCALE	256

#define XM_SC_ST_NORMAL		0x04

#ifdef CONFIG_TARGET_XM510

#define XM_XCLK			24000000
#define XM_SFC_BASE		0x100F0000	/* xmsfc v1 */
#define XM_FLASH_BASE		0x00000000	/* v1 read window */
#define XM_GMAC_BASE		0x20040000
#define XM_DDR_MAX		(IS_ENABLED(CONFIG_XM_DDR_64M) ? 0x4000000 : 0x2000000)

#define XM_SC_CTRL		(XM_SYS_BASE + 0x00)
#define XM_PLLA_CTRL		(XM_SYS_BASE + 0x04)
#define XM_PLLA_STAT		(XM_SYS_BASE + 0x08)
#define XM_PLLC_CTRL		(XM_SYS_BASE + 0x18)
#define XM_PLLC_STAT		(XM_SYS_BASE + 0x1C)
#define XM_PLLD1_CTRL		(XM_SYS_BASE + 0x20)
#define XM_PLLD2_CTRL		(XM_SYS_BASE + 0x24)
#define XM_PLLD_STAT		(XM_SYS_BASE + 0x28)
#define XM_SC_LOCK		(XM_SYS_BASE + 0x2C)
#define XM_SYS_SOFT_RSTEN	(XM_SYS_BASE + 0x30)
#define XM_SYS_SOFT_RST		(XM_SYS_BASE + 0x34)
#define XM_DDR_CTRL_SOFT_RST	(XM_SYS_BASE + 0x64)

#define XM_GPIO5_MULT_USE_EN	(XM_GPIO_BASE + 0x14)
#define XM_GPIO78_MULT_USE_EN	(XM_GPIO_BASE + 0x1C)
#define XM_GPIO8_IO		(XM_GPIO_BASE + 0xC0)
#define XM_GPIO8_OUT_EN		(XM_GPIO_BASE + 0xC4)
#define XM_GPIO8_OUT_REG	(XM_GPIO_BASE + 0xC8)

#else /* XM530, XM550 */

#define XM_XCLK			12000000
#define XM_SFC_BASE		0x100F4000	/* xmsfc v2 */
#define XM_SFC_RAM		0x100F6000
#define XM_FLASH_BASE		0x60000000	/* v2 read window */
#define XM_GMAC_BASE		0x10010000
#define XM_GMAC_GLUE		0x10014000
#define XM_DDR_MAX		(IS_ENABLED(CONFIG_XM_DDR3) ? 0x8000000 : 0x4000000)

#define XM_SC_LOCK		(XM_SYS_BASE + 0x00)
#define XM_SC_CTRL		(XM_SYS_BASE + 0x04)
#define XM_PLLA_CTRL		(XM_SYS_BASE + 0x08)
#define XM_PLLB_CTRL		(XM_SYS_BASE + 0x10)
#define XM_PLLC_CTRL		(XM_SYS_BASE + 0x18)
#define XM_PLLD_CTRL		(XM_SYS_BASE + 0x20)
#define XM_PLL_LOCKED		0x80000000
#define XM_CPUCLK_CTRL		(XM_SYS_BASE + 0x28)
#define XM_GMACCLK_CTRL		(XM_SYS_BASE + 0x30)
#define XM_SDIO0CLK_CTRL	(XM_SYS_BASE + 0x34)
#define XM_SDIO1CLK_CTRL	(XM_SYS_BASE + 0x38)
#define XM_SENSORCLK_CTRL	(XM_SYS_BASE + 0x3C)
#define XM_VENCCLK_CTRL		(XM_SYS_BASE + 0x4C)
#define XM_DDRCLK_CTRL		(XM_SYS_BASE + 0x50)
#define XM_FLASHCLK_CTRL	(XM_SYS_BASE + 0x54)
#define XM_IISCLK_CTRL		(XM_SYS_BASE + 0x58)
#define XM_USBCLK_SEL		(XM_SYS_BASE + 0x64)
#define XM_GSDBUS_CTRL		(XM_SYS_BASE + 0x68)
#define XM_SYS_SOFT_RSTEN	(XM_SYS_BASE + 0x80)
#define XM_SYS_SOFT_RST		(XM_SYS_BASE + 0x84)
#define XM_DDR_RSTN		(XM_SYS_BASE + 0xDC)

/* GPIO pads: one register per pad, function in the low bits */
#define XM_GPIO_PAD(n)		(XM_GPIO_BASE + (n) * 4)
#define XM_GPIO_MUX2_EN		BIT(2)
#define XM_GPIO_DRIVE_8MA	(2 << 6)

#endif

#ifndef __ASSEMBLY__
void xm_early_init(void);
void xm_early_udelay(unsigned int us);
void xm_soc_reset(void);
void xm_pll_init(void);
void xm_ddr_init(void);
#endif

#endif /* __MACH_XM_H */
