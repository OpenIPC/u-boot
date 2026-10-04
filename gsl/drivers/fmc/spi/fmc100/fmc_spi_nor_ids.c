/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
*/

#include "td_type.h"
#include "share_drivers.h"
#include "soc_errno.h"
#include "share_drivers.h"
#include "fmc100.h"

struct spi_nor_info *g_spiinfo;
#define ID_LEN_4BYTE 3

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
set_read_fast(1, INFINITE, 120);
set_read_fast(1, INFINITE, 133);
set_read_fast4b(1, INFINITE, 133);

set_read_dual(1, INFINITE, 80);
set_read_dual4b(1, INFINITE, 80);
set_read_dual(1, INFINITE, 104);
set_read_dual4b(1, INFINITE, 104);
set_read_dual(1, INFINITE, 108);
set_read_dual4b(1, INFINITE, 108);
set_read_dual(1, INFINITE, 120);
set_read_dual(1, INFINITE, 133);
set_read_dual4b(1, INFINITE, 133);

set_read_dual_addr(1, INFINITE, 80);
set_read_dual_addr4b(1, INFINITE, 80);
set_read_dual_addr4b(1, INFINITE, 90);
set_read_dual_addr4b(1, INFINITE, 108);
set_read_dual_addr(1, INFINITE, 104);
set_read_dual_addr(1, INFINITE, 108);
set_read_dual_addr(1, INFINITE, 120);
set_read_dual_addr(1, INFINITE, 133);
set_read_dual_addr4b(1, INFINITE, 133);

set_read_quad(1, INFINITE, 80);
set_read_quad4b(1, INFINITE, 80);
set_read_quad(1, INFINITE, 84);
set_read_quad(1, INFINITE, 104);
set_read_quad4b(1, INFINITE, 104);
set_read_quad(1, INFINITE, 108);
set_read_quad4b(1, INFINITE, 108);
set_read_quad(1, INFINITE, 120);
set_read_quad(1, INFINITE, 133);
set_read_quad4b(1, INFINITE, 133);

set_read_quad_addr(3, INFINITE, 80);
set_read_quad_addr4b(3, INFINITE, 80);
set_read_quad_addr(3, INFINITE, 104);
set_read_quad_addr4b(3, INFINITE, 104);
set_read_quad_addr4b(3, INFINITE, 108);
set_read_quad_addr(3, INFINITE, 108);
set_read_quad_addr(3, INFINITE, 120);
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
set_write_std(0, 256, 120);
set_write_std(0, 256, 133);
set_write_std4b(0, 256, 120);
set_write_std4b(0, 256, 133);

set_write_quad(0, 256, 80);
set_write_quad4b(0, 256, 80);
set_write_quad(0, 256, 104);
set_write_quad4b(0, 256, 104);
set_write_quad(0, 256, 108);
set_write_quad(0, 256, 120);
set_write_quad(0, 256, 133);

/* As Micron MT25Q(and MIXC) and N25Q have different QUAD I/O write code,
 * but they have the same ID, so we cannot compatiable it. User can open
 * by theirselves. */
set_write_quad_addr(0, 256, 80);
set_write_quad_addr(0, 256, 104);
set_write_quad_addr(0, 256, 108);
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
set_erase_sector_64k(0, _64K, 120);
set_erase_sector_64k(0, _64K, 133);
set_erase_sector_64k4b(0, _64K, 133);

#include "fmc100_spi_general.c"
FMC_STATIC struct spi_drv spi_driver_general = {
	.wait_ready = spi_general_wait_ready,
	.write_enable = spi_general_write_enable,
	.entry_4addr = spi_general_entry_4addr,
	.qe_enable = spi_general_qe_enable,
};

FMC_STATIC struct spi_drv spi_driver_no_qe = {
	.wait_ready = spi_general_wait_ready,
	.write_enable = spi_general_write_enable,
	.entry_4addr = spi_general_entry_4addr,
	.qe_enable = spi_do_not_qe_enable,
};


#include "fmc100_spi_w25q256fv.c"
FMC_STATIC struct spi_drv spi_driver_w25q256fv = {
	.wait_ready = spi_general_wait_ready,
	.write_enable = spi_general_write_enable,
	.entry_4addr = spi_w25q256fv_entry_4addr,
	.qe_enable = spi_w25q256fv_qe_enable,
};

