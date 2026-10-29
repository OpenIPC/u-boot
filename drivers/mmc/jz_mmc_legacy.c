// SPDX-License-Identifier: GPL-2.0+
/*
 * Ingenic T-series SD/MMC (MSC) controller, legacy (non-DM) MMC API.
 *
 * Ported from drivers/mmc/jz_mmc.c of OpenIPC/u-boot-ingenic (the Ingenic
 * SDK U-Boot 2013.07), keeping its hardware behaviour: PIO through the
 * FIFOs, busy polling of the status/flag registers with the controller's
 * own response/read timeouts, the reset sequence, the MSC clock set-up
 * through the CPM and the per-SoC differences.  The MSC clock gate is
 * opened by clk_init(), the MSC0 port B pins by the gpio_func tables (and
 * by jz_mmc_init() on the T23, as OpenIPC does).
 *
 * Copyright (c) 2013 Imagination Technologies
 * Author: Paul Burton <paul.burton@imgtec.com>
 */

#include <common.h>
#include <errno.h>
#include <mmc.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/base.h>
#include <asm/arch/clk.h>
#include <asm/arch/mmc.h>
#include <asm/unaligned.h>

struct jz_mmc_priv {
	uintptr_t base;
	uint32_t flags;
	int clk;
};

/* jz_mmc_priv flags */
#define JZ_MMC_BUS_WIDTH_MASK 0x3
#define JZ_MMC_BUS_WIDTH_1    0x0
#define JZ_MMC_BUS_WIDTH_4    0x2
#define JZ_MMC_BUS_WIDTH_8    0x3
#define JZ_MMC_SENT_INIT (1 << 2)

static struct jz_mmc_priv mmc_priv[3];
static struct mmc_config mmc_cfg[3];
static const char *const mmc_name[3] = { "msc0", "msc1", "msc2" };

static uint16_t jz_mmc_readw(struct jz_mmc_priv *priv, uintptr_t off)
{
	return readw((void __iomem *)(priv->base + off));
}

static uint32_t jz_mmc_readl(struct jz_mmc_priv *priv, uintptr_t off)
{
	return readl((void __iomem *)(priv->base + off));
}

static void jz_mmc_writel(uint32_t value, struct jz_mmc_priv *priv,
			  uintptr_t off)
{
	writel(value, (void __iomem *)(priv->base + off));
}

/*
 * The vendor driver polls without delay: it defines udelay() away in this
 * file ("for decreasing time"), so its udelay() calls are left out here.
 */
static int jz_mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd,
			   struct mmc_data *data)
{
	struct jz_mmc_priv *priv = mmc->priv;
	uint32_t stat, cmdat = 0;

	/* setup command */
	jz_mmc_writel(cmd->cmdidx, priv, MSC_CMD);
	jz_mmc_writel(cmd->cmdarg, priv, MSC_ARG);

	if (data) {
		/* setup data */
		cmdat |= MSC_CMDAT_DATA_EN;
		if (data->flags & MMC_DATA_WRITE)
			cmdat |= MSC_CMDAT_WRITE;

		jz_mmc_writel(data->blocks, priv, MSC_NOB);
		jz_mmc_writel(data->blocksize, priv, MSC_BLKLEN);
	}

	/* setup response */
	switch (cmd->resp_type) {
	case MMC_RSP_R1:
	case MMC_RSP_R1b:
		cmdat |= MSC_CMDAT_RESPONSE_R1;
		break;
	case MMC_RSP_R2:
		cmdat |= MSC_CMDAT_RESPONSE_R2;
		break;
	case MMC_RSP_R3:
		cmdat |= MSC_CMDAT_RESPONSE_R3;
		break;
	default:
		break;
	}

	if (cmd->resp_type & MMC_RSP_BUSY)
		cmdat |= MSC_CMDAT_BUSY;

	/* set init for the first command only */
	if (!(priv->flags & JZ_MMC_SENT_INIT)) {
		cmdat |= MSC_CMDAT_INIT;
		priv->flags |= JZ_MMC_SENT_INIT;
	}

	cmdat |= (priv->flags & JZ_MMC_BUS_WIDTH_MASK) << 9;

	/* write the data setup */
	jz_mmc_writel(cmdat, priv, MSC_CMDAT);

	jz_mmc_writel(0xffffffff, priv, MSC_IMASK);
	/* clear interrupts */
	jz_mmc_writel(0xffffffff, priv, MSC_IFLG);

