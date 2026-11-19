// SPDX-License-Identifier: GPL-2.0+
#include <stdarg.h>
#include "cmn.h"
#include "uart.h"

u32 readl(u32 addr)
{
	return *(volatile u32 *)addr;
}

void writel(u32 val, u32 addr)
{
	*(volatile u32 *)addr = val;
}

void reg_field_write(u32 addr, u32 width, u32 shift, u32 val)
{
	u32 mask = (0xffffffffU >> (32 - (width + shift))) & (~0U << shift);

	writel((readl(addr) & ~mask) | (val << shift), addr);
}

void *memset(void *s, int c, size_t n)
{
	u8 *p = s;

	while (n--)
		*p++ = c;
	return s;
}

size_t strlen(const char *s)
{
	const char *p = s;

	while (*p)
		p++;
	return p - s;
}

/*
 * The vendor's small printf: %c %d %s %x (%X, %ld, %lx the same), width,
 * '0' and '-' flags, precision for %s. Hex digits are upper case.
 */
struct fmt_spec {
	int len;
	int width;
	int prec;
	char pad;
	int have_width;
	int left;
};

static void put_pad(int now, struct fmt_spec *f)
{
	int i;

	if (f->have_width && now && f->len < f->width)
		for (i = f->len; i < f->width; i++)
			uart_putc(f->pad);
}

static void put_str(const char *s, struct fmt_spec *f)
{
	f->len = strlen(s);
	put_pad(!f->left, f);
	while (*s && f->prec-- != 0)
		uart_putc(*s++);
	put_pad(f->left, f);
}

static void put_num(int v, unsigned int base, struct fmt_spec *f)
{
	static const char digits[] = "0123456789ABCDEF";
	char buf[32];
	char *p = buf;
	unsigned int n;
	int neg = 0;

	if (base == 10 && v < 0) {
		neg = 1;
		n = -v;
	} else {
		n = v;
	}
	do {
		*p++ = digits[n % base];
		n /= base;
	} while (n);
	if (neg)
		*p++ = '-';
	*p-- = 0;
	f->len = strlen(buf);
	put_pad(!f->left, f);
	while (p >= buf)
		uart_putc(*p--);
	put_pad(f->left, f);
}

static int get_int(const char **s)
{
	int n = 0;

	while (**s >= '0' && **s <= '9')
		n = n * 10 + (*(*s)++ - '0');
	return n;
}

void info(const char *fmt, ...)
{
	struct fmt_spec f;
	va_list ap;
	int dot;
	char c;

	va_start(ap, fmt);
	for (; *fmt; fmt++) {
		if (*fmt != '%') {
			uart_putc(*fmt);
			continue;
		}
		f.have_width = 0;
		f.left = 0;
		dot = 0;
		f.pad = ' ';
		f.prec = 0x7fff;
		for (;;) {
			c = *++fmt;
			if (c >= '0' && c <= '9') {
				if (!dot) {
					if (c == '0')
						f.pad = '0';
					f.width = get_int(&fmt);
					f.have_width = 1;
				} else {
					f.prec = get_int(&fmt);
				}
				fmt--;
				continue;
			}
			if (c >= 'A' && c <= 'Z')
				c += 0x20;
			if (c == '-') {
				f.left = 1;
				continue;
			}
			if (c == '.') {
				dot = 1;
				continue;
			}
			if (c == 'l')
				continue;
			break;
		}
		switch (c) {
		case '%':
			uart_putc('%');
			break;
		case 'c':
			uart_putc(va_arg(ap, int) & 0xff);
			break;
		case 'd':
			put_num(va_arg(ap, int), 10, &f);
			break;
		case 's':
			put_str(va_arg(ap, const char *), &f);
			break;
		case 'x':
			put_num(va_arg(ap, int), 16, &f);
			break;
		default:
			break;
		}
	}
	va_end(ap);
}

/* libgcc's division by zero hook: return 0, as the vendor's does. */
int __aeabi_idiv0(int r)
{
	(void)r;
	return 0;
}

long long __aeabi_ldiv0(long long r)
{
	(void)r;
	return 0;
}
