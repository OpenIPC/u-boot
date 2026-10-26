// SPDX-License-Identifier: GPL-2.0+
/*
 * Fullhan camera SoCs: clocks, timer, watchdog, reset, DRAM size, GPIO.
 *
 * The boot ROM has run the DDR (and PLL) script from the flash header
 * before U-Boot starts; the rest of the clock set-up is the vendor
 * U-Boots' clock_init().
 */

#include <common.h>
#include <cpu_func.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/setup.h>
#include <asm/arch/fullhan.h>
#include <linux/delay.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

void fh_pmu_update(unsigned long reg, u32 mask, u32 val)
{
	writel((readl(reg) & ~mask) | (val & mask), reg);
}

/* Pulse a block reset; the bits come back set when it is done */
void fh_pmu_reset(unsigned long reg, u32 bits)
{
	int t;

	writel(~bits, reg);
	for (t = 0; t < 10000 && readl(reg) != 0xffffffff; t++)
		udelay(1);
}

void fh_gpio_output(unsigned int gpio, int value)
{
	unsigned long base = gpio < 32 ? FH_GPIO0_BASE : FH_GPIO1_BASE;
	u32 bit = BIT(gpio % 32);

	if (value)
		setbits_le32(base + FH_GPIO_DR, bit);
	else
		clrbits_le32(base + FH_GPIO_DR, bit);
	setbits_le32(base + FH_GPIO_DDR, bit);
}

/* The vendor U-Boots' clock_init(): UART0, timer 0, PTS and watchdog */
static void fh_clock_init(void)
{
#if defined(CONFIG_FULLHAN_V200) || defined(CONFIG_FULLHAN_865X)
	fh_pmu_update(FH_PMU_CLK_DIV4, 0x1f, 5);		/* UART0 */
	fh_pmu_update(FH_PMU_CLK_DIV5, 0xff0000, 0x63 << 16);	/* timer 0 */
	fh_pmu_update(FH_PMU_CLK_DIV2, 0x1ff, 0x63);		/* PTS */
	fh_pmu_update(FH_PMU_CLK_DIV5, 0xff00, 0xc7 << 8);	/* WDT */
	clrbits_le32(FH_PMU_CLK_GATE, FH_GATE_UART0 | FH_GATE_TMR0);
#elif defined(CONFIG_FULLHAN_V100)
	setbits_le32(FH_PMU_CLK_GATE, 0x120a2280);
	fh_pmu_update(FH_PMU_CLK_DIV6, 0xf000000, 1 << 24);	/* GMAC 50 MHz */
	fh_pmu_update(FH_PMU_CLK_DIV4, 0x1f, 5);
	fh_pmu_update(FH_PMU_CLK_DIV5, 0xff0000, 0x63 << 16);
	fh_pmu_update(FH_PMU_CLK_DIV2, 0xff, 0x63);
	fh_pmu_update(FH_PMU_CLK_DIV5, 0xff00, 0x95 << 8);
	clrbits_le32(FH_PMU_CLK_GATE, 0x120a2280);
#elif defined(CONFIG_FULLHAN_8626)
	setbits_le32(FH_PMU_CLK_GATE, 0x120a2280);
	setbits_le32(FH_PMU_CLK_GATE1, 0x1c0);
	fh_pmu_update(FH_PMU_CLK_DIV2, 0xf000000, 1 << 24);	/* GMAC 50 MHz */
	fh_pmu_update(FH_PMU_CLK_DIV3, 0x1f, 5);		/* UART0 */
	fh_pmu_update(FH_PMU_CLK_DIV5, 0xff, 0x63);		/* timer 0 */
	fh_pmu_update(FH_PMU_CLK_DIV5, 0x1ff0000, 0x63 << 16);	/* PTS */
	fh_pmu_update(FH_PMU_CLK_DIV5, 0xff00, 0x7f << 8);	/* WDT */
	clrbits_le32(FH_PMU_CLK_GATE, 0x120a2280);
	clrbits_le32(FH_PMU_CLK_GATE1, 0x1c0);
#elif defined(CONFIG_FULLHAN_8632)
	/* From PLL1, 600 MHz: UART0 30 MHz, timer 0 and PTS 1 MHz */
	setbits_le32(FH_PMU_CLK_GATE, 0x720a2080);
	fh_pmu_update(FH_PMU_CLK_DIV4, 0x1f, 1);
	fh_pmu_update(FH_PMU_CLK_DIV5, 0xff0000, 0x3b << 16);
	fh_pmu_update(FH_PMU_CLK_DIV2, 0xff, 0x3b);
	fh_pmu_update(FH_PMU_CLK_DIV5, 0xff00, 0x3b << 8);
	/*
	 * Everything ungated, as the vendor's PHY reset leaves it: the
	 * kernel's clocksource, the PTS counter, sits in the PAE block,
	 * whose clock (bit 4) the kernel never enables itself.
	 */
	writel(0, FH_PMU_CLK_GATE);
#endif
}

