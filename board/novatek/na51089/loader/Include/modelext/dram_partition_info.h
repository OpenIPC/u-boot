#ifndef _DRAM_PARTITION_INFO_H
#define _DRAM_PARTITION_INFO_H

#if defined(__UITRON)
#include "Type.h"
#else
#include "nvt_type.h"
#endif

/**
    DRAM Partition

    This is common header used between firmware of uITRON, eCos, Linux, DSP
    so !!!!!! DO NOT modify it !!!!!!
*/

#define DRAM_PARTITION_INFO_VER 0x17032309 ///< YYYY/MM/DD HH

typedef struct _DRAM_PARTITION {
	unsigned int fourcc;	///< FourCC DRAM
	unsigned int info_size;	///< sizeof(CORE_INFO)
	unsigned int dram_addr; ///< whole dram starting address
	unsigned int dram_size; ///< whole dram size
	unsigned int rev_addr;  ///< reversed starting address
	unsigned int rev_size;  ///< reversed size
	unsigned int fdt_addr;	///< ipc starting address
	unsigned int fdt_size;	///< ipc size
	unsigned int linux_addr;///< linux-kernel starting address
	unsigned int linux_size;///< linux-kernel size
	unsigned int uboot_addr;///< u-boot starting address
	unsigned int uboot_size;///< u-boot size
	unsigned int rtos_addr; ///< rtos starting address
	unsigned int rtos_size; ///< rtos size
	unsigned int dsp1_addr; ///< DSP1 starting address
	unsigned int dsp1_size; ///< DSP1 size
	unsigned int dsp2_addr; ///< ecos(na51023) / DSP2(na51000) starting address
	unsigned int dsp2_size; ///< ecos(na51023) / DSP2(na51000) size
	unsigned int loader_addr; ///< loader starting address
	unsigned int loader_size; ///< loader size
	unsigned int nuttx_addr; ///< nuttx starting address
	unsigned int nuttx_size; ///< nuttx size
	unsigned int teeos_addr; ///< teeos starting address
	unsigned int teeos_size; ///< teeos size
	unsigned int hdal1_addr; ///< hdal[0] starting address
	unsigned int hdal1_size; ///< hdal[0] size
	unsigned int hdal2_addr; ///< hdal[1] starting address for dram2
	unsigned int hdal2_size; ///< hdal[1] size for dram2
   	unsigned int core2_entry1_addr; ///< for smp boot, cpu2 start at 0 then jump to core2_entry2 for addr 0 can be reuse.
	unsigned int core2_entry1_size; ///< for smp boot, cpu2 start at 0 then jump to core2_entry2 for addr 0 can be reuse.
	unsigned int core2_entry2_addr; ///< asm code to start smp
	unsigned int core2_entry2_size; ///< asm code to start smp

}DRAM_PARTITION;

#endif