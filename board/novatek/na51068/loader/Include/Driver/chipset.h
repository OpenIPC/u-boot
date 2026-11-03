# ifndef _CHIPSET_H
# define _CHIPSET_H

/* Copyright (c) 2005  Novatek Microelectronics Corporation

COMMENT Module Name:
COMMENT     Chipset.h

COMMENT Abstract:
COMMENT     Definitions for CPU Mode, Interrupt and Stack
COMMENT     This is a shared header file for ASM and C
COMMENT     For more information, please refer to http://www.arm.com/support/faqdev/1208.html
COMMENT     Only support "# define" now

COMMENT Notes:
COMMENT     Copyright (c) 2005 Novatek Microelectronics Corporation.  All Rights Reserved.

COMMENT Revision History:
COMMENT     03/11/05: Created by Chris Hsu */

/* CPU */


#ifndef KNL_SUPPORT_ISR_DUMP_INTERVAL
#define KNL_SUPPORT_ISR_DUMP_INTERVAL       0
#endif

#ifndef KNL_SUPPORT_TASK_DUMP_UTILIZATION
#define KNL_SUPPORT_TASK_DUMP_UTILIZATION   0
#endif


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

//# define INT_CTRL_ADDR         	(0xF0080000)
//# define TIMER_CTRL_ADDR       	(0xF0040000)
# define L2_MEM_BASE			(0xf9C20000)
/*
 ************************************************************************
 *          System Control Register, SCTLR (cp15, 0, c1, c0, 0)         *
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
#define S_SCTLR_TE      30
#define M_SCTLR_TE      (0x1 << S_SCTLR_TE)         /* Thumb exception enable */
#define S_SCTLR_AFE     29
#define M_SCTLR_AFE     (0x1 << S_SCTLR_AFE)        /* Access flag enable */
#define S_SCTLR_TRE     28
#define M_SCTLR_TRE     (0x1 << S_SCTLR_TRE)        /* TXE remap enable */
#define S_SCTLR_NMFI    27
#define M_SCTLR_NMFI    (0x1 << S_SCTLR_NMFI)       /* None-maskable FIQ enbale (RO) */
#define S_SCTLR_EE      25
#define M_SCTLR_EE      (0x1 << S_SCTLR_EE)         /* Exception Endianness: big-endian */
#define S_SCTLR_VE      24
#define M_SCTLR_VE      (0x1 << S_SCTLR_VE)         /* Interrupt Vector Enable */
#define S_SCTLR_FI      21
#define M_SCTLR_FI      (0x1 << S_SCTLR_FI)         /* Fast interrupt configuraiont enable */
#define S_SCTLR_UWXN    20
#define M_SCTLR_UWXN    (0x1 << S_SCTLR_UWXN)       /* */
#define S_SCTLR_WXN     19
#define M_SCTLR_WXN     (0x1 << S_SCTLR_WXN)
#define S_SCTLR_HA      17
#define M_SCTLR_HA      (0x1 << S_SCTLR_HA)         /* Hardware access flag enable */
#define S_SCTLR_RR      14
#define M_SCTLR_RR      (0x1 << S_SCTLR_RR)         /* Round-robin select */
#define S_SCTLR_V       13
#define M_SCTLR_V       (0x1 << S_SCTLR_V)          /* Vector bits */
#define S_SCTLR_I       12
#define M_SCTLR_I       (0x1 << S_SCTLR_I)          /* Instruction cache enable */
#define S_SCTLR_Z       11
#define M_SCTLR_Z       (0x1 << S_SCTLR_Z)          /* Branch prediction enable */
#define S_SCTLR_SW      10
#define M_SCTLR_SW      (0x1 << S_SCTLR_SW)         /* SW and SWP enable */
#define S_SCTLR_CP15BEN 5
#define M_SCTLR_CP15BEN (0x1 << S_SCTLR_CP15BEN)    /* CP15 barrier support */
#define S_SCTLR_C       2
#define M_SCTLR_C       (0x1 << S_SCTLR_C)          /* Data and Unidied cache enable */
#define S_SCTLR_A       1
#define M_SCTLR_A       (0x1 << S_SCTLR_A)          /* Alignment fault checking enable */
#define S_SCTLR_M       0
#define M_SCTLR_M       (0x1 << S_SCTLR_M)          /* PL1&0 stage MMU enable */

