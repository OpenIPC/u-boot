/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 *
 * The inter-processor mailbox the Hi3519DV500 boot chain talks over: U-Boot
 * asks the GSL to start BL31, and BL31 to start the kernel
 * (board/vendor/hi3519dv500/bl31_handoff.c).
 * From the vendor SDK's U-Boot 2022.07 (include/ipc.h); the registers and the
 * command codes are the SoC's, in <asm/arch/platform.h>.
 */

#ifndef __IPC_H
#define __IPC_H

#include <linux/types.h>
#include <asm/arch/platform.h>

#define ipc_share_reg(x)	(IPC_SHARE_REG_BASE + ((x) * 4))
#define IPC_CMD_REG		ipc_share_reg(0)
#define IPC_LEN_REG		ipc_share_reg(1)
#define IPC_XOR_REG		ipc_share_reg(IPC_SHARE_MAX_REG - 1)
#define ipc_data_reg(x)		ipc_share_reg(2 + (x))
#define IPC_DATA_BUF_SIZE	(IPC_SHARE_MAX_REG - 3)

/*
 * Message Format:
 * | REG[0] | REG[1] | REG[2..n-2] | REG[n-1] |
 * |  cmd   |  len   |    buffer   |   xor    |
 *
 * @cmd: the command of message
 * @len: the length of data in buffer
 * @buffer: message data
 * @xor: the XOR value of cmd, len and the valid data in buffer
 */
struct ipc_share_msg {
	uint32_t cmd;
	uint32_t len;
	uint32_t buf[IPC_DATA_BUF_SIZE];
	uint32_t xor;
};

int ipc_send_msg(uint32_t node, const struct ipc_share_msg *msg);
int ipc_recv_msg(uint32_t node, struct ipc_share_msg *msg, ulong timeout_ms);

#endif /* __IPC_H */
