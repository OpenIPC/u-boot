/**
    CPU module driver.

    This file is the driver of CPU module.

    @file       Cache.h
    @ingroup    mIHALCore
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2005.  All rights reserved.
*/

#ifndef _CACHE_H
#define _CACHE_H
#include "constant.h"


/*
     csselrcache level & type selection

     Detailed reference to [trm p4-177 CSSELR]
*/
typedef enum _CACHE_LV_TYPE_
{
    LEVEL_1_DCACHE = 0x0,   //< Level 1 data cache
    LEVEL_1_ICACHE,         //< Level 1 instruction cache
    LEVEL_2_DCACHE,         //< Level 2 data cache
} CACHE_LV_TYPE;


/*
     Values for Ctype fields in CLIDR
*/

typedef enum _CLIDR_CTYPE_
{
    CLIDR_CTYPE_NO_CACHE        = 0x0,
    CLIDR_CTYPE_INSTRUCTION_ONLY= 0x1,
    CLIDR_CTYPE_DATA_ONLY       = 0x2,
    CLIDR_CTYPE_INSTRUCTION_DATA= 0x3,
    CLIDR_CTYPE_UNIFIED         = 0x4,
}CLIDR_CTYPE;


//Instruction Synchronization Barrier.
#define _ISB() \
	__asm__ __volatile__("isb\n\t")

//Data Synchronization Barrier
#define _DSB() \
	__asm__ __volatile__("dsb\n\t")

//Data Memory Barrier
#define _DMB() \
	__asm__ __volatile__("dmb\n\t")


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
#define S_DC_SETS	8
#define S_DC_W		2
#define S_DC_LS		5

#define _DC_SETS	(1 << S_DC_SETS)	/* 256 sets */
#define _DC_W 		(1 << S_DC_W)		/* 4 way */
#define _DC_LS 		(1 << S_DC_LS)		/* 32 byte line size */

#define S_IC_SETS	8
#define S_IC_W		2
#define S_IC_LS		5

#define _IC_SETS	(1 << S_IC_SETS)	/* 256 sets */
#define _IC_W 		(1 << S_IC_W)		/* 4 way */
#define _IC_LS 		(1 << S_IC_LS)		/* 32 byte line size */

#define S_DC_W_L2	4                   /* L2 way shift*/

#define S_SC_SETS	9
#define S_SC_W		3
#define S_SC_LS		5

#define _SC_SETS	(1 << S_SC_SETS)	/* 512 sets */
#define _SC_W 		(1 << S_SC_W)		/* 8 way */
#define _SC_LS 		(1 << S_SC_LS)		/* 32 byte line size */


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
 * SCTLR_EL1/SCTLR_EL2/SCTLR_EL3 bits definitions
 */
#define CR_M		(1 << 0)	/* MMU enable			*/
#define CR_A		(1 << 1)	/* Alignment abort enable	*/
#define CR_C		(1 << 2)	/* Dcache enable		*/
#define CR_SA		(1 << 3)	/* Stack Alignment Check Enable	*/
#define CR_I		(1 << 12)	/* Icache enable		*/
#define CR_WXN		(1 << 19)	/* Write Permision Imply XN	*/
#define CR_EE		(1 << 25)	/* Exception (Big) Endian	*/

#define SCTLR() \
	({ \
	 unsigned long val; \
	 __asm__ __volatile__( \
		 "mrc p15, 0, %0, c1, c0, 0\n\t" \
		 : "=r" (val)); \
	 val; \
	 })

#define LEVEL_1 (1 - 1)
#define LEVEL_2 (2 - 1)

#define CCSIDR() \
	({ \
	 unsigned long val; \
	 __asm__ __volatile__( \
		 "mrc p15, 1, %0, c0, c0, 0\n\t" \
		 : "=r" (val)); \
	 val; \
	 })


#define sel_CSSELR(InD) \
	__asm__ __volatile__(	\
			"mcr p15, 2, %0, c0, c0, 0\n\t" \
			: \
			: "r"(InD));

#define _ICACHE_INV_ALL() _ICIALLU() \
                        _BPIALL()

/*
    ICIALLU, Instruction Cache Invalidate All to PoU
    The ICIALLU characteristics are:

        Invalidate all instruction caches to PoU. If branch predictors are
        architecturally visible, also flush branch predictors.

        This register is part of the Cache maintenance instructions functional group.

    Usage constraints
    If EL3 is implemented and is using AArch32, this operation can be performed at the following
    exception levels:
*/
#define _ICIALLU() \
	__asm__ __volatile__("mcr p15, 0, %0, c7, c5, 0" : : "r" (0));

/*
    Branch Predictor Invalidate All
    The BPIALL characteristics are:

        Invalidate all entries from branch predictors.
        This register is part of the Cache maintenance instructions functional group.
    Usage constraints
    If EL3 is implemented and is using AArch32, this operation can be performed at the following
    exception levels:
*/
#define _BPIALL() \
    __asm__ __volatile__("mcr p15, 0, %0, c7, c5, 6" : : "r" (0));


