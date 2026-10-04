/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include <common.h>
#include <asm/io.h>
#include <linux/mtd/mtd.h>
#include <linux/bug.h>
#include "fmc100.h"

struct spi_nor_info *spinor_info;

set_read_std(0, INFINITE, 33);
set_read_std(0, INFINITE, 50);
set_read_std4b(0, INFINITE, 50);
set_read_std(0, INFINITE, 55);
set_read_std4b(0, INFINITE, 55);
set_read_std(0, INFINITE, 66);
set_read_std4b(0, INFINITE, 66);
set_read_std(0, INFINITE, 80);

set_read_fast(1, INFINITE, 80);
set_read_fast4b(1, INFINITE, 80);
set_read_fast(1, INFINITE, 86);
set_read_fast(1, INFINITE, 104);
set_read_fast4b(1, INFINITE, 104);
set_read_fast(1, INFINITE, 108);
set_read_fast4b(1, INFINITE, 108);
set_read_fast(1, INFINITE, 133);
set_read_fast4b(1, INFINITE, 133);

set_read_dual(1, INFINITE, 80);
set_read_dual4b(1, INFINITE, 80);
set_read_dual(1, INFINITE, 104);
set_read_dual4b(1, INFINITE, 104);
set_read_dual(1, INFINITE, 108);
set_read_dual4b(1, INFINITE, 108);
set_read_dual(1, INFINITE, 133);
set_read_dual4b(1, INFINITE, 133);

set_read_dual_addr(1, INFINITE, 80);
set_read_dual_addr4b(1, INFINITE, 80);
set_read_dual_addr4b(1, INFINITE, 90);
set_read_dual_addr4b(1, INFINITE, 104);
set_read_dual_addr4b(1, INFINITE, 108);
set_read_dual_addr(1, INFINITE, 104);
set_read_dual_addr(1, INFINITE, 108);
set_read_dual_addr(1, INFINITE, 133);
set_read_dual_addr4b(1, INFINITE, 133);

set_read_quad(1, INFINITE, 80);
set_read_quad4b(1, INFINITE, 80);
set_read_quad(1, INFINITE, 84);
set_read_quad(1, INFINITE, 104);
set_read_quad4b(1, INFINITE, 104);
set_read_quad(1, INFINITE, 108);
set_read_quad4b(1, INFINITE, 108);
set_read_quad(1, INFINITE, 133);
set_read_quad4b(1, INFINITE, 133);

set_read_quad_addr(3, INFINITE, 80);
set_read_quad_addr4b(3, INFINITE, 80);
set_read_quad_addr(3, INFINITE, 104);
set_read_quad_addr4b(3, INFINITE, 104);
set_read_quad_addr4b(3, INFINITE, 108);
set_read_quad_addr(3, INFINITE, 108);
set_read_quad_addr(3, INFINITE, 133);
set_read_quad_addr(3, INFINITE, 72);
set_read_quad_addr4b(3, INFINITE, 133);

#ifdef CONFIG_DTR_MODE_SUPPORT
set_read_quad_dtr(8, INFINITE, 80);
set_read_quad_dtr4b(10, INFINITE, 100);
set_read_quad_dtr(10, INFINITE, 100);
#endif
set_write_std(0, 256, 33);
set_write_std(0, 256, 80);
set_write_std4b(0, 256, 80);
set_write_std(0, 256, 86);
set_write_std(0, 256, 100);
set_write_std(0, 256, 104);
set_write_std4b(0, 256, 104);
set_write_std(0, 256, 108);
set_write_std(0, 256, 133);
set_write_std4b(0, 256, 120);
set_write_std4b(0, 256, 133);

set_write_quad(0, 256, 80);
set_write_quad4b(0, 256, 80);
set_write_quad(0, 256, 104);
set_write_quad4b(0, 256, 104);
set_write_quad(0, 256, 108);
set_write_quad(0, 256, 133);

/* As Micron MT25Q(and MIXC) and N25Q have different QUAD I/O write code,
 * but they have the same ID, so we cannot compatiable it. User can open
 * by theirselves. */
set_write_quad_addr(0, 256, 80);
set_write_quad_addr(0, 256, 104);
set_write_quad_addr(0, 256, 108);
set_write_quad_addr4b(0, 256, 104);
set_write_quad_addr4b(0, 256, 108);
set_write_quad_addr4b(0, 256, 120);
set_write_quad_addr(0, 256, 133);
set_write_quad_addr4b(0, 256, 133);

set_erase_sector_64k(0, _64K, 80);
set_erase_sector_64k(0, _64K, 86);
set_erase_sector_64k(0, _64K, 100);
set_erase_sector_64k(0, _64K, 104);
set_erase_sector_64k4b(0, _64K, 104);
set_erase_sector_64k(0, _64K, 108);
set_erase_sector_64k4b(0, _64K, 108);
set_erase_sector_64k4b(0, _64K, 120);
set_erase_sector_64k(0, _64K, 133);
set_erase_sector_64k4b(0, _64K, 133);

#include "fmc100_spi_general.c"
static struct spi_drv spi_driver_general = {
	.wait_ready = spi_general_wait_ready,
	.write_enable = spi_general_write_enable,
	.entry_4addr = spi_general_entry_4addr,
	.qe_enable = spi_general_qe_enable,
};

static struct spi_drv spi_driver_no_qe = {
	.wait_ready = spi_general_wait_ready,
	.write_enable = spi_general_write_enable,
	.entry_4addr = spi_general_entry_4addr,
	.qe_enable = spi_do_not_qe_enable,
};

#include "fmc100_spi_w25q256fv.c"
static struct spi_drv spi_driver_w25q256fv = {
	.wait_ready = spi_general_wait_ready,
	.write_enable = spi_general_write_enable,
	.entry_4addr = spi_w25q256fv_entry_4addr,
	.qe_enable = spi_w25q256fv_qe_enable,
};

#include "fmc100_spi_mx25l25635e.c"
static struct spi_drv spi_driver_mx25l25635e = {
	.wait_ready = spi_general_wait_ready,
	.write_enable = spi_general_write_enable,
	.entry_4addr = spi_general_entry_4addr,
	.qe_enable = spi_mx25l25635e_qe_enable,
#ifdef CONFIG_DTR_MODE_SUPPORT
	.dtr_set_device =
	spi_mxic_output_driver_strength_set,
#endif
};

#include "fmc100_spi_gd25qxxx.c"
static struct spi_drv spi_driver_gd25qxxx = {
	.wait_ready = spi_general_wait_ready,
	.write_enable = spi_general_write_enable,
	.entry_4addr = spi_general_entry_4addr,
	.qe_enable = spi_gd25qxxx_qe_enable,
};

#include "fmc100_spi_xtx.c"
static struct spi_drv spi_driver_xtx = {
	.wait_ready = spi_general_wait_ready,
	.write_enable = spi_general_write_enable,
	.entry_4addr = spi_general_entry_4addr,
	.qe_enable = spi_xtx_qe_enable,
};

#define SPI_NOR_ID_TBL_VER     "1.0"

/******************************************************************************
 * We do not guarantee the compatibility of the following device models in the
 * table.Device compatibility is based solely on the list of compatible devices
 * in the release package.
 ******************************************************************************/

static struct spi_nor_info fmc_spi_nor_info_table[] = {
	/* name     id  id_len  chipsize(Bytes) erasesize  */

