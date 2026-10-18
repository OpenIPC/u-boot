// SPDX-License-Identifier: GPL-2.0+
/*
 * GPIO maps of the Goke GK7101/GK7102 reference boards, which camera makers
 * copied, and the copy the kernel takes over.
 *
 * The vendor U-Boot builds one image per board (arch-gk7101/gpio_rb_*.h)
 * and leaves that board's map in the top 64 KiB of DDR; the vendor kernel,
 * OpenIPC's included, reads it from there ("gpio map get from uboot") and
 * configures every pin from it -- sensor I2C and reset, IR cut, SD, the
 * Ethernet PHY pins. Without a map the camera boots with no sensor bus.
 *
 * The SDK's 20-odd board headers come down to two layouts:
 *  - v2 (the default): GK7102 rb-sc1035 v2.1, rb-sc1045, rb-imx225,
 *    rb-ov9750, rb-jxh42 v2.0, ak-gc1024, GK7101 rb-ov2710 v2.2.
 *    UART0 on GPIO 25/26, SD on 39-47.
 *  - v1: GK7102 rb-ov9712, rb-ar0130, rb-jxh42 v1.0, GK7101 rb-imx222.
 *    UART0 on GPIO 30/31, the external-PHY pins on 41-47.
 * gpiomap=v1 in the environment selects the second.
 *
 * Pin types are the vendor's GPIO_TYPE_* words (mach-gk710x/gpio.c), named
 * in the comments; the tables were generated from the SDK headers.
 */

#include <common.h>
#include <env.h>
#include <asm/arch/gk710x.h>

struct gk_gpio_pin {
	u32 pin;
	u32 type;
};

static const struct gk_gpio_pin v2_xref[] = {
	{  0, 0xa0000024 },	/* output_sf_cs0 */
	{  1, 0xa0000025 },	/* output_sf_cs1 */
	{  2, 0x40010000 },	/* input */
	{  4, 0x40010000 },	/* input */
	{  5, 0x40010000 },	/* input */
	{  6, 0x40010000 },	/* input */
	{  7, 0x40010000 },	/* input */
	{  8, 0x40010000 },	/* input */
	{  9, 0x40010000 },	/* input */
	{ 10, 0x40010000 },	/* input */
	{ 11, 0x40010000 },	/* input */
	{ 12, 0xa0000000 },	/* output_0 (IR_LED_CTRL) */
	{ 13, 0x40010000 },	/* input */
	{ 14, 0x40010000 },	/* input */
	{ 15, 0x40010000 },	/* input (IR_DET_ADC) */
	{ 16, 0x40010000 },	/* input */
	{ 17, 0x40010000 },	/* input */
	{ 18, 0x40010000 },	/* input */
	{ 19, 0x40010000 },	/* input */
	{ 20, 0xa0000000 },	/* output_0 (IR_CUT+) */
	{ 21, 0x40010000 },	/* input */
	{ 22, 0x40010000 },	/* input */
	{ 23, 0xa0000000 },	/* output_0 (IR_CUT-) */
	{ 24, 0x40010000 },	/* input */
	{ 25, 0x50010900 },	/* input_uart0_rx */
	{ 26, 0x90000009 },	/* output_uart0_tx */
	{ 27, 0xa0000001 },	/* output_1 (Sensor reset) */
	{ 28, 0xd0030300 },	/* inout_i2c_data (S D) */
	{ 29, 0xd0040400 },	/* inout_i2c_clk (S C) */
	{ 34, 0x40010000 },	/* input */
	{ 37, 0x40010000 },	/* input */
	{ 38, 0x40010000 },	/* input */
	{ 39, 0x50010f00 },	/* input_sd_cd_n */
	{ 40, 0x40010000 },	/* input */
	{ 41, 0xd08b1319 },	/* inout_sd_data_3 */
	{ 42, 0xd08a1218 },	/* inout_sd_data_2 */
	{ 43, 0xd090181e },	/* inout_sd_cmd */
	{ 44, 0x9000001f },	/* output_sdio_clk */
	{ 45, 0xd0891117 },	/* inout_sd_data_1 */
	{ 46, 0xa0000001 },	/* output_1 (SDIO_CARD_POWER_EN) */
	{ 47, 0xd0881016 },	/* inout_sd_data_0 */
	{ 55, 0x40010000 },	/* input */
};

