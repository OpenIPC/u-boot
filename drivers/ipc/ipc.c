// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 *
 * Hi3519DV500 inter-processor mailbox, from the vendor SDK's U-Boot 2022.07
 * (drivers/ipc/ipc.c), with its securec and logging dependencies dropped.
 */

#include <common.h>
#include <ipc.h>
#include <asm/io.h>

static int ipc_send_msg_int(uint32_t node)
{
	if (node > IPC_INT_MAX) {
		printf("ipc: invalid interrupt %u\n", node);
		return -1;
	}
	writel(node, IPC_SET_REG);
	return 0;
}

static void ipc_msg_received(uint32_t node)
{
	writel(node, IPC_CLEAR_REG);
}

static int ipc_has_msg(uint32_t node)
{
	return (readl(IPC_STATUS_REG) & (1 << node)) != 0;
}

int ipc_send_msg(uint32_t node, const struct ipc_share_msg *msg)
{
	uint32_t i;
	uint32_t xor = 0;

	if (node >= IPC_INT_MAX || !msg || msg->len > IPC_DATA_BUF_SIZE) {
		printf("ipc: bad message to node %u\n", node);
		return -1;
	}
	writel(msg->cmd, IPC_CMD_REG);
	xor ^= msg->cmd;
	writel(msg->len, IPC_LEN_REG);
	xor ^= msg->len;
	for (i = 0; i < msg->len; i++) {
		writel(msg->buf[i], ipc_data_reg(i));
		xor ^= msg->buf[i];
	}
	writel(xor, IPC_XOR_REG);
	return ipc_send_msg_int(node);
}

static int do_ipc_recv_msg(struct ipc_share_msg *msg)
{
	uint32_t i;
	uint32_t xor = 0;

	msg->cmd = readl(IPC_CMD_REG);
	xor ^= msg->cmd;
	msg->len = readl(IPC_LEN_REG);
	xor ^= msg->len;
	if (msg->len > IPC_DATA_BUF_SIZE) {
		printf("ipc: bad length 0x%x\n", msg->len);
		return -1;
	}
	for (i = 0; i < msg->len; i++) {
		msg->buf[i] = readl(ipc_data_reg(i));
		xor ^= msg->buf[i];
	}
	msg->xor = readl(IPC_XOR_REG);
	if (msg->xor != xor) {
		printf("ipc: xor 0x%x, message says 0x%x\n", xor, msg->xor);
		return -1;
	}
	return 0;
}

/*
 * ipc_recv_msg - Receive a message for @node.
 *
 * @block: wait for one to arrive
 * Return: the length of the message, or a negative error.
 */
int ipc_recv_msg(uint32_t node, struct ipc_share_msg *msg, int block)
{
	int ret;

	if (!msg || node >= IPC_INT_MAX) {
		printf("ipc: bad receive on node %u\n", node);
		return -1;
	}
	if (block)
		while (!ipc_has_msg(node))
			;
	ret = do_ipc_recv_msg(msg);
	ipc_msg_received(node);
	return ret ? ret : (int)msg->len;
}