	/* MXIC 3.3V MX25L1606EM2I-12G */
	{
		"MX25L1606EM2I-12G",  {0xc2, 0x20, 0x15}, 3, _2M,    _64K, 3,
		{
			/* dummy clock:1 byte, read size:INFINITE bytes,
			 * clock frq:33MHz */
			&read_std(0, INFINITE, 33), /* 33MHz */
			&read_fast(1, INFINITE, 86), /* 86MHz */
			&read_dual(1, INFINITE, 80), /* 80MHz */
			0
		},
		{
			/* dummy clock:0 byte, write size:256 bytes,
			 * clock frq:86MHz */
			&write_std(0, 256, 86), /* 86MHz */
			0
		},
		{
			/* dummy clock:0byte, erase size:64K,
			 * clock frq:86MHz */
			&erase_sector_64k(0, _64K, 86), /* 86MHz */
			0
		},
		&spi_driver_no_qe,
	},

	/* MXIC 3.3V MX25L6433FM2I-08G */
	{
		"MX25L6433FM2I-08G",  {0xc2, 0x20, 0x17}, 3, _8M,    _64K, 3,
		{
			&read_std(0, INFINITE, 50), /* 50MHz */
			&read_fast(1, INFINITE, 133), /* 133MHz */
			&read_dual(1, INFINITE, 133), /* 133MHz */
			&read_dual_addr(1, INFINITE, 133), /* 133MHz */
			&read_quad(1, INFINITE, 133), /* 133MHz */
			&read_quad_addr(3, INFINITE, 133), /* 133MHz */
			0
		},

		{
			&write_std(0, 256, 133), /* 133MHz */
			&write_quad_addr(0, 256, 133), /* 133MHz */
			0
		},

		{
			&erase_sector_64k(0, _64K, 133), /* 133MHz */
			0
		},
		&spi_driver_mx25l25635e,
	},

	/* MXIC 3.3V MX25L51245GZ2I-08G */
	{
		"MX25L51245GZ2I-08G",  {0xc2, 0x20, 0x1a}, 3, _64M,    _64K, 4,
		{
			&read_std4b(0, INFINITE, 66),
			&read_fast4b(1, INFINITE, 133),
			&read_dual4b(1, INFINITE, 133),
			&read_dual_addr4b(1, INFINITE, 133),
			&read_quad4b(1, INFINITE, 133),
			&read_quad_addr4b(3, INFINITE, 133),
			0
		},

		{
			&write_std4b(0, 256, 133),
			&write_quad_addr4b(0, 256, 133),
			0
		},

		{
			&erase_sector_64k4b(0, _64K, 133),
			0
		},
		&spi_driver_mx25l25635e,
	},

	/* MXIC 3.3V MX25V1635FM2I */
	{
		"MX25V1635FM2I",  {0xc2, 0x23, 0x15}, 3, _2M,    _64K, 3,
		{
			&read_std(0, INFINITE, 33), /* 33MHz */
			&read_fast(1, INFINITE, 80), /* 80MHz */
			&read_dual(1, INFINITE, 80), /* 80MHz */
			&read_dual_addr(1, INFINITE, 80), /* 80MHz */
			&read_quad(1, INFINITE, 80), /* 80MHz */
			&read_quad_addr(3, INFINITE, 80), /* 80MHz */
			0
		},

		{
			&write_std(0, 256, 33), /* 33MHz */
			&write_quad_addr(0, 256, 80), /* 80MHz */
			0
		},

		{
			&erase_sector_64k(0, _64K, 80), /* 80MHz */
			0
		},
		&spi_driver_mx25l25635e,
	},

	/* MXIC 1.8V MX25U6435FM2I-10G */
	{
		"MX25U6435FM2I-10G", {0xc2, 0x25, 0x37}, 3, _8M, _64K, 3,
		{
			&read_std(0, INFINITE, 50), /* 50MHz */
			&read_fast(1, INFINITE, 108), /* 108MHz */
			&read_dual(1, INFINITE, 108), /* 108MHz */
			&read_dual_addr(1, INFINITE, 108), /* 108MHz */
			&read_quad(1, INFINITE, 108), /* 108MHz */
			&read_quad_addr(3, INFINITE, 108), /* 108MHz */
			0
		},

		{
			&write_std(0, 256, 80), /* 80MHz */
			&write_quad_addr(0, 256, 108), /* 108MHz */
			0
		},

		{
			&erase_sector_64k(0, _64K, 108), /* 108MHz */
			0
		},
		&spi_driver_mx25l25635e,
	},

	/* MXIC 1.8V MX25U12835FM2I-10G/MX25U12832FM2I02 */
	{
		"MX25U12835F/32F", {0xc2, 0x25, 0x38}, 3, _16M, _64K, 3,
		{
			&read_std(0, INFINITE, 55), /* 55MHz */
			&read_fast(1, INFINITE, 108), /* 108MHz */
			&read_dual(1, INFINITE, 108), /* 108MHz */
			&read_dual_addr(1, INFINITE, 108), /* 108MHz */
			&read_quad(1, INFINITE, 108), /* 108MHz */
			&read_quad_addr(3, INFINITE, 108), /* 108MHz */
			0
		},

		{
			&write_std(0, 256, 80), /* 80MHz */
			&write_quad_addr(0, 256, 108), /* 108MHz */
			0
		},

		{
			&erase_sector_64k(0, _64K, 108), /* 108MHz */
			0
		},
		&spi_driver_mx25l25635e,
	},

	/* MXIC 3.3V MX25L12845GM2I-08G/MX25L12845GM2I-10G/MX25L12833FM2I-10G/MX25L12835FM2I-10G
	   The suffix "45G" indicates the flash supporting DTR mode */
	{
		"MX25L12835F/45G", {0xc2, 0x20, 0x18}, 3, _16M, _64K, 3,
		{
			&read_std(0, INFINITE, 50), /* 50MHz */
			&read_fast(1, INFINITE, 104), /* 104MHz */
			&read_dual(1, INFINITE, 104), /* 104MHz */
			&read_dual_addr(1, INFINITE, 104), /* 104MHz */
			&read_quad(1, INFINITE, 104), /* 104MHz */
			&read_quad_addr(3, INFINITE, 104), /* 104MHz */
#ifdef CONFIG_DTR_MODE_SUPPORT
			&read_quad_dtr(10, INFINITE, 100 /* 84 */), /* 100MHz */
#endif
			0
		},

		{
			&write_std(0, 256, 104), /* 104MHz */
			&write_quad_addr(0, 256, 104), /* 104MHz */
			0
		},

		{
			&erase_sector_64k(0, _64K, 104), /* 104MHz */
			0
		},
		&spi_driver_mx25l25635e,
	},

	/* MXIC 1.8V MX25U25635FZ4I-10G/MX25U25645GZ4I00   The suffix "45G" indicates the flash supporting DTR mode */
	{
		"MX25U25635F/45G", {0xc2, 0x25, 0x39}, 3, _32M, _64K, 4,
		{
			&read_std4b(0, INFINITE, 55), /* 55MHz */
			&read_fast4b(1, INFINITE, 108), /* 108MHz */
			&read_dual4b(1, INFINITE, 108), /* 108MHz */
			&read_dual_addr4b(1, INFINITE, 108), /* 108MHz */
			&read_quad4b(1, INFINITE, 108), /* 108MHz */
			&read_quad_addr4b(3, INFINITE, 108), /* 108MHz */
#ifdef CONFIG_DTR_MODE_SUPPORT
			&read_quad_dtr4b(10, INFINITE, 100), /* 100MHz */
#endif
			0
		},

		{
			&write_std4b(0, 256, 80), /* 80MHz */
			&write_quad_addr4b(0, 256, 108), /* 108MHz */
			0
		},

		{
			&erase_sector_64k4b(0, _64K, 108), /* 108MHz */
			0
		},
		&spi_driver_mx25l25635e,
	},

