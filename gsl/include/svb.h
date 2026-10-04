/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
 */
#ifndef __SVB_H__
#define __SVB_H__
#include "platform.h"

#define HPM_NUM        				 	6
#define CYCLE_NUM						8
#define SVB_VER_REG						0x11020168
#define SVB_VER_VAL						0x00090000 // SVB VER

#define SYSCTRL_BASE_REG				0x11020000
#define UBOOT_REG_TSENSOR_CTRL			0x1102A000

#define TSENSOR0_CTRL0					0x0
#define TSENSOR_CTRL0_CFG0				0xc08ffc00
#define TSENSOR0_CTRL1					0x4
#define TSENSOR_CTRL1_CFG0				0x0
#define TSENSOR_STATUS0					0x8

#define HPM_BASE_ADDR					0x1102B000
#define HPM_CTRL_VALUE 					0x60080003
#define HPM_LIMIT						0x3ff
#define CORE_HPM_CTRL0					0x30
#define CORE_HPM_CTRL1					0x34
#define HPM_CORE_SVT40_REG0				0xb058
#define HPM_CORE_SVT40_REG1				0xb05c

#define CRG_BASE_ADDR					0x11010000
#define MISC_BASE_ADDR					0x11020000

#define SVB_VERSION_ADDR  				0X0134
#define SYSCTRL_REG						0x11020000
#define HPM_STORAGE_REG 				0x340
#define CORE_V_STORAGE_REG 				0x344
#define CORE_TRAINING_V_STORAGE_REG 	0x348
#define SVB_PWM_CRG_STORAGE_REG		 	0x34c
#define DYN_COMP_FALG_STORAGE_REG 		0x354
#define PWM_CORE_SVT40_VOL_REG	    	(SYSCTRL_REG + 0x9000)

#define HPM_CLK_CFG0					0x11
#define HPM_CLK_CFG1					0x10
#define TSENSOR_CLK_CFG0				0x11
#define SVB_PWM_CRG						0x4580
#define HPM_CTRL_CPU_SVT40_CLK_REG		0x11014988
#define HPM_8T_CPU_SVT40_CLK_REG		0x11014984
#define TSENSOR_CTRL_CLK_REG		    0x11014740

#define OTP_BASE_REG					0x101E0000
#define OTP_VERSION_ID_REG				0x010c
#define OTP_KGD_ID_REG					0x20
#define OTP_HPM_OFFSET  				0x0128
#define OTP_DELTA_CORE_OFFSET 			0x0118
#define OTP_DELTA_CPU_NPU_OFFSET 		0x0124
#define OTP_10B_VERSION_ID				0x240101
#define OTP_20B_VERSION_ID				0x240103
#define OTP_20S_VERSION_ID				0x240104
#define OTP_20G_VERSION_ID				0x240105
#define OTP_00B_VERSION_ID				0x240106
#define OTP_00S_VERSION_ID				0x240107
#define OTP_00G_VERSION_ID				0x240108
#define OTP_608_VERSION_ID				0x240180

#define CORE_CURVE_VOLT_MAX10			950
#define CORE_CURVE_VOLT_MAX10_ESMT		976
#define CORE_CURVE_VOLT_MIN10			844
#define CORE_CURVE_B10					12545914
#define CORE_CURVE_A10					13257

#define CORE_CURVE_VOLT_MAX20	        990
#define CORE_CURVE_VOLT_MIN20	        838
#define CORE_CURVE_B20			        13356000
#define CORE_CURVE_A20			        15549

#define CORE_CURVE_VOLT_MAX00			970
#define CORE_CURVE_VOLT_MIN00			855
#define CORE_CURVE_B00					135270000
#define CORE_CURVE_A00					153100

#define CORE_VOLT_MAX0					1080
#define CORE_VOLT_MIN0					643

#define CORE_SVB_MAX0					1035
#define CORE_SVB_MIN0					810

#define SVB_TRAINING_DELTA_VOLT_10		40
#define SVB_TRAINING_DELTA_VOLT_20		40
#define SVB_TRAINING_DELTA_VOLT_00		0
#define CORE_HPM_TEM_COM_BOUNDED    	260

#define TEMPERATURE_115             	115
#define TEMPERATURE_110             	110
#define TEMPERATURE_100             	100
#define TEMPERATURE_85              	85
#define TEMPERATURE_50              	50
#define TEMPERATURE_0                	0
#define TEMPERATURE_NEG20           	(-20)

#define WAITING_10_US					10
#define WAITING_20_US					20
#define WAITING_200_US					200
#define WAITING_10_MS					10
#define WAITING_6_MS					6
#define PA_IS_NEGATIVE					1
#define NEG_1							(-1)

#define LOW_TEMPERATURE_V				930
#define FORMULA_MULTIPLES           	10000

#define TEMPERATURE_A					127
#define TEMPERATURE_B					165
#define TEMPERATURE_C					784
#define TEMPERATURE_D					40

