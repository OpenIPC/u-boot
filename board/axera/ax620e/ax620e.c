// SPDX-License-Identifier: GPL-2.0+
/*
 * OpenIPC board file for AX620E cameras: the OpenIPC boot layer from
 * misc_init_r(). Pads stay as the first stage and the board's own kernel
 * set them; U-Boot uses UART0, the boot SPI flash and the Ethernet MAC's
 * own 10/100 PHY, none of which needs a pad of its own here.
 */

#include <common.h>
#include <init.h>
#include <openipc.h>

int board_init(void)
{
	return 0;
}

int misc_init_r(void)
{
	return openipc_helper();
}
