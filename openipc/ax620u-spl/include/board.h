/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef __BOARD_H__
#define __BOARD_H__

#define CHIP_MODE_STATUS	0x02220010	/* boot strap pins */
#define CHIP_SECURE_STATUS	0x0222001c
#define CHIP_SW_RESET		0x02220044
#define CHIP_CHIP_GRADE		0x02220114
#define SMP_BOOT_ADDR		0x02220300
#define SECURE_BOOT_EN		(1 << 26)

/* boot medium, from CHIP_MODE_STATUS */
enum flash_type {
	FLASH_NAND = 0,
	FLASH_NOR = 1,
	FLASH_NAND_2 = 2,
	FLASH_EMMC = 3,
	FLASH_SD = 4,
};

int is_secure_boot(void);
void timer_clk_init(void);
int clk_init(void);
void wakeup_slave(void);
void chip_grade_init(void);
void setup_boot_mode(int flash_type);
void chip_reset(void);
void emmc_clk_set(int on);
#endif
