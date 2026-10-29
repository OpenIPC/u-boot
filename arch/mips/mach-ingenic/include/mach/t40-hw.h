/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Ingenic T40: caches, PLL, bus and DDR set-up, as the vendor's
 * include/configs/isvp_t40.h has it (OpenIPC/u-boot-t40). The SoC, the
 * package variant and the DDR chip are Kconfig options.
 *
 * Copyright (c) Ingenic Semiconductor Co.,Ltd
 */

#ifndef __MACH_INGENIC_T40_HW_H
#define __MACH_INGENIC_T40_HW_H

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

#if defined(CONFIG_T40N)
/* T40N */
#define APLL_912M
#define DDR_550M
#elif defined(CONFIG_T40A)
/* T40A */
#define APLL_1008M
#define DDR_700M
#else
/* T40XP */
#define APLL_1008M
#define DDR_700M
#endif

#ifdef APLL_300M
#define CONFIG_SYS_APLL_FREQ		300000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((50 << 20) | (1 << 14) | (4 << 11) | (1<<8))
#elif defined APLL_804M
#define CONFIG_SYS_APLL_FREQ		804000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((67 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined APLL_864M
#define CONFIG_SYS_APLL_FREQ		864000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((72 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined APLL_900M
#define CONFIG_SYS_APLL_FREQ		900000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((75 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined APLL_912M
#define CONFIG_SYS_APLL_FREQ		912000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((76 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined APLL_1000M
#define CONFIG_SYS_APLL_FREQ		1000000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((125 << 20) | (1 << 14) | (3 << 11) | (1<<8))
#elif defined APLL_1008M
#define CONFIG_SYS_APLL_FREQ		1008000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((84 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined APLL_1080M
#define CONFIG_SYS_APLL_FREQ		1080000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((90 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined APLL_1104M
#define CONFIG_SYS_APLL_FREQ		1104000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((92 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined APLL_1200M
#define CONFIG_SYS_APLL_FREQ		1200000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((100 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined APLL_1392M
#define CONFIG_SYS_APLL_FREQ		1392000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((116 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined APLL_1404M
#define CONFIG_SYS_APLL_FREQ		1404000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((117 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined APLL_1500M
#define CONFIG_SYS_APLL_FREQ		1500000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD		((125 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#else
#error please define APLL_FREQ
#endif

#ifdef DDR_300M
#define CONFIG_SYS_MPLL_FREQ		900000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD        ((75 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_400M
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
#elif defined DDR_550M
#define CONFIG_SYS_MPLL_FREQ		1100000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD        ((275 << 20) | (2 << 14) | (3 << 11) | (1<<8))
#elif defined DDR_570M
#define CONFIG_SYS_MPLL_FREQ		1140000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD        ((95 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_600M
#define CONFIG_SYS_MPLL_FREQ		1200000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((100 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_650M
#define CONFIG_SYS_MPLL_FREQ		1308000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((109 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_700M
#define CONFIG_SYS_MPLL_FREQ		1400000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((350 << 20) | (2 << 14) | (3 << 11) | (1<<8))
#elif defined DDR_750M
#define CONFIG_SYS_MPLL_FREQ		1500000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((125 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_762M
#define CONFIG_SYS_MPLL_FREQ		1524000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((127 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_798M
#define CONFIG_SYS_MPLL_FREQ		1596000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((133 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#elif defined DDR_800M
#define CONFIG_SYS_MPLL_FREQ		1608000000	/*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD		((134 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#else
#error please define DDR_FREQ
#endif

#define CONFIG_SYS_VPLL_FREQ		1008000000	/*If VPLL not use mast be set 0*/
#define CONFIG_SYS_VPLL_MNOD		((84 << 20) | (1 << 14) | (2 << 11) | (1<<8))
#define CONFIG_SYS_EPLL_FREQ		891000000	/*If EPLL not use mast be set 0*/
#define CONFIG_SYS_EPLL_MNOD		((297 << 20) | (2 << 14) | (4 << 11) | (1<<8))
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

#define CONFIG_SYS_EXTAL		    24000000	/* EXTAL freq: 48 MHz */

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
#define CONFIG_SYS_UART_INDEX       1
#define CONFIG_SYS_UART_CONTROLLER_STEP     0x1000

