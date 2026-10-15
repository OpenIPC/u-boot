/*
 * Copyright (c) XMEDIA. All rights reserved.
 */
#include <config.h>
#include <openipc.h>
#include <common.h>
#include <cpu_func.h>
#include <asm/io.h>
#include <asm/arch/platform.h>
#include <spi_flash.h>
#include <linux/mtd/mtd.h>
#include <nand.h>
#include <netdev.h>
#include <env.h>
#include <asm/sections.h>
#include <asm/mach-types.h>

#ifndef CONFIG_SYS_DCACHE_OFF
void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}
#endif

/* In .data: get_boot_media() is called before relocation clears .bss */
static int boot_media __section(".data") = BOOT_MEDIA_UNKNOWN;
int get_boot_media(void)
{
	unsigned int reg_val, boot_mode;

	if (boot_media != BOOT_MEDIA_UNKNOWN)
		return boot_media;

	reg_val = readl(SYS_CTRL_REG_BASE + REG_SYSSTAT);
	boot_mode = get_sys_boot_mode(reg_val);

	switch (boot_mode) {
	case BOOT_FROM_SPI_NOR:
		boot_media = BOOT_MEDIA_SPIFLASH;
		break;
	case BOOT_FROM_SPI_NAND:
		boot_media = BOOT_MEDIA_NAND;
		break;
	case BOOT_FROM_EMMC_4BITS:
	case BOOT_FROM_EMMC_8BITS:
		boot_media = BOOT_MEDIA_EMMC;
		break;
	default:
		boot_media = BOOT_MEDIA_UNKNOWN;
		break;
	}

	return boot_media;
}

unsigned int get_chipid(void)
{
	return readl(REG_SC_CHIPID);
}

#if defined(CONFIG_SHOW_BOOT_PROGRESS)
void show_boot_progress(int progress)
{
	printf("Boot reached stage %d\n", progress);
}
#endif

int get_text_base(void)
{
	return CONFIG_SYS_TEXT_BASE;
}

int board_early_init_f(void)
{
	return 0;
}

int misc_init_r(void)
{
	openipc_helper();
#ifdef CONFIG_RANDOM_ETHADDR
	random_init_r();
#endif
	env_set("verify", "n");

	return 0;
}

int board_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gd->bd->bi_arch_number = MACH_TYPE_XMORCA;
	gd->bd->bi_boot_params = CFG_BOOT_PARAMS;

	get_boot_media();

	return 0;
}

int dram_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gd->ram_size = openipc_ram_size();
	return 0;
}

void reset_cpu(void)
{
	/* 0x12345678:writing any value will cause a reset. */
	writel(0x12345678, SYS_CTRL_REG_BASE + REG_SC_SYSRES);
	while (1);
}

int timer_init(void)
{
	/*
	 * Under uboot, 0xffffffff is set to load register,
	 * timer_clk equals BUSCLK/2/256.
	 * e.g. BUSCLK equals 50M, it will roll back after 0xffffffff/timer_clk
	 * 43980s equals 12hours
	 */
	__raw_writel(0, TIMER0_REG_BASE + REG_TIMER_CONTROL);
	__raw_writel(~0, TIMER0_REG_BASE + REG_TIMER_RELOAD);

	/* 32 bit, periodic */
	__raw_writel(CFG_TIMER_CTRL, TIMER0_REG_BASE + REG_TIMER_CONTROL);

	return 0;
}

int board_eth_init(struct bd_info *bis)
{
	int rc = 0;

#ifdef CONFIG_FTGMAC030
	rc = ftgmac030_initialize(bis);
#endif
	return rc;
}

#ifdef CONFIG_ARMV7_NONSEC
void smp_set_core_boot_addr(unsigned long addr, int corenr)
{
}

void smp_kick_all_cpus(void)
{
}

void smp_waitloop(unsigned previous_address)
{
}
#endif
