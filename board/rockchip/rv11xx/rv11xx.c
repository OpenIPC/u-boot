// SPDX-License-Identifier: GPL-2.0+
/*
 * OpenIPC on Rockchip RV11xx cameras: the DRAM size from the preloader
 * ATAGs the DDR-init blob leaves, the SPI NAND as nand0, and the OpenIPC
 * boot layer from misc_init_r().
 */
#include <common.h>
#include <dm.h>
#include <env.h>
#include <init.h>
#include <nand.h>
#include <openipc.h>
#include <linux/mtd/mtd.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * The DDR blob's ATAGs, 8 KiB below 2 MiB (Rockchip's
 * arch-rockchip/rk_atags.h, which mainline does not carry): a list of
 * { size in words, tag, ... }, ended by ATAG_NONE. ATAG_DDR_MEM is
 * { count, flags?, data[2 * 10] }: bank i's base in data[i], its size in
 * data[count + i].
 */
#define ATAGS_PHYS_BASE		0x1fe000
#define ATAGS_SIZE		0x2000
#define ATAG_CORE		0x54410001
#define ATAG_NONE		0x00000000
#define ATAG_DDR_MEM		0x54410052

static u64 rv11xx_atag_dram_size(void)
{
	u32 *p = (u32 *)ATAGS_PHYS_BASE;
	u32 *end = (u32 *)(ATAGS_PHYS_BASE + ATAGS_SIZE);

	if (p[1] != ATAG_CORE)
		return 0;
	while (p < end && p[0] >= 2 && p[1] != ATAG_NONE) {
		if (p[1] == ATAG_DDR_MEM) {
			u32 count = p[2], i;
			u64 size = 0;

			/* bank[] (u64, two words each) after count, version */
			for (i = 0; i < count && i < 10; i++) {
				u32 *b = &p[4 + 2 * (count + i)];

				size += b[0] | (u64)b[1] << 32;
			}
			return size;
		}
		p += p[0];
	}
	return 0;
}

int dram_init(void)
{
	u64 size = rv11xx_atag_dram_size();

	/*
	 * Without ATAGs (a DDR blob that leaves none): the smallest RV1103's
	 * 64 MiB, safe on every part
	 */
	gd->ram_size = size ? size : SZ_64M;
	return 0;
}

/* All of it: U-Boot relocates to its top (mainline's Rockchip SPL asks too) */
ulong board_get_usable_ram_top(ulong total_size)
{
	return gd->ram_base + gd->ram_size;
}

#if defined(CONFIG_SYS_NAND_SELF_INIT) && !defined(CONFIG_SPL_BUILD)
/*
 * Mainline's spi-nand is a DM MTD device, registered by its probe as
 * spi-nand0. The environment and the nand command want it as nand0:
 * take it out of the MTD table and let nand_register() put it back
 * under that name (mtdids maps nand0 to the kernel's spi-nand0), as on
 * openipc-axera.
 */
void board_nand_init(void)
{
	struct udevice *dev;
	struct mtd_info *mtd;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_MTD,
					  DM_DRIVER_GET(spinand), &dev);
	if (ret) {
		printf("SPI NAND: not found (%d)\n", ret);
		return;
	}
	mtd = dev_get_uclass_priv(dev);
	del_mtd_device(mtd);
	ret = nand_register(0, mtd);
	if (ret)
		printf("SPI NAND: nand_register: %d\n", ret);
}
#endif

/*
 * The kernel's mem= when the environment has none: all of the DRAM. The
 * Rockchip kernel reserves the media buffers itself, as CMA from its
 * device tree, instead of an MMZ pool behind mem=.
 */
static void rv11xx_osmem(void)
{
	char buf[16];

	if (env_get("osmem"))
		return;
	snprintf(buf, sizeof(buf), "%luM", (ulong)(gd->ram_size / SZ_1M));
	env_set("osmem", buf);
}

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	rv11xx_osmem();
	return openipc_helper();
}
#endif
