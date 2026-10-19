/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Anyka AK39 camera SoCs: AK3918EV200, the AK391xEV300 family and the
 * AK391xEV330 family. One ARM926EJ-S with DDR2 in the package; the memory
 * map is the same in all three generations.
 *
 * From the vendor SDKs: AnyCloud39EV300 (U-Boot 2013.10 arch-ak39, kernel
 * 3.4.35 mach-ak39), the AK3918EV200 4.4 kernel port and its U-Boot port,
 * and the EV330 4.4 device trees (anycloud_ak39ev330*.dtsi).
 */

#ifndef __MACH_ANYKA_H
#define __MACH_ANYKA_H

#include <linux/bitops.h>

#define AK_SYSCTRL_BASE		0x08000000
#define AK_SPI0_BASE		0x20120000
#define AK_UART0_BASE		0x20130000
#define AK_L2_BASE		0x20140000
#define AK_GPIO_BASE		0x20170000
#define AK_GMAC_BASE		0x20300000
#define AK_L2_MEM_BASE		0x48000000
#define AK_DDR_BASE		0x80000000
#define AK_DDR_MAX		0x08000000	/* AK3919: 128 MiB */

/* System control */
#define AK_SYS_CHIP_ID		(AK_SYSCTRL_BASE + 0x00)
#define AK_SYS_CPU_PLL		(AK_SYSCTRL_BASE + 0x04)
#define AK_SYS_ASIC_PLL		(AK_SYSCTRL_BASE + 0x08)
#define AK_SYS_MAC_CLK		(AK_SYSCTRL_BASE + 0x14)
#define AK_SYS_CLK_GATE		(AK_SYSCTRL_BASE + 0x1c)	/* 1 = gated */
#define AK_SYS_SOFT_RST		(AK_SYSCTRL_BASE + 0x20)	/* 1 = in reset */
#define AK_SYS_SHAREPIN0	(AK_SYSCTRL_BASE + 0x74)
#define AK_SYS_SHAREPIN2	(AK_SYSCTRL_BASE + 0x7c)
#define AK_SYS_PULL0		(AK_SYSCTRL_BASE + 0x80)	/* 1 = pull off */
#define AK_SYS_TIMER1_CTRL1	(AK_SYSCTRL_BASE + 0xb4)
#define AK_SYS_TIMER1_CTRL2	(AK_SYSCTRL_BASE + 0xb8)
#define AK_SYS_WDT_CFG1		(AK_SYSCTRL_BASE + 0xe4)
#define AK_SYS_WDT_CFG2		(AK_SYSCTRL_BASE + 0xe8)

#define AK_CLK_MCI0		BIT(1)
#define AK_CLK_UART0		BIT(7)
#define AK_CLK_MAC		BIT(13)

/*
 * ASIC PLL: 12 MHz * M / (N << OD), then the "vclk" divider (0 counts as
 * 1), then halved again for the ASIC (peripheral) clock when bit 24 is set
 * (the vendor U-Boot's get_asic_freq()).
 */
#define AK_XTAL_HZ		12000000
#define AK_PLL_APPLY		BIT(28)
/* EV200: CPU 400 / memory 200 / ASIC 100 MHz */
#define AK_EV200_ASIC_PLL	0x018213c8

/* Timer 1: a 32-bit down counter off the 12 MHz crystal */
#define AK_TIMER_READ_SEL	BIT(26)
#define AK_TIMER_EN		BIT(28)
#define AK_TIMER_FEED		BIT(29)

/* Watchdog: CFG1 = key | count (1/46875 s), CFG2 = key | feed | enable */
#define AK_WDT_CFG1_KEY		0x55000000
#define AK_WDT_CFG2_KEY		0xaa000000
#define AK_WDT_EN		BIT(0)
#define AK_WDT_FEED		BIT(1)
#define AK_WDT_COUNT_8S		0x05b8d8

/* L2 buffer controller, UART buffers; UART0 TX/RX buffers in L2 memory */
#define AK_L2_FRAC_ADDR		(AK_L2_BASE + 0x84)
#define AK_L2_CONBUF8_15	(AK_L2_BASE + 0x8c)
#define AK_L2_UART0_TX_CLR	BIT(16)
#define AK_L2_UART0_RX_CLR	BIT(17)
#define AK_UART0_TXBUF		(AK_L2_MEM_BASE + 0x1000)
#define AK_UART0_RXBUF		(AK_L2_MEM_BASE + 0x1080)
#define AK_UART_RXBUF_SIZE	128

/* UART0 pads: shared-pin bits and pull bits */
#ifdef CONFIG_AK39_EV200
#define AK_SHAREPIN0_UART0	(BIT(1) | BIT(2))
#else
#define AK_SHAREPIN0_UART0	(BIT(2) | BIT(3))
#endif
#define AK_PULL0_UART0		(BIT(1) | BIT(2))

/* GPIO: direction (1 = out) and output banks, 32 pins a bank */
#define AK_GPIO_DIR(n)		(AK_GPIO_BASE + 0x00 + 4 * ((n) / 32))
#define AK_GPIO_OUT(n)		(AK_GPIO_BASE + 0x0c + 4 * ((n) / 32))

/* Soft reset bits (AK_SYS_SOFT_RST) */
#define AK_RST_MCI0		BIT(1)
#define AK_RST_SPI0		BIT(5)

#define AK_MCI0_BASE		0x20100000
#define AK_SYS_SHAREPIN3	(AK_SYSCTRL_BASE + 0xdc)

#ifndef __ASSEMBLY__
unsigned long ak_asic_clk(void);
#endif

#endif /* __MACH_ANYKA_H */
