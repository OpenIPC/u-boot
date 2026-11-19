// SPDX-License-Identifier: GPL-2.0+
/*
 * AX620U clocks, PLLs, core voltage, pads and SMP release, as the vendor
 * first stage sets them up before the DDR.
 */
#include "cmn.h"
#include "board.h"
#include "efuse.h"
#include "timer.h"

#define COMMON_CLK_BASE		0x049ba000	/* peripheral clock gates */
#define CPU_CLK_MUX		0x01900000
#define CPU_SYS_CTRL		0x01900014
#define PLL_BASE		0x02210000
#define PLL_STATUS		(PLL_BASE + 0x18)
#define PWM_BASE		0x049a2000
#define TSENSOR_BASE		0x02000000

volatile int pen_release[4] __attribute__((section(".pen")));

static u32 cpu_clk_inited;

int is_secure_boot(void)
{
	return (readl(CHIP_SECURE_STATUS) & SECURE_BOOT_EN) != 0;
}

void timer_clk_init(void)
{
	writel(readl(COMMON_CLK_BASE) | 0x1000, COMMON_CLK_BASE);
	writel(readl(COMMON_CLK_BASE + 0x8) | 0x20, COMMON_CLK_BASE + 0x8);
	writel(6, 0x049a0008);
	timer_init();
}

/* Core supply: PWM duty cycle in percent. */
static void core_voltage_set(int duty)
{
	writel(readl(COMMON_CLK_BASE) | 4, COMMON_CLK_BASE);
	writel(0x1a, PWM_BASE + 0x44);
	writel(100 - duty, PWM_BASE + 0x3c);
	writel(duty, PWM_BASE + 0xbc);
	writel(0x1f, PWM_BASE + 0x44);
	udelay(500);
	writel(0x30014, 0x049bc0fc);		/* pinmux: PWM out */
	mdelay(3);
}

static void npu_voltage_set(int v)
{
	(void)v;
}

/* CPU PLL: the vendor's field arguments, kept as they are. */
static void cpll_set(void)
{
	reg_field_write(PLL_BASE, 1, 5, 0);
	reg_field_write(PLL_BASE + 0x1c, 0x16, 0, 1);
	reg_field_write(PLL_BASE + 0x1c, 0x14, 0, 0);
	reg_field_write(PLL_BASE + 0xc0, 9, 0, 200);
	reg_field_write(PLL_BASE + 0x1c, 0x16, 0, 1);
	reg_field_write(PLL_BASE + 0x1c, 0x14, 0, 1);
	reg_field_write(PLL_BASE, 1, 5, 1);
}

static void cpll_wait_lock(void)
{
	int n = 0x10000;

	do {
		n--;
		if (readl(PLL_STATUS) & 0x20)
			return;
	} while (n);
}

static void cpll_init(void)
{
	cpll_set();
	cpll_wait_lock();
}

static int cpu_clk_src_pll(void)
{
	writel(readl(CPU_CLK_MUX) & 0xfffffff1, CPU_CLK_MUX);
	cpll_init();
	return 2;
}

static int cpu_bus_clk_init(void)
{
	u32 v, sel;

	if (!cpu_clk_inited) {
		v = readl(CPU_CLK_MUX);
		sel = cpu_clk_src_pll();
		writel((v & 0xffffffc0) | 1 | sel | 0x30, CPU_CLK_MUX);
		writel((readl(COMMON_CLK_BASE) & 0xfffffffc) | 0x40000007, COMMON_CLK_BASE);
		writel(readl(0x02220060) | 0x31ff, 0x02220060);
		writel((readl(0x10030000) & 0xffffff93) | 0x2b, 0x10030000);
		cpu_clk_inited = 1;
		writel((readl(COMMON_CLK_BASE) & 0xffffffe7) | 0x18, COMMON_CLK_BASE);
		writel((readl(COMMON_CLK_BASE) & 0xffffff9f) | 0x60, COMMON_CLK_BASE);
		writel(readl(COMMON_CLK_BASE) | 0x6000000, COMMON_CLK_BASE);
	}
	return 0;
}

