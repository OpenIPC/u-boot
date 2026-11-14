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

#ifdef CONFIG_AK39_EV300L
unsigned long ak_pll2_clk(void)
{
	u32 reg = readl(AK_SYS_PLL2);

	return AK_EV300L_XTAL_HZ / 1000 * (AK_PLL2_F(reg) + 1) /
	       (AK_PLL2_R(reg) + 1) / (AK_PLL2_OD(reg) + 1) * 1000;
}

/* GCLK, the vendor U-Boot's get_asic_freq() */
unsigned long ak_asic_clk(void)
{
	return ak_pll2_clk() / 2 / (AK_PLL2_GDIV(readl(AK_SYS_PLL2)) + 1);
}

/*
 * PLL2 to 400 MHz and GCLK to 200 MHz, as the vendor U-Boot's fixed-clk
 * driver does from its device tree, and the vendor kernel again from its
 * own: wait for an earlier change to be taken, write the new values, set
 * the bandwidth, then ask for the change (PLL1 bit 28, which clears when
 * it is done). The boot header leaves PLL2 alone.
 */
int arch_cpu_init(void)
{
	int spins = 1000000;

	while ((readl(AK_SYS_PLL1) & AK_PLL1_PLL2_ADJ) && --spins)
		;
	if ((readl(AK_SYS_PLL2) & AK_PLL2_CFG_MASK) ==
	    (AK_EV300L_PLL2 & AK_PLL2_CFG_MASK))
		return 0;
	writel(AK_EV300L_PLL2, AK_SYS_PLL2);
	clrsetbits_le32(AK_SYS_PLL_BW, 0xfff, AK_EV300L_PLL_BW);
	setbits_le32(AK_SYS_PLL1, AK_PLL1_PLL2_ADJ);
	spins = 1000000;
	while ((readl(AK_SYS_PLL1) & AK_PLL1_PLL2_ADJ) && --spins)
		;
	return 0;
}
#else
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
#endif

#ifdef CONFIG_AK39_EV200
/*
 * The ASIC PLL: CPU 400, memory 200, ASIC (peripheral) 100 MHz, as the EV200
 * vendor U-Boot and the EV200 U-Boot port both set it before anything else.
 * Out of reset it gives 60 MHz. Bit 28 of the CPU PLL register applies it.
 */
int arch_cpu_init(void)
{
	int spins = 1000000;

	writel(AK_EV200_ASIC_PLL, AK_SYS_ASIC_PLL);
	setbits_le32(AK_SYS_CPU_PLL, AK_PLL_APPLY);
	while ((readl(AK_SYS_CPU_PLL) & AK_PLL_APPLY) && --spins)
		;
	return 0;
}
#endif

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

/*
 * The watchdog, armed the way the vendor U-Boots do it: an 8 s count,
 * enable, enable and feed; then again with a count of 1, which is what
 * resets the part at once. A single write of count 1 with enable and feed
 * hangs an AK3918EV200.
 */
static void ak_wdt_arm(u32 count)
{
	writel(AK_WDT_CFG1_KEY | count, AK_SYS_WDT_CFG1);
	writel(AK_WDT_CFG2_KEY | AK_WDT_EN, AK_SYS_WDT_CFG2);
	writel(AK_WDT_CFG2_KEY | AK_WDT_EN | AK_WDT_FEED, AK_SYS_WDT_CFG2);
}

void reset_cpu(void)
{
	ak_wdt_arm(AK_WDT_COUNT_8S);
	ak_wdt_arm(1);
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
