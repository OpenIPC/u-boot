/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef __TIMER_H__
#define __TIMER_H__
enum time_unit { SEC = 1, USEC = 2, MSEC = 3 };
void timer_init(void);
u32 get_curr_time(enum time_unit unit);	/* counts down */
void mdelay(u32 ms);
void udelay(u32 us);
#endif
