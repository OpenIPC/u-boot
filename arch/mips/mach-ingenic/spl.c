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
#include <asm/sections.h>
#include <asm/gpio.h>
#include <asm/arch/clk.h>
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
void sdram_init(void);
void sfc_nor_load(unsigned int src_addr, unsigned int count,
		  unsigned int dst_addr);
void flush_cache_all(void);

static void __noreturn ingenic_spl_boot_nor(void)
{
	struct image_header *hdr = (void *)INGENIC_DECMP_BUFFER;
	typedef void __noreturn (*entry_t)(void);
	size_t len = INGENIC_UNPACK_MAX;
	ulong size, load, ep;
	int ret;

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
	printf("image entry point: 0x%lX\n", ep);
	((entry_t)ep)();
}

void board_init_f(ulong dummy)
{
	/*
	 * The vendor SPL cleared the BSS only after the DDR, but the serial
	 * core reads its state there from serial_init() on. It is cache RAM
	 * like the rest of the SPL, so clear it first.
	 */
	memset(__bss_start, 0, (char *)&__bss_end - __bss_start);

	gd = &gdata;
	gd->arch.gi = &ginfo;

	gpio_init();

	/* Init uart first */
	enable_uart_clk();

	gd->baudrate = CONFIG_BAUDRATE;
	serial_init();
	gd->have_console = 1;
	puts("\nU-Boot SPL " PLAIN_VERSION " (" U_BOOT_DATE " - "
	     U_BOOT_TIME ")\n");

	debug("Timer init\n");
	timer_init();

	debug("CLK stop\n");
	clk_prepare();

	debug("PLL init\n");
	pll_init();

	debug("CLK init\n");
	clk_init();

	debug("SDRAM init\n");
	sdram_init();
	debug("SDRAM init ok\n");

	/* The vendor board_init_r() starts the timer over */
	timer_init();

	ingenic_spl_boot_nor();
}