static const struct gk_gpio_pin v2_intphy[] = {
	{  3, 0x40010000 },	/* input */
	{ 30, 0x40010000 },	/* input */
	{ 31, 0x40010000 },	/* input */
	{ 32, 0x40010000 },	/* input */
	{ 33, 0x40010000 },	/* input */
	{ 35, 0x40010000 },	/* input */
	{ 36, 0x40010000 },	/* input */
	{ 48, 0xa0000020 },	/* output_aomclk */
	{ 49, 0x40010000 },	/* input */
	{ 50, 0x40010000 },	/* input */
	{ 51, 0x40010000 },	/* input */
	{ 52, 0x40010000 },	/* input */
	{ 53, 0x40010000 },	/* input */
	{ 54, 0xa0000029 },	/* output_ephy_led_3 */
};

static const struct gk_gpio_pin v2_extphy[] = {
	{  3, 0x60012400 },	/* input_enet_phy_rxdv */
	{ 30, 0xa0000030 },	/* output_enet_phy_txen */
	{ 31, 0xa000002c },	/* output_enet_phy_txd_1 */
	{ 32, 0xa000002b },	/* output_enet_phy_txd_0 */
	{ 33, 0x60012300 },	/* input_enet_phy_rxer */
	{ 35, 0xa0000032 },	/* output_enet_gmii_mdc_o */
	{ 36, 0xe0932531 },	/* inout_eth_mdio */
	{ 48, 0x40010000 },	/* input */
	{ 49, 0xa0000001 },	/* output_1 (ETH reset) */
	{ 50, 0xa0000020 },	/* output_aomclk */
	{ 51, 0x60011e00 },	/* input_enet_phy_rxd_1 */
	{ 52, 0x60011f00 },	/* input_enet_phy_rxd_2 (CLK) */
	{ 53, 0x60011d00 },	/* input_enet_phy_rxd_0 */
	{ 54, 0x40010000 },	/* input */
};

static const struct gk_gpio_pin v1_xref[] = {
	{  0, 0xa0000024 },	/* output_sf_cs0 */
	{  1, 0xa0000025 },	/* output_sf_cs1 */
	{  4, 0x40010000 },	/* input */
	{  5, 0x40010000 },	/* input */
	{  6, 0x40010000 },	/* input */
	{  7, 0x40010000 },	/* input */
	{  8, 0x40010000 },	/* input */
	{  9, 0x40010000 },	/* input */
	{ 10, 0x40010000 },	/* input */
	{ 11, 0x40010000 },	/* input */
	{ 12, 0xa0000000 },	/* output_0 (IR_LED_CTRL) */
	{ 13, 0x40010000 },	/* input */
	{ 14, 0x40010000 },	/* input */
	{ 15, 0x40010000 },	/* input (IR_DET_ADC) */
	{ 16, 0x40010000 },	/* input */
	{ 17, 0x40010000 },	/* input */
	{ 18, 0x40010000 },	/* input */
	{ 19, 0x40010000 },	/* input */
	{ 20, 0xa0000000 },	/* output_0 (IR_CUT+) */
	{ 21, 0x40010000 },	/* input */
	{ 22, 0x40010000 },	/* input */
	{ 23, 0xa0000000 },	/* output_0 (IR_CUT-) */
	{ 24, 0x40010000 },	/* input */
	{ 25, 0x40010000 },	/* input */
	{ 26, 0x40010000 },	/* input */
	{ 27, 0xa0000001 },	/* output_1 (Sensor reset) */
	{ 28, 0xd0030300 },	/* inout_i2c_data (S D) */
	{ 29, 0xd0040400 },	/* inout_i2c_clk (S C) */
	{ 30, 0x90000009 },	/* output_uart0_tx */
	{ 31, 0x50010900 },	/* input_uart0_rx */
	{ 34, 0x40010000 },	/* input */
	{ 35, 0xd0050500 },	/* inout_i2c_data2 */
	{ 36, 0xd0060600 },	/* inout_i2c_clk2 */
	{ 37, 0x40010000 },	/* input */
	{ 38, 0x40010000 },	/* input */
	{ 39, 0x40010000 },	/* input */
	{ 40, 0x40010000 },	/* input */
	{ 42, 0x40010000 },	/* input */
	{ 45, 0x40010000 },	/* input */
	{ 53, 0x40010000 },	/* input (ALARM_OUT) */
	{ 54, 0x40010000 },	/* input (ALARM_IN) */
	{ 55, 0x40010000 },	/* input */
};

