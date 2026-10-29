// SPDX-License-Identifier: GPL-2.0+
/*
 * Ingenic XBurst2: watchdog reset and the cache operations of the vendor
 * arch/mips/cpu/xburst2/cpu.c (OpenIPC/u-boot-t40), with the L1 and L2
 * cache loops of asm/jz_cache.h. They replace the generic MIPS ones.
 *
 * As in the vendor tree, the range operations work on the L1 data and
 * instruction caches; only flush_cache_all() writes back the L2 as well,
 * and then reads uncached memory so the write-back has completed.
 *
 * Copyright (c) 2016 Ingenic Semiconductor Co.,Ltd
 */

#include <common.h>
#include <cpu_func.h>
#include <init.h>
#include <linux/delay.h>
#include <linux/sizes.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/mipsregs.h>
#include <asm/reboot.h>
#include <asm/arch/clk.h>
#include <asm/arch/wdt.h>
#include <mach/ingenic.h>
#include <asm/jz_cache.h>

/* The vendor's names for its loops; this file defines the functions */
#undef flush_dcache_range
#undef flush_icache_range
#undef flush_dcache_all
#undef flush_l2cache_all

/* Reset by the watchdog, as the vendor U-Boot does */
void _machine_restart(void)
{
	int time = RTC_FREQ / WDT_DIV * RESET_DELAY_MS / 1000;

	if (time > 65535)
		time = 65535;

	writel(TSCR_WDTSC, TCU_BASE + TCU_TSCR);

	writel(0, WDT_BASE + WDT_TCNT);
	writel(time, WDT_BASE + WDT_TDR);
	writel(TCSR_PRESCALE | TCSR_RTC_EN, WDT_BASE + WDT_TCSR);
	writel(0, WDT_BASE + WDT_TCER);

	printf("reset in %dms", RESET_DELAY_MS);
	writel(TCER_TCEN, WDT_BASE + WDT_TCER);
	mdelay(1000);
}

void flush_dcache_range(ulong start, ulong stop)
{
	if (start == stop)
		return;
	blast_dcache_range(start, stop);
}

void invalidate_dcache_range(ulong start, ulong stop)
{
	if (start == stop)
		return;
	blast_inv_dcache_range(start, stop);
}

void flush_cache(ulong start_addr, ulong size)
{
	if (size == 0)
		return;

	blast_dcache_range(start_addr, start_addr + size);
	blast_icache_range(start_addr, start_addr + size);
}

void flush_dcache_all(void)
{
	blast_dcache32();
}

void flush_l2cache_all(void)
{
	blast_scache64();
}

void flush_cache_all(void)
{
	flush_icache_all(); /* invalid icache */

	flush_dcache_all(); /* writeback invalid dcache */
	__asm__ volatile(
		".set push     \n\t"
		".set mips32r2 \n\t"
		"sync          \n\t"
		".set pop      \n\t"
		);

	flush_l2cache_all(); /* writeback invalid l2cache */
	__asm__ volatile(
		".set push     \n\t"
		".set mips32r2 \n\t"
		"lw $0,0(%0)   \n\t"
		".set pop      \n\t"
		::"r" (0xa0000000));
}

#ifndef CONFIG_SPL_BUILD
DECLARE_GLOBAL_DATA_PTR;

/* The DDR the SPL set up, as the vendor's initdram() reports it */
int dram_init(void)
{
	gd->ram_size = PHYS_SDRAM_1_SIZE;
	return 0;
}

/*
 * KSEG0 reaches only the first 256 MiB below the I/O space; the vendor
 * kept U-Boot under CONFIG_SYS_SDRAM_MAX_TOP (0x90000000) for the same
 * reason.
 */
ulong board_get_usable_ram_top(ulong total_size)
{
	return CONFIG_SYS_SDRAM_BASE + min_t(ulong, total_size, SZ_256M);
}

int print_cpuinfo(void)
{
	printf("CPU:   Ingenic %s, %u MHz, DDR %u MHz\n", CONFIG_SYS_SOC,
	       clk_get_rate(CPU) / 1000000, clk_get_rate(DDR) / 1000000);
	return 0;
}
#endif
