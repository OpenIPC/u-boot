// SPDX-License-Identifier: GPL-2.0+
/*
 * Ethernet MAC of the Novatek NA51089: a Synopsys DWC Ethernet QoS
 * (GMAC 4.x) core, run polled with one Tx and one Rx descriptor ring.
 *
 * The block has the MAC at 0x0000, the MTL (the FIFOs) at 0x0D00 and the
 * DMA, channel 0 from 0x1100, at 0x1000; Novatek's glue (interface and
 * clock selection, LEDs) at 0x3000 and the embedded PHY's analog part at
 * 0x3800. Board code sets those last two up before this driver runs
 * (board/novatek/na51089), and the order matters: the DMA software reset
 * only completes once the PHY feeds the MAC its Rx clock.
 *
 * Descriptors are four words, OWN in DES3 bit 31, each padded to a cache
 * line; the DMA skips the padding (DSL). The DMA works through a ring up
 * to the tail pointer, so a descriptor handed to it is followed by a
 * tail pointer write. The PHY sits on the MAC's clause 22 MDIO and is the
 * first one phylib finds there; its speed and duplex go into the MAC
 * configuration at each start.
 *
 * After the vendor U-Boot 2019.04 (drivers/net/eth_na51089.c, itself a
 * cut-down early dwc_eth_qos.c), without the flow control. Non-DM, as is
 * the rest of this port.
 */

#include <common.h>
#include <cpu_func.h>
#include <malloc.h>
#include <miiphy.h>
#include <net.h>
#include <phy.h>
#include <asm/cache.h>
#include <asm/io.h>
#include <asm/arch/na51089.h>
#include <linux/delay.h>

#define EQ(off)			(NVT_ETH_BASE + (off))

/* MAC */
#define MAC_CFG			EQ(0x000)
#define MAC_RXQ_CTRL0		EQ(0x0A0)
#define MAC_HW_FEATURE1		EQ(0x120)
#define MAC_MDIO_ADDR		EQ(0x200)
#define MAC_MDIO_DATA		EQ(0x204)
#define MAC_ADDR0_HI		EQ(0x300)
#define MAC_ADDR0_LO		EQ(0x304)
/* MTL, queue 0 */
#define MTL_TXQ0_OPMODE		EQ(0xD00)
#define MTL_TXQ0_DEBUG		EQ(0xD08)
#define MTL_RXQ0_OPMODE		EQ(0xD30)
#define MTL_RXQ0_DEBUG		EQ(0xD38)
/* DMA, channel 0 */
#define DMA_MODE		EQ(0x1000)
#define DMA_SYSBUS_MODE		EQ(0x1004)
#define DMA_CH_CTRL		EQ(0x1100)
#define DMA_CH_TX_CTRL		EQ(0x1104)
#define DMA_CH_RX_CTRL		EQ(0x1108)
#define DMA_CH_TXDESC_LIST	EQ(0x1114)
#define DMA_CH_RXDESC_LIST	EQ(0x111C)
#define DMA_CH_TXDESC_TAIL	EQ(0x1120)
#define DMA_CH_RXDESC_TAIL	EQ(0x1128)
#define DMA_CH_TXDESC_RING_LEN	EQ(0x112C)
#define DMA_CH_RXDESC_RING_LEN	EQ(0x1130)

