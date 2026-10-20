// SPDX-License-Identifier: GPL-2.0+
/*
 * Goke GK710x pin functions, from the vendor U-Boot's drivers/gpio/
 * goke_gpio.c (gk7101_gpio_func_config()) with the HAL undone.
 *
 * A pin type is the vendor's GPIO_TYPE_* word: output function [7:0], input
 * function + 2 [15:8], output-enable source [21:16], output and enable
 * inversion [23:22], pad control [29:24] and direction [31:30]. The kernel
 * decodes the same words from the map U-Boot leaves it (board/goke/gk710x).
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/gk710x.h>

#define TYPE_OUT_SEL(t)		((t) & 0xff)
#define TYPE_IN_SEL(t)		(((t) >> 8) & 0xff)
#define TYPE_OEN_SEL(t)		(((t) >> 16) & 0x3f)
#define TYPE_OUT_INV(t)		(((t) >> 22) & 1)
#define TYPE_OEN_INV(t)		(((t) >> 23) & 1)
#define TYPE_IOCTRL(t)		(((t) >> 24) & 0x3f)
#define TYPE_FUNC(t)		((t) >> 30)

#define FUNC_OUT		2

/* Where a pin's pad-control byte sits in its GK_RCT_IOCTRL_GPIO() word */
static unsigned int gk_ioctrl_shift(unsigned int pin)
{
#ifdef CONFIG_GK710X_S
	/* gk7101s_gpio.c, in silicon byte order: it shuffles them by range */
	static const u8 low[4] = { 1, 2, 0, 3 };	/* GPIO 0-3 */
	static const u8 mid[4] = { 0, 1, 2, 3 };	/* GPIO 4-51 */
	static const u8 top[4] = { 0, 2, 1, 3 };	/* GPIO 52-55 */
	static const u8 high[4] = { 2, 0, 1, 3 };	/* GPIO 56-63 */
	const u8 *b = pin < 4 ? low : pin < 52 ? mid : pin < 56 ? top : high;

	return 8 * b[pin % 4];
#else
	return 8 * (pin % 4);
#endif
}

void gk_gpio_config(unsigned int pin, unsigned int type)
{
	unsigned int shift = gk_ioctrl_shift(pin);
	u32 v;

	if (!type)
		return;

	/* Input functions >= 2 are routed from a pin; 0 and 1 are constants */
	if (TYPE_FUNC(type) != FUNC_OUT && TYPE_IN_SEL(type) >= 2)
		writel(pin, GK_GPIO_INPUT_CFG(TYPE_IN_SEL(type) - 2));

	writel(TYPE_OUT_SEL(type) | TYPE_OEN_SEL(type) << 8 |
	       TYPE_OUT_INV(type) << 14 | TYPE_OEN_INV(type) << 15,
	       GK_GPIO_OUTPUT_CFG(pin));

	if (TYPE_IOCTRL(type)) {
		v = readl(GK_RCT_IOCTRL_GPIO(pin));
		v &= ~(0x3f << shift);
		v |= TYPE_IOCTRL(type) << shift;
		writel(v, GK_RCT_IOCTRL_GPIO(pin));
	}
}

/* After a map is applied, as the vendor gk7101_gpio_initialize() ends */
void gk_gpio_init(void)
{
	writel(1, GK_GPIO_IN_EN);
	writel(GK_IOCTRL_MISC2_VAL, GK_RCT_IOCTRL_MISC2);
}