static const struct gk_gpio_pin v1_intphy[] = {
	{  2, 0x40010000 },	/* input */
	{  3, 0x40010000 },	/* input */
	{ 32, 0x40010000 },	/* input */
	{ 33, 0x40010000 },	/* input */
	{ 41, 0x40010000 },	/* input */
	{ 43, 0x40010000 },	/* input */
	{ 44, 0x40010000 },	/* input */
	{ 46, 0x40010000 },	/* input */
	{ 47, 0x40010000 },	/* input */
	{ 48, 0xa0000020 },	/* output_aomclk */
	{ 49, 0xa0000029 },	/* output_ephy_led_3 */
	{ 50, 0x40010000 },	/* input */
	{ 51, 0x40010000 },	/* input */
	{ 52, 0x40010000 },	/* input */
};

static const struct gk_gpio_pin v1_extphy[] = {
	{  2, 0x60012300 },	/* input_enet_phy_rxer */
	{  3, 0xa0000032 },	/* output_enet_gmii_mdc_o */
	{ 32, 0x60011e00 },	/* input_enet_phy_rxd_1 */
	{ 33, 0xe0932531 },	/* inout_eth_mdio */
	{ 41, 0xa0000001 },	/* output_1 (ETH reset) */
	{ 43, 0xa0000020 },	/* output_aomclk */
	{ 44, 0xa0000030 },	/* output_enet_phy_txen */
	{ 46, 0xa000002c },	/* output_enet_phy_txd_1 */
	{ 47, 0xa000002b },	/* output_enet_phy_txd_0 */
	{ 48, 0x40010000 },	/* input */
	{ 49, 0x40010000 },	/* input */
	{ 50, 0x60011f00 },	/* input_enet_phy_rxd_2 (CLK) */
	{ 51, 0x60012400 },	/* input_enet_phy_rxdv */
	{ 52, 0x60011d00 },	/* input_enet_phy_rxd_0 */
};

struct gk_gpio_map {
	const char *name;
	const struct gk_gpio_pin *pins, *intphy, *extphy;
	u32 npins, nintphy, nextphy;
	/* the pins the kernel drives itself, in gpio_cfg_t order */
	u32 ir_led_ctl, ir_cut1, ir_cut2, sensor_reset, phy_reset, phy_speed_led;
	u32 spi0_en0, spi1_en0, pwm[4], usb_host, sd_detect, sd_power, pmu_ctl;
};

#define MAP(v) .pins = v##_xref, .npins = ARRAY_SIZE(v##_xref), \
	.intphy = v##_intphy, .nintphy = ARRAY_SIZE(v##_intphy), \
	.extphy = v##_extphy, .nextphy = ARRAY_SIZE(v##_extphy)

