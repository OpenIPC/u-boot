/*
 * MIPS register definitions, originally from:
 *
 * include/asm-mips/regdefs.h
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License, Version 2.  See the file "COPYING" in the main directory of this
 * archive for more details.
 *
 * Copyright (C) 1994, 1995 by Ralf Baechle
 */

/*
 * Symbolic register names for 32 bit ABI
 */
# define Mode_USR               0x10        /* M[4:0] = 01010 */
# define Mode_FIQ               0x11        /* M[4:0] = 10001 */
# define Mode_IRQ               0x12        /* M[4:0] = 10010 */
# define Mode_SVC               0x13        /* M[4:0] = 10011 */
# define Mode_ABT               0x17        /* M[4:0] = 10111 */
# define Mode_UNDEF             0x1B        /* M[4:0] = 11011 */
# define Mode_SYS               0x1F        /* M[4:0] = 11111 */
# define Mode_MSK               0x1F        /* M[4:0] = 11111 */
# define I_Bit                  0x80        /* 7  6  5  4  3  2  1  0 */
# define F_Bit                  0x40        /* I  F  T M4 M3 M2 M1 M0 */

/*
 ************************************************************************
 *    		Cache Size ID Register, CCSIDR (cp15, 1, c0, c0, 0)       	*
 ************************************************************************
 *
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |W|W|R|W|NumSets			 			 |Associativity      |L    |
 * |T|B|A|A|                             |                   |S    |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
#define S_CCSIDR_WT		31
#define M_CCSIDR_WT		(0x1  << S_CCSIDR_WT)		/* Support write-through */
#define S_CCSIDR_WB		30
#define M_CCSIDR_WB		(0x1  << S_CCSIDR_WB)		/* Support write-back */
#define S_CCSIDR_RA		29
#define M_CCSIDR_RA		(0x1  << S_CCSIDR_RA)		/* Support read-allocation */
#define S_CCSIDR_WA		28
#define M_CCSIDR_WA		(0x1  << S_CCSIDR_WA)		/* Support write-allocation */
#define S_CCSIDR_SETS	13
#define M_CCSIDR_SETS	(0x7fff  << S_CCSIDR_SETS)	/* Number of sets */
#define S_CCSIDR_A		3
#define M_CCSIDR_A		(0x3ff  << S_CCSIDR_A)		/* Number of associatiovity */
#define S_CCSIDR_LS		0
#define M_CCSIDR_LS		(0x7  << S_CCSIDR_LS)		/* Cache line size */

/*
 ************************************************************************
 *    		Cache Level ID Register, CLIDR (cp15, 1, c0, c0, 1)       	*
 ************************************************************************
 *
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |0|0|LoUU |LoC  |LoUIS|CT7  |CT6  |CT5  |CT4  |CT3  |CT2  |CT1  |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
#define S_CLIDR_LoUU	27
#define M_CLIDR_LoUU	(0x7 << S_CLIDR_LoUU)
#define S_CLIDR_LoC		24
#define M_CLIDR_LoC		(0x7 << S_CLIDR_LoC)	/* Cache cohernecy level */
#define S_CLIDR_LoUIS	21
#define M_CLIDR_LoUIS	(0x7 << S_CLIDR_LoUIS)
#define S_CLIDR_Ctype7	18
#define M_CLIDR_Ctype7	(0x7 << S_CLIDR_Ctype7)	/* Cache type of level 7 cache */
#define S_CLIDR_Ctype6	15
#define M_CLIDR_Ctype6	(0x7 << S_CLIDR_Ctype6)	/* Cache type of level 6 cache */
#define S_CLIDR_Ctype5	12
#define M_CLIDR_Ctype5	(0x7 << S_CLIDR_Ctype5)	/* Cache type of level 5 cache */
#define S_CLIDR_Ctype4	9
#define M_CLIDR_Ctype4	(0x7 << S_CLIDR_Ctype4)	/* Cache type of level 4 cache */
#define S_CLIDR_Ctype3	6
#define M_CLIDR_Ctype3	(0x7 << S_CLIDR_Ctype3)	/* Cache type of level 3 cache */
#define S_CLIDR_Ctype2	3
#define M_CLIDR_Ctype2	(0x7 << S_CLIDR_Ctype2)	/* Cache type of level 2 cache */
#define S_CLIDR_Ctype1	0
#define M_CLIDR_Ctype1	(0x7 << S_CLIDR_Ctype1)	/* Cache type of level 1 cache */

#define K_CLIDR_Ctype_NO	0	/* No cache */
#define K_CLIDR_Ctype_IC	1	/* Instruction cache only */
#define K_CLIDR_Ctype_DC	2	/* Data cache only */
#define K_CLIDR_Ctype_ID	3	/* Seperate instruction and data caches, Harvard cache */
#define K_CLIDR_Ctype_U		4	/* Unified cache, von Neumann cacge */

/*
 ************************************************************************
 *    	Cache Size Selection Register, CCSELR (cp15, 2, c0, c0, 0)      *
 ************************************************************************
 *
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |Reserved                                               |Level|I|
 * |                                                       |     |n|
 * |                                                       |     |D|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
#define S_CCSELR_Level	1
#define M_CCSELR_Level	(0x7  << S_CCSELR_Level)	/* Cache level */
#define S_CCSELR_InD	0
#define M_CCSELR_InD	(0x1  << S_CCSELR_InD)		/* I$ or D$ */

