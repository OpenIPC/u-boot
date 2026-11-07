// SPDX-License-Identifier: GPL-2.0+
/*
 * SigmaStar camera board, after the vendor U-Boot 2015.01
 * (arch/arm/cpu/armv7/<die>/board.c).
 */

#include <common.h>
#include <init.h>
#include <net.h>
#include <netdev.h>
#include <openipc.h>
#include <asm/global_data.h>
#include <asm/arch/sigmastar.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	gd->bd->bi_boot_params = SS_BOOT_PARAMS;
	return 0;
}

int misc_init_r(void)
{
#ifdef CONFIG_SS_MMA_BOOTARGS
	ss_set_mem_env();
#endif
	return openipc_helper();
}

#ifdef CONFIG_MMC_MS_SDMMC
int board_mmc_init(struct bd_info *bis)
{
	return ms_sdmmc_initialize();
}
#endif

#ifdef CONFIG_MS_EMAC
int board_eth_init(struct bd_info *bis)
{
	return ms_emac_initialize();
}
#endif
