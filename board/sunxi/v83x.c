// SPDX-License-Identifier: GPL-2.0+
/*
 * Allwinner V831/V833 boards, as OpenIPC boots them
 */

#include <common.h>
#include <env.h>
#include <gzip.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/spl.h>
#include <spl.h>
#include <linux/bitops.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>

DECLARE_GLOBAL_DATA_PTR;

/* The vendor kernel device trees are about 60 KiB */
#define V83X_KERNEL_DTB_MAX	0x20000

extern const unsigned char v83x_kernel_dtb_gz[];
extern const unsigned char v83x_kernel_dtb_gz_end[];

void v83x_board_init(void)
{
	/*
	 * The external RMII PHY takes its 25 MHz from the SoC (EPHY_25M,
	 * PLL_PERI0 / 24): the vendor device tree's use_ephy25m
	 */
	setbits_le32(SUNXI_CCM_BASE + 0x970, BIT(31) | BIT(30));
}

/*
 * The kernels carry no device tree and the vendor U-Boot hands them its
 * own: unpack the one built in for bootm (boot/image-fdt.c boots a plain
 * uImage with the tree at ${fdtaddr}).
 */
void v83x_kernel_fdt(void)
{
	unsigned long len = v83x_kernel_dtb_gz_end - v83x_kernel_dtb_gz;
	void *fdt = malloc(V83X_KERNEL_DTB_MAX);

	if (!fdt)
		return;

	if (gunzip(fdt, V83X_KERNEL_DTB_MAX, (uchar *)v83x_kernel_dtb_gz,
		   &len) || fdt_check_header(fdt)) {
		printf("V83x: built-in kernel device tree is bad\n");
		free(fdt);
		return;
	}

	env_set_hex("fdtaddr", (ulong)fdt);
}

/*
 * Booted from an SD card (the -mmc images): that card is where the
 * unbricking files are, so ub/uk/ur read them from it, as after srcmmc.
 * Set for this boot only, not saved: the next boot from the SPI NOR keeps
 * the environment's source.
 */
void v83x_boot_source(void)
{
	if (sunxi_get_boot_device() != BOOT_DEVICE_MMC1)
		return;

	printf("V83x: booted from the SD card, update source: fatload mmc 0\n");
	env_set("source", "fatload mmc 0");
}

/* What the vendor U-Boot sets at boot besides /memory and /chosen */
void v83x_ft_board_setup(void *blob)
{
	int node = fdt_path_offset(blob, "/dram");

	/* the DRAM size, in MiB, in the upper half */
	if (node >= 0)
		fdt_setprop_u32(blob, node, "dram_para2",
				(gd->ram_size >> 20) << 16);
}
