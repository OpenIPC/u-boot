#include "time_util.h"
#include "string.h"

#define _THUMB2 __attribute__((target("thumb")))
#define IS_LEAP_YEAR(year) ((((year % 4) == 0) && (((year % 100) != 0) || ((year % 400) == 0))))

_THUMB2 static int calc_month_day(int year, int month)
{
	int day = 31;
	int year_ext = 0;
	if (IS_LEAP_YEAR(year)) {
		year_ext = 1;
	}
	if ((month == 4) || (month == 6) || (month == 9) || (month == 11)) {
		day = 30;
	}
	if (month == 2) {
		day = 28 + year_ext;
	}
	return day;
}

_THUMB2 int time_util_add_sec(struct rtc_time *p_out, struct rtc_time *p_in, int sec)
{
	int maxday;
	utl_memcpy(p_out, p_in, sizeof(struct rtc_time));

	p_out->tm_sec += sec;   /* seconds after the minute - [0,59] */

	if (p_out->tm_sec >= 60) {
		p_out->tm_min += p_out->tm_sec / 60;
		p_out->tm_sec = p_out->tm_sec % 60;
	}
	if (p_out->tm_min >= 60) {
		p_out->tm_hour += p_out->tm_min / 60;
		p_out->tm_min = p_out->tm_min % 60;
	}
	if (p_out->tm_hour >= 24) {
		p_out->tm_mday += p_out->tm_hour / 24;
		p_out->tm_hour = p_out->tm_hour % 24;
	}
	maxday = calc_month_day(p_out->tm_year, p_out->tm_mon);
	while (p_out->tm_mday > maxday) {
		p_out->tm_mday -= maxday;
		p_out->tm_mon ++;
		//month range = [1,12]
		if (p_out->tm_mon > 12) {
			p_out->tm_year += (p_out->tm_mon - 1) / 12;
			p_out->tm_mon = ((p_out->tm_mon - 1) % 12) + 1;
		}
		maxday = calc_month_day(p_out->tm_year, p_out->tm_mon);
	}
	//month range = [1,12]
	if (p_out->tm_mon > 12) {
		p_out->tm_year += (p_out->tm_mon - 1) / 12;
		p_out->tm_mon = ((p_out->tm_mon - 1) % 12) + 1;
	}
	return 0;
}

_THUMB2 int time_util_diff_sec(struct rtc_time *p_t2, struct rtc_time *p_t1)
{
	int day2 = calc_month_day(p_t2->tm_year, p_t2->tm_mon) + p_t2->tm_mday;
	int day1 = calc_month_day(p_t1->tm_year, p_t1->tm_mon) + p_t1->tm_mday;
	int sec2 = p_t2->tm_hour*60*60 + p_t2->tm_min*60 + p_t2->tm_sec;
	int sec1 = p_t1->tm_hour*60*60 + p_t1->tm_min*60 + p_t1->tm_sec;
	int diff = (day2-day1)*24*60*60 + (sec2-sec1);
	return diff;
}