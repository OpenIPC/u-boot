// SPDX-License-Identifier: GPL-2.0+
/*
 * T41: the DDR set-up with the vendor SPL's checks around it.
 *
 * Copyright (c) 2023 Ingenic Semiconductor Co.,Ltd
 */

#include <common.h>
#include <command.h>
#include <mach/ingenic.h>
#include <asm/jz_cache.h>

void sdram_init(int pll_sel);
void epc_debug(void);
int do_reset(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);

/*
 * The T41 vendor SPL's DDR checks (arch/mips/cpu/xburst2/t41/t41.c of the
 * T41 SDK): after each DDR set-up a write/read pass over the whole DDR,
 * uncached, every 0x104 bytes; while it fails, the set-up again with the
 * next PLL selection. Then a cache pass: 64 bytes written, written back,
 * read, invalidated and read again through L1 and L2; it fails, and the
 * SPL resets, when any of the eight odd words does not read back.
 */
static int ddr_mem_test12(void)
{
	int i = 0;
	int j = 0;
	int ret = 8;
	unsigned int addr;
	unsigned int value_get;
	unsigned int data;
	unsigned int test_size =  64;
	unsigned int test_data =  0xffffffff;

	for (i = 0; i < 1; i++) {
		addr = 0x81000000;
		for (j = 0; j < test_size; j += 4) {
			data = (j / 4) % 2 ? test_data : 0;
			*(volatile unsigned int *)(addr + j) = data;
		}

		/* flush cache */
		flush_dcache_range(addr, addr + test_size);
		flush_l2cache_range(addr, addr + test_size);

		/* read cached */
		addr = 0x81000000;
		for (j = 0; j < test_size; j += 4)
			value_get = *(volatile unsigned int *)(addr + j);

		/* invalid cache */
		addr = 0x81000000;
		invalid_dcache_range(addr, addr + test_size);
		invalid_l2cache_range(addr, addr + test_size);
		/* read cached */
		addr = 0x81000000;
		printf("mem data:\n");
		for (j = 0; j < test_size; j += 4) {
			value_get = *(volatile unsigned int *)(addr + j);
			if (j / 4 % 4 % 2)
				if (test_data == value_get)
					ret -= 1;
			if (0 == j / 4 % 4)
				printf("%x: ", addr + j);
			printf("  %x", value_get);
			if (3 == j / 4 % 4)
				printf("\n");
		}
	}
	return ret;
}

static int ddr_mem_test4(void)
{
	volatile u32 tmp = 0;
	u32 tmp_data = 0;

#ifdef CONFIG_DDR_TYPE_DDR2
	for (tmp = 0xa0000000; tmp < 0xa4000000; tmp += 0x104) {
#else
	for (tmp = 0xa0000000; tmp < 0xa8000000; tmp += 0x104) {
#endif
		u32 td = 0x5a5a5a5a;

		*(u32 *)tmp = td;
		tmp_data = *(u32 *)tmp;
		if (tmp_data != td) {
			printf("\n##### addr = %x, want = %x, get = %x\n",
			       tmp, td, tmp_data);
			return -1;
		}
	}
	return 0;
}

void t41_sdram_init(void)
{
	int pll_sel = 0;

	sdram_init(pll_sel);
	while (ddr_mem_test4()) {
		pll_sel++;
		sdram_init(pll_sel);
	}

	epc_debug();

	if (ddr_mem_test12()) {
		printf("ddr error!\n");
		do_reset(NULL, 0, 0, NULL);
	}
}
