/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: cmd_write_otp_init.c
 * Author: Hisilicon multimedia software group
 * Create: 2023/05/04
 */

#include "cmd_write_otp_init.h"

#ifdef __KERNEL__
/*
 * For UBOOT
 */
#include <command.h>

static int do_write_otp(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	return do_write_otp_main(argc, argv);
}

U_BOOT_CMD(write_otp, CONFIG_SYS_MAXARGS, 0, do_write_otp, "write otp operation", "write otp\n");

#else
/*
 * For User Mode
 */
int main(int argc, char *argv[])
{
	return do_write_otp_main(argc, argv);
}
#endif
