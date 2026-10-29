/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Ingenic T41: caches, PLL, bus and DDR set-up, as the vendor's
 * include/configs/isvp_t41.h has it (Ingenic T41 SDK 1.2.0). The SoC, the
 * package variant and the DDR chip are Kconfig options.
 *
 * Copyright (c) Ingenic Semiconductor Co.,Ltd
 */

#ifndef __MACH_INGENIC_T41_HW_H
#define __MACH_INGENIC_T41_HW_H

#define CONFIG_K0BASE		0x80000000
#define CONFIG_L1CACHE_SIZE		    (32 * 1024)	    /* 32K dcache and icache */
#define CONFIG_L1CACHELINE_SIZE	    (32)		    /* 32bytes dcache and icache line size */
#define CONFIG_L1CACHE_SETS		    (128)		    /* 128 dcache and icache sets */
#define CONFIG_L1CACHE_WAYNUM		(8)		        /* 8 way dcache and icache associativity */
#define CONFIG_L1CACHE_TAG_MASK	    ((~(((CONFIG_L1CACHE_SIZE) / (CONFIG_L1CACHE_WAYNUM)) - 1)) & 0x7fffffff)
#define CONFIG_L2CACHE_REG		    ((*((volatile unsigned int *)(0xb2200060)) >> 10) & 0x7)				/* reg:0xb2200060 */
#define CONFIG_L2CACHE_SIZE		    (CONFIG_L2CACHE_REG ? ((128 << (CONFIG_L2CACHE_REG - 1)) * 1024) : 0)	/* set l2cache size */
#define CONFIG_L2CACHE_SETS		    (128)		    /* 128 l2cache sets */
#define CONFIG_L2CACHELINE_SIZE	    (64)		    /* xburst2: 64bytes l2cache line size */
#define CONFIG_L2CACHE_WAYNUM	    (8)		        /* 8 way l2cache associativity */
#define CONFIG_STACK_SIZE		    CONFIG_L1CACHE_SIZE

/*#define CONFIG_FAST_BOOT*/
#if defined(CONFIG_T41N)
/* T41N */
#if defined(CONFIG_LP)
#define APLL_1008M
#define DDR_550M
#else
#define APLL_1104M
#define DDR_750M
#endif
#elif defined(CONFIG_T41L) || defined(CONFIG_T41LQ)
/* T41L & T41LQ */
#if defined(CONFIG_LP)
#define APLL_1008M
#define DDR_550M
#else
#define APLL_1104M
#define DDR_600M
#endif
#elif defined(CONFIG_T41NQ)
/* T41NQ */
#if defined(CONFIG_LP)
#define APLL_1008M
#define DDR_600M
#else
#define APLL_1104M
#define DDR_700M
#endif
#elif defined(CONFIG_T41ZX)
/* T41ZX */
#if defined(CONFIG_LP)
#define APLL_1008M
#define DDR_550M
#else
#define APLL_1104M
#define DDR_700M
#endif
#elif defined(CONFIG_T41XQ)
/* T41XQ */
#if defined(CONFIG_LP)
#define APLL_1008M
#define DDR_550M
#else
#define APLL_1104M
#define DDR_700M
#endif
#elif defined(CONFIG_T41A)
/* T41A */
#define APLL_1300M
#define DDR_800M
#endif

