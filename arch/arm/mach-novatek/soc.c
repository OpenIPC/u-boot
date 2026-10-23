// SPDX-License-Identifier: GPL-2.0+
/*
 * Novatek NA51089: the loader's HEADINFO, reset and DRAM size. The
 * timer is timer 0 of the timer block, which the loader leaves running
 * (CONFIG_SYS_TIMER_COUNTER in include/configs/novatek.h).
 */

#include <common.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/na51089.h>
#include <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * HEADINFO, at 0x300 into the image (board/novatek/na51089/u-boot.lds).
 * The loader starts U-Boot at CodeEntry, and refuses an image whose
 * CodeEntry is not where its device tree's /nvt_memory_cfg/uboot puts it.
 * openipc/nvt-image.py fills in the length and the checksum.
 */
struct nvt_headinfo {
	u32 code_entry;
	u32 resv1[19];		/* written by the loader */
	char chip_name[8];
	char version[8];
	char date[8];
	u32 bin_length;
	u32 checksum;
	u32 crc_len_check;
	u32 modelext_addr;	/* the loader writes its FDT address here */
	u32 bin_ctrl;
	u32 crc_tag;
};

static const struct nvt_headinfo headinfo
	__used __section(".nvt_headinfo") = {
	.code_entry	= CONFIG_SYS_TEXT_BASE,
	.chip_name	= "ub51089 ",
	.version	= "OpenIPC ",
	.date		= "        ",
	.bin_length	= 0xffffffff,
	.checksum	= 0xffffffff,
};

/* As the vendor U-Boot resets: the watchdog, then its manual trigger */
void reset_cpu(void)
{
	setbits_le32(NVT_CG_CLKEN0, NVT_CG_WDT);
	setbits_le32(NVT_CG_RSTN0, NVT_CG_WDT);
	writel(NVT_WDT_KEY | 0x112, NVT_WDT_CTRL);
	udelay(80);
	writel(NVT_WDT_KEY | 0x112 | NVT_WDT_EN, NVT_WDT_CTRL);
	writel(1, NVT_WDT_MANUAL);
	while (1)
		;
}

/* The DDR is in the package: its size is the part's */
int dram_init(void)
{
	gd->ram_size = PHYS_SDRAM_1_SIZE;
	return 0;
}

#ifdef CONFIG_DISPLAY_CPUINFO
int print_cpuinfo(void)
{
	printf("CPU:   Novatek %s (chip id %04x)\n", CONFIG_PRODUCT_SOC,
	       readl(NVT_TOP_CHIP_ID) >> 16);
	return 0;
}
#endif
