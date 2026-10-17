// SPDX-License-Identifier: GPL-2.0+
/*
 * XM530 / XM550 PLL and DDR bring-up, from the vendor U-Boot 2014.04
 * (arch/arm/cpu/armv7/xm/s_init.c and board/xm/xm530/xm530.c): DDR2-1066
 * on the XM530, DDR3 with a 1600M PLL C on the XM550 (CONFIG_XM_DDR3, the
 * vendor's CONFIG_128MDDR3).
 *
 * Called from lowlevel_init on the SRAM stack, running from flash: no
 * global data and no writes to .data or .bss.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/xm.h>

#define DDRC(off)		(XM_DDRC_BASE + (off))

static void wait_locked(ulong reg)
{
	while (!(readl(reg) & XM_PLL_LOCKED))
		;
}

static void wait_eq(ulong reg, u32 val)
{
	while (readl(reg) != val)
		;
}

void xm_pll_init(void)
{
	/* 24 MHz crystal */
	writel(1, XM_SC_LOCK);

	writel(0x00110101, XM_CPUCLK_CTRL);
	writel(0x17, XM_GMACCLK_CTRL);
	writel(0x17, XM_SDIO0CLK_CTRL);
	writel(0x2f, XM_SDIO1CLK_CTRL);
	writel(0x1005, XM_GSDBUS_CTRL);
	writel(0x12046404, XM_PLLA_CTRL);	/* 1200M */
	wait_locked(XM_PLLA_CTRL);

	writel(0x31, XM_SENSORCLK_CTRL);	/* 24M */
	writel(0x1030103, XM_VENCCLK_CTRL);	/* 300M */
	writel(0x12046404, XM_PLLB_CTRL);	/* 1200M */
	wait_locked(XM_PLLB_CTRL);

	writel(0x0, XM_DDRCLK_CTRL);
#ifdef CONFIG_XM_DDR3
	writel(0x12046403, XM_PLLC_CTRL);	/* 1600M */
#else
	writel(0x12048546, XM_PLLC_CTRL);	/* 1066M */
#endif
	wait_locked(XM_PLLC_CTRL);

	writel(0x9, XM_FLASHCLK_CTRL);
	writel(0xF900, XM_IISCLK_CTRL);
	writel(0x12042003, XM_PLLD_CTRL);	/* 512M */
	wait_locked(XM_PLLD_CTRL);

	writel(0x2, XM_USBCLK_SEL);
	writel(0x1105, XM_GSDBUS_CTRL);
	writel(0x109, XM_FLASHCLK_CTRL);
	writel(XM_SC_ST_NORMAL, XM_SC_CTRL);
	writel(0, XM_SC_LOCK);
}

/* Start the controller: release its resets, then PHY and controller init */
static void ddr_start(void)
{
	writel(0x02, XM_DDR_RSTN);	/* {axibus, phyclkdiv, sys}_rstn */
	xm_early_udelay(1);
	writel(0x03, XM_DDR_RSTN);
	while ((readl(XM_DDR_RSTN) & 0x03) != 0x03)
		;
	writel(0x00, DDRC(0x0C8));	/* PHY init */
	wait_eq(DDRC(0x0F4), 1);
	xm_early_udelay(200);
	writel(0x01, DDRC(0x0C8));	/* controller enable */
	wait_eq(DDRC(0x0F8), 1);
	xm_early_udelay(1);
}

/* RX calibration, with auto-refresh off while it runs */
static void ddr_rx_calibrate(void)
{
	writel(0x01, DDRC(0x0C4));
	writel(0xa1, DDRC(0x1008));
	while (readl(DDRC(0x10B8)) != readl(DDRC(0x10A8)))
		;
	writel(0xa0, DDRC(0x1008));
	writel(0x21, DDRC(0x0C4));
}

static void ddr_rx_dll(void)
{
	writel(0x04, DDRC(0x11C4));	/* rx dll dqs0 */
	writel(0x34, DDRC(0x11D8));	/* rx dll dqs1 */
	writel(0x08, DDRC(0x1448));	/* rx dqs0 deskew */
	writel(0x08, DDRC(0x149C));	/* rx dqs1 deskew */
}

