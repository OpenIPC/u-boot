// SPDX-License-Identifier: GPL-2.0+
/*
 * OpenIPC board file for AX620E cameras and AX630C boards: the SD slot's
 * card clock and pads (and the eMMC's card clock) from board_init(), the
 * OpenIPC boot layer from misc_init_r(). Other pads stay as the first
 * stage and the board's own kernel set them; UART0, the boot SPI flash
 * and the Ethernet MAC's own 10/100 PHY need none of their own here.
 */

#include <common.h>
#include <env.h>
#include <fdtdec.h>
#include <init.h>
#include <openipc.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/system.h>
#include <linux/kernel.h>
#include <linux/libfdt.h>
#include <linux/sizes.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <mach/ax620e.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_MMC_SDHCI_CADENCE
/* FLASH_SYS_GLB's set and clear aliases */
#define AX_FLASH_SET(reg)	(AX_FLASH_SYS_GLB + 0x4000 + (reg))
#define AX_FLASH_CLR(reg)	(AX_FLASH_SYS_GLB + 0x8000 + (reg))
#define AX_FLASH_CLK_MUX0	0x00
#define  AX_SD_CARD_SEL_MASK	(3 << 16)
#define  AX_SD_CARD_SEL_NPLL400	(3 << 16)
#define AX_FLASH_CLK_EB0	0x04
#define  AX_SD_CARD_EB		BIT(9)
#define AX_FLASH_CLK_DIV0	0x0c
#define  AX_SD_CARD_DIV_MASK	(0x3f << 20)
#define  AX_SD_CARD_DIV_2	(1 << 20)
#define  AX_SD_CARD_DIV_UP	BIT(26)

/* The SD pads' group (PIN_MUX_G9) */
#define AX_PINMUX_G9		0x104f1000UL
#define AX_PINMUX_G9_CTRL_SET	(AX_PINMUX_G9 + 0x04)
#define AX_PINMUX_G9_CTRL_CLR	(AX_PINMUX_G9 + 0x08)
/* Set: the SD pads at 1.8 V */
#define  AX_SD_PADS_1V8		GENMASK(8, 7)
#define AX_PINMUX_G9_VDET	(AX_PINMUX_G9 + 0x58)
#define  AX_SD_VDET_3V3		BIT(0)
#define AX_PAD_PULL_MASK	GENMASK(7, 6)
#define AX_PAD_PULL_UP		(2 << 6)

/*
 * As the vendor SDK's U-Boot does for the slot (drivers/mmc/
 * sdhci_ax620e.c, the controller at 0x104e0000): the card clock from
 * NPLL 400 MHz divided by 2, the 200 MHz the controller then divides;
 * pull-ups on CMD and DATA0-3; the pads' IO voltage to what the
 * detector reports (the slot's supply).
 */
static void ax620e_sd_init(void)
{
	static const u16 pads[] = { 0x30, 0x0c, 0x18, 0x3c, 0x48 };
	int i;

	writel(AX_SD_CARD_EB, AX_FLASH_CLR(AX_FLASH_CLK_EB0));
	udelay(2);
	writel(AX_SD_CARD_SEL_MASK, AX_FLASH_CLR(AX_FLASH_CLK_MUX0));
	writel(AX_SD_CARD_SEL_NPLL400, AX_FLASH_SET(AX_FLASH_CLK_MUX0));
	writel(AX_SD_CARD_EB, AX_FLASH_SET(AX_FLASH_CLK_EB0));
	writel(AX_SD_CARD_DIV_MASK, AX_FLASH_CLR(AX_FLASH_CLK_DIV0));
	writel(AX_SD_CARD_DIV_2, AX_FLASH_SET(AX_FLASH_CLK_DIV0));
	writel(AX_SD_CARD_DIV_UP, AX_FLASH_SET(AX_FLASH_CLK_DIV0));
	udelay(2);
	writel(AX_SD_CARD_DIV_UP, AX_FLASH_CLR(AX_FLASH_CLK_DIV0));

	for (i = 0; i < ARRAY_SIZE(pads); i++)
		clrsetbits_le32(AX_PINMUX_G9 + pads[i], AX_PAD_PULL_MASK,
				AX_PAD_PULL_UP);

	if (readl(AX_PINMUX_G9_VDET) & AX_SD_VDET_3V3)
		writel(AX_SD_PADS_1V8, AX_PINMUX_G9_CTRL_CLR);
	else
		writel(AX_SD_PADS_1V8, AX_PINMUX_G9_CTRL_SET);
}
#endif

#ifdef CONFIG_AXERA_AX630C_EMMC
#define AX_EMMC_CARD_SEL_MASK	(3 << 5)
#define AX_EMMC_CARD_SEL_NPLL400 (3 << 5)
#define AX_EMMC_CARD_EB		BIT(2)
#define AX_EMMC_CARD_DIV_MASK	GENMASK(5, 0)
#define AX_EMMC_CARD_DIV_2	1
#define AX_EMMC_CARD_DIV_UP	BIT(6)

