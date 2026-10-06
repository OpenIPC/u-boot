#ifndef DDR_TRAINING_CUSTOM_H
#define DDR_TRAINING_CUSTOM_H

#include <asm/arch/platform.h>

/* config DDRC, PHY, DDRT type */
#define DDR_DDRC_V500_CONFIG
#define DDR_PHY_S40_CONFIG
#define DDR_DDRT_S40_CONFIG

/* config special item */
#define DDR_TRAINING_UART_DISABLE
#define DDR_TRAINING_ADJUST_CMD_DISABLE

#define DDR_PHY_NUM              1 /* phy number */
#define DDR_DMC_PER_PHY_MAX      1 /* dmc number per phy max */
#define DDR_AXI_SWITCH_NUM       1 /* ddr training axi switch number */

/* config DDRC, PHY, DDRT base address */
#define DDR_REG_BASE_PHY0        DDR_PHY0_REG_BASE
#define DDR_REG_BASE_DMC0        DDR_DMC0_REG_BASE
#define DDR_REG_BASE_DDRT        DDR_DDRT_REG_BASE
#define DDR_REG_BASE_SYSCTRL     SYS_CTRL_REG_BASE
#define DDR_REG_BASE_UART0       UART0_REG_BASE
#define DDR_REG_BASE_DMC1        DDR_DMC0_REG_BASE
#define SYSCTRL_DDR_TRAINING_CFG_SEC     0x90
#define SYSCTRL_DDR_HW_PHY0_RANK1        0x98

/* config offset address */
#define SYSCTRL_DDRT_PATTERN             0x94
#define SYSCTRL_DDR_TRAINING_CFG         0x90
#define SYSCTRL_DDR_TRAINING_STAT        0x98
#define SYSCTRL_DDR_HW_PHY0_RANK0        0x90

/* config other special */
#define DDRT_CFG_BASE_ADDR       0x80000000
#define DDR_TRAINING_RUN_STACK   0x04010500

/* armv5 has no dsb, use nop */
#define DDR_ASM_DSB()            { __asm__ __volatile__("nop"); }

#define DDR_TRAINING_DDRT_PREPARE_FUNC()   ddr_ddrt_prepare_custom()
void ddr_ddrt_prepare_custom(void);

#endif /* DDR_TRAINING_CUSTOM_H */
