// SPDX-License-Identifier: GPL-2.0+
/*
 * Anyka AK39 UART0, non-DM.
 *
 * Not a 16550: the UART has no data register. Its TX and RX buffers are
 * fixed areas of L2 memory (0x48001000 / 0x48001080 for UART0); the
 * registers start a send and say where the next received byte will land in
 * the 128-byte RX ring. After the vendor U-Boot's serial_ak39.c, except
 * that tstc() compares that position with the next byte to read, as getc()
 * does, rather than looking at the RX threshold flag, which says only that
 * something arrived since the flag was last cleared.
 */

#include <common.h>
#include <serial.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/anyka.h>
#include <linux/compiler.h>

#define UART_CONF1		(AK_UART0_BASE + 0x00)
#define UART_CONF2		(AK_UART0_BASE + 0x04)
#define UART_DATA		(AK_UART0_BASE + 0x08)
#define UART_THRESH		(AK_UART0_BASE + 0x0c)
#define UART_TIMEOUT		(AK_UART0_BASE + 0x18)

/* CONF1 */
#define CONF1_ENABLE		BIT(21)
#define CONF1_TX_CLR		BIT(28)
#define CONF1_RX_CLR		BIT(29)
/* CONF2 */
#define CONF2_TX_CNT(n)		((n) << 4)
#define CONF2_TX_GO		BIT(16)
#define CONF2_TX_DONE		BIT(19)
/* DATA: where the next received byte goes */
#define DATA_RX_POS(v)		(((v) >> 13) & 0x7f)
/* THRESH */
#define THRESH_RX_MASK		0x3f
#define THRESH_RX_ON		BIT(31)

DECLARE_GLOBAL_DATA_PTR;

/*
 * The next RX ring byte to read, kept modulo the ring size. Initialised so
 * that it lives in .data: U-Boot runs from DDR before relocation, where
 * .data is writable, but .bss is not cleared until after it.
 */
static unsigned int rx_pos = AK_UART_RXBUF_SIZE;

/* Also restarts the RX ring at 0 */
static void ak_serial_setbrg(void)
{
	unsigned long clk = ak_asic_clk();
	u32 div = clk ? clk / gd->baudrate - 1 : 0;

	writel(CONF1_RX_CLR | CONF1_TX_CLR | CONF1_ENABLE | (div & 0xffff),
	       UART_CONF1);
	rx_pos = 0;
}

static int ak_serial_init(void)
{
	clrbits_le32(AK_SYS_CLK_GATE, AK_CLK_UART0);
	setbits_le32(AK_SYS_SHAREPIN0, AK_SHAREPIN0_UART0);
	clrbits_le32(AK_SYS_PULL0, AK_PULL0_UART0);
	/* The vendor's L2 set-up for the UART buffers */
	setbits_le32(AK_L2_FRAC_ADDR, 0x3 << 28);
	setbits_le32(AK_L2_CONBUF8_15, AK_L2_UART0_TX_CLR | AK_L2_UART0_RX_CLR);

	ak_serial_setbrg();
	/* RX threshold 1, timeout as the vendor's, then receive */
	clrsetbits_le32(UART_THRESH, THRESH_RX_MASK, 1);
	writel(0x1f << 16 | 1, UART_TIMEOUT);
	setbits_le32(UART_THRESH, THRESH_RX_ON);
	return 0;
}

static void ak_serial_putc(const char c)
{
	if (c == '\n')
		ak_serial_putc('\r');

	setbits_le32(AK_L2_CONBUF8_15, AK_L2_UART0_TX_CLR);
	writel(c, AK_UART0_TXBUF);
	/* The last word of the 64-byte block hands it to the UART */
	writel(0, AK_UART0_TXBUF + 0x3c);
	setbits_le32(UART_CONF1, CONF1_TX_CLR);
	setbits_le32(UART_CONF2, CONF2_TX_CNT(1) | CONF2_TX_GO);
	while (!(readl(UART_CONF2) & CONF2_TX_DONE))
		;
}

static int ak_serial_tstc(void)
{
	return DATA_RX_POS(readl(UART_DATA)) != rx_pos % AK_UART_RXBUF_SIZE;
}

static int ak_serial_getc(void)
{
	unsigned int pos;

	while (!ak_serial_tstc())
		;
	pos = rx_pos % AK_UART_RXBUF_SIZE;
	rx_pos = pos + 1;
	return readb(AK_UART0_RXBUF + pos);
}

static struct serial_device ak_serial_device = {
	.name	= "ak_serial",
	.start	= ak_serial_init,
	.setbrg	= ak_serial_setbrg,
	.getc	= ak_serial_getc,
	.tstc	= ak_serial_tstc,
	.putc	= ak_serial_putc,
	.puts	= default_serial_puts,
};

void ak39_serial_initialize(void)
{
	serial_register(&ak_serial_device);
}

__weak struct serial_device *default_serial_console(void)
{
	return &ak_serial_device;
}
