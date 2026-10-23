/*
    Main control function

    This file is implement by user mode

    @file       main.c

    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.
*/
#include "main.h"
#include "bl_func.h"
#include "Cache.h"
#include "global.h"
#include "fuart.h"
#include "debug.h"

extern char _loader_exec_compres_start[];
extern char _loader_exec_compress_load_cpu_addr[];
extern char _load_LOADER_CONFIGRAM_FREQ_PARAM_start_base[];
extern char _load_BOOT_EXEC_start_base[];
extern char _loader_data_start_base[];
#define _THUMB2 __attribute__((target("thumb")))
//---------------------------------------------------------------------------
// Static function
//---------------------------------------------------------------------------
/*
    BL_RunPartOne

    Expend loader if use compress

    @return void
*/
_THUMB2 static void BL_RunPartOne(void)
{
#if (_LOADER_COMPRESSED_ == ENABLE)
#if (_PROCESS_2ND_CHECKSUM_ == 1)
	NVTPACK_MEM mem = {0};
	UINT16	sum;
	UINT16  src_sum;
#endif
    UINT8 *in_addr = (UINT8 *)(_loader_exec_compress_load_cpu_addr);
    UINT8 *out_addr = (UINT8 *)(_loader_exec_compres_start);
    UINT32 uiCompressedOutSize;

    // Get decompress size within specific header
    uiCompressedOutSize = (((UINT32)*(in_addr + 12)) << 24) +
                          (((UINT32)*(in_addr + 13)) << 16) +
                          (((UINT32)*(in_addr + 14)) <<  8) +
                          ((UINT32)*(in_addr + 15));

    // Decompress loader image
#if (_PROCESS_2ND_CHECKSUM_ == 1)
	*(UINT32 *)0xF0290000 = 'L';
#endif
	LZ_Un_compress((unsigned char *)(in_addr + LDC_HEADER_SIZE), (unsigned char *)(out_addr), uiCompressedOutSize);
#if (_PROCESS_2ND_CHECKSUM_ == 1)
	*(UINT32 *)0xF0290000 = 'Z';
#endif
    // Need to clean data cache
    CPUInvalidateICacheAll();
#if (_PROCESS_2ND_CHECKSUM_ == 1)
	*(UINT32 *)0xF0290000 = 'I';
#endif
    CPUCleanInvalidateDCacheAll();
#if (_PROCESS_2ND_CHECKSUM_ == 1)
	*(UINT32 *)0xF0290000 = 'D';
#endif

#if (_PROCESS_2ND_CHECKSUM_ == 1)
	//	debug_msg_var("addr", (UINT32)(_load_BOOT_EXEC_start_base));
	mem.p_data = (void *)out_addr;
	mem.len = *(UINT16 *)(_load_BOOT_EXEC_start_base + 0x14 + 0x2);
	src_sum = *(UINT16 *)(_load_BOOT_EXEC_start_base + 0x14);
	sum = nvtpack_calc_nvt_sum(&mem);
	if(src_sum == sum) {
		debug_msg("\r\nCHKSUM UNZOK!");
	} else {
		debug_msg_var("\r\nCHKSUM UNZNG!", sum);
		debug_msg_var("calc_sum=",sum);
		debug_msg_var(" bin_sum=",src_sum);
	}
#else
	debug_msg("\r\nUNZOK!");
#endif
#else
#endif
}

/*
void BL_PostHandle(void)
{
    UINT32  uiParamCnt;
    UINT32 *pParamOffset;
    UINT32 *pDramParam;

    pParamOffset = (UINT32*)(&_load_LOADER_CONFIGRAM_FREQ_PARAM_start_base[5*4]);
    pDramParam = (UINT32*)(*pParamOffset);
    for (uiParamCnt=0; uiParamCnt < 7*2; uiParamCnt++)
    {
        OUTREG32(DDR_PARAM_LEVEL_0_ADDR + uiParamCnt*4, pDramParam[uiParamCnt]);
    }

    // Disable IDE2 clock
    CLRREG32(0xC0020070, 1<<17);

    //invalid Instruciton and data cache
    CPUCleanInvalidateDCacheAll();
    CPUInvalidateICacheAll();
}
*/
_THUMB2 UINT32 main(void)
{
    UINT32 uiReturn;
    UINT32 load_addr, target_addr, uitron_size;
    UINT32 i;
    UINT32 *pSrc, *pDst;
    // Run part one loader
    // If using compressed loader, just decompress the loader image at first!
    BL_RunPartOne();
    utl_dram_protect_enable((UINT32)_loader_exec_compres_start, (UINT32)(_loader_data_start_base - _loader_exec_compres_start));
    uiReturn = bl_mainFlow();
    utl_dram_protect_check();
    utl_dram_protect_disable();

    bl_read_rtos_addr(&load_addr, &target_addr, &uitron_size);
    if (load_addr != target_addr) {
        uart_putSystemUARTStr("uitron target_addr= ");
        PrintDec2Hex(target_addr);
        uart_putSystemUARTStr("\r\n");
        uart_putSystemUARTStr("uitron load_addr= ");
        PrintDec2Hex(load_addr);
        uart_putSystemUARTStr("\r\n");
        uart_putSystemUARTStr("Copy from load to target by loader, this may has longer boot time\r\n");
        CPUflushReadCache(0, 128*1024);
    	CPUInvalidateICacheAll();

#if 1
    	// disable MMU
    	{ \
    		__asm__ __volatile__( \
    				"mrc p15, 0, r0, c1, c0, 0\n\t" \
    				"bic r0, r0, #0x5\n\t" \
    				"bic r0, r0, #0x1000\n\t" \
    				"mcr p15, 0, r0, c1, c0, 0\n\t" \
    				: \
    				: \
    				: "r0" \
    		); \
    	}
    	asm volatile("dsb");
    	asm volatile("isb");
        uart_putSystemUARTStr("disable MMU done\r\n");
#endif

        pSrc = (UINT32*)load_addr;
        pDst = (UINT32*)target_addr;
        for (i=0; i<uitron_size; i+=4) {
            *pDst++ = *pSrc++;
        }
        // Don't invoke uart_putSystemUARTStr() hereafter.
        // Because ZI/RW may be corrupted after code copy
//        uart_putSystemUARTStr("copy done\r\n");
    }

    //BL_PostHandle();

    return uiReturn;
}

/*
 * OpenIPC: libgcc's division by zero handler calls raise() when the
 * loader is built with a Linux toolchain
 */
int raise(int sig);
int raise(int sig)
{
	return 0;
}
