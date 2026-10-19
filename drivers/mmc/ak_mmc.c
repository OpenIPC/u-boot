// SPDX-License-Identifier: GPL-2.0+
/*
 * SD card on MCI0 of the Anyka AK39 SoCs, non-DM.
 *
 * A PL180-like host with its own bit layout: STATUS clears when read, data
 * goes through the FIFO at +0x40 (PIO). After the AK3918EV200 U-Boot port's
 * ak39_mci.c (DM, v2026.07), checked against the vendor U-Boots' set-up:
 * every one of them -- EV200, EV300 SDK, the EV330 device trees -- wires a
 * 1-bit bus, MCI0 on pads 31 (clock), 32 (command) and 33 (data 0).
 *
 * Card detect is a board GPIO that differs from board to board (5, 24, 41
 * and 57 in the vendor sources), so it is not used: with no card the first
 * command times out, which the core reports as no card.
 */

#include <common.h>
#include <malloc.h>
#include <mmc.h>
#include <asm/io.h>
#include <asm/unaligned.h>
#include <asm/arch/anyka.h>
#include <linux/delay.h>
#include <linux/iopoll.h>

#define MCI_CLK			(AK_MCI0_BASE + 0x04)
#define MCI_ARGUMENT		(AK_MCI0_BASE + 0x08)
#define MCI_COMMAND		(AK_MCI0_BASE + 0x0c)
#define MCI_RESPONSE(n)		(AK_MCI0_BASE + 0x14 + 4 * (n))
#define MCI_DATATIMER		(AK_MCI0_BASE + 0x24)
#define MCI_DATALENGTH		(AK_MCI0_BASE + 0x28)
#define MCI_DATACTRL		(AK_MCI0_BASE + 0x2c)
#define MCI_STATUS		(AK_MCI0_BASE + 0x34)
#define MCI_FIFO		(AK_MCI0_BASE + 0x40)

/* CLK: divider low/high bytes, then enables */
#define CLK_ENABLE		(BIT(16) | BIT(17) | BIT(19) | BIT(20))

/* COMMAND */
#define CPSM_ENABLE		BIT(0)
#define CPSM_CMD(x)		(((x) & 0x3f) << 1)
#define CPSM_RESPONSE		BIT(7)
#define CPSM_LONGRSP		BIT(8)
#define CPSM_RSPCRC_NOCHK	BIT(10)
#define CPSM_WITHDATA		BIT(11)

/* DATACTRL */
#define DPSM_ENABLE		BIT(0)
#define DPSM_READ		BIT(1)
#define DPSM_BLOCKSIZE(x)	(((x) & 0xfff) << 16)

/* STATUS, read to clear */
#define ST_RESPCRCFAIL		BIT(0)
#define ST_DATACRCFAIL		BIT(1)
#define ST_RESPTIMEOUT		BIT(2)
#define ST_DATATIMEOUT		BIT(3)
#define ST_RESPEND		BIT(4)
#define ST_CMDSENT		BIT(5)
#define ST_DATAEND		BIT(6)
#define ST_TXACTIVE		BIT(10)
#define ST_RXACTIVE		BIT(11)
#define ST_FIFOFULL		BIT(12)
#define ST_FIFOEMPTY		BIT(13)
#define ST_CMD_ERR		(ST_RESPCRCFAIL | ST_RESPTIMEOUT)
#define ST_DATA_ERR		(ST_DATACRCFAIL | ST_DATATIMEOUT)

#define MCI_DATATIMER_VAL	0x00800000
#define MCI_F_MIN		400000
#define MCI_F_MAX		20000000
#define MCI_POLL_US		1000000
#define MCI_XFER_MS		2000
/* DATALENGTH is 16 bits */
#define MCI_MAX_XFER		65535
#define MCI_BLOCK_SIZE		512

/* Pads 31-33 as MCI0 */
#ifdef CONFIG_AK39_EV200
#define MCI0_SHAREPIN3_CLR	0x3c0	/* pad 33 has a 2-bit field */
#else
#define MCI0_SHAREPIN3_CLR	0x1c0
#endif
#define MCI0_SHAREPIN3_SET	0x1c0

static uint mci_clock = MCI_F_MIN;