#define	DUTY_ONE						1
#define	DUTY_TWO						2

#define	DELTA_FORMULA_MULTIPLES			10000

#define	CORE_HPM_BOUND_20				222
#define	CORE_HPM_BOUND_10				230
#define	CORE_HPM_BOUND_10_ESMT			210

enum svb_version_id {
	SVB10_ID = 1,
	SVB20_ID,
	SVB00_ID,
};

enum hpm_record {
	HPM_RECORD0 = 0,
	HPM_RECORD1,
	HPM_RECORD2,
	HPM_RECORD3,
	HPM_RECORD_BUTT
};

enum temperature_zone {
	TEMPERATURE_ZONE_110 = 0,
	TEMPERATURE_ZONE_100,
	TEMPERATURE_ZONE_85,
	TEMPERATURE_ZONE_50,
	TEMPERATURE_ZONE_0,
	TEMPERATURE_ZONE_NEG20,
	TEMPERATURE_ZONE_BUTT
};

enum product_type {
	SVB_NONE = 0,
	SVB_10,
	SVB_20,
	SVB_00,
	SVB_608
};

enum kgd_type {
	KGD_A = 0,
	KGD_B,
	KGD_C = 3
};

enum ddr_type {
	LPDDR3 = 3,
	DDR3 = 6,
	DDR4 = 7,
	LPDDR4_4X = 8
};

enum batteries_product_type {
	NO_BATTERIES = 0,
	YES_BATTERIES
};

typedef struct {
	int coef_k;
	int coef_b;
} svb_coef;

typedef union {
	struct {
		unsigned int work_v	: 16;	/* [15..0] */
		unsigned int cacl_v	: 16;	/* [31..16] */
	} bits;
	unsigned int u32;
} u_storage_reg;

typedef union {
	struct {
		unsigned int svb_pwm_enable	: 1;	/* [0..0] */
		unsigned int svb_pwm_inv    : 1;	/* [1..1] */
		unsigned int svb_pwm_load	: 1;	/* [2..2] */
		unsigned int reserved0      : 1;	/* [3..3] */
		unsigned int svb_pwm_period	: 10;	/* [13..4] */
		unsigned int reserved1	    : 2;	/* [15..14] */
		unsigned int svb_pwm_duty	: 10;	/* [25..16] */
		unsigned int reserved2	    : 6;	/* [31..26] */
	} bits;
	unsigned int u32;
} u_svb_pwm_ctrl;

typedef union {
	struct {
		unsigned int reserved0		: 20;	/* [19..0] */
		int core_delta_v			: 7;	/* [26..20] */
		unsigned int core_flag		: 1;	/* [27] */
		unsigned int reserved1		: 4;	/* [31..28] */
	} bits;
	unsigned int u32;
} u_core_delta_v_reg;

typedef union {
	struct {
		unsigned int hpm0		: 10;	/* [9..0] */
		unsigned int reserved_0	: 6;	/* [15..10] */
		unsigned int hpm1		: 10;	/* [25..16] */
		unsigned int reserved_1	: 6;	/* [31..26] */
	} bits;
	unsigned int u32;
} u_hpm_reg;

typedef union {
	struct {
		unsigned int reserved0		    : 16;	/* [15..0] */
		unsigned int start_up_temp	    : 8;	/* [23..16] */
		unsigned int temp_negative      : 1;	/* [24..24] */
		unsigned int reserved1		    : 7;	/* [31..25] */
	} bits;
	unsigned int u32;
} u_dyn_comp_falg_storage_reg;

typedef union {
	struct {
		unsigned int core_hpm_value	: 10;	/* [9..0] */
		unsigned int reserved1	    : 20;	/* [29..10] */
		unsigned int use_board_hpm	: 1;	/* [30..30] */
		unsigned int reserved0	    : 1;	/* [31..31] */
	} bits;
	unsigned int u32;
} u_hpm_storage_reg;

typedef union {
	struct {
		unsigned int battery_type	: 2;	/* [1..0] */
		unsigned int svb_type		: 4;	/* [5..2] */
		unsigned int reserved	    : 26;	/* [31..6] */
	} bits;
	unsigned int u32;
} u_svb_version_reg;

typedef union {
	struct {
		unsigned int reserved0		: 20;	/* [19..0] */
		unsigned int kgd_vendor_sel	: 4;	/* [23..20] */
		unsigned int reserved1	    : 8;	/* [31..24] */
	} bits;
	unsigned int u32;
} u_kgd_reg;

typedef union {
	struct {
		unsigned int dram_type		: 4;	/* [3..0] */
		unsigned int reserved	    : 28;	/* [31..4] */
	} bits;
	unsigned int u32;
} u_ddr_phy_reg;

typedef struct {
	int curve_a;
	int curve_b;
	int volt_min;
	int volt_max;
	int volt_fix;
} svb_curve;

typedef struct {
	svb_curve core_curve;
} svb_cfg;

void start_svb(void);
void end_svb(void);

#endif
