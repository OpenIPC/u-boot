/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Ingenic T23: PLL, bus and DDR set-up, as the vendor's
 * include/configs/isvp_t23.h has it (OpenIPC/u-boot-ingenic).
 * The SoC, the package variant and the DDR chip are Kconfig options.
 *
 * Copyright (c) Ingenic Semiconductor Co.,Ltd
 */

#ifndef __MACH_INGENIC_T23_HW_H
#define __MACH_INGENIC_T23_HW_H

/* #define CONFIG_DDR_AUTO_SELF_REFRESH */
/*#define CONFIG_SPL_DDR_SOFT_TRAINING*/

/* T23 */
#ifdef CONFIG_T23N /* T23N */
#ifdef CONFIG_HP
#define APLL_1400M
#define DDR_600M
#elif defined CONFIG_LP
#define APLL_936M
#define DDR_400M
#else /* normal */
#define APLL_1188M
#define DDR_600M
#endif /* T23N */

#elif CONFIG_T23X /* T23X */
#ifdef CONFIG_HP
#define APLL_1400M
#define DDR_600M
#elif defined CONFIG_LP
#define APLL_936M
#define DDR_400M
#else /* normal */
#define APLL_1188M
#define DDR_600M
#endif /* T23X */

#else
#define APLL_1000M
#define DDR_500M
#endif

#ifdef APLL_891M
#define CONFIG_SYS_APLL_FREQ		891000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((297 << 20) | (4 << 14) | (2 << 11) | (1<<8))
#elif defined APLL_936M
#define CONFIG_SYS_APLL_FREQ		936000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((234 << 20) | (3 << 14) | (2 << 11) | (1<<8))
#elif defined APLL_1000M
#define CONFIG_SYS_APLL_FREQ		1000000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((125 << 20) | (1 << 14) | (3 << 11) | (1<<8))
#elif defined APLL_1100M
#define CONFIG_SYS_APLL_FREQ		1100000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((275 << 20) | (3 << 14) | (2 << 11) | (1<<8))
#elif defined APLL_1188M
#define CONFIG_SYS_APLL_FREQ		1188000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((297 << 20) | (3 << 14) | (2 << 11) | (1<<8))
#elif defined APLL_1200M
#define CONFIG_SYS_APLL_FREQ		1200000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((100 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined APLL_1300M
#define CONFIG_SYS_APLL_FREQ		1300000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((325 << 20) | (3 << 14) | (2 << 11) | (1<<8))
#elif defined APLL_1400M
#define CONFIG_SYS_APLL_FREQ		1400000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((350 << 20) | (3 << 14) | (2 << 11) | (1<<8))
#elif defined APLL_1485M
#define CONFIG_SYS_APLL_FREQ		1485000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((495 << 20) | (4 << 14) | (2 << 11) | (1<<8))
#elif defined APLL_1500M
#define CONFIG_SYS_APLL_FREQ		1500000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((125 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#else
#error please define APLL_FREQ
#endif

#ifdef DDR_400M
#define CONFIG_SYS_MPLL_FREQ		1200000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((100 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_500M
#define CONFIG_SYS_MPLL_FREQ		1000000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((125 << 20) | (1 << 14) | (3 << 11) | (1<<8))
#elif defined DDR_550M
#define CONFIG_SYS_MPLL_FREQ		1100000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((275 << 20) | (3 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_594M
#define CONFIG_SYS_MPLL_FREQ		1188000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((297 << 20) | (3 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_600M
#define CONFIG_SYS_MPLL_FREQ		1200000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((100 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_650M
#define CONFIG_SYS_MPLL_FREQ		1300000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((325 << 20) | (3 << 14) | (2 << 11) | (1<<8))
#else
#error please define DDR_FREQ
#endif

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
#elif defined DDR_550M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_594M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_600M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_650M
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
#elif defined DDR_550M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_594M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_600M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_650M
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

#if defined(CONFIG_T23N)
#elif defined(CONFIG_T23X)
#else
    /* #define CONFIG_DDR3_M15T1G1664A_2C */
#endif

#define DDR2_CHIP_DRIVER_OUT_STRENGTH 0
#define DDR2_CHIP_MR0_DLL_RST

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
