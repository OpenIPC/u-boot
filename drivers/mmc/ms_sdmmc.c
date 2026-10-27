// SPDX-License-Identifier: GPL-2.0+
/*
 * SD card on SDIO0, the FCIE5 SD host of the SigmaStar infinity3.
 *
 * One job runs a command and, with DTRX_EN, its data: the command goes in
 * through the CMD/RSP FIFO (byte 0 = 0x40 | index, then the argument most
 * significant byte first), the response comes back in the same FIFO, and
 * data moves between the card and DDR by DMA (MIU addresses) in blocks of
 * BLK_SIZE. MIE_EVENT reports the command's and the data's end, SD_STS
 * the errors. A write sends the command first, then runs the data alone
 * with JOB_DIR set.
 *
 * After the vendor U-Boot 2015.01 (drivers/mstar/sdmmc: hal_sdmmc_v5.c,
 * hal_card_platform_iNF3.c and ms_sdmmc_ub.c, which drives U-Boot's mmc
 * core the same way): the SDIO0 IP on the SD pads, DMA, bypass timing.
 */

#include <common.h>
#include <log.h>
#include <mmc.h>
#include <part.h>
#include <time.h>
#include <wait_bit.h>
#include <asm/io.h>
#include <asm/arch/sigmastar.h>
#include <linux/delay.h>

#define SD_REG(r)		SS_REG(SS_BANK_SDIO0, r)
#define SD_MIE_EVENT		SD_REG(0x00)
#define SD_MMA_PRI		SD_REG(0x02)
#define SD_DMA_ADDR_L		SD_REG(0x03)
#define SD_DMA_ADDR_H		SD_REG(0x04)
#define SD_DMA_LEN_L		SD_REG(0x05)
#define SD_DMA_LEN_H		SD_REG(0x06)
#define SD_MIE_FUNC_CTL		SD_REG(0x07)
#define SD_JOB_BLK_CNT		SD_REG(0x08)
#define SD_BLK_SIZE		SD_REG(0x09)
#define SD_CMD_RSP_SIZE		SD_REG(0x0A)
#define SD_MODE			SD_REG(0x0B)
#define SD_CTL			SD_REG(0x0C)
#define SD_STS			SD_REG(0x0D)
#define SD_BOOT_MOD		SD_REG(0x0E)
#define SD_DDR_MOD		SD_REG(0x0F)
#define SD_FIFO(i)		SD_REG(0x20 + (i) / 2)	/* CMD/RSP bytes */
#define SD_BOOT			SD_REG(0x37)
#define SD_FCIE_RST		SD_REG(0x3F)

/* MIE_EVENT */
#define EV_DATA_END		BIT(0)
#define EV_CMD_END		BIT(1)
#define EV_ERR_STS		BIT(2)
#define EV_BUSY_END		BIT(4)
#define EV_R2N_RDY		BIT(5)
#define EV_ALL			(EV_DATA_END | EV_CMD_END | EV_ERR_STS | \
				 EV_BUSY_END | EV_R2N_RDY)
/* MMA_PRI */
#define MMA_PRI_RW		(BIT(0) | BIT(1))
/* MIE_FUNC_CTL */
#define FUNC_SDIO_MODE		BIT(2)
/* SD_MODE */
#define MODE_CLK_EN		BIT(0)
#define MODE_BUS_4		BIT(1)
#define MODE_DMA_RD_CLK_STOP	BIT(7)
/* SD_CTL */
#define CTL_RSPR2_EN		BIT(0)
#define CTL_RSP_EN		BIT(1)
#define CTL_CMD_EN		BIT(2)
#define CTL_DTRX_EN		BIT(3)
#define CTL_JOB_DIR		BIT(4)	/* to the card */
#define CTL_JOB_START		BIT(6)
/* SD_STS */
#define STS_RD_CERR		BIT(0)
#define STS_WR_CERR		BIT(1)
#define STS_WR_TOUT		BIT(2)
#define STS_CMD_NORSP		BIT(3)
#define STS_CMDRSP_CERR		BIT(4)
#define STS_RD_TOUT		BIT(5)
#define STS_DAT0		BIT(8)
#define STS_ERRS		0x3f
#define STS_TOUTS		(STS_WR_TOUT | STS_CMD_NORSP | STS_RD_TOUT)
/* CMD_RSP_SIZE: the command is 5 bytes */
#define CMD_SIZE		(5 << 8)
/* BOOT_MOD, BOOT */
#define BOOT_MOD_BOOT		BIT(2)
#define BOOT_NAND_SRAM_IMI	(BIT(0) | BIT(1) | BIT(2))
/* FCIE_RST */
#define RST_SOFT		BIT(0)
#define RST_STS			(BIT(1) | BIT(2) | BIT(3))

