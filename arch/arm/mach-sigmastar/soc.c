// SPDX-License-Identifier: GPL-2.0+
/*
 * SigmaStar infinity3: timer, reset, DRAM size, MIU pipe flush.
 * After the vendor U-Boot 2015.01 (arch/arm/cpu/armv7/infinity3).
 */

#include <common.h>
#include <cpu_func.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/sigmastar.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * PIU timer 0, free running up to its 32-bit maximum at the crystal rate,
 * as the vendor U-Boot runs it.
 */
int timer_init(void)
{
	writew(0, SS_TIMER0_CTRL);
	writew(0xffff, SS_TIMER0_MAX_L);
	writew(0xffff, SS_TIMER0_MAX_H);
	writew(SS_TIMER_EN, SS_TIMER0_CTRL);
	gd->timebase_h = 0;
	gd->timebase_l = 0;
	return 0;
}

unsigned long notrace timer_read_counter(void)
{
	u16 hi, lo;

	/* The halves are not latched together */
	do {
		hi = readw(SS_TIMER0_CNT_H);
		lo = readw(SS_TIMER0_CNT_L);
	} while (hi != readw(SS_TIMER0_CNT_H));
	return hi << 16 | lo;
}

void reset_cpu(void)
{
	writew(0xff, SS_PM_RESET);
	writew(SS_PM_RESET_KEY, SS_PM_RESET);
	while (1)
		;
}

/*
 * The IPL has sized and trained the DDR in the package. It wraps past its
 * end, which get_ram_size() sees.
 */
int dram_init(void)
{
	gd->ram_size = get_ram_size((long *)SS_DDR_BASE, PHYS_SDRAM_1_SIZE);
	return 0;
}

/*
 * The MIU buffers writes: what the CPU stored is in DDR, for a DMA engine
 * to read, once the L3 bridge's flush reports done.
 */
void ss_miu_flush(void)
{
	int spins = 1000000;

	writew(0x10, SS_L3_FLUSH);
	writew(0x11, SS_L3_FLUSH);
	while (!(readw(SS_L3_FLUSH_STATUS) & SS_L3_FLUSH_DONE) && --spins)
		;
}

#ifdef CONFIG_DISPLAY_CPUINFO
int print_cpuinfo(void)
{
	printf("CPU:   SigmaStar %s (chip id %02x)\n", CONFIG_PRODUCT_SOC,
	       readw(SS_CHIP_ID) & 0xff);
	return 0;
}
#endif