	/* start the command (& the clock) */
	jz_mmc_writel(MSC_CTRL_START_OP, priv, MSC_CTRL);

	/* wait for completion */
	while (!(stat = (jz_mmc_readl(priv, MSC_IFLG) &
			 (MSC_IREG_END_CMD_RES | MSC_IREG_TIME_OUT_RES))))
		;
	jz_mmc_writel(stat, priv, MSC_IFLG);
	if (stat & MSC_IREG_TIME_OUT_RES)
		return -ETIMEDOUT;

	if (cmd->resp_type & MMC_RSP_PRESENT) {
		/* read the response */
		if (cmd->resp_type & MMC_RSP_136) {
			uint16_t a, b, c, i;

			a = jz_mmc_readw(priv, MSC_RES);
			for (i = 0; i < 4; i++) {
				b = jz_mmc_readw(priv, MSC_RES);
				c = jz_mmc_readw(priv, MSC_RES);
				cmd->response[i] = (a << 24) | (b << 8) | (c >> 8);
				a = c;
			}
		} else {
			cmd->response[0] = jz_mmc_readw(priv, MSC_RES) << 24;
			cmd->response[0] |= jz_mmc_readw(priv, MSC_RES) << 8;
			cmd->response[0] |= jz_mmc_readw(priv, MSC_RES) & 0xff;
		}
	}

	if (cmd->resp_type == MMC_RSP_R1b) {
		while (!(jz_mmc_readl(priv, MSC_STAT) & MSC_STAT_PRG_DONE))
			;
		jz_mmc_writel(MSC_IREG_PRG_DONE, priv, MSC_IFLG);
	}

	if (data && (data->flags & MMC_DATA_WRITE)) {
		/* write the data */
		int sz = DIV_ROUND_UP(data->blocks * data->blocksize, 4);
		const void *buf = data->src;

		while (sz--) {
			uint32_t val = get_unaligned_le32(buf);

			while (!(jz_mmc_readl(priv, MSC_IFLG) &
				 MSC_IREG_TXFIFO_WR_REQ))
				;
			jz_mmc_writel(val, priv, MSC_TXFIFO);
			buf += 4;
		}
		while (!(jz_mmc_readl(priv, MSC_STAT) & MSC_STAT_PRG_DONE))
			;
		jz_mmc_writel(MSC_IREG_PRG_DONE, priv, MSC_IFLG);
	} else if (data && (data->flags & MMC_DATA_READ)) {
		/* read the data */
		int sz = data->blocks * data->blocksize;
		void *buf = data->dest;

		do {
			stat = jz_mmc_readl(priv, MSC_STAT);
			if (stat & MSC_STAT_TIME_OUT_READ)
				return -ETIMEDOUT;
			if (stat & MSC_STAT_CRC_READ_ERROR)
				return -ECOMM;
			if (stat & MSC_STAT_DATA_FIFO_EMPTY)
				continue;
			do {
				uint32_t val = jz_mmc_readl(priv, MSC_RXFIFO);

				if (sz == 1)
					*(uint8_t *)buf = (uint8_t)val;
				else if (sz == 2)
					put_unaligned_le16(val, buf);
				else if (sz >= 4)
					put_unaligned_le32(val, buf);
				buf += 4;
				sz -= 4;
				stat = jz_mmc_readl(priv, MSC_STAT);
			} while (!(stat & MSC_STAT_DATA_FIFO_EMPTY));
		} while (!(stat & MSC_STAT_DATA_TRAN_DONE));
		while (!(jz_mmc_readl(priv, MSC_IFLG) & MSC_IREG_DATA_TRAN_DONE))
			;
		jz_mmc_writel(MSC_IREG_DATA_TRAN_DONE, priv, MSC_IFLG);
	}

	return 0;
}

