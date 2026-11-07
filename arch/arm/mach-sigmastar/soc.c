// SPDX-License-Identifier: GPL-2.0+
/*
 * SigmaStar: timer, reset, DRAM size, MIU pipe flush.
 * After the vendor U-Boot 2015.01 (arch/arm/cpu/armv7/<die>).
 */

#include <common.h>
#include <cpu_func.h>
#include <env.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/sigmastar.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * PIU timer 0, free running up to its 32-bit maximum at the crystal rate,
 * as the vendor U-Boot runs it.
 */
int timer_init(void)
{
	writew(0, SS_TIMER0_CTRL);
	writew(0xffff, SS_TIMER0_MAX_L);
	writew(0xffff, SS_TIMER0_MAX_H);
	writew(SS_TIMER_EN, SS_TIMER0_CTRL);
	gd->timebase_h = 0;
	gd->timebase_l = 0;
	return 0;
}

unsigned long notrace timer_read_counter(void)
{
	u16 hi, lo;

	/* The halves are not latched together */
	do {
		hi = readw(SS_TIMER0_CNT_H);
		lo = readw(SS_TIMER0_CNT_L);
	} while (hi != readw(SS_TIMER0_CNT_H));
	return hi << 16 | lo;
}

void reset_cpu(void)
{
	writew(0xff, SS_PM_RESET);
	writew(SS_PM_RESET_KEY, SS_PM_RESET);
	while (1)
		;
}

/*
 * The IPL has sized and trained the DDR in the package. On infinity3 and
 * infinity2m it wraps past its end, which get_ram_size() sees; the later
 * dies keep the size where their vendor U-Boot reads it.
 */
int dram_init(void)
{
#if defined(CONFIG_SS_INFINITY3) || defined(CONFIG_SS_INFINITY2M) || \
    defined(CONFIG_SS_IFADO) || defined(CONFIG_SS_IFLIEGEN)
	gd->ram_size = get_ram_size((long *)SS_DDR_BASE, PHYS_SDRAM_1_SIZE);
#else
	unsigned int log2 = SS_DDR_LOG2_MB;

	/*
	 * Iford's vendor U-Boot takes a fixed size and nothing documents the
	 * register there: trust it only when it reads as 64 MiB to 2 GiB
	 */
	if (IS_ENABLED(CONFIG_SS_IFORD) && (log2 < 6 || log2 > 11))
		gd->ram_size = get_ram_size((long *)SS_DDR_BASE, SZ_512M);
	else
		gd->ram_size = SZ_1M * (1UL << log2);
#endif
	return 0;
}

#ifdef CONFIG_SS_MMA_BOOTARGS
/*
 * U-Boot stays in the first 64 MiB, as the vendor's does
 * (CONFIG_UBOOT_RAM_SIZE).
 */
ulong board_get_usable_ram_top(ulong total_size)
{
#ifdef CONFIG_SS_IFLIEGEN
	/*
	 * Ifliegen's IPL leaves TF-A's SP_MIN, the kernel's PSCI, at
	 * 0x23FE0000: its SDK's U-Boot keeps to 48 MiB (CONFIG_SGS_RAM_SIZE)
	 */
	return min_t(ulong, gd->ram_top, SS_DDR_BASE + SZ_32M + SZ_16M);
#endif
	return min_t(ulong, gd->ram_top, SS_DDR_BASE + SZ_64M);
}
#endif

#ifdef CONFIG_SS_IFLIEGEN
/*
 * Before the console: UART0 out of the URDMA's DMA mode, as the SDK's
 * hal_uart_config() takes it, once the IPL's last bytes have left.
 */
int board_early_init_f(void)
{
	int spins = 1000000;

	if (!(readw(SS_URDMA_CTRL) & SS_URDMA_DMA_MODE))
		return 0;
	while (!(readw(SS_URDMA_TXFIFO_STS) & SS_URDMA_TXFIFO_EMPTY) && --spins)
		;
	clrbits_le16(SS_URDMA_CTRL, SS_URDMA_TX_DMA_EN | SS_URDMA_RX_DMA_EN);
	spins = 1000000;
	while ((readw(SS_URDMA_CTRL) & (SS_URDMA_TX_BUSY | SS_URDMA_RX_BUSY)) &&
	       --spins)
		;
	clrbits_le16(SS_URDMA_CTRL, SS_URDMA_DMA_MODE);
	clrbits_le16(SS_UART0_RX_OFF, SS_UART0_RX_OFF_BIT);
	return 0;
}
#endif