/* MAC_CFG */
#define CFG_RE			BIT(0)
#define CFG_TE			BIT(1)
#define CFG_DM			BIT(13)		/* full duplex */
#define CFG_FES			BIT(14)		/* 100 Mb/s, with PS */
#define CFG_PS			BIT(15)		/* 10/100 Mb/s (MII) */
#define CFG_ACS			BIT(20)		/* strip the FCS */
#define CFG_CST			BIT(21)
/* MAC_RXQ_CTRL0 */
#define RXQ0_EN			2		/* enabled, generic */
/* MAC_HW_FEATURE1: FIFO sizes, log2(bytes / 128) */
#define HWF1_RXFIFO(v)		((v) & 0x1f)
#define HWF1_TXFIFO(v)		(((v) >> 6) & 0x1f)
/* MAC_MDIO_ADDR */
#define MDIO_GB			BIT(0)		/* busy */
#define MDIO_GOC_WRITE		(1 << 2)
#define MDIO_GOC_READ		(3 << 2)
#define MDIO_CR(n)		((n) << 8)	/* MDC from the CSR clock */
#define MDIO_RDA(n)		((n) << 16)
#define MDIO_PA(n)		((n) << 21)
/* MTL_TXQ0_OPMODE, MTL_RXQ0_OPMODE: queue sizes in 256-byte units, -1 */
#define TXQ_TSF			BIT(1)		/* store and forward */
#define TXQ_EN			(2 << 2)
#define TXQ_TQS(n)		((n) << 16)
#define RXQ_RSF			BIT(5)
#define RXQ_RQS(n)		((n) << 20)
/* MTL_TXQ0_DEBUG: data in the queue, or being read out */
#define TXQ_BUSY		(BIT(4) | BIT(1))
/* MTL_RXQ0_DEBUG: packets, or data, in the queue */
#define RXQ_BUSY		0x3fff0030
/* DMA_MODE */
#define DMA_SWR			BIT(0)
/* DMA_SYSBUS_MODE: outstanding AXI reads, as the vendor driver */
#define SYSBUS_RD_OSR_LMT(n)	((n) << 16)
/* DMA_CH_CTRL */
#define CH_PBLX8		BIT(16)
#define CH_DSL(n)		((n) << 18)	/* skip, in 8-byte units */
/* DMA_CH_TX_CTRL, DMA_CH_RX_CTRL */
#define CH_ST_SR		BIT(0)		/* start Tx, start Rx */
#define CH_TX_OSP		BIT(4)
#define CH_RBSZ(n)		((n) << 1)
#define CH_PBL(n)		((n) << 16)

#define DES3_OWN		BIT(31)
#define DES3_FD			BIT(29)
#define DES3_LD			BIT(28)
#define DES3_BUF1V		BIT(24)		/* read format, Rx */
#define DES3_ES			BIT(15)		/* write-back, Rx */
#define DES3_LEN(v)		((v) & 0x7fff)

/* The core's own descriptor, padded to a cache line */
struct eqos_desc {
	u32 des0, des1, des2, des3;
	u8 pad[ARCH_DMA_MINALIGN - 16];
};

#define TX_DESCS		4
#define RX_DESCS		16
/* A frame with a VLAN tag fits, a cache-line multiple */
#define PKT_SIZE		ALIGN(1568, ARCH_DMA_MINALIGN)

/* What the CSR clock (CG+0x10 [9:8]) is, as an MDC divider (CR) */
#define NVT_CG_SYSCLK		(NVT_CG_BASE + 0x10)
static const u8 mdio_cr[4] = {
	3,	/* 48 MHz: CR 35-60 MHz, /26 */
	0,	/* 60 MHz: CR 60-100 MHz, /42 */
	0,	/* 80 MHz */
	1,	/* 120 MHz: CR 100-150 MHz, /62 */
};

static struct eqos_desc tx_descs[TX_DESCS] __aligned(ARCH_DMA_MINALIGN);
static struct eqos_desc rx_descs[RX_DESCS] __aligned(ARCH_DMA_MINALIGN);
static u8 tx_buf[PKT_SIZE] __aligned(ARCH_DMA_MINALIGN);
static u8 rx_bufs[RX_DESCS][PKT_SIZE] __aligned(ARCH_DMA_MINALIGN);

static struct {
	struct eth_device dev;
	struct mii_dev *mii;
	struct phy_device *phy;
	phy_interface_t intf;
	u32 cr;
	int tx_idx, rx_idx;
} eqos;

static void eqos_flush(void *p, size_t len)
{
	flush_dcache_range((ulong)p, (ulong)p + len);
}

static void eqos_inval(void *p, size_t len)
{
	invalidate_dcache_range((ulong)p, (ulong)p + len);
}

/* Wait for (reg & mask) == 0 */
static int eqos_wait(ulong reg, u32 mask, ulong ms)
{
	ulong start = get_timer(0);

	while (readl(reg) & mask) {
		if (get_timer(start) > ms)
			return -ETIMEDOUT;
	}
	return 0;
}

static int eqos_mdio(int addr, int reg, u32 goc)
{
	writel(MDIO_PA(addr) | MDIO_RDA(reg) | MDIO_CR(eqos.cr) | goc | MDIO_GB,
	       MAC_MDIO_ADDR);
	return eqos_wait(MAC_MDIO_ADDR, MDIO_GB, 10);
}