/* Platform: clock, pads, card power and card detect */
#define SD_CLKGEN		SS_REG(SS_BANK_CLKGEN0, 0x45)
#define SD_CLKGEN_MASK		0x3f	/* [5:2] select, [1] invert, [0] gate */
#define CHIPTOP_SD_MODE		SS_REG(SS_BANK_CHIPTOP, 0x08)
#define CHIPTOP_SD_MODE2	BIT(3)
#define CHIPTOP_SDIO_MODE1	BIT(8)
#define CHIPTOP_SD_PULL_DRV	SS_REG(SS_BANK_CHIPTOP, 0x32)
#define CHIPTOP_SD_PULL		(0x1f << 8)	/* D3-D0, CMD pull-ups */
#define CHIPTOP_SD_DRV		0x3f		/* CLK, D3-D0, CMD drive */
#define CHIPTOP_ALLPAD		SS_REG(SS_BANK_CHIPTOP, 0x50)
#define CHIPTOP_ALLPAD_IN	BIT(15)
#define PADTOP_SD_POWER		SS_REG(SS_BANK_PADTOP, 0x17)
#define PADTOP_SD_PAD(i)	SS_REG(SS_BANK_PADTOP, 0x50 + (i)) /* CLK, CMD, D0-D3 */
#define PAD_OEN			BIT(5)	/* set: input, the pad to the IP */
#define PAD_OUT			BIT(4)
#define PM_SD_CDZ_MODE		SS_REG(SS_BANK_PMSLEEP, 0x28)
#define PM_SD_CDZ_EN		BIT(14)
#define PM_GPIO_SD_CDZ		SS_REG(SS_BANK_PMGPIO, 0x47)
#define CDZ_OEN			BIT(0)
#define CDZ_IN			BIT(2)	/* low: a card is in */

/* The vendor's waits */
#define WT_RESET_MS		100
#define WT_RSP_MS		10
#define WT_READ_MS		2000
#define WT_WRITE_MS		3000
#define WT_DAT0_MS		1000
#define WT_POWEROFF_MS		25
#define WT_POWERUP_MS		20
#define WT_POWERON_MS		60

#define SD_MAX_BLOCKS		1024

/* CLKGEN selects 0-7: what the SDIO0 clock runs at */
static const u32 sd_clocks[] = {
	48000000, 43200000, 40000000, 36000000,
	32000000, 20000000, 12000000, 300000,
};

static u16 sd_bus_width;
static unsigned int sd_nrc_us = 100;

static void sd_reset(void)
{
	clrbits_le16(SD_CTL, CTL_JOB_START);
	clrbits_le16(SD_FCIE_RST, RST_SOFT);
	if (wait_for_bit_le16((void *)SD_FCIE_RST, RST_STS, true, WT_RESET_MS,
			      false))
		printf("SD: IP reset low failed\n");
	setbits_le16(SD_FCIE_RST, RST_SOFT);
	if (wait_for_bit_le16((void *)SD_FCIE_RST, RST_STS, false, WT_RESET_MS,
			      false))
		printf("SD: IP reset high failed\n");
}

/* All of the events, or -ETIMEDOUT */
static int sd_wait_event(u16 events, unsigned int ms)
{
	return wait_for_bit_le16((void *)SD_MIE_EVENT, events, true, ms,
				 false);
}

static u8 sd_fifo_byte(unsigned int i)
{
	return readw(SD_FIFO(i)) >> (i & 1 ? 8 : 0);
}

static u32 sd_rsp_word(unsigned int first)
{
	return sd_fifo_byte(first) << 24 | sd_fifo_byte(first + 1) << 16 |
	       sd_fifo_byte(first + 2) << 8 | sd_fifo_byte(first + 3);
}

/* Response size and SD_CTL bits for a response type */
static void sd_rsp_setting(uint resp_type, u16 *size, u16 *ctl)
{
	if (!(resp_type & MMC_RSP_PRESENT)) {
		*size = 0;
		*ctl = 0;
	} else if (resp_type & MMC_RSP_136) {
		*size = 0x10;
		*ctl = CTL_RSPR2_EN | CTL_RSP_EN;
	} else {
		*size = 5;
		*ctl = CTL_RSP_EN;
	}
}

static int sd_status_err(uint resp_type)
{
	u16 sts = readw(SD_STS) & STS_ERRS;

	/* R3 carries no CRC; the IP flags one anyway */
	if (!(resp_type & MMC_RSP_CRC))
		sts &= ~STS_CMDRSP_CERR;
	if (sts & STS_TOUTS)
		return -ETIMEDOUT;
	return sts ? -EIO : 0;
}

/* SD_STS and MIE_EVENT clear before a job starts */
static void sd_clear_status(void)
{
	writew(STS_ERRS, SD_STS);
	writew(EV_ALL, SD_MIE_EVENT);
}