	/* MXIC 3.3V MX25L25635FMI-10G/MX25L25645G(MI-08G/MI-10G/M2I-08G)/MX25L25735F(MI-10G/Z2I-10G)
	   The suffix "45G" indicates the flash supporting DTR mode 3.3V */
	{
		"MX25L25635F/45G", {0xc2, 0x20, 0x19}, 3, _32M, _64K, 4,
		{
			&read_std4b(0, INFINITE, 50), /* 50MHz */
			&read_fast4b(1, INFINITE, 80), /* 80MHz */
			&read_dual4b(1, INFINITE, 80), /* 80MHz */
			&read_dual_addr4b(1, INFINITE, 80), /* 80MHz */
			&read_quad4b(1, INFINITE, 80), /* 80MHz */
			&read_quad_addr4b(3, INFINITE, 80), /* 80MHz */
#ifdef CONFIG_DTR_MODE_SUPPORT
			&read_quad_dtr4b(10, INFINITE, 100), /* 100MHz */
#endif
			0
		},

		{
			&write_std4b(0, 256, 120), /* 120MHz */
			&write_quad_addr4b(0, 256, 120), /* 120MHz */
			0
		},

		{
			&erase_sector_64k4b(0, _64K, 120), /* 120MHz */
			0
		},
		&spi_driver_mx25l25635e,
	},

	/* MXIC 1.8V MX25U51245GZ4I00 */
	{
		"MX25U51245GZ4I00", {0xc2, 0x25, 0x3a}, 3, _64M, _64K, 4,
		{
			&read_std4b(0, INFINITE, 66), /* 66MHz */
			&read_fast4b(1, INFINITE, 104), /* 104MHz */
			&read_dual4b(1, INFINITE, 104), /* 104MHz */
			&read_dual_addr4b(1, INFINITE, 104), /* 104MHz */
			&read_quad4b(1, INFINITE, 104), /* 104MHz */
			&read_quad_addr4b(3, INFINITE, 104), /* 104MHz */
#ifdef CONFIG_DTR_MODE_SUPPORT
			&read_quad_dtr4b(10, INFINITE, 100), /* 100MHz */
#endif
			0
		},

		{
			&write_std4b(0, 256, 104), /* 104MHz */
			&write_quad_addr4b(0, 256, 104), /* 104MHz */
			0
		},

		{
			&erase_sector_64k4b(0, _64K, 104), /* 104MHz */
			0
		},
		&spi_driver_mx25l25635e,
	},

	/* WINBOND 3.3V W25Q32JVSSIQ */
	{
		"W25Q32JVSSIQ", {0xef, 0x40, 0x16}, 3, (_64K * _64B), _64K, 3,
		{
			&read_std(0, INFINITE, 50), /* 50MHz */
			&read_fast(1, INFINITE, 80), /* 80MHz */
			&read_dual(1, INFINITE, 80), /* 80MHz */
			&read_quad(1, INFINITE, 80), /* 80MHz */
			0
		},

		{
			&write_std(0, 256, 80), /* 80MHz */
			&write_quad(0, 256, 80), /* 80MHz */
			0
		},

		{
			&erase_sector_64k(0, _64K, 80), /* 80MHz */
			0
		},
		&spi_driver_general,
	},

	/* WINBOND 1.8V W25Q32FWSSIG/W25Q32JWSSIQ 1.8V */
	{
		"W25Q32FW/JW",  {0xef, 0x60, 0x16}, 3, _4M,  _64K, 3,
		{
			&read_std(0, INFINITE, 50), /* 50MHz */
			&read_fast(1, INFINITE, 133), /* 133MHz */
			&read_dual(1, INFINITE, 133), /* 133MHz */
			&read_quad(1, INFINITE, 133), /* 133MHz */
			&read_quad_addr(3, INFINITE, 133), /* 133MHz */
			0
		},

		{
			&write_std(0, 256, 133), /* 133MHz */
			&write_quad(0, 256, 133), /* 133MHz */
			0
		},

		{
			&erase_sector_64k(0, _64K, 133), /* 133MHz */
			0
		},
		&spi_driver_w25q256fv,
	},

	/* WINBOND 3.3V W25Q64FVFIG/W25Q64FVSSIG/W25Q64JVSFIQ/W25Q64JVSSIQ */
	{
		"W25Q64FV/JV",  {0xef, 0x40, 0x17}, 3, _8M,   _64K, 3,
		{
			&read_std(0, INFINITE, 50), /* 50MHz */
			&read_fast(1, INFINITE, 80), /* 80MHz */
			&read_dual(1, INFINITE, 80), /* 80MHz */
			&read_quad(1, INFINITE, 80), /* 80MHz */
			0
		},

		{
			&write_std(0, 256, 80), /* 80MHz */
			&write_quad(0, 256, 80), /* 80MHz */
			0
		},

		{
			&erase_sector_64k(0, _64K, 80), /* 80MHz */
			0
		},
		&spi_driver_general,
	},

	/* WINBOND 1.8V W25Q64FWSSIG/W25Q64JWSSIQ */
	{
		"W25Q64FW/JW",  {0xef, 0x60, 0x17}, 3, _8M,   _64K, 3,
		{
			&read_std(0, INFINITE, 50), /* 50MHz */
			&read_fast(1, INFINITE, 104), /* 104MHz */
			&read_dual(1, INFINITE, 104), /* 104MHz */
			&read_quad(1, INFINITE, 80), /* 80MHz */
			&read_quad_addr(3, INFINITE, 104), /* 104MHz */
			0
		},

		{
			&write_std(0, 256, 104), /* 104MHz */
			&write_quad(0, 256, 80), /* 80MHz */
			0
		},

		{
			&erase_sector_64k(0, _64K, 104), /* 104MHz */
			0
		},
		&spi_driver_w25q256fv,
	},

	/* WINBOND 1.8V W25Q128FWSIG/W25Q128FWSIQ/W25Q128JWSIQ */
	{
		"W25Q128FW/JW",  {0xef, 0x60, 0x18}, 3, _16M,   _64K, 3,
		{
			&read_std(0, INFINITE, 50), /* 50MHz */
			&read_fast(1, INFINITE, 104), /* 104MHz */
			&read_dual(1, INFINITE, 104), /* 104MHz */
			&read_dual_addr(1, INFINITE, 80), /* 80MHz */
			&read_quad(1, INFINITE, 80), /* 80MHz */
			&read_quad_addr(3, INFINITE, 104), /* 104MHz */
			0
		},

		{
			&write_std(0, 256, 104), /* 104MHz */
			&write_quad(0, 256, 80), /* 80MHz */
			0
		},

		{
			&erase_sector_64k(0, _64K, 104), /* 104MHz */
			0
		},
		&spi_driver_w25q256fv,
	},

	/* WINBOND 3.3V W25Q128JV(EAQ/SIQ/FIQ)/W25Q128FVSIG/W25Q128FVSIQ */
	{
		"W25Q128(B/F)V", {0xEF, 0x40, 0x18}, 3, _16M, _64K, 3,
		{
			&read_std(0, INFINITE, 33), /* 33MHz */
			&read_fast(1, INFINITE, 104), /* 104MHz */
			&read_dual(1, INFINITE, 104), /* 104MHz */
			&read_quad(1, INFINITE, /* 70 */ 80), /* 80MHz */
			0
		},

		{
			&write_std(0, 256, 104), /* 104MHz */
			&write_quad(0, 256, /* 70 */ 80), /* 80MHz */
			0
		},

		{
			&erase_sector_64k(0, _64K, 104), /* 104MHz */
			0
		},
		&spi_driver_w25q256fv,
	},

