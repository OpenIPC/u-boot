// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 HiSilicon (Shanghai) Technologies CO., LIMITED.
 *
 */
#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/arch/platform.h>
#include <spi_flash.h>
#include <nand.h>
#include <netdev.h>
#include <mmc.h>
#include <asm/sections.h>
#include <sdhci.h>
#include <cpu_common.h>
#include <asm/mach-types.h>
#include <serial.h>
#include <linux/mtd/mtd.h>
#include <linux/delay.h>
#include <openipc.h>
#if CONFIG_IS_ENABLED(CMD_TIMESTAMP)
#include "cmd_timestamp.h"
#endif

#ifndef CONFIG_SYS_DCACHE_OFF
void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}
#endif

#define REG_BASE_GPIO0          0x11090000
#define GPIO0_0_DATA_OFST       0x4
#define GPIO_DIR_OFST		0x400
#define PROC_TIME_OUT           100
#define PROC_LOOP               5
#define DOWNLOAD_FLAG           0x1f
#define ACK     		0xAA

/* Connect to TOOLS */
static int uart_self_boot_check(void)
{
	uint32_t count, i;
	unsigned char cr;

	for (count = 0; count < PROC_LOOP; ++count) {
		for (i = 0; i < PROC_LOOP; i++)
			serial_putc((unsigned char) DOWNLOAD_FLAG);

		uint32_t timer_count = 0;
		while (timer_count < PROC_TIME_OUT) {
			if (serial_tstc()) {
				cr = (unsigned char)serial_getc();
				if (cr == ACK)
					return 1;
			}
			timer_count++;
			udelay(100); /* delay 100 us */
		}
	}
	serial_putc((unsigned char) '\n');

	return 0;
}

/* return the flag for usb/sd update */
static int check_usb_sd_update_flag(void)
{
	u32 flag;

	writel(0x0, REG_BASE_GPIO0 + GPIO_DIR_OFST);

	flag = readl(REG_BASE_GPIO0 + GPIO0_0_DATA_OFST);
	if (!flag) {
		mdelay(10); /* delay 10 ms */
		flag = readl(REG_BASE_GPIO0 + GPIO0_0_DATA_OFST);
		if (!flag) {
			mdelay(10); /* delay 10 ms */
			flag = readl(REG_BASE_GPIO0 + GPIO0_0_DATA_OFST);
		}
	}

	return (!flag);
}

static int boot_media = BOOT_MEDIA_UNKNOWN;

int get_boot_media(void)
{
	return boot_media;
}

#if defined(CONFIG_SHOW_BOOT_PROGRESS)
void show_boot_progress(int progress)
{
	printf("Boot reached stage %d\n", progress);
}
#endif

#define COMP_MODE_ENABLE ((unsigned int)0x0000EAEF)

static inline void delay(unsigned long loops)
{
	__asm__ volatile("1:\n"
			"subs %0, %1, #1\n"
			"bne 1b" : "=r"(loops) : "0"(loops));
}

static void boot_flag_init(void)
{
	unsigned int regval, boot_mode;

	/* get boot mode */
	regval = __raw_readl(SYS_CTRL_REG_BASE + REG_SYSSTAT);
	boot_mode = get_sys_boot_mode(regval);

	switch (boot_mode) {
		/* [3:2] 00b - boot from Spi Nor device */
	case BOOT_FROM_SPI:
		boot_media = BOOT_MEDIA_SPIFLASH;
		break;
	/* [3:2] 01b - boot from Spi Nand device */
	case BOOT_FROM_SPI_NAND:
		boot_media = BOOT_MEDIA_NAND;
		break;
	/* [3:2] 10b - boot from Nand device */
	case BOOT_FROM_NAND:
		boot_media = BOOT_MEDIA_NAND;
		break;
	/* [3:2] 11b - boot from emmc */
	case BOOT_FROM_EMMC:
		boot_media = BOOT_MEDIA_EMMC;
		break;
	default:
		boot_media = BOOT_MEDIA_UNKNOWN;
		break;
	}
}

int board_early_init_f(void)
{
	return 0;
}

#define UBOOT_DATA_ADDR     0x41000000UL
#define UBOOT_DATA_SIZE     0x80000UL

