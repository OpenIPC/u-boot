/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include <common.h>
#include <asm/io.h>

#include "mpp_board.h"
#include "aiao_hal_common.h"
#include "amp.h"

#define amp_writel(addr, value) ((*((volatile unsigned int *)(addr))) = (value))

void amp_unmute(void)
{
    /* enable GPIO1 apb clk */
    ot_reg_set_bit(1, 1, io_address(CRG_PERCTL4570_ADDR));

    /* GPIO1_4 */
    amp_writel(IOCFG_REGS_ADDR + 0x0004, 0x1200);

    /* output high */
    amp_writel(GPIO_REGS_ADDR + 0x1400, 0x10);
    amp_writel(GPIO_REGS_ADDR + 0x1040, 0x10);
}

void amp_mute(void)
{
    /* GPIO1_4 */
    amp_writel(IOCFG_REGS_ADDR + 0x0004, 0x1200);

    /* output high */
    amp_writel(GPIO_REGS_ADDR + 0x1400, 0x00);
    amp_writel(GPIO_REGS_ADDR + 0x1040, 0x00);

    /* disable GPIO1 apb clk */
    ot_reg_set_bit(0, 1, io_address(CRG_PERCTL4570_ADDR));
}
