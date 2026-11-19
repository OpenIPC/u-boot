/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef __DDR_TABLES_H__
#define __DDR_TABLES_H__
struct ddr_reg_val {
	u32 reg;
	u32 val;
};

struct ddr_reg_field {
	u32 reg;
	u32 val;
	u8 width;
	u8 shift;
};

extern const struct ddr_reg_val ddr_ctl_regs[0x179];
extern const struct ddr_reg_val ddr_phy_regs[0x2ad];
extern const struct ddr_reg_val ddr_phy_board_regs[0x5d];
extern const struct ddr_reg_val ddr_ctl_timing_regs[0x17];
extern const struct ddr_reg_val ddr_pi_timing_regs[0xe];
extern const u32 ddr_pi_regs[0x559];
extern const struct ddr_reg_field ddr_phy_fields[0x16];
extern const struct ddr_reg_field ddr_phy_fields_cfg100[0x72];
extern const struct ddr_reg_field ddr_phy_fields_mode0[4];
extern const struct ddr_reg_field ddr_phy_fields_mode1[0xc];
extern const struct ddr_reg_field ddr_phy_fields_mode2[4];
extern const struct ddr_reg_field ddr_phy_fields_mode3[0x16];
/* board: drive strength code, DDR PLL setting, PHY setting, options */
extern const u32 ddr_cfg_dq_drv, ddr_cfg_dpll, ddr_cfg_phy, ddr_cfg;
#define DDR_CFG_MODE_MASK	0x300	/* 0x300: two passes, CA trained once */
#define DDR_CFG_EXTRA_FIELDS	0x100
#endif
