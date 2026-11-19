// SPDX-License-Identifier: GPL-2.0+
/* DesignWare 8250 UART, polled output. */
#include "cmn.h"
#include "timer.h"
#include "uart.h"

#define UART_THR	0x00
#define UART_DLL	0x00
#define UART_IER	0x04
#define UART_DLH	0x04
#define UART_FCR	0x08
#define UART_LCR	0x0c
#define UART_MCR	0x10
#define UART_LSR	0x14
#define LCR_DLAB	0x80
#define LSR_THRE	0x20
#define UART_DIVISOR	0x6c	/* 115200 Bd */

void uart_init(u32 base)
{
	u32 lcr;

	if (base == UART1_BASE) {
		writel(0x10083, 0x049bc0d0);	/* pinmux: UART1 TX/RX */
		writel(0x10083, 0x049bc0dc);
	}
	writel(0, base + UART_IER);
	writel(3, base + UART_MCR);
	writel(7, base + UART_FCR);
	writel(3, base + UART_LCR);		/* 8N1 */
	lcr = readl(base + UART_LCR);
	writel((lcr & ~LCR_DLAB) | LCR_DLAB, base + UART_LCR);
	writel(UART_DIVISOR, base + UART_DLL);
	writel(0, base + UART_DLH);
	writel(lcr & ~LCR_DLAB, base + UART_LCR);
}

static void uart_wait_tx(u32 base, u32 timeout_us)
{
	u32 start = get_curr_time(USEC);
	u32 lsr;

	do {
		lsr = readl(base + UART_LSR);
		if (lsr & LSR_THRE)
			return;
	} while (start - get_curr_time(USEC) <= timeout_us);
}

void uart_putc(char c)
{
	uart_wait_tx(UART0_BASE, 1000);
	writel(c, UART0_BASE + UART_THR);
}
