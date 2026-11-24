// SPDX-License-Identifier: GPL-2.0+
/*
 * Grain Media GM8126: clocks, timer, watchdog reset and DRAM size.
 *
 * nsboot has programmed DDR and the PLLs before U-Boot starts; the rest is
 * the GM8136 SDK U-Boot's board/gm/gm8126.
 */

#include <common.h>
#include <cpu_func.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/gm8126.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

#define GM_OSC_HZ	30000000

/*
 * PLL1 reads back shifted left by one (MP parts): MUL at 12:4, DIV at
 * 17:13, the clock mode from bit 25
 */
unsigned long gm_pll1_hz(void)
{
	u32 v = readl(GM_PMU_PLL1);
	u32 div = (v >> 13) & 0x1f;

	return GM_OSC_HZ / (div ? div : 1) * ((v >> 4) & 0x1ff);
}

unsigned long gm_pll2_hz(void)
{
	u32 v = readl(GM_PMU_PLL2);
	u32 div = (v >> 12) & 0x1f;

	return GM_OSC_HZ / (div ? div : 1) * ((v >> 3) & 0x1ff);
}

unsigned long gm_ahb_hz(void)
{
	switch ((readl(GM_PMU_PLL1) >> 25) & 3) {
	case 0:
		return gm_pll1_hz();
	case 1:
		return gm_pll1_hz() / 2;
	case 2:
		return gm_pll1_hz() / 3;
	default:
		return gm_pll2_hz() / 2;
	}
}

static unsigned long gm_cpu_hz(void)
{
	switch ((readl(GM_PMU_PLL1) >> 27) & 3) {
	case 0:
		return gm_pll1_hz();
	case 1:
		return gm_pll1_hz() / 2;
	case 2:
		return gm_pll1_hz() / 3 * 2;
	default:
		return gm_pll2_hz();
	}
}

/* APB: AHB / 2, what the timers and the watchdog count */
unsigned long gm_apb_hz(void)
{
	return gm_ahb_hz() / 2;
}

/* SSP0: PLL2 over the divider nsboot set */
unsigned long gm_ssp_hz(void)
{
	return gm_pll2_hz() / ((readl(GM_PMU_SSP_DIV) & 0x3f) + 1);
}

/*
 * The vendor's init_sys_freq(), less the PLLs (nsboot's) and the LCD's
 * clock: bus priorities, the clocks on, the blocks nothing here uses off.
 */
int arch_cpu_init(void)
{
	writel(2, GM_AHBC1_BASE + 0x80);		/* LCD */
	writel(6, GM_AHBC4_BASE + 0x80);		/* capture */
	writel(6, GM_AHBC5_BASE + 0x80);

	clrbits_le32(GM_PMU_REG(0x08), 0xfff00000);	/* PLL charge pumps */
	setbits_le32(GM_PMU_REG(0x0c), BIT(12));	/* ISP off */
	writel(0xf87ff2e3, GM_PMU_GATE_AHB);
	/* SSP0, GPIO0 and, for reset_cpu(), the watchdog stay on */
	writel(0xfbffadde & ~GM_GATE_WDT, GM_PMU_GATE_APB);
	setbits_le32(GM_PMU_REG(0x6c), BIT(17));	/* the sensor's clock */
	setbits_le32(GM_PMU_REG(0x7c), 0xf);		/* TV encoder off */
	setbits_le32(GM_PMU_REG(0x90), 0x00780000);	/* ADC off */
	setbits_le32(GM_PMU_REG(0x9c), 0x03007000);	/* audio off */
	writel(0x200200, GM_PMU_REG(0x44));		/* clock trim */
	return 0;
}

int timer_init(void)
{
	/* channel 0 counts APB clocks down; the interrupt masks stay */
	clrbits_le32(GM_TIMER_CR, 7);
	writel(0xffffffff, GM_TIMER0_LOAD);
	writel(0xffffffff, GM_TIMER0_COUNTER);
	writel(0, GM_TIMER0_MATCH1);
	writel(0, GM_TIMER0_MATCH2);
	setbits_le32(GM_TIMER_CR, GM_TIMER_CR_TM1_EN);
	gd->timebase_h = 0;
	gd->timebase_l = 0;
	return 0;
}

void reset_cpu(void)
{
	clrbits_le32(GM_PMU_GATE_APB, GM_GATE_WDT);
	writel(0, GM_WDT_CR);
	writel(1, GM_WDT_LOAD);
	writel(GM_WDT_KICK, GM_WDT_RESTART);
	writel(GM_WDT_CR_EN | GM_WDT_CR_RST, GM_WDT_CR);
	while (1)
		;
}

/*
 * nsboot's table is for 128 MiB; a smaller part aliases past its end.
 * Probed well above U-Boot (loaded at 0) and its early stack.
 */
#define GM_PROBE	0x01000000

int dram_init(void)
{
	volatile u32 *lo = (u32 *)GM_PROBE;
	volatile u32 *hi = (u32 *)(GM_PROBE + SZ_64M);
	u32 save_lo = *lo, save_hi = *hi;

	*lo = 0x5a5aa5a5;
	*hi = 0xa5a55a5a;
	gd->ram_size = *hi == 0xa5a55a5a && *lo == 0x5a5aa5a5 ? SZ_128M
							      : SZ_64M;
	*hi = save_hi;
	*lo = save_lo;
	return 0;
}

#ifdef CONFIG_DISPLAY_CPUINFO
int print_cpuinfo(void)
{
	printf("CPU:   Grain Media GM8126 (ID %08x), %lu MHz, AHB %lu MHz, DDR %lu MHz\n",
	       readl(GM_PMU_ID), gm_cpu_hz() / 1000000,
	       gm_ahb_hz() / 1000000, gm_pll1_hz() / 1000000);
	return 0;
}
#endif
