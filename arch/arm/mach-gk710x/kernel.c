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

#ifdef CONFIG_GK710X_S
#include "hal_data_s.h"
#else
#include "hal_data.h"
#endif

DECLARE_GLOBAL_DATA_PTR;

/* A size as the kernel's mem= reads it: bytes, 0x for hex, K/M/G suffix */
static ulong env_size(const char *name, const char *def)
{
	const char *s = env_get(name);
	char *end;

	return ustrtoul(s ? s : def, &end, 0);
}

void board_prep_linux(bootm_headers_t *images)
{
	const ulong top = GK_DDR_BASE + gd->ram_size;

	memcpy((void *)GK_HAL_ADDR, gk_hal_data, sizeof(gk_hal_data));

	writel(env_get_ulong("phytype", 10, 0), GK_U2K_PHY_TYPE);
	/*
	 * The kernel's decompressor (clk_setting() in the vendor
	 * arch/arm/boot/compressed/misc.c) writes this word to RCT + 0x8c,
	 * the CPU PLL, before it decompresses. On the GK7101/GK7102 that is
	 * the setting arch_cpu_init() already made. On the GK710xS the boot
	 * header sets the PLLs, nothing here programs RCT + 0x8c, and the
	 * GK7102C's own kernel never writes it; with the GK7101/GK7102 value
	 * handed over, a GK7102C halted in the decompressor right after that
	 * write ("XZ decompressor ran out of memory"). Hand over what the PLL
	 * holds, so that write changes nothing.
	 */
#ifdef CONFIG_GK710X_S
	writel(readl(GK_RCT_PLL_ARM), GK_U2K_ARM_FREQ);
#else
	writel(GK_ARM_FREQ, GK_U2K_ARM_FREQ);
#endif
	writel(GK_HAL_ADDR, GK_U2K_HAL_ADDR);
	writel(0, GK_U2K_SOC);
	writel(env_size("osmem", OPENIPC_OSMEM), GK_U2K_MEM);
	writel(env_size("bsbsize", __stringify(GK_BSB_MIB) "M"), GK_U2K_BSB);
	writel(GK_U2K_MAGIC, GK_U2K_ENABLE);
	writel(0, GK_U2K_USR);
	writel(gd->ram_size / SZ_1M, GK_U2K_TOTAL_MEM);

	gk_gpio_map_save((void *)(top - GK_USR_SIZE));
}