static void ak_mci_set_clock(uint want)
{
	unsigned long asic = ak_asic_clk();
	u32 lo = 0, hi = 0;

	/* SD clock = ASIC / (2 + lo + hi) */
	if (asic >= 2UL * want) {
		unsigned long d = DIV_ROUND_UP(asic, want);

		d = d >= 2 ? d - 2 : 0;
		hi = (d >> 1) & 0xff;
		lo = (d - hi) & 0xff;
	}
	writel(lo | hi << 8 | CLK_ENABLE, MCI_CLK);
}

/* Also after a data fault: TX/RXACTIVE stay set until the host is reset */
static void ak_mci_host_init(void)
{
	clrsetbits_le32(AK_SYS_SHAREPIN3, MCI0_SHAREPIN3_CLR,
			MCI0_SHAREPIN3_SET);
	clrbits_le32(AK_SYS_CLK_GATE, AK_CLK_MCI0);
	setbits_le32(AK_SYS_SOFT_RST, AK_RST_MCI0);
	clrbits_le32(AK_SYS_SOFT_RST, AK_RST_MCI0);
	writel(0, MCI_COMMAND);
	writel(0, MCI_DATACTRL);
	readl(MCI_STATUS);
	ak_mci_set_clock(mci_clock);
	mdelay(10);
}

static int ak_mci_cmd(uint opcode, uint resp_type, uint arg, bool with_data)
{
	u32 ccon = CPSM_ENABLE | CPSM_CMD(opcode);
	u32 status;

	if (resp_type & MMC_RSP_PRESENT) {
		ccon |= CPSM_RESPONSE;
		if (resp_type & MMC_RSP_136)
			ccon |= CPSM_LONGRSP;
	}
	/* R3 (CMD1, ACMD41) and R4 (CMD5) carry no CRC */
	if (opcode == 41 || (opcode & ~4u) == 1)
		ccon |= CPSM_RSPCRC_NOCHK;
	if (with_data)
		ccon |= CPSM_WITHDATA;

	writel(arg, MCI_ARGUMENT);
	writel(ccon, MCI_COMMAND);

	if (!(resp_type & MMC_RSP_PRESENT))
		return readl_poll_timeout(MCI_STATUS, status,
					  status & ST_CMDSENT, MCI_POLL_US);

	if (readl_poll_timeout(MCI_STATUS, status,
			       status & (ST_RESPEND | ST_CMD_ERR), MCI_POLL_US))
		return -ETIMEDOUT;
	if (status & ST_RESPTIMEOUT)
		return -ETIMEDOUT;
	if (status & ST_RESPCRCFAIL)
		return -EILSEQ;
	return 0;
}

static int ak_mci_data_error(u32 status)
{
	writel(0, MCI_DATACTRL);
	ak_mci_host_init();
	if (status & ST_DATACRCFAIL)
		return -EILSEQ;
	if (status & ST_DATATIMEOUT)
		return -ETIMEDOUT;
	return -EIO;
}

static void ak_mci_arm_data(uint blocksize, uint nbytes, bool read)
{
	readl(MCI_STATUS);
	writel(MCI_DATATIMER_VAL, MCI_DATATIMER);
	writel(nbytes, MCI_DATALENGTH);
	writel(DPSM_ENABLE | DPSM_BLOCKSIZE(blocksize) | (read ? DPSM_READ : 0),
	       MCI_DATACTRL);
}

static int ak_mci_read(u8 *buf, uint len)
{
	ulong start = get_timer(0);
	uint done = 0;

	while (1) {
		u32 status = readl(MCI_STATUS);
		u32 word;

		if ((status & ST_FIFOFULL) && len - done >= 4) {
			word = readl(MCI_FIFO);
			put_unaligned_le32(word, buf + done);
			done += 4;
		}
		if (len - done && len - done < 4 && (status & ST_DATAEND)) {
			word = readl(MCI_FIFO);
			while (done < len) {
				buf[done++] = word;
				word >>= 8;
			}
		}
		if (status & ST_DATA_ERR)
			return ak_mci_data_error(status);
		if (!(status & ST_RXACTIVE)) {
			writel(0, MCI_DATACTRL);
			return done < len ? -EIO : 0;
		}
		if (get_timer(start) > MCI_XFER_MS)
			return ak_mci_data_error(ST_DATATIMEOUT);
	}
}

