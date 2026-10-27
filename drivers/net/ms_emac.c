// SPDX-License-Identifier: GPL-2.0+
/*
 * The EMAC of the SigmaStar infinity3 and its internal ("Albany") PHY.
 *
 * An Atmel AT91RM9200-style 10/100 MAC behind the RIU: register G (the AT91
 * numbering) is two 16-bit RIU registers, bits 15:0 at +2G and 31:16 at
 * +2G+4, written low half first. G >= 0x100 are MStar's additions.
 *
 * - Transmit: the frame's MIU address into TAR, then its length into TCR.
 * - Receive: in "software descriptor" mode, a ring of 8-byte descriptors
 *   at RBQP, each pointing at its own buffer: { addr | WRAP on the last,
 *   DONE set by the MAC; length in size[10:0] }.
 * - The PHY is reached over MDIO through MAN, registered as the MDIO bus
 *   "ms_emac" for the mii command.
 *
 * After the vendor U-Boot 2015.01 (drivers/mstar/emac, mhal_emac.c of
 * infinity3, CONFIG_ETHERNET_ALBANY and SOFTWARE_DESCRIPTOR as its
 * infinity3_defconfig builds it).
 */

#include <common.h>
#include <log.h>
#include <malloc.h>
#include <miiphy.h>
#include <net.h>
#include <netdev.h>
#include <time.h>
#include <wait_bit.h>
#include <asm/io.h>
#include <asm/arch/sigmastar.h>
#include <linux/delay.h>
#include <linux/mii.h>

#define EMAC_BASE		SS_BANK(SS_BANK_EMAC0)

/* Registers, by their 32-bit (AT91) offset */
#define EMAC_CTL		0x00
#define EMAC_CFG		0x04
#define EMAC_SR			0x08
#define EMAC_TAR		0x0C
#define EMAC_TCR		0x10
#define EMAC_TSR		0x14
#define EMAC_RBQP		0x18
#define EMAC_RBCFG		0x1C
#define EMAC_MAN		0x34
#define EMAC_RBRP		0x38
#define EMAC_RBWP		0x3C
#define EMAC_SA1L		0x98
#define EMAC_SA1H		0x9C
#define EMAC_SA2L		0xA0
#define EMAC_SA4H		0xB4
#define EMAC_J100		0x100
#define EMAC_J104		0x104
#define EMAC_J134		0x134
#define EMAC_J138		0x138

/* CTL */
#define CTL_RE			BIT(2)
#define CTL_TE			BIT(3)
#define CTL_MPE			BIT(4)
#define CTL_CSR			BIT(5)
/* CFG */
#define CFG_SPD			BIT(0)
#define CFG_FD			BIT(1)
#define CFG_MDC_HCLK_32		(2 << 10)
/* SR */
#define SR_IDLE			BIT(2)
/* TSR */
#define TSR_IDLE		BIT(3)
#define TSR_BNQ			BIT(4)
/* MAN */
#define MAN_HIGH		BIT(30)
#define MAN_RW_R		(2 << 28)
#define MAN_RW_W		(1 << 28)
#define MAN_PHYA(a)		((a) << 23)
#define MAN_REGA(r)		((r) << 18)
#define MAN_CODE		(2 << 16)
/* J100: the Albany PHY's mode, the vendor's JULIAN_100_VAL */
#define J100_ALBANY		0x0000F011
/* J104 */
#define J104_SW_DESC		BIT(0)

/* Descriptors */
#define DESC_DONE		BIT(0)
#define DESC_WRAP		BIT(1)
#define DESC_LEN		0x7ff

#define RX_DESCS		128
#define RX_BUF_SIZE		0x600
/* The ring and the buffers it points at: the vendor's 16 KiB alignment */
#define RX_ALIGN		0x4000
#define RX_BUFS_SIZE		(RX_DESCS * RX_BUF_SIZE)
#define RX_MEM_SIZE		(RX_BUFS_SIZE + RX_DESCS * sizeof(struct ms_desc))
/* RBCFG: the buffer area, size select 1 as the vendor sets it */
#define RBCFG_SEL		1

#define MDIO_TIMEOUT_MS		100
#define AN_TIMEOUT_MS		4000

struct ms_desc {
	u32 addr;
	u32 size;
};

