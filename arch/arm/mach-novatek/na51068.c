// SPDX-License-Identifier: GPL-2.0+
/*
 * Novatek NA51068: the loader's HEADINFO, the timer, reset and the DRAM
 * size.
 */

#include <common.h>
#include <init.h>
#include <time.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/na51068.h>
#include <linux/delay.h>
#include <linux/libfdt.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * HEADINFO, at 0x300 into the image (board/novatek/na51068/u-boot.lds),
 * as on NA51089. The loader starts U-Boot at CodeEntry, and refuses an
 * image whose CodeEntry is not where its device tree's
 * /nvt_memory_cfg/uboot puts it. openipc/nvt-image.py fills in the length
 * and the checksum. BinCtrl bit 1 (SMP), as the vendor U-Boot sets it for
 * its SMP kernels: the loader then parks the second core at
 * /nvt_memory_cfg/core2entry2, where Linux releases it (cpu-release-addr).
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

#define NVT_BIN_CTRL_SMP	BIT(1)

static const struct nvt_headinfo headinfo
	__used __section(".nvt_headinfo") = {
	.code_entry	= CONFIG_SYS_TEXT_BASE,
	.chip_name	= "ub51068 ",
	.version	= "OpenIPC ",
	.date		= "        ",
	.bin_length	= 0xffffffff,
	.checksum	= 0xffffffff,
	.bin_ctrl	= NVT_BIN_CTRL_SMP,
};

/* PLL16, in Hz: 12 MHz times its 24-bit ratio over 2^17 */
static ulong nvt_pll16_hz(void)
{
	u32 ratio = (readl(NVT_CG_PLL16 + 0x20) & 0xff) |
		    (readl(NVT_CG_PLL16 + 0x24) & 0xff) << 8 |
		    (readl(NVT_CG_PLL16 + 0x28) & 0xff) << 16;

	return ((u64)ratio * 12000000) >> 17;
}

/* The CPU's clock: eight times PLL16, or 80 MHz off the APLL */
static ulong nvt_cpu_hz(void)
{
	if (!(readl(NVT_CG_CPU_CKCTRL) & NVT_CPU_CK_APLL))
		return 80000000;
	return nvt_pll16_hz() * 8;
}

/*
 * The A9 global timer, 64 bits at an eighth of the CPU's clock. The CPU
 * clock is the loader's: the vendor U-Boot raises it to the device tree's
 * after setting the core voltage from the eFuse trim, which is not done
 * here.
 */
int timer_init(void)
{
	gd->arch.timer_rate_hz = nvt_cpu_hz() / 8;
	setbits_le32(NVT_GTIMER_CTRL, BIT(0));
	return 0;
}

ulong get_tbclk(void)
{
	return gd->arch.timer_rate_hz;
}

uint64_t notrace get_ticks(void)
{
	u32 hi, lo;

	do {
		hi = readl(NVT_GTIMER_HI);
		lo = readl(NVT_GTIMER_LO);
	} while (hi != readl(NVT_GTIMER_HI));
	return (u64)hi << 32 | lo;
}

/*
 * As the vendor kernel resets (plat-na51068/reset.c): the watchdog's
 * clock and reset, then its manual trigger
 */
void reset_cpu(void)
{
	clrbits_le32(NVT_CG_APB_RSTN, NVT_CG_WDT_RSTN);
	udelay(50);
	setbits_le32(NVT_CG_APB_RSTN, NVT_CG_WDT_RSTN);
	clrbits_le32(NVT_CG_APB_GATE, NVT_CG_WDT_GATE);
	writel(NVT_WDT_KEY | 0x112, NVT_WDT_CTRL);
	udelay(80);
	writel(NVT_WDT_KEY | 0x112 | NVT_WDT_EN, NVT_WDT_CTRL);
	writel(1, NVT_WDT_MANUAL);
	while (1)
		;
}

/* The loader's device tree, from SHMINFO, or NULL */
const void *nvt_loader_fdt(void)
{
	const void *shm = (const void *)NVT_SHMINFO;
	const void *fdt;

	if (memcmp(shm + NVT_SHM_LD_NVT, "LD_NVT", 6))
		return NULL;
	fdt = (const void *)readl(shm + NVT_SHM_FDT_ADDR);
	return fdt_check_header(fdt) ? NULL : fdt;
}

/*
 * The DDR is on the board: its size is what the loader's device tree
 * says (/nvt_memory_cfg/dram), which its DDR set-up was built for
 */
int dram_init(void)
{
	const void *fdt = nvt_loader_fdt();
	const fdt32_t *reg = NULL;
	int node, len;

	gd->ram_size = PHYS_SDRAM_1_SIZE;
	if (fdt) {
		node = fdt_path_offset(fdt, "/nvt_memory_cfg/dram");
		if (node >= 0)
			reg = fdt_getprop(fdt, node, "reg", &len);
		if (reg && len == 8 && fdt32_to_cpu(reg[1]))
			gd->ram_size = fdt32_to_cpu(reg[1]);
	}
	return 0;
}

#ifdef CONFIG_DISPLAY_CPUINFO
int print_cpuinfo(void)
{
	printf("CPU:   Novatek %s @ %lu MHz\n", CONFIG_PRODUCT_SOC,
	       (nvt_cpu_hz() + 500000) / 1000000);
	return 0;
}
#endif
