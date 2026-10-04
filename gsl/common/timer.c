/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
 */
#include <platform.h>

#define TIMER_DIVIDER_MS  (TIMER_FEQ / TIMER_DIV / 1000)

unsigned long timer_get_divider(void)
{
	return TIMER_DIVIDER_MS;
}
