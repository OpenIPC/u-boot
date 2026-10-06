/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include <common.h>
#include <command.h>
#include <div64.h>
#include <malloc.h>
#include <sdhci.h>
#include <linux/delay.h>
#include "mmc_private.h"
#include "mmc_internal.h"

#define MMC_M_MASK		0x70003FFF
#define MMC_M_DS		(MMC_CAP(MMC_LEGACY))
#define MMC_M_HS		(MMC_CAP(MMC_HS) | MMC_M_DS)
#define MMC_M_HS_52		(MMC_CAP(MMC_HS_52) | MMC_HS)
#define MMC_M_DDR_52	(MMC_CAP(MMC_DDR_52) | MMC_M_HS_52)
#define MMC_M_HS200		(MMC_CAP(MMC_HS_200) | MMC_M_DDR_52)
#define MMC_M_HS400		(MMC_CAP(MMC_HS_400) | MMC_M_HS200)
#define MMC_M_HS400_ES	(MMC_CAP(MMC_HS_400_ES) | MMC_M_HS400)
#define M_STR_LEN        8

typedef struct emmc_mode {
	char mode[M_STR_LEN];
	u32 speed;
} emmc_mode_t;

static const emmc_mode_t g_emmc_mode[] = {
	{"hs400es", MMC_M_HS400_ES},
	{"hs400", MMC_M_HS400},
	{"hs200", MMC_M_HS200},
	{"hsd52", MMC_M_DDR_52},
	{"hs52", MMC_M_HS_52},
	{"hs", MMC_M_HS},
	{"ds", MMC_M_DS}
};

static void mmc_reset_capblity(struct mmc *mmc, char *esm)
{
	int i;
	struct sdhci_host *host = mmc->priv;

	if (esm) {
		host->cfg.host_caps &= ~MMC_M_MASK;
		host->cfg.host_caps |= MMC_M_DS;
		for (i = 0; i < ARRAY_SIZE(g_emmc_mode); i ++) {
			if (strstr(esm, g_emmc_mode[i].mode)) {
				host->cfg.host_caps |= g_emmc_mode[i].speed;
				break;
			}
		}
		mmc->strobe_enhanced = 0;

		host->cfg.host_caps |= MMC_MODE_1BIT;
		host->cfg.host_caps |= (strstr(esm, "-8"))? (MMC_MODE_8BIT |MMC_MODE_4BIT) :
			 ((strstr(esm, "-4"))? (MMC_MODE_4BIT) : MMC_MODE_1BIT);
	}
}

#ifdef CONFIG_MMC_QUICKBOOT
void mmc_set_cur_mode(emmc_mode_u mode)
{
	emmc_param_gen1_u param1;

	param1.u32 = readl(EMMC_PARAM_GEN1);
	param1.bits.emmc_cur_mode = mode;
	writel(param1.u32, EMMC_PARAM_GEN1);
}

emmc_mode_u mmc_get_cur_mode(void)
{
	emmc_param_gen1_u param1;

	param1.u32 = readl(EMMC_PARAM_GEN1);

	return (emmc_mode_u)param1.bits.emmc_cur_mode;
}

emmc_qboot_u mmc_is_fast_boot(void)
{
	emmc_param_gen1_u param1;

	param1.u32 = readl(EMMC_PARAM_GEN1);

	return (emmc_qboot_u)param1.bits.emmc_qboot_mode;
}

void mmc_enable_fast_boot(emmc_qboot_u mode)
{
	emmc_param_gen1_u param1;

	if (mode < QUICK_BOOT_WARM || mode >= QUICK_BOOT_MAX) {
		printf("unavalid qboot mode %d\n", mode);
		return;
	}

	param1.u32 = readl(EMMC_PARAM_GEN1);
	param1.bits.emmc_qboot_mode = mode;

	writel(param1.u32, EMMC_PARAM_GEN1);
}

void mmc_disable_fast_boot(void)
{
	emmc_param_gen1_u param1;

	param1.u32 = readl(EMMC_PARAM_GEN1);
	param1.bits.emmc_qboot_mode = QUICK_BOOT_DIS;

	writel(param1.u32, EMMC_PARAM_GEN1);
}

static int mmc_check_status(struct mmc *mmc)
{
	int err;
	unsigned int status;

	err = mmc_send_status(mmc, &status);
	if (err) {
		printf("get emmc status failed.\n");
		return err;
	}

	if (!(status & MMC_STATUS_RDY_FOR_DATA) ||
		((status & MMC_STATUS_CURR_STATE) == MMC_STATE_PRG)) {
		printf("emmc not ready status = 0x%x\n", status);
		return -1;
	}

	return 0;
}