struct ms_emac {
	struct eth_device dev;
	u8 *rx_bufs;
	struct ms_desc *rx_ring;
	unsigned int rx_next;
	int phy;
};

/*
 * Two TX buffers, used in turn: TSR's BNQ says only that the queue slot is
 * free, while the frame queued before may still be read from its buffer
 */
static u8 ms_tx_buf[2][PKTSIZE_ALIGN] __aligned(32);
static unsigned int ms_tx_next;

static u32 emac_readl(unsigned int reg)
{
	return (readl(EMAC_BASE + reg * 2) & 0xffff) |
	       (readl(EMAC_BASE + reg * 2 + 4) & 0xffff) << 16;
}

static void emac_writel(u32 val, unsigned int reg)
{
	writel(val & 0xffff, EMAC_BASE + reg * 2);
	writel(val >> 16, EMAC_BASE + reg * 2 + 4);
}

static void emac_clrsetbits(unsigned int reg, u32 clr, u32 set)
{
	emac_writel((emac_readl(reg) & ~clr) | set, reg);
}

/* The vendor's 8-bit RIU register access: byte r of bank b */
static void riu_writeb(u8 val, unsigned int bank, unsigned int r)
{
	writeb(val, SS_BANK(bank) + (r << 1) - (r & 1));
}

static u8 riu_readb(unsigned int bank, unsigned int r)
{
	return readb(SS_BANK(bank) + (r << 1) - (r & 1));
}

/*
 * One MDIO frame through MAN, with the management port on meanwhile;
 * SR's IDLE bit is in the register's low half
 */
static int ms_mdio_xfer(u32 man)
{
	u32 ctl = emac_readl(EMAC_CTL);
	int ret;

	emac_writel(ctl | CTL_MPE, EMAC_CTL);
	emac_writel(MAN_HIGH | MAN_CODE | man, EMAC_MAN);
	ret = wait_for_bit_le32((void *)(EMAC_BASE + EMAC_SR * 2), SR_IDLE,
				true, MDIO_TIMEOUT_MS, false);
	if (!ret)
		ret = emac_readl(EMAC_MAN) & 0xffff;
	emac_writel(ctl, EMAC_CTL);
	return ret;
}

static int ms_mdio_read(int phy, int reg)
{
	return ms_mdio_xfer(MAN_RW_R | MAN_PHYA(phy) | MAN_REGA(reg));
}

static int ms_mdio_write(int phy, int reg, u16 val)
{
	int ret = ms_mdio_xfer(MAN_RW_W | MAN_PHYA(phy) | MAN_REGA(reg) | val);

	return ret < 0 ? ret : 0;
}

static int ms_mdio_bus_read(struct mii_dev *bus, int phy, int devad, int reg)
{
	return ms_mdio_read(phy, reg);
}

static int ms_mdio_bus_write(struct mii_dev *bus, int phy, int devad, int reg,
			     u16 val)
{
	return ms_mdio_write(phy, reg, val);
}

/*
 * The vendor's MHal_EMAC_ClkGen() for the Albany PHY: RX descriptor mode 1,
 * the RX shift and TX underrun fixes, the MAC clocks, then the PHY's
 * analog front end, register by register as the vendor writes it.
 */