	/* WINBOND 3.3V W25Q128JVSIM    support DTR mode */
	{
		"W25Q128JVSIM", {0xEF, 0x70, 0x18}, 3, _16M, _64K, 3,
		{
			&read_std(0, INFINITE, 33), /* 33MHz */
			&read_fast(1, INFINITE, 104), /* 104MHz */
			&read_dual(1, INFINITE, 104), /* 104MHz */
			&read_quad(1, INFINITE, /* 70 */80), /* 80MHz */
#ifdef CONFIG_DTR_MODE_SUPPORT
			&read_quad_dtr(8, INFINITE, 80), /* 80MHz */
#endif
			0
		},

		{
			&write_std(0, 256, 104), /* 104MHz */
			&write_quad(0, 256, /* 70 */ 80), /* 80MHz */
			0
		},

		{
			&erase_sector_64k(0, _64K, 104), /* 104MHz */
			0
		},
		&spi_driver_w25q256fv,
	},

	/* WINBOND 3.3V W25Q256JVFIQ */
	{
		"W25Q256JVFIQ", {0xEF, 0x40, 0x19}, 3, _32M, _64K, 4,
#ifdef CONFIG_AUTOMOTIVE_GRADE
		{
			&read_std(0, INFINITE, 50),  /* 50MHz */
			&read_fast(1, INFINITE, 80), /* 80MHz */
			&read_dual(1, INFINITE, 80), /* 80MHz */
			&read_dual_addr(1, INFINITE, 80), /* 80MHz */
			&read_quad(1, INFINITE, 80), /* 80MHz */
			&read_quad_addr(3, INFINITE, 80), /* 80MHz */
			0
		},

		{
			&write_std(0, 256, 80), /* 80MHz */
			&write_quad(0, 256, 80),  /* 80MHz */
			0
		},

		{
			&erase_sector_64k(0, _64K, 104),  /* 104MHz */
			0
		},
#else
		{
			&read_std4b(0, INFINITE, 50),  /* 50MHz */
			&read_fast4b(1, INFINITE, 80), /* 80MHz */
			&read_dual4b(1, INFINITE, 80), /* 80MHz */
			&read_dual_addr4b(1, INFINITE, 80), /* 80MHz */
			&read_quad4b(1, INFINITE, 80), /* 80MHz */
			&read_quad_addr4b(3, INFINITE, 80), /* 80MHz */
			0
		},

		{
			&write_std4b(0, 256, 80), /* 80MHz */
			&write_quad4b(0, 256, 80),  /* 80MHz */
			0
		},

		{
			&erase_sector_64k4b(0, _64K, 104),  /* 104MHz */
			0
		},
#endif
		&spi_driver_w25q256fv,
	},

	/* WINBOND 3.3V W25Q01JVZEIQ */
	{
		"W25Q01JVZEIQ", {0xEF, 0x40, 0x21}, 3, _128M, _64K, 4,
		{
			&read_std4b(0, INFINITE, 50),  /* 50MHz */
			&read_fast4b(1, INFINITE, 104), /* 104MHz */
			&read_dual4b(1, INFINITE, 104), /* 104MHz */
			&read_dual_addr4b(1, INFINITE, 90), /* 90MHz */
			&read_quad4b(1, INFINITE, 104), /* 104MHz */
			&read_quad_addr4b(3, INFINITE, 104), /* 104MHz */
			0
		},

		{
			&write_std4b(0, 256, 104), /* 104MHz */
			&write_quad4b(0, 256, 104), /* 104MHz */
			0
		},

		{
			&erase_sector_64k4b(0, _64K, 104), /* 104MHz */
			0
		},
		&spi_driver_w25q256fv,
	},

	/* ESMT 3.3V EN25QH32B-104HIP2B */
	{
		"EN25QH32B-104HIP2B", {0x1c, 0x70, 0x16}, 3, _4M,  _64K, 3,
		{
			&read_std(0, INFINITE, 50), /* 50MHz */
			&read_fast(1, INFINITE, 133), /* 133MHz */
			&read_dual(1, INFINITE, 133), /* 133MHz */
			&read_dual_addr(1, INFINITE, 133), /* 133MHz */
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&read_quad(1, INFINITE, 133), /* 133MHz */
			&read_quad_addr(3, INFINITE, 133), /* 133MHz */
#endif
			0
		},

		{
			&write_std(0, 256, 133), /* 133MHz */
			&write_quad(0, 256, 133), /* 133MHz */
			0
		},

		{
			&erase_sector_64k(0, _64K, 133), /* 133MHz */
			0
		},
		&spi_driver_no_qe,
	},

	/* EXMT 3.3V EN25QH64A-104HIP */
	{
		"EN25QH64A-104HIP", {0x1c, 0x70, 0x17}, 3, (_64K * _128B),  _64K, 3,
		{
			&read_std(0, INFINITE, 50), /* 50MHz */
			&read_fast(1, INFINITE, 104), /* 104MHz */
			&read_dual(1, INFINITE, 104), /* 104MHz */
			&read_dual_addr(1, INFINITE, 104), /* 104MHz */
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&read_quad(1, INFINITE, 104), /* 104MHz */
			&read_quad_addr(3, INFINITE, 104), /* 104MHz */
#endif
			0
		},

		{
			&write_std(0, 256, 80), /* 80MHz */
			0
		},

		{
			&erase_sector_64k(0, _64K, 104), /* 104MHz */
			0
		},
		&spi_driver_no_qe,
	},

	/* ESMT 3.3V EN25QH128A-104HIP2T */
	{
		"EN25QH128A-104HIP2T", {0x1c, 0x70, 0x18}, 3, _16M,  _64K, 3,
		{
			&read_std(0, INFINITE, 50), /* 50MHz */
			&read_fast(1, INFINITE, 104), /* 104MHz */
			&read_dual(1, INFINITE, 104), /* 104MHz */
			&read_dual_addr(1, INFINITE, 104), /* 104MHz */
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&read_quad(1, INFINITE, 104), /* 104MHz */
			&read_quad_addr(3, INFINITE, 104), /* 104MHz */
#endif
			0
		},

		{
			&write_std(0, 256, 104), /* 104MHz */
			&write_quad(0, 256, 104), /* 104MHz */
			0
		},

		{
			&erase_sector_64k(0, _64K, 104), /* 104MHz */
			0
		},
		&spi_driver_no_qe,
	},

	/* GD 1.8V GD25LQ128CSIG */
	{
		"GD25LQ128CSIG", {0xC8, 0x60, 0x18}, 3, _16M,  _64K, 3,
		{
			&read_std(0, INFINITE, 80), /* 80MHz */
			&read_fast(1, INFINITE, 80), /* 80MHz */
			&read_dual(1, INFINITE, 80), /* 80MHz */
			&read_dual_addr(1, INFINITE, 80), /* 80MHz */
			&read_quad(1, INFINITE, 80), /* 80MHz */
			&read_quad_addr(3, INFINITE, 80), /* 80MHz */
			0
		},
		{
			&write_std(0, 256, 80), /* 80MHz */
			&write_quad(0, 256, 80), /* 80MHz */
			0
		},
		{
			&erase_sector_64k(0, _64K, 80), /* 80MHz */
			0
		},
		&spi_driver_gd25qxxx,
	},

