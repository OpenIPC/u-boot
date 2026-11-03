#ifndef _ADD_TIME_H
#define _ADD_TIME_H
#include "rtc.h"

extern int time_util_add_sec(struct rtc_time *p_out, struct rtc_time *p_in, int sec);
extern int time_util_diff_sec(struct rtc_time *p_t2, struct rtc_time *p_t1);

#endif