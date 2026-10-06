/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#ifndef _MMC_INTERNAL_H_
#define _MMC_INTERNAL_H_

#include <mmc.h>

typedef enum {
	BUS_WIDTH_1BIT = 1,
	BUS_WIDTH_4BIT = 4,
	BUS_WIDTH_8BIT = 8
} emmc_bus_width_u;

typedef enum {
	BUS_UHS = 1,
	BUS_HS200 = 8
} emmc_uhs_mode_u;

#if defined(CONFIG_TARGET_HI3519DV500_FAMILY)
#define EMMC_PARAM_GEN0 (0x11120020)
#define EMMC_PARAM_GEN1 (0x11120024)
#elif defined(CONFIG_TARGET_HI3516CV610_FAMILY)
#define EMMC_PARAM_GEN0 (0x11120138)
#define EMMC_PARAM_GEN1 (0x1112014c)
#endif

#define IO_DRV_BIT 4
#define IO_SL_BIT  10
#define IO_DRV_MASK (0xF << IO_DRV_BIT)

#define MMC_CMD_SLEEP_AWAKE         5
#define MMC_SAWAKE_SHIFT_BIT        15
#define MMC_RCA_SHIFT_BIT           16

/* emmc parameters type */
typedef union {
	struct {
		u32 emmc_clk_ph_sel          : 5; // [4:0]
		u32 emmc_clk_sel             : 3; // [7:5]
		u32 emmc_sw_clk_ph           : 8; // [15:8]
		u32 emmc_cmd_drv             : 4; // [19:16]
		u32 emmc_cmd_sl              : 1; // [20]
		u32 emmc_clk_drv             : 4; // [24:21]
		u32 emmc_clk_sl              : 1; // [25]
		u32 emmc_data_drv            : 4; // [29:26]
		u32 emmc_data_sl             : 1; // [30]
		u32 reserved_0               : 1; // [31]
	} bits;
	u32 u32;
} emmc_param_gen0_u;

/* emmc parameters type */
typedef union {
	struct {
		u32 emmc_uhs_mode_sel        : 3;  // [2:0]
		u32 emmc_enh_strobe          : 1;  // [3]
		u32 emmc_bus_width           : 2;  // [5:4]
		u32 emmc_hcs_mode            : 1;  // [6]
		u32 emmc_spec_ver            : 4;  // [10:7]
		u32 emmc_chip_size           : 11; // [21:11]
		u32 emmc_qboot_mode          : 2;  // [23:22]
		u32 emmc_cur_mode            : 8;  // [31:24]
	} bits;
	u32 u32;
} emmc_param_gen1_u;

typedef enum {
	INIT_MODE = 0x0,
	SLEEP_MODE = 0x5A,
	BOOT_MODE,
	DS_MODE,
	TRAN_MODE
} emmc_mode_u;

typedef enum {
	QUICK_BOOT_DIS = 0x0,
	QUICK_BOOT_WARM,
	QUICK_BOOT_COLD,
	QUICK_BOOT_MAX
} emmc_qboot_u;

u32 mmc_get_clk(void);
u32 mmc_get_clk_phase(void);
void mmc_get_io_info(void);

emmc_qboot_u mmc_is_fast_boot(void);
void mmc_enable_fast_boot(emmc_qboot_u mode);
void mmc_disable_fast_boot(void);
void mmc_set_cur_mode(emmc_mode_u mode);
emmc_mode_u mmc_get_cur_mode(void);
void printf_mmc(int dev_num);

int mmc_restore(struct mmc *mmc);
int mmc_save_parameters(struct mmc *mmc);

int mmc_select_mode(struct mmc *mmc, enum bus_mode mode);
int mmc_startup_v4(struct mmc *mmc);
int mmc_get_capabilities(struct mmc *mmc);
int mmc_select_mode_and_width(struct mmc *mmc, uint card_caps);

#endif