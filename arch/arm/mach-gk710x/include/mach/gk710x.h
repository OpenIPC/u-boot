/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Goke GK7101 / GK7102: the silicon register map.
 *
 * The vendor U-Boot and kernel never touch these addresses. They go through
 * a HAL blob (hal_data.h) that takes a fictitious map (AHB 0x90000000, APB
 * 0xA0000000), moves each block to its real place, shuffles registers inside
 * some blocks and permutes the bits of some registers. The addresses and
 * values here are what the HAL itself writes, recovered by executing it
 * (qemu-hisilicon's qemu-boot/gk710x-halmap.py). Underneath, the UART is a
 * 16550, the MAC a Synopsys DWMAC and the timers and watchdog Ambarella's.
 */

#ifndef __MACH_GK710X_H
#define __MACH_GK710X_H

#define GK_DDR_BASE		0xC0000000
#define GK_DDR_MAX		0x08000000	/* GK7101 */

/* AHB */
#define GK_DDRC_BASE		0x60004000
#define GK_DDRC_CFG		(GK_DDRC_BASE + 0x04)	/* from the boot header */
#define GK_DDRC_CFG_128M	0x00100354
#define GK_GMAC_BASE		0x6000E000
#define GK_SFLASH_BASE		0x60016000
#define GK_AHB_GENERAL0		0x60020E00	/* bit 1: external PHY */
#define GK_EPHY_BASE		0x60022000	/* MII register n at +4n */

/* APB */
#define GK_UART0_BASE		0x70005000
#define GK_GPIO_BASE		0x70009000
#define GK_TIMER_BASE		0x7000B000
#define GK_WDT_BASE		0x7000C000
#define GK_RCT_BASE		0x70170000

#define GK_UART_CLK		24000000

/* Timer 1 counts down at the APB clock; control is a nibble per timer */
#define GK_APB_CLK		69000000
#define GK_TIMER1_STATUS	(GK_TIMER_BASE + 0x00)
#define GK_TIMER1_RELOAD	(GK_TIMER_BASE + 0x04)
#define GK_TIMER_CTRL		(GK_TIMER_BASE + 0x30)
#define GK_TIMER_CTRL_EN	0x1

#define GK_WDT_RELOAD		(GK_WDT_BASE + 0x04)
#define GK_WDT_RESTART		(GK_WDT_BASE + 0x08)
#define GK_WDT_CTRL		(GK_WDT_BASE + 0x0C)
#define GK_WDT_MAGIC		0x4755
#define GK_WDT_CTRL_EN		0x1
#define GK_WDT_CTRL_RESET	0x2

/* GPIO: a function word per pin and a pin number per input function */
#define GK_GPIO_IN_EN		(GK_GPIO_BASE + 0x038)
#define GK_GPIO_PER_SEL		(GK_GPIO_BASE + 0x03C)	/* bit 0: external PHY */
#define GK_GPIO_OUTPUT_CFG(n)	(GK_GPIO_BASE + 0x100 + 4 * (n))
#define GK_GPIO_INPUT_CFG(n)	(GK_GPIO_BASE + 0x200 + 4 * (n))

/* RCT: clocks and pad control */
#define GK_RCT_PLL_AUDIO_CTRL	(GK_RCT_BASE + 0x054)
#define GK_RCT_PLL_AUDIO_FRAC	(GK_RCT_BASE + 0x058)
#define GK_RCT_DIV_AUDIO_PST	(GK_RCT_BASE + 0x05C)
#define GK_RCT_DIV_AUDIO_PRE	(GK_RCT_BASE + 0x060)
#define GK_RCT_SCALER_UART	(GK_RCT_BASE + 0x038)
#define GK_RCT_PLL_CORE		(GK_RCT_BASE + 0x0E4)
#define GK_RCT_IOCTRL_MISC2	(GK_RCT_BASE + 0x200)
/* Pad drive and pull: one byte per pin, four pins per word */
#define GK_RCT_IOCTRL_GPIO(n)	(GK_RCT_BASE + 0x230 + 4 * ((n) / 4))

/*
 * What the vendor U-Boot leaves for the kernel (include/configs/gk7101.h):
 * the HAL, its address, and a few words of board parameters ("U2K") at the
 * bottom of DDR; the board's GPIO map in its top 64 KiB.
 */
#define GK_HAL_ADDR		(GK_DDR_BASE + 0x12000)
#define GK_U2K_PHY_TYPE		(GK_DDR_BASE + 0x00)
#define GK_U2K_ARM_FREQ		(GK_DDR_BASE + 0x04)
#define GK_U2K_HAL_ADDR		(GK_DDR_BASE + 0x08)
#define GK_U2K_SOC		(GK_DDR_BASE + 0x10)
#define GK_U2K_MEM		(GK_DDR_BASE + 0x14)
#define GK_U2K_BSB		(GK_DDR_BASE + 0x18)
#define GK_U2K_ENABLE		(GK_DDR_BASE + 0x20)
#define GK_U2K_USR		(GK_DDR_BASE + 0x24)
#define GK_U2K_TOTAL_MEM	(GK_DDR_BASE + 0x28)
#define GK_U2K_MAGIC		0x7101
#define GK_USR_SIZE		0x10000

/* The CPU PLL setting the vendor U-Boot programs, as the HAL takes it */
#define GK_ARM_FREQ		0x00112032

#ifndef __ASSEMBLY__
void gk_gpio_config(unsigned int pin, unsigned int type);
void gk_gpio_init(void);
/* board/goke/gk710x/gpio_map.c */
void gk_gpio_map_apply(void);
void gk_gpio_map_save(void *dst);
#endif

#endif