	/* GD 3.3V GD25Q128ESIG/GD25Q128CSIG/GD25Q127CSIG */
	{
		"GD25Q128XX", {0xC8, 0x40, 0x18}, 3, _16M,  _64K, 3,
		{
			&read_std(0, INFINITE, 80), /* 80MHz */
			&read_fast(1, INFINITE, 80), /* 80MHz */
			&read_dual(1, INFINITE, 104), /* 104MHz */
			&read_dual_addr(1, INFINITE, 104), /* 104MHz */
			&read_quad(1, INFINITE, 104), /* 104MHz */
			&read_quad_addr(3, INFINITE, 104), /* 104MHz */
			0
		},
		{
			&write_std(0, 256, 100), /* 100MHz */
			&write_quad(0, 256, 80), /* 80MHz */
			0
		},
		{
			&erase_sector_64k(0, _64K, 100), /* 100MHz */
			0
		},
		&spi_driver_gd25qxxx,
	},

	/* GD 3.3V GD25Q64ESIG/GD25Q64CSIG */
	{
		"GD25Q64XX", {0xC8, 0x40, 0x17}, 3, _8M,  _64K, 3,
		{
			&read_std(0, INFINITE, 66),  /* 66MHz */
			&read_quad(1, INFINITE, 80), /* 80MHz */
			0
		},
		{
			&write_std(0, 256, 100),  /* 100MHz */
			&write_quad(0, 256, 80),  /* 80MHz */
			0
		},
		{
			&erase_sector_64k(0, _64K, 100),  /* 100MHz */
			0
		},
		&spi_driver_gd25qxxx,
	},

	/* GD 3.3V GD25Q16ESIG */
	{
		"GD25Q16ESIG", {0xC8, 0x40, 0x15}, 3, _2M,  _64K, 3,
		{
			&read_std(0, INFINITE, 80), /* 80MHz */
			&read_fast(1, INFINITE, 80), /* 80MHz */
			&read_dual(1, INFINITE, 80), /* 80MHz */
			&read_dual_addr(1, INFINITE, 80), /* 80MHz */
			&read_quad(1, INFINITE, 80), /* 80MHz */
			&read_quad_addr(3, INFINITE, 80), /* 80MHz */
			0
		},
		{
			&write_std(0, 256, 80), /* 80MHz */
			&write_quad(0, 256, 80),  /* 80MHz */
			0
		},
		{
			&erase_sector_64k(0, _64K, 80), /* 80MHz */
			0
		},
		&spi_driver_gd25qxxx,
	},

	/* GD 1.8V GD25LQ16ESIG */
	{
		"GD25LQ16ESIG", {0xC8, 0x60, 0x15}, 3, _2M,  _64K, 3,
		{
			&read_std(0, INFINITE, 80), /* 80MHz */
			&read_fast(1, INFINITE, 80), /* 80MHz */
			&read_dual(1, INFINITE, 80), /* 80MHz */
			&read_dual_addr(1, INFINITE, 80), /* 80MHz */
			&read_quad(1, INFINITE, 80), /* 80MHz */
			&read_quad_addr(3, INFINITE, 104), /* 80MHz */
			0
		},
		{
			&write_std(0, 256, 80), /* 80MHz */
			&write_quad(0, 256, 104),  /* 80MHz */
			0
		},
		{
			&erase_sector_64k(0, _64K, 104), /* 80MHz */
			0
		},
		&spi_driver_gd25qxxx,
	},

	/* GD 1.8V GD25LQ64CSIGR */
	{
		"GD25LQ64CSIGR", {0xC8, 0x60, 0x17}, 3, _8M,  _64K, 3,
		{
			&read_std(0, INFINITE, 80), /* 80MHz */
			&read_fast(1, INFINITE, 80), /* 80MHz */
			&read_dual(1, INFINITE, 80), /* 80MHz */
			&read_dual_addr(1, INFINITE, 80), /* 80MHz */
			&read_quad(1, INFINITE, 80), /* 80MHz */
			&read_quad_addr(3, INFINITE, 80), /* 80MHz */
			0
		},
		{
			&write_std(0, 256, 80), /* 80MHz */
			&write_quad(0, 256, 80),  /* 80MHz */
			0
		},
		{
			&erase_sector_64k(0, _64K, 80), /* 80MHz */
			0
		},
		&spi_driver_gd25qxxx,
	},

	/* GD 3.3V GD25Q32ESIG */
	{
		"GD25Q32ESIG", {0xC8, 0x40, 0x16}, 3, _4M,  _64K, 3,
		{
			&read_std(0, INFINITE, 66),  /* 66MHz */
			&read_quad(1, INFINITE, 80), /* 80MHz */
			0
		},
		{
			&write_std(0, 256, 100),  /* 100MHz */
			&write_quad(0, 256, 80),  /* 80MHz */
			0
		},
		{
			&erase_sector_64k(0, _64K, 100),  /* 100MHz */
			0
		},
		&spi_driver_gd25qxxx,
	},

	/* XMC 3.3V XM25QH64AHIG */
	{
		"XM25QH64AHIG", {0x20, 0x70, 0x17}, 3, _8M,  _64K, 3,
		{
			&read_std(0, INFINITE, 80), /* 80MHz */
			&read_fast(1, INFINITE, 104), /* 104MHz */
			&read_dual(1, INFINITE, 104), /* 104MHz */
			&read_dual_addr(1, INFINITE, 104), /* 104MHz */
			&read_quad(1, INFINITE, 104), /* 104MHz */
			&read_quad_addr(3, INFINITE, 104), /* 104MHz */
			0
		},
		{
			&write_std(0, 256, 104), /* 104MHz */
			&write_quad(0, 256, 104),  /* 104MHz */
			0
		},
		{
			&erase_sector_64k(0, _64K, 104), /* 104MHz */
			0
		},
		&spi_driver_no_qe,
	},

	/* XMC 3.3V XM25QH128AHIG */
	{
		"XM25QH128AHIG", {0x20, 0x70, 0x18}, 3, _16M,  _64K, 3,
		{
			&read_std(0, INFINITE, 80), /* 80MHz */
			&read_fast(1, INFINITE, 104), /* 104MHz */
			&read_dual(1, INFINITE, 104), /* 104MHz */
			&read_dual_addr(1, INFINITE, 104), /* 104MHz */
			&read_quad(1, INFINITE, 104), /* 104MHz */
			&read_quad_addr(3, INFINITE, 104), /* 104MHz */
			0
		},
		{
			&write_std(0, 256, 104), /* 104MHz */
			&write_quad(0, 256, 104),  /* 104MHz */
			0
		},
		{
			&erase_sector_64k(0, _64K, 104), /* 104MHz */
			0
		},
		&spi_driver_no_qe,
	},

	/* XMC 3.3V XM25QH64BHIQ */
	{
		"XM25QH64BHIQ", {0x20, 0x60, 0x17}, 3, _8M,  _64K, 3,
		{
			&read_std(0, INFINITE, 80), /* 80MHz */
			&read_fast(1, INFINITE, 104), /* 104MHz */
			&read_dual(1, INFINITE, 104), /* 104MHz */
			&read_dual_addr(1, INFINITE, 104), /* 104MHz */
			&read_quad(1, INFINITE, 104), /* 104MHz */
			&read_quad_addr(3, INFINITE, 104), /* 104MHz */
			0
		},
		{
			&write_std(0, 256, 104), /* 104MHz */
			&write_quad(0, 256, 104),  /* 104MHz */
			0
		},
		{
			&erase_sector_64k(0, _64K, 104), /* 104MHz */
			0
		},
		&spi_driver_no_qe,
	},

