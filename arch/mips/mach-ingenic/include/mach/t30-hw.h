/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Ingenic T30: PLL, bus and DDR set-up, as the vendor's
 * include/configs/isvp_t30.h has it (OpenIPC/u-boot-ingenic).
 * The SoC, the package variant and the DDR chip are Kconfig options.
 *
 * Copyright (c) Ingenic Semiconductor Co.,Ltd
 */

#ifndef __MACH_INGENIC_T30_HW_H
#define __MACH_INGENIC_T30_HW_H

/* #define CONFIG_DDR_AUTO_SELF_REFRESH */

#if defined(CONFIG_LITE_VERSION)
#define CONFIG_SYS_APLL_FREQ		750000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((124 << 20) | (1 << 14) | (2 << 11) | (2<<5))
#elif defined(CONFIG_T30A)
#define CONFIG_SYS_APLL_FREQ		1200000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((149 << 20) | (2 << 14) | (1 << 11) | (1<<5))
#else
#define CONFIG_SYS_APLL_FREQ		900000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((74 << 20) | (1 << 14) | (1 << 11) | (2<<5))
#endif

#if (defined(CONFIG_T30A) || defined(CONFIG_DDR3_128M))
#define DDR_600M
#else
#define DDR_500M
#endif

#ifdef DDR_400M
#define CONFIG_SYS_MPLL_FREQ		1200000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((149 << 20) | (2 << 14) | (1 << 11) | (1<<5))
#elif defined DDR_500M
#if 1
#define CONFIG_SYS_MPLL_FREQ		1000000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD        ((124 << 20) | (2 << 14) | (1 << 11) | (1<<5))
#else
#define CONFIG_SYS_MPLL_FREQ		1500000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((249 << 20) | (3 << 14) | (1 << 11) | (1<<5))
#endif
#elif defined DDR_600M
#define CONFIG_SYS_MPLL_FREQ		1200000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((149 << 20) | (2 << 14) | (1 << 11) | (1<<5))
#else
#error please define DDR_FREQ
#endif

#define CONFIG_SYS_VPLL_FREQ		1200000000	/*If VPLL not use mast be set 0*/
#define CONFIG_SYS_EPLL_MNOD        ((197 << 20) | (3 << 14) | (1 << 11) | (1<<5))
#define CONFIG_SYS_EPLL_FREQ		1188000000
#define SEL_SCLKA		2
#define SEL_CPU			1
#define SEL_H0			2
#define SEL_H2			2

#ifdef DDR_400M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_500M
#define DIV_PCLK		8
#define DIV_H2			4
#define DIV_H0			4
#elif defined DDR_600M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#else
#error please define DDR_FREQ
#endif

#define DIV_L2			2
#define DIV_CPU			1
#define CONFIG_SYS_CPCCR_SEL		(((SEL_SCLKA & 3) << 30)			\
									 | ((SEL_CPU & 3) << 28)			\
									 | ((SEL_H0 & 3) << 26)				\
									 | ((SEL_H2 & 3) << 24)				\
									 | (((DIV_PCLK - 1) & 0xf) << 16)	\
									 | (((DIV_H2 - 1) & 0xf) << 12)		\
									 | (((DIV_H0 - 1) & 0xf) << 8)		\
									 | (((DIV_L2 - 1) & 0xf) << 4)		\
									 | (((DIV_CPU - 1) & 0xf) << 0))

#define CONFIG_CPU_SEL_PLL		APLL
#define CONFIG_DDR_SEL_PLL		MPLL
#define CONFIG_SYS_CPU_FREQ		CONFIG_SYS_APLL_FREQ

#ifdef DDR_400M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 3)
#elif defined DDR_500M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_600M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#else
#error please define DDR_FREQ
#endif

#define CONFIG_SYS_EXTAL		24000000	/* EXTAL freq: 24 MHz */

#define CONFIG_SYS_CACHELINE_SIZE	32

#define CONFIG_SYS_UART_INDEX		1
#define CONFIG_SYS_UART_CONTROLLER_STEP		0x1000

/*#define CONFIG_DDR_TEST_CPU
#define CONFIG_DDR_TEST*/
#define CONFIG_DDR_FORCE_SELECT_CS1
#define CONFIG_DDR_CS0			1	/* 1-connected, 0-disconnected */
#define CONFIG_DDR_CS1			0	/* 1-connected, 0-disconnected */
#define CONFIG_DDR_DW32			0	/* 1-32bit-width, 0-16bit-width */
#define CONFIG_DDRC_CTRL_PDT DDRC_CTRL_PDT_128

#ifdef CONFIG_DDR2_128M
#elif defined(CONFIG_DDR3_128M)
#else
#endif
#define DDR2_CHIP_DRIVER_OUT_STRENGTH 0

#define CONFIG_DDR_PHY_IMPEDANCE 40000
#define CONFIG_DDR_PHY_ODT_IMPEDANCE 50000 //75000
/*#define CONFIG_DDR_PHY_IMPED_PULLUP	0xf*/
/*#define CONFIG_DDR_PHY_IMPED_PULLDOWN	0xf*/

/* #define CONFIG_DDR_DLL_OFF */

/*#define CONFIG_DDR_CHIP_ODT*/
/*#define CONFIG_DDR_PHY_ODT*/
/*#define CONFIG_DDR_PHY_DQ_ODT*/
/*#define CONFIG_DDR_PHY_DQS_ODT*/

#endif