#ifdef CONFIG_XM_DDR3
/* 1 Gbit DDR3 */
void xm_ddr_init(void)
{
	writel(0x01, XM_SYS_SOFT_RSTEN);	/* unlock reset */
	writel(0x00, XM_DDR_RSTN);
	xm_early_udelay(1);
	writel(0x21, DDRC(0x0C4));	/* auto refresh on */
	writel(0xDA, DDRC(0x018));	/* row[7:4], col[3:0] */
	writel(0x03, DDRC(0x01C));	/* bank size */
	writel(0x04, DDRC(0x0C0));	/* DDR3 */
	writel(0x10, DDRC(0x1004));	/* PHY: BL8, DDR3 */
	/* DDR3-800 timings, from 0x280 on */
	writel(0xC0, DDRC(0x280));	/* TREF */
	writel(0x0A, DDRC(0x284));	/* TRCD */
	writel(0x15, DDRC(0x288));	/* TRC */
	writel(0x1C, DDRC(0x28C));	/* TRAS */
	writel(0x0C, DDRC(0x290));	/* TRP */
	writel(0x58, DDRC(0x294));	/* TRFC */
	writel(0x07, DDRC(0x298));	/* TRTP */
	writel(0x0C, DDRC(0x29C));	/* TWR */
	writel(0x08, DDRC(0x2A0));	/* TWTR */
	writel(0x1C, DDRC(0x2A4));	/* TWRPDEN */
	writel(0x1B, DDRC(0x2A8));	/* TRDPDEN */
	writel(0x70, DDRC(0x2B8));	/* MR0: CL */
	writel(0x04, DDRC(0x2C0));	/* MR1: AL, RTT_NOM, drive */
	writel(0x18, DDRC(0x2C8));	/* MR2: CWL */
	writel(0x18, DDRC(0x2B4));	/* ODT on, TRRD */
	writel(0x0B, DDRC(0x1014));	/* PHY_CL */
	writel(0x00, DDRC(0x1018));	/* PHY_AL */
	writel(0x08, DDRC(0x101C));	/* PHY_CWL */
	writel(0x03, DDRC(0x10A8));	/* PHY DQ width: 16 bit */
	writel(0x10, DDRC(0x000));	/* DQ16 */
	writel(0x44, DDRC(0x020));	/* outstanding, channels 1/2 */
	writel(0x44, DDRC(0x024));	/* outstanding, channels 3/4 */
	writel(0x13, DDRC(0x040));	/* channel 1/2 priority */
	writel(0x02, DDRC(0x044));	/* channel 3/4 priority */
	writel(0x00, DDRC(0x060));	/* group 1 */
	writel(0x00, DDRC(0x068));	/* group 2 */
	writel(0xA8, DDRC(0x0A0));	/* exclusive access */
	ddr_start();
	ddr_rx_calibrate();
	ddr_rx_dll();
	writel(0x07, XM_DDR_RSTN);
	xm_early_udelay(1);
	writel(0x00, XM_SYS_SOFT_RSTEN);	/* lock reset */
}
#else
/* 512 Mbit DDR2-1066 */
void xm_ddr_init(void)
{
	int i;

	writel(0x01, XM_SYS_SOFT_RSTEN);	/* unlock reset */
	writel(0x00, XM_DDR_RSTN);
	xm_early_udelay(1);
	writel(0x02, DDRC(0x0C0));	/* DDR2 */
	writel(0xDA, DDRC(0x018));	/* row[7:4], col[3:0] */
	writel(0x02, DDRC(0x01C));	/* bank size, 512 Mbit */
	/* DDR2-1066 timings, from 0x180 on */
	writel(0x20, DDRC(0x180));	/* TREF */
	writel(0x06, DDRC(0x184));	/* TRCD */
	writel(0x11, DDRC(0x188));	/* TRC */
	writel(0x12, DDRC(0x18C));	/* TRAS */
	writel(0x07, DDRC(0x190));	/* TRP */
	writel(0x20, DDRC(0x194));	/* TRFC */
	writel(0x04, DDRC(0x198));	/* TRTP */
	writel(0x08, DDRC(0x19C));	/* TWR */
	writel(0x04, DDRC(0x1A0));	/* TWTR */
	writel(0x73, DDRC(0x1B8));	/* MR: CL, BL */
	writel(0x0E, DDRC(0x1BC));	/* MR: WR */
	writel(0x04, DDRC(0x1C0));	/* EMR: AL */
	writel(0x07, DDRC(0x1014));	/* PHY_CL */
	writel(0x00, DDRC(0x1018));	/* PHY_AL */
	writel(0x06, DDRC(0x101C));	/* PHY_CWL */
	writel(0x09, DDRC(0x1004));	/* PHY: BL8, DDR2 */
	writel(0x03, DDRC(0x10A8));	/* PHY DQ width: 16 bit */
	writel(0x10, DDRC(0x000));	/* DQ16 */
	writel(0x88, DDRC(0x020));	/* outstanding, channels 1/2 */
	writel(0x08, DDRC(0x024));	/* outstanding, channels 3/4 */
	writel(0x13, DDRC(0x040));	/* channel 1/2 priority */
	writel(0x02, DDRC(0x044));	/* channel 3/4 priority */
	writel(0x00, DDRC(0x060));	/* group 1 */
	writel(0xA2, DDRC(0x0A0));	/* exclusive access */
	ddr_start();
	ddr_rx_dll();

	/* Calibrate until a word reads back, three tries, then reset */
	for (i = 0; i < 3; i++) {
		ddr_rx_calibrate();
		writel(0x07, XM_DDR_RSTN);
		xm_early_udelay(1);
		writel(0x00, XM_SYS_SOFT_RSTEN);	/* lock reset */
		writel(0xFFFFFFFF, XM_DDR_BASE);
		if (readl(XM_DDR_BASE) == 0xFFFFFFFF)
			return;
	}
	xm_soc_reset();
}
#endif