#if defined(CONFIG_MMC)
static int data_to_emmc(void)
{
	struct mmc *mmc = find_mmc_device(0);
	void *buf = NULL;

	if (!mmc)
		return 1;

	(void)mmc_init(mmc);

	buf = map_physmem((unsigned long)UBOOT_DATA_ADDR,
			UBOOT_DATA_SIZE, MAP_WRBACK);
	if (!buf) {
		puts("Failed to map physical memory\n");
		return 1;
	}

	printf("MMC write...\n");
	blk_dwrite(mmc_get_blk_desc(mmc), 0, (UBOOT_DATA_SIZE >> NUM_9), buf);
	unmap_physmem(buf, UBOOT_DATA_SIZE);
	return 0;
}
#endif

#if (CONFIG_AUTO_UPDATE == 1)
static int data_to_spiflash(void)
{
	static struct spi_flash *flash = NULL;
	void *buf = NULL;
	int spi_flash_erase_ret;
	ssize_t val;

	/* 0:bus  0:cs  1000000:max_hz  0x3:spi_mode */
	flash = spi_flash_probe(0, 0, 1000000, 0x3);
	if (!flash) {
		printf("Failed to initialize SPI flash\n");
		return -1;
	}

	/* erase the address range. */
	printf("Spi flash erase...\n");
	spi_flash_erase_ret = spi_flash_erase(flash, NUM_0, UBOOT_DATA_SIZE);
	if (spi_flash_erase_ret) {
		printf("SPI flash sector erase failed\n");
		return 1;
	}

	buf = map_physmem((unsigned long)UBOOT_DATA_ADDR,
			UBOOT_DATA_SIZE, MAP_WRBACK);
	if (!buf) {
		puts("Failed to map physical memory\n");
		return 1;
	}

	/* copy the data from RAM to FLASH */
	printf("Spi flash write...\n");
	val = flash->write(flash, NUM_0, UBOOT_DATA_SIZE, buf);
	if (val) {
		printf("SPI flash write failed, return %zd\n", val);
		unmap_physmem(buf, UBOOT_DATA_SIZE);
		return 1;
	}

	unmap_physmem(buf, UBOOT_DATA_SIZE);
	return 0; /* 0:success */
}

static int data_to_nandflash(void)
{
	struct mtd_info *nand_flash = NULL;
	void *buf = NULL;
	size_t length = UBOOT_DATA_SIZE;
	int val;

	nand_flash = nand_info[0];

	printf("Nand flash erase...\n");
	val = nand_erase(nand_flash, 0, UBOOT_DATA_SIZE);
	if (val) {
		printf("Nand flash erase failed\n");
		return 1;
	}

	buf = map_physmem((unsigned long)UBOOT_DATA_ADDR,
			UBOOT_DATA_SIZE, MAP_WRBACK);
	if (!buf) {
		puts("Failed to map physical memory\n");
		return 1;
	}

	printf("Nand flash write...\n");
	val = nand_write(nand_flash, 0, &length, buf);
	if (val) {
		printf("Nand flash write failed, return %d\n", val);
		unmap_physmem(buf, UBOOT_DATA_SIZE);
		return 1;
	}

	unmap_physmem(buf, UBOOT_DATA_SIZE);
	return 0;
}

static int save_bootdata_to_flash(void)
{
	unsigned int sd_update_flag;

	sd_update_flag = readl(REG_BASE_SCTL + REG_SC_GEN4);
	if (sd_update_flag == START_MAGIC) {
		int ret;
#if defined(CONFIG_FMC)
		if (boot_media == BOOT_MEDIA_SPIFLASH) {
			ret = data_to_spiflash();
			if (ret != 0)
				return ret;
		}
		if (boot_media == BOOT_MEDIA_NAND) {
			ret = data_to_nandflash();
			if (ret != 0)
				return ret;
		}
#endif
#if defined(CONFIG_MMC)
		if (boot_media == BOOT_MEDIA_EMMC) {
			ret = data_to_emmc();
			if (ret != 0)
				return ret;
		}
#endif

		printf("update success!\n");
	}

	return 0;
}

static int is_bare_program(void)
{
	return 1;
}

