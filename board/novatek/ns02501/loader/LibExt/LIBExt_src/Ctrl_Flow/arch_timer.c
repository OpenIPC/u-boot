#include "bl_func.h"
#include "arch_timer.h"


#define _THUMB2 __attribute__((target("thumb")))
#define isb() asm volatile("isb" : : : "memory")

#if (BOOT_LINUX_SUPPORT)

_THUMB2 unsigned int read_cntfrq(void)
{
	unsigned int freq;
	asm volatile("mrc p15, 0, %0, c14, c0, 0" : "=r"(freq));
	return freq;
}

_THUMB2 void write_cntfrq(unsigned int freq)
{
	asm volatile("mcr p15, 0, %0, c14, c0, 0" : : "r"(freq));
	return;
}

_THUMB2 unsigned int read_cntkctl(void)
{
	unsigned int cntkctl;
	asm volatile("mrc p15, 0, %0, c14, c1, 0" : "=r"(cntkctl));
	return cntkctl;
}

_THUMB2 void write_cntkctl(unsigned int cntkctl)
{
	asm volatile("mcr p15, 0, %0, c14, c1, 0" : : "r"(cntkctl));
	isb();
}

_THUMB2 unsigned long long read_cntvct(void)
{
	unsigned long long cval;
	unsigned int cval_high;
	unsigned int cval_low;

	asm volatile("mrrc p15, 3, %0, %1, c14" : "=r"(cval_low), "=r"(cval_high));

	cval = cval_high;
	cval <<= 32;
	cval |= cval_low;

	return cval;
}

_THUMB2 void write_cntv_cval(unsigned long long cval)
{
	unsigned int cval_high = (unsigned int)((cval) >> 32);
	unsigned int cval_low  = (unsigned int)((cval) & -1);

	asm volatile("mcrr p15, 3, %0, %1, c14" : : "r"(cval_low), "r"(cval_high));
}

_THUMB2 unsigned int read_cntv_ctl(void)
{
	unsigned int ctl;

	asm volatile("mrc p15, 0, %0, c14, c3, 1" : "=r"(ctl));

	return ctl;
}

_THUMB2 void write_cntv_ctl(unsigned int ctl)
{
	asm volatile("mcr p15, 0, %0, c14, c3, 1" : : "r"(ctl));
}

_THUMB2 unsigned long long read_cntpct(void)
{
	unsigned long long cval;
	unsigned int cval_high;
	unsigned int cval_low;

	asm volatile("mrrc p15, 0, %0, %1, c14" : "=r"(cval_low), "=r"(cval_high));

	cval = cval_high;
	cval <<= 32;
	cval |= cval_low;

	return cval;
}

_THUMB2 void write_cntp_cval(unsigned long long cval)
{
	unsigned int cval_high = (unsigned int)((cval) >> 32);
	unsigned int cval_low  = (unsigned int)((cval) & -1);

	asm volatile("mcrr p15, 2, %0, %1, c14" : : "r"(cval_low), "r"(cval_high));
}

_THUMB2 unsigned int read_cntp_ctl(void)
{
	unsigned int ctl;

	asm volatile("mrc p15, 0, %0, c14, c2, 1" : "=r"(ctl));

	return ctl;
}

_THUMB2 void write_cntp_ctl(unsigned int ctl)
{
	asm volatile("mcr p15, 0, %0, c14, c2, 1" : : "r"(ctl));
}

#endif // (BOOT_LINUX_SUPPORT)