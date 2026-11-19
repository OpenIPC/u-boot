// SPDX-License-Identifier: GPL-2.0+
/*
 * Load U-Boot: its 1 KiB header from flash UBOOT_FLASH_ADDR to
 * UBOOT_HEADER_ADDR, check it, then the image behind it to UBOOT_ENTRY.
 * SPI NAND only; the vendor first stage also boots from NOR, eMMC and SD
 * and checks signatures on secure boot parts, which this one refuses.
 */
#include "cmn.h"
#include "board.h"
#include "boot.h"
#include "dma.h"
#include "spinand.h"

#define NAND_SPI_HZ	60000000

static int flash_buswidth(int type, struct boot_header *hdr)
{
	if ((type == FLASH_NAND || type == FLASH_NAND_2) && (hdr->capability & CAP_NAND_QUAD))
		return 4;
	return 1;
}

static int flash_read(int type, u32 dst, u32 src, u32 len)
{
	int ret;

	if (type != FLASH_NAND && type != FLASH_NAND_2)
		return -1;
	ret = spinand_read(src, len, (void *)dst);
	if (ret < 0)
		return ret;
	return len;
}

static u32 word_sum(const u32 *p, int bytes)
{
	u32 sum = 0;
	int i;

	for (i = 0; i < bytes / 4; i++)
		sum += p[i];
	return sum;
}

static int verify_uboot_header(struct boot_header *hdr)
{
	if (hdr->magic != IMG_HEADER_MAGIC)
		return -1;
	if (word_sum(&hdr->capability, IMG_HEADER_SIZE - 8) != hdr->check_sum)
		return -1;
	return 0;
}

static int load_uboot_header(int type, struct boot_header *uboot, struct boot_header *spl)
{
	if (type != FLASH_NAND && type != FLASH_NAND_2)
		return -3;
	spinand_set_geometry(spl->flash_cfg);
	if (spinand_init(NAND_SPI_HZ, flash_buswidth(type, spl)) < 0)
		return -3;
	if (flash_read(type, (u32)uboot, UBOOT_FLASH_ADDR, IMG_HEADER_SIZE) < 0)
		return -4;
	if (verify_uboot_header(uboot) < 0) {
		info("verify_uboot_header failed\r\n");
		return -1;
	}
	return 0;
}

static int load_uboot(int type, struct boot_header *uboot)
{
	if (is_secure_boot())
		return -5;		/* no signature check here */
	if (uboot->capability & CAP_ENCRYPTED)
		return -10;		/* no decryption here */
	if (flash_read(type, UBOOT_ENTRY, UBOOT_FLASH_ADDR + IMG_HEADER_SIZE, uboot->img_size) < 0)
		return -4;
	return UBOOT_ENTRY;
}

int boot_from_flash(int type)
{
	struct boot_header *uboot = (struct boot_header *)UBOOT_HEADER_ADDR;
	int ret;

	emmc_clk_set(0);
	dma_init();
	ret = load_uboot_header(type, uboot, (struct boot_header *)SPL_HEADER_ADDR);
	if (ret >= 0)
		ret = load_uboot(type, uboot);
	return ret;
}