/************************************************************************
 *          Auxiliary Control Register, ACTLR (cp15, 0, c1, c0, 1)      *
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
#define S_ACTLR_PAR_ON      9
#define M_ACTLR_PAR_ON      (0x1 << S_ACTLR_PAR_ON)     /* Parity on */
#define S_ACTLR_ALLOC_ONE   8
#define M_ACTLR_ALLOC_ONE   (0x1 << S_ACTLR_ALLOC_ONE)  /* Alloc in one way */
#define S_ACTLR_EXCL        7
#define M_ACTLR_EXCL        (0x1 << S_ACTLR_EXCL)       /* Exclusive cache */
#define S_ACTLR_SMP         6
#define M_ACTLR_SMP         (0x1 << S_ACTLR_SMP)        /* SMP */
#define S_ACTLR_W_FL_ZEROS  3
#define M_ACTLR_W_FL_ZEROS  (0x1 << S_ACTLR_W_FL_ZEROS) /* Write full line of zeros mode */
#define S_ACTLR_L1_PERF_EN  2
#define M_ACTLR_L1_PERF_EN  (0x1 << S_ACTLR_L1_PERF_EN) /* Dside prefetch enable */
#define S_ACTLR_L2_PERF_EN  1
#define M_ACTLR_L2_PERF_EN  (0x1 << S_ACTLR_L2_PERF_EN) /* L2 prefetch enable */
#define S_ACTLR_FW          0
#define M_ACTLR_FW          (0x1 << S_ACTLR_FW)         /* Cache and TLB maintenance boardcast */

#define L2_REG0_BASE	(L2_MEM_BASE + 0x000)	/* Cache ID and Cache Type */
#define L2_REG1_BASE	(L2_MEM_BASE + 0x100)	/* Control */
#define L2_REG2_BASE	(L2_MEM_BASE + 0X200)	/* Interrupt and Counter Control Registers */
#define L2_REG7_BASE	(L2_MEM_BASE + 0x700)	/* Cache Maintenance Operations */
#define L2_REG9_BASE	(L2_MEM_BASE + 0x900)	/* Cache Lockdown */
#define L2_REG12_BASE	(L2_MEM_BASE + 0xC00)	/* Address Filtering */
#define L2_REG15_BASE	(L2_MEM_BASE + 0xF00)	/* Debug, Prefetch and Power */

/**
 * Cache ID and Cache Type
 */
#define L2_REG0_CACHE_ID				(*((volatile unsigned long *)(L2_REG0_BASE + 0x00)))
#define L2_REG0_CACHE_TYPE				(*((volatile unsigned long *)(L2_REG0_BASE + 0x04)))

#define S_L2_REG0_CACHE_TYPE_DB			(31)
#define M_L2_REG0_CACHE_TYPE_DB			(0xf << S_L2_REG0_CACHE_TYPE_DB)
#define S_L2_REG0_CACHE_TYPE_CTYPE		(25)
#define M_L2_REG0_CACHE_TYPE_CTYPE		(0xf << S_L2_REG0_CACHE_TYPE_CTYPE)
#define S_L2_REG0_CACHE_TYPE_H			(24)
#define M_L2_REG0_CACHE_TYPE_H			(0x1 << S_L2_REG0_CACHE_TYPE_H)
#define S_L2_REG0_CACHE_TYPE_DWS		(20)
#define M_L2_REG0_CACHE_TYPE_DWS		(0x7 << S_L2_REG0_CACHE_TYPE_DWS)
#define S_L2_REG0_CACHE_TYPE_DA			(18)
#define M_L2_REG0_CACHE_TYPE_DA			(0x1 << S_L2_REG0_CACHE_TYPE_DA)
#define S_L2_REG0_CACHE_TYPE_DLS		(12)
#define M_L2_REG0_CACHE_TYPE_DLS		(0x3 << S_L2_REG0_CACHE_TYPE_DLS)
#define S_L2_REG0_CACHE_TYPE_IWS		(8)
#define M_L2_REG0_CACHE_TYPE_IWS		(0x7 << S_L2_REG0_CACHE_TYPE_IWS)
#define S_L2_REG0_CACHE_TYPE_IA			(6)
#define M_L2_REG0_CACHE_TYPE_IA			(0x1 << S_L2_REG0_CACHE_TYPE_IA)
#define S_L2_REG0_CACHE_TYPE_ILS		(0)
#define M_L2_REG0_CACHE_TYPE_ILS		(0x3 << S_L2_REG0_CACHE_TYPE_ILS)