#ifdef APLL_804M
#define CONFIG_SYS_APLL_FREQ		804000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		(((0x42) << 20) | ((0x0) << 14) | ((0x1) << 11) | ((0x1) << 7) | ((0x3) << 4))
#elif defined APLL_864M
#define CONFIG_SYS_APLL_FREQ		864000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		(((0x47) << 20) | ((0x0) << 14) | ((0x1) << 11) | ((0x1) << 7) | ((0x3) << 4))
#elif defined APLL_900M
#define CONFIG_SYS_APLL_FREQ		900000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		(((0x4a) << 20) | ((0x0) << 14) | ((0x1) << 11) | ((0x1) << 7) | ((0x3) << 4))
#elif defined APLL_1000M
#define CONFIG_SYS_APLL_FREQ		1000000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		(((0x7c) << 20) | ((0x0) << 14) | ((0x1) << 11) | ((0x2) << 7) | ((0x3) << 4))
#elif defined APLL_1008M
#define CONFIG_SYS_APLL_FREQ		1008000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		(((0x53) << 20) | ((0x0) << 14) | ((0x1) << 11) | ((0x1) << 7) | ((0x3) << 4))
#elif defined APLL_1104M
#define CONFIG_SYS_APLL_FREQ		1104000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		(((0x5b) << 20) | ((0x0) << 14) | ((0x1) << 11) | ((0x1) << 7) | ((0x3) << 4))
#elif defined APLL_1200M
#define CONFIG_SYS_APLL_FREQ		1200000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		(((0x63) << 20) | ((0x0) << 14) | ((0x1) << 11) | ((0x1) << 7) | ((0x3) << 4))
#elif defined APLL_1300M
#define CONFIG_SYS_APLL_FREQ		1300000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		(((0x144) << 20) | ((0x2) << 14) | ((0x1) << 11) | ((0x1) << 7) | ((0x1) << 4))
#elif defined APLL_1400M
#define CONFIG_SYS_APLL_FREQ		1400000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		(((0x15d) << 20) | ((0x2) << 14) | ((0x1) << 11) | ((0x1) << 7) | ((0x1) << 4))
#elif defined APLL_1500M
#define CONFIG_SYS_APLL_FREQ		1500000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		(((0x7c) << 20) | ((0x0) << 14) | ((0x1) << 11) | ((0x1) << 7) | ((0x3) << 4))
#elif defined APLL_1600M
#define CONFIG_SYS_APLL_FREQ		1600000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		(((0xc7) << 20) | ((0x2) << 14) | ((0x1) << 11) | ((0x0) << 7) | ((0x1) << 4))
#else
#error please define APLL_FREQ
#endif

#ifdef DDR_300M
#define CONFIG_SYS_MPLL_FREQ		900000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD        (((0x4a) << 20) | ((0x0) << 14) | ((0x1) << 11) | ((0x1) << 7) | ((0x3) << 4))
#elif defined DDR_400M
#define CONFIG_SYS_MPLL_FREQ		1200000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD	    (((0x63) << 20) | ((0x0) << 14) | ((0x1) << 11) | ((0x1) << 7) | ((0x3) << 4))
#elif defined DDR_450M
#define CONFIG_SYS_MPLL_FREQ		900000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD        (((0x4a) << 20) | ((0x0) << 14) | ((0x1) << 11) | ((0x1) << 7) | ((0x3) << 4))
#elif defined DDR_500M
#define CONFIG_SYS_MPLL_FREQ		1000000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD        (((0x7c) << 20) | ((0x0) << 14) | ((0x1) << 11) | ((0x2) << 7) | ((0x3) << 4))
#elif defined DDR_540M
#define CONFIG_SYS_MPLL_FREQ		1080000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD        (((0x59) << 20) | ((0x0) << 14) | ((0x1) << 11) | ((0x1) << 7) | ((0x3) << 4))
#elif defined DDR_550M
#define CONFIG_SYS_MPLL_FREQ		1100000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD        (((0x112) << 20) | ((0x2) << 14) | ((0x1) << 11) | ((0x1) << 7) | ((0x1) << 4))
#elif defined DDR_570M
#define CONFIG_SYS_MPLL_FREQ		1140000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD        (((0x5e) << 20) | ((0x0) << 14) | ((0x1) << 11) | ((0x1) << 7) | ((0x3) << 4))
#elif defined DDR_600M
#define CONFIG_SYS_MPLL_FREQ		1200000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		(((0x63) << 20) | ((0x0) << 14) | ((0x1) << 11) | ((0x1) << 7) | ((0x3) << 4))
#elif defined DDR_650M
#define CONFIG_SYS_MPLL_FREQ		1300000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		(((0x144) << 20) | ((0x2) << 14) | ((0x1) << 11) | ((0x1) << 7) | ((0x1) << 4))
#elif defined DDR_700M
#define CONFIG_SYS_MPLL_FREQ		1400000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		(((0x15d) << 20) | ((0x2) << 14) | ((0x1) << 11) | ((0x1) << 7) | ((0x1) << 4))
#elif defined DDR_750M
#define CONFIG_SYS_MPLL_FREQ		1500000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		(((0x176) << 20) | ((0x2) << 14) | ((0x1) << 11) | ((0x1) << 7) | ((0x1) << 4))
#elif defined DDR_800M
#define CONFIG_SYS_MPLL_FREQ		1600000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		(((0xc7) << 20) | ((0x2) << 14) | ((0x1) << 11) | ((0x0) << 7) | ((0x1) << 4))
#elif defined DDR_852M
#define CONFIG_SYS_MPLL_FREQ		1704000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		(((0x46) << 20) | ((0x0) << 14) | ((0x1) << 11) | ((0x0) << 7) | ((0x3) << 4))
#elif defined DDR_900M
#define CONFIG_SYS_MPLL_FREQ		1800000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		(((0x4a) << 20) | ((0x0) << 14) | ((0x1) << 11) | ((0x0) << 7) | ((0x3) << 4))
#elif defined DDR_1000M
#define CONFIG_SYS_MPLL_FREQ		2000000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		(((0xf9) << 20) | ((0x2) << 14) | ((0x1) << 11) | ((0x0) << 7) | ((0x1) << 4))
#else
#error please define DDR_FREQ
#endif