/*
 * The eMMC's card clock (CPU_SYS_GLB), as the SDK U-Boot's sdhci_ax620e.c
 * sets it for the controller at 0x1b40000: NPLL 400 MHz divided by 2.
 */
static void ax630c_emmc_init(void)
{
	writel(AX_EMMC_CARD_EB, AX_CPU_SYS_GLB_CLK_EB0_CLR);
	udelay(2);
	writel(AX_EMMC_CARD_SEL_MASK, AX_CPU_SYS_GLB_CLK_MUX0_CLR);
	writel(AX_EMMC_CARD_SEL_NPLL400, AX_CPU_SYS_GLB_CLK_MUX0_SET);
	writel(AX_EMMC_CARD_EB, AX_CPU_SYS_GLB_CLK_EB0_SET);
	writel(AX_EMMC_CARD_DIV_MASK, AX_CPU_SYS_GLB_CLK_DIV0_CLR);
	writel(AX_EMMC_CARD_DIV_2, AX_CPU_SYS_GLB_CLK_DIV0_SET);
	writel(AX_EMMC_CARD_DIV_UP, AX_CPU_SYS_GLB_CLK_DIV0_SET);
	udelay(2);
	writel(AX_EMMC_CARD_DIV_UP, AX_CPU_SYS_GLB_CLK_DIV0_CLR);
}

/*
 * The kernel's mem= when the environment has none: a quarter of the
 * DRAM, at least 128 MiB, at most 1 GiB (256 MiB on the 1 GiB board, 1
 * GiB on the 4 GiB one; the Sipeed SDK gives 512 MiB and 1 GiB). The rest,
 * from the end of it, is the vendor media drivers' CMM pool (load_axera
 * sizes it from totalmem and mem=).
 */
static void ax630c_osmem(void)
{
	ulong mib = clamp((ulong)(gd->ram_size / SZ_1M / 4), 128UL, 1024UL);
	char buf[16];

	if (env_get("osmem"))
		return;
	snprintf(buf, sizeof(buf), "%luM", mib);
	env_set("osmem", buf);
}

/*
 * Firmware below U-Boot: the board's own boot chain has its ATF (PSCI)
 * and OP-TEE resident, which an OpenIPC kernel's device tree (no ATF:
 * spin-table CPUs, as the OpenIPC first stage starts them) does not know
 * about. Entered in EL2, U-Boot was started from that chain; psci=1 or
 * psci=0 in the environment says so instead.
 */
static bool ax630c_has_psci(void)
{
	const char *p = env_get("psci");

	if (p)
		return *p == '1' || *p == 'y';
	return current_el() > 1;
}

/* The board's ATF and OP-TEE (the Sipeed SDK's partition_ab.mak:
 * ATF_IMG_ADDR, ATF_IMG_PKG_SIZE, OPTEE_IMAGE_ADDR, OPTEE_RESERVED_SIZE) */
static const struct fdt_memory ax630c_secure[] = {
	{ .start = 0x40040000, .end = 0x4007ffff },
	{ .start = 0x44200000, .end = 0x461fffff },
};

/*
 * With that firmware there, the kernel brings up its second CPU by PSCI
 * and keeps out of the ATF's and OP-TEE's memory: /psci, the CPUs'
 * enable-method, and two no-map reserved-memory nodes.
 */
int ft_board_setup(void *blob, struct bd_info *bd)
{
	int cpus, cpu, off, i;

	if (!ax630c_has_psci())
		return 0;
	printf("PSCI:  the board's ATF; secure memory reserved\n");

	off = fdt_path_offset(blob, "/psci");
	if (off < 0)
		off = fdt_add_subnode(blob, 0, "psci");
	if (off < 0 ||
	    fdt_setprop_string(blob, off, "compatible", "arm,psci-1.0") ||
	    fdt_setprop_string(blob, off, "method", "smc") ||
	    fdt_setprop_string(blob, off, "status", "okay"))
		return -1;

	cpus = fdt_path_offset(blob, "/cpus");
	if (cpus >= 0) {
		fdt_for_each_subnode(cpu, blob, cpus) {
			const char *type = fdt_getprop(blob, cpu, "device_type",
						       NULL);

			if (!type || strcmp(type, "cpu"))
				continue;
			fdt_setprop_string(blob, cpu, "enable-method", "psci");
			fdt_delprop(blob, cpu, "cpu-release-addr");
		}
	}

	for (i = 0; i < ARRAY_SIZE(ax630c_secure); i++)
		if (fdtdec_add_reserved_memory(blob, i ? "optee" : "atf",
					       &ax630c_secure[i], NULL, 0, NULL,
					       FDTDEC_RESERVED_MEMORY_NO_MAP) < 0)
			return -1;
	return 0;
}
#endif

int board_init(void)
{
#ifdef CONFIG_MMC_SDHCI_CADENCE
	ax620e_sd_init();
#endif
#ifdef CONFIG_AXERA_AX630C_EMMC
	ax630c_emmc_init();
#endif
	return 0;
}

int misc_init_r(void)
{
#ifdef CONFIG_AXERA_AX630C_EMMC
	ax630c_osmem();
#endif
	return openipc_helper();
}