#define K_L2_REG0_CACHE_TYPE_DA_16WAY	(1)
#define K_L2_REG0_CACHE_TYPE_DA_8WAY	(0)


/**
 * Control
 */
#define L2_REG1_CONTROL					(*((volatile unsigned long *)(L2_REG1_BASE + 0x00)))
#define L2_REG1_AUX_CTRL				(*((volatile unsigned long *)(L2_REG1_BASE + 0x04)))
#define L2_REG1_TAG_RAM_CTRL			(*((volatile unsigned long *)(L2_REG1_BASE + 0x08)))
#define L2_REG1_DATA_RAM_CTRL			(*((volatile unsigned long *)(L2_REG1_BASE + 0x0C)))

#define S_L2_REG1_CONTROL_EN	(0)
#define M_L2_REG1_CONTROL_EN	(0x1 << S_L2_REG1_CONTROL_EN)

#define K_L2_REG1_CONTROL_EN_ON		1
#define K_L2_REG1_CONTROL_EN_OFF	0

#define S_L2_REG1_AUX_CTRL_BRESP						(30)
#define M_L2_REG1_AUX_CTRL_BRESP						(0x1 << S_L2_REG1_AUX_CTRL_BRESP)
#define S_L2_REG1_AUX_CTRL_INSTR_PREF					(29)
#define M_L2_REG1_AUX_CTRL_INSTR_PREF					(0x1 << S_L2_REG1_AUX_CTRL_INSTR_PREF)
#define S_L2_REG1_AUX_CTRL_DATA_PERF					(28)
#define M_L2_REG1_AUX_CTRL_DATA_PERF					(0x1 << S_L2_REG1_AUX_CTRL_DATA_PERF)
#define S_L2_REG1_AUX_CTRL_NS_INT_CTRL					(27)
#define M_L2_REG1_AUX_CTRL_NS_INT_CTRL					(0x1 << S_L2_REG1_AUX_CTRL_NS_INT_CTRL)
#define S_L2_REG1_AUX_CTRL_NS_LOCK_EN					(26)
#define M_L2_REG1_AUX_CTRL_NS_LOCK_EN					(0x1 << S_L2_REG1_AUX_CTRL_NS_LOCK_EN)
#define S_L2_REG1_AUX_CTRL_CACHE_POLICY					(25)
#define M_L2_REG1_AUX_CTRL_CACHE_POLICY					(0x1 << S_L2_REG1_AUX_CTRL_CACHE_POLICY)
#define S_L2_REG1_AUX_CTRL_FORCE_WA						(23)
#define M_L2_REG1_AUX_CTRL_FORCE_WA						(0x3 << S_L2_REG1_AUX_CTRL_FORCE_WA)
#define S_L2_REG1_AUX_CTRL_SHARED_OVERRIDE_EN			(22)
#define M_L2_REG1_AUX_CTRL_SHARED_OVERRIDE_EN			(0x1 << S_L2_REG1_AUX_CTRL_SHARED_OVERRIDE_EN)
#define S_L2_REG1_AUX_CTRL_PARITY_EN					(21)
#define M_L2_REG1_AUX_CTRL_PARITY_EN					(0x1 << S_L2_REG1_AUX_CTRL_PARITY_EN)
#define S_L2_REG1_AUX_CTRL_EVENT_MON_BUD_EN				(20)
#define M_L2_REG1_AUX_CTRL_EVENT_MON_BUD_EN				(0x1 << S_L2_REG1_AUX_CTRL_EVENT_MON_BUD_EN)
#define S_L2_REG1_AUX_CTRL_WAT_SIZE						(17)
#define M_L2_REG1_AUX_CTRL_WAT_SIZE						(0x7 << S_L2_REG1_AUX_CTRL_WAT_SIZE)
#define S_L2_REG1_AUX_CTRL_ASSOCIATIVITY				(16)
#define M_L2_REG1_AUX_CTRL_ASSOCIATIVITY				(0x1 << S_L2_REG1_AUX_CTRL_ASSOCIATIVITY)
#define S_L2_REG1_AUX_CTRL_SHARED_INV_EN				(13)
#define M_L2_REG1_AUX_CTRL_SHARED_INV_EN				(0x1 << S_L2_REG1_AUX_CTRL_SHARED_INV_EN)
#define S_L2_REG1_AUX_CTRL_EXCLUSIVE_CACHE_CONF			(12)
#define M_L2_REG1_AUX_CTRL_EXCLUSIVE_CACHE_CONF			(0x1 << S_L2_REG1_AUX_CTRL_EXCLUSIVE_CACHE_CONF)
#define S_L2_REG1_AUX_CTRL_STORE_BUD_DEV_LIMIT_EN		(11)
#define M_L2_REG1_AUX_CTRL_STORE_BUD_DEV_LIMIT_EN		(0x1 << S_L2_REG1_AUX_CTRL_STORE_BUD_DEV_LIMIT_EN)
#define S_L2_REG1_AUX_CTRL_HIGH_PRIO_SO_DEV_READS_EN	(10)
#define M_L2_REG1_AUX_CTRL_HIGH_PRIO_SO_DEV_READS_EN	(0x1 << S_L2_REG1_AUX_CTRL_HIGH_PRIO_SO_DEV_READS_EN)
#define S_L2_REG1_AUX_CTRL_FULL_LINE_Z_EN				(0)
#define M_L2_REG1_AUX_CTRL_FULL_LINE_Z_EN				(0x1 << S_L2_REG1_AUX_CTRL_FULL_LINE_Z_EN)

