/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include <common.h>
#include <asm/io.h>

#include "mpp_board.h"
#include "amp.h"

#define amp_writel(addr, value) ((*((volatile unsigned int *)(addr))) = (value))

void amp_unmute(void)
{
    /* GPIO2_1 */
    amp_writel(IOCFG_REGS_ADDR + 0x0004, 0x1100);

    /* output high */
    amp_writel(GPIO_REGS_ADDR + 0x2400, 0x02);
    amp_writel(GPIO_REGS_ADDR + 0x2008, 0x02);
}

void amp_mute(void)
{
    /* GPIO2_1 */
    amp_writel(IOCFG_REGS_ADDR + 0x0004, 0x1100);

    /* output high */
    amp_writel(GPIO_REGS_ADDR + 0x2400, 0x00);
    amp_writel(GPIO_REGS_ADDR + 0x2008, 0x00);
}
