/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef __UART_H__
#define __UART_H__
#define UART0_BASE	0x04990000
#define UART1_BASE	0x04992000
void uart_init(u32 base);
void uart_putc(char c);
#endif
