/*
 * Board init for Hi3516AV100 / Hi3516DV100 (ARMv7-a).
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
	unsigned long reg = readl(SYS_CTRL_REG_BASE + REG_SYSSTAT);
	unsigned long ret = (reg >> 5) & 0x1;

	switch (ret) {
	case 0x0:
		if ((reg >> 3) & 0x1)
			boot_media = BOOT_MEDIA_NAND;
		else
			boot_media = BOOT_MEDIA_SPIFLASH;
		break;
	case 0x1:
	default:
		boot_media = BOOT_MEDIA_NAND;
		break;
	}
}

int board_eth_init(struct bd_info *bis)
{
	int rc = 0;

#ifdef CONFIG_NET_HIGMACV300
	rc = higmac_initialize(bis);
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

	/* set uart clk from apb bus */
	reg = readl(CRG_REG_BASE + PERI_CRG57);
	reg &= ~UART_CKSEL_APB;
	writel(reg, CRG_REG_BASE + PERI_CRG57);

	gd->bd->bi_arch_number = MACH_TYPE_HI3516A;
	gd->bd->bi_boot_params = CFG_BOOT_PARAMS;

	boot_flag_init();

	return 0;
}

static void do_phy_init(void)
{
	char *mdio_intf = env_get("mdio_intf");

	if (!mdio_intf)
		return;

	printf("PHY Init... ");
	if (!strncmp(mdio_intf, "rmii", 4) || !strncmp(mdio_intf, "mii", 3)) {
		writel(0x2, 0x200f0174);	/* GPIO4_0 RMII_CLK_OUT/MII_TX_CLK */
		printf("rmii/mii\n");
	} else if (!strncmp(mdio_intf, "rgmii", 5)) {
		writel(0x1, 0x200f0174);	/* GPIO4_0 RGMII_TX_CLK */
		printf("rgmii\n");
	}
}

int misc_init_r(void)
{
	openipc_helper();
	env_set("verify", "n");
	do_phy_init();

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
	writel(0, CFG_TIMERBASE + REG_TIMER_CONTROL);
	writel(~0, CFG_TIMERBASE + REG_TIMER_RELOAD);
	/* 32 bit, periodic, 256 divider */
	writel(CFG_TIMER_CTRL, CFG_TIMERBASE + REG_TIMER_CONTROL);

	return 0;
}
