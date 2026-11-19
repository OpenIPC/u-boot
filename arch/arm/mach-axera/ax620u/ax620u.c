// SPDX-License-Identifier: GPL-2.0+
/*
 * Axera AX620U. U-Boot is the second stage: the first stage has trained
 * the DDR and set the PLLs and the boot clocks, and nothing here touches
 * them. The timer is the Cortex-A7 generic timer (SYS_ARCH_TIMER).
 */
#include <common.h>
#include <cpu_func.h>
#include <fdtdec.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

/* Chip software reset, as the stock AX620U U-Boot does it */
#define AX620U_CHIP_RST		0x02220044
#define AX620U_CHIP_RST_VAL	0x81

/*
 * The DesignWare watchdog, the stock kernel's restart handler: shortest
 * timeout, enabled in reset mode (or kicked, if already running)
 */
#define AX620U_WDT		0x049aa000
#define DW_WDT_CR		0x00
#define DW_WDT_CR_EN		BIT(0)
#define DW_WDT_TORR		0x04
#define DW_WDT_CRR		0x0c
#define DW_WDT_CRR_KICK		0x76

void reset_cpu(void)
{
	writel(AX620U_CHIP_RST_VAL, AX620U_CHIP_RST);
	udelay(10000);

	writel(0, AX620U_WDT + DW_WDT_TORR);
	if (readl(AX620U_WDT + DW_WDT_CR) & DW_WDT_CR_EN)
		writel(DW_WDT_CRR_KICK, AX620U_WDT + DW_WDT_CRR);
	else
		writel(DW_WDT_CR_EN, AX620U_WDT + DW_WDT_CR);
	while (1)
		;
}

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}

#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
void enable_caches(void)
{
	icache_enable();
	dcache_enable();
}
#endif

#ifdef CONFIG_DISPLAY_CPUINFO
int print_cpuinfo(void)
{
	printf("SoC:   Axera AX620U\n");
	return 0;
}
#endif
