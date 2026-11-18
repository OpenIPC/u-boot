// SPDX-License-Identifier: GPL-2.0+
/*
 * Axera AX620E GPIO: 32 pins per bank, one register per pin at
 * 4 * (pin + 1), bit 0 the output level and bit 1 the direction (1: out);
 * the input levels in one word at 0x8c. Written from the vendor SDK's
 * U-Boot 2020.04 (drivers/gpio/axera_gpio.c).
 */

#include <common.h>
#include <dm.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <linux/bitops.h>

#define AX_GPIO_PIN(n)		(4 * ((n) + 1))
#define AX_GPIO_DR		BIT(0)
#define AX_GPIO_DDR		BIT(1)
#define AX_GPIO_EXT_PORTA	0x8c

struct ax_gpio_priv {
	void __iomem *base;
};

static int ax_gpio_direction_input(struct udevice *dev, unsigned int pin)
{
	struct ax_gpio_priv *priv = dev_get_priv(dev);

	clrbits_le32(priv->base + AX_GPIO_PIN(pin), AX_GPIO_DDR);
	return 0;
}

static int ax_gpio_set_value(struct udevice *dev, unsigned int pin, int val)
{
	struct ax_gpio_priv *priv = dev_get_priv(dev);

	clrsetbits_le32(priv->base + AX_GPIO_PIN(pin), AX_GPIO_DR,
			val ? AX_GPIO_DR : 0);
	return 0;
}

static int ax_gpio_direction_output(struct udevice *dev, unsigned int pin,
				    int val)
{
	struct ax_gpio_priv *priv = dev_get_priv(dev);

	ax_gpio_set_value(dev, pin, val);
	setbits_le32(priv->base + AX_GPIO_PIN(pin), AX_GPIO_DDR);
	return 0;
}

static int ax_gpio_get_value(struct udevice *dev, unsigned int pin)
{
	struct ax_gpio_priv *priv = dev_get_priv(dev);
	u32 reg = readl(priv->base + AX_GPIO_PIN(pin));

	if (reg & AX_GPIO_DDR)
		return !!(reg & AX_GPIO_DR);
	return (readl(priv->base + AX_GPIO_EXT_PORTA) >> pin) & 1;
}

static int ax_gpio_get_function(struct udevice *dev, unsigned int pin)
{
	struct ax_gpio_priv *priv = dev_get_priv(dev);

	if (readl(priv->base + AX_GPIO_PIN(pin)) & AX_GPIO_DDR)
		return GPIOF_OUTPUT;
	return GPIOF_INPUT;
}

static const struct dm_gpio_ops ax_gpio_ops = {
	.direction_input	= ax_gpio_direction_input,
	.direction_output	= ax_gpio_direction_output,
	.get_value		= ax_gpio_get_value,
	.set_value		= ax_gpio_set_value,
	.get_function		= ax_gpio_get_function,
};

static int ax_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct ax_gpio_priv *priv = dev_get_priv(dev);

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -EINVAL;
	uc_priv->gpio_count = dev_read_u32_default(dev, "ngpios", 32);
	uc_priv->bank_name = dev_read_string(dev, "gpio-bank-name");
	if (!uc_priv->bank_name)
		uc_priv->bank_name = dev->name;
	return 0;
}

static const struct udevice_id ax_gpio_ids[] = {
	{ .compatible = "axera,ax620e-gpio" },
	{ }
};

U_BOOT_DRIVER(axera_gpio) = {
	.name		= "axera_gpio",
	.id		= UCLASS_GPIO,
	.of_match	= ax_gpio_ids,
	.ops		= &ax_gpio_ops,
	.probe		= ax_gpio_probe,
	.priv_auto	= sizeof(struct ax_gpio_priv),
};
