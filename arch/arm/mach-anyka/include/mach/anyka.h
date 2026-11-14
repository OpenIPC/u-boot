/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Anyka AK39 camera SoCs: AK3918EV200, the AK391xEV300 family, the
 * AK391xEV330 family and the AK3918AV100 family (AV100, EV300L;
 * CONFIG_AK39_AV100). One ARM926EJ-S with DDR2 in the package; the memory
 * map is the same in the first three generations. The AV100 family keeps
 * the system control, UART, L2, GPIO, MCI and GMAC blocks where they were,
 * but moves the SPI controllers, the shared pins and the PLLs.
 *
 * From the vendor SDKs: AnyCloud39EV300 (U-Boot 2013.10 arch-ak39, kernel
 * 3.4.35 mach-ak39), the AK3918EV200 4.4 kernel port and its U-Boot port,
 * the EV330 4.4 device trees (anycloud_ak39ev330*.dtsi), and for the EV300L
 * the anycloud39av130 SDK V1.03 (U-Boot 2019.10 arch-ak3918ev300l,
 * clk-ak3918ev300l.c, pinctrl-ak3918ev300l.c, ak_spi.c, ak_serial.c), checked
 * for the AV100 against the AnyCloud39AV100 SDK V1.12 (arch-ak3918av100,
 * clk-ak3918av100.c, pinctrl-ak3918av100.c, the same ak_spi.c, ak_serial.c,
 * ak_mci.c and ak_timer_wdt.c, the AV100 device trees).
 */

#ifndef __MACH_ANYKA_H
#define __MACH_ANYKA_H

#include <linux/bitops.h>

#define AK_SYSCTRL_BASE		0x08000000
#ifdef CONFIG_AK39_AV100
#define AK_SPI0_BASE		0x21100000
#else
#define AK_SPI0_BASE		0x20120000
#endif
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
#if defined(CONFIG_AK39_EV200)
#define AK_SHAREPIN0_UART0	(BIT(1) | BIT(2))
#else
#define AK_SHAREPIN0_UART0	(BIT(2) | BIT(3))
#endif
#define AK_PULL0_UART0		(BIT(1) | BIT(2))

/*
 * GPIO: direction (1 = out) and output banks, 32 pins a bank. The AV100
 * family has four direction banks (pins up to 104), so its outputs start later.
 */
#ifdef CONFIG_AK39_AV100
#define AK_GPIO_OUT_OFF		0x14
#else
#define AK_GPIO_OUT_OFF		0x0c
#endif
#define AK_GPIO_DIR(n)		(AK_GPIO_BASE + 0x00 + 4 * ((n) / 32))
#define AK_GPIO_OUT(n)		(AK_GPIO_BASE + AK_GPIO_OUT_OFF + 4 * ((n) / 32))

/* Soft reset bits (AK_SYS_SOFT_RST) */
#define AK_RST_MCI0		BIT(1)
#define AK_RST_SPI0		BIT(5)

#define AK_MCI0_BASE		0x20100000
#define AK_MCI1_BASE		0x20108000
#define AK_SYS_SHAREPIN3	(AK_SYSCTRL_BASE + 0xdc)

/*
 * AV100 family (arch-ak3918ev300l, clk-ak3918ev300l.h). 24 MHz crystal. PLL1
 * (+0x04) clocks the CPU, PLL2 (+0x08) the peripherals: PLL2 = 24 MHz *
 * (F + 1) / (R + 1) / (OD + 1), GCLK (the ASIC clock the UART and SPI
 * dividers count) = PLL2 / 2 / (GDIV + 1). The boot header sets only PLL1
 * and the DDR controller; out of reset GCLK is 60 MHz. The timers still
 * count at 12 MHz.
 */
#define AK_AV100_XTAL_HZ	24000000
#define AK_SYS_PLL1		(AK_SYSCTRL_BASE + 0x04)
#define AK_SYS_PLL2		(AK_SYSCTRL_BASE + 0x08)
#define AK_SYS_SPI_CLK		(AK_SYSCTRL_BASE + 0x0c)
#define AK_SYS_PLL_BW		(AK_SYSCTRL_BASE + 0xe0)
#define AK_PLL2_F(v)		((v) & 0x1fff)
#define AK_PLL2_R(v)		(((v) >> 13) & 0x3f)
#define AK_PLL2_OD(v)		(((v) >> 19) & 0xf)
#define AK_PLL2_GDIV(v)		(((v) >> 23) & 0x3)
#define AK_PLL2_GDIV_VLD	BIT(25)
#define AK_PLL2_CFG_MASK	0x1ffffff	/* F, R, OD, GDIV */
#define AK_PLL1_PLL2_ADJ	BIT(28)		/* in PLL1: apply PLL2 */
/*
 * PLL2 400 MHz, GCLK 200 MHz, bandwidth 16: what the vendor U-Boot's and
 * kernel's device trees ask for (F 99, R 0, OD 5, GDIV 0)
 */
