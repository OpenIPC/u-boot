// SPDX-License-Identifier: GPL-2.0+
/*
 * Grain Media GM8135S / GM8136 / GM8136S: clocks, timer, watchdog reset
 * and DRAM size.
 *
 * nsboot has programmed DDR and the PLLs before U-Boot starts; the rest
 * is the SDK U-Boot's init_sys_freq() and timer.c.
 */

#include <common.h>
#include <cpu_func.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/grainmedia.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

#define GM_OSC_HZ	30000000

/*
 * 30 MHz x N (bits 10:4) / M (bits 15:11), divided by 8, 4, 2 or 1 as
 * the range (bits 3:2) is 0 to 3
 */
static unsigned long gm_pll_hz(u32 reg)
{
	unsigned long m = (reg >> 11) & 0x1f;
	unsigned long hz = GM_OSC_HZ * ((reg >> 4) & 0x7f) / (m ? m : 1);

	return hz >> (3 - ((reg >> 2) & 3));
}

unsigned long gm_pll1_hz(void)
{
	return gm_pll_hz(readl(GM_PMU_PLL1));
}

unsigned long gm_pll2_hz(void)
{
	return gm_pll_hz(readl(GM_PMU_PLL23));
}

/* AHB: PLL1 / 3, PLL2 / 3 or PLL2 / 4 (PLL1 register bits 19:18) */
unsigned long gm_ahb_hz(void)
{
	switch ((readl(GM_PMU_PLL1) >> 18) & 3) {
	case 0:
		return gm_pll1_hz() / 3;
	case 1:
		return gm_pll2_hz() / 3;
	default:
		return gm_pll2_hz() / 4;
	}
}

/* APB: AHB / 2, what the timers and the watchdog count */
unsigned long gm_apb_hz(void)
{
	return gm_ahb_hz() / 2;
}

int arch_cpu_init(void)
{
	/* The SDK U-Boot's init_sys_freq(): no automatic watchdog... */
	setbits_le32(GM_PMU_MISC, GM_MISC_NO_AUTO_WDT);
	/* ...and the clocks of what U-Boot uses */
	clrbits_le32(GM_PMU_GATE_APB, GM_APB_UART0 | GM_APB_WDT | GM_APB_TIMER);
	clrbits_le32(GM_PMU_GATE_AHB, GM_AHB_SPI020);
	return 0;
}

int timer_init(void)
{
	/*
	 * Channel 0 counts APB clocks down over the whole 32 bits, its
	 * overflow interrupt off (CR bit 2). The interrupt mask stays as it
	 * is: the vendor kernel never clears it, and its tick would stop.
	 */
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

/* Through the watchdog, as the SDK U-Boot and kernel do */
void reset_cpu(void)
{
	writel(0, GM_WDT_CR);
	writel(1, GM_WDT_LOAD);
	writel(GM_WDT_KICK, GM_WDT_RESTART);
	writel(GM_WDT_CR_EN | GM_WDT_CR_RST, GM_WDT_CR);
	while (1)
		;
}

/*
 * nsboot's table leaves the DRAM size code in the DDR controller (2 is
 * 64 MiB ... 5 is 512 MiB), which is what the SDK's U-Boot and kernel
 * trust. A part smaller than the table says aliases past its end, or is
 * absent there; look for that well above U-Boot (loaded at 0) and its
 * early stack.
 */
#define GM_PROBE	0x01000000

static unsigned long gm_ram_size(unsigned long max)
{
	volatile u32 *lo = (u32 *)(GM_DDR_BASE + GM_PROBE);
	unsigned long size;

	for (size = SZ_64M; size < max; size <<= 1) {
		volatile u32 *hi = (u32 *)(GM_DDR_BASE + GM_PROBE + size);
		u32 save_lo = *lo, save_hi = *hi;
		int ends;

		*lo = 0x5a5aa5a5;
		*hi = 0xa5a55a5a;
		ends = *hi != 0xa5a55a5a || *lo != 0x5a5aa5a5;
		*hi = save_hi;
		*lo = save_lo;
		if (ends)
			break;
	}
	return size;
}

int dram_init(void)
{
	unsigned int code = readl(GM_DDRC_SIZE) & 7;
	unsigned long max = code >= 2 && code <= 5 ? SZ_16M << code : SZ_64M;

	gd->ram_size = gm_ram_size(max);
	return 0;
}

#ifdef CONFIG_DISPLAY_CPUINFO
int print_cpuinfo(void)
{
	u32 id = readl(GM_PMU_ID);
	u32 pkg = GM_PACKAGE(id);

	printf("CPU:   Grain Media %s (ID %08x), %lu MHz, AHB %lu MHz\n",
	       pkg == GM_PACKAGE_8136 ? "GM8136" :
	       GM_PACKAGE_8136S(pkg) ? "GM8136S" : "GM8135S",
	       id, gm_pll1_hz() / 1000000, gm_ahb_hz() / 1000000);
	return 0;
}
#endif
