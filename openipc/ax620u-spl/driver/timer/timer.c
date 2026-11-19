// SPDX-License-Identifier: GPL-2.0+
/* 32-bit down counter at 24 MHz (timer 0 of the 0x049a0000 block). */
#include "cmn.h"
#include "timer.h"

#define TIMER_BASE	0x049a0000
#define TIMER_LOAD	(TIMER_BASE + 0x0)
#define TIMER_VALUE	(TIMER_BASE + 0x4)
#define TIMER_CTRL	(TIMER_BASE + 0x8)
#define TIMER_HZ	24000000

void timer_init(void)
{
	writel(readl(TIMER_CTRL) & ~1, TIMER_CTRL);
	writel(0xffffffff, TIMER_LOAD);
	writel(readl(TIMER_CTRL) | 1, TIMER_CTRL);
}

u32 get_curr_time(enum time_unit unit)
{
	u32 div;

	if (unit == USEC)
		div = TIMER_HZ / 1000000;
	else if (unit == MSEC)
		div = TIMER_HZ / 1000;
	else
		div = TIMER_HZ;
	return readl(TIMER_VALUE) / div;
}

void mdelay(u32 ms)
{
	u32 start = get_curr_time(MSEC);

	while (start - get_curr_time(MSEC) <= ms)
		;
}

void udelay(u32 us)
{
	u32 start = get_curr_time(USEC);

	while (start - get_curr_time(USEC) <= us)
		;
}
