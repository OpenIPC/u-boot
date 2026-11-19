// SPDX-License-Identifier: GPL-2.0+
/*
 * Axera AX620E: memory map, DDR, system counter, watchdog and reset.
 *
 * Written from the vendor SDK's U-Boot 2020.04 (arch/arm/mach-axera/ax620e:
 * ax620e.c, timer.c, chip_config.c). The first stage has set up the DDR,
 * the clocks and the pads U-Boot needs (UART0, the boot SPI flash) before
 * it starts U-Boot.
 */

#include <common.h>
#include <cpu_func.h>
#include <init.h>
#include <linux/delay.h>
#include <asm/armv8/mmu.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/sizes.h>
#include <mach/ax620e.h>

DECLARE_GLOBAL_DATA_PTR;

static struct mm_region ax620e_mem_map[] = {
	{
		/* Normal memory over the DRAM there is: dram_init() sizes it */
		.virt = AX_DDR_BASE,
		.phys = AX_DDR_BASE,
		.size = SZ_256M,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		/* The SoC's registers, IRAM included */
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0x10500000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = ax620e_mem_map;

/*
 * The CoreSight system counter behind the generic timer: 24 MHz, enabled
 * here as the vendor's generic_timer_init() does, in case the first stage
 * has not.
 */
static void ax620e_counter_init(void)
{
	if (readl(AX_SYS_COUNTER_CNTCR) & BIT(0))
		return;
	writel(0, AX_SYS_COUNTER_CNTCV_LO);
	writel(0, AX_SYS_COUNTER_CNTCV_HI);
	writel(AX_SYS_COUNTER_FREQ, AX_SYS_COUNTER_CNTFID0);
	writel(0x11, AX_SYS_COUNTER_CNTCR);
}

int arch_cpu_init(void)
{
	ax620e_counter_init();
	/*
	 * Watchdog 0 off, as the vendor's chip_init() does: a camera left at
	 * the prompt, or loading a large image over TFTP, must not reset.
	 * The kernel's own watchdog driver arms it again.
	 */
	writel(0, AX_WDT0_BASE);
	return 0;
}

/*
 * The DDR: 256 MiB in the AX620Q package, at 0x40000000. U-Boot needs no
 * more, and the kernel's share is mem= in the environment.
 */
int dram_init(void)
{
	gd->ram_size = get_ram_size((long *)AX_DDR_BASE, SZ_256M);
	/* No cacheable mapping past it: speculation there may fault */
	ax620e_mem_map[0].size = gd->ram_size;
	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = AX_DDR_BASE;
	gd->bd->bi_dram[0].size = gd->ram_size;
	return 0;
}

/*
 * Software chip reset, as the vendor U-Boot's chip_rst_sw(): clear the
 * boot ROM's download and kernel-fail flags first, so the next boot is a
 * normal one. Should that not reset, watchdog 0 does, as the kernel's
 * restart handler drives it: 24 MHz, enabled, a zero timeout loaded.
 */
void reset_cpu(void)
{
	clrbits_le32(AX_CHIPMODE_GLB_BACKUP0,
		     AX_BOOT_DOWNLOAD | AX_BOOT_KERNEL_FAIL);
	setbits_le32(AX_COMM_ABORT_CFG, AX_CHIP_RST_SW);
	mdelay(10);

	writel(AX_WDT0_CLK_24M, AX_PERI_SYS_GLB_CLK_MUX0_SET);
	writel(1, AX_WDT0_BASE + AX_WDT_EN);
	for (int i = 0; i < 2; i++) {
		writel(0, AX_WDT0_BASE + AX_WDT_TIMEOUT);
		writel(1, AX_WDT0_BASE + AX_WDT_TIMEOUT_LOAD);
		udelay(40);
		writel(0, AX_WDT0_BASE + AX_WDT_TIMEOUT_LOAD);
	}
	while (1)
		;
}

#ifdef CONFIG_DISPLAY_CPUINFO
int print_cpuinfo(void)
{
	struct ax_misc_info *mi = (void *)AX_MISC_INFO_ADDR;

	printf("SoC:   Axera AX620E (chip type %u)\n", mi->chip_type);
	return 0;
}
#endif
