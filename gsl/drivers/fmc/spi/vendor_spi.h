/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
*/

#ifndef __HIFMC100_SPI_H__
#define __HIFMC100_SPI_H__

#include <spi_flash.h>

struct spi_flash *fmc100_spi_nor_probe(struct mtd_info_ex **);
struct mtd_info_ex *fmc100_get_spi_nor_info(struct spi_flash *);

#endif

