/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
*/

#include <fmc_common.h>
#define BUFF_LEN	20

static unsigned char fmc_current_dev_type = FLASH_TYPE_DEFAULT;
static unsigned char fmc_boot_dev_type = FLASH_TYPE_DEFAULT;

unsigned char fmc_ip_user;
unsigned char fmc_cs_user[CONFIG_FMC_MAX_CS_NUM];

void *get_fmc_ip(void)
{
	return &fmc_ip_user;
}

unsigned char *get_cs_number(unsigned char cs)
{
	return fmc_cs_user + cs;
}

int fmc_ip_ver_check(void)
{
	unsigned int fmc_ip_ver;

	fmc_pr(FMC_INFO, "Check Flash Memory Controller v100 ...");
	fmc_ip_ver = readl(CONFIG_HIFMC_REG_BASE + FMC_VERSION);
	if (fmc_ip_ver != FMC_VER_100) {
		fmc_print("\n");
		return -EFAULT;
	}
	fmc_pr(FMC_INFO, " Found\n");

	return 0;
}

void fmc_dev_type_switch(unsigned char type)
{
	unsigned int reg, spi_device_type, flash_type;
	if (fmc_current_dev_type == type)
		return;

	fmc_pr(BT_DBG, "\t|*-Start switch current device type\n");

	if (type > FLASH_TYPE_DEFAULT) {
		fmc_pr(BT_DBG, "\t||-Switch unknown device type %d\n", type);
		return;
	}

	if (fmc_boot_dev_type == FLASH_TYPE_DEFAULT) {
		reg = readl((void *)(SYS_CTRL_REG_BASE + REG_SYSSTAT));
		fmc_pr(BT_DBG, "\t||-Get system STATUS[%#x]%#x\n",
		       SYS_CTRL_REG_BASE + REG_SYSSTAT, reg);
		fmc_boot_dev_type = get_spi_device_type(reg);
	}

	if (type == FLASH_TYPE_DEFAULT)
		spi_device_type = fmc_boot_dev_type;
	else
		spi_device_type = type;

	fmc_pr(BT_DBG, "\t||-Switch type to %s flash\n", str[type]);
	reg = readl((void *)(CONFIG_HIFMC_REG_BASE + FMC_CFG));
	fmc_pr(BT_DBG, "\t||-Get HIFMC CFG[%#x]%#x\n", FMC_CFG, reg);
	flash_type = (reg & FLASH_SEL_MASK) >> FLASH_SEL_SHIFT;
	if (spi_device_type != flash_type) {
		reg &= ~FLASH_SEL_MASK;
		reg |= fmc_cfg_flash_sel(spi_device_type);
		writel(reg, (void *)(CONFIG_HIFMC_REG_BASE + FMC_CFG));
		fmc_pr(BT_DBG, "\t||-Set HIFMC CFG[%#x]%#x\n", FMC_CFG, reg);
	}
	fmc_current_dev_type = spi_device_type;

	fmc_pr(BT_DBG, "\t|*-End switch current device type\n");
}

/* REG_SYSSTAT 0: 3 Bytes address boot mode; 1: 4Bytes address boot mode */
unsigned int get_fmc_boot_mode(void)
{
	unsigned int regval;
	unsigned int boot_mode;

	regval = readl(SYS_CTRL_REG_BASE + REG_SYSSTAT);
	boot_mode = get_spi_nor_addr_mode(regval);
	return boot_mode;
}