static int is_auto_update(void)
{
#if (CONFIG_AUTO_SD_UPDATE == 1) || (CONFIG_AUTO_USB_UPDATE == 1)
	/* to add some judgement if neccessary */
	unsigned int  val[NUM_3];

	writel(0, REG_BASE_GPIO0 + GPIO_DIR_OFST);

	val[NUM_0] = readl(REG_BASE_GPIO0 + GPIO0_0_DATA_OFST);
	if (val[NUM_0])
		return 0;

	udelay(10000); /* delay 10000 us */
	val[NUM_1] = readl(REG_BASE_GPIO0 + GPIO0_0_DATA_OFST);
	udelay(10000); /* delay 10000 us */
	val[NUM_2] = readl(REG_BASE_GPIO0 + GPIO0_0_DATA_OFST);
	udelay(10000); /* delay 10000 us */

	if (val[NUM_0] == val[NUM_1] && val[NUM_1] == val[NUM_2] && val[NUM_0] == NUM_0)
		return 1;    /* update enable */
	else
		return 0;

#else
	return 0;
#endif
}
#endif /* CONFIG_AUTO_UPDATE */

int auto_update_flag = 0;
int bare_chip_program = 0;

static void set_bootloader_download_process_flag(void)
{
	if (readl(REG_START_FLAG) == START_MAGIC)
		return;

#ifndef CONFIG_BSP_DISABLE_DOWNLOAD
	/* set download flag */
	if (uart_self_boot_check()) {
		writel(START_MAGIC, REG_START_FLAG);
		return;
	}

	if (check_usb_sd_update_flag()) {
		writel(START_MAGIC, REG_START_FLAG);
		writel(SELF_BOOT_TYPE_USBDEV, SYS_CTRL_REG_BASE + REG_SC_GEN9);
		return;
	}
#endif
}

int misc_init_r(void)
{
	openipc_helper();
#ifdef CONFIG_RANDOM_ETHADDR
	random_init_r();
#endif
	env_set("verify", "n");

#if (CONFIG_AUTO_UPDATE == 1)
	/* auto update flag */
	if (is_auto_update())
		auto_update_flag = 1;
	else
		auto_update_flag = 0;

	/* bare chip program flag */
	if (is_bare_program())
		bare_chip_program = 1;
	else
		bare_chip_program = 0;

#ifdef CFG_MMU_HANDLEOK
	dcache_stop();
#endif

#ifdef CFG_MMU_HANDLEOK
	dcache_start();
#endif

	if (auto_update_flag)
		do_auto_update();
	if (bare_chip_program && !auto_update_flag)
		save_bootdata_to_flash();
#endif /* CONFIG_AUTO_UPDATE */
	set_bootloader_download_process_flag();
	return 0;
}

int board_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gd->bd->bi_arch_number = MACH_TYPE_HI3516CV610;
	gd->bd->bi_boot_params = CFG_BOOT_PARAMS;

	boot_flag_init();

	return 0;
}

int dram_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE, 0x20000000);
	return 0;
}

void reset_cpu(ulong addr)
{
	writel(WATCH_DOG_LOAD_VAL, REG_BASE_WATCH_DOG);
	writel(WATCH_DOG_ENABLE, WATCH_DOG_CONTROL);

	while (1);
}

typedef union {
	struct {
		unsigned int reserved1 : 31;  /* [30:0] */
		unsigned int fephy_rom_ken_high_indication : 1;  /* [31] */
	} bits;
	unsigned int val;
} otp_reg_fephy;

#define OTP_REG_FEPHY_ADDR 0x101e0020

typedef union {
	struct {
		unsigned int reserved0 : 15;  /* [14:0] */
		unsigned int fephy_rom_ken_high_indication : 1;  /* [15] */
		unsigned int reserved1 : 16;  /* [31:16] */
	} bits;
	unsigned int val;
} fephy_ctrl1;

#define FEPHY_CTRL1_ADDR 0x17950108

int board_eth_init(struct bd_info *bis)
{
	int rc = 0;
	otp_reg_fephy otp_reg_fephy;
	fephy_ctrl1 fephy_ctrl1;

	otp_reg_fephy.val = readl((uintptr_t)OTP_REG_FEPHY_ADDR);

	fephy_ctrl1.val = readl((uintptr_t)FEPHY_CTRL1_ADDR);

	if (otp_reg_fephy.bits.fephy_rom_ken_high_indication == 1) {
		fephy_ctrl1.bits.fephy_rom_ken_high_indication = 1;
		writel(fephy_ctrl1.val, FEPHY_CTRL1_ADDR);
	}

#ifdef CONFIG_SFV300_ETH
	rc = bspeth_initialize(bis);
#endif
	return rc;
}