#include "fmc100_spi_mx25l25635e.c"
FMC_STATIC struct spi_drv spi_driver_mx25l25635e = {
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
FMC_STATIC struct spi_drv spi_driver_gd25qxxx = {
	.wait_ready = spi_general_wait_ready,
	.write_enable = spi_general_write_enable,
	.entry_4addr = spi_general_entry_4addr,
	.qe_enable = spi_gd25qxxx_qe_enable,
};

#include "fmc100_spi_xtx.c"
FMC_STATIC struct spi_drv spi_driver_xtx = {
	.wait_ready = spi_general_wait_ready,
	.write_enable = spi_general_write_enable,
	.entry_4addr = spi_general_entry_4addr,
	.qe_enable = spi_xtx_qe_enable,
};


#define SPI_NOR_ID_TBL_VER     "1.0"
#define MINI_TABLE 1

static size_t strnlen(const char *s, size_t count)
{
	const char *sc;
	for (sc = s; count-- && *sc != '\0'; ++sc);
	return sc - s;
}
/******************************************************************************
 * We do not guarantee the compatibility of the following device models in the
 * table.Device compatibility is based solely on the list of compatible devices
 * in the release package.
 ******************************************************************************/
#ifndef MINI_TABLE
static struct spi_nor_info fmc_spi_nor_info_table[] = {
	/* name     id  id_len  chipsize(Bytes) erasesize  */
	{
		"MX25L1606E",  {0xc2, 0x20, 0x15}, 3, _2M,    _64K, 3,
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

	/* MXIC 3.3V MX25L6436FM2I-08G/MX25L6433FM2I-08G */
	{
		"MX25L64XXFM2I",  {0xc2, 0x20, 0x17}, 3, _8M,    _64K, 3,
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
	/* MX25L51245G  3.3V */
	{
		"MX25L51245G",  {0xc2, 0x20, 0x1a}, 3, _64M,    _64K, 4,
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

	/* MX25V1635F  3.3V */
	{
		"MX25V1635F",  {0xc2, 0x23, 0x15}, 3, _2M,    _64K, 3,
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

	/* MX25U6435F, 1.65-2.0V */
	{
		"MX25U6435F", {0xc2, 0x25, 0x37}, 3, _8M, _64K, 3,
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

	/* MX25U12835F/MX25U12832F, 1.65-2.0V */
	{
		"MX25U128XXF", {0xc2, 0x25, 0x38}, 3, _16M, _64K, 3,
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

	/* MX25L12835F/MX25L12845G,  The suffix "45G" indicates the flash supporting DTR mode */
	{
		"MX25L128XX", {0xc2, 0x20, 0x18}, 3, _16M, _64K, 3,
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

	/* MX25U25635F/MX25U25645G, The suffix "45G" indicates the flash supporting DTR mode 1.65-2.0V */
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

	/* MX25L25635F/MX25L25645G/MX25L25735F/MX25L25635E, The suffix "45G" indicates the flash supporting DTR mode 3.3V */
	{
		"MX25L25(6/7)XX",
		{0xc2, 0x20, 0x19}, 3, _32M, _64K, 4,
		{
			&read_std4b(0, INFINITE, 50), /* 50MHz */
			&read_fast4b(1, INFINITE, 80 /* 84 */), /* 80MHz */
			&read_dual4b(1, INFINITE, 80 /* 84 */), /* 80MHz */
			&read_dual_addr4b(1, INFINITE, 80 /* 84 */), /* 80MHz */
			&read_quad4b(1, INFINITE, 80 /* 84 */), /* 80MHz */
			&read_quad_addr4b(3, INFINITE, 80 /* 84 */), /* 80MHz */
#ifdef CONFIG_DTR_MODE_SUPPORT
			&read_quad_dtr4b(10, INFINITE, 100 /* 84 */), /* 100MHz */
#endif
			0
		},

		{
			&write_std4b(0, 256, 120 /* 133 */), /* 120MHz */
			&write_quad_addr4b(0, 256, 120 /* 133 */), /* 120MHz */
			0
		},

		{
			&erase_sector_64k4b(0, _64K, 120 /* 133 */), /* 120MHz */
			0
		},
		&spi_driver_mx25l25635e,
	},

	/* Winbond -- w25x "blocks" are 64K, "sectors" are 4KiB */
	/* winbond W25Q16JV-IQ/S25FL016K  3.3V */
	{
		"W25Q16/S25FL016",
		{0xef, 0x40, 0x15}, 3, (_64K * _32B), _64K, 3,
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

	{
		"W25Q32(B/F)V", {0xef, 0x40, 0x16}, 3, (_64K * _64B), _64K, 3,
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

	/* winbond w25q32fw is 1.8v */
	{
		"W25Q32FW",  {0xef, 0x60, 0x16}, 3, _4M,  _64K, 3,
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

	/* winbond W25Q64FV(SPI)/W25Q64JV_IQ 3.3V */
	{
		"W25Q64XX",  {0xef, 0x40, 0x17}, 3, _8M,   _64K, 3,
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

	/* winbond w25q64fw is 1.8v */
	{
		"W25Q64FW",  {0xef, 0x60, 0x17}, 3, _8M,   _64K, 3,
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

	/* winbond w25q128fw is 1.8v */
	{
		"W25Q128FW",  {0xef, 0x60, 0x18}, 3, _16M,   _64K, 3,
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

	/* winbond W25Q128JVEAQ  3.3v */
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

	/* "W25Q128JV_IM" can support DTR mode 80MHz */
	{
		"W25Q128JV_IM", {0xEF, 0x70, 0x18}, 3, _16M, _64K, 3,
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

	/* Winbond W25Q256(F/J)V-IQ */
	{
		"W25Q256(F/J)V", {0xEF, 0x40, 0x19}, 3, _32M, _64K, 4,
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

	/* Winbond W25Q01JVZEIQ  3.3V  128M */
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

	/* CFEON EN25QH32B-104HIP2B 3.3V */
	{
		"EN25QH32B", {0x1c, 0x70, 0x16}, 3, _4M,  _64K, 3,
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

	/* CFEON EN25QH64A 3.3V */
	{
		"EN25QH64A", {0x1c, 0x70, 0x17}, 3, (_64K * _128B),  _64K, 3,
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

	/* CFEON EN25QH128A-104HIP2T 3.3V */
	{
		"EN25QH128A", {0x1c, 0x70, 0x18}, 3, _16M,  _64K, 3,
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

	/* GD GD25LQ128 1.8V */
	{
		"GD25LQ128", {0xC8, 0x60, 0x18}, 3, _16M,  _64K, 3,
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

	{
		"GD25Q256", {0xC8, 0x40, 0x19}, 3, _32M,  _64K, 4,
		{
			&read_std4b(0, INFINITE, 50), /* 50MHz */
			&read_fast4b(1, INFINITE, 80), /* 80MHz */
			&read_dual4b(1, INFINITE, 80), /* 80MHz */
			&read_dual_addr4b(1, INFINITE, 80), /* 80MHz */
			&read_quad4b(1, INFINITE, 80), /* 80MHz */
			&read_quad_addr4b(3, INFINITE, 80), /* 80MHz */
			0
		},

		{
			&write_std4b(0, 256, 80), /* 80MHz */
			&write_quad4b(0, 256, 80), /* 80MHz */
			0
		},

		{
			&erase_sector_64k4b(0, _64K, 104), /* 104MHz */
			0
		},
		&spi_driver_gd25qxxx,
	},

	/* GD GD25Q128/GD25Q127 3/3V */
	{
		"GD25Q12XX", {0xC8, 0x40, 0x18}, 3, _16M,  _64K, 3,
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

	{
		"GD25Q64", {0xC8, 0x40, 0x17}, 3, _8M,  _64K, 3,
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

	{
		"GD25Q16C", {0xC8, 0x40, 0x15}, 3, _2M,  _64K, 3,
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

	/* GD GD25LQ64C 1.8V */
	{
		"GD25LQ64C", {0xC8, 0x60, 0x17}, 3, _8M,  _64K, 3,
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

	{
		"GD25Q32", {0xC8, 0x40, 0x16}, 3, _4M,  _64K, 3,
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

	/* XMC */
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

	/* XMC 3.3v */
	{
		"XM25QH128A", {0x20, 0x70, 0x18}, 3, _16M,  _64K, 3,
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

	/* XMC */
	{
		"XM25QH64B", {0x20, 0x60, 0x17}, 3, _8M,  _64K, 3,
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

	/* XMC 3.3v */
	{
		"XM25QH128B", {0x20, 0x60, 0x18}, 3, _16M,  _64K, 3,
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

	/* XMC XM25QH64CHIQ 3.3v */
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

	/* XMC XM25QH128CHIQ 3.3v */
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

	/* XTX XT25F128BSSIGU/XT25F128BSSHGU 3.3v */
	{
		"XT25F128BSS", {0x0B, 0x40, 0x18}, 3, _16M,  _64K, 3,
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

	/* XTX XT25F64BSSIGU-S/XT25F64BSSHGU-S 3.3v */
	{
		"XT25F64BSS", {0x0B, 0x40, 0x17}, 3, _8M,  _64K, 3,
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

	/* XTX XT25F32BSSIGU-S 3.3V */
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

	{
		"XT25F16BSSIGU", {0x0B, 0x40, 0x15}, 3, _2M,  _64K, 3,
		{
			&read_std(0, INFINITE, 80), /* 80MHz */
			&read_fast(1, INFINITE, 120), /* 120MHz */
			&read_dual(1, INFINITE, 120), /* 120MHz */
			&read_dual_addr(1, INFINITE, 120), /* 120MHz */
			&read_quad(1, INFINITE, 120),  /* 120MHz */
			&read_quad_addr(3, INFINITE, 120),  /* 120MHz */
			0
		},
		{
			&write_std(0, 256, 120), /* 120MHz */
			&write_quad(0, 256, 120),  /* 120MHz */
			0
		},
		{
			&erase_sector_64k(0, _64K, 120), /* 120MHz */
			0
		},
		&spi_driver_xtx,
	},

	/* FM FM25Q64-SOB-T-G 3.3V */
	{
		"FM25Q64", {0xa1, 0x40, 0x17}, 3, _8M,  _64K, 3,
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

	/* FM FM25Q128A-SOB-T-G 3.3V */
	{
		"FM25Q128A", {0xa1, 0x40, 0x18}, 3, _16M,  _64K, 3,
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

	{0, {0}, 0, 0, 0, 0, {0}, {0}, {0}, NULL},
};
#else
static struct spi_nor_info fmc_spi_nor_info_table[] = {
	/* MX25L25635F/MX25L25645G/MX25L25735F/MX25L25635E, The suffix "45G" indicates the flash supporting DTR mode 3.3V */
	{
		"MX25L25(6/7)XX",
		{0xc2, 0x20, 0x19}, 3, _32M, _64K, 4,
		{
			&read_std4b(0, INFINITE, 50), /* 50MHz */
			&read_fast4b(1, INFINITE, 80 /* 84 */), /* 80MHz */
			&read_dual4b(1, INFINITE, 80 /* 84 */), /* 80MHz */
			&read_dual_addr4b(1, INFINITE, 80 /* 84 */), /* 80MHz */
			&read_quad4b(1, INFINITE, 80 /* 84 */), /* 80MHz */
			&read_quad_addr4b(3, INFINITE, 80 /* 84 */), /* 80MHz */
#ifdef CONFIG_DTR_MODE_SUPPORT
			&read_quad_dtr4b(10, INFINITE, 100 /* 84 */), /* 100MHz */
#endif
			0
		},

		{
			&write_std4b(0, 256, 120 /* 133 */), /* 120MHz */
			&write_quad_addr4b(0, 256, 120 /* 133 */), /* 120MHz */
			0
		},

		{
			&erase_sector_64k4b(0, _64K, 120 /* 133 */), /* 120MHz */
			0
		},
		&spi_driver_mx25l25635e,
	},
	{0, {0}, 0, 0, 0, 0, {0}, {0}, {0}, NULL},
};
#endif

static struct spi_nor_info *fmc_spi_nor_serach_ids(u_char* const ids, int len)
{
	struct spi_nor_info *info = fmc_spi_nor_info_table;
	struct spi_nor_info *fit_info = NULL;

	if (len <= 0)
		return NULL;

	for (; info->name; info++) {
		if (memcmp_ss(info->id, ids, info->id_len, NO_CHECK_WORD) != EXT_SEC_SUCCESS)
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
		fmc_print("%s %d ERR:memcpy_s fail !\n", __func__, __LINE__);
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
		fmc_print("spi nor err:spi->func is null!\n");
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

void fmc_spi_nor_get_erase(struct spi_nor_info* const info,
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
				fmc_print("%s %d ERR:memcpy_s fail !\n", __func__, __LINE__);
			break;
		}
	}
}

static void switch_to_4byte(struct fmc_spi* const spi, u_char* const ids, int len)
{
	unsigned char manu_id;
	/* auto check fmc_addr_mode 3 bytes or 4 bytes */
	unsigned int start_up_addr_mode = get_fmc_boot_mode();

	if (len < ID_LEN_4BYTE)
		return;

	manu_id = ids[0];
	if ((spi->addrcycle == SPI_NOR_3BYTE_ADDR_LEN)
			&& (start_up_addr_mode == SPI_NOR_ADDR_MODE_4_BYTES))
		fmc_print("\nError!!! the flash's addres len is 3bytes and start \
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

static void spi_data_init(struct fmc_spi *spi, const struct spi_nor_info *spiinfo,
				unsigned char cs)
{
	spi->name = spiinfo->name;
	spi->chipselect = cs;
	spi->chipsize = spiinfo->chipsize;
	spi->erasesize = spiinfo->erasesize;
	spi->addrcycle = spiinfo->addrcycle;
	spi->driver = spiinfo->driver;
}

#define MAX_LEN (256)
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
			fmc_print("BUG! ID len out of range.\n");
		}

		mtd->id_length = spiinfo->id_len;
		if (memcpy_s(mtd->ids, sizeof(mtd->ids), spiinfo->id, spiinfo->id_len))
			fmc_print("%s %d ERR:memcpy_s fail !\n", __func__, __LINE__);
		if (sizeof(mtd->name) < strnlen(spi->name, MAX_LEN)) {
			fmc_print("BUG! spi->name len err %d.\n", __LINE__);
			return;
		}

		if (memcpy_s(mtd->name, sizeof(mtd->name) - 1,
		    spi->name, sizeof(mtd->name) - 1) != EOK) {
			fmc_print("BUG! mtd->name len err %s %d.\n", __func__, __LINE__);
			return;
		}
		mtd->name[sizeof(mtd->name) - 1] = '\0';
	}
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
		fmc_print("err:spi->driver is NULL");
		ret = -1;
		return ret;
	}
	if (spi->driver->qe_enable == NULL) {
		ret = -1;
		return ret;
	}

	spi->driver->qe_enable(spi);
	switch_to_4byte(spi, ids, MAX_SPI_NOR_ID_LEN);
	mtd_data_set(mtd, spiinfo, spi);

	return ret;
}

static int match_chip_id(struct mtd_info_ex *mtd, struct fmc_spi *spi)
{
	unsigned char cs = 0;
	unsigned char ids[MAX_SPI_NOR_ID_LEN] = {0};
	unsigned int total = 0;
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

		g_spiinfo = fmc_spi_nor_serach_ids(ids, MAX_SPI_NOR_ID_LEN);
		if (g_spiinfo) {
			fmc_pr(BT_DBG, "\t|||-CS-%d found SPI nor flash: %s\n",
			       cs, g_spiinfo->name);

			ret = chip_spi_init(mtd, spi, g_spiinfo, cs, ids);
			if (ret)
				return ret;

			mtd->numchips++;
			total += (unsigned int)spi->chipsize;
			spi++;
			(*fmc_cs)++;
		} else {
			fmc_print("SPI Nor(cs %d) ID: %#x %#x %#x can't find"
			 " in the ID table !!!\n", cs, ids[0], ids[1], ids[2]);
		}
	}

	return ret;
}

int fmc_spi_nor_probe(struct mtd_info_ex *mtd, struct fmc_spi *spi)
{
	int ret;

	if (!mtd || !spi) {
		fmc_print("err:mtd or spi is NULL");
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
	for (fitspiop = spiop = g_spiinfo->read;
	     (*spiop) && ix < MAX_SPI_OP; spiop++, ix++)
		if (((*spiop)->iftype & FMC_SPI_NOR_SUPPORT_READ) &&
		((*spiop)->dummy != spi_dtr_dummy) &&
		((*fitspiop)->iftype < (*spiop)->iftype))
			fitspiop = spiop;

	if (memcpy_s(spi->read, sizeof(struct spi_op), (*fitspiop), sizeof(struct spi_op)))
		fmc_print("%s %d ERR:memcpy_s fail !\n", __func__, __LINE__);

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
