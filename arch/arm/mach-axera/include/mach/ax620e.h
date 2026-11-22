/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Axera AX620E: the registers U-Boot touches, from the vendor SDK's U-Boot
 * 2020.04 (arch/arm/include/asm/arch-axera/ax620e.h, boot_mode.h).
 */

#ifndef __MACH_AX620E_H
#define __MACH_AX620E_H

#define AX_DDR_BASE			0x40000000

/* CoreSight system counter: CNTCR, CNTFID0 */
#define AX_SYS_COUNTER_BASE		0x01b30000
#define AX_SYS_COUNTER_CNTCR		(AX_SYS_COUNTER_BASE + 0x00)
#define AX_SYS_COUNTER_CNTCV_LO		(AX_SYS_COUNTER_BASE + 0x08)
#define AX_SYS_COUNTER_CNTCV_HI		(AX_SYS_COUNTER_BASE + 0x0c)
#define AX_SYS_COUNTER_CNTFID0		(AX_SYS_COUNTER_BASE + 0x20)
#define AX_SYS_COUNTER_FREQ		24000000

/* COMM_SYS_GLB: software chip reset, abort (watchdog/thermal) control */
#define AX_COMM_SYS_GLB			0x02340000
#define AX_COMM_ABORT_CFG		(AX_COMM_SYS_GLB + 0xa8)
#define AX_COMM_ABORT_STATUS		(AX_COMM_SYS_GLB + 0xb4)
#define AX_CHIP_RST_SW			BIT(0)

/* CHIPMODE_GLB backup word: the boot ROM's download/kernel-fail flags */
#define AX_CHIPMODE_GLB_BACKUP0		0x02390024
#define AX_BOOT_KERNEL_FAIL		BIT(7)
#define AX_BOOT_DOWNLOAD		BIT(8)

/* Watchdog 0: enable, timeout (in 65536 clocks) and its load strobe */
#define AX_WDT0_BASE			0x04840000
#define AX_WDT_EN			0x00
#define AX_WDT_TIMEOUT			0x0c
#define AX_WDT_TIMEOUT_LOAD		0x18
/* PERI_SYS_GLB: watchdog 0's clock to the 24 MHz crystal */
#define AX_PERI_SYS_GLB_CLK_MUX0_SET	0x048700a8
#define AX_WDT0_CLK_24M			BIT(19)

/* CPU_SYS_GLB: clk_h_ssi (the boot SPI controller's clock), the eMMC's
 * card clock */
#define AX_CPU_SYS_GLB			0x01900000
#define AX_CPU_SYS_GLB_CLK_MUX0		(AX_CPU_SYS_GLB + 0x0)
#define AX_CPU_SYS_GLB_CLK_MUX0_SET	(AX_CPU_SYS_GLB + 0x1000)
#define AX_CPU_SYS_GLB_CLK_MUX0_CLR	(AX_CPU_SYS_GLB + 0x2000)
#define AX_CPU_SYS_GLB_CLK_EB0_SET	(AX_CPU_SYS_GLB + 0x1004)
#define AX_CPU_SYS_GLB_CLK_EB0_CLR	(AX_CPU_SYS_GLB + 0x2004)
#define AX_CPU_SYS_GLB_CLK_DIV0_SET	(AX_CPU_SYS_GLB + 0x100c)
#define AX_CPU_SYS_GLB_CLK_DIV0_CLR	(AX_CPU_SYS_GLB + 0x200c)

/* FLASH_SYS_GLB: the Ethernet MAC's clocks, resets and its 10/100 PHY */
#define AX_FLASH_SYS_GLB		0x10030000

/*
 * What the first stage leaves in IRAM0 from the eFuses: misc_info at
 * 0x740. U-Boot reads the Ethernet PHY's bandgap trim from it.
 */
#define AX_MISC_INFO_ADDR		0x740

/* misc_info's chip_type (eFuse) and phy_board_id (ADC) on the AX630C
 * boards (the SDK's boot_mode.h) */
#define AX_CHIP_AX630C			4
#define AX_CHIP_AX631			5
#define AX_BOARD_MAIXCAM2_0_5G		2
#define AX_BOARD_MAIXCAM2_1G		5
#define AX_BOARD_MAIXCAM2_2G		10
#define AX_BOARD_MAIXCAM2_4G		14

struct ax_misc_info {
	u32 pub_key_hash[8];
	u32 aes_key[8];
	u32 board_id;
	u32 chip_type;
	u32 uid_l;
	u32 uid_h;
	u32 thm_vref;
	u32 thm_temp;
	u16 bgs;
	u16 trim;
	u32 phy_board_id;
};

#endif
