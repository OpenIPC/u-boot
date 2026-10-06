/*
 * Board init for Hi3516CV300 / Hi3516EV100 (ARM926EJS).
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
	boot_mode = (reg & BOOT_MODE_MASK) >> BOOT_MODE_SHIFT;

	switch (boot_mode) {
	case BOOT_FROM_FMC:
		spi_device_mode = GET_SPI_DEVICE_TYPE(reg);
		if (spi_device_mode)
			boot_media = BOOT_MEDIA_NAND;
		else
			boot_media = BOOT_MEDIA_SPIFLASH;
		break;
	case BOOT_FROM_EMMC:
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

	/* UART0: release reset, enable clock, 24M */
	reg = readl(CRG_REG_BASE + PERI_CRG57);
	reg &= ~UART0_SRST;
	reg |= UART0_CLK_EN;
	reg &= ~UART_CLK_SEL_MASK;
	reg |= UART_CLK_SEL_24M << UART_CLK_SEL_SHIFT;
	writel(reg, CRG_REG_BASE + PERI_CRG57);

	gd->bd->bi_arch_number = MACH_TYPE_HI3516CV300;
	gd->bd->bi_boot_params = CFG_BOOT_PARAMS;

	boot_flag_init();

	return 0;
}

int misc_init_r(void)
{
	openipc_helper();
	env_set("verify", "n");

	return 0;
}

int dram_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	gd->ram_size = openipc_ram_size();
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = gd->ram_size;

	return 0;
}

int timer_init(void)
{
	unsigned long reg;

	/* Select APB clock as the reference clk for Timer0 */
	reg = readl(SYS_CTRL_REG_BASE + REG_SC_CTRL);
	reg &= ~TIME0_CLK_SEL;
	reg |= (TIME0_CLK_SEL_APB << TIME0_CLK_SEL_SHIFT);
	writel(reg, SYS_CTRL_REG_BASE + REG_SC_CTRL);

	writel(0, CFG_TIMERBASE + REG_TIMER_CONTROL);
	writel(~0, CFG_TIMERBASE + REG_TIMER_RELOAD);
	/* 32 bit, periodic, 256 divider */
	writel(CFG_TIMER_CTRL, CFG_TIMERBASE + REG_TIMER_CONTROL);

	return 0;
}