static int eqos_mdio_read(struct mii_dev *bus, int addr, int devad, int reg)
{
	if (eqos_mdio(addr, reg, MDIO_GOC_READ))
		return -ETIMEDOUT;
	return readl(MAC_MDIO_DATA) & 0xffff;
}

static int eqos_mdio_write(struct mii_dev *bus, int addr, int devad, int reg,
			   u16 val)
{
	writel(val, MAC_MDIO_DATA);
	return eqos_mdio(addr, reg, MDIO_GOC_WRITE);
}

static int eqos_write_hwaddr(struct eth_device *dev)
{
	const u8 *a = dev->enetaddr;

	writel(a[4] | a[5] << 8, MAC_ADDR0_HI);
	writel(a[0] | a[1] << 8 | a[2] << 16 | a[3] << 24, MAC_ADDR0_LO);
	return 0;
}

static int eqos_start(struct eth_device *dev, struct bd_info *bis)
{
	struct phy_device *phy;
	u32 cfg, hwf1;
	int i, ret;

	if (!eqos.phy) {
		phy = phy_connect(eqos.mii, -1, dev, eqos.intf);
		if (!phy)
			return -ENODEV;
		phy_config(phy);
		eqos.phy = phy;
	}
	phy = eqos.phy;
	ret = phy_startup(phy);
	if (ret)
		return ret;
	if (!phy->link) {
		printf("%s: no link\n", dev->name);
		return -ENOLINK;
	}

	/*
	 * Back to the defaults. This needs the Rx clock from the PHY, so it
	 * comes once the link is up, as in the vendor driver.
	 */
	writel(DMA_SWR, DMA_MODE);
	if (eqos_wait(DMA_MODE, DMA_SWR, 100)) {
		printf("%s: DMA reset timed out, no Rx clock?\n", dev->name);
		return -ETIMEDOUT;
	}

	cfg = CFG_CST | CFG_ACS;
	if (phy->duplex)
		cfg |= CFG_DM;
	if (phy->speed == SPEED_100)
		cfg |= CFG_PS | CFG_FES;
	else if (phy->speed == SPEED_10)
		cfg |= CFG_PS;
	writel(cfg, MAC_CFG);

	/* All of each FIFO to queue 0, store and forward both ways */
	hwf1 = readl(MAC_HW_FEATURE1);
	writel(TXQ_TQS((128 << HWF1_TXFIFO(hwf1)) / 256 - 1) | TXQ_EN | TXQ_TSF,
	       MTL_TXQ0_OPMODE);
	writel(RXQ_RQS((128 << HWF1_RXFIFO(hwf1)) / 256 - 1) | RXQ_RSF,
	       MTL_RXQ0_OPMODE);
	writel(RXQ0_EN, MAC_RXQ_CTRL0);
	eqos_write_hwaddr(dev);

	/* Bursts of 16 x 8 beats, as the vendor driver */
	writel(CH_PBLX8 | CH_DSL((ARCH_DMA_MINALIGN - 16) / 8), DMA_CH_CTRL);
	writel(CH_PBL(16) | CH_TX_OSP, DMA_CH_TX_CTRL);
	writel(CH_PBL(16) | CH_RBSZ(PKT_SIZE), DMA_CH_RX_CTRL);
	writel(SYSBUS_RD_OSR_LMT(2), DMA_SYSBUS_MODE);

	memset(tx_descs, 0, sizeof(tx_descs));
	memset(rx_descs, 0, sizeof(rx_descs));
	for (i = 0; i < RX_DESCS; i++) {
		rx_descs[i].des0 = (ulong)rx_bufs[i];
		rx_descs[i].des3 = DES3_OWN | DES3_BUF1V;
	}
	eqos_flush(tx_descs, sizeof(tx_descs));
	eqos_flush(rx_descs, sizeof(rx_descs));
	eqos_inval(rx_bufs, sizeof(rx_bufs));
	eqos.tx_idx = 0;
	eqos.rx_idx = 0;
	writel((ulong)tx_descs, DMA_CH_TXDESC_LIST);
	writel(TX_DESCS - 1, DMA_CH_TXDESC_RING_LEN);
	writel((ulong)rx_descs, DMA_CH_RXDESC_LIST);
	writel(RX_DESCS - 1, DMA_CH_RXDESC_RING_LEN);

	setbits_le32(MAC_CFG, CFG_TE | CFG_RE);
	setbits_le32(DMA_CH_TX_CTRL, CH_ST_SR);
	setbits_le32(DMA_CH_RX_CTRL, CH_ST_SR);
	/*
	 * The DMA stops short of the tail: pointing it at the first
	 * descriptor would mean none is free, so it gets all but the last.
	 */
	writel((ulong)&rx_descs[RX_DESCS - 1], DMA_CH_RXDESC_TAIL);
	return 0;
}

