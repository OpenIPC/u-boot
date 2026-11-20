// SPDX-License-Identifier: GPL-2.0+
/*
 * OpenIPC board file for AX620E cameras: the SD slot's card clock and
 * pads from board_init(), the OpenIPC boot layer from misc_init_r().
 * Other pads stay as the first stage and the board's own kernel set
 * them; UART0, the boot SPI flash and the Ethernet MAC's own 10/100 PHY
 * need none of their own here.
 */

#include <common.h>
#include <init.h>
#include <openipc.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <mach/ax620e.h>

#ifdef CONFIG_MMC_SDHCI_CADENCE
/* FLASH_SYS_GLB's set and clear aliases */
#define AX_FLASH_SET(reg)	(AX_FLASH_SYS_GLB + 0x4000 + (reg))
#define AX_FLASH_CLR(reg)	(AX_FLASH_SYS_GLB + 0x8000 + (reg))
#define AX_FLASH_CLK_MUX0	0x00
#define  AX_SD_CARD_SEL_MASK	(3 << 16)
#define  AX_SD_CARD_SEL_NPLL400	(3 << 16)
#define AX_FLASH_CLK_EB0	0x04
#define  AX_SD_CARD_EB		BIT(9)
#define AX_FLASH_CLK_DIV0	0x0c
#define  AX_SD_CARD_DIV_MASK	(0x3f << 20)
#define  AX_SD_CARD_DIV_2	(1 << 20)
#define  AX_SD_CARD_DIV_UP	BIT(26)

/* The SD pads' group (PIN_MUX_G9) */
#define AX_PINMUX_G9		0x104f1000UL
#define AX_PINMUX_G9_CTRL_SET	(AX_PINMUX_G9 + 0x04)
#define AX_PINMUX_G9_CTRL_CLR	(AX_PINMUX_G9 + 0x08)
/* Set: the SD pads at 1.8 V */
#define  AX_SD_PADS_1V8		GENMASK(8, 7)
#define AX_PINMUX_G9_VDET	(AX_PINMUX_G9 + 0x58)
#define  AX_SD_VDET_3V3		BIT(0)
#define AX_PAD_PULL_MASK	GENMASK(7, 6)
#define AX_PAD_PULL_UP		(2 << 6)

/*
 * As the vendor SDK's U-Boot does for the slot (drivers/mmc/
 * sdhci_ax620e.c, the controller at 0x104e0000): the card clock from
 * NPLL 400 MHz divided by 2, the 200 MHz the controller then divides;
 * pull-ups on CMD and DATA0-3; the pads' IO voltage to what the
 * detector reports (the slot's supply).
 */
static void ax620e_sd_init(void)
{
	static const u16 pads[] = { 0x30, 0x0c, 0x18, 0x3c, 0x48 };
	int i;

	writel(AX_SD_CARD_EB, AX_FLASH_CLR(AX_FLASH_CLK_EB0));
	udelay(2);
	writel(AX_SD_CARD_SEL_MASK, AX_FLASH_CLR(AX_FLASH_CLK_MUX0));
	writel(AX_SD_CARD_SEL_NPLL400, AX_FLASH_SET(AX_FLASH_CLK_MUX0));
	writel(AX_SD_CARD_EB, AX_FLASH_SET(AX_FLASH_CLK_EB0));
	writel(AX_SD_CARD_DIV_MASK, AX_FLASH_CLR(AX_FLASH_CLK_DIV0));
	writel(AX_SD_CARD_DIV_2, AX_FLASH_SET(AX_FLASH_CLK_DIV0));
	writel(AX_SD_CARD_DIV_UP, AX_FLASH_SET(AX_FLASH_CLK_DIV0));
	udelay(2);
	writel(AX_SD_CARD_DIV_UP, AX_FLASH_CLR(AX_FLASH_CLK_DIV0));

	for (i = 0; i < ARRAY_SIZE(pads); i++)
		clrsetbits_le32(AX_PINMUX_G9 + pads[i], AX_PAD_PULL_MASK,
				AX_PAD_PULL_UP);

	if (readl(AX_PINMUX_G9_VDET) & AX_SD_VDET_3V3)
		writel(AX_SD_PADS_1V8, AX_PINMUX_G9_CTRL_CLR);
	else
		writel(AX_SD_PADS_1V8, AX_PINMUX_G9_CTRL_SET);
}
#endif

int board_init(void)
{
#ifdef CONFIG_MMC_SDHCI_CADENCE
	ax620e_sd_init();
#endif
	return 0;
}

int misc_init_r(void)
{
	return openipc_helper();
}