static void mmc_bdesc_init(struct mmc *mmc)
{
	struct blk_desc *bdesc;

	/* fill in device description */
	bdesc = mmc_get_blk_desc(mmc);
	if (bdesc == NULL) {
		return;
	}

	bdesc->lun = 0;
	bdesc->hwpart = 0;
	bdesc->type = 0;
	bdesc->blksz = mmc->read_bl_len;
	bdesc->log2blksz = LOG2(bdesc->blksz);
	bdesc->lba = lldiv(mmc->capacity, mmc->read_bl_len);
}

static void mmc_reparse_cid(struct mmc *mmc)
{
	int i;
	struct sdhci_host *host = mmc->priv;

	/* CRC is stripped so we need to do some shifting. */
	for (i = 0; i < 4; i++) { /* 4: resp len */
		mmc->cid[i] = sdhci_readl(host,
				SDHCI_RESPONSE + (3 - i) * 4) << 8; /* 3,4,8: idx */
		if (i != 3) /* 3: idx */
			mmc->cid[i] |= sdhci_readb(host,
					SDHCI_RESPONSE + (3 - i) * 4 - 1); /* 3,4: idx */
	}
	/* CID[3] is invalid */
	mmc->cid[3] = 0;
}

static void sdhci_reset(struct sdhci_host *host, u8 mask)
{
	unsigned long timeout;

	/* Wait max 100 ms, 10000 loop */
	timeout = 10000;
	sdhci_writeb(host, mask, SDHCI_SOFTWARE_RESET);
	while ((sdhci_readb(host, SDHCI_SOFTWARE_RESET) & mask) != 0) {
		if (timeout == 0) {
			printf("%s: Reset 0x%x never completed.\n",
			       __func__, (int)mask);
			return;
		}
		timeout--;
		udelay(10); /* 10us per loop */
	}
}

static void mmc_paramters_init(struct mmc *mmc)
{
	emmc_param_gen1_u param1;
	u8 bus_width[] = {BUS_WIDTH_1BIT, BUS_WIDTH_4BIT, BUS_WIDTH_8BIT, BUS_WIDTH_1BIT};
	u8 uhs_mode[] = {MMC_LEGACY, MMC_HS, 0, MMC_HS_200, MMC_DDR_52, 0, 0, MMC_HS_400};

	param1.u32 = readl(EMMC_PARAM_GEN1);

	mmc->host_caps = mmc->cfg->host_caps | MMC_CAP(MMC_LEGACY) | MMC_MODE_1BIT;

	mmc->ocr = mmc->ocr_from_bootrom;
	if (mmc_get_cur_mode() == DS_MODE) {
		/* DS set to MMC_LEGACY mode, should be reassignment */
		param1.bits.emmc_uhs_mode_sel = 0x0;
		/* DS hcs mode from bootrom */
		param1.bits.emmc_hcs_mode = !!(mmc->ocr & OCR_HCS);
		/* DS set bus width to 0x0, should be reassignment */
		param1.bits.emmc_bus_width = 0x0;
		/* DS enhance strobe should be disabled */
		param1.bits.emmc_enh_strobe = 0x0;
	}

	mmc->read_bl_len = MMC_MAX_BLOCK_LEN;
	mmc->write_bl_len = MMC_MAX_BLOCK_LEN;
	mmc->high_capacity = param1.bits.emmc_hcs_mode;
	/* follow uboot, rca set to 0x1 */
	mmc->rca = 0x1;

	mmc->bus_width = bus_width[param1.bits.emmc_bus_width];
	mmc_select_mode(mmc, uhs_mode[param1.bits.emmc_uhs_mode_sel]);
	mmc->clock = mmc->tran_speed;
	mmc->selected_mode = uhs_mode[param1.bits.emmc_uhs_mode_sel];
	if (mmc_get_cur_mode() == TRAN_MODE) {
		sdhci_reset(mmc->priv, SDHCI_RESET_CMD | SDHCI_RESET_DATA);
		mmc_set_clock(mmc, mmc->clock, MMC_CLK_ENABLE);
	}

	mmc->strobe_enhanced = param1.bits.emmc_enh_strobe;
	if (mmc->strobe_enhanced) {
#if !CONFIG_IS_ENABLED(MMC_HS400_ES_SUPPORT)
		/* Enhace mode has been restored, warning only */
		printf("Warning: HS400 ES mismatch between kernel and uboot.\n");
#endif
	}

	/* Warm boot CID is invalid, set vendor name "*" */
	mmc->cid[0] = 0x2a; /* "*" */
	mmc->cid[1] = 0x2a2a2a2a; /* "****" */
	mmc->part_config = MMCPART_NOAVAILABLE;
}