#define CONFIG_SYS_VPLL_FREQ		1188000000	/*If VPLL not use mast be set 0*/
#define CONFIG_SYS_VPLL_MNOD        ((98 << 20) | (0 << 14) | (1 << 11) | (1 << 7) | (3 << 4))

#define SEL_SCLKA		2
#define SEL_CPU			1
#define SEL_H0			2
#define SEL_H2			2

#ifdef DDR_300M
#define DIV_PCLK		8
#define DIV_H2			4
#define DIV_H0			4
#elif defined DDR_400M
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
#elif defined DDR_550M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_570M
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
#elif defined DDR_792M
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
#elif defined DDR_828M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_840M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_852M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_864M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_876M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_888M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_900M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#elif defined DDR_1000M
#define DIV_PCLK		12
#define DIV_H2			6
#define DIV_H0			6
#else
#error please define DDR_FREQ
#endif

#define DIV_L2			3
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

#ifdef DDR_300M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 3)
#elif defined DDR_400M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 3)
#elif defined DDR_450M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_500M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_540M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_550M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_570M
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
#elif defined DDR_792M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_798M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_800M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_810M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_816M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_828M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_840M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_852M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_864M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_876M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_888M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_900M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#elif defined DDR_1000M
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_MPLL_FREQ / 2)
#else
#error please define DDR_FREQ
#endif

#define CONFIG_SYS_EXTAL		    24000000	/* EXTAL freq: 24 MHz */

/* CACHE */
#define CONFIG_SYS_DCACHELINE_SIZE	CONFIG_L1CACHELINE_SIZE
#define CONFIG_SYS_DCACHE_WAYS		CONFIG_L1CACHE_WAYNUM
#define CONFIG_SYS_ICACHELINE_SIZE	CONFIG_L1CACHELINE_SIZE
#define CONFIG_SYS_ICACHE_WAYS		CONFIG_L1CACHE_WAYNUM
#define CONFIG_SYS_CACHELINE_SIZE	CONFIG_L1CACHELINE_SIZE
#define CONFIG_BOARD_L2CACHE
#define CONFIG_SYS_L2CACHE_SIZE		CONFIG_L2CACHE_SIZE
#define CONFIG_SYS_L2CACHELINE_SIZE	CONFIG_L2CACHELINE_SIZE
#define CONFIG_SYS_L2CACHE_WAYS		CONFIG_L2CACHE_WAYNUM

/* UART */
#define CONFIG_SYS_UART_INDEX               1
#define CONFIG_SYS_UART_CONTROLLER_STEP     0x1000

#if defined(CONFIG_T41N) || defined(CONFIG_T41NQ)
/* T41N */
#define CONFIG_DDR_DW32                        0       /* 1-32bit-width, 0-16bit-width */
#elif defined(CONFIG_T41L) || defined(CONFIG_T41LQ)
/* T41L */
#define CONFIG_DDR_DW32                        0       /* 1-32bit-width, 0-16bit-width */
#elif defined (CONFIG_T41A)
/* T41A */
#define CONFIG_DDR_DW32                        0       /* 1-32bit-width, 0-16bit-width */
#elif defined (CONFIG_T41ZX)
/* T41ZX */
#define CONFIG_DDR_DW32                        0       /* 1-32bit-width, 0-16bit-width */
#elif defined (CONFIG_T41XQ)
/* T41XQ */
#define CONFIG_DDR_DW32                        0       /* 1-32bit-width, 0-16bit-width */
#else
#define CONFIG_DDR_DW32                        0       /* 1-32bit-width, 0-16bit-width */
#endif

