/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
 */

#ifndef DDR_TRAINING_CUSTOM_H
#define DDR_TRAINING_CUSTOM_H

#include "share_drivers.h"

/* boot adaptation */
#if BOOTLOADER == DDR_BOOT_TYPE_UBOOT
#define DDR_VREF_TRAINING_CONFIG
#define DDR_VREF_WITHOUT_BDL_CONFIG
#elif BOOTLOADER == DDR_BOOT_TYPE_CMD_BIN
#define DDR_VREF_TRAINING_CONFIG
#define DDR_VREF_WITHOUT_BDL_CONFIG
#elif BOOTLOADER == DDR_BOOT_TYPE_HSL
#define DDR_TRAINING_LOG_DISABLE
#elif BOOTLOADER == DDR_BOOT_TYPE_AUX_CODE
#define DDR_TRAINING_LOG_DISABLE
#else
# error Unknown boot Type
#endif

/* config DDRC, PHY, DDRT typte */
#define DDR_DDRC_S14_CONFIG
#define DDR_PHY_S14_CONFIG
#define DDR_DDRT_V2_3_SHF0_CONFIG

/* config special item */
/* s40/t28/t16 not support dcc training */
#define DDR_RDQBDL_ADJUST_CONFIG
#define DDR_ANTI_AGING_CONFIG
#define DDR_AUTO_PD_CONFIG
#define DDR_LOW_POWER_CONFIG
#define DDR_CAPAT_ADAPT_CFG
#define DDR_ZQ_TRIM_CONFIG
#define DDR_TRAINING_UART_DISABLE
#define DDR_TRAINING_MINI_LOG_CONFIG

#define DDR_PHY_NUM         1 /* phy number */

#define DDR_DMC_PER_PHY_NUM 1 /* dmc number per phy */

#define DDR_RDQS_OFFSET          10 /* rdqs offset */
#define DDR_RDQS_OFFSET_LPDDR4   0 /* rdqs offset */
#define DDR_RDQ_OFFSET           0x3 /* rdq offset */

/* config DDRC, PHY, DDRT base address */
/* [CUSTOM] DDR PHY0 base register */
#define DDR_REG_BASE_PHY0        0x11150000
/* [CUSTOM] DDR DMC0 base register */
#define DDR_REG_BASE_DMC0        0x11148000
/* [CUSTOM] DDR DMC1 base register */
#define DDR_REG_BASE_DMC1        0x11149000
/* [CUSTOM] DDR DDRT base register */
#define DDR_REG_BASE_DDRT        0x11160000
/* [CUSTOM] DDR training item system control */
#define DDR_REG_BASE_SYSCTRL     0x11020000
#define DDR_REG_BASE_AXI         0x11140000
/* Serial Configuration */
#define DDR_REG_BASE_UART0       0x11040000

/* config offset address */
/* Assume sysctrl offset address for DDR training as follows,
if not please define. */
/* [CUSTOM] ddr hw training item */
#define SYSCTRL_DDR_HW_PHY0_RANK0       0x90
#define SYSCTRL_DDR_HW_PHY0_RANK1       0x94
/* [CUSTOM] ddr sw training item */
#define SYSCTRL_DDR_TRAINING_CFG        0xa0 /* RANK0 */
#define SYSCTRL_DDR_TRAINING_CFG_SEC    0xa4 /* RANK1 */
/* [CUSTOM] PHY0 ddrt reversed data */
#define SYSCTRL_DDRT_PATTERN            0xa8
/* [CUSTOM] ddr training stat */
#define SYSCTRL_DDR_TRAINING_STAT       0xb0
/* [CUSTOM] ddr training version flag */
#define SYSCTRL_DDR_TRAINING_VERSION_FLAG	0xb4
#define SYSCTRL_DDR_TRAINING_CTX        0xb8
#define SYSCTRL_DDR_TRAINING_PHY        0xbc
#define SYSCTRL_DDR_TRAINING_ADJUST     0xc0
#define SYSCTRL_DDR_TRAINING_DQ_TYPE    0xc4

/* MISC DDRT control register */
#define MISC_DDRT_CTRL     0x4174
#define DDRT_MST_SEL_BIT   0 /* bit[0] ddrt_mst_sel */

/* config other special */
/* [CUSTOM] DDR training start address. MEM_BASE_DDR */
#define DDRT_CFG_BASE_ADDR       0x40000000
/* [CUSTOM] SRAM start address.
NOTE: Makefile will parse it, plase define it as Hex. eg: 0xFFFF0C00 */
#define DDR_TRAINING_RUN_STACK      0x04028000

#define CRG_REG_BASE_ADDR  0x11010000
#define CRG_DDRT           0x22a0   /* DDRT clock and soft reset control register */
#define DDR_TEST0_CKEN_BIT     4  /* ddrtest0_cken */
#define DDR_TEST0_SRST_REQ_BIT 0  /* ddrtest0_srst_req */

#define DDR_RELATE_REG_DECLARE
struct ddrtrn_hal_training_custom_reg {
	unsigned int ddrt_clk_reg;
	unsigned int age_compst_en[DDR_PHY_NUM];
	unsigned int trfc_en[DDR_PHY_NUM];
	unsigned int ddrt_ctrl;
};

#define OTP_RG_RES_TRIM           0x101e0110
typedef union {
	struct {
		unsigned int reserved1 : 16;  /* [15:0] */
		unsigned int rg_res_trim : 5; /* [20:16] */
		unsigned int reserved2 : 11;  /* [31:21] */
	} bits;
	unsigned int val;
} otp_rg_res_trim_desc;
#define DDRPHY0_OTP_RG_RES_TRIM_MASK  0x1f /* [20:16] */

#if defined(DDR_TRAINING_CMD) || defined(DDR_TRAINING_UBOOT)
#define ddr_serial_puts uart_early_puts
#define ddr_serial_put_hex uart_early_put_hex
#else
#define ddr_serial_puts(x) log_serial_puts((const s8 *)(x))
#define ddr_serial_put_hex serial_put_hex
#endif

int ddrtrn_hal_boot_cmd_save(struct ddrtrn_hal_training_custom_reg *custom_reg);
void ddrtrn_hal_boot_cmd_restore(const struct ddrtrn_hal_training_custom_reg *custom_reg);
void ddrtrn_zq_res_trim(void);
unsigned int ddrtrn_read_ddr_in_dword(unsigned int addr);
void ddrtrn_write_ddr_in_dword(unsigned int val, unsigned int addr);
#endif /* DDR_TRAINING_CUSTOM_H */