/* The flash controller's clock: SCLK = FH_SPI0_CLK / BAUDR */
void fh_spi0_setup(void)
{
#if defined(CONFIG_FULLHAN_V200) || defined(CONFIG_FULLHAN_865X)
	fh_pmu_update(FH_PMU_CLK_DIV3, 0xff, 5);	/* 1200 MHz / 6 */
#elif defined(CONFIG_FULLHAN_V100)
	fh_pmu_update(FH_PMU_CLK_DIV3, 0xff, 0);	/* 100 MHz */
#elif defined(CONFIG_FULLHAN_8626)
	fh_pmu_update(FH_PMU_CLK_DIV2, 0xf00, 0);	/* 100 MHz */
#elif defined(CONFIG_FULLHAN_8632)
	fh_pmu_update(FH_PMU_CLK_DIV3, 0xff, 0x0b);	/* 600 MHz / 12 */
#endif
	clrbits_le32(FH_PMU_CLK_GATE, FH_GATE_SPI0);
}

int arch_cpu_init(void)
{
	fh_clock_init();

	/*
	 * Nothing here feeds the watchdog. Like the vendor U-Boots, find it
	 * running only if something before started it; it cannot be turned
	 * off then, so give it its longest period (minutes at these clocks).
	 */
	if (readl(FH_WDT_CR) & 1) {
		writel(15, FH_WDT_TORR);
		writel(FH_WDT_KICK, FH_WDT_CRR);
	}
	return 0;
}

int timer_init(void)
{
	/* Channel 0, free running over the whole 32 bits for lib/time.c */
	writel(FH_TIMER_CTRL_INTMASK, FH_TIMER0_CTRL);
	writel(0xffffffff, FH_TIMER0_LOAD);
	writel(FH_TIMER_CTRL_INTMASK | FH_TIMER_CTRL_EN, FH_TIMER0_CTRL);
	gd->arch.tbl = 0;
	gd->arch.tbu = 0;
	return 0;
}

void reset_cpu(void)
{
	/* Leave the flash deselected for the boot ROM */
	fh_gpio_output(FH_SPI0_CS_GPIO, 1);
	writel(~FH_SWRST_CHIP, FH_PMU_SWRST_MAIN);
	while (1)
		;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((long *)FH_DDR_BASE, CONFIG_FH_DDR_MAX);
	return 0;
}

#ifdef CONFIG_FH_ATAG_PHYMODE
/*
 * The 3.0.8 kernels' fh_gmac takes the MAC interface from a vendor tag and
 * runs MII without it; these boards are wired RMII.
 */
#define ATAG_PHYMODE		0x41000601
#define FH_PHYMODE_RMII		4

void setup_board_tags(struct tag **in_params)
{
	struct tag *t = *in_params;

	t->hdr.tag = ATAG_PHYMODE;
	t->hdr.size = (sizeof(struct tag_header) + sizeof(u32)) >> 2;
	*(u32 *)&t->u = FH_PHYMODE_RMII;
	*in_params = tag_next(t);
}
#endif

#ifdef CONFIG_DISPLAY_CPUINFO
int print_cpuinfo(void)
{
	printf("CPU:   Fullhan %s (ID %08x/%x)\n", CONFIG_PRODUCT_SOC,
	       readl(FH_PMU_CHIP_ID), readl(FH_PMU_PACKAGE_ID));
	return 0;
}
#endif