#define reg0_cache_type_ofs		0x004			//reg0_cache_type
#define CACHE_CTRL_REG_OFS	    0x100
#define CACHE_AUX_CTRL_REG_OFS	0x104

#define CACHE_TAG_RAM_CTRL_OFS	0x108
#define CACHE_DATA_RAM_CTRL_OFS	0x10C

#define reg2_int_clear_ofs		0x220			//reg2_int_clear
#define reg7_cache_sync			0x730			//reg7_cache_sync
#define reg7_inv_way			0x77C			//reg7_inv_way
#define CACHE_PREFETCH_REG_OFS	0xF60


/**
 * Interrupt and Counter Control Registers
 */
#define L2_REG2_EV_CNT_CTRL				(*((volatile unsigned long *)(L2_REG2_BASE + 0x00)))
#define L2_REG2_EV_CNT1_CFG				(*((volatile unsigned long *)(L2_REG2_BASE + 0x04)))
#define L2_REG2_EV_CNT0_CFG				(*((volatile unsigned long *)(L2_REG2_BASE + 0x08)))
#define L2_REG2_EV_CNT1					(*((volatile unsigned long *)(L2_REG2_BASE + 0x0C)))
#define L2_REG2_EV_CNT0					(*((volatile unsigned long *)(L2_REG2_BASE + 0x10)))
#define L2_REG2_INT_MASK				(*((volatile unsigned long *)(L2_REG2_BASE + 0x14)))
#define L2_REG2_INT_MASK_STATUS			(*((volatile unsigned long *)(L2_REG2_BASE + 0x18)))
#define L2_REG2_INT_RAW_STATUS			(*((volatile unsigned long *)(L2_REG2_BASE + 0x1C)))
#define L2_REG2_INT_CLEAR				(*((volatile unsigned long *)(L2_REG2_BASE + 0x20)))

#define S_L2_REG2_INT_DECERR	(8)
#define M_L2_REG2_INT_DECERR	(0x1 << S_L2_REG2_INT_DECERR)	/* Decode error */
#define S_L2_REG2_INT_SLVERR	(7)
#define M_L2_REG2_INT_SLVERR	(0x1 << S_L2_REG2_INT_SLVERR)	/* Slave error */
#define S_L2_REG2_INT_ERRRD		(6)
#define M_L2_REG2_INT_ERRRD		(0x1 << S_L2_REG2_INT_ERRRD)	/* Data RAM read error */
#define S_L2_REG2_INT_ERRRT		(5)
#define M_L2_REG2_INT_ERRRT		(0x1 << S_L2_REG2_INT_ERRRT)	/* Tag RAM read error */
#define S_L2_REG2_INT_ERRWD		(4)
#define M_L2_REG2_INT_ERRWD		(0x1 << S_L2_REG2_INT_ERRWD)	/* Data RAM write error */
#define S_L2_REG2_INT_ERRWT		(3)
#define M_L2_REG2_INT_ERRWT		(0x1 << S_L2_REG2_INT_ERRWT)	/* Tag RAM write error */
#define S_L2_REG2_INT_PARRD		(2)
#define M_L2_REG2_INT_PARRD		(0x1 << S_L2_REG2_INT_PARRD)	/* Parity error on data RAM read */
#define S_L2_REG2_INT_PARRT		(1)
#define M_L2_REG2_INT_PARRT		(0x1 << S_L2_REG2_INT_PARRT)	/* Pariry error on tag RAM read */
#define S_L2_REG2_INT_ECNTR		(0)
#define M_L2_REG2_INT_ECNTR		(0x1 << S_L2_REG2_INT_ECNTR)	/* Event counter1/0 overflow increment */

