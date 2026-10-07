// SPDX-License-Identifier: GPL-2.0+
/*
 * Legacy GPIO driver for HiSilicon/Goke SoCs.
 *
 * The GPIO controllers are PL061-compatible: 8 lines per bank, data at
 * 0x000 and direction at 0x400, bank bases from the arch platform.h.
 * Flat GPIO numbering is bank * 8 + pin, the same as ipctool's linear
 * pad numbers. Every access first puts the pad back into GPIO mode using
 * the pad-mux tables (hi_padmux.c).
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/platform.h>
#include <asm-generic/gpio.h>
#include <linux/errno.h>
#include <vsprintf.h>
#include <hi_padmux.h>

#define HI_GPIO_PER_BANK	8
#define HI_GPIO_DIR		0x400

static const u32 hi_gpio_bases[] = {
#ifdef GPIO0_REG_BASE
	GPIO0_REG_BASE,
#endif
#ifdef GPIO1_REG_BASE
	GPIO1_REG_BASE,
#endif
#ifdef GPIO2_REG_BASE
	GPIO2_REG_BASE,
#endif
#ifdef GPIO3_REG_BASE
	GPIO3_REG_BASE,
#endif
#ifdef GPIO4_REG_BASE
	GPIO4_REG_BASE,
#endif
#ifdef GPIO5_REG_BASE
	GPIO5_REG_BASE,
#endif
#ifdef GPIO6_REG_BASE
	GPIO6_REG_BASE,
#endif
#ifdef GPIO7_REG_BASE
	GPIO7_REG_BASE,
#endif
#ifdef GPIO8_REG_BASE
	GPIO8_REG_BASE,
#endif
#ifdef GPIO9_REG_BASE
	GPIO9_REG_BASE,
#endif
#ifdef GPIO10_REG_BASE
	GPIO10_REG_BASE,
#endif
#ifdef GPIO11_REG_BASE
	GPIO11_REG_BASE,
#endif
#ifdef GPIO12_REG_BASE
	GPIO12_REG_BASE,
#endif
#ifdef GPIO13_REG_BASE
	GPIO13_REG_BASE,
#endif
#ifdef GPIO14_REG_BASE
	GPIO14_REG_BASE,
#endif
#ifdef GPIO15_REG_BASE
	GPIO15_REG_BASE,
#endif
#ifdef GPIO16_REG_BASE
	GPIO16_REG_BASE,
#endif
#ifdef GPIO17_REG_BASE
	GPIO17_REG_BASE,
#endif
#ifdef GPIO18_REG_BASE
	GPIO18_REG_BASE,
#endif
};

#define HI_GPIO_MAX	(ARRAY_SIZE(hi_gpio_bases) * HI_GPIO_PER_BANK)

static inline bool hi_gpio_valid(unsigned int gpio)
{
	return gpio < HI_GPIO_MAX;
}

static inline u32 hi_gpio_base(unsigned int gpio)
{
	return hi_gpio_bases[gpio / HI_GPIO_PER_BANK];
}

static inline u32 hi_gpio_bit(unsigned int gpio)
{
	return BIT(gpio % HI_GPIO_PER_BANK);
}

/*
 * The data register is address-masked (PL061): address bits [9:2] select
 * which data bits are read or written, a plain access at offset 0 sees and
 * changes nothing.
 */
static inline u32 hi_gpio_data_addr(unsigned int gpio, u32 mask)
{
	return hi_gpio_base(gpio) + (mask << 2);
}

/* Make sure the pad carries GPIO rather than a peripheral function */
static void hi_gpio_mux(unsigned int gpio)
{
	hi_padmux_set_gpio(gpio);
}

/* Accept both ipctool spellings: "5_6" (bank 5, pin 6) and "46" (linear) */
int name_to_gpio(const char *name)
{
	unsigned long bank, pin;
	char *end;

	bank = simple_strtoul(name, &end, 10);

	if (end != name && *end == '_') {
		pin = simple_strtoul(end + 1, &end, 10);
		if (*end != '\0' || pin >= HI_GPIO_PER_BANK)
			return -1;
		return bank * HI_GPIO_PER_BANK + pin;
	}

	if (end != name && *end == '\0')
		return bank;

	return simple_strtoul(name, NULL, 0);
}

int gpio_request(unsigned int gpio, const char *label)
{
	return hi_gpio_valid(gpio) ? 0 : -EINVAL;
}

int gpio_free(unsigned int gpio)
{
	return 0;
}

int gpio_direction_input(unsigned int gpio)
{
	if (!hi_gpio_valid(gpio))
		return -EINVAL;

	hi_gpio_mux(gpio);
	clrbits_le32(hi_gpio_base(gpio) + HI_GPIO_DIR, hi_gpio_bit(gpio));

	return 0;
}

int gpio_direction_output(unsigned int gpio, int value)
{
	if (!hi_gpio_valid(gpio))
		return -EINVAL;

	hi_gpio_mux(gpio);
	gpio_set_value(gpio, value);
	setbits_le32(hi_gpio_base(gpio) + HI_GPIO_DIR, hi_gpio_bit(gpio));

	return 0;
}

int gpio_get_value(unsigned int gpio)
{
	u32 mask;

	if (!hi_gpio_valid(gpio))
		return -EINVAL;

	mask = hi_gpio_bit(gpio);
	hi_gpio_mux(gpio);

	return !!(readl(hi_gpio_data_addr(gpio, mask)) & mask);
}

int gpio_set_value(unsigned int gpio, int value)
{
	u32 mask;

	if (!hi_gpio_valid(gpio))
		return -EINVAL;

	mask = hi_gpio_bit(gpio);
	hi_gpio_mux(gpio);

	writel(value ? mask : 0, hi_gpio_data_addr(gpio, mask));

	return 0;
}

/* Called by the legacy `gpio status` command */
void hi_gpio_status(void)
{
	int b, p;

	for (b = 0; b < ARRAY_SIZE(hi_gpio_bases); b++) {
		u32 data = readl(hi_gpio_bases[b] + (0xff << 2));
		u32 dir = readl(hi_gpio_bases[b] + HI_GPIO_DIR);

		printf("Bank %d @0x%08x: Data 0x%02x, Dir 0x%02x\n", b,
		       hi_gpio_bases[b], data & 0xff, dir & 0xff);

		for (p = 0; p < HI_GPIO_PER_BANK; p++) {
			unsigned int gpio = b * HI_GPIO_PER_BANK + p;
			const char *func = hi_padmux_get_func(gpio);

			printf("  GPIO%d_%d: %s %d  mux: %s\n", b, p,
			       (dir & BIT(p)) ? "out" : "in ",
			       !!(data & BIT(p)),
			       func ? func : "-");
		}
	}
}