/*
 ************************************************************************
 *    		System Control Register, SCTLR (cp15, 0, c1, c0, 0)         *
 ************************************************************************
 *
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |R|T|A|T|N|0|E|V|1|U|F|U|W|1|H|1|0|R|V|I|Z|S|0|0|B|1|B|1|1|C|A|M|
 * | |E|F|R|M| |E|E| | |I|W|X| |A| | | | | | |W| | | | |E| | | | | |
 * | | |E|E|F| | | | | | |X|N| | | | | | | | | | | | | |N| | | | | |
 * | | | | |I| | | | | | |N| | | | | | | | | | | | | | | | | | | | |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
#define S_SCTLR_TE		30
#define M_SCTLR_TE		(0x1 << S_SCTLR_TE)			/* Thumb exception enable */
#define S_SCTLR_AFE		29
#define M_SCTLR_AFE		(0x1 << S_SCTLR_AFE)		/* Access flag enable */
#define S_SCTLR_TRE		28
#define M_SCTLR_TRE		(0x1 << S_SCTLR_TRE)		/* TXE remap enable */
#define S_SCTLR_NMFI	27
#define M_SCTLR_NMFI	(0x1 << S_SCTLR_NMFI)		/* None-maskable FIQ enbale (RO) */
#define S_SCTLR_EE		25
#define M_SCTLR_EE		(0x1 << S_SCTLR_EE)			/* Exception Endianness: big-endian */
#define S_SCTLR_VE		24
#define M_SCTLR_VE		(0x1 << S_SCTLR_VE)			/* Interrupt Vector Enable */
#define S_SCTLR_FI		21
#define M_SCTLR_FI		(0x1 << S_SCTLR_FI)			/* Fast interrupt configuraiont enable */
#define S_SCTLR_UWXN	20
#define M_SCTLR_UWXN	(0x1 << S_SCTLR_UWXN)		/* */
#define S_SCTLR_WXN		19
#define M_SCTLR_WXN		(0x1 << S_SCTLR_WXN)
#define S_SCTLR_HA		17
#define M_SCTLR_HA		(0x1 << S_SCTLR_HA)			/* Hardware access flag enable */
#define S_SCTLR_RR		14
#define M_SCTLR_RR		(0x1 << S_SCTLR_RR)			/* Round-robin select */
#define S_SCTLR_V		13
#define M_SCTLR_V		(0x1 << S_SCTLR_V)			/* Vector bits */
#define S_SCTLR_I		12
#define M_SCTLR_I		(0x1 << S_SCTLR_I)			/* Instruction cache enable */
#define S_SCTLR_Z		11
#define M_SCTLR_Z		(0x1 << S_SCTLR_Z)			/* Branch prediction enable */
#define S_SCTLR_SW		10
#define M_SCTLR_SW		(0x1 << S_SCTLR_SW)			/* SW and SWP enable */
#define S_SCTLR_CP15BEN	5
#define M_SCTLR_CP15BEN	(0x1 << S_SCTLR_CP15BEN)	/* CP15 barrier support */
#define S_SCTLR_C		2
#define M_SCTLR_C		(0x1 << S_SCTLR_C)			/* Data and Unidied cache enable */
#define S_SCTLR_A		1
#define M_SCTLR_A		(0x1 << S_SCTLR_A)			/* Alignment fault checking enable */
#define S_SCTLR_M		0
#define M_SCTLR_M		(0x1 << S_SCTLR_M)			/* PL1&0 stage MMU enable */

/************************************************************************
 *    		Auxiliary Control Register, ACTLR (cp15, 0, c1, c0, 1)      *
 ************************************************************************
 *
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |UNP/SBZP                                   |P|A|E|S|RAZ|W|L|L|F|
 * |                                           |a|l|X|M|   | |1|2|W|
 * |                                           |r|l|C|P|   |f| | | |
 * |                                           | |o|L| |   |l|p|p| |
 * |                                           |o|c| | |   | |r|r| |
 * |                                           |n| | | |   |z|e|e| |
 * |                                           | |o| | |   |e|f|f| |
 * |                                           | |n| | |   |r| | | |
 * |                                           | |e| | |   |o|e|e| |
 * |                                           | | | | |   |s|n|n| |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
#define S_ACTLR_PAR_ON		9
#define M_ACTLR_PAR_ON		(0x1 << S_ACTLR_PAR_ON)		/* Parity on */
#define S_ACTLR_ALLOC_ONE	8
#define M_ACTLR_ALLOC_ONE	(0x1 << S_ACTLR_ALLOC_ONE)	/* Alloc in one way */
#define S_ACTLR_EXCL		7
#define M_ACTLR_EXCL		(0x1 << S_ACTLR_EXCL)		/* Exclusive cache */
#define S_ACTLR_SMP			6
#define M_ACTLR_SMP			(0x1 << S_ACTLR_SMP)		/* SMP */
#define S_ACTLR_W_FL_ZEROS	3
#define M_ACTLR_W_FL_ZEROS	(0x1 << S_ACTLR_W_FL_ZEROS)	/* Write full line of zeros mode */
#define S_ACTLR_L1_PERF_EN	2
#define M_ACTLR_L1_PERF_EN	(0x1 << S_ACTLR_L1_PERF_EN)	/* Dside prefetch enable */
#define S_ACTLR_L2_PERF_EN	1
#define M_ACTLR_L2_PERF_EN	(0x1 << S_ACTLR_L2_PERF_EN)	/* L2 prefetch enable */
#define S_ACTLR_FW			0
#define M_ACTLR_FW			(0x1 << S_ACTLR_FW)			/* Cache and TLB maintenance boardcast */


