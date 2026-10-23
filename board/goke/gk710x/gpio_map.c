// SPDX-License-Identifier: GPL-2.0+
/*
 * GPIO maps of the Goke GK7101/GK7102 and GK7101S/GK7102S reference
 * boards, which camera makers copied, and the copy the kernel takes over.
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
 * gpiomap=v1 in the environment selects the second. The S boards of the
 * same names (arch-gk7101s/gpio_rb_*.h) split the same way, with the S
 * pin-function numbers. The GK7102S image adds a third, "c", for GK7102C
 * boards, and makes it its default.
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

#ifndef CONFIG_GK710X_S

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

#else /* GK710xS: its own pin-function numbers (arch-gk7101s/gpio.h) */

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
	{ 56, 0x40010000 },	/* input */
	{ 57, 0x40010000 },	/* input */
	{ 58, 0x40010000 },	/* input */
	{ 59, 0x40010000 },	/* input */
	{ 60, 0x40010000 },	/* input */
	{ 61, 0x40010000 },	/* input */
	{ 62, 0x40010000 },	/* input */
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
	{ 56, 0x40010000 },	/* input */
	{ 57, 0x40010000 },	/* input */
	{ 58, 0x40010000 },	/* input */
	{ 59, 0x40010000 },	/* input */
	{ 60, 0x40010000 },	/* input */
	{ 61, 0x40010000 },	/* input */
	{ 62, 0x40010000 },	/* input */
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

/*
 * c: the GK7102C board in the camera dump this port was checked against
 * ("devkit-v1.1", vendor U-Boot of 2020). Its U-Boot hands the kernel
 * pins 0-32 as function IDs of the GK7102C SDK; here they are the S
 * pin types of the same functions. No PHY pins: the package has no
 * GPIO 48. A type of 0 (GPIO_TYPE_UNDEFINED) leaves a pin as it is; U-Boot
 * and the kernel skip it.
 */
static const struct gk_gpio_pin c_xref[] = {
	/*
	 * The flash chip select: left as the boot ROM sets it (output select
	 * 0). The S type output_sf_cs0 (output select 36) is not that
	 * function on the GK7102C: written there, flash reads return
	 * garbage, and the kernel's flash driver found no chip.
	 */
	{  0, 0 },
	{  1, 0xa0000001 },	/* output_1 (sensor reset) */
	{  2, 0xd0030300 },	/* inout_i2c_data */
	{  3, 0xd0040400 },	/* inout_i2c_clk */
	{  4, 0xa0000001 },	/* output_1 */
	{  5, 0xa0000000 },	/* output_0 */
	{  6, 0xd090181e },	/* inout_sd_cmd */
	{  7, 0xd0891117 },	/* inout_sd_data_1 */
	{  8, 0xd0881016 },	/* inout_sd_data_0 */
	{  9, 0x9000001f },	/* output_sdio_clk */
	{ 10, 0x50010f00 },	/* input_sd_cd_n */
	{ 11, 0xa0000000 },	/* output_0 */
	{ 12, 0x60010000 },	/* input_0 */
	{ 13, 0xa0000000 },	/* output_0 */
	{ 14, 0xa0000000 },	/* output_0 */
	{ 15, 0xa0000000 },	/* output_0 */
	{ 16, 0x60010000 },	/* input_0 */
	{ 17, 0xa0000000 },	/* output_0 */
	{ 18, 0x60010000 },	/* input_0 */
	{ 19, 0xd08b1319 },	/* inout_sd_data_3 */
	{ 20, 0xd08a1218 },	/* inout_sd_data_2 */
	{ 21, 0xa0000000 },	/* output_0 */
	{ 22, 0xa0000000 },	/* output_0 */
	{ 23, 0xa0000000 },	/* output_0 */
	{ 24, 0xa0000000 },	/* output_0 */
	{ 25, 0x50010900 },	/* input_uart0_rx */
	{ 26, 0x90000009 },	/* output_uart0_tx */
	{ 27, 0xa0000000 },	/* output_0 */
	{ 28, 0xa0000000 },	/* output_0 */
	{ 29, 0x60010000 },	/* input_0 */
	{ 30, 0x60010000 },	/* input_0 */
	{ 31, 0x60010000 },	/* input_0 */
	{ 32, 0x60010000 },	/* input_0 */
};

#endif

struct gk_gpio_map {
	const char *name;
	const struct gk_gpio_pin *pins, *intphy, *extphy;
	u32 npins, nintphy, nextphy;
	/* the pins the kernel drives itself, in gpio_cfg_t order */
	u32 ir_led_ctl, ir_cut1, ir_cut2, sensor_reset, phy_reset, phy_speed_led;
	u32 spi0_en0, spi1_en0, pwm[4], usb_host, sd_detect, sd_power, pmu_ctl;
	u32 sd1_detect, sd1_power;	/* GK710xS */
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
		.usb_host = 11, .sd_detect = 39, .sd_power = 46,
#ifdef CONFIG_GK710X_S
		.sd1_detect = 63, .sd1_power = 63,	/* none */
#else
		.pmu_ctl = 55,
#endif
	}, {
		.name = "v1", MAP(v1),
		.ir_led_ctl = 12, .ir_cut1 = 20, .ir_cut2 = 23,
		.sensor_reset = 27, .phy_reset = 41, .phy_speed_led = 49,
		.spi0_en0 = 37, .spi1_en0 = 38, .pwm = { 39, 40, 42, 45 },
		.usb_host = 55, .sd_detect = 4, .sd_power = 5,
#ifdef CONFIG_GK710X_S
		.sd1_detect = 63, .sd1_power = 63,
#else
		.pmu_ctl = 55,
#endif
	},