static void sd_setup_job(struct mmc_cmd *cmd, struct mmc_data *data)
{
	u16 rsp_size, rsp_ctl;
	ulong addr;
	u32 len;

	sd_rsp_setting(cmd->resp_type, &rsp_size, &rsp_ctl);

	writew((cmd->cmdarg >> 24 & 0xff) << 8 | (0x40 | cmd->cmdidx),
	       SD_FIFO(0));
	writew((cmd->cmdarg >> 8 & 0xff) << 8 | (cmd->cmdarg >> 16 & 0xff),
	       SD_FIFO(2));
	writew(cmd->cmdarg & 0xff, SD_FIFO(4));

	if (data) {
		addr = data->flags & MMC_DATA_READ ? (ulong)data->dest :
						     (ulong)data->src;
		len = data->blocks * data->blocksize;
		if (data->flags & MMC_DATA_WRITE)
			ss_miu_flush();
		writew(data->blocksize, SD_BLK_SIZE);
		writew(data->blocks, SD_JOB_BLK_CNT);
		writew(len & 0xffff, SD_DMA_LEN_L);
		writew(len >> 16, SD_DMA_LEN_H);
		writew(SS_MIU_ADDR(addr) & 0xffff, SD_DMA_ADDR_L);
		writew(SS_MIU_ADDR(addr) >> 16, SD_DMA_ADDR_H);
	}

	writew(CMD_SIZE | rsp_size, SD_CMD_RSP_SIZE);
	writew(FUNC_SDIO_MODE, SD_MIE_FUNC_CTL);
	writew(MODE_CLK_EN | sd_bus_width | (data ? MODE_DMA_RD_CLK_STOP : 0),
	       SD_MODE);
	writew(rsp_ctl | (data && data->flags & MMC_DATA_WRITE ?
			  CTL_JOB_DIR : 0), SD_CTL);
	writew(MMA_PRI_RW, SD_MMA_PRI);
	/* Bypass timing, as the vendor runs every speed up to HS */
	writew(0, SD_DDR_MOD);
	clrbits_le16(SD_BOOT_MOD, BOOT_MOD_BOOT);
	clrbits_le16(SD_BOOT, BOOT_NAND_SRAM_IMI);
	udelay(sd_nrc_us);
	sd_clear_status();
}

/* A write's data, after its command: the job again, data alone */
static int sd_write_data(void)
{
	sd_clear_status();
	writew(CTL_JOB_DIR, SD_CTL);
	setbits_le16(SD_CTL, CTL_DTRX_EN | CTL_JOB_START);
	return sd_wait_event(EV_DATA_END, WT_WRITE_MS);
}

static int ms_sd_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd,
			  struct mmc_data *data)
{
	bool read = data && data->flags & MMC_DATA_READ;
	int ret;

	sd_setup_job(cmd, data);
	if (read) {
		setbits_le16(SD_CTL, CTL_CMD_EN | CTL_DTRX_EN | CTL_JOB_START);
		ret = sd_wait_event(EV_CMD_END | EV_DATA_END,
				    WT_RSP_MS + WT_READ_MS);
	} else {
		setbits_le16(SD_CTL, CTL_CMD_EN | CTL_JOB_START);
		ret = sd_wait_event(EV_CMD_END, WT_RSP_MS);
	}
	if (ret) {
		/* No end event: the IP is stuck, as the vendor resets it */
		ret = sd_status_err(cmd->resp_type) ?: -ETIMEDOUT;
		sd_reset();
		return ret;
	}

	if ((cmd->resp_type & MMC_RSP_BUSY) &&
	    wait_for_bit_le16((void *)SD_STS, STS_DAT0, true, WT_DAT0_MS,
			      false)) {
		sd_reset();
		return -ETIMEDOUT;
	}

	ret = sd_status_err(cmd->resp_type);
	if (!ret && data && !read && sd_write_data()) {
		sd_reset();
		return -ETIMEDOUT;
	}
	if (!ret)
		ret = sd_status_err(cmd->resp_type);

	/* Byte 0 is the command index (0x3F for R2); the CRC is not kept */
	cmd->response[0] = sd_rsp_word(1);
	if (cmd->resp_type & MMC_RSP_136) {
		cmd->response[1] = sd_rsp_word(5);
		cmd->response[2] = sd_rsp_word(9);
		cmd->response[3] = sd_rsp_word(13) & ~0xff;
	}
	return ret;
}

/* The vendor's Nrc wait before a command, for the clock it runs at */
static unsigned int sd_nrc_for(u32 hz)
{
	if (hz >= 8000000)
		return 1;
	if (hz >= 4000000)
		return 2;
	if (hz >= 2000000)
		return 4;
	if (hz >= 1000000)
		return 8;
	if (hz >= 400000)
		return 20;
	return 27;
}