/* DDR */
/*
#define CONFIG_DDR_TEST
#define CONFIG_DDR_TEST_CPU
#define CONFIG_DDR_TEST_DMA
*/
#if defined(CONFIG_T40N)
/* T40N */
#define CONFIG_DDR_BUSWIDTH_32
#define CONFIG_DDR_DW32			1	/* 1-32bit-width, 0-16bit-width */
#elif defined(CONFIG_T40A)
/* T40A */
#define CONFIG_DDR_BUSWIDTH_32
#define CONFIG_DDR_DW32			1	/* 1-32bit-width, 0-16bit-width */
/*#define CONFIG_DDR3_M15T2G16128A_2R*/
/*#define CONFIG_DDR3_M15T8G16512A_2S*/
/*#define CONFIG_DDR3_W631GU6NG*/
#else
/* T40XP */
#define CONFIG_DDR_BUSWIDTH_32
#define CONFIG_DDR_DW32			1	/* 1-32bit-width, 0-16bit-width */
/*#define CONFIG_DDR3_W631GU6NG*/
#endif
/*#define CONFIG_DDR_DLL_OFF*/
#define CONFIG_DDR_CHIP_ODT
#define CONFIG_DDR_CS0			1	/* 1-connected, 0-disconnected */
#define CONFIG_DDR_CS1			0	/* 1-connected, 0-disconnected */

#if 0
#define CONFIG_OPEN_KGD_DRIVER_STRENGTH
#endif
#ifdef CONFIG_OPEN_KGD_DRIVER_STRENGTH
/* KGD driver strength is combined with MR1 A5 and A1 */
#define CONFIG_DDR_DRIVER_OUT_STRENGTH
#define CONFIG_DDR_DRIVER_OUT_STRENGTH_1 1
#define CONFIG_DDR_DRIVER_OUT_STRENGTH_0 0
#endif

#if 0
#define CONFIG_OPEN_KGD_ODT
#endif
#ifdef CONFIG_OPEN_KGD_ODT
/* KGD ODT is combined with RTT_Nom and RTT_WR */
#define CONFIG_DDR_CHIP_ODT_VAL
#define CONFIG_DDR_CHIP_ODT_VAL_RTT_NOM_9 0 /* RTT_Nom_9 is MR1 A9 bit */
#define CONFIG_DDR_CHIP_ODT_VAL_RTT_NOM_6 1 /* RTT_Nom_6 is MR1 A6 bit */
#define CONFIG_DDR_CHIP_ODT_VAL_RTT_NOM_2 0 /* RTT_Nom_2 is MR1 A2 bit */

#define CONFIG_DDR_CHIP_ODT_VAL_RTT_WR 0 /* RTT_WR is odt for KGD write of MR2*/
#endif

#define CONFIG_DDR_PHY_IMPEDANCE 40
#define CONFIG_DDR_PHY_ODT_IMPEDANCE 120
#if 0
#define CONFIG_DDR_SOFT_TRAINING
#else
#define CONFIG_DDR_HARDWARE_TRAINING
#endif
#define CONFIG_DDR_AUTO_SELF_REFRESH_CNT 257

/* MSC0 on port B, 4 bits, the SD card slot */
#define CONFIG_JZ_MMC_SPLMSC 0
#define CONFIG_JZ_MMC_MSC0_PB_4BIT

/* SFC: SPI NOR, the controller without the command descriptor table */
#define CONFIG_JZ_SFC
#define CONFIG_JZ_SFC_PA
#define CONFIG_SFC_MIN_ALIGN 0x10000

#ifdef CONFIG_INGENIC_SPI_NAND
/* SPI NAND on the SFC, as isvp_t40.h has it for the SFC_NAND targets */
#define CONFIG_SFC_NAND_RATE	50000000
#define CONFIG_SPI_NAND_BPP	(2048 + 64)	/* Bytes Per Page */
#define CONFIG_SPI_NAND_PPB	(64)		/* Page Per Block */
/* single-bit reads and loads, the chips' need_quad off */
#define CONFIG_SPI_STANDARD
#endif

/* GMAC, as isvp_t40.h has it: PHY_TYPE_IP101G is named but not defined */
#define GMAC_PHY_MII	1
#define GMAC_PHY_RMII	2
#define GMAC_PHY_GMII	3
#define GMAC_PHY_RGMII	4

#define PHY_TYPE_DM9161      1
#define PHY_TYPE_88E1111     2
#define PHY_TYPE_DP83867     3

#define CONFIG_NET_GMAC_PHY_MODE GMAC_PHY_RMII
#define CONFIG_NET_PHY_TYPE   PHY_TYPE_IP101G

#define CONFIG_NET_GMAC
#define CONFIG_GPIO_IP101G_RESET	GPIO_PC(7)
#define CONFIG_GPIO_IP101G_RESET_ENLEVEL	0

#define CONFIG_SYS_OST_FREQ         12500000        /* OST_FREQ 12.5MHz */

#endif
