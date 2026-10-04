/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
 */
#include "platform.h"
#include "share_drivers.h"
#include "common.h"

typedef union {
	struct {
		u32 reserved0 : 4;
		u32 cfgsys_cksel : 3;
		u32 reserved1 : 1;
		u32 dataaxi_cksel : 3;
		u32 reserved2 : 1;
		u32 tcxo_div_cken : 1;
		u32 reserved3 : 19;
	} bits;
	u32 u32;
} soc_cksel;

typedef union {
	struct {
		u32 reserved0 : 4;
		u32 arm_dbg0_srst_req : 1;
		u32 reserved1 : 5;
		u32 arm_dbg1_srst_req : 1;
		u32 reserved2 : 1;
		u32 l2_srst_req : 1;
		u32 reserved3 : 1;
		u32 socdbg_srst_req : 1;
		u32 reserved4 : 1;
		u32 cs_srst_req : 1;
		u32 reserved5 : 1;
		u32 a7_cksel : 3;
	} bits;
	u32 u32;
} cpu_cksel;

typedef union {
	struct {
		u32 sdio_mmc_cken : 1;
		u32 sdio_mmc_hclk_cken : 1;
		u32 reserved0 : 14;
		u32 sdio_mmc_srst_req : 1;
		u32 sdio_mmc_rx_srst_req : 1;
		u32 sdio_mmc_tx_srst_req : 1;
		u32 reserved1 : 5;
		u32 sdio_cksel : 3;
	} bits;
	u32 u32;
} sdio_cksel;

typedef union {
	struct {
		u32 fmc_srst_req : 1;
		u32 reserved0 : 3;
		u32 fmc_cken : 1;
		u32 reserved1 : 7;
		u32 fmc_cksel : 3;
	} bits;
	u32 u32;
} fmc_cksel;

typedef union {
	struct {
		u32 reserved0 : 4;
		u32 cfgsys_sc_seled : 3;
		u32 reserved1 : 1;
		u32 dataaxi_sc_seled : 3;
	} bits;
	u32 u32;
} soc_sc_seled;

typedef union {
	struct {
		u32 a7_sc_seled : 3;
	} bits;
	u32 u32;
} cpu_sc_seled;

typedef union {
	struct {
		u32 reserved0 : 8;
		u32 fmc_sc_seled : 3;
	} bits;
	u32 u32;
} fmc_sc_seled;

typedef union {
	struct {
		u32 apll_lock : 1;
		u32 reserved : 3;
		u32 apll_lock_final : 1;
	} bits;
	u32 u32;
} apll_lock_status;

#define PERI_CRG_PLL1 0x4
#define PERI_CRG_PLL0 0x0
#define PERI_CRG14 0x38
#define PERI_CRG27 0x6C
#define PERI_CRG2048 0x2000
#define PERI_CRG2064 0x2040
#define PERI_CRG3440 0x35C0
#define PERI_CRG3504 0x36C0
#define PERI_CRG4048 0x3F40
#define PERI_CRG2056 0x2020
#define PERI_CRG2065 0x2044
#define PERI_CRG4049 0x3F44

#define APLL_PD 0x101027
#define APLL_PD_ON 0x1027

#define FOUTVCO2X 0x0

#define CPU_792M 0x140000
#define CFG_SYS_CLK_24M 0x1100

#define PLL0_APLL_HIGH_FREQ 0x11955555
#define PLL1_APLL_HIGH_FREQ 0x101027

#define PLL_LOCK_DELAY 20

#define CPU_CLK_792M 0x5
#define SYS_CLK_24M 0x100

void call_reset(void)
{
	sdio_cksel sdio0_cksel, sdio1_cksel;

	// CPU clksel 792M
	reg_set(REG_BASE_CRG + PERI_CRG2064, CPU_792M);

	// clk_cfg_sys clksel 24M
	reg_set(REG_BASE_CRG + PERI_CRG2048, CFG_SYS_CLK_24M);

	do {
		u32 cpu_clk = reg_get(REG_BASE_CRG + PERI_CRG2065);
		u32 sys_clk = reg_get(REG_BASE_CRG + PERI_CRG2056);
		if (cpu_clk == CPU_CLK_792M && sys_clk == SYS_CLK_24M)
			break;
	} while (1);

	sdio0_cksel.u32 = reg_get(REG_BASE_CRG + PERI_CRG3440);
	sdio1_cksel.u32 = reg_get(REG_BASE_CRG + PERI_CRG3504);

	sdio0_cksel.bits.sdio_mmc_cken = 0x0;
	sdio1_cksel.bits.sdio_mmc_cken = 0x0;

	reg_set(REG_BASE_CRG + PERI_CRG3440, sdio0_cksel.u32);
	reg_set(REG_BASE_CRG + PERI_CRG3504, sdio1_cksel.u32);

	reg_set(REG_BASE_CRG + PERI_CRG_PLL1, APLL_PD);
	reg_set(REG_BASE_CRG + PERI_CRG27, FOUTVCO2X);

	// apll high freq
	reg_set(REG_BASE_CRG + PERI_CRG_PLL0, PLL0_APLL_HIGH_FREQ);
	reg_set(REG_BASE_CRG + PERI_CRG_PLL1, PLL1_APLL_HIGH_FREQ);

	reg_set(REG_BASE_CRG + PERI_CRG_PLL1, APLL_PD_ON);

	// wait pll lock
	mdelay(PLL_LOCK_DELAY);

	do {
		apll_lock_status apll_lock_status;

		apll_lock_status.u32 = reg_get(REG_BASE_CRG + PERI_CRG14);
		if (apll_lock_status.bits.apll_lock_final == 1)
			break;
	} while (1);

	reg_set(REG_SYSCTRL_BASE + REG_SC_SYSRES, 0x1);
	reg_set(REG_SYSCTRL_BASE + REG_SC_SYSRES, 0x1);
	reg_set(REG_SYSCTRL_BASE + REG_SC_SYSRES, 0x1);
	asm("b .");
}