#ifdef CONFIG_GENERIC_MMC
int board_mmc_init(struct bd_info *bis)
{
	int ret = 0;
	__maybe_unused int dev_num = 0;

#ifdef CONFIG_MMC_SDHCI

#ifndef CONFIG_FMC
	ret = ext_sdhci_add_port(0, EMMC_BASE_REG); // 0 for port0
	if (!ret) {
		ret = mmc_flash_init(dev_num);
		if (ret)
			printf("No EMMC device found !\n");
	}
	++dev_num;
#endif

#ifdef CONFIG_AUTO_SD_UPDATE
	if (is_auto_update()) {
#ifdef CONFIG_AUTO_SD_UPDATE_IS_IN_SDIO0
		ret = ext_sdhci_add_port(0, SDIO0_BASE_REG); // 0 for port0
#else
		ret = ext_sdhci_add_port(1, SDIO1_BASE_REG); // 1 for port1
#endif
		if (ret)
			return ret;

		ret = mmc_flash_init(dev_num);
		if (ret)
			printf("No SD device found !\n");
		++dev_num;
	}
#endif

#endif

	return ret;
}
#endif
#ifdef CONFIG_ARMV7_NONSEC
void smp_set_core_boot_addr(unsigned long addr, int corenr)
{
}

void smp_kick_all_cpus(void)
{
}

void smp_waitloop(unsigned previous_address)
{
}
#endif

int timer_init(void)
{
	/*
	 *  *Under uboot, 0xffffffff is set to load register,
	 *   * timer_clk = BUSCLK/2/256.
	 *    * e.g. BUSCLK = 50M, it will roll back after 0xffffffff/timer_clk
	 *     * = 43980s = 12hours
	 *      */
	__raw_writel(0, CFG_TIMERBASE + REG_TIMER_CONTROL);
	__raw_writel(~0, CFG_TIMERBASE + REG_TIMER_RELOAD);

	/* 32 bit, periodic */
	__raw_writel(CFG_TIMER_CTRL, CFG_TIMERBASE + REG_TIMER_CONTROL);

	return 0;
}

#if CONFIG_IS_ENABLED(CMD_TIMESTAMP)
unsigned int timestamp_sram_record(void)
{
	unsigned int count = 0;
	bool ret = false;

	ret = timestamp_record_once_sram(count, SYS_CNT_BOOTROM_START_OFFSET, "bootrom_start", __LINE__);
	if (!ret) {
		printf("can't record SYS_CNT_BOOTROM_START_OFFSET\n");
		return count;
	}
	count++;

	ret = timestamp_record_once_sram(count, SYS_CNT_BOOTROM_GET_CHANNEL_TYPE_OFFSET, "bootrom_getchannel", __LINE__);
	if (!ret) {
		printf("can't record SYS_CNT_BOOTROM_GET_CHANNEL_TYPE_OFFSET\n");
		return count;
	}
	count++;

	ret = timestamp_record_once_sram(count, SYS_CNT_BOOTROM_GSL_AREA_END_OFFSET, "bootrom_gsl_end", __LINE__);
	if (!ret) {
		printf("can't record SYS_CNT_BOOTROM_GSL_AREA_END_OFFSET\n");
		return count;
	}
	count++;

	ret = timestamp_record_once_sram(count, SYS_CNT_BOOTROM_END_OFFSET, "bootrom_end", __LINE__);
	if (!ret) {
		printf("can't record SYS_CNT_BOOTROM_END_OFFSET\n");
		return count;
	}
	count++;

	ret = timestamp_record_once_sram(count, SYS_CNT_GSL_START_OFFSET, "gsl_start", __LINE__);
	if (!ret) {
		printf("can't record SYS_CNT_GSL_START_OFFSET\n");
		return count;
	}
	count++;

	ret = timestamp_record_once_sram(count, SYS_CNT_UBOOT_START_OFFSET, "uboot_start", __LINE__);
	if (!ret) {
		printf("can't record SYS_CNT_UBOOT_START_OFFSET\n");
		return count;
	}
	count++;

	return count;
}
#endif

static int do_watch_dog_reset(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned int reg_val = 0;

	printf("watch dog reset ...\n");
	mdelay(10); /* delay 10 ms */
	writel(WATCH_DOG_LOAD_VAL, REG_BASE_WATCH_DOG);
	reg_val = readl((uintptr_t)WATCH_DOG_CONTROL);
	reg_val |= WATCH_DOG_ENABLE;
	writel(reg_val, WATCH_DOG_CONTROL);

	return 0;
}

U_BOOT_CMD(
		dog_reset, CONFIG_SYS_MAXARGS, 0, do_watch_dog_reset,
		"watchdog reset system",
		"watchdog reset \n"
	  );
