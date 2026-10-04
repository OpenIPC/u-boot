/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#ifndef __FMC100_OS_H__
#define __FMC100_OS_H__

#include "fmc100.h"

#define IO_ADDR_R_SIZE 16

int board_nand_init(struct nand_chip *chip);

#endif /* End of __FMC100_OS_H__ */
