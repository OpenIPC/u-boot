/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
*/

#include <config.h>
#include <fmc_common.h>
#include <spi_flash.h>
#include "vendor_spi.h"

static struct spi_flash *spiflash;
static int get_boot_media(void)
{
	unsigned int reg_val, boot_mode, spi_device_mode;
	int boot_media;
	
	reg_val = readl(SYS_CTRL_REG_BASE + REG_SYSSTAT);
	boot_mode = get_sys_boot_mode(reg_val);
	
	switch (boot_mode) {
	case BOOT_FROM_SPI:
		spi_device_mode = get_spi_device_type(reg_val);
		if (spi_device_mode)
		    boot_media = BOOT_MEDIA_NAND;
		else
		    boot_media = BOOT_MEDIA_SPIFLASH;
		break;
	case BOOT_FROM_EMMC:
		boot_media = BOOT_MEDIA_EMMC;
		break;
	default:
		boot_media = BOOT_MEDIA_UNKNOWN;
		break;
	}
	
	return boot_media;
}

struct spi_flash *spi_flash_probe(unsigned int bus, unsigned int cs,
				unsigned int max_hz, unsigned int spi_mode)
{
	struct mtd_info_ex *spiinfo_ex;

	if (get_boot_media() != BOOT_MEDIA_SPIFLASH) {
		return NULL;
	}

	if (spiflash)
		return spiflash;

#ifdef CONFIG_HIFMC_SPI_NOR
	spiflash = fmc100_spi_nor_probe(&spiinfo_ex);
	spiflash->erase_size = spiinfo_ex->erasesize;
#endif

	return spiflash;
}


int spi_flash_read(struct spi_flash *flash, u32 offset,
		size_t len, void *buf)
{
	return flash->read(flash, offset, len, buf);
}
