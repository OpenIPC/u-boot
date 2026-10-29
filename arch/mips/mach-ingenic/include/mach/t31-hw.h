/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Ingenic T31: PLL, bus and DDR set-up, as the vendor's
 * include/configs/isvp_t31.h has it (OpenIPC/u-boot-ingenic).
 * The SoC, the package variant and the DDR chip are Kconfig options.
 *
 * Copyright (c) Ingenic Semiconductor Co.,Ltd
 */

#ifndef __MACH_INGENIC_T31_HW_H
#define __MACH_INGENIC_T31_HW_H

/* #define CONFIG_DDR_AUTO_SELF_REFRESH */

#if defined(CONFIG_LITE_VERSION)
#define APLL_1008M
#define DDR_500M
#elif defined(CONFIG_DDR2_128M)
#define APLL_1392M
#define DDR_600M
#elif defined(CONFIG_T31A)
#define APLL_1392M
#define DDR_750M
#elif defined(CONFIG_T31LC)
#define APLL_1104M
#define DDR_500M
#else
/* T31N */
#define APLL_1400M
#define DDR_500M
#endif

#ifdef APLL_804M
#define CONFIG_SYS_APLL_FREQ		804000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((67 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined APLL_864M
#define CONFIG_SYS_APLL_FREQ		864000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((72 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined APLL_900M
#define CONFIG_SYS_APLL_FREQ		900000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((75 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined APLL_1000M
#define CONFIG_SYS_APLL_FREQ		1000000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((125 << 20) | (1 << 14) | (3 << 11) | (1<<8))
#elif defined APLL_1008M
#define CONFIG_SYS_APLL_FREQ		1008000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((84 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined APLL_1104M
#define CONFIG_SYS_APLL_FREQ		1104000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((92 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined APLL_1200M
#define CONFIG_SYS_APLL_FREQ		1200000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((100 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined APLL_1392M
#define CONFIG_SYS_APLL_FREQ		1392000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((116 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined APLL_1400M
#define CONFIG_SYS_APLL_FREQ		1404000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((117 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined APLL_1500M
#define CONFIG_SYS_APLL_FREQ		1500000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((125 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined APLL_1800M
#define CONFIG_SYS_APLL_FREQ		1800000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((150 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#else
#error please define APLL_FREQ
#endif

#ifdef DDR_400M
#define CONFIG_SYS_MPLL_FREQ		1200000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((100 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_450M
#define CONFIG_SYS_MPLL_FREQ		900000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD        ((75 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_500M
#define CONFIG_SYS_MPLL_FREQ		1000000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD        ((125 << 20) | (1 << 14) | (3 << 11) | (1<<8))
#elif defined DDR_540M
#define CONFIG_SYS_MPLL_FREQ		1080000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD        ((90 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_600M
#define CONFIG_SYS_MPLL_FREQ		1200000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((100 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_650M
#define CONFIG_SYS_MPLL_FREQ		1308000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((109 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_700M
#define CONFIG_SYS_MPLL_FREQ		1404000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((117 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_750M
#define CONFIG_SYS_MPLL_FREQ		1500000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((125 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_762M
#define CONFIG_SYS_MPLL_FREQ		1524000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((127 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_774M
#define CONFIG_SYS_MPLL_FREQ		1548000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((129 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_786M
#define CONFIG_SYS_MPLL_FREQ		1572000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((131 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_798M
#define CONFIG_SYS_MPLL_FREQ		1596000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((133 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_800M
#define CONFIG_SYS_MPLL_FREQ		1608000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((134 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_810M
#define CONFIG_SYS_MPLL_FREQ		1620000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((135 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_816M
#define CONFIG_SYS_MPLL_FREQ		1632000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((136 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_900M
#define CONFIG_SYS_MPLL_FREQ		1800000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((150 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#else
#error please define DDR_FREQ
#endif

#define CONFIG_SYS_VPLL_FREQ		1200000000	/*If VPLL not use mast be set 0*/
#define SEL_SCLKA		2
#define SEL_CPU			1
#define SEL_H0			2
#define SEL_H2			2

#ifdef DDR_400M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_450M
#define DIV_PCLK		8
#define DIV_H2			4
#define DIV_H0			4
#elif defined DDR_500M
#define DIV_PCLK		8
#define DIV_H2			4
#define DIV_H0			4
#elif defined DDR_540M
#define DIV_PCLK		8
#define DIV_H2			4
#define DIV_H0			4
#elif defined DDR_600M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_650M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_700M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_750M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_762M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_774M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_786M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_798M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_800M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_810M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_816M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_900M
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
#elif defined DDR_450M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_500M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_540M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_600M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_650M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_700M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_750M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_762M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_774M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_786M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_798M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_800M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_810M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_816M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_900M
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