#ifdef CONFIG_GK710X_S
	{
		/* 63: none, as the vendor U-Boot hands them over */
		.name = "c",
		.pins = c_xref, .npins = ARRAY_SIZE(c_xref),
		.ir_led_ctl = 63, .ir_cut1 = 63, .ir_cut2 = 63,
		.sensor_reset = 1, .phy_reset = 63, .phy_speed_led = 63,
		.spi0_en0 = 63, .spi1_en0 = 63, .pwm = { 63, 63, 63, 63 },
		.usb_host = 63, .sd_detect = 10, .sd_power = 63,
		.sd1_detect = 63, .sd1_power = 63,
	},
#endif
};

/* The GK7102S image boots the GK7102C's boot header: its map comes first */
#ifdef CONFIG_TARGET_GK7102S
#define GK_GPIO_MAP_DEFAULT	"c"
#else
#define GK_GPIO_MAP_DEFAULT	"v2"
#endif

/*
 * The vendor's gpio_cfg_t (drivers/gpio/goke_gpio.c, gk7101s_gpio.c), as the
 * kernel reads it. The GK710xS one has two SD1 pins before soc_type.
 */
struct gk_gpio_cfg {
	u32 gpio_count;
	struct gk_gpio_pin gpio_chip[64];
	u32 extphy_gpio_count;
	struct gk_gpio_pin ext_phy_gpio[16];
	u32 intphy_gpio_count;
	struct gk_gpio_pin int_phy_gpio[16];
	u32 ir_led_ctl, ir_cut1, ir_cut2, sensor_reset, phy_reset, phy_speed_led;
	u32 spi0_en0, spi1_en0, pwm[4], usb_host, sd_detect, sd_power;
#ifdef CONFIG_GK710X_S
	u32 sd1_detect, sd1_power;
#endif
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

	if (!name)
		name = GK_GPIO_MAP_DEFAULT;
	for (i = 0; i < ARRAY_SIZE(gk_maps); i++)
		if (!strcmp(name, gk_maps[i].name))
			return &gk_maps[i];
	return &gk_maps[0];
}

/*
 * U-Boot itself needs the internal PHY's reference clock, out on GPIO 48
 * in the layouts that have it, and the SD pins where the map is certain. The vendor U-Boot applied
 * the whole map here, but the map is a guess until gpiomap is set, and the
 * v2 map moves UART0 off GPIO 30/31 -- a v1 board would lose its console
 * before anyone could set gpiomap=v1. The other pins stay as the boot ROM
 * left them; the kernel applies the full map it is handed.
 */
#define GK_EPHY_CLK_PIN		48

/*
 * The SD pin functions: card detect, CMD, CLK, DATA0-3. Their pin types
 * are the same on both dies. With the map's SD power pin they are what
 * U-Boot needs for SD-card recovery; on boards without SD the map has
 * none of them.
 */
static bool gk_gpio_is_sd(u32 type)
{
	static const u32 sd[] = {
		0x50010f00, 0xd090181e, 0x9000001f,
		0xd0881016, 0xd0891117, 0xd08a1218, 0xd08b1319,
	};
	int i;

	for (i = 0; i < ARRAY_SIZE(sd); i++)
		if (type == sd[i])
			return true;
	return false;
}

void gk_gpio_map_apply(void)
{
	const struct gk_gpio_map *m = gk_map();
	int i;

	for (i = 0; i < m->nintphy; i++)
		if (m->intphy[i].pin == GK_EPHY_CLK_PIN)
			gk_gpio_config(m->intphy[i].pin, m->intphy[i].type);
	/*
	 * The SD pins only from a map known to be the board's: the GK710xS
	 * has one, the GK7101/GK7102 default is a guess until gpiomap is set,
	 * and the v2 SD pins are a v1 board's PHY reset, audio clock and PHY
	 * data lines (v1 boards have no SD).
	 */
	if (!IS_ENABLED(CONFIG_GK710X_S) && !env_get("gpiomap"))
		m = NULL;
	for (i = 0; m && i < m->npins; i++)
		if (gk_gpio_is_sd(m->pins[i].type) ||
		    m->pins[i].pin == m->sd_power)
			gk_gpio_config(m->pins[i].pin, m->pins[i].type);
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
#ifdef CONFIG_GK710X_S
	c->sd1_detect = m->sd1_detect;
	c->sd1_power = m->sd1_power;
#endif
	/* As the vendor numbers them: GK7101, GK7102, GK7101S, GK7102S */
	c->soc_type = IS_ENABLED(CONFIG_TARGET_GK7101) ? 0 :
		      IS_ENABLED(CONFIG_TARGET_GK7102) ? 1 :
		      IS_ENABLED(CONFIG_TARGET_GK7101S) ? 2 : 3;
	snprintf(c->board_type, sizeof(c->board_type), "openipc-%s", m->name);
	c->board_version = 0;
	c->ext_phy_clk = 0;
}
