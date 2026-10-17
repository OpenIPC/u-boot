// SPDX-License-Identifier: GPL-2.0+
/*
 * XM510 PLL and DDR2 bring-up, from the vendor U-Boot 2014.04
 * (arch/arm/cpu/armv7/xm/s_init.c and board/xm/xm510/xm510.c dram_init()).
 *
 * Called from lowlevel_init on the SRAM stack, running from flash: no
 * global data and no writes to .data or .bss.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/xm.h>

/* Synopsys uPCTL and its PHY, at XM_DDRC_BASE */
#define DDRC(off)		(XM_DDRC_BASE + (off))
#define SCTL			DDRC(0x004)
#define STAT			DDRC(0x008)
#define MCMD			DDRC(0x040)
#define POWCTL			DDRC(0x044)
#define POWSTAT			DDRC(0x048)
#define CMDTSTATEN		DDRC(0x050)
#define MCFG			DDRC(0x080)
#define TOGCNT1U		DDRC(0x0C0)
#define TINT			DDRC(0x0C4)
#define TRSTH			DDRC(0x0C8)
#define TOGCNT100N		DDRC(0x0CC)
#define TREFI			DDRC(0x0D0)
#define TMRD			DDRC(0x0D4)
#define TRFC			DDRC(0x0D8)
#define TRP			DDRC(0x0DC)
#define TAL			DDRC(0x0E4)
#define TCL			DDRC(0x0E8)
#define TCWL			DDRC(0x0EC)
#define TRCD			DDRC(0x0F8)
#define TEXSR			DDRC(0x10C)
#define TXPDLL			DDRC(0x114)
#define DFIODTCFG		DDRC(0x244)
#define DFIODTCFG1		DDRC(0x248)
#define DFITPHYWRDATA		DDRC(0x250)
#define DFITPHYWRLAT		DDRC(0x254)
#define DFITRDDATAEN		DDRC(0x260)
#define DFITPHYRDLAT		DDRC(0x264)
#define DCFG			DDRC(0x484)
#define SOFT_RST_N		DDRC(0x1000)
#define PZQ_RST_N		DDRC(0x1004)
#define PZQ_INIT_UPD		DDRC(0x1008)
#define DLL_RST_N		DDRC(0x100C)
#define DLL_INIT_UPD		DDRC(0x1010)
#define PHY_ITR_MODE		DDRC(0x1014)
#define PHY_ITR_REQ		DDRC(0x1018)
#define ODT_ACK_DONE		DDRC(0x1038)
#define PHY_MODE		DDRC(0x2024)
#define DLL_MODE2		DDRC(0x202C)
#define DLL_MODE3		DDRC(0x2040)
#define TRAINING_MODE_SEL	DDRC(0x2050)
#define WRITE_TRAINING1		DDRC(0x2054)
#define WRITE_TRAINING2		DDRC(0x2058)
#define GATE_TRAINING1		DDRC(0x2074)
#define GATE_TRAINING2		DDRC(0x2078)

#ifdef CONFIG_XM_DDR_64M
#define DDR_DCFG		0x10E	/* 512 Mbit */
#define DDR_GATE_TRAINING	0x0015030
#define DDR_WRITE_TRAINING	0x04a030
#else
#define DDR_DCFG		0x10A	/* 256 Mbit */
#define DDR_GATE_TRAINING	0x0014030
#define DDR_WRITE_TRAINING	0x050030
#endif

static void wait_eq(ulong reg, u32 val)
{
	while (readl(reg) != val)
		;
}

/* A mode register command: issue it and wait for the start bit to clear */
static void mcmd(u32 cmd)
{
	writel(cmd, MCMD);
	while (readl(MCMD) & 0x80000000)
		;
}

/* Run one PHY training step and wait for its acknowledge */
static void phy_itr(u32 mode)
{
	writel(mode, PHY_ITR_MODE);
	xm_early_udelay(1);
	writel(0x01, PHY_ITR_REQ);
	wait_eq(ODT_ACK_DONE, 1);
	writel(0x00, PHY_ITR_REQ);
	writel(0x00, PHY_ITR_MODE);
}