/**
 * Cache Maintenance Operations
 */
#define L2_REG7_CACHE_SYNC				(*((volatile unsigned long *)(L2_REG7_BASE + 0x30)))
#define L2_REG7_INV_PA					(*((volatile unsigned long *)(L2_REG7_BASE + 0x70)))
#define L2_REG7_INV_WAY					(*((volatile unsigned long *)(L2_REG7_BASE + 0x7C)))
#define L2_REG7_CLEAN_PA				(*((volatile unsigned long *)(L2_REG7_BASE + 0xB0)))
#define L2_REG7_CLEAN_INDEX				(*((volatile unsigned long *)(L2_REG7_BASE + 0xB8)))
#define L2_REG7_CLEAN_WAY				(*((volatile unsigned long *)(L2_REG7_BASE + 0xBC)))
#define L2_REG7_CLEAN_INV_PA			(*((volatile unsigned long *)(L2_REG7_BASE + 0xF0)))
#define L2_REG7_CLEAN_INV_INDEX			(*((volatile unsigned long *)(L2_REG7_BASE + 0xF8)))
#define L2_REG7_CLEAN_INV_WAY			(*((volatile unsigned long *)(L2_REG7_BASE + 0xFC)))


#define K_L2_REG7_CACHE_SYNC_C			(0x1)

#define K_L2_REG7_INV_WAY_8WAY			(0x00ff)
#define K_L2_REG7_INV_WAY_16WAY			(0xffff)
#define K_L2_REG7_CLEAN_WAY_8WAY        (0x00ff)
#define K_L2_REG7_CLEAN_WAY_16WAY       (0xffff)
#define L2_INT_CLEAR_STATUS	             0x1FF
/**
 * Cache Lockdown
 */
#define L2_REG9_D_LOCKDOWN0				(*((volatile unsigned long *)(L2_REG9_BASE + 0x00)))
#define L2_REG9_I_LOCKDOWN0				(*((volatile unsigned long *)(L2_REG9_BASE + 0x04)))
#define L2_REG9_D_LOCKDOWN1				(*((volatile unsigned long *)(L2_REG9_BASE + 0x08)))
#define L2_REG9_I_LOCKDOWN1				(*((volatile unsigned long *)(L2_REG9_BASE + 0x0C)))
#define L2_REG9_D_LOCKDOWN2				(*((volatile unsigned long *)(L2_REG9_BASE + 0x10)))
#define L2_REG9_I_LOCKDOWN2				(*((volatile unsigned long *)(L2_REG9_BASE + 0x14)))
#define L2_REG9_D_LOCKDOWN3				(*((volatile unsigned long *)(L2_REG9_BASE + 0x18)))
#define L2_REG9_I_LOCKDOWN3				(*((volatile unsigned long *)(L2_REG9_BASE + 0x1C)))
#define L2_REG9_D_LOCKDOWN4				(*((volatile unsigned long *)(L2_REG9_BASE + 0x20)))
#define L2_REG9_I_LOCKDOWN4				(*((volatile unsigned long *)(L2_REG9_BASE + 0x24)))
#define L2_REG9_D_LOCKDOWN5				(*((volatile unsigned long *)(L2_REG9_BASE + 0x28)))
#define L2_REG9_I_LOCKDOWN5				(*((volatile unsigned long *)(L2_REG9_BASE + 0x2C)))
#define L2_REG9_D_LOCKDOWN6				(*((volatile unsigned long *)(L2_REG9_BASE + 0x30)))
#define L2_REG9_I_LOCKDOWN6				(*((volatile unsigned long *)(L2_REG9_BASE + 0x34)))
#define L2_REG9_D_LOCKDOWN7				(*((volatile unsigned long *)(L2_REG9_BASE + 0x38)))
#define L2_REG9_I_LOCKDOWN7				(*((volatile unsigned long *)(L2_REG9_BASE + 0x3C)))
#define L2_REG9_LOCK_LINE_EN			(*((volatile unsigned long *)(L2_REG9_BASE + 0x50)))
#define L2_REG9_UNLOCK_WAY				(*((volatile unsigned long *)(L2_REG9_BASE + 0x54)))