int mmc_restore(struct mmc *mmc)
{
	int err;

	if (mmc == NULL) {
		return -1;
	}

	/* BOOT_MODE not support fast boot */
	if (mmc_get_cur_mode() == BOOT_MODE) {
		return 0;
	}

	mmc_paramters_init(mmc);

	/* Only support eMMC 4.0 upper version */
	mmc->version = MMC_VERSION_4;
	err = mmc_startup_v4(mmc);
	if (err) {
		printf("parse device configuration failed\n");
		return err;
	}
	mmc->capacity = mmc->capacity_user;

	if (mmc_get_cur_mode() == DS_MODE) {
		/* DS mode, bootrom get cid by respone[3:0],
		 * respone[0] should corrupted by other cmd,
		 * only for reference.
		 */
		mmc_reparse_cid(mmc);

		err = mmc_get_capabilities(mmc);
		if (err) {
			printf("mmc get capabilities failed.\n");
			return err;
		}

		err = mmc_select_mode_and_width(mmc, mmc->card_caps);
		if (err) {
			printf("mmc select mode failed.\n");
			return err;
		}
	}
	mmc->best_mode = mmc->selected_mode;

	mmc_bdesc_init(mmc);

	err = mmc_check_status(mmc);
	if (err)
		return err;

	mmc->has_init = 1;

	return 0;
}

int mmc_save_parameters(struct mmc *mmc)
{
	emmc_param_gen0_u param0;
	emmc_param_gen1_u param1;
	struct sdhci_host *host = NULL;

	if ((mmc == NULL) || (mmc->priv == NULL)) {
		printf("mmc struct or priv invalid\n");
		return -1;
	}

	host = mmc->priv;

	/* parameters had been saved before, cold boot no need */
	if ((mmc_get_cur_mode() == TRAN_MODE) || \
		(mmc_is_fast_boot() == QUICK_BOOT_COLD)) {
		return 0;
	}

	param0.u32 = readl(EMMC_PARAM_GEN0);

	/* store clock freq */
	param0.bits.emmc_clk_sel = mmc_get_clk();
	/* store driver phase */
	param0.bits.emmc_clk_ph_sel = mmc_get_clk_phase();
	/* store software clock phase */
	param0.bits.emmc_sw_clk_ph = sdhci_readb(host, SDHCI_AT_STAT);

	writel(param0.u32, EMMC_PARAM_GEN0);

	/* store io information to pmc register */
	mmc_get_io_info();

	param1.u32 = readl(EMMC_PARAM_GEN1);
	param1.bits.emmc_uhs_mode_sel = sdhci_readb(host, SDHCI_HOST_CONTROL2) & SDHCI_CTRL_UHS_MASK;
	param1.bits.emmc_enh_strobe = mmc->strobe_enhanced;
	/* 0: 1bits, 1: 4bits, 2: 8bits */
	param1.bits.emmc_bus_width = (mmc->bus_width == BUS_WIDTH_8BIT)? 0x2 : \
								((mmc->bus_width == BUS_WIDTH_4BIT)? 1 : 0);
	param1.bits.emmc_hcs_mode = mmc->high_capacity;
	writel(param1.u32, EMMC_PARAM_GEN1);

	mmc_set_cur_mode(TRAN_MODE);

	return 0;
}

static int mmc_enter_sleep(struct mmc *mmc)
{
	int err;
	struct mmc_cmd cmd;

	mmc_save_parameters(mmc);

	/* disconnect card(set card to stand-by mode) */
	cmd.cmdidx = MMC_CMD_SELECT_CARD;
	cmd.resp_type = MMC_RSP_NONE;
	cmd.cmdarg = 0x0;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err) {
		printf("Disconnect card failed, err = %d\n", err);
		return err;
	}

	/* set card to sleep mode */
	cmd.cmdidx = MMC_CMD_SLEEP_AWAKE;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = mmc->rca << MMC_RCA_SHIFT_BIT;
	cmd.cmdarg |= 1 << MMC_SAWAKE_SHIFT_BIT;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err) {
		printf("Enter sleep mode failed, err = %d\n", err);
		return err;
	}

	/* Only warm boot support */
	if ((mmc_is_fast_boot() == QUICK_BOOT_WARM)) {
		mmc_set_cur_mode(SLEEP_MODE);
	}

	return 0;
}