/*
     ICIMVAU : Instruction Cache line Invalidate by VA to PoU
*/
#define _ICACHE_INV_MVAU(addr) _ICIMVAU(addr)
#define _ICIMVAU(addr) \
	__asm__ __volatile__(	\
			"mcr p15, 0, %0, c7, c5, 1\n\t" \
			: \
			: "r"(addr));

#define _DCACHE_INV_MVAC(addr) _DCIMVAC(addr)
#define _DCIMVAC(addr) \
	__asm__ __volatile__(	\
			"mcr p15, 0, %0, c7, c6, 1\n\t" \
			: \
			: "r"(addr));

//DCCMVAC Clean data cache line by VA to PoC
#define _DCACHE_WBACK_MVAC(addr) _DCCMVAC(addr)
#define _DCCMVAC(addr) \
	__asm__ __volatile__(	\
			"mcr p15, 0, %0, c7, c10, 1\n\t" \
			: \
			: "r"(addr));

//_DCCIMVAC Clean and invalidate data cache line by VA to PoC
#define _DCACHE_WBACK_INV_MVAC(addr) _DCCIMVAC(addr)
#define _DCCIMVAC(addr) \
	__asm__ __volatile__(	\
			"mcr p15, 0, %0, c7, c14, 1\n\t" \
			: \
			: "r"(addr));

//DCCISW Clean and invalidate data cache line by set/way
#define _DCCISW(way_set) \
	__asm__ __volatile__(	\
			"mcr p15, 0, %0, c7 ,c14, 2\n\t"  \
			:	\
			: "r"(way_set));

//DCISW Invalidate data cache line by set/way
/*
    The DCISW input value bit assignments are:

    |31......................4 | 3...1 | 0 |
     Setway                      level  rsv
     bit[3..1] = 0x0 = Level 1
     bit[3..1] = 0x1 = Level 2
     ...
*/
#define _DCISW(way_set) \
	__asm__ __volatile__(	\
			"mcr p15, 0, %0, c7 ,c6, 2\n\t"  \
			:	\
			: "r"(way_set));


//DCCSW Clean data cache line by set/way
/*
    The DCCSW input value bit assignments are:

    |31......................4 | 3...1 | 0 |
     Setway                      level  rsv
     bit[3..1] = 0x0 = Level 1
     bit[3..1] = 0x1 = Level 2
     ...
*/
#define _DCCSW(way_set) \
	__asm__ __volatile__(   \
							"mcr p15, 0, %0, c7 ,c10, 2\n\t"  \
							:   \
							: "r"(way_set))




/* Cache dirty register () */
#define read_CDSR() \
	({ \
	 unsigned long val; \
	 __asm__ __volatile__(\
		 "mrc p15, 0, %0, c7, c10, 6\n\t" \
		 : "=r"(val) \
		 );\
	 val;\
	 })

/* Get CPU ID */
#define read_MPIDR() \
	({ \
	 unsigned long val; \
	 __asm__ __volatile__(\
		 "mrc p15, 0, %0, c0, c0, 5\n\t" \
		 : "=r"(val) \
		 );\
	 val;\
	 })

/* ead current CP15 Cache Level ID Register */
#define read_CLIDR() \
	({ \
	 unsigned long val; \
	 __asm__ __volatile__(\
		 "mrc p15, 1, %0, c0, c0, 1\n\t" \
		 : "=r"(val) \
		 );\
	 val;\
	 })

#define change_property() \
	({ \
	 __asm__ __volatile__( \
		 ".arch_extension sec\n\t" \
		 "smc #123\n\t" \
		 : \
		 : \
		 ); \
	 })

#define CLEAR_MMU()	\
	__asm__ __volatile__("mcr p15, 0, r0, c8, c7, 0\n\t");


#define DISABLE_MMU()	\
   ({ \
	 unsigned long val = 0; \
	__asm__ __volatile__( \
		"mrc p15, 0, %0, c1, c0, 0\n\t" \
		"bic %0, %0, #0x1\n\t" \
		"mcr p15, 0, %0, c1, c0, 0\n\t" \
		: \
		: "r" (val) \
		); \
	})

#define DISABLE_DCACHE()	\
   ({ \
	 unsigned long val = 0; \
	__asm__ __volatile__( \
		"mrc p15, 0, %0, c1, c0, 0\n\t" \
		"bic %0, %0, #0x4\n\t" \
		"mcr p15, 0, %0, c1, c0, 0\n\t" \
		: \
		: "r" (val) \
		); \
	})

#define DISABLE_ICACHE()	\
   ({ \
	 unsigned long val = 0; \
	__asm__ __volatile__( \
		"mrc p15, 0, %0, c1, c0, 0\n\t" \
		"bic %0, %0, #0x1000\n\t" \
		"mcr p15, 0, %0, c1, c0, 0\n\t" \
		: \
		: "r" (val) \
		); \
	})

#define SETUP_TTBR0(val)	\
	({	\
	 __asm__ __volatile__(	\
		 "mcr p15, 0, %0, c2, c0, 0\n\t"	\
		 : \
		 :	"r" (val) \
		 ); \
	 })