	/* XMC 3.3V XM25QH128BHIQ */
	{
		"XM25QH128BHIQ", {0x20, 0x60, 0x18}, 3, _16M,  _64K, 3,
		{
			&read_std(0, INFINITE, 50), /* 50MHz */
			&read_fast(1, INFINITE, 104), /* 104MHz */
			&read_dual(1, INFINITE, 104), /* 104MHz */
			&read_dual_addr(1, INFINITE, 104), /* 104MHz */
			&read_quad(1, INFINITE, 104), /* 104MHz */
			&read_quad_addr(3, INFINITE, 104), /* 104MHz */
			0
		},
		{
			&write_std(0, 256, 104), /* 104MHz */
			&write_quad(0, 256, 104),  /* 104MHz */
			0
		},
		{
			&erase_sector_64k(0, _64K, 104), /* 104MHz */
			0
		},
		&spi_driver_no_qe,
	},

	/* XMC 3.3V XM25QH64CHIQ */
	{
		"XM25QH64CHIQ", {0x20, 0x40, 0x17}, 3, _8M,  _64K, 3,
		{
			&read_std(0, INFINITE, 50), /* 50MHz */
			&read_fast(1, INFINITE, 133), /* 133MHz */
			&read_dual(1, INFINITE, 133), /* 133MHz */
			&read_dual_addr(1, INFINITE, 133), /* 133MHz */
			&read_quad(1, INFINITE, 133), /* 133MHz */
			&read_quad_addr(3, INFINITE, 133), /* 133MHz */
			0
		},
		{
			&write_std(0, 256, 133), /* 133MHz */
			&write_quad(0, 256, 133),  /* 133MHz */
			0
		},
		{
			&erase_sector_64k(0, _64K, 133), /* 133MHz */
			0
		},
		&spi_driver_general,
	},

	/* XMC 3.3V XM25QH128CHIQ */
	{
		"XM25QH128CHIQ", {0x20, 0x40, 0x18}, 3, _16M,  _64K, 3,
		{
			&read_std(0, INFINITE, 50), /* 50MHz */
			&read_fast(1, INFINITE, 133), /* 133MHz */
			&read_dual(1, INFINITE, 133), /* 133MHz */
			&read_dual_addr(1, INFINITE, 133), /* 133MHz */
			&read_quad(1, INFINITE, 133), /* 133MHz */
			&read_quad_addr(3, INFINITE, 133), /* 133MHz */
			0
		},
		{
			&write_std(0, 256, 133), /* 133MHz */
			&write_quad(0, 256, 133),  /* 133MHz */
			0
		},
		{
			&erase_sector_64k(0, _64K, 133), /* 133MHz */
			0
		},
		&spi_driver_general,
	},

	/* XTX 3.3V XT25F128BSSIGU */
	{
		"XT25F128BSSIGU", {0x0B, 0x40, 0x18}, 3, _16M,  _64K, 3,
		{
			&read_std(0, INFINITE, 80), /* 80MHz */
			&read_dual(1, INFINITE, 108), /* 108MHz */
			&read_dual_addr(1, INFINITE, 108), /* 108MHz */
			&read_quad(1, INFINITE, 108), /* 108MHz */
			&read_quad_addr(3, INFINITE, 72),  /* 72MHz */
			0
		},
		{
			&write_std(0, 256, 80), /* 80MHz */
			&write_quad(0, 256, 80),  /* 80MHz */
			0
		},
		{
			&erase_sector_64k(0, _64K, 80), /* 80MHz */
			0
		},
		&spi_driver_xtx,
	},

	/* XTX 3.3V XT25F64BSSIGU-S */
	{
		"XT25F64BSSIGU-S", {0x0B, 0x40, 0x17}, 3, _8M,  _64K, 3,
		{
			&read_std(0, INFINITE, 80), /* 80MHz */
			&read_dual(1, INFINITE, 108), /* 108MHz */
			&read_dual_addr(1, INFINITE, 108), /* 108MHz */
			&read_quad(1, INFINITE, 84),  /* 84MHz */
			&read_quad_addr(3, INFINITE, 72),  /* 72MHz */
			0
		},
		{
			&write_std(0, 256, 80), /* 80MHz */
			&write_quad(0, 256, 80),  /* 80MHz */
			0
		},
		{
			&erase_sector_64k(0, _64K, 80), /* 80MHz */
			0
		},
		&spi_driver_xtx,
	},

	/* XTX 3.3V XT25F32BSSIGU-S */
	{
		"XT25F32BSSIGU-S", {0x0B, 0x40, 0x16}, 3, _4M,  _64K, 3,
		{
			&read_std(0, INFINITE, 80), /* 80MHz */
			&read_fast(1, INFINITE, 108), /* 108MHz */
			&read_dual(1, INFINITE, 108), /* 108MHz */
			&read_dual_addr(1, INFINITE, 108), /* 108MHz */
			&read_quad(1, INFINITE, 108),  /* 108MHz */
			&read_quad_addr(3, INFINITE, 108),  /* 108MHz */
			0
		},
		{
			&write_std(0, 256, 108), /* 108MHz */
			&write_quad(0, 256, 108),  /* 108MHz */
			0
		},
		{
			&erase_sector_64k(0, _64K, 108), /* 108MHz */
			0
		},
		&spi_driver_xtx,
	},

	/* FM 3.3V FM25Q64-SOB-T-G 3.3V */
	{
		"FM25Q64-SOB-T-G", {0xa1, 0x40, 0x17}, 3, _8M,  _64K, 3,
		{
			&read_std(0, INFINITE, 66),
			&read_fast(1, INFINITE, 104),
			&read_dual(1, INFINITE, 104),
			&read_dual_addr(1, INFINITE, 104),
			&read_quad(1, INFINITE, 104),
			&read_quad_addr(3, INFINITE, 104),
			0
		},
		{
			&write_std(0, 256, 104),
			&write_quad(0, 256, 104),
			0
		},
		{
			&erase_sector_64k(0, _64K, 104),
			0
		},
		&spi_driver_general,
	},

	/* FM 3.3V FM25Q128A-SOB-T-G 3.3V */
	{
		"FM25Q128A-SOB-T-G", {0xa1, 0x40, 0x18}, 3, _16M,  _64K, 3,
		{
			&read_std(0, INFINITE, 50),
			&read_fast(1, INFINITE, 104),
			&read_dual(1, INFINITE, 104),
			&read_dual_addr(1, INFINITE, 104),
			&read_quad(1, INFINITE, 104),
			&read_quad_addr(3, INFINITE, 104),
			0
		},
		{
			&write_std(0, 256, 104),
			&write_quad(0, 256, 104),
			0
		},
		{
			&erase_sector_64k(0, _64K, 104),
			0
		},
		&spi_driver_general,
	},

	{
		"BY25Q64ES",  {0x68, 0x40, 0x17}, 3, _8M,   _64K, 3,
		{
			&read_std(0, INFINITE, 50), /* 50MHz */
			0
		},

		{
			&write_std(0, 256, 33), /* 80MHz */
			0
		},

		{
			&erase_sector_64k(0, _64K, 80), /* 80MHz */
			0
		},
		&spi_driver_general,
	},

	{0, {0}, 0, 0, 0, 0, {0}, {0}, {0}, NULL},
};

static struct spi_nor_info *fmc_spi_nor_serach_ids(u_char* const ids, int len)
{
	struct spi_nor_info *info = fmc_spi_nor_info_table;
	struct spi_nor_info *fit_info = NULL;

	if (len <= 0)
		return NULL;

	for (; info->name; info++) {
		if (memcmp(info->id, ids, info->id_len))
			continue;

		if ((fit_info == NULL) || (fit_info->id_len < info->id_len))
			fit_info = info;
	}
	return fit_info;
}