/* The bank of the FSP the flash is on */
unsigned int ss_fsp_bank(void)
{
#ifdef CONFIG_SS_IFORD_FAMILY
	return SS_BANK_FSP_NONPM;
#else
#ifdef CONFIG_SS_INFINITY6E
	if ((readw(SS_CHIP_BOND) & SS_CHIP_BOND_MASK) != SS_CHIP_BOND_QFN88)
		return SS_BANK_FSP_NONPM;
#endif
	return SS_BANK_FSP_PM;
#endif
}

#ifdef CONFIG_SS_MMA_BOOTARGS
/*
 * What the arguments LX_MEM and mma_heap take, by the DDR's size: the
 * values of OpenIPC's vendor U-Boot (OpenIPC/u-boot-sigmastar, board_late_init()
 * in arch/arm/cpu/armv7/<die>/chip.c), which saved environments rely on.
 * LX_MEM is the kernel's memory, a little under the DDR's size on most
 * dies; memsz the MMA heap inside it.
 */
struct ss_mem_row {
	u16 mib;
	const char *memlx;
	const char *memsz;
};

static const struct ss_mem_row ss_mem_table[] = {
#if defined(CONFIG_SS_INFINITY6)
	{ 256, "0xFFC6000", "0xA000000" },
	{ 128, "0x7FC6000", "0x4600000" },
	{ 0,   "0x3FC6000", "0x2000000" },
#elif defined(CONFIG_SS_INFINITY6E)
	{ 1024, "0x3FFE0000", "0x1FE9C000" },
	{ 512,  "0x1FFE0000", "0x0FE9C000" },
	{ 256,  "0xFFE0000",  "0x9E9C000" },
	{ 128,  "0x7FE0000",  "0x4E9C000" },
	{ 0,    "0x3FE0000",  "0x1E9C000" },
#elif defined(CONFIG_SS_IFORD)
	/* The Iford SDK's own images (set_config) by DDR size; 64 MiB has none */
	{ 512, "0x1FC00000", "0x10000000" },
	{ 256, "0x10000000", "0x7000000" },
	{ 128, "0x8000000",  "0x4000000" },
	{ 0,   "0x4000000",  "0x2000000" },
#elif defined(CONFIG_SS_IFLIEGEN)
	/* The SDK's set_config by package: QFN128 128 MiB, QFN88 64 MiB */
	{ 128, "0x7FE0000", "0x5000000" },
	{ 0,   "0x3FE0000", "0x2000000" },
#elif defined(CONFIG_SS_I6C_FAMILY) || defined(CONFIG_SS_INFINITY2M) || \
      defined(CONFIG_SS_IFADO)
	/* infinity2m has no OpenIPC values; its kernel wants 1 MiB alignment */
	{ 256, "0x10000000", "0x0A000000" },
	{ 128, "0x8000000",  "0x4600000" },
	{ 0,   "0x4000000",  "0x2000000" },
#else	/* infinity6b0 */
	{ 256, "0xFFE0000", "0xA000000" },
	{ 128, "0x7FE0000", "0x4600000" },
	{ 0,   "0x3FE0000", "0x2000000" },
#endif
};

void ss_set_mem_env(void)
{
	unsigned int mib = gd->ram_size / SZ_1M;
	const struct ss_mem_row *row = ss_mem_table;

	while (row->mib && row->mib != mib)
		row++;
	env_set("memlx", row->memlx);
	env_set("memsz", row->memsz);
}
#endif

/*
 * The MIU buffers writes: what the CPU stored is in DDR, for a DMA engine
 * to read, once the L3 bridge's flush reports done.
 */
void ss_miu_flush(void)
{
#ifdef CONFIG_SS_IFORD_FAMILY
	/* No MIU pipe to flush, as the vendor's chip.c says */
	return;
#endif
	int spins = 1000000;

	writew(0x10, SS_L3_FLUSH);
	writew(0x11, SS_L3_FLUSH);
	while (!(readw(SS_L3_FLUSH_STATUS) & SS_L3_FLUSH_DONE) && --spins)
		;
}

#ifdef CONFIG_DISPLAY_CPUINFO
int print_cpuinfo(void)
{
	printf("CPU:   SigmaStar %s (chip id %02x)\n", CONFIG_PRODUCT_SOC,
	       readw(SS_CHIP_ID) & 0xff);
	return 0;
}
#endif
