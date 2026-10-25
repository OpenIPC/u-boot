// SPDX-License-Identifier: GPL-2.0+
/*
 * PSCI for the Allwinner V831/V833: one core, so no CPU_ON. What the
 * vendor kernels ask for is the version, SYSTEM_RESET (the watchdog) and,
 * for poweroff, SYSTEM_OFF (the core parks; there is no PMIC to cut).
 */
#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/timer.h>
#include <asm/arch/watchdog.h>
#include <asm/armv7.h>
#include <asm/io.h>
#include <asm/psci.h>
#include <asm/secure.h>
#include <asm/system.h>

u32 __secure psci_version(void)
{
	return ARM_PSCI_VER_1_0;
}

s32 __secure psci_features(u32 function_id, u32 psci_fid)
{
	switch (psci_fid) {
	case ARM_PSCI_0_2_FN_PSCI_VERSION:
	case ARM_PSCI_0_2_FN_CPU_OFF:
	case ARM_PSCI_0_2_FN_AFFINITY_INFO:
	case ARM_PSCI_0_2_FN_MIGRATE_INFO_TYPE:
	case ARM_PSCI_0_2_FN_SYSTEM_OFF:
	case ARM_PSCI_0_2_FN_SYSTEM_RESET:
		return 0x0;
	}
	return ARM_PSCI_RET_NI;
}

s32 __secure psci_affinity_info(u32 function_id, u32 target_affinity,
				u32 lowest_affinity_level)
{
	/* the one core, MPIDR affinity 0 */
	if (lowest_affinity_level > 0 || (target_affinity & 0xffffff))
		return ARM_PSCI_RET_INVAL;

	return 0;	/* ON */
}

u32 __secure psci_migrate_info_type(void)
{
	return 2;	/* no trusted OS */
}

s32 __secure psci_cpu_off(void)
{
	psci_cpu_off_common();

	while (1)
		wfi();
}

void __secure psci_system_reset(void)
{
	struct sunxi_wdog *wdog =
		((struct sunxi_timer_reg *)SUNXI_TIMER_BASE)->wdog;

	writel(WDT_CFG_RESET, &wdog->cfg);
	writel(WDT_MODE_EN, &wdog->mode);
	writel(WDT_CTRL_KEY | WDT_CTRL_RESTART, &wdog->ctl);

	while (1)
		wfi();
}

void __secure psci_system_off(void)
{
	while (1)
		wfi();
}

void __secure psci_arch_init(void)
{
}
