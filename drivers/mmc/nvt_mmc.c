// SPDX-License-Identifier: GPL-2.0+
/*
 * SD card on SDIO1 of the Novatek NA51089 (NT98562, NT98566), non-DM.
 *
 * Software sets a data phase up first (block size, DATA_LENGTH, DATA_EN,
 * the FIFO's direction, then FIFO_EN), then writes the command with
 * CMD_EN; STATUS (write 1 to clear) says CMD_SENT when the command and
 * its response are done, with RSP_CRC_OK, RSP_CRC_FAIL or RSP_TIMEOUT.
 * Data goes through a 16-word FIFO at +0x100 (PIO here; the block can
 * also DMA), and DATA_END ends the data phase. An R2 lands in RSP3..RSP0,
 * bits 127..96 in RSP3.
 *
 * After the vendor U-Boot 2019.04 (drivers/mmc/nvt_ivot_mmc{,reg}.{c,h}):
 * its SDIO1 set-up for the NA51089 -- pins CGPIO11-16, the module reset,
 * a 192 MHz source divided down -- and its driving table from the XM
 * device tree (15 mA on every pin at default speed). 4-bit, as that tree
 * has it, at 24 MHz at most, the default-speed timing the vendor driver
 * uses below 25 MHz.
 *
 * Card detect (GPIO 9 in the XM tree) is not used: with no card the first
 * command that wants a response times out, which the core reports as no
 * card.
 */

#include <common.h>
#include <mmc.h>
#include <asm/io.h>
#include <asm/unaligned.h>
#include <asm/arch/na51089.h>
#include <linux/delay.h>
#include <linux/iopoll.h>

#define SDIO_CMD		(NVT_SDIO1_BASE + 0x00)
#define SDIO_ARG		(NVT_SDIO1_BASE + 0x04)
#define SDIO_RSP(n)		(NVT_SDIO1_BASE + 0x08 + 4 * (n))
#define SDIO_DATA_CTRL		(NVT_SDIO1_BASE + 0x1C)
#define SDIO_DATA_TIMER		(NVT_SDIO1_BASE + 0x20)
#define SDIO_STATUS		(NVT_SDIO1_BASE + 0x28)
#define SDIO_CLK_CTRL		(NVT_SDIO1_BASE + 0x38)
#define SDIO_BUS_WIDTH		(NVT_SDIO1_BASE + 0x3C)
#define SDIO_BUS_STATUS		(NVT_SDIO1_BASE + 0x40)
#define SDIO_CLK_CTRL2		(NVT_SDIO1_BASE + 0x44)
#define SDIO_PHY		(NVT_SDIO1_BASE + 0x4C)
#define SDIO_DLY0		(NVT_SDIO1_BASE + 0x50)
#define SDIO_DLY1		(NVT_SDIO1_BASE + 0x54)
#define SDIO_DLY5		(NVT_SDIO1_BASE + 0x64)
#define SDIO_DATA_PORT		(NVT_SDIO1_BASE + 0x100)
#define SDIO_DATA_LEN		(NVT_SDIO1_BASE + 0x104)
#define SDIO_FIFO_STS		(NVT_SDIO1_BASE + 0x108)
#define SDIO_FIFO_CTRL		(NVT_SDIO1_BASE + 0x10C)
#define SDIO_FIFO_SWITCH	(NVT_SDIO1_BASE + 0x1B0)

/* CMD */
#define CMD_NEED_RSP		BIT(6)
#define CMD_LONG_RSP		BIT(7)
#define CMD_EN			BIT(9)
#define CMD_SDC_RST		BIT(10)

/* DATA_CTRL */
#define DATA_EN			BIT(6)
#define DATA_DIS_SDIO_INT	BIT(9)
#define DATA_BLK_SIZE(x)	((x) << 16)

/* STATUS */
#define ST_RSP_CRC_FAIL		BIT(0)
#define ST_DATA_CRC_FAIL	BIT(1)
#define ST_RSP_TIMEOUT		BIT(2)
#define ST_DATA_TIMEOUT		BIT(3)
#define ST_RSP_CRC_OK		BIT(4)
#define ST_DATA_CRC_OK		BIT(5)
#define ST_CMD_SENT		BIT(6)
#define ST_DATA_END		BIT(7)
#define ST_DATA_ERR		(ST_DATA_CRC_FAIL | ST_DATA_TIMEOUT)

