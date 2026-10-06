/*
 * DDR training custom define.
 *
 * Copyright (c) 2016 HiSilicon Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/* config DDRC, PHY, DDRT typte */
#include <asm/arch/platform.h>

/* #define CFG_EDA_VERIFY */

#define DDR_DDRC_V500_CONFIG
#define DDR_PHY_T28_CONFIG
#define DDR_DDRT_T28_CONFIG

/* config special item */
#define DDR_TRAINING_UART_DISABLE
/* #define DDR_TRAINING_LOG_DISABLE */

/* config DDRC, PHY, DDRT base address */
/* [CUSTOM] DDR DMC base register */
#define DDR_REG_BASE_DMC0              (DDRC0_REG_BASE + 0x1000)
/* [CUSTOM] DDR PHY base register */
#define DDR_REG_BASE_PHY0              (DDRC0_REG_BASE + 0x8000)
/* [CUSTOM] DDR DDRT base register */
#define DDR_REG_BASE_DDRT              DDR_DDRT_REG_BASE
/* [CUSTOM] DDR training item system control */
#define DDR_REG_BASE_SYSCTRL           SYS_CTRL_REG_BASE

#define DDR_PHY_NUM              1 /* phy number */
#define DDR_DMC_PER_PHY_MAX      1 /* dmc number per phy max */
#define DDR_AXI_SWITCH_NUM       1 /* ddr training axi switch number */

#define DDR_REG_BASE_UART0       UART0_REG_BASE
#define DDR_REG_BASE_DMC1        (DDRC0_REG_BASE + 0x1000)
#define SYSCTRL_DDR_TRAINING_CFG_SEC     0x90
#define SYSCTRL_DDR_HW_PHY0_RANK1        0x98
#define SYSCTRL_DDR_HW_PHY0_RANK0        0x90

/* config offset address */
/* [CUSTOM] ddrt reversed data */
#define SYSCTRL_DDRT_PATTERN           0x94
/* [CUSTOM] ddr training item */
#define SYSCTRL_DDR_TRAINING_CFG       0x90
/* [CUSTOM] ddr training stat*/
#define SYSCTRL_DDR_TRAINING_STAT      0x98

#define DDR_ASM_DSB()                     { __asm__ __volatile__("nop"); }

/* config other special */
/* [CUSTOM] DDR training start address. MEM_BASE_DDR*/
#define DDRT_CFG_BASE_ADDR		DDR_MEM_BASE
/* [CUSTOM] SRAM start address.
NOTE: Makefile will parse it, plase define it as Hex. eg: 0xFFFF0C00 */
#define DDR_TRAINING_RUN_STACK		0x04010500

#define DDR_TRAINING_DDRT_PREPARE_FUNC()