/*#define CONFIG_DDR_DLL_OFF*/
#define CONFIG_DDR_CHIP_ODT
#define CONFIG_DDR_CS0			1	/* 1-connected, 0-disconnected */
#define CONFIG_DDR_CS1			0	/* 1-connected, 0-disconnected */

#define CONFIG_DDR_HARDWARE_TRAINING

/* KGD driver strength is combined with MR1 A5 and A1 */
/* #define CONFIG_OPEN_KGD_DRIVER_STRENGTH */
#ifdef CONFIG_OPEN_KGD_DRIVER_STRENGTH
#if defined(CONFIG_T41N)
#define CONFIG_DDR_DRIVER_OUT_STRENGTH
#define CONFIG_DDR_DRIVER_OUT_STRENGTH_1 1
#define CONFIG_DDR_DRIVER_OUT_STRENGTH_0 0
#endif
#endif

/* KGD ODT is combined with RTT_Nom and RTT_WR */
/* #define CONFIG_OPEN_KGD_ODT */
#ifdef CONFIG_OPEN_KGD_ODT
#if defined(CONFIG_T41N)
#define CONFIG_DDR_CHIP_ODT_VAL
#define CONFIG_DDR_CHIP_ODT_VAL_RTT_NOM_9 0 /* RTT_Nom_9 is MR1 A9 bit */
#define CONFIG_DDR_CHIP_ODT_VAL_RTT_NOM_6 0 /* RTT_Nom_6 is MR1 A6 bit */
#define CONFIG_DDR_CHIP_ODT_VAL_RTT_NOM_2 1 /* RTT_Nom_2 is MR1 A2 bit */
#define CONFIG_DDR_CHIP_ODT_VAL_RTT_WR 0 /* RTT_WR is odt for KGD write of MR2*/
#endif
#endif

#define CONFIG_DDR_PHY_IMPEDANCE 40
#define CONFIG_DDR_PHY_ODT_IMPEDANCE 120
#define CONFIG_DDR_AUTO_SELF_REFRESH_CNT 257

/* MSC0 on port B, 4 bits, the SD card slot */
#define CONFIG_JZ_MMC_SPLMSC 0
#define CONFIG_JZ_MMC_MSC0_PB_4BIT

/*
 * SFC: SPI NOR on SFC0, as the isvp_t41*_sfc_nor targets have it
 * (SFC0_NOR and SFC1_NOR, SFC0 taking precedence), with both
 * controllers' pins set up
 */
#define CONFIG_JZ_SFC
#define CONFIG_SFC0_NOR			1
#define CONFIG_SFC1_NOR			1
#define CONFIG_JZ_SFC0_PA
#define CONFIG_JZ_SFC1_PA
#define CONFIG_SFC_MIN_ALIGN			0x1000	/*0x1000->4K Erase,0x8000->32k 0x10000->64k*/

/* GMAC */
#define GMAC_PHY_MII	1
#define GMAC_PHY_RMII	2
#define GMAC_PHY_GMII	3
#define GMAC_PHY_RGMII	4

#define PHY_TYPE_DM9161      1
#define PHY_TYPE_88E1111     2
#define PHY_TYPE_DP83867     3
#define PHY_TYPE_LAN8720     4

#define CONFIG_NET_GMAC_PHY_MODE    GMAC_PHY_RMII
#define CONFIG_NET_PHY_TYPE         PHY_TYPE_LAN8720

#define CONFIG_NET_GMAC

#define CONFIG_GPIO_PHY_RESET	GPIO_PB(28)
#define CONFIG_GPIO_PHY_RESET_ENLEVEL	0

/* OST */
#define CONFIG_SYS_OST_FREQ	12500000        /* OST_FREQ 12.5MHz */

#endif