static int mmc_reset(struct mmc *mmc)
{
	int err;
	struct mmc_cmd cmd;

	udelay(1000);

	cmd.cmdidx = MMC_CMD_GO_IDLE_STATE;
	cmd.cmdarg = 0;
	cmd.resp_type = MMC_RSP_NONE;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err)
		return err;

	udelay(2000);

	/* Only warm boot support */
	if ((mmc_is_fast_boot() == QUICK_BOOT_WARM)) {
		mmc_set_cur_mode(SLEEP_MODE);
	}

	return 0;
}
#else /* no define CONFIG_MMC_QUICKBOOT */

emmc_qboot_u mmc_is_fast_boot(void) { return 0; }
void mmc_enable_fast_boot(emmc_qboot_u mode) {}
void mmc_disable_fast_boot(void) {}
void mmc_set_cur_mode(emmc_mode_u mode) {}
emmc_mode_u mmc_get_cur_mode(void) { return 0; }
int mmc_restore(struct mmc *mmc) { return 0; }
int mmc_save_parameters(struct mmc *mmc) { return 0; }

#endif

static int curr_device = -1;

static struct mmc *get_mmc_device(int dev, bool force_init)
{
	struct mmc *mmc = NULL;

	mmc = find_mmc_device(dev);
	if (mmc == NULL) {
		printf("no mmc device at slot %x\n", dev);
		return NULL;
	}

	if (!mmc_getcd(mmc)) {
		force_init = true;
	}

	if (force_init) {
		mmc->has_init = 0;
	}

	if (mmc_init(mmc)) {
		return NULL;
	}

#ifdef CONFIG_MMC_QUICKBOOT
	mmc_set_cur_mode(INIT_MODE);
	mmc_save_parameters(mmc);
#endif

#ifdef CONFIG_BLOCK_CACHE
	struct blk_desc *bd = mmc_get_blk_desc(mmc);
	blkcache_invalidate(bd->if_type, bd->devnum);
#endif

	return mmc;
}

static int do_mmcut(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	__maybe_unused int err;
	struct mmc *mmc = NULL;

	if (curr_device < 0) {
		if (get_mmc_num() > 0) {
			curr_device = 0;
		} else {
			puts("No MMC device available\n");
			return 1;
		}
	}
	if (argc < 2) /* args should be 2 at least */
		return CMD_RET_USAGE;

	mmc = get_mmc_device(curr_device, false);
	if (mmc == NULL)
		return CMD_RET_FAILURE;

	if (strcmp(argv[1], "rescan") == 0) {
		mmc_reset_capblity(mmc, argv[2]); /* argv 2 */
		mmc = get_mmc_device(curr_device, true);
		if (mmc == NULL)
			return CMD_RET_FAILURE;
		printf_mmc(curr_device);
	} else if (strcmp(argv[1], "qsupport") == 0) {
#ifdef CONFIG_MMC_QUICKBOOT
		printf("yes\n");
	} else if (strcmp(argv[1], "reset") == 0) {
		err = mmc_reset(mmc);
		if (err)
			return CMD_RET_FAILURE;
	} else if (strcmp(argv[1], "sleep") == 0) {
		err = mmc_enter_sleep(mmc);
		if (err)
			return CMD_RET_FAILURE;
	} else if (strcmp(argv[1], "quick") == 0) {
		char *s = argv[2]; /* argv 2 */
		if (argc == 2) /* args 2 show mode */ {
			printf("qboot mode: %d\n", mmc_is_fast_boot());
			return CMD_RET_SUCCESS;
		}
		printf("quick setting (%d)> ", mmc_is_fast_boot());
		if (*s == '0') {
			mmc_disable_fast_boot();
		} else {
			mmc_enable_fast_boot(*s - '0');
		}
		printf("%d\n", mmc_is_fast_boot());
#endif
	} else {
		return CMD_RET_USAGE;
	}

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	mmcut, CONFIG_SYS_MAXARGS, 0, do_mmcut,
	"mmc user tools",
	"qsupport - quick boot macro support?\n"
#ifdef CONFIG_MMC_QUICKBOOT
	"mmcut reset - set mmc go to idle mode\n"
	"mmcut sleep - set mmc device enter sleep mode\n"
	"mmcut quick <0|1|2> - <disable|warm|cold> set mmc quick boot mode\n"
#endif
	"mmcut rescan <esm mode> - rescan mmc device with choice mode\n"
	"     esm mode format: <mode>-<bit_witdh>\n"
	"     mode: ds, hs, hs52, hsd52, hs200, hs400, hs400es\n"
	"     bit_width: 1,4,8\n"
	"     esm mode sample: hs200-4\n"
);