static void fmc_spi_nor_search_rw(struct spi_nor_info *info,
				struct spi_op *spiop_rw,
				u_int iftype,
				u_int max_dummy,
				int rw_type)
{
	int ix = 0;
	struct spi_op **spiop, **fitspiop;

	for (fitspiop = spiop = (rw_type ? info->write : info->read);
	     (*spiop) && ix < MAX_SPI_OP; spiop++, ix++)
		if (((*spiop)->iftype & iftype) &&
			((*spiop)->dummy <= max_dummy) &&
			((*fitspiop)->iftype < (*spiop)->iftype))
			fitspiop = spiop;

	if (memcpy_s(spiop_rw, sizeof(struct spi_op), (*fitspiop), sizeof(struct spi_op)))
		printf("%s %d ERR:memcpy_s fail !\n", __func__, __LINE__);
}

static void fmc_map_iftype_and_clock(struct fmc_spi *spi)
{
	int ix;
	const int iftype_read[] = {
		SPI_IF_READ_STD,    IF_TYPE_STD,
		SPI_IF_READ_FAST,   IF_TYPE_STD,
		SPI_IF_READ_DUAL,   IF_TYPE_DUAL,
		SPI_IF_READ_DUAL_ADDR,  IF_TYPE_DIO,
		SPI_IF_READ_QUAD,   IF_TYPE_QUAD,
		SPI_IF_READ_QUAD_ADDR,  IF_TYPE_QIO,
#ifdef CONFIG_DTR_MODE_SUPPORT
		SPI_IF_READ_QUAD_DTR,   IF_TYPE_DTR,
#endif
		0,          0,
	};
	const int iftype_write[] = {
		SPI_IF_WRITE_STD,   IF_TYPE_STD,
		SPI_IF_WRITE_DUAL,  IF_TYPE_DUAL,
		SPI_IF_WRITE_DUAL_ADDR, IF_TYPE_DIO,
		SPI_IF_WRITE_QUAD,  IF_TYPE_QUAD,
		SPI_IF_WRITE_QUAD_ADDR, IF_TYPE_QIO,
		0,          0,
	};

	if (!spi->write || !spi->read || !spi->erase) {
		printf("spi nor err:spi->func is null!\n");
		return;
	}
	/* Only an even number of values is required,so increase length is 2 */
	for (ix = 0; iftype_write[ix]; ix += 2) {
		if (spi->write->iftype == iftype_write[ix]) {
			spi->write->iftype = iftype_write[ix + 1];
			break;
		}
	}
	fmc_get_fmc_best_2x_clock(&spi->write->clock);

	/* Only an even number of values is required,so increase length is 2 */
	for (ix = 0; iftype_read[ix]; ix += 2) {
		if (spi->read->iftype == iftype_read[ix]) {
			spi->read->iftype = iftype_read[ix + 1];
			break;
		}
	}
#ifdef CONFIG_DTR_MODE_SUPPORT
	if (spi->dtr_mode_support)
		/* get the div4 clock */
		fmc_get_fmc_best_4x_clock(&spi->read->clock);
	else
		fmc_get_fmc_best_2x_clock(&spi->read->clock);
#else
	fmc_get_fmc_best_2x_clock(&spi->read->clock);
#endif

	fmc_get_fmc_best_2x_clock(&spi->erase->clock);
	spi->erase->iftype = IF_TYPE_STD;
}

void fmc_spi_nor_get_erase(struct spi_nor_info *info,
				struct spi_op *spiop_erase)
{
	int ix;
	if (!info || !spiop_erase)
		return;

	spiop_erase->size = 0;
	for (ix = 0; ix < MAX_SPI_OP; ix++) {
		if (info->erase[ix] == NULL)
			break;
		if (info->erasesize == info->erase[ix]->size) {
			if (memcpy_s(&spiop_erase[ix], sizeof(struct spi_op),
				info->erase[ix], sizeof(struct spi_op)))
				printf("%s %d ERR:memcpy_s fail !\n", __func__, __LINE__);
			break;
		}
	}
}

static void switch_to_4byte(struct fmc_spi* const spi, u_char* const ids, int len)
{
	unsigned char manu_id;
	/* auto check fmc_addr_mode 3 bytes or 4 bytes */
	unsigned int start_up_addr_mode = get_fmc_boot_mode();

	if (len < 3)
		return;

	manu_id = ids[0];
	if ((spi->addrcycle == SPI_NOR_3BYTE_ADDR_LEN)
			&& (start_up_addr_mode == SPI_NOR_ADDR_MODE_4_BYTES))
		printf("\nError!!! the flash's addres len is 3bytes and start \
			up address mode is 4bytes,please set the start up \
				address mode to 3bytes mode");
	if ((spi->addrcycle == SPI_NOR_4BYTE_ADDR_LEN)
			&& (start_up_addr_mode == SPI_NOR_ADDR_MODE_3_BYTES)) {
		switch (manu_id) {
		case MID_WINBOND:
#ifdef CONFIG_AUTOMOTIVE_GRADE
			if ((ids[1] == 0x40) && (ids[2] == 0x19)) { /* W25Q256FV/W25Q256JV */
				spi->driver->entry_4addr(spi, ENABLE);
				break;
			}
#endif
		case MID_MXIC:
		case MID_MICRON:
			fmc_pr(BT_DBG, "\t|||-4-Byte Command Operation\n");
			break;
		default:
				fmc_pr(BT_DBG, "\t|||-start up: 3-Byte mode\n");
				spi->driver->entry_4addr(spi, ENABLE);
			break;
		}
	} else {
		fmc_pr(BT_DBG, "\t|||-start up: 4-Byte mode or 4-Byte Command\n");
	}
}

static void spi_data_init(struct fmc_spi *spi, struct spi_nor_info *spiinfo,
				unsigned char cs)
{
	spi->name = spiinfo->name;
	spi->chipselect = cs;
	spi->chipsize = spiinfo->chipsize;
	spi->erasesize = spiinfo->erasesize;
	spi->addrcycle = spiinfo->addrcycle;
	spi->driver = spiinfo->driver;
}
static void mtd_data_set(struct mtd_info_ex* const mtd, struct spi_nor_info* const spiinfo,
				struct fmc_spi *spi)
{
	if (mtd->type == 0) {
		mtd->type = MTD_NORFLASH;
		mtd->chipsize = spi->chipsize;
		mtd->erasesize = spi->erasesize;
		mtd->pagesize = 1;
		mtd->addrcycle = spi->addrcycle;

		if (spiinfo->id_len > sizeof(mtd->ids)) {
			printf("BUG! ID len out of range.\n");
			BUG();
		}

		mtd->id_length = spiinfo->id_len;
		if (memcpy_s(mtd->ids, spiinfo->id_len, spiinfo->id, spiinfo->id_len))
			printf("%s %d ERR:memcpy_s fail !\n", __func__, __LINE__);
		if (strlen(spi->name) > sizeof(mtd->name)) {
			printf("BUG! spi->name len err %d.\n", __LINE__);
			return;
		}

		if (strncpy_s(mtd->name, sizeof(mtd->name) - 1,
			spi->name, sizeof(mtd->name) - 1)) {
			printf("BUG! mtd->name len err %s %d.\n", __func__, __LINE__);
			return;
		}
		mtd->name[sizeof(mtd->name) - 1] = '\0';
	}
}