/* CLK_CTRL */
#define CLK_SD			BIT(9)		/* sample data on rising edge */
#define CLK_DIS			BIT(10)
#define CLK_DLY_ACT		BIT(12)
#define CLK_SD_CMD		BIT(21)		/* the same for CMD */

#define BUS_READY		BIT(0)		/* card not busy */
#define PHY_SW_RST		BIT(0)
#define PHY_BLK_FIFO_EN		BIT(4)
#define DLY0_PAD_CLK_SEL	BIT(14)		/* sample before the pad */
#define DLY1_READ_DLY_MASK	(0xf << 28)
#define DLY1_READ_DLY		(0xa << 28)	/* DATA/DET_READ_DLY = 2 */
#define DLY5_OUT_INV		(BIT(5) | BIT(6))
#define FIFO_EN			BIT(0)
#define FIFO_WRITE		BIT(2)
#define FIFO_CNT		0x7f
#define FIFO_EMPTY		BIT(8)
#define FIFO_FULL		BIT(9)
#define FIFO_SWITCH_DLY		BIT(4)
#define FIFO_WORDS		16

/* SDIO1 set-up in TOP, CG and PAD */
#define TOP_PINMUX		(NVT_TOP_BASE + 0x04)
#define TOP_SDIO_EN		BIT(14)
#define TOP_PGPIO_C		(NVT_TOP_BASE + 0xA0)
#define TOP_CGPIO11_16		(0x3f << 11)	/* clock, command, data 0-3 */
#define CG_CLKSEL		(NVT_CG_BASE + 0x20)
#define CG_SDIO_CLKSEL		(3 << 4)	/* 0: 192 MHz */
#define CG_SDIO_DIV		(NVT_CG_BASE + 0x3C)
#define CG_SDIO_DIV_MASK	0x7ff
#define CG_SDIO1		BIT(2)		/* in CLKEN0 and RSTN0 */
#define PAD_DS10		(NVT_PAD_BASE + 0x68)
#define PAD_DS_SDIO1		0x222222	/* 15 mA, 3 bits per pin */

#define SDIO_SRC_CLK		192000000
#define SDIO_F_MIN		400000
#define SDIO_F_MAX		24000000
#define SDIO_CMD_US		100000
#define SDIO_BUSY_US		1000000
#define SDIO_XFER_MS		1000		/* without progress */
#define SDIO_BLOCK_SIZE		512

static uint nvt_mmc_clock;

static void nvt_mmc_reset_sm(void)
{
	u32 val;

	setbits_le32(SDIO_CMD, CMD_SDC_RST);
	readl_poll_timeout(SDIO_CMD, val, !(val & CMD_SDC_RST), 1000);
	udelay(3);	/* a clock or so at 400 kHz */
}

static void nvt_mmc_phy_reset(void)
{
	u32 val;

	setbits_le32(SDIO_PHY, PHY_SW_RST);
	readl_poll_timeout(SDIO_PHY, val, !(val & PHY_SW_RST), 1000);
}

/* sdiohost_resetdata(): also after a data fault */
static void nvt_mmc_reset_data(void)
{
	writel(ST_DATA_END, SDIO_STATUS);
	writel(0, SDIO_FIFO_CTRL);
	clrbits_le32(SDIO_DATA_CTRL, DATA_EN);
	nvt_mmc_reset_sm();
	setbits_le32(SDIO_FIFO_SWITCH, FIFO_SWITCH_DLY);
	nvt_mmc_phy_reset();
	clrsetbits_le32(SDIO_DLY1, DLY1_READ_DLY_MASK, DLY1_READ_DLY);
}

static void nvt_mmc_set_clock(uint hz)
{
	uint div = DIV_ROUND_UP(SDIO_SRC_CLK, hz);

	if (div > CG_SDIO_DIV_MASK + 1)
		div = CG_SDIO_DIV_MASK + 1;
	setbits_le32(SDIO_CLK_CTRL, CLK_DIS);
	clrsetbits_le32(CG_SDIO_DIV, CG_SDIO_DIV_MASK, div - 1);
	clrbits_le32(SDIO_CLK_CTRL, CLK_DIS);
	setbits_le32(SDIO_CLK_CTRL, CLK_DLY_ACT | CLK_SD | CLK_SD_CMD);

	/* Below 25 MHz: no input delay, no block FIFO */
	clrbits_le32(SDIO_CLK_CTRL2, 0x3f << 16);
	clrbits_le32(SDIO_PHY, PHY_BLK_FIFO_EN);
	clrbits_le32(SDIO_DLY5, DLY5_OUT_INV);
	nvt_mmc_phy_reset();
	nvt_mmc_reset_sm();

	/* 300 ms between blocks, in SD clocks */
	writel(hz / 1000 * 300, SDIO_DATA_TIMER);
}

