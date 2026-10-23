/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Goke GK7101 / GK7102 and GK7101S / GK7102S: the silicon register maps.
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

#ifndef CONFIG_GK710X_S

/* AHB */
#define GK_SDIO0_BASE		0x60002000	/* SDHCI */
#define GK_DDRC_BASE		0x60004000
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

/* Timer 1 counts down at the APB clock; control is a nibble per timer */
#define GK_TIMER1_STATUS	(GK_TIMER_BASE + 0x00)
#define GK_TIMER1_RELOAD	(GK_TIMER_BASE + 0x04)
#define GK_TIMER_CTRL		(GK_TIMER_BASE + 0x30)
#define GK_TIMER1_CTRL_MASK	0xf
#define GK_TIMER1_CTRL_EN	0x1

#define GK_WDT_RELOAD		(GK_WDT_BASE + 0x04)
#define GK_WDT_RESTART		(GK_WDT_BASE + 0x08)
#define GK_WDT_CTRL		(GK_WDT_BASE + 0x0C)
#define GK_WDT_CTRL_RESET	0x3	/* enable, reset on timeout */

/* RCT: clocks and pad control */
#define GK_RCT_PLL_AUDIO_CTRL	(GK_RCT_BASE + 0x054)
#define GK_RCT_PLL_AUDIO_FRAC	(GK_RCT_BASE + 0x058)
#define GK_RCT_DIV_AUDIO_PST	(GK_RCT_BASE + 0x05C)
#define GK_RCT_DIV_AUDIO_PRE	(GK_RCT_BASE + 0x060)
#define GK_RCT_SCALER_UART	(GK_RCT_BASE + 0x038)
#define GK_RCT_PLL_CORE		(GK_RCT_BASE + 0x0E4)
#define GK_RCT_PLL_SD		(GK_RCT_BASE + 0x000)	/* fields moved */
#define GK_RCT_SCALER_SD	(GK_RCT_BASE + 0x00C)
#define GK_RCT_IOCTRL_MISC2	(GK_RCT_BASE + 0x200)
#define GK_IOCTRL_MISC2_VAL	0x11
/* Pad drive and pull: one byte per pin, four pins per word */
#define GK_RCT_IOCTRL_GPIO(n)	(GK_RCT_BASE + 0x230 + 4 * ((n) / 4))

/* Audio PLL at 96 MHz / 4 for the internal PHY, in silicon bit order */
#define GK_PLL_AUDIO_EPHY	0x02402001
#define GK_PLL_AUDIO_EXTPHY	0x02402101	/* 100 MHz, as the HAL writes 0x00124021 */

#else /* CONFIG_GK710X_S */

/*
 * The GK710xS die has its blocks at the vendor drivers' addresses and in
 * their register and bit layout: its HAL passes nearly every access through
 * unchanged. The UARTs and the VICs are the exceptions, reordered by the
 * HAL as on the GK7101/GK7102, which leaves the UART a plain 16550 here too.
 */

/* AHB */
#define GK_SDIO0_BASE		0x90000000	/* vendor layout, sdhci.c */
#define GK_DDRC_BASE		0x90002000
#define GK_GMAC_BASE		0x9000E000	/* DWMAC, registers reordered */
#define GK_SFLASH_BASE		0x90016000
#define GK_AHB_GENERAL0		0x90020E00
#define GK_EPHY_BASE		0x90022000

/* APB */
#define GK_TIMER_BASE		0xA0000000
#define GK_UART0_BASE		0xA0005000
#define GK_WDT_BASE		0xA0006000
#define GK_GPIO_BASE		0xA0009000
#define GK_RCT_BASE		0xA0170000

/* Timer 1: count at +0, reload at +0x30; enable, ext clock, irq at 10/6/2 */
#define GK_TIMER1_STATUS	(GK_TIMER_BASE + 0x00)
#define GK_TIMER1_RELOAD	(GK_TIMER_BASE + 0x30)
#define GK_TIMER_CTRL		(GK_TIMER_BASE + 0x0C)
#define GK_TIMER1_CTRL_MASK	0x444
#define GK_TIMER1_CTRL_EN	0x400

#define GK_WDT_CTRL		(GK_WDT_BASE + 0x00)
#define GK_WDT_RELOAD		(GK_WDT_BASE + 0x10)
#define GK_WDT_RESTART		(GK_WDT_BASE + 0x14)
#define GK_WDT_CTRL_RESET	0x5	/* enable, reset on timeout */

#define GK_RCT_PLL_AUDIO_CTRL	(GK_RCT_BASE + 0x024)
#define GK_RCT_PLL_AUDIO_FRAC	(GK_RCT_BASE + 0x078)
#define GK_RCT_DIV_AUDIO_PST	(GK_RCT_BASE + 0x084)
#define GK_RCT_DIV_AUDIO_PRE	(GK_RCT_BASE + 0x080)
#define GK_RCT_SCALER_UART	(GK_RCT_BASE + 0x028)
#define GK_RCT_PLL_SD		(GK_RCT_BASE + 0x014)
#define GK_RCT_SCALER_SD	(GK_RCT_BASE + 0x018)
#define GK_RCT_PLL_ARM		(GK_RCT_BASE + 0x08C)	/* the kernel's arm_freq */
#define GK_RCT_IOCTRL_MISC2	(GK_RCT_BASE + 0x198)
#define GK_IOCTRL_MISC2_VAL	0x12	/* the flash HOLD pin pulled up */
/* Pad control words count down from GPIO 55 (gk7101s_gpio.c) */
#define GK_RCT_IOCTRL_GPIO(n)	(GK_RCT_BASE + 0x230 + 4 * \
				 ((n) < 56 ? (55 - (n)) / 4 : (n) / 4))

#define GK_PLL_AUDIO_EPHY	0x00124020
#define GK_PLL_AUDIO_EXTPHY	0x00124021

#endif /* CONFIG_GK710X_S */

/* Both: the DDR controller word the boot header sets tells 64 from 128 MiB */
#define GK_DDRC_CFG		(GK_DDRC_BASE + 0x04)
#define GK_DDRC_CFG_128M	0x00100354

#define GK_UART_CLK		24000000
#define GK_APB_CLK		69000000

#define GK_WDT_MAGIC		0x4755

/* GPIO: a function word per pin and a pin number per input function */
#define GK_GPIO_IN_EN		(GK_GPIO_BASE + 0x038)
#define GK_GPIO_PER_SEL		(GK_GPIO_BASE + 0x03C)	/* bit 0: external PHY */
#define GK_GPIO_OUTPUT_CFG(n)	(GK_GPIO_BASE + 0x100 + 4 * (n))
#define GK_GPIO_INPUT_CFG(n)	(GK_GPIO_BASE + 0x200 + 4 * (n))

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

/* The vendor's GPIO_TYPE_OUTPUT_UART0_TX, GPIO_TYPE_OUTPUT_0 and _1 */
#define GK_GPIO_UART0_TX	0x90000009
#define GK_GPIO_OUTPUT_0	0xa0000000
#define GK_GPIO_OUTPUT_1	0xa0000001

#ifndef __ASSEMBLY__
void gk_gpio_config(unsigned int pin, unsigned int type);
void gk_gpio_init(void);
/* board/goke/gk710x/gpio_map.c */
void gk_gpio_map_early(void);
void gk_gpio_map_apply(void);
void gk_gpio_map_phy(int phytype);
unsigned int gk_gpio_map_phy_reset(void);
void gk_gpio_map_save(void *dst);
#endif

#endif
