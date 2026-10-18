// SPDX-License-Identifier: GPL-2.0+
/*
 * What the GK710x kernel needs from its boot loader besides ATAGs, as the
 * vendor U-Boot leaves it (common/cmd_bootm.c, CONFIG_UBOOT_TO_KERNEL):
 *  - the HAL blob at GK_HAL_ADDR and that address in a U2K word;
 *  - the other U2K words: total DDR, the BSB (bitstream buffer) carved out
 *    above the kernel's mem=, the PHY type;
 *  - the board's GPIO map in the top 64 KiB of DDR, which the kernel
 *    applies itself (board/goke/gk710x).
 * The kernel places its DSP memory from mem=, bsbsize and the total.
 */

#include <common.h>
#include <env.h>
#include <image.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/gk710x.h>
#include <linux/sizes.h>
#include <linux/stringify.h>

#include "hal_data.h"

DECLARE_GLOBAL_DATA_PTR;

/* "36M" -> 36: the environment spells these sizes in MiB */
static ulong env_mib(const char *name, const char *def)
{
	const char *s = env_get(name);

	return simple_strtoul(s ? s : def, NULL, 10);
}

void board_prep_linux(bootm_headers_t *images)
{
	const ulong top = GK_DDR_BASE + gd->ram_size;

	memcpy((void *)GK_HAL_ADDR, gk_hal_data, sizeof(gk_hal_data));

	writel(env_get_ulong("phytype", 10, 0), GK_U2K_PHY_TYPE);
	writel(GK_ARM_FREQ, GK_U2K_ARM_FREQ);
	writel(GK_HAL_ADDR, GK_U2K_HAL_ADDR);
	writel(0, GK_U2K_SOC);
	writel(env_mib("osmem", OPENIPC_OSMEM) * SZ_1M, GK_U2K_MEM);
	writel(env_mib("bsbsize", __stringify(GK_BSB_MIB)) * SZ_1M, GK_U2K_BSB);
	writel(GK_U2K_MAGIC, GK_U2K_ENABLE);
	writel(0, GK_U2K_USR);
	writel(gd->ram_size / SZ_1M, GK_U2K_TOTAL_MEM);

	gk_gpio_map_save((void *)(top - GK_USR_SIZE));
}
