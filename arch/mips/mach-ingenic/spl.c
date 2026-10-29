// SPDX-License-Identifier: GPL-2.0+
/*
 * Ingenic T-series SPL: the vendor SPL's board_init_f() and SFC NOR boot
 * (arch/mips/cpu/xburst/<soc>/soc.c, common/spl/spl.c and spl_sfc_nor.c of
 * OpenIPC/u-boot-ingenic), in the order the vendor runs them.
 *
 * The boot ROM copies the SPL from the start of the SPI NOR into the cache,
 * locked as RAM at CONFIG_SPL_TEXT_BASE, and runs it on the ROM's stack. The
 * SPL brings up the PLLs and the DDR, reads the LZO-packed U-Boot (a legacy
 * uImage, at the end of the SPL slot) into the DDR, unpacks it to its link
 * address and jumps to it.
 *
 * Copyright (c) 2013-2019 Ingenic Semiconductor Co.,Ltd
 */

/* The vendor SPL prints each step */
#define DEBUG
#include <common.h>
#include <cpu_func.h>
#include <hang.h>
#include <image.h>
#include <init.h>
#include <serial.h>
#include <linux/lzo.h>
#include <timestamp.h>
#include <version.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/sections.h>
#include <asm/gpio.h>
#include <asm/arch/clk.h>
#ifdef CONFIG_CPU_XBURST2
#include <asm/arch/tcu.h>
#endif
#include <mach/ingenic.h>
#include <mach/global_info.h>

DECLARE_GLOBAL_DATA_PTR;

/* In .data: the BSS is in the cache too, and is cleared only after DDR */
gd_t gdata __section(".data");
static struct global_info ginfo __section(".data") = {
	.extal		= CONFIG_SYS_EXTAL,
	.cpufreq	= CONFIG_SYS_CPU_FREQ,
	.ddrfreq	= CONFIG_SYS_MEM_FREQ,
	.uart_idx	= CONFIG_SYS_UART_INDEX,
	.baud_rate	= CONFIG_BAUDRATE,
};

/* Where the vendor SPL reads the packed image to, below U-Boot's unpacked
 * end, and where U-Boot is unpacked */
#define INGENIC_DECMP_BUFFER	0x80200000
#define INGENIC_UNPACK_MAX	0x00100000

void pll_init(void);
void clk_prepare(void);
void clk_init(void);
void enable_uart_clk(void);
#ifdef CONFIG_T41
void t41_sdram_init(void);
#else
void sdram_init(void);
#endif
void sfc_nor_load(unsigned int src_addr, unsigned int count,
		  unsigned int dst_addr);
void sfc_init(void);
void flush_cache_all(void);

static void __noreturn ingenic_spl_boot_nor(void)
{
	struct image_header *hdr = (void *)INGENIC_DECMP_BUFFER;
	typedef void __noreturn (*entry_t)(void);
	size_t len = INGENIC_UNPACK_MAX;
	ulong size, load, ep;
	int ret;

#ifdef CONFIG_CPU_XBURST2
	/* The T40 vendor SPL sets the SFC up once, before reading */
	sfc_init();
#endif
	sfc_nor_load(CONFIG_INGENIC_SPL_PAD_TO, sizeof(*hdr), (ulong)hdr);
	if (image_get_magic(hdr) != IH_MAGIC) {
		printf("No U-Boot image at 0x%x\n", CONFIG_INGENIC_SPL_PAD_TO);
		hang();
	}
	size = image_get_data_size(hdr);
	load = image_get_load(hdr);
	ep = image_get_ep(hdr);

	sfc_nor_load(CONFIG_INGENIC_SPL_PAD_TO, size + sizeof(*hdr), (ulong)hdr);
	ret = lzop_decompress((void *)image_get_data(hdr), size, (void *)load,
			      &len);
	if (ret) {
		printf("U-Boot image does not unpack (%d)\n", ret);
		hang();
	}

	flush_cache_all();
#ifdef CONFIG_CPU_XBURST2
	/*
	 * The L2 cache size, as the T40 vendor SPL sets it before the jump
	 * (0xb2200060 bits 12:10: 1 = 128 KiB), behind a sync and an
	 * uncached load on both sides
	 */
	__asm__ volatile(
			".set push     \n\t"
			".set mips32r2 \n\t"
			"sync          \n\t"
			"lw $0,0(%0)   \n\t"
			".set pop      \n\t"
			::"r" (0xa0000000));
#ifdef CONFIG_T41
	/* the T41 vendor SPL changes those bits only */
	writel((readl((void __iomem *)0xb2200060) & ~(0x7 << 10)) | (0x1 << 10),
	       (void __iomem *)0xb2200060);
#else
	writel(0x00000400, (void __iomem *)0xb2200060);
#endif
	__asm__ volatile(
			".set push     \n\t"
			".set mips32r2 \n\t"
			"sync          \n\t"
			"lw $0,0(%0)   \n\t"
			".set pop      \n\t"
			::"r" (0xa0000000));
#endif
	printf("image entry point: 0x%lX\n", ep);
	((entry_t)ep)();
}