static int jz_mmc_set_ios(struct mmc *mmc)
{
	struct jz_mmc_priv *priv = mmc->priv;
	uint32_t real_rate = 0;
	uint32_t lpm = LPM_LPM;
	uint8_t clk_div = 0;

	if (mmc->clock > 1000000)
		clk_set_rate(priv->clk, mmc->clock);
	else
		clk_set_rate(priv->clk, 24000000);

	real_rate = clk_get_rate(priv->clk);

	/* calculate clock divide */
	while ((real_rate > mmc->clock) && (clk_div < 7)) {
		real_rate >>= 1;
		clk_div++;
	}

	jz_mmc_writel(clk_div, priv, MSC_CLKRT);

	if (real_rate > 25000000)
		lpm |= (0x2 << LPM_DRV_SEL_SHF) | LPM_SMP_SEL;

	jz_mmc_writel(lpm, priv, MSC_LPM);

	/* set the bus width for the next command */
	priv->flags &= ~JZ_MMC_BUS_WIDTH_MASK;

	if (mmc->bus_width == 8)
		priv->flags |= JZ_MMC_BUS_WIDTH_8;
	else if (mmc->bus_width == 4)
		priv->flags |= JZ_MMC_BUS_WIDTH_4;
	else
		priv->flags |= JZ_MMC_BUS_WIDTH_1;

	debug("jzmmc:clk_want:%d, clk_set:%d bus_width:%d\n",
	      mmc->clock, clk_get_rate(priv->clk) / (1 << clk_div),
	      mmc->bus_width);

	return 0;
}

static int jz_mmc_core_init(struct mmc *mmc)
{
	struct jz_mmc_priv *priv = mmc->priv;
	unsigned int clkrt = jz_mmc_readl(priv, MSC_CLKRT);
	__maybe_unused int tmp;

	/* reset */
	jz_mmc_writel(MSC_CTRL_RESET, priv, MSC_CTRL);
#if defined(CONFIG_T10) || defined(CONFIG_T20) || defined(CONFIG_T21) || \
	defined(CONFIG_T23) || defined(CONFIG_T30) || defined(CONFIG_T31)
	tmp = jz_mmc_readl(priv, MSC_CTRL);
	tmp &= ~MSC_CTRL_RESET;
	jz_mmc_writel(tmp, priv, MSC_CTRL);
#else
	while (jz_mmc_readl(priv, MSC_STAT) & MSC_STAT_IS_RESETTING)
		;
#endif

	/* enable low power mode */
	jz_mmc_writel(0x1, priv, MSC_LPM);

	/*
	 * maximum timeouts; the vendor leaves MSC_RESTO at its reset
	 * default ("for decreasing time")
	 */
	jz_mmc_writel(0xffffffff, priv, MSC_RDTO);

	jz_mmc_writel(clkrt, priv, MSC_CLKRT);

	return 0;
}

static const struct mmc_ops jz_mmc_ops = {
	.send_cmd	= jz_mmc_send_cmd,
	.set_ios	= jz_mmc_set_ios,
	.init		= jz_mmc_core_init,
};

static void jz_mmc_init_one(int idx, int controller, uintptr_t base, int clock)
{
	struct jz_mmc_priv *priv = &mmc_priv[idx];
	struct mmc_config *cfg = &mmc_cfg[idx];

	/* setup priv */
	priv->base = base;
	priv->clk = clock;
	priv->flags = 0;

	/* setup mmc */
	cfg->name = mmc_name[controller];
	cfg->ops = &jz_mmc_ops;

	cfg->voltages = MMC_VDD_27_28 |
		MMC_VDD_28_29 | MMC_VDD_29_30 | MMC_VDD_30_31 | MMC_VDD_31_32 |
		MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_34_35 | MMC_VDD_35_36;

	cfg->f_min = 200000;
	cfg->f_max = 52000000;
#ifdef CONFIG_JZ_MMC_MSC0_PA_8BIT
	cfg->host_caps = MMC_MODE_8BIT | MMC_MODE_HS_52MHz | MMC_MODE_HS;
#else
	cfg->host_caps = MMC_MODE_4BIT | MMC_MODE_HS_52MHz | MMC_MODE_HS;
#endif
	/* the 2013 core's default when a driver left b_max at 0 */
	cfg->b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;

	if (!mmc_create(cfg, priv))
		printf("%s: mmc_create failed\n", cfg->name);
}

void jz_mmc_init(void)
{
	__maybe_unused int i = 0;

#if defined(CONFIG_JZ_MMC_MSC0)
#ifdef CONFIG_T23
	gpio_set_func(GPIO_PORT_B, GPIO_FUNC_0, 0x3 << 4 | 0xf << 0);
#endif
	jz_mmc_init_one(i++, 0, MSC0_BASE, MSC0);
#endif
#if defined(CONFIG_JZ_MMC_MSC1)
	jz_mmc_init_one(i++, 1, MSC1_BASE, MSC1);
#endif
#if defined(CONFIG_JZ_MMC_MSC2)
	jz_mmc_init_one(i++, 2, MSC2_BASE, MSC2);
#endif
}