static int ak_mci_write(const u8 *buf, uint len)
{
	ulong start = get_timer(0);
	uint done = 0;

	while (1) {
		u32 status = readl(MCI_STATUS);

		if (done < len && (status & ST_FIFOEMPTY)) {
			u32 word = 0;
			uint i;

			for (i = 0; i < 4 && done + i < len; i++)
				word |= buf[done + i] << (8 * i);
			writel(word, MCI_FIFO);
			done += 4;
		}
		if (status & ST_DATA_ERR)
			return ak_mci_data_error(status);
		if (!(status & ST_TXACTIVE)) {
			writel(0, MCI_DATACTRL);
			return done < len ? -EIO : 0;
		}
		if (get_timer(start) > MCI_XFER_MS)
			return ak_mci_data_error(ST_DATATIMEOUT);
	}
}

static int ak_mci_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd,
			   struct mmc_data *data)
{
	uint nbytes = 0;
	int ret, i;

	if (data) {
		nbytes = data->blocks * data->blocksize;
		if (!nbytes || nbytes > MCI_MAX_XFER)
			return -EINVAL;
	}
	if (readl(MCI_STATUS) & (ST_TXACTIVE | ST_RXACTIVE)) {
		ak_mci_host_init();
		if (readl(MCI_STATUS) & (ST_TXACTIVE | ST_RXACTIVE))
			return -EBUSY;
	}

	/*
	 * Before CMD25 an SD card is told how many blocks to pre-erase
	 * (ACMD23), as the vendor drivers do. It is only a hint: a card that
	 * refuses it is written anyway, and MMC cards do not get it.
	 */
	if (data && cmd->cmdidx == MMC_CMD_WRITE_MULTIPLE_BLOCK && IS_SD(mmc) &&
	    !ak_mci_cmd(MMC_CMD_APP_CMD, MMC_RSP_R1, mmc->rca << 16, false))
		ak_mci_cmd(MMC_CMD_SET_BLOCK_COUNT, MMC_RSP_R1, data->blocks, false);

	/*
	 * A read's data path is armed before the command, as the vendor kernels
	 * do: the card may start sending a few clocks after its response.
	 */
	if (data && (data->flags & MMC_DATA_READ))
		ak_mci_arm_data(data->blocksize, nbytes, true);

	ret = ak_mci_cmd(cmd->cmdidx, cmd->resp_type, cmd->cmdarg, data != NULL);
	if (ret) {
		if (data)
			writel(0, MCI_DATACTRL);
		return ret;
	}

	if (cmd->resp_type & MMC_RSP_136) {
		/* RESPONSE0 holds the most significant word */
		for (i = 0; i < 4; i++)
			cmd->response[i] = readl(MCI_RESPONSE(i));
	} else if (cmd->resp_type & MMC_RSP_PRESENT) {
		cmd->response[0] = readl(MCI_RESPONSE(0));
	}

	if (!data)
		return 0;
	if (data->flags & MMC_DATA_READ)
		return ak_mci_read((u8 *)data->dest, nbytes);
	ak_mci_arm_data(data->blocksize, nbytes, false);
	return ak_mci_write((const u8 *)data->src, nbytes);
}

static int ak_mci_set_ios(struct mmc *mmc)
{
	/* > 1: mmc_power_off() calls this with bus_width still 0 */
	if (mmc->bus_width > 1)
		return -EOPNOTSUPP;
	if (mmc->clock) {
		mci_clock = mmc->clock;
		ak_mci_set_clock(mci_clock);
	}
	return 0;
}

static int ak_mci_init(struct mmc *mmc)
{
	ak_mci_host_init();
	return 0;
}

static const struct mmc_ops ak_mci_ops = {
	.send_cmd	= ak_mci_send_cmd,
	.set_ios	= ak_mci_set_ios,
	.init		= ak_mci_init,
};

static struct mmc_config ak_mci_cfg = {
	.name		= "ak_mci",
	.ops		= &ak_mci_ops,
	.host_caps	= MMC_MODE_HS | MMC_MODE_1BIT,
	.voltages	= MMC_VDD_27_28 | MMC_VDD_28_29 | MMC_VDD_29_30 |
			  MMC_VDD_30_31 | MMC_VDD_31_32 | MMC_VDD_32_33 |
			  MMC_VDD_33_34 | MMC_VDD_34_35 | MMC_VDD_35_36,
	.f_min		= MCI_F_MIN,
	.f_max		= MCI_F_MAX,
	.b_max		= MCI_MAX_XFER / MCI_BLOCK_SIZE,
	.part_type	= PART_TYPE_DOS,
};

int cpu_mmc_init(struct bd_info *bis)
{
	return mmc_create(&ak_mci_cfg, NULL) ? 0 : -ENOMEM;
}