static void fmc_init_print(struct fmc_spi* const spi)
{
	const char *str[] = {"STD", "DUAL", "DIO", "QUAD", "QIO"};

	fmc_pr(FMC_INFO, "Block:%sB ", ulltostr(spi->erasesize));
	fmc_pr(FMC_INFO, "Chip:%sB ", ulltostr(spi->chipsize));
	fmc_pr(FMC_INFO, "Name:\"%s\"\n", spi->name);

	fmc_pr(BT_DBG, "\t|||-Read if: %s, cmd: %#X, clock reg: %#x\n",
			str[spi->read->iftype],
			spi->read->cmd, spi->read->clock);
	fmc_pr(BT_DBG, "\t|||-Write if: %s, cmd: %#X, clock reg: %#x\n",
			str[spi->write->iftype],
			spi->write->cmd, spi->write->clock);
	fmc_pr(BT_DBG, "\t|||-Erase if: %s, cmd: %#X, clock reg: %#x\n",
			str[spi->erase[0].iftype],
			spi->erase[0].cmd, spi->erase[0].clock);
}

static void fmc_spi_map_op(struct spi_nor_info *spiinfo, struct fmc_spi *spi)
{
#ifdef CONFIG_DTR_MODE_SUPPORT
	if (spi->dtr_mode_support) {
		/* to match the best dummy/if_type/clock */
		fmc_spi_nor_search_rw(spiinfo, spi->read,
				FMC_SPI_NOR_SUPPORT_READ,
				FMC_SPI_NOR_DTR_MAX_DUMMY, RW_OP_READ);
	} else {
		fmc_spi_nor_search_rw(spiinfo, spi->read,
				FMC_SPI_NOR_SUPPORT_READ,
				FMC_SPI_NOR_STR_MAX_DUMMY, RW_OP_READ);
	}
#else
	/* to match the best dummy/if_type/clock */
	fmc_spi_nor_search_rw(spiinfo, spi->read,
			FMC_SPI_NOR_SUPPORT_READ,
			FMC_SPI_NOR_STR_MAX_DUMMY, RW_OP_READ);
#endif
	fmc_spi_nor_search_rw(spiinfo, spi->write,
			FMC_SPI_NOR_SUPPORT_WRITE,
			FMC_SPI_NOR_STR_MAX_DUMMY, RW_OP_WRITE);

	fmc_spi_nor_get_erase(spiinfo, spi->erase);
	fmc_map_iftype_and_clock(spi);
}

static int chip_spi_init(struct mtd_info_ex *mtd,
		 	struct fmc_spi *spi,
			struct spi_nor_info *spiinfo,
			unsigned char cs,
			unsigned char *ids)
{
	int ret = 0;

	spi_data_init(spi, spiinfo, cs);

#ifdef CONFIG_DTR_MODE_SUPPORT
	/* to check weather current device support DTR mode */
	fmc_check_spi_dtr_support(spi, ids, MAX_SPI_NOR_ID_LEN);
#endif
	fmc_spi_map_op(spiinfo, spi);
	if (!spi->driver) {
		printf("err:spi->driver is NULL");
		ret = -1;
		return ret;
	}
	if (spi->driver->qe_enable == NULL) {
		ret = -1;
		return ret;
	}
	spi->driver->qe_enable(spi);

	switch_to_4byte(spi, ids, MAX_SPI_NOR_ID_LEN);

	fmc_init_print(spi);
	mtd_data_set(mtd, spiinfo, spi);

	return ret;
}

static int match_chip_id(struct mtd_info_ex *mtd, struct fmc_spi *spi)
{
	unsigned char cs = 0;
	unsigned char ids[MAX_SPI_NOR_ID_LEN] = {0};
	unsigned char  ix;
	int len;
	unsigned int total = 0;
	char buffer[TMP_BUF_LEN];
	unsigned char *fmc_cs = NULL;
	int ret = 0;

	for (cs = 0; cs < CONFIG_SPI_NOR_MAX_CHIP_NUM; cs++) {
		fmc_cs = get_cs_number(cs);
		if (*fmc_cs) {
			fmc_pr(BT_DBG, "\t|||-CS(%d) is occupied\n", cs);
			continue;
		}

		fmc100_read_ids(spi, cs, ids);

		/* can't find spi flash device, for id 0-2 */
		if (!(ids[0] | ids[1] | ids[2]) ||
				((ids[0] & ids[1] & ids[2]) == 0xFF)) /* id 0-2 */
			continue;

		ret = sprintf_s(buffer, TMP_BUF_LEN, "SPI Nor(cs %d) ID: %#x %#x %#x", cs,
				ids[0], ids[1], ids[2]); /* id 0-2 */
		if (ret < 0)
			return ret;
		len = ret;
		spinor_info = fmc_spi_nor_serach_ids(ids, MAX_SPI_NOR_ID_LEN);
		/* id 3-7th */
		for (ix = 3; (spinor_info) && (ix < spinor_info->id_len); ix++) {
			ret = sprintf_s(buffer + len, TMP_BUF_LEN, " %#x", ids[ix]);
			if (ret < 0)
				return ret;
			len += ret;
		}

		if (spinor_info) {
			fmc_pr(FMC_INFO, "%s\n", buffer);

			fmc_pr(BT_DBG, "\t|||-CS-%d found SPI nor flash: %s\n",
			       cs, spinor_info->name);

			ret = chip_spi_init(mtd, spi, spinor_info, cs, ids);
			if (ret)
				return ret;

			mtd->numchips++;
			total += (unsigned int)spi->chipsize;
			spi++;
			(*fmc_cs)++;
		} else {
			printf("SPI Nor(cs %d) ID: %#x %#x %#x can't find"
			 " in the ID table !!!\n", cs, ids[0], ids[1], ids[2]);
		}
	}

	return ret;
}

int fmc_spi_nor_probe(struct mtd_info_ex *mtd, struct fmc_spi *spi)
{
	int ret;

	if (!mtd || !spi) {
		printf("err:mtd or spi is NULL");
		return -1;
	}

	mtd->numchips = 0;

	fmc_pr(FMC_INFO, "SPI Nor ID Table Version %s\n", SPI_NOR_ID_TBL_VER);

	ret = match_chip_id(mtd, spi);
	if (ret)
		return ret;

	fmc_pr(BT_DBG, "\t||*-End probe SPI nor flash, num: %d\n",
			mtd->numchips);

	return mtd->numchips;
}

#ifdef CONFIG_DTR_MODE_SUPPORT
void spi_dtr_to_sdr_switch(struct fmc_spi *spi)
{
	unsigned int ix = 0;
	unsigned int spi_dtr_dummy;
	struct spi_op **spiop, **fitspiop;
	const int iftype_read[] = {
		SPI_IF_READ_QUAD,   IF_TYPE_QUAD,
		SPI_IF_READ_QUAD_ADDR,  IF_TYPE_QIO,
		0,          0,
	};

	/* the dummy in SDR mode is impossible equal to DTR */
	spi_dtr_dummy = spi->read->dummy;

	/* match the best clock and dummy value agian */
	for (fitspiop = spiop = spinor_info->read;
	     (*spiop) && ix < MAX_SPI_OP; spiop++, ix++)
		if (((*spiop)->iftype & FMC_SPI_NOR_SUPPORT_READ) &&
		((*spiop)->dummy != spi_dtr_dummy) &&
		((*fitspiop)->iftype < (*spiop)->iftype))
			fitspiop = spiop;

	if (memcpy_s(spi->read, sizeof(struct spi_op), (*fitspiop), sizeof(struct spi_op)))
		printf("%s %d ERR:memcpy_s fail !\n", __func__, __LINE__);

	/* to map the iftype and clock of SDR mode */
	/* Only an even number of values is required,so increase length is 2 */
	for (ix = 0; iftype_read[ix]; ix += 2) {
		if (spi->read->iftype == iftype_read[ix]) {
			spi->read->iftype = iftype_read[ix + 1];
			break;
		}
	}
	fmc_get_fmc_best_2x_clock(&spi->read->clock);
}
#endif /* CONFIG_DTR_MODE_SUPPORT */