/**
 * Address Filtering
 */
#define L2_REG12_ADDR_FILTERING_START	(*((volatile unsigned long *)(L2_REG12_BASE + 0x00)))
#define L2_REG12_ADDR_FILTERING_END		(*((volatile unsigned long *)(L2_REG12_BASE + 0x04)))

/**
 * Debug, Prefetch and Power
 */
#define L2_REG15_DEBUG_CTRL				(*((volatile unsigned long *)(L2_REG15_BASE + 0x40)))
#define L2_REG15_PREF_CTRL				(*((volatile unsigned long *)(L2_REG15_BASE + 0x60)))
#define L2_REG15_POWER_CTRL				(*((volatile unsigned long *)(L2_REG15_BASE + 0x80)))

#define S_L2_REG15_DEBUG_CTRL_SPNIDEN	(2)
#define M_L2_REG15_DEBUG_CTRL_SPNIDEN	(0x1 << S_L2_REG15_DEBUG_CTRL_SPNIDEN)
#define S_L2_REG15_DEBUG_CTRL_DWB		(1)
#define M_L2_REG15_DEBUG_CTRL_DWB		(0x1 << S_L2_REG15_DEBUG_CTRL_DWB)
#define S_L2_REG15_DEBUG_CTRL_DCL		(0)
#define M_L2_REG15_DEBUG_CTRL_DCL		(0x1 << S_L2_REG15_DEBUG_CTRL_DCL)

#define S_L2_REG15_PREF_CTRL_DL_FILL_EN					(30)
#define M_L2_REG15_PREF_CTRL_DL_FILL_EN					(0x1 << S_L2_REG15_PREF_CTRL_DL_FILL_EN)
#define S_L2_REG15_PREF_CTRL_INST_PREF_EN				(29)
#define M_L2_REG15_PREF_CTRL_INST_PREF_EN				(0x1 << S_L2_REG15_PREF_CTRL_INST_PREF_EN)
#define S_L2_REG15_PREF_CTRL_DATA_PREF_EN				(28)
#define M_L2_REG15_PREF_CTRL_DATA_PREF_EN				(0x1 << S_L2_REG15_PREF_CTRL_DATA_PREF_EN)
#define S_L2_REG15_PREF_CTRL_DL_WRAP_READ_DIS			(27)
#define M_L2_REG15_PREF_CTRL_DL_WRAP_READ_DIS			(0x1 << S_L2_REG15_PREF_CTRL_DL_WRAP_READ_DIS)
#define S_L2_REG15_PREF_CTRL_PREF_DROP_EN				(24)
#define M_L2_REG15_PREF_CTRL_PREF_DROP_EN				(0x1 << S_L2_REG15_PREF_CTRL_PREF_DROP+EN)
#define S_L2_REG15_PREF_CTRL_INCR_DL_FILL_EN			(23)
#define M_L2_REG15_PREF_CTRL_INCR_DL_FILL_EN			(0x1 << S_L2_REG15_PREF_CTRL_INCR_DL_FILL_EN)
#define S_L2_REG15_PREF_CTRL_NOT_SAME_ID_EXCLU_SEQ_EN	(21)
#define M_L2_REG15_PREF_CTRL_NOT_SAME_ID_EXCLU_SEQ_EN	(0x1 << S_L2_REG15_PREF_CTRL_NOT_SAME_ID_EXCLU_SEQ_EN)
#define S_L2_REG15_PREF_CTRL_PREF_OFF					(0)
#define M_L2_REG15_PREF_CTRL_PREF_OFF					(0xf << S_L2_REG15_PREF_CTRL_PREF_OFF)
# endif

