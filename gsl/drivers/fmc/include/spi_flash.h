/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
*/

#ifndef _SPI_FLASH_H_
#define _SPI_FLASH_H_
#include "fmc_type.h"

#define MID_SPANSION    0x01    /* Spansion Manufacture ID */
#define MID_WINBOND     0xef    /* Winbond  Manufacture ID */
#define MID_MXIC        0xc2    /* MXIC Manufacture ID */
#define MID_MICRON      0x20    /* Micron Manufacture ID */
#define MID_GD          0xc8    /* GD Manufacture ID */
#define MID_ESMT        0x8c    /* ESMT Manufacture ID */
#define MID_CFEON       0x1c    /* CFeon Manufacture ID */
#define MID_MICRON      0x20    /* Micron Manufacture ID */
#define MID_PARAGON     0xe0    /* Paragon Manufacture ID */


// spi flash struct define
struct mtd_info_ex {
	u_char    type;  /* chip type  MTD_NORFLASH / MTD_NANDFLASH */
	uint64_t  chipsize; /* total size of the nand/spi chip */
	uint32_t erasesize;
	uint32_t pagesize;
	uint32_t numchips; /* number of nand chips */
	uint32_t addrcycle;
	u_char    ids[8];
	uint32_t  id_length;
	char      name[16]; /* chip names */
};

struct spi_flash {
	ssize_t (*read)(struct spi_flash *nor, loff_t from,
			size_t len, u_char *read_buf);
	const char *name;
	u32 size;
	u32 erase_size;
};

struct spi_flash *spi_flash_probe(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int spi_mode);

int spi_flash_read(struct spi_flash *flash, u32 offset,
		size_t len, void *buf);
#endif /* _SPI_FLASH_H_ */
