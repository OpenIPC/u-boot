// SPDX-License-Identifier: GPL-2.0+
/*
 * Ingenic XBurst: watchdog reset and the cache operations of the vendor
 * arch/mips/cpu/xburst/cpu.c (OpenIPC/u-boot-ingenic). They replace the
 * generic MIPS ones: after a data cache write-back the write buffer is
 * drained with an uncached read, and after the instruction cache is
 * invalidated so is the branch target buffer (Config7 bit 1).
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 */

#include <common.h>
#include <linux/delay.h>
#include <mach/ingenic.h>
#include <cpu_func.h>
#include <linux/delay.h>
#include <asm/mipsregs.h>
#include <asm/cacheops.h>
#include <asm/reboot.h>
#include <asm/system.h>
#include <asm/io.h>
#include <asm/arch/wdt.h>
#include <asm/arch/clk.h>
#include <asm/global_data.h>

#define cache_op(op, addr)		\
	__asm__ __volatile__(		\
		".set	push\n"		\
		".set	noreorder\n"	\
		".set	mips3\n"	\
		"cache	%0, %1\n"	\
		".set	pop\n"		\
		:			\
		: "i" (op), "R" (*(unsigned char *)(addr)))

/* Reset by the watchdog, as the vendor U-Boot does */
void _machine_restart(void)
{
	int time = RTC_FREQ / WDT_DIV * RESET_DELAY_MS / 1000;

	if(time > 65535)
		time = 65535;

	writel(TSCR_WDTSC, TCU_BASE + TCU_TSCR);

	writel(0, WDT_BASE + WDT_TCNT);
	writel(time, WDT_BASE + WDT_TDR);
	writel(TCSR_PRESCALE | TCSR_RTC_EN, WDT_BASE + WDT_TCSR);
	writel(0,WDT_BASE + WDT_TCER);

	printf("reset in %dms", RESET_DELAY_MS);
	writel(TCER_TCEN,WDT_BASE + WDT_TCER);
	mdelay(1000);
}

void flush_cache(ulong start_addr, ulong size)
{
	unsigned long lsize = CONFIG_SYS_CACHELINE_SIZE;
	unsigned long addr = start_addr & ~(lsize - 1);
	unsigned long aend = (start_addr + size - 1) & ~(lsize - 1);

	for (; addr <= aend; addr += lsize) {
		cache_op(HIT_WRITEBACK_INV_D, addr);
		cache_op(HIT_INVALIDATE_I, addr);
	}
	__asm__ __volatile__("sync");
	/* invalidate btb */
	write_c0_config7(read_c0_config7() | 2);
	instruction_hazard_barrier();
}

void flush_dcache_range(ulong start_addr, ulong stop)
{
	unsigned long lsize = CONFIG_SYS_CACHELINE_SIZE;
	unsigned long addr = start_addr & ~(lsize - 1);
	unsigned long aend = (stop - 1) & ~(lsize - 1);
	unsigned int writebuffer;
	for (; addr <= aend; addr += lsize)
		cache_op(HIT_WRITEBACK_INV_D, addr);
	__asm__ __volatile__("sync");
	writebuffer = *(volatile unsigned int *)0xa0000000;
}

void invalidate_dcache_range(ulong start_addr, ulong stop)
{
	unsigned long lsize = CONFIG_SYS_CACHELINE_SIZE;
	unsigned long addr = start_addr & ~(lsize - 1);
	unsigned long aend = (stop - 1) & ~(lsize - 1);

	for (; addr <= aend; addr += lsize)
		cache_op(HIT_INVALIDATE_D, addr);
}

void flush_icache_all(void)
{
	u32 addr, t = 0;

	for (addr = CKSEG0; addr < CKSEG0 + CONFIG_SYS_ICACHE_SIZE;
	     addr += CONFIG_SYS_CACHELINE_SIZE) {
		cache_op(INDEX_INVALIDATE_I, addr);
	}

	/* invalidate btb */
	__asm__ __volatile__(
		".set mips32\n\t"
		"mfc0 %0, $16, 7\n\t"
		"nop\n\t"
		"ori %0,2\n\t"
		"mtc0 %0, $16, 7\n\t"
		".set mips2\n\t"
		:
		: "r" (t));
}

void flush_dcache_all(void)
{
	u32 addr;

	for (addr = CKSEG0; addr < CKSEG0 + CONFIG_SYS_DCACHE_SIZE;
	     addr += CONFIG_SYS_CACHELINE_SIZE) {
		cache_op(INDEX_WRITEBACK_INV_D, addr);
	}

	__asm__ __volatile__("sync");
}

void flush_cache_all(void)
{
	flush_dcache_all();
	flush_icache_all();
}

#ifndef CONFIG_SPL_BUILD
DECLARE_GLOBAL_DATA_PTR;

/* The DDR the SPL set up: the package's, as the vendor U-Boot takes it */
int dram_init(void)
{
	gd->ram_size = PHYS_SDRAM_1_SIZE;
	return 0;
}

int print_cpuinfo(void)
{
	printf("CPU:   Ingenic %s, %u MHz, DDR %u MHz\n", CONFIG_SYS_SOC,
	       clk_get_rate(CPU) / 1000000, clk_get_rate(DDR) / 1000000);
	return 0;
}
#endif
