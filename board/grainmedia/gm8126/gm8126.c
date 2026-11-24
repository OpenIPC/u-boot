// SPDX-License-Identifier: GPL-2.0+
/*
 * Grain Media GM8126 camera boards, after the GM8136 SDK's U-Boot 2013.01
 * (board/gm/gm8126/gm8126.c).
 */

#include <common.h>
#include <env.h>
#include <init.h>
#include <net.h>
#include <netdev.h>
#include <openipc.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/gm8126.h>
#include <linux/delay.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	gd->bd->bi_boot_params = GM_DDR_BASE + 0x100;
	return 0;
}

int misc_init_r(void)
{
	char buf[16];

	if (!env_get("osmem")) {
		sprintf(buf, "%luM", (ulong)(gd->ram_size / SZ_1M));
		env_set("osmem", buf);
	}
	return openipc_helper();
}

#ifdef CONFIG_FTMAC110
/*
 * The internal PHY, as the vendor's board_pmu_eth_init(): its power and
 * clock, 25 MHz from PLL1, the LED pins, and its reset.
 */
int board_eth_init(struct bd_info *bis)
{
	u32 mhz = gm_pll1_hz() / 1000000;

	setbits_le32(GM_PMU_MAC, BIT(10) | BIT(18));
	clrbits_le32(GM_PMU_GATE_AHB, BIT(8) | BIT(10));
	mdelay(60);
	writel(0x0012c100, GM_PMU_EPHY);
	clrsetbits_le32(GM_PMU_CLK_DIV1, 0xfc000000, (mhz / 25 - 1) << 26);
	clrsetbits_le32(GM_PMU_PINMUX3, 0xf0000000, 0x50000000);
	mdelay(10);
	writel(0x0012c101, GM_PMU_EPHY);	/* autonegotiate */

	return ftmac110_initialize(bis);
}
#endif