/* nvt_emmc_arch_host_preinit() for SDIO1 on the NA51089 */
static int nvt_mmc_init(struct mmc *mmc)
{
	setbits_le32(TOP_PINMUX, TOP_SDIO_EN);
	clrbits_le32(TOP_PGPIO_C, TOP_CGPIO11_16);
	clrbits_le32(NVT_CG_RSTN0, CG_SDIO1);
	udelay(10);
	setbits_le32(NVT_CG_RSTN0, CG_SDIO1);
	clrbits_le32(CG_CLKSEL, CG_SDIO_CLKSEL);
	clrsetbits_le32(CG_SDIO_DIV, CG_SDIO_DIV_MASK,
			DIV_ROUND_UP(SDIO_SRC_CLK, SDIO_F_MIN) - 1);
	setbits_le32(NVT_CG_CLKEN0, CG_SDIO1);
	clrsetbits_le32(PAD_DS10, 0xffffff, PAD_DS_SDIO1);
	udelay(10);

	clrbits_le32(SDIO_CLK_CTRL, CLK_DIS);
	/* Sample from the pad side, positive edge (neg-sample-edge = 0) */
	clrsetbits_le32(SDIO_DLY0, BIT(12) | BIT(13), DLY0_PAD_CLK_SEL);
	nvt_mmc_reset_data();
	nvt_mmc_clock = SDIO_F_MIN;
	nvt_mmc_set_clock(SDIO_F_MIN);
	/* 1 ms of clock before the first command (SD spec) */
	mdelay(1);
	return 0;
}

static int nvt_mmc_wait_ready(void)
{
	u32 val;

	return readl_poll_timeout(SDIO_BUS_STATUS, val, val & BUS_READY,
				  SDIO_BUSY_US);
}

static void nvt_mmc_arm_data(struct mmc_data *data, uint len)
{
	bool read = data->flags & MMC_DATA_READ;
	u32 ctrl = DATA_EN | DATA_BLK_SIZE(data->blocksize);
	int i;

	/* The vendor's "patch for sd fifo bug": a few dummy reads first */
	for (i = 0; i < 24; i++)
		readl(SDIO_CMD);
	if (read && data->blocks > 1)
		ctrl |= DATA_DIS_SDIO_INT;
	writel(ctrl, SDIO_DATA_CTRL);
	writel(len, SDIO_DATA_LEN);
	writel(read ? 0 : FIFO_WRITE, SDIO_FIFO_CTRL);
	writel(FIFO_EN | (read ? 0 : FIFO_WRITE), SDIO_FIFO_CTRL);
}

static int nvt_mmc_xfer(struct mmc_data *data, uint len)
{
	bool read = data->flags & MMC_DATA_READ;
	uint words = DIV_ROUND_UP(len, 4), n, i;
	u8 *buf = read ? (u8 *)data->dest : (u8 *)data->src;
	ulong start = get_timer(0);
	u32 st = 0, fifo;

	while (words) {
		fifo = readl(SDIO_FIFO_STS);
		n = min_t(uint, words, FIFO_WORDS);
		if (read && ((fifo & FIFO_FULL) || (fifo & FIFO_CNT) == n)) {
			for (i = 0; i < n; i++, buf += 4)
				put_unaligned_le32(readl(SDIO_DATA_PORT), buf);
			words -= n;
			start = get_timer(0);
		} else if (!read && (fifo & FIFO_EMPTY)) {
			for (i = 0; i < n; i++, buf += 4)
				writel(get_unaligned_le32(buf), SDIO_DATA_PORT);
			words -= n;
			start = get_timer(0);
		}
		st = readl(SDIO_STATUS);
		if (st & ST_DATA_ERR)
			goto fail;
		if (get_timer(start) > SDIO_XFER_MS)
			goto fail;
	}

	while (!((st = readl(SDIO_STATUS)) & (ST_DATA_END | ST_DATA_ERR))) {
		if (get_timer(start) > SDIO_XFER_MS)
			goto fail;
	}
	if (st & ST_DATA_ERR)
		goto fail;
	if (read)
		clrbits_le32(SDIO_FIFO_CTRL, FIFO_EN);
	writel(ST_DATA_END | ST_DATA_CRC_OK, SDIO_STATUS);
	return 0;

fail:
	nvt_mmc_reset_data();
	return st & ST_DATA_CRC_FAIL ? -EILSEQ : -ETIMEDOUT;
}