static const struct gk_gpio_map gk_maps[] = {
	{
		.name = "v2", MAP(v2),
		.ir_led_ctl = 4, .ir_cut1 = 20, .ir_cut2 = 23,
		.sensor_reset = 27, .phy_reset = 49, .phy_speed_led = 54,
		.spi0_en0 = 5, .spi1_en0 = 6, .pwm = { 7, 8, 9, 10 },
		.usb_host = 11, .sd_detect = 39, .sd_power = 46, .pmu_ctl = 55,
	}, {
		.name = "v1", MAP(v1),
		.ir_led_ctl = 12, .ir_cut1 = 20, .ir_cut2 = 23,
		.sensor_reset = 27, .phy_reset = 41, .phy_speed_led = 49,
		.spi0_en0 = 37, .spi1_en0 = 38, .pwm = { 39, 40, 42, 45 },
		.usb_host = 55, .sd_detect = 4, .sd_power = 5, .pmu_ctl = 55,
	},
};

/* The vendor's gpio_cfg_t (drivers/gpio/goke_gpio.c), as the kernel reads it */
struct gk_gpio_cfg {
	u32 gpio_count;
	struct gk_gpio_pin gpio_chip[64];
	u32 extphy_gpio_count;
	struct gk_gpio_pin ext_phy_gpio[16];
	u32 intphy_gpio_count;
	struct gk_gpio_pin int_phy_gpio[16];
	u32 ir_led_ctl, ir_cut1, ir_cut2, sensor_reset, phy_reset, phy_speed_led;
	u32 spi0_en0, spi1_en0, pwm[4], usb_host, sd_detect, sd_power;
	u32 soc_type;
	char board_type[32];
	u32 board_version;
	u32 reserve1;
	u32 ext_phy_clk;		/* MHz; 0: the internal PHY */
	u32 reserve2;
	u32 pmu_ctl;
};

static const struct gk_gpio_map *gk_map(void)
{
	const char *name = env_get("gpiomap");
	int i;

	for (i = 0; name && i < ARRAY_SIZE(gk_maps); i++)
		if (!strcmp(name, gk_maps[i].name))
			return &gk_maps[i];
	return &gk_maps[0];
}

/* As the vendor U-Boot does at start: every pin, then the internal PHY's */
void gk_gpio_map_apply(void)
{
	const struct gk_gpio_map *m = gk_map();
	int i;

	for (i = 0; i < m->npins; i++)
		gk_gpio_config(m->pins[i].pin, m->pins[i].type);
	for (i = 0; i < m->nintphy; i++)
		gk_gpio_config(m->intphy[i].pin, m->intphy[i].type);
	gk_gpio_init();
}

void gk_gpio_map_save(void *dst)
{
	const struct gk_gpio_map *m = gk_map();
	struct gk_gpio_cfg *c = dst;

	memset(c, 0, sizeof(*c));
	c->gpio_count = m->npins;
	memcpy(c->gpio_chip, m->pins, m->npins * sizeof(*m->pins));
	c->extphy_gpio_count = m->nextphy;
	memcpy(c->ext_phy_gpio, m->extphy, m->nextphy * sizeof(*m->extphy));
	c->intphy_gpio_count = m->nintphy;
	memcpy(c->int_phy_gpio, m->intphy, m->nintphy * sizeof(*m->intphy));
	c->ir_led_ctl = m->ir_led_ctl;
	c->ir_cut1 = m->ir_cut1;
	c->ir_cut2 = m->ir_cut2;
	c->sensor_reset = m->sensor_reset;
	c->phy_reset = m->phy_reset;
	c->phy_speed_led = m->phy_speed_led;
	c->spi0_en0 = m->spi0_en0;
	c->spi1_en0 = m->spi1_en0;
	memcpy(c->pwm, m->pwm, sizeof(c->pwm));
	c->usb_host = m->usb_host;
	c->sd_detect = m->sd_detect;
	c->sd_power = m->sd_power;
	c->pmu_ctl = m->pmu_ctl;
	c->soc_type = IS_ENABLED(CONFIG_TARGET_GK7101) ? 0 : 1;
	snprintf(c->board_type, sizeof(c->board_type), "openipc-%s", m->name);
	c->board_version = 0;
	c->ext_phy_clk = 0;
}
