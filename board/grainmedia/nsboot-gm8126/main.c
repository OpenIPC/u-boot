// SPDX-License-Identifier: GPL-2.0+
/*
 * GM8126 first stage, in place of Grain Media's nsboot and "burn-in" image,
 * whose sources are not public.
 *
 * Written from what the vendor's two stages do on a camera, observed
 * through their register accesses, and from the GM8126 board file of the
 * GM8136 SDK's U-Boot 2013.01 (board/gm/gm8126: lowlevel_init.S,
 * init_sys_freq(), FCS_go()). The DDR controller's values are the
 * vendor's, for one 1 Gbit DDR3 chip (128 MiB), the only fit seen: a board
 * with another one needs its own table.
 *
 * The order is the vendor's: remap DDR to 0, program the controller, read
 * the flash, raise the clocks, load the next stage to 0 and enter it.
 */

typedef unsigned int u32;
typedef unsigned char u8;

#define REG(a)		(*(volatile u32 *)(a))

#define AHBC		0x90000000
#define SSP0		0x98200000
#define UART0		0x98300000
#define PMU		0x99000000
#define DDRC		0x99300000
#define GPIO0		0x99400000
#define INTC		0x99500000

#define IRQ_PMU		8
#define CS_PIN		(1u << 14)

#define IMAGE_MAGIC	0x805a474d
#define IMAGE_HDR	0x100

void fcs_irq(void);
void fcs_wait(void);
void fcs_handler(void);

static void set(u32 a, u32 v)
{
	REG(a) |= v;
}

static void clr(u32 a, u32 v)
{
	REG(a) &= ~v;
}

static void put(u32 a, u32 mask, u32 v)
{
	REG(a) = (REG(a) & ~mask) | v;
}

static void delay(volatile u32 n)
{
	while (n--)
		;
}

/* ---- console ---------------------------------------------------------- */

/*
 * The UARTs' clock, set rather than trusted: the 30 MHz oscillator over
 * 16, what the vendor's divisor of 3 for 38400 and its kernel imply. Over
 * 16 x 1 that is 117 kBaud, 1.7 % off 115200.
 */
static void uart_init(void)
{
	clr(PMU + 0x28, 1 << 15);
	put(PMU + 0x78, 0x3f << 18, 15 << 18);
	REG(UART0 + 0x08) = 0x35;
	REG(UART0 + 0x0c) = 0x83;
	REG(UART0 + 0x00) = 1;
	REG(UART0 + 0x04) = 0;
	REG(UART0 + 0x0c) = 0x03;
}

static void putch(char c)
{
	u32 n = 100000;

	while (!(REG(UART0 + 0x14) & 0x20) && n--)
		;
	REG(UART0) = c;
}

static void puts(const char *s)
{
	for (; *s; s++) {
		if (*s == '\n')
			putch('\r');
		putch(*s);
	}
}

static void puthex(u32 v)
{
	int i;

	for (i = 28; i >= 0; i -= 4)
		putch("0123456789abcdef"[(v >> i) & 15]);
}

static void stop(const char *why)
{
	puts(why);
	for (;;)
		;
}

/* ---- DDR --------------------------------------------------------------- */

/*
 * The AHB slots that name the DDR: 0x10000000 until the remap bit is set,
 * 0 after it. Nothing to do when a loader has been here already.
 */
static void ddr_remap(void)
{
	u32 i;

	if (REG(AHBC + 0x88) & 1)
		return;
	REG(AHBC + 0x10) = 0x00080000;
	REG(AHBC + 0x18) = 0x10080000;
	REG(AHBC + 0x100000) = 0x00080000;
	for (i = 4; i <= 6; i++)
		REG(AHBC + i * 0x100000) = 0x00080000;
	set(AHBC + 0x88, 1);
	REG(AHBC + 0x18) = 0x00080000;
}

struct ddr_reg {
	u32 off, val;
};

/* the same in both of the vendor's tables */
static const struct ddr_reg ddr_common[] = {
	{ 0x00, 0x00075d01 }, { 0x0c, 0 }, { 0x10, 0x10004343 },
	{ 0x20, 0x00000f52 }, { 0x24, 3 }, { 0x28, 0x410 }, { 0x2c, 0 },
	{ 0x30, 0 }, { 0x34, 0x06060606 }, { 0x38, 0x06060606 },
	{ 0x40, 0 }, { 0x48, 0xff }, { 0x74, 1 }, { 0x7c, 0xffffffff },
	{ 0xa0, 0x87878787 }, { 0xa4, 0x87878787 }, { 0xa8, 0x0001a0ab },
	{ 0xac, 0x00061a80 },
};

/* mode and timings: by the DDR clock the straps chose (PLL1 bits 25:24) */
static const struct ddr_reg ddr_slow[] = {
	{ 0x08, 0x00420642 }, { 0x14, 0x1f0c0e0b }, { 0x18, 0x21311323 },
	{ 0x1c, 0x0100192f },
};

static const struct ddr_reg ddr_fast[] = {
	{ 0x08, 0x00400a62 }, { 0x14, 0x32131711 }, { 0x18, 0x32511535 },
	{ 0x1c, 0x01001960 },
};