static int nvt_mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd,
			    struct mmc_data *data)
{
	u32 reg = cmd->cmdidx & 0x3f, st;
	uint len = 0;
	int i;

	if (data) {
		len = data->blocks * data->blocksize;
		if (nvt_mmc_wait_ready())
			return -ETIMEDOUT;
		nvt_mmc_arm_data(data, len);
	}

	if (cmd->resp_type & MMC_RSP_PRESENT)
		reg |= CMD_NEED_RSP;
	if (cmd->resp_type & MMC_RSP_136)
		reg |= CMD_LONG_RSP;
	writel(cmd->cmdarg, SDIO_ARG);
	writel(reg, SDIO_CMD);
	writel(readl(SDIO_STATUS), SDIO_STATUS);
	writel(reg | CMD_EN, SDIO_CMD);

	if (readl_poll_timeout(SDIO_STATUS, st,
			       st & (ST_CMD_SENT | ST_RSP_TIMEOUT),
			       SDIO_CMD_US) || (st & ST_RSP_TIMEOUT)) {
		writel(ST_CMD_SENT | ST_RSP_TIMEOUT, SDIO_STATUS);
		if (data)
			nvt_mmc_reset_data();
		return -ETIMEDOUT;
	}
	writel(ST_CMD_SENT | ST_RSP_CRC_OK | ST_RSP_CRC_FAIL, SDIO_STATUS);
	/* R3 carries no CRC */
	if ((st & ST_RSP_CRC_FAIL) && (cmd->resp_type & MMC_RSP_CRC)) {
		if (data)
			nvt_mmc_reset_data();
		return -EILSEQ;
	}

	if (cmd->resp_type & MMC_RSP_136) {
		for (i = 0; i < 4; i++)
			cmd->response[i] = readl(SDIO_RSP(3 - i));
	} else if (cmd->resp_type & MMC_RSP_PRESENT) {
		cmd->response[0] = readl(SDIO_RSP(0));
	}

	if (data)
		return nvt_mmc_xfer(data, len);
	if (cmd->resp_type & MMC_RSP_BUSY)
		return nvt_mmc_wait_ready();
	return 0;
}

static int nvt_mmc_set_ios(struct mmc *mmc)
{
	writel(mmc->bus_width == 4 ? 1 : 0, SDIO_BUS_WIDTH);
	if (mmc->clock && mmc->clock != nvt_mmc_clock) {
		nvt_mmc_clock = mmc->clock;
		nvt_mmc_set_clock(nvt_mmc_clock);
	}
	return 0;
}

static const struct mmc_ops nvt_mmc_ops = {
	.send_cmd	= nvt_mmc_send_cmd,
	.set_ios	= nvt_mmc_set_ios,
	.init		= nvt_mmc_init,
};

static struct mmc_config nvt_mmc_cfg = {
	.name		= "nvt_mmc",
	.ops		= &nvt_mmc_ops,
	.host_caps	= MMC_MODE_4BIT,
	.voltages	= MMC_VDD_27_28 | MMC_VDD_28_29 | MMC_VDD_29_30 |
			  MMC_VDD_30_31 | MMC_VDD_31_32 | MMC_VDD_32_33 |
			  MMC_VDD_33_34 | MMC_VDD_34_35 | MMC_VDD_35_36,
	.f_min		= SDIO_F_MIN,
	.f_max		= SDIO_F_MAX,
	.b_max		= 0x3ffffff / SDIO_BLOCK_SIZE,	/* DATA_LENGTH */
	.part_type	= PART_TYPE_DOS,
};

int cpu_mmc_init(struct bd_info *bis)
{
	return mmc_create(&nvt_mmc_cfg, NULL) ? 0 : -ENOMEM;
}