static const struct {
	u16 bank;
	u8 reg;
	u8 val;
} ms_albany_init[] = {
	{ SS_BANK_CLKGEN0,  0x84, 0x00 },	/* CLK_EMAC_AHB 123 MHz */
	{ SS_BANK_SCGPCTRL, 0x44, 0x00 },	/* CLK_EMAC_RX from the PHY */
	{ SS_BANK_SCGPCTRL, 0x46, 0x00 },	/* CLK_EMAC_TX from the PHY */
	{ SS_BANK_ALBANY1, 0xb4, 0x02 },	/* gain shift */
	{ SS_BANK_ALBANY1, 0x4f, 0x02 },	/* det max */
	{ SS_BANK_ALBANY1, 0x51, 0x01 },	/* det min */
	{ SS_BANK_ALBANY1, 0x77, 0x18 },	/* snr len (emc noise) */
	{ SS_BANK_ALBANY0, 0x72, 0xa0 },	/* lpbk_enable off */
	{ SS_BANK_ALBANY1, 0xfc, 0x00 },	/* power on: LDO */
	{ SS_BANK_ALBANY1, 0xfd, 0x00 },
	{ SS_BANK_ALBANY2, 0xa1, 0x80 },	/* SADC */
	{ SS_BANK_ALBANY1, 0xcc, 0x40 },	/* ADCPL */
	{ SS_BANK_ALBANY1, 0xbb, 0x04 },	/* REF */
	{ SS_BANK_ALBANY2, 0x3a, 0x00 },	/* TX */
	{ SS_BANK_ALBANY2, 0xf1, 0x00 },	/* TX */
	{ SS_BANK_ALBANY2, 0x8a, 0x01 },	/* CLKO_ADC_SEL */
	{ SS_BANK_ALBANY1, 0x3b, 0x01 },	/* adc_clk_select */
	{ SS_BANK_ALBANY1, 0xc4, 0x44 },	/* TEST */
	{ SS_BANK_ALBANY2, 0x80, 0x30 },	/* sadc timer */
	{ SS_BANK_ALBANY2, 0xc5, 0x00 },	/* 100 gat */
	{ SS_BANK_ALBANY2, 0x30, 0x43 },	/* 200 gat */
	{ SS_BANK_ALBANY2, 0x39, 0x41 },	/* en_100t_phase, save2x_tx */
	{ SS_BANK_ALBANY2, 0xf2, 0xf5 },	/* LP mode, DAC off */
	{ SS_BANK_ALBANY2, 0xf3, 0x0d },	/* DAC off */
	{ SS_BANK_ALBANY0, 0x79, 0xd0 },	/* no drops on inverted waveforms */
	{ SS_BANK_ALBANY0, 0x77, 0x5a },
	{ SS_BANK_ALBANY0, 0x2d, 0x7c },	/* EEE off */
	{ SS_BANK_ALBANY2, 0xe8, 0x06 },	/* 10T waveform */
	{ SS_BANK_ALBANY0, 0x2b, 0x00 },
	{ SS_BANK_ALBANY2, 0xe8, 0x00 },
	{ SS_BANK_ALBANY0, 0x2b, 0x00 },
	{ SS_BANK_ALBANY2, 0xe8, 0x06 },	/* shadow_ctrl */
	{ SS_BANK_ALBANY0, 0xaa, 0x1c },	/* tin17_s2 */
	{ SS_BANK_ALBANY0, 0xac, 0x1c },	/* tin18_s2 */
	{ SS_BANK_ALBANY0, 0xad, 0x1c },
	{ SS_BANK_ALBANY0, 0xae, 0x1c },	/* tin19_s2 */
	{ SS_BANK_ALBANY0, 0xaf, 0x1c },
	{ SS_BANK_ALBANY2, 0xe8, 0x00 },
	{ SS_BANK_ALBANY0, 0xaa, 0x1c },
	{ SS_BANK_ALBANY0, 0xab, 0x28 },
	{ SS_BANK_ALBANY1, 0xf5, 0x02 },	/* faster timing recovery */
	{ SS_BANK_ALBANY1, 0x0f, 0xc9 },	/* signal_det k */
	{ SS_BANK_ALBANY1, 0x89, 0x50 },	/* snr_h */
	{ SS_BANK_ALBANY1, 0x8b, 0x80 },
	{ SS_BANK_ALBANY1, 0x8e, 0x0e },
	{ SS_BANK_ALBANY1, 0x90, 0x04 },
	{ SS_BANK_ALBANY1, 0xc7, 0x80 },	/* clock source hv */
};

static void ms_emac_clk_init(void)
{
	unsigned int i;

	/* RX descriptor format mode 1 */
	riu_writeb(0x00, SS_BANK_EMAC1, 0x3a);
	riu_writeb(0x01, SS_BANK_EMAC1, 0x3b);
	/* RX shift and TX underrun fixes */
	riu_writeb(riu_readb(SS_BANK_EMAC1, 0x00) | 0x10, SS_BANK_EMAC1, 0x00);
	riu_writeb(riu_readb(SS_BANK_EMAC1, 0x39) | 0x01, SS_BANK_EMAC1, 0x39);

	for (i = 0; i < ARRAY_SIZE(ms_albany_init); i++)
		riu_writeb(ms_albany_init[i].val, ms_albany_init[i].bank,
			   ms_albany_init[i].reg);

	/* CHIPTOP allpad_in off */
	riu_writeb(riu_readb(SS_BANK_CHIPTOP, 0xa1) & 0x7f, SS_BANK_CHIPTOP,
		   0xa1);
	/* The link LED: PM sleep 0x28[5:4] = 01 */
	riu_writeb((riu_readb(SS_BANK_PMSLEEP, 0x50) & ~0x30) | 0x10,
		   SS_BANK_PMSLEEP, 0x50);
}