static void npll_set(void)
{
	reg_field_write(PLL_BASE, 1, 2, 0);
	reg_field_write(PLL_BASE + 0x1c, 10, 0, 1);
	reg_field_write(PLL_BASE + 0x1c, 8, 0, 0);
	reg_field_write(PLL_BASE + 0xc4, 9, 0, 0x66);
	reg_field_write(PLL_BASE + 0xb4, 0x18, 0, 0x666666);
	reg_field_write(PLL_BASE + 0x1c, 10, 0, 1);
	reg_field_write(PLL_BASE + 0x1c, 8, 0, 1);
	reg_field_write(PLL_BASE, 1, 2, 1);
}

static void npll_wait_lock(void)
{
	int n = 0x10000;

	do {
		n--;
		if (readl(PLL_STATUS) & 4)
			return;
	} while (n);
}

static void npll_init(void)
{
	npll_set();
	npll_wait_lock();
}

static void pad_ctrl_extra(void)
{
	writel(readl(0x049a8008) & 0xffffffc1, 0x049a8008);
	writel(readl(0x049a8004) | 0x3e, 0x049a8004);
	writel(readl(0x049a8000) & 0xffffffc1, 0x049a8000);
}

static int pad_drive_set(int strong)
{
	static const u32 pads[] = {
		0x049bc630, 0x049bc648, 0x049bc63c, 0x049bc618, 0x049bc60c,
	};
	u32 v;
	int i;

	for (i = 0; i < ARRAY_SIZE(pads); i++) {
		v = readl(pads[i]) & 0xfff8ffff;
		if (strong == 1)
			v |= 0x60000;
		writel(v, pads[i]);
	}
	if (strong == 1)
		pad_ctrl_extra();
	return 0;
}

/* eMMC/SD clock: off (0) or on. */
void emmc_clk_set(int on)
{
	if (!on) {
		writel(readl(COMMON_CLK_BASE + 0x8) & 0xfff7ffff, COMMON_CLK_BASE + 0x8);
		writel(0xee, 0x049aa004);
	} else {
		writel(readl(COMMON_CLK_BASE + 0x8) | 0x80000, COMMON_CLK_BASE + 0x8);
	}
}

static void crypto_clk_off(void)
{
	writel(readl(COMMON_CLK_BASE + 0x20) & 0xfffffffe, COMMON_CLK_BASE + 0x20);
	writel(readl(COMMON_CLK_BASE + 0x20) & 0xfffffff1, COMMON_CLK_BASE + 0x20);
	writel(readl(COMMON_CLK_BASE + 0x8) & 0xffffff7f, COMMON_CLK_BASE + 0x8);
	writel(readl(COMMON_CLK_BASE + 0xc) & 0xffffffef, COMMON_CLK_BASE + 0xc);
}

/* Thermal sensor, trimmed from the eFuse; its reading grades the part. */
static int tsensor_init(void)
{
	u32 v, sel, trim = 0;

	if (efuse_ready())
		return -1;
	if (efuse_read(3, &v))
		return -1;
	sel = ((int)v >> 20) & 0xf;
	if (sel < 7) {
		if (sel == 0) {
			if (efuse_read(2, &v))
				return -1;
			if (v == 0 && efuse_read(0x42, &v))
				return -1;
			sel = v & 0xf;
			trim = ((int)v >> 4) & 0x3ff;
		}
	} else {
		sel = v & 0xf;
		trim = ((int)v >> 4) & 0x3ff;
	}
	writel(0xb, TSENSOR_BASE + 0x4);
	writel(0xe, TSENSOR_BASE + 0x0);
	writel(1, TSENSOR_BASE + 0xc);
	writel(sel << 1 | (readl(TSENSOR_BASE + 0x8) & 0xffffffe1), TSENSOR_BASE + 0x8);
	writel(trim | (readl(TSENSOR_BASE + 0x10) & 0xfffffc00), TSENSOR_BASE + 0x10);
	writel(0x1e00, TSENSOR_BASE + 0x20);
	readl(TSENSOR_BASE + 0x20);
	mdelay(1);
	return 1;
}

static u32 tsensor_read(u32 reg)
{
	tsensor_init();
	return readl(reg);
}

