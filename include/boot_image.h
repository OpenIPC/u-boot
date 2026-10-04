/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#ifndef __BOOT_BACKUP_H
#define __BOOT_BACKUP_H

#include <stdio.h>

#define SYS_CTRL_REG_BASE		0x11020000
#define SYS_CTRL_BOOT_REG		(SYS_CTRL_REG_BASE + 0X0130)
#define	sys_boot(x)			(SYS_CTRL_BOOT_REG + (x) * 4)

#define BACKUP_IMAGE_FLG_REG		(SYS_CTRL_REG_BASE + 0xC8)
#define BACKUP_IMAGE_OFF_REG		(SYS_CTRL_REG_BASE + 0xCC)

#define	dbg_info(fmt, args...)
#define	dbg_err(fmt, args...)		printf("err:[%s, %d]"fmt, __func__, __LINE__, ##args)

int verify_boot_image(void);
int get_otp_ree_verify_enable(void);
bool get_otp_boot_backup_enable(void);
int get_reg_verify_backup_image(void);

#endif /* __BOOT_BACKUP_H */
