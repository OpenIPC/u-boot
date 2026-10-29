// SPDX-License-Identifier: GPL-2.0+
/*
 * The T41 vendor SPL's epc_debug() (arch/mips/cpu/xburst2/epc_debug.c of
 * the Ingenic T41 SDK 1.2.0): print the reset error PC of both cores. Core
 * 1 is woken on a stub copied to the ORAM (its reset entry pointed there
 * for the while), stores its ErrorEPC in CCU_MBR1 and waits; core 0 then
 * puts it back to sleep and restores the reset entry. The vendor SDK notes
 * that the values are only meaningful after a reset from U-Boot.
 *
 * The instructions are the vendor's. The vendor asm listed the five
 * registers it writes as inputs, so the compiler was free to keep live
 * values in them, and read core 1's PC from an uninitialised variable;
 * here they are outputs.
 *
 * Copyright (c) 2023 Ingenic Semiconductor Co.,Ltd
 */

#include <common.h>

void epc_debug(void)
{
	unsigned int tmp0, tmp1, tmp2, tmp3;
	unsigned int core1_error_epc;
	unsigned int errorpc;

	__asm__ __volatile__ (
			"mfc0  %0, $30,  0   \n\t"
			"nop                  \n\t"
			:"=r"(errorpc)
			:);
	printf("CPU0 RESET ERROR PC: %x\n", errorpc);

	__asm__ __volatile__(
			".set push                             \n"
			".set noat                             \n"
			".set noreorder                        \n"
			"   lui  %0,0xb220                     \n"
			"   lui  %1,0xb266                     \n"
			"#change reset entry into oram         \n"
			"   lw   %4,0x0f00(%0)                 \n"
			"   sw   %1,0x0f00(%0)                 \n"
			"   la   %0,__read_error_epc_begin     \n"
			"   la   %3,__read_error_epc_end       \n"
			"   subu %3, %3, %0                    \n"
			"   add  %3, %3, %1                    \n"
			"_move_to_oram:                        \n"
			"   lw   %2, 0(%0)                     \n"
			"   sw   %2, 0(%1)                     \n"
			"   addiu %0, %0, 4                    \n"
			"   bne   %1, %3, _move_to_oram        \n"
			"   addiu %1, %1, 4                    \n"
			"   lui   %0,0xb220                    \n"
			"#wakeup core1                         \n"
			"   sw    $0,0x40(%0)                  \n"
			"   li    %1,0x2                       \n"
			"__core0_wait_core1_sleep:             \n"
			"   lw    %2, 0x20(%0)                 \n"
			"   and   %2, %2,%1                    \n"
			"   beqz  %2,__core0_wait_core1_sleep  \n"
			"   nop                                \n"
			"   sw    %1,0x40(%0)                  \n"
			"   j     __read_error_epc_end         \n"
			"   nop                                \n"
			"__read_error_epc_begin:               \n"
			"   mfc0  %0, $30,0                    \n"
			"   lui   %1, 0xb220                   \n"
			"#CCU_MBR1                             \n"
			"   sw    %0, 0x1004(%1)               \n"
			"   wait                               \n"
			"   nop                                \n"
			"   nop                                \n"
			"   nop                                \n"
			"__read_error_epc_end:                 \n"
			"   lui   %0,0xb220                    \n"
			"   sw    %4,0x0f00(%0)                \n"
			"   lw    %4,0x1004(%0)                \n"
			".set pop                              \n"
			: "=&r"(tmp0), "=&r"(tmp1), "=&r"(tmp2), "=&r"(tmp3),
			  "=&r"(core1_error_epc)
			:
			: "memory");
	printf("CPU1 RESET ERROR PC: %x\n", core1_error_epc);
}
