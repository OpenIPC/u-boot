// SPDX-License-Identifier: GPL-2.0+
/*
 * Anyka AK39: clocks, timer, reset and DRAM size.
 */

#include <common.h>
#include <cpu_func.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/anyka.h>

DECLARE_GLOBAL_DATA_PTR;

unsigned long ak_asic_clk(void)
{
	u32 reg = readl(AK_SYS_ASIC_PLL);
	unsigned int m = reg & 0xff;
	unsigned int n = (reg >> 8) & 0xf;
	unsigned int od = (reg >> 12) & 0x3;
	unsigned int div = (reg >> 17) & 0x7;
	unsigned long clk;

	if (!n)
		return 0;
	clk = AK_XTAL_HZ / 1000000 * m / (n << od);
	clk >>= div ? div : 1;
	if (reg & BIT(24))
		clk >>= 1;
	return clk * 1000000;
}

/*
 * Timer 1, free running from 0xffffffff at the crystal rate. The count is
 * read back through CTRL1 while READ_SEL is set.
 */
int timer_init(void)
{
	writel(0xffffffff, AK_SYS_TIMER1_CTRL1);
	writel(AK_TIMER_EN | AK_TIMER_FEED, AK_SYS_TIMER1_CTRL2);
	gd->timebase_h = 0;
	gd->timebase_l = 0;
	return 0;
}

unsigned long notrace timer_read_counter(void)
{
	u32 ctrl = readl(AK_SYS_TIMER1_CTRL2) & ~AK_TIMER_FEED;
	u32 count;

	writel(ctrl | AK_TIMER_READ_SEL, AK_SYS_TIMER1_CTRL2);
	count = readl(AK_SYS_TIMER1_CTRL1);
	writel(ctrl & ~AK_TIMER_READ_SEL, AK_SYS_TIMER1_CTRL2);
	return ~count;
}

/* The watchdog, at its shortest count */
void reset_cpu(void)
{
	writel(AK_WDT_CFG1_KEY | 1, AK_SYS_WDT_CFG1);
	writel(AK_WDT_CFG2_KEY | AK_WDT_EN | AK_WDT_FEED, AK_SYS_WDT_CFG2);
	while (1)
		;
}

/*
 * The DDR2 die is in the package: its size is the part's. Probing past it
 * is not safe, as nothing is known to alias there.
 */
int dram_init(void)
{
	gd->ram_size = get_ram_size((long *)AK_DDR_BASE, PHYS_SDRAM_1_SIZE);
	return 0;
}

#ifdef CONFIG_DISPLAY_CPUINFO
int print_cpuinfo(void)
{
	printf("CPU:   Anyka %s (chip id %08x), ASIC %lu MHz\n",
	       CONFIG_PRODUCT_SOC, readl(AK_SYS_CHIP_ID),
	       ak_asic_clk() / 1000000);
	return 0;
}
#endif