static void eqos_halt(struct eth_device *dev)
{
	/* Let what is queued go out, and what came in drain */
	clrbits_le32(DMA_CH_TX_CTRL, CH_ST_SR);
	eqos_wait(MTL_TXQ0_DEBUG, TXQ_BUSY, 10);
	clrbits_le32(MAC_CFG, CFG_TE | CFG_RE);
	eqos_wait(MTL_RXQ0_DEBUG, RXQ_BUSY, 10);
	clrbits_le32(DMA_CH_RX_CTRL, CH_ST_SR);
}

static int eqos_send(struct eth_device *dev, void *packet, int len)
{
	struct eqos_desc *d = &tx_descs[eqos.tx_idx];
	ulong start;

	memcpy(tx_buf, packet, len);
	eqos_flush(tx_buf, ALIGN(len, ARCH_DMA_MINALIGN));

	d->des0 = (ulong)tx_buf;
	d->des1 = 0;
	d->des2 = len;
	mb();
	d->des3 = DES3_OWN | DES3_FD | DES3_LD | len;
	eqos_flush(d, sizeof(*d));

	eqos.tx_idx = (eqos.tx_idx + 1) % TX_DESCS;
	writel((ulong)&tx_descs[eqos.tx_idx], DMA_CH_TXDESC_TAIL);

	/* The buffer is reused: wait for the DMA to have read it */
	start = get_timer(0);
	do {
		eqos_inval(d, sizeof(*d));
		if (!(d->des3 & DES3_OWN))
			return 0;
	} while (get_timer(start) < 100);
	printf("%s: Tx timed out\n", dev->name);
	return -ETIMEDOUT;
}

static int eqos_recv(struct eth_device *dev)
{
	struct eqos_desc *d = &rx_descs[eqos.rx_idx];
	u8 *buf = rx_bufs[eqos.rx_idx];
	u32 des3;
	int len;

	eqos_inval(d, sizeof(*d));
	des3 = d->des3;
	if (des3 & DES3_OWN)
		return 0;

	/* One buffer holds any frame; anything else is an error */
	len = DES3_LEN(des3);
	if ((des3 & (DES3_FD | DES3_LD | DES3_ES)) == (DES3_FD | DES3_LD)) {
		eqos_inval(buf, PKT_SIZE);
		net_process_received_packet(buf, len);
	}

	/* Back to the DMA, which may then go up to it */
	d->des0 = (ulong)buf;
	d->des1 = 0;
	d->des2 = 0;
	mb();
	d->des3 = DES3_OWN | DES3_BUF1V;
	eqos_flush(d, sizeof(*d));
	writel((ulong)d, DMA_CH_RXDESC_TAIL);
	eqos.rx_idx = (eqos.rx_idx + 1) % RX_DESCS;
	return len;
}

/*
 * Register the MAC; the board has its clocks running, the pins muxed and
 * the PHY powered, @intf says which PHY that is.
 */
int nvt_eqos_initialize(phy_interface_t intf)
{
	struct eth_device *dev = &eqos.dev;
	struct mii_dev *mii;

	eqos.intf = intf;
	eqos.cr = mdio_cr[(readl(NVT_CG_SYSCLK) >> 8) & 3];

	mii = mdio_alloc();
	if (!mii)
		return -ENOMEM;
	strcpy(mii->name, "eqos");
	mii->read = eqos_mdio_read;
	mii->write = eqos_mdio_write;
	if (mdio_register(mii))
		return -EIO;
	eqos.mii = mii;

	strcpy(dev->name, "eqos");
	dev->iobase = NVT_ETH_BASE;
	dev->init = eqos_start;
	dev->halt = eqos_halt;
	dev->send = eqos_send;
	dev->recv = eqos_recv;
	dev->write_hwaddr = eqos_write_hwaddr;
	return eth_register(dev);
}
