/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "boot_image.h"
#include <compiler.h>
#include <asm/io.h>
#include <cpu_common.h>
#include <linux/delay.h>
#include "flash_read.h"
#include "flash_write.h"

#define	TEE_VERIFY_NOT_ENABLE 0x42
#define	REE_VERIFY_NOT_ENABLE 0x42

#define BOOT_BACKUP_ENABLE_BIT		(1 << 7)
#define SYS_REG_MAJOR_BOOT_ADDR		0
#define SYS_REG_MINOR_BOOT_ADDR		BOOT_IMAGE_ADDR_REG_ADDR
#define SYS_REG_BOOT_IMAGE_SIZE		BOOT_IMAGE_SIZE_REG_ADDR

#define	SYS_BOOT_TMP_DDR_ADDR		0x50080000 /* TODO */

struct boot_image_addr_info {
	unsigned int major_addr;
	unsigned int minor_addr;
	size_t image_size;
	uintptr_t ddr_addr;
};

static inline ulong read_reg(uintptr_t addr)
{
	return readl(addr);
}

static inline void write_reg(uintptr_t addr, ulong val)
{
	writel(addr, val);
}

int get_otp_tee_verify_enable(void)
{
	uint32_t val = readl(OTP_BYTE_ALIGNED_LOCKABLE_0);
	return (int)((val >> 24) & 0xFF); /* 24 bit[31:24]=tee_verify_enable */
}

int get_otp_ree_verify_enable(void)
{
	uint32_t val = readl(OTP_BYTE_ALIGNED_LOCKABLE_0);
	return (int)((val >> 16) & 0xFF); /* 16 bit[23:16]=ree_verify_enable */
}

bool get_otp_boot_backup_enable(void)
{
	uint32_t val = readl(OTP_BIT_ALIGNED_LOCKABLE);
	if ((val & BOOT_BACKUP_ENABLE_BIT) == 0) {
		return 0;
	} else {
		return 1;
	}
}

int get_reg_verify_backup_image(void)
{
	uint32_t val = readl(VERIFY_BACKUP_IMAGE_REG_ADDR);
	int ret = (int)((val & VERIFY_BACKUP_IMAGE_FLAG_BIT_MASK) >> VERIFY_BACKUP_IMAGE_FLAG_BIT_OFFSET);
	return ret;
}

static void set_reg_verify_backup_image(int val)
{
	write_reg(SYS_REG_MINOR_BOOT_ADDR, val);
	write_reg(SYS_REG_BOOT_IMAGE_SIZE, val);
}

static void get_reg_boot_image_addr(struct boot_image_addr_info *addr)
{
	addr->major_addr = SYS_REG_MAJOR_BOOT_ADDR;
	addr->minor_addr = read_reg(SYS_REG_MINOR_BOOT_ADDR);
	addr->image_size = read_reg(SYS_REG_BOOT_IMAGE_SIZE);
	addr->ddr_addr = SYS_BOOT_TMP_DDR_ADDR;
}

static int do_flash_dump(uint src_addr, uint dest_addr, size_t size, uintptr_t ddr_addr)
{
	int ret;
	size_t i;
	const uint32_t block = 64 * 1024;		/* block size: 64 * 1024 = 64K */
	ret = do_flash_init();
	if (ret < 0) {
		dbg_err("do_flash_init error\n");
		return -1;
	}
	ret = do_flash_erase(dest_addr, size);
	if (ret < 0) {
		dbg_err("do_flash_erase error\n");
		return -1;
	}
	dbg_info("src_addr=%#X, erase @%#X %#zX bytes\n", src_addr, dest_addr, size);
	for (i = 0; i < size; i += block) {
		ret = flash_read(src_addr + i, block, (unsigned char *)ddr_addr, NULL);
		if (ret < 0) {
			dbg_err("flash_read error\n");
			return -1;
		}
		ret = do_flash_write(dest_addr + i, (unsigned char *)ddr_addr, block);
		if (ret < 0) {
			dbg_err("flash_write error\n");
			return -1;
		}
	}
	dbg_info("dump over\n");
	return 0;
}

static int do_boot_image_recover(const struct boot_image_addr_info *image_info)
{
	int ret;

	ret = do_flash_dump(image_info->minor_addr, image_info->major_addr, image_info->image_size,\
		image_info->ddr_addr);
	if (ret < 0) {
		dbg_err("do_flash_dump error\n");
		return -1;
	}
	/* on success */
	set_reg_verify_backup_image(0);
	dbg_info("boot image recover success\n");
	return 0;
}

int verify_boot_image(void)
{
	struct boot_image_addr_info image_addr = {0};
	int ret;
	if (get_otp_tee_verify_enable() == TEE_VERIFY_NOT_ENABLE) {
		dbg_info("tee verify not support\n");
		return 0;
	} else if  (get_otp_boot_backup_enable() == 0) {
		dbg_info("backup is disable\n");
		return 0;
	} else {
		dbg_info("\n");
	}
	get_reg_boot_image_addr(&image_addr);
	if ((image_addr.minor_addr == 0) || (image_addr.image_size == 0)) { /* invalid image address */
		dbg_info("invalid image info(address for size is 0)\n");
		return 0;
	} else {
		dbg_info("image_addr.minor_addr=%#X, image_size=%#zX\n", image_addr.minor_addr, image_addr.image_size);
	}
	if (get_reg_verify_backup_image() == 1) {
		ret = do_boot_image_recover(&image_addr);
		if (ret != 0) {
			dbg_err("do_boot_image_recover failed\n");
			return ret;
		}
		dbg_info("do_boot_image_recover OK\n");
		do_reset(NULL, 0, 0, NULL);
	}
	return 0;
}