/* The PHY at the address the vendor starts from (0), else the first that answers */
static int ms_emac_find_phy(void)
{
	int phy, val;

	emac_writel(J100_ALBANY, EMAC_J100);
	for (phy = 0; phy < 32; phy++) {
		val = ms_mdio_read(phy, MII_BMSR);
		if (val > 0 && val != 0xffff)
			return phy;
	}
	printf("EMAC: no PHY answers, using address 0\n");
	return 0;
}

/*
 * CFG's speed and duplex bits for what both ends offer: BMSR 15:11 against
 * the partner's LPA 8:5, the best common mode as the vendor picks it
 */
static u32 ms_emac_cfg_mode(u16 bmsr, u16 lpa)
{
	unsigned int common = (bmsr >> 11) & (lpa >> 5) & 0xf;

	if (common & 0x8)
		return CFG_SPD | CFG_FD;
	if (common & 0x4)
		return CFG_SPD;
	if (common & 0x2)
		return CFG_FD;
	return 0;
}

/*
 * Auto-negotiation as the vendor runs it: enabled (not restarted), then up
 * to 4 s each for it to complete and for the link. Returns CFG's speed and
 * duplex bits, 10 Mbit/s half duplex when it fails.
 */
static u32 ms_emac_negotiate(int phy)
{
	ulong start;
	int bmsr;

	ms_mdio_write(phy, MII_BMCR, BMCR_ANENABLE);
	start = get_timer(0);
	for (;;) {
		bmsr = ms_mdio_read(phy, MII_BMSR);
		if (bmsr >= 0 && (bmsr & BMSR_ANEGCOMPLETE) &&
		    (bmsr & BMSR_LSTATUS))
			break;
		if (get_timer(start) > 2 * AN_TIMEOUT_MS) {
			printf("EMAC: no link\n");
			return 0;
		}
		mdelay(10);
	}
	return ms_emac_cfg_mode(bmsr, max(ms_mdio_read(phy, MII_LPA), 0));
}

static void ms_emac_set_mac(const u8 *mac)
{
	emac_writel(mac[3] << 24 | mac[2] << 16 | mac[1] << 8 | mac[0],
		    EMAC_SA1L);
	emac_writel(mac[5] << 8 | mac[4], EMAC_SA1H);
}

static int ms_emac_write_hwaddr(struct eth_device *dev)
{
	ms_emac_set_mac(dev->enetaddr);
	return 0;
}

static int ms_emac_init(struct eth_device *dev, struct bd_info *bis)
{
	struct ms_emac *priv = dev->priv;
	u32 cfg, rx_base = SS_MIU_ADDR(priv->rx_bufs);
	unsigned int i, reg;

	/* TX, RX and MDIO off, so nothing lands in the buffers meanwhile */
	emac_writel(0, EMAC_CTL);

	for (i = 0; i < RX_DESCS; i++) {
		priv->rx_ring[i].addr = rx_base + i * RX_BUF_SIZE;
		priv->rx_ring[i].size = 0;
	}
	priv->rx_ring[RX_DESCS - 1].addr |= DESC_WRAP;
	priv->rx_next = 0;
	ss_miu_flush();

	emac_writel(rx_base | RBCFG_SEL, EMAC_RBCFG);
	emac_writel(0, EMAC_RBRP);
	emac_writel(0, EMAC_RBWP);
	emac_writel(SS_MIU_ADDR(priv->rx_ring), EMAC_RBQP);

	ms_emac_set_mac(dev->enetaddr);
	for (reg = EMAC_SA2L; reg <= EMAC_SA4H; reg += 4)
		emac_writel(0, reg);
	emac_writel(J104_SW_DESC, EMAC_J104);

	/* Negotiate before CFG is written, as the vendor says to */
	cfg = ms_emac_negotiate(priv->phy);
	emac_writel(CFG_MDC_HCLK_32 | cfg, EMAC_CFG);
	emac_writel(CTL_MPE, EMAC_CTL);
	emac_writel(J100_ALBANY, EMAC_J100);

	/*
	 * The vendor's MHal_EMAC_HW_init(): MIU priority, TX threshold 4,
	 * burst-16, RX FIFO enlarged, the TX hang and MIU range fixes
	 */
	emac_clrsetbits(EMAC_J100, BIT(19), BIT(19));
	emac_clrsetbits(EMAC_J134, 0x00f00000 | BIT(29) | 0x03000000 |
			BIT(18) | BIT(28),
			4 << 20 | BIT(29) | 2 << 24 | BIT(18) | BIT(28));
	emac_clrsetbits(EMAC_J138, 0, BIT(0));

	emac_writel(emac_readl(EMAC_CTL) | CTL_CSR, EMAC_CTL);
	emac_writel(emac_readl(EMAC_CTL) | CTL_RE | CTL_TE, EMAC_CTL);
	return 0;
}

static int ms_emac_send(struct eth_device *dev, void *packet, int length)
{
	ulong start = get_timer(0);
	u32 tsr;
	u8 *buf;

	/* A free TX slot */
	do {
		tsr = emac_readl(EMAC_TSR);
		if (get_timer(start) > 100) {
			printf("EMAC: TX stuck (TSR %08x)\n", tsr);
			return -ETIMEDOUT;
		}
	} while (!(tsr & (TSR_IDLE | TSR_BNQ)));

	buf = ms_tx_buf[ms_tx_next];
	ms_tx_next ^= 1;
	memcpy(buf, packet, length);
	if (length < 60) {
		memset(buf + length, 0, 60 - length);
		length = 60;
	}
	ss_miu_flush();
	emac_writel(SS_MIU_ADDR(buf), EMAC_TAR);
	emac_writel(length, EMAC_TCR);
	return 0;
}

static int ms_emac_recv(struct eth_device *dev)
{
	struct ms_emac *priv = dev->priv;
	struct ms_desc *desc;
	unsigned int len;
	int n = 0;

	for (;;) {
		desc = &priv->rx_ring[priv->rx_next];
		if (!(readl(&desc->addr) & DESC_DONE))
			break;
		len = readl(&desc->size) & DESC_LEN;
		/* The length includes the FCS */
		if (len > 4)
			net_process_received_packet(priv->rx_bufs +
						    priv->rx_next * RX_BUF_SIZE,
						    len - 4);
		/* The read pointer, as the vendor advances it */
		emac_writel(emac_readl(EMAC_RBRP) + ALIGN(len, 4), EMAC_RBRP);
		writel(readl(&desc->addr) & ~DESC_DONE, &desc->addr);
		ss_miu_flush();
		priv->rx_next = (priv->rx_next + 1) % RX_DESCS;
		n++;
	}
	return n;
}

static void ms_emac_halt(struct eth_device *dev)
{
	emac_writel(emac_readl(EMAC_CTL) & ~(CTL_TE | CTL_RE), EMAC_CTL);
}

static int ms_emac_mdio_register(void)
{
	struct mii_dev *bus = mdio_alloc();

	if (!bus)
		return -ENOMEM;
	strlcpy(bus->name, "ms_emac", MDIO_NAME_LEN);
	bus->read = ms_mdio_bus_read;
	bus->write = ms_mdio_bus_write;
	return mdio_register(bus);
}

int ms_emac_initialize(void)
{
	struct ms_emac *priv;
	u8 *mem;
	int ret;

	priv = calloc(1, sizeof(*priv));
	mem = memalign(RX_ALIGN, RX_MEM_SIZE);
	if (!priv || !mem) {
		free(priv);
		free(mem);
		return -ENOMEM;
	}
	memset(mem, 0, RX_MEM_SIZE);
	priv->rx_bufs = mem;
	priv->rx_ring = (struct ms_desc *)(mem + RX_BUFS_SIZE);

	ms_emac_clk_init();
	priv->phy = ms_emac_find_phy();

	strcpy(priv->dev.name, "ms_emac");
	priv->dev.priv = priv;
	priv->dev.init = ms_emac_init;
	priv->dev.halt = ms_emac_halt;
	priv->dev.send = ms_emac_send;
	priv->dev.recv = ms_emac_recv;
	priv->dev.write_hwaddr = ms_emac_write_hwaddr;
	ret = eth_register(&priv->dev);
	return ret ? ret : ms_emac_mdio_register();
}