static void ddr_init(void)
{
	const struct ddr_reg *t = ddr_fast;
	u32 i;

	clr(PMU + 0x38, 1 << 3);		/* the controller's clock */
	if (((REG(PMU + 0x30) >> 24) & 3) == 1)
		t = ddr_slow;
	for (i = 0; i < sizeof(ddr_common) / sizeof(ddr_common[0]); i++)
		REG(DDRC + ddr_common[i].off) = ddr_common[i].val;
	for (i = 0; i < 4; i++)
		REG(DDRC + t[i].off) = t[i].val;
	REG(DDRC + 0x04) = 1;
	for (i = 0; !(REG(DDRC + 0x04) & 0x100); i++)
		if (i > 10000000)
			stop("DDR controller does not get ready\n");
	/* the static memory controller's bank, as the vendor leaves it */
	REG(0x10000000) = 0x10000040;
	REG(0x10000004) = 0x010ff0f1;
}

static int ddr_test(void)
{
	volatile u32 *p = (volatile u32 *)0x00100000;
	u32 i;

	for (i = 0; i < 1024; i++)
		p[i] = i * 0x9e3779b1;
	for (i = 0; i < 1024; i++)
		if (p[i] != i * 0x9e3779b1)
			return -1;
	return 0;
}

/* ---- clocks ------------------------------------------------------------- */

void fcs_handler(void)
{
	REG(PMU + 0x20) = 1 << 17;
	clr(PMU + 0x0c, 4);
	REG(INTC + 0x08) = 1 << IRQ_PMU;
	clr(INTC + 0x04, 1 << IRQ_PMU);
}

/*
 * PLL2 594 MHz, PLL1 (DDR) 800 MHz, CPU 594 MHz, AHB 297 MHz: the
 * vendor's. The change takes the core's clock away, so it waits for the
 * PMU's interrupt in WFI, running from SRAM.
 */
static void clocks(void)
{
	u32 v;

	put(PMU + 0x28, (3 << 18) | (1 << 7), 1 << 18);	/* MPEG4, H.264 */
	REG(PMU + 0x34) = (REG(PMU + 0x34) & 0xfffe0000) | (5 << 12) |
			  (99 << 3) | 7;

	v = (444u << 22) | (3 << 20) | (3 << 12) | (80 << 3);
	/* it reads back shifted left by one */
	if ((REG(PMU + 0x30) & ~0xfu) == (v << 1))
		return;
	REG(0x18) = 0xe59ff018;			/* ldr pc, [pc, #0x18] */
	REG(0x38) = (u32)fcs_irq;
	REG(INTC + 0x24) = 0;
	REG(INTC + 0x04) = 0;
	REG(INTC + 0x08) = 0xffffffff;
	REG(PMU + 0x30) = v;
	clr(INTC + 0x0c, 1 << IRQ_PMU);
	clr(INTC + 0x10, 1 << IRQ_PMU);
	REG(INTC + 0x04) = 1 << IRQ_PMU;
	fcs_wait();
}

/* ---- flash ---------------------------------------------------------------- */

static void cs(int on)
{
	REG(GPIO0 + (on ? 0x14 : 0x10)) = CS_PIN;
}

static u8 spi(u8 v)
{
	REG(SSP0 + 0x18) = v;
	while (!(REG(SSP0 + 0x0c) & 0x1f0))
		;
	return REG(SSP0 + 0x18);
}

static void spi_init(void)
{
	put(PMU + 0x78, 0x3f, 2);		/* SSP0 from PLL2 / 3 */
	clr(PMU + 0x3c, 0x220);			/* SSP0 and GPIO0 clocks */
	put(PMU + 0x50, 0xfc000000, 0x54000000);	/* pins */
	clr(PMU + 0x54, 0x37);
	set(GPIO0 + 0x08, CS_PIN);
	cs(0);
	REG(SSP0 + 0x00) = 0x102c;		/* SPI master, mode 0 */
	REG(SSP0 + 0x10) = 0;
	REG(SSP0 + 0x04) = (7 << 16) | 5;	/* 8-bit frames */
	REG(SSP0 + 0x08) = 0xd;
}

static void spi_read(u32 from, u8 *to, u32 len)
{
	cs(1);
	spi(0x03);
	spi(from >> 16);
	spi(from >> 8);
	spi(from);
	while (len--)
		*to++ = spi(0);
	cs(0);
}

void main(void)
{
	u32 hdr[4], at;

	ddr_remap();
	/* the clocks U-Boot's lowlevel_init() turns on */
	REG(PMU + 0x38) &= 0xfffff7b3;
	REG(PMU + 0x3c) &= 0xffffadde;
	/* the console first: it runs from SRAM, and DDR may not come up */
	uart_init();
	puts("\nOpenIPC GM8126 nsboot\n");
	ddr_init();
	if (ddr_test())
		stop("DDR test failed\n");
	clocks();
	delay(1000);
	spi_init();

	/* the system header's "bootm" address; swapped, as the ROM reads it */
	spi_read(0x08, (u8 *)hdr, 4);
	at = (hdr[0] >> 24) | ((hdr[0] >> 8) & 0xff00) |
	     ((hdr[0] << 8) & 0xff0000) | (hdr[0] << 24);
	spi_read(at, (u8 *)hdr, sizeof(hdr));
	if (hdr[0] != IMAGE_MAGIC || !hdr[2] || hdr[2] > 0x100000) {
		puts("no image at ");
		puthex(at);
		stop("\n");
	}
	spi_read(at + IMAGE_HDR, (u8 *)0, hdr[2]);
	((void (*)(void))0)();
	stop("U-Boot returned\n");
}
