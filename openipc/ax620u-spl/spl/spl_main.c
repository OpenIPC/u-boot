// SPDX-License-Identifier: GPL-2.0+
/*
 * AX620U first stage: clocks, UART, DDR, SMP release, then U-Boot from
 * SPI NAND, entered at UBOOT_ENTRY in ARM state.
 */
#include "cmn.h"
#include "board.h"
#include "boot.h"
#include "ddr.h"
#include "uart.h"


static void __attribute__((noreturn)) jump_to_execute(u32 entry)
{
	__asm__ __volatile__(
		"mov	r3, %0\n\t"
		"mrs	r0, cpsr\n\t"
		"bic	r0, r0, #0x20\n\t"
		"msr	cpsr_fc, r0\n\t"
		"mov	r0, r3\n\t"
		"bx	r0\n\t"
		: : "r" (entry) : "r0", "r3", "memory");
	__builtin_unreachable();
}

int main(void)
{
	u32 strap = readl(CHIP_MODE_STATUS);
	int flash_type;
	int ret;

	timer_clk_init();
	clk_init();
	uart_init(UART0_BASE);
	ddr_init();
	wakeup_slave();
	chip_grade_init();
	if (strap & 8)
		flash_type = FLASH_SD;
	else
		flash_type = strap >> 1 & 3;
	setup_boot_mode(flash_type);
	ret = boot_from_flash(flash_type);
	if (ret >= 0)
		jump_to_execute(ret);
	chip_reset();
	while (1)
		;
}