/* Cache dirty register () */
#define read_CDSR() \
	({ \
	 unsigned long val; \
	 __asm__ __volatile__(\
		 "mrc p15, 0, %0, c7, c10, 6\n\t" \
		 : "=r"(val) \
		 );\
	 val;\
	 })
/**
    @addtogroup mIHALCore
*/
//@{

/**
    @name CPU clean invalidate all DCache

    Clean and invalidate all data cache

    @return void
*/
//@{
#define CPUCleanInvalidateDCacheAll()       cpu_cleanInvalidateDCacheAll()          ///< clean invalidate all DCache
//@}

/**
    @name CPU clean invalidate DCache

    Clean and invalidate data cache
    Cache line associated with input address will be clean and invalidated

    @param[in] addr     data address to be clean and invalidated

    @return void
*/
//@{
#define CPUCleanInvalidateDCache(addr)      cpu_cleanInvalidateDCache(addr)         ///< clean invalidate DCache
//@}

/**
    @name CPU clean invalidate DCache block

    Clean and invalidate data cache block
    Cache line associated with input region will be clean and invalidated

    @param[in] m        starting data address to be clean and invalidated
    @param[in] n        end data address to be clean and invalidated

    @return void
*/
//@{
#define CPUCleanInvalidateDCacheBlock(m,n)  cpu_cleanInvalidateDCacheBlock(m, n)    ///< clean invalidate DCache block
//@}

/**
    @name CPU invalidate DCache

    Invalidate data cache
    Cache line associated with input address will be invalidated

    @param[in] addr     data address to be invalidated

    @return void
*/
//@{
#define CPUInvalidateDCache(m)              cpu_invalidateDCache(m)                 ///< invalidate DCache
//@}

/**
    @name CPU invalidate DCache block

    Invalidate data cache block
    Cache line associated with input region will be invalidated

    @param[in] m        starting data address to be invalidated
    @param[in] n        end data address to be invalidated

    @return void
*/
//@{
#define CPUInvalidateDCacheBlock(m,n)       cpu_invalidateDCacheBlock(m,n)          ///< invalidate DCache block
//@}

/**
    @name CPU clean DCache

    Clean data cache
    Cache line associated with input address will be clean

    @param[in] addr     data address to be clean

    @return void
*/
//@{
#define CPUCleanDCache(m)                   cpu_cleanDCache(m)                      ///< clean DCache
//@}

/**
    @name CPU clean DCache block

    Clean data cache block
    Cache line associated with input region will be clean

    @param[in] m        starting data address to be clean
    @param[in] n        end data address to be clean

    @return void
*/
//@{
#define CPUCleanDCacheBlock(m, n)           cpu_cleanDCacheBlock(m, n)              ///< clean DCache block
//@}

/**
    @name CPU invalidate all DCache

    Invalidate all data cache

    @return void
*/
//@{
#define CPUInvalidateDCacheAll()            cpu_invalidateDCacheAll()               ///< invalidate all DCache
//@}


extern BOOL     CPUChkDCacheEnabled(UINT32 addr);
extern void     CPUInvalidateICacheAll(void) __attribute__ ((section (".part1")));
//extern void     CPUInvalidateICacheAll(void) __attribute__ ((section (".part1"), far));
//extern void     CPUInvalidateICache(UINT32 addr);
extern void     CPUInvalidateICacheBlock(UINT32 start, UINT32 end);

#if 0
extern void     CPUInvalidateDCacheAll(void);
extern void     CPUInvalidateDCache(UINT32 addr);
extern void     CPUInvalidateDCacheBlock(UINT32 start, UINT32 end);
#endif
extern void     cpu_invalidateDCacheBlock(UINT32 start, UINT32 end);
extern void     cpu_invalidateDCache(UINT32 addr);
extern void     cpu_invalidateDCacheAll(void)__attribute__ ((section (".part1")));
extern void     cpu_cleanDCache(UINT32 addr);
extern void     cpu_cleanDCacheBlock(UINT32 start, UINT32 end);
extern void     _cache_clean_d_cache_all(UINT32 level_type, BOOL DSB)__attribute__ ((section (".part1")));
extern void     _cache_invalidate_data_cache_all(UINT32 level_type, BOOL DSB)__attribute__ ((section (".part1")));
extern void     _cache_clean_invalidate_d_cache_All(UINT32 level_type, BOOL DSB)__attribute__ ((section (".part1")));



extern void     cpu_cleanInvalidateDCacheAll(void)__attribute__ ((section (".part1")));
//extern void     cpu_cleanInvalidateDCacheAll(void)__attribute__ ((section (".part1"), far));
extern void     cpu_cleanInvalidateDCache(UINT32 addr);
extern void     cpu_cleanInvalidateDCacheBlock(UINT32 start, UINT32 end);

extern void     CPUflushReadCache(UINT32 uiStartAddr, UINT32 uiLength);
extern void     CPUflushWriteCache(UINT32 uiStartAddr, UINT32 uiLength);



#endif

//@}