void xm_pll_init(void)
{
	/* 24 MHz crystal */
	writel(1, XM_SC_LOCK);

	writel(0x10180002, XM_PLLA_CTRL);
	wait_eq(XM_PLLA_STAT, 1);

	writel(0x10180000, XM_PLLC_CTRL);
	wait_eq(XM_PLLC_STAT, 1);

	/* Ethernet, SDIO0 and SDIO1 at 50 MHz */
#ifdef CONFIG_XM_DDR_64M
	writel(0x00000C0C, XM_PLLD2_CTRL);
	writel(0x1018000C, XM_PLLD1_CTRL);
#else
	writel(0x00000303, XM_PLLD2_CTRL);
	writel(0x11180103, XM_PLLD1_CTRL);
#endif
	wait_eq(XM_PLLD_STAT, 1);

	writel(XM_SC_ST_NORMAL, XM_SC_CTRL);
	writel(0, XM_SC_LOCK);
}

void xm_ddr_init(void)
{
	writel(0xA9B1000B, XM_DDR_CTRL_SOFT_RST);
	xm_early_udelay(50);

	writel(DDR_DCFG, DCFG);
	writel(0x12C, TOGCNT1U);	/* 300 MHz */
	writel(0xC8, TINT);
	writel(0x00, TRSTH);
	writel(0x1E, TOGCNT100N);	/* 300 MHz */
	writel(0x8000004E, TREFI);
	writel(0x01, MCFG);
	writel(0x01, CMDTSTATEN);

	/* PHY: read ODT off */
	writel(0x00031208, PHY_MODE);
	writel(0x1010, DLL_MODE2);
	writel(0x18, DLL_MODE3);
	writel(0x01, PZQ_RST_N);
	xm_early_udelay(55);
	writel(0x01, PZQ_INIT_UPD);
	writel(0x00, PZQ_INIT_UPD);
	writel(0x01, DLL_RST_N);
	xm_early_udelay(89);
	writel(0x01, DLL_INIT_UPD);
	writel(0x00, DLL_INIT_UPD);
	writel(0x01, SOFT_RST_N);
	xm_early_udelay(1);

	/* Read data eye (gate training) */
	writel(0x0001f, TRAINING_MODE_SEL);
	writel(DDR_GATE_TRAINING, GATE_TRAINING1);
	writel(DDR_GATE_TRAINING, GATE_TRAINING2);
	phy_itr(0x03);

	/* Write data eye */
	writel(0x0001f, TRAINING_MODE_SEL);
	writel(DDR_WRITE_TRAINING, WRITE_TRAINING1);
	writel(DDR_WRITE_TRAINING, WRITE_TRAINING2);
	xm_early_udelay(1);
	phy_itr(0x04);
	xm_early_udelay(1);

	writel(0x01, POWCTL);
	wait_eq(POWSTAT, 1);
	writel(0x02, TMRD);
	writel(0x10, TRFC);
	writel(0x05, TRP);
	writel(0x00, TAL);
	writel(0x05, TCL);
	writel(0x04, TCWL);
	writel(0x05, TRCD);
	writel(0xc8, TEXSR);
	writel(0x03, TXPDLL);
	writel(0x00, DFIODTCFG);	/* write ODT off */
	writel(0x06060201, DFIODTCFG1);

	/* DRAM init: mode registers, BL=8, ODT off and reduced drive */
	mcmd(0x87100000);
	mcmd(0x87104001);
	mcmd(0x80140003);
	mcmd(0x80160003);
	mcmd(0x80120023);
	mcmd(0x8810b533);
	mcmd(0x80104001);
	mcmd(0x80100002);
	mcmd(0x80100002);
	mcmd(0x8010a533);
	mcmd(0x80123823);
	mcmd(0x80120023);
	mcmd(0x8000000a);

	writel(0x01, SCTL);
	wait_eq(STAT, 1);
	writel(0x01, DFITPHYWRDATA);
	writel(0x03, DFITPHYWRLAT);
	writel(0x02, DFITRDDATAEN);
	writel(0x0f, DFITPHYRDLAT);
	writel(0x02, SCTL);
	wait_eq(STAT, 3);
	xm_early_udelay(15);
}