static char chip_grade(void)
{
	int t = tsensor_read(TSENSOR_BASE + 0x28);
	int q;

	if (t >= 0 && t <= 0x20)
		return '0';
	if (t >= 0x3e0 && t <= 0x3ff)
		return 'G';
	q = (t - 0x20) / 64;
	if (q + 1 < 10)
		return '1' + q;
	return '8' + q;
}

static void chip_grade_tune(void)
{
	if (chip_grade() == '2')
		writel(readl(COMMON_CLK_BASE + 0x140) | 1, COMMON_CLK_BASE + 0x140);
}

void chip_grade_init(void)
{
	writel(chip_grade(), CHIP_CHIP_GRADE);
}

int clk_init(void)
{
	pad_drive_set(1);
	core_voltage_set(60);
	npu_voltage_set(0x2b);
	emmc_clk_set(0);
	cpu_bus_clk_init();
	writel(0x80, 0x02220080);
	writel(readl(0x02220170) | 0x1d, 0x02220170);
	writel(readl(CHIP_SW_RESET) | 0x80, CHIP_SW_RESET);
	if (!is_secure_boot())
		crypto_clk_off();
	npll_init();
	chip_grade_tune();
	return 0;
}

/* Release cores 1-3 into start.S and wait until each has checked in. */
static void wakeup_secondary(void)
{
	int woken = 0;
	u32 start;

	writel(0x800, SMP_BOOT_ADDR);
	__asm__ __volatile__("isb" ::: "memory");
	start = get_curr_time(MSEC);
	while (pen_release[1] != -1 || pen_release[2] != -1 || pen_release[3] != -1) {
		if (start - get_curr_time(MSEC) > 49)
			goto out;
	}
	woken = 1;
out:
	if (!woken) {
		info("wake secondary cpus fail state:%d, %d, %d\r\n",
		     pen_release[1], pen_release[2], pen_release[3]);
		while (1)
			;
	}
	writel(0, SMP_BOOT_ADDR);
}

void wakeup_slave(void)
{
	u32 v;
	int i;

	wakeup_secondary();
	v = (readl(CPU_SYS_CTRL) & 0xffff00ff) | 0x400 | 1;
	writel(v, CPU_SYS_CTRL);
	for (i = 0; i < 10; i++)
		__asm__ __volatile__("wfe");
	writel(v & ~1, CPU_SYS_CTRL);
}

struct boot_mode_info {
	u32 magic;
	u32 mode;
	u32 reserved;
	u32 storage_sel;
};

#define BOOT_MODE_INFO_ADDR	0x0
#define BOOT_MODE_ENV_MAGIC	0x12345678
#define BOOT_MODE_NORMALBOOT	2

void setup_boot_mode(int flash_type)
{
	struct boot_mode_info *boot_mode = (struct boot_mode_info *)BOOT_MODE_INFO_ADDR;

	memset(boot_mode, 0, sizeof(boot_mode));	/* sic: 4 bytes */
	boot_mode->magic = BOOT_MODE_ENV_MAGIC;
	boot_mode->mode = BOOT_MODE_NORMALBOOT;
	switch (flash_type) {
	case FLASH_NAND:
		boot_mode->storage_sel = 0x11;
		info("The system boot form NAND\r\n");
		break;
	case FLASH_NOR:
		boot_mode->storage_sel = 0x10;
		info("The system boot form NOR\r\n");
		break;
	case FLASH_NAND_2:
		boot_mode->storage_sel = 0x11;
		info("The system boot form NAND\r\n");
		break;
	case FLASH_EMMC:
		boot_mode->storage_sel = 0x12;
		info("The system boot form EMMC\r\n");
		break;
	case FLASH_SD:
		boot_mode->storage_sel = 0x13;
		info("The system boot form SD\r\n");
		break;
	default:
		return;
	}
	info("enter boot normal mode\r\n");
}

void chip_reset(void)
{
	u32 v = readl(CHIP_SW_RESET);

	info("Set REG 0x%X, value 0x%X\n", CHIP_SW_RESET, v | 1);
	writel(v | 1, CHIP_SW_RESET);
}
