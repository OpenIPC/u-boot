/*
 * Board init for Hi3516CV200 / Hi3518EV200 (ARM926EJS).
 */

#include <config.h>
#include <openipc.h>
#include <common.h>
#include <asm/io.h>
#include <asm/global_data.h>
#include <asm/arch/platform.h>
#include <asm/mach-types.h>
#include <env.h>
#include <netdev.h>
#include <hicpu_common.h>

static int boot_media = BOOT_MEDIA_UNKNOWN;

int get_boot_media(void)
{
	return boot_media;
}

int get_text_base(void)
{
	return CONFIG_SYS_TEXT_BASE;
}

static void boot_flag_init(void)
{
	unsigned long reg, boot_mode, spi_device_mode;

	reg = readl(SYS_CTRL_REG_BASE + REG_SYSSTAT);
	boot_mode = (reg >> BOOT_MODE_SHIFT) & 0x1;

	switch (boot_mode) {
	case 0:	/* spi mode: 0 = spi nor, 1 = spi nand */
		spi_device_mode = (reg >> FMC_DEVICE_MODE_SHIFT) & 0x1;
		if (spi_device_mode)
			boot_media = BOOT_MEDIA_NAND;
		else
			boot_media = BOOT_MEDIA_SPIFLASH;
		break;
	case 1:	/* emmc mode */
		boot_media = BOOT_MEDIA_EMMC;
		break;
	default:
		boot_media = BOOT_MEDIA_UNKNOWN;
		break;
	}
}

int board_eth_init(struct bd_info *bis)
{
	int rc = 0;

#ifdef CONFIG_NET_HISFV300
	rc = hieth_initialize(bis);
#endif
	return rc;
}

int board_early_init_f(void)
{
	return 0;
}

void reset_cpu(ulong addr)
{
	writel(0x2, SYS_CTRL_REG_BASE + REG_SC_SYSRES);
	while (1)
		;
}

int board_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	unsigned long reg;

	/* set uart clk from 24M */
	reg = readl(CRG_REG_BASE + PERI_CRG57);
	reg &= ~UART_CKSEL_MASK;
	reg |= UART_CKSEL_24M;
	writel(reg, CRG_REG_BASE + PERI_CRG57);

	gd->bd->bi_arch_number = MACH_TYPE_HI3518EV200;
	gd->bd->bi_boot_params = CFG_BOOT_PARAMS;

	boot_flag_init();

	return 0;
}

void detect_memory(void)
{
	ulong tested_ram = get_ram_size((long *)PHYS_SDRAM_1,
					PHYS_SDRAM_1_SIZE) / 1024 / 1024;
	char msize[128];

	printf("RAM size: %dMB\n", tested_ram);
	sprintf(msize, "%dM", tested_ram);
	env_set("totalmem", msize);
}

int misc_init_r(void)
{
	openipc_helper();
	detect_memory();
	env_set("verify", "n");

	return 0;
}

int dram_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	gd->ram_size = PHYS_SDRAM_1_SIZE;
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return 0;
}

int timer_init(void)
{
	/* Select APB clock as the reference clk for Timer0 */
	writel(readl(SYS_CTRL_REG_BASE + REG_SC_CTRL) | TIME0_CLK_APB,
	       SYS_CTRL_REG_BASE + REG_SC_CTRL);

	writel(0, CFG_TIMERBASE + REG_TIMER_CONTROL);
	writel(~0, CFG_TIMERBASE + REG_TIMER_RELOAD);
	/* 32 bit, periodic, 256 divider */
	writel(CFG_TIMER_CTRL, CFG_TIMERBASE + REG_TIMER_CONTROL);

	return 0;
}
