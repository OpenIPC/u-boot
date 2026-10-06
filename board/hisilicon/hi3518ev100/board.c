/*
 * Board init for Hi3518EV100 (ARM926EJS).
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
	unsigned long ret;

	/* SCTL SYSSTAT bit5: 0 - SPI flash, 1 - NAND */
	ret = (readl(REG_BASE_SCTL + REG_SYSSTAT) >> 5) & 0x1;

	switch (ret) {
	case 0x0:
		boot_media = BOOT_MEDIA_SPIFLASH;
		break;
	case 0x1:
		boot_media = BOOT_MEDIA_NAND;
		break;
	default:
		boot_media = BOOT_MEDIA_UNKNOWN;
		break;
	}
}

#ifdef CONFIG_USB_OHCI_HCD
#define PERI_CRG46		(CRG_REG_BASE + 0xb8)
#define USB_CKEN		(1 << 7)
#define USB_CTRL_UTMI0_REG	(1 << 5)
#define USB_CTRL_HUB_REG	(1 << 4)
#define USBPHY_PORT0_TREQ	(1 << 2)
#define USBPHY_REQ		(1 << 1)
#define USB_AHB_SRST_REQ	(1 << 0)
#define PERI_USB		(REG_BASE_SCTL + 0x80)
#define WORDINTERFACE		(1 << 0)
#define ULPI_BYPASS_EN		(1 << 3)
#define SS_BURST16_EN		(1 << 9)
#define USBOVR_P_CTRL		(1 << 17)
#define PERI1_USB		(REG_BASE_SCTL + 0x84)
#define USBPHY_SHUTDOWN		(1 << 22)

int board_usb_init(int index, enum usb_init_type init)
{
	unsigned int reg;

	reg = readl(PERI1_USB);
	reg &= ~USBPHY_SHUTDOWN;
	writel(reg, PERI1_USB);

	reg = readl(PERI_CRG46);
	reg |= USB_CKEN;
	reg &= ~(USB_CTRL_UTMI0_REG | USB_CTRL_HUB_REG |
		 USBPHY_PORT0_TREQ | USBPHY_REQ | USB_AHB_SRST_REQ);
	writel(reg, PERI_CRG46);
	udelay(10);

	reg = readl(PERI_USB);
	reg |= ULPI_BYPASS_EN;
	reg &= ~(WORDINTERFACE | SS_BURST16_EN | USBOVR_P_CTRL);
	writel(reg, PERI_USB);
	udelay(10);

	return 0;
}
#endif

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
	writel(0x2, REG_BASE_SCTL + REG_SC_SYSRES);
	while (1)
		;
}

int board_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gd->bd->bi_arch_number = MACH_TYPE_HI3518EV100;
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
	if (!strncmp(mdio_intf, "mii", 3)) {
		writel(0x1, 0x200f005c);	/* 25 MHz RMII */
		writel(0x0, 0x200f0070);	/* GPIO 1_3 PHYRST */
		writel(0x2, 0x200300cc);	/* MII mode */
		printf("mii\n");
	} else if (!strncmp(mdio_intf, "rmii", 4)) {
		writel(0x3, 0x200f005c);	/* 50 MHz RMII */
		writel(0x0, 0x200f0070);	/* GPIO 1_3 PHYRST */
		writel(0xa, 0x200300cc);	/* RMII mode */
		printf("rmii\n");
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
	/* enable the reference clock */
	writel(readl(REG_BASE_SCTL + REG_SC_CTRL) | (1 << 16) | (1 << 18) |
	       (1 << 20), REG_BASE_SCTL + REG_SC_CTRL);

	writel(0, CFG_TIMERBASE + REG_TIMER_CONTROL);
	writel(~0, CFG_TIMERBASE + REG_TIMER_RELOAD);
	/* 32 bit, periodic, 256 divider */
	writel(CFG_TIMER_CTRL, CFG_TIMERBASE + REG_TIMER_CONTROL);

	return 0;
}