#ifdef CONFIG_CPU_XBURST2
/* TCU timer 3 counts from here, for the vendor's boot-time stamps */
static void init_bootuptime_timer(void)
{
	writel(TCU_TCSR_PRESCALE16 | TCU_TCSR_RTC_EN, TCU_TCSR(3));
	writel(0xffff, TCU_TDFR(3));
	writel(0, TCU_TCNT(3));
	writel(TCU_TER_TCEN3, TCU_TESR);
}
#endif

void board_init_f(ulong dummy)
{
#ifdef CONFIG_CPU_XBURST2
	/* As early as possible, as the T40 vendor SPL */
	init_bootuptime_timer();
#endif

	/*
	 * The vendor SPL cleared the BSS only after the DDR, but the serial
	 * core reads its state there from serial_init() on. It is cache RAM
	 * like the rest of the SPL, so clear it first.
	 */
	memset(__bss_start, 0, (char *)&__bss_end - __bss_start);

	gd = &gdata;

#ifdef CONFIG_CPU_XBURST2
	/* close the CCU IFU simple loop, as the T40 vendor SPL; the T41's
	 * sets bits 6:5 too */
#ifdef CONFIG_T41
	writel(readl((void __iomem *)0xb2200fe0) | 0x00000078,
	       (void __iomem *)0xb2200fe0);
#else
	writel(readl((void __iomem *)0xb2200fe0) | 0x00000018,
	       (void __iomem *)0xb2200fe0);
#endif
#endif

	gd->arch.gi = &ginfo;

	gpio_init();

	/* Init uart first */
	enable_uart_clk();

	gd->baudrate = CONFIG_BAUDRATE;
	serial_init();
	gd->have_console = 1;
	puts("\nU-Boot SPL " PLAIN_VERSION " (" U_BOOT_DATE " - "
	     U_BOOT_TIME ")\n");
#ifdef CONFIG_T23
	/* The chip ID in the eFuse, as the T23 vendor SPL prints it */
	switch (readl((void __iomem *)0xb3540238) >> 16) {
	case 0x1111:
		puts("Board info: T23N\n");
		break;
	case 0x6666:
		puts("Board info: T23ZN\n");
		break;
	default:
		puts("Board info: No SOC Info\n");
		break;
	}
#endif
#ifdef CONFIG_T41
	/* The chip ID in the eFuse, as the T41 SDK's SPL prints it */
	switch (readl((void __iomem *)0xb3540250) >> 16) {
	case 0x1111:
		puts("Board info: T41N\n");
		break;
	case 0x3333:
		puts("Board info: T41L\n");
		break;
	case 0x5555:
		puts("Board info: T41ZL\n");
		break;
	case 0x7777:
		puts("Board info: T41ZN\n");
		break;
	case 0x6666:
		puts("Board info: T41ZX\n");
		break;
	case 0x4444:
		puts("Board info: T41A\n");
		break;
	case 0x9999:
		puts("Board info: T41LQ\n");
		break;
	case 0xAAAA:
		puts("Board info: T41NQ\n");
		break;
	default:
		puts("Board info: not support\n");
		break;
	}
#endif

	debug("Timer init\n");
	timer_init();

	debug("CLK stop\n");
	clk_prepare();

	debug("PLL init\n");
	pll_init();

	debug("CLK init\n");
	clk_init();

	debug("SDRAM init\n");
#ifdef CONFIG_T41
	t41_sdram_init();
#else
	sdram_init();
#endif
	debug("SDRAM init ok\n");

	/* The vendor board_init_r() starts the timer over */
	timer_init();

	ingenic_spl_boot_nor();
}