static int ms_sd_set_ios(struct mmc *mmc)
{
	unsigned int sel;

	if (mmc->clock) {
		/* The fastest clock not above the one asked for */
		for (sel = 0; sel < ARRAY_SIZE(sd_clocks) - 1; sel++)
			if (sd_clocks[sel] <= mmc->clock)
				break;
		clrsetbits_le16(SD_CLKGEN, SD_CLKGEN_MASK, sel << 2);
		sd_nrc_us = sd_nrc_for(sd_clocks[sel]);
	}
	sd_bus_width = mmc->bus_width == 4 ? MODE_BUS_4 : 0;
	return 0;
}

static int ms_sd_getcd(struct mmc *mmc)
{
	setbits_le16(PM_SD_CDZ_MODE, PM_SD_CDZ_EN);
	setbits_le16(PM_GPIO_SD_CDZ, CDZ_OEN);
	return !(readw(PM_GPIO_SD_CDZ) & CDZ_IN);
}

/* The pads to the IP with pull-ups (on), or driven low (off) */
static void sd_pads(bool on)
{
	unsigned int i;

	if (on)
		setbits_le16(CHIPTOP_SD_PULL_DRV, CHIPTOP_SD_PULL);
	else
		clrbits_le16(CHIPTOP_SD_PULL_DRV, CHIPTOP_SD_PULL);
	for (i = 0; i < 6; i++) {
		if (on)
			setbits_le16(PADTOP_SD_PAD(i), PAD_OEN);
		else
			clrbits_le16(PADTOP_SD_PAD(i), PAD_OEN | PAD_OUT);
	}
	clrbits_le16(CHIPTOP_SD_MODE, CHIPTOP_SD_MODE2);
	if (on)
		setbits_le16(CHIPTOP_SD_MODE, CHIPTOP_SDIO_MODE1);
	else
		clrbits_le16(CHIPTOP_SD_MODE, CHIPTOP_SDIO_MODE1);
}

/* Card power, low active */
static void sd_power(bool on, unsigned int ms)
{
	clrbits_le16(PADTOP_SD_POWER, PAD_OEN);
	if (on)
		clrbits_le16(PADTOP_SD_POWER, PAD_OUT);
	else
		setbits_le16(PADTOP_SD_POWER, PAD_OUT);
	mdelay(ms);
}

/*
 * As the vendor's mmc_core_init(): the clock and the SD pads to SDIO0,
 * the card powered off, then, with a card in, powered up with the clock
 * running and the IP reset
 */
static int ms_sd_init(struct mmc *mmc)
{
	clrbits_le16(CHIPTOP_ALLPAD, CHIPTOP_ALLPAD_IN);
	clrbits_le16(SD_CLKGEN, SD_CLKGEN_MASK);
	clrbits_le16(CHIPTOP_SD_MODE, CHIPTOP_SD_MODE2);
	setbits_le16(CHIPTOP_SD_MODE, CHIPTOP_SDIO_MODE1);
	clrsetbits_le16(CHIPTOP_SD_PULL_DRV, CHIPTOP_SD_DRV, CHIPTOP_SD_PULL);

	writew(FUNC_SDIO_MODE, SD_MIE_FUNC_CTL);
	clrbits_le16(SD_MODE, MODE_CLK_EN);
	sd_pads(false);
	sd_power(false, WT_POWEROFF_MS);

	if (!ms_sd_getcd(mmc))
		return -ENOMEDIUM;

	sd_pads(true);
	sd_power(true, WT_POWERUP_MS);
	writew(FUNC_SDIO_MODE, SD_MIE_FUNC_CTL);
	setbits_le16(SD_MODE, MODE_CLK_EN);
	mdelay(WT_POWERON_MS);
	sd_reset();
	return 0;
}

static const struct mmc_ops ms_sd_ops = {
	.send_cmd	= ms_sd_send_cmd,
	.set_ios	= ms_sd_set_ios,
	.init		= ms_sd_init,
	.getcd		= ms_sd_getcd,
};

static struct mmc_config ms_sd_cfg = {
	.name		= "ms_sdmmc",
	.ops		= &ms_sd_ops,
	.host_caps	= MMC_MODE_4BIT | MMC_MODE_HS | MMC_MODE_HS_52MHz,
	.voltages	= MMC_VDD_27_28 | MMC_VDD_28_29 | MMC_VDD_29_30 |
			  MMC_VDD_30_31 | MMC_VDD_31_32 | MMC_VDD_32_33,
	.f_min		= 300000,
	.f_max		= 48000000,
	.b_max		= SD_MAX_BLOCKS,
	.part_type	= PART_TYPE_DOS,
};

int ms_sdmmc_initialize(void)
{
	return mmc_create(&ms_sd_cfg, NULL) ? 0 : -ENOMEM;
}
