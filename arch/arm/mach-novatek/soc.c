// SPDX-License-Identifier: GPL-2.0+
/*
 * Novatek: the loader's HEADINFO, reset and DRAM size. On NA51089,
 * NA51055, NA51084 and NS02501 the timer is timer 0 of the timer block,
 * which the loader leaves running (CONFIG_SYS_TIMER_COUNTER in
 * include/configs/novatek.h); on the AArch64 NVR dies (NA51103, NA51090)
 * it is the architected timer, and the MMU maps the DDR and the blocks
 * above 4 GiB.
 */

#include <common.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#ifdef CONFIG_ARM64
#include <asm/armv8/mmu.h>
#endif
#include <linux/delay.h>
#include <linux/libfdt.h>

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
	/* The vendor U-Boot's tag; no loader checks it */
#if defined(CONFIG_NVT_NA51103)
	.chip_name	= "ub51103 ",
#elif defined(CONFIG_NVT_NA51090)
	.chip_name	= "ub51090 ",
#elif defined(CONFIG_NVT_NA51055)
	.chip_name	= "ub51055 ",
#elif defined(CONFIG_NVT_NS02501)
	.chip_name	= "ub02501 ",
#else
	.chip_name	= "ub51089 ",
#endif
	.version	= "OpenIPC ",
	.date		= "        ",
	.bin_length	= 0xffffffff,
	.checksum	= 0xffffffff,
#ifdef CONFIG_NVT_NA511XX
	/* As the vendor's SMP builds: Linux brings up the other cores */
	.bin_ctrl	= 2,
#endif
};

const void *nvt_loader_fdt(void)
{
	const void *shm = (const void *)NVT_SHMINFO;
	const void *fdt;

	if (memcmp(shm + NVT_SHM_LD_NVT, "LD_NVT", 6))
		return NULL;
	fdt = (const void *)(ulong)readl(shm + NVT_SHM_FDT_ADDR);
	return fdt_check_header(fdt) ? NULL : fdt;
}

/*
 * As the vendor U-Boot resets: the watchdog, then its manual trigger.
 * CTRL is written as each vendor U-Boot writes it: 0x112 under the key,
 * 0x102 on NS02501 (ns02501_hw_init.c).
 */
#ifdef CONFIG_NVT_NS02501
#define NVT_WDT_CFG	0x102
#else
#define NVT_WDT_CFG	0x112
#endif

void reset_cpu(void)
{
#if defined(CONFIG_NVT_NA511XX)
	setbits_le32(NVT_CG_WDT_RSTN, NVT_CG_WDT_RST);
	setbits_le32(NVT_CG_WDT_CLKEN, NVT_CG_WDT_EN);
#elif defined(CONFIG_NVT_NS02501)
	/* ns02501_hw_init.c: its reset and clock bits, a zero count */
	setbits_le32(NVT_CG_WDT_RSTN, NVT_CG_WDT_RST);
	setbits_le32(NVT_CG_CLKEN0, NVT_CG_WDT);
	writel(0, NVT_WDT_DATA);
#else
	setbits_le32(NVT_CG_CLKEN0, NVT_CG_WDT);
	setbits_le32(NVT_CG_RSTN0, NVT_CG_WDT);
#endif
	writel(NVT_WDT_KEY | NVT_WDT_CFG, NVT_WDT_CTRL);
	udelay(80);
	writel(NVT_WDT_KEY | NVT_WDT_CFG | NVT_WDT_EN, NVT_WDT_CTRL);
	writel(1, NVT_WDT_MANUAL);
	while (1)
		;
}

#ifdef CONFIG_ARM64
/*
 * The DDR, cached, and the blocks at 0x2_F000_0000 as device memory. With
 * the MMU off every access would be a device one, and the unaligned ones
 * U-Boot makes would fault.
 */
static struct mm_region nvt_mem_map[] = {
	{
		.virt = NVT_DDR_BASE,
		.phys = NVT_DDR_BASE,
		.size = CONFIG_NVT_DDR_SIZE,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_INNER_SHARE,
	}, {
		.virt = NVT_IO(0),
		.phys = NVT_IO(0),
		.size = NVT_IO_SIZE,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN,
	}, {
		0,
	}
};

struct mm_region *mem_map = nvt_mem_map;
#endif

/* The DDR is in the package, or on the board: its size is the part's */
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