#define AK_AV100_PLL2		(AK_PLL2_GDIV_VLD | 5 << 19 | 0 << 13 | 99)
#define AK_AV100_PLL_BW	(16 - 1)

/* SPI0 clock: SYSCTRL +0x0c; SPI clock = PLL2 / 2 / (divider + 1) */
#define AK_SPI_CLK_DIV(d)	((d) << 10)
#define AK_SPI_CLK_DIV_MASK	(0xff << 10)
#define AK_SPI_CLK_EN		BIT(18)
#define AK_SPI_CLK_APPLY	BIT(19)
#define AK_RST_SPI_HSCLK	BIT(31)		/* in AK_SYS_SOFT_RST */
/* SPI controller: WORK_MODE bit 0 = normal mode, read back when taken */
#define AK_SPI_WORK_MODE	(AK_SPI0_BASE + 0x24)

/* Shared pins: 3-bit function fields, ten pins a register, from 0x178 */
#define AK_SYS_SHAREPIN_CFG(n)	(AK_SYSCTRL_BASE + 0x178 + 4 * (n))
/*
 * SPI0 (CFG4): the SDK's spi_sharepin_cfg() value; pads 42-47 (WP, DIN,
 * CS, HOLD, CLK, DOUT) function 2, and fields 2:0 and 8:6 at 1
 */
#define AK_AV100_SPI0_PINS	0x2492441
/* UART0 TXD/RXD on GPIO 51/52: CFG5 fields 8:6 and 11:9, function 1 */
#define AK_AV100_UART0_PINS_CLR	(0x3f << 6)
#define AK_AV100_UART0_PINS	(BIT(6) | BIT(9))
/* SD0 pads 31-36 (D2, D3, CMD, MCLK, D0, D1): CFG3 fields 8:6 .. 23:21 */
#define AK_AV100_SD0_PINS_CLR	(0x3ffff << 6)
#define AK_AV100_SD0_PINS	0x249240	/* function 1 each */
/* Pad 98: SYS_RSTN_OUT (function 0) or GPIO 98 (1), CFG8 bits 23:21 */
#define AK_AV100_PAD98_CLR	(0x7 << 21)
#define AK_AV100_PAD98_GPIO	(0x1 << 21)

/*
 * TF card power, OpenIPC's per-SoC defaults ("sdpower=" overrides them).
 * The SDK's boards power the card from GPIO 50 (card detect on 98) or from
 * GPIO 98 (detect on 37); the first pin is tried first, the other if no
 * card answers. The EV300L's is that of the gzhongshi HVT3610S3, the one
 * EV300L board seen (GPIO 98); the AV100's that of the SDK's EVB_CBDM
 * boards (GPIO 50).
 */
#if defined(CONFIG_TARGET_AK3918EV300L)
#define AK_AV100_SD_POWER	98
#define AK_AV100_SD_POWER_ALT	50
#else
#define AK_AV100_SD_POWER	50
#define AK_AV100_SD_POWER_ALT	98
#endif
/* Pad control, 32 pads a register: pull enable, pull select (0 = up), IE */
#define AK_SYS_PUPD_EN(n)	(AK_SYSCTRL_BASE + 0x264 + 4 * (n))
#define AK_SYS_PUPD_SEL(n)	(AK_SYSCTRL_BASE + 0x274 + 4 * (n))
#define AK_SYS_PAD_IE(n)	(AK_SYSCTRL_BASE + 0x1c0 + 4 * (n))
#define AK_CLK_MCI1		BIT(2)
#define AK_RST_MCI1		BIT(2)

#ifndef __ASSEMBLY__
unsigned long ak_asic_clk(void);
#ifdef CONFIG_AK39_AV100
unsigned long ak_pll2_clk(void);
#endif
#endif

#endif /* __MACH_ANYKA_H */
