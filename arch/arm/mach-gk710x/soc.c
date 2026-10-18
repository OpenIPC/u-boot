// SPDX-License-Identifier: GPL-2.0+
/*
 * Goke GK7101 / GK7102: clocks, timer, reset and DRAM size.
 */

#include <common.h>
#include <cpu_func.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/gk710x.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

int arch_cpu_init(void)
{
	/*
	 * The CPU PLL and the UART divider as the vendor U-Boot sets them
	 * (arm_freq 0x00112032 and REG_PLL_SCALER_UART 1), in silicon bit
	 * order: the HAL permutes the PLL word on its way to the register.
	 */
	writel(0x01203201, GK_RCT_PLL_CORE);
	writel(1, GK_RCT_SCALER_UART);
	return 0;
}

int timer_init(void)
{
	u32 ctrl = readl(GK_TIMER_CTRL) & ~0xf;

	/* Timer 1, free running over the whole 32 bits for lib/time.c */
	writel(ctrl, GK_TIMER_CTRL);
	writel(0xffffffff, GK_TIMER1_STATUS);
	writel(0xffffffff, GK_TIMER1_RELOAD);
	writel(ctrl | GK_TIMER_CTRL_EN, GK_TIMER_CTRL);
	gd->arch.tbl = 0;
	gd->arch.tbu = 0;
	return 0;
}

void reset_cpu(void)
{
	writel(0xff, GK_WDT_RELOAD);
	writel(GK_WDT_MAGIC, GK_WDT_RESTART);
	writel(GK_WDT_CTRL_EN | GK_WDT_CTRL_RESET, GK_WDT_CTRL);
	while (1)
		;
}

/* The boot header's DDR controller set-up tells the 128 MiB GK7101 apart */
int dram_init(void)
{
	ulong max = readl(GK_DDRC_CFG) == GK_DDRC_CFG_128M ? SZ_128M : SZ_64M;

	gd->ram_size = get_ram_size((long *)GK_DDR_BASE, max);
	return 0;
}

/* The top 64 KiB carry the GPIO map to the kernel: keep U-Boot below it */
ulong board_get_usable_ram_top(ulong total_size)
{
	return gd->ram_top - GK_USR_SIZE;
}

#ifdef CONFIG_DISPLAY_CPUINFO
int print_cpuinfo(void)
{
	printf("CPU:   Goke %s\n", CONFIG_PRODUCT_SOC);
	return 0;
}
#endif
