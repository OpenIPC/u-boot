// SPDX-License-Identifier: GPL-2.0+
/*
 * Faraday FTGMAC100 Ethernet, without driver model
 *
 * (C) Copyright 2009 Faraday Technology
 * Po-Yu Chuang <ratbert@faraday-tech.com>
 *
 * (C) Copyright 2010 Andes Technology
 * Macpaul Lin <macpaul@andestech.com>
 *
 * This is the driver as it was before its conversion to driver model
 * (f95de0bd104 "net: ftgmac100: convert to driver model"), for boards
 * that do not use driver model. Unlike that driver, it programs the MAC's
 * speed and duplex from the PHY whenever the link is up; it used to leave
 * a MAC at 10 Mbit/s talking to a PHY at 100 when the PHY had been found
 * with its link up.
 */

#include <common.h>
#include <malloc.h>
#include <net.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/dma-mapping.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/mii.h>

#include "ftgmac100.h"

#define ETH_ZLEN	60
#define CFG_XBUF_SIZE	1536

/* RBSR - hw default init value is also 0x640 */
#define RBSR_DEFAULT_VALUE	0x640

/* PKTBUFSTX/PKTBUFSRX must both be power of 2 */
#define PKTBUFSTX	4	/* must be power of 2 */

#define PHY_MAX_ADDR	32

struct ftgmac100_data {
	ulong txdes_dma;
	struct ftgmac100_txdes *txdes;
	ulong rxdes_dma;
	struct ftgmac100_rxdes *rxdes;
	int tx_index;
	int rx_index;
	int phy_addr;
};

static int ftgmac100_mdiobus_read(struct eth_device *dev, int phy_addr,
				  int regnum)
{
	struct ftgmac100 *ftgmac100 = (struct ftgmac100 *)dev->iobase;
	int phycr;
	int i;

	phycr = readl(&ftgmac100->phycr);

	/* preserve MDC cycle threshold */
	phycr &= FTGMAC100_PHYCR_MDC_CYCTHR_MASK;

	phycr |= FTGMAC100_PHYCR_PHYAD(phy_addr)
	      |  FTGMAC100_PHYCR_REGAD(regnum)
	      |  FTGMAC100_PHYCR_MIIRD;

	writel(phycr, &ftgmac100->phycr);

	for (i = 0; i < 10; i++) {
		phycr = readl(&ftgmac100->phycr);

		if ((phycr & FTGMAC100_PHYCR_MIIRD) == 0) {
			int data;

			data = readl(&ftgmac100->phydata);
			return FTGMAC100_PHYDATA_MIIRDATA(data);
		}

		mdelay(10);
	}

	debug("mdio read timed out\n");
	return -1;
}

static int ftgmac100_mdiobus_write(struct eth_device *dev, int phy_addr,
				   int regnum, u16 value)
{
	struct ftgmac100 *ftgmac100 = (struct ftgmac100 *)dev->iobase;
	int phycr;
	int data;
	int i;

	phycr = readl(&ftgmac100->phycr);

	/* preserve MDC cycle threshold */
	phycr &= FTGMAC100_PHYCR_MDC_CYCTHR_MASK;

	phycr |= FTGMAC100_PHYCR_PHYAD(phy_addr)
	      |  FTGMAC100_PHYCR_REGAD(regnum)
	      |  FTGMAC100_PHYCR_MIIWR;

	data = FTGMAC100_PHYDATA_MIIWDATA(value);

	writel(data, &ftgmac100->phydata);
	writel(phycr, &ftgmac100->phycr);

	for (i = 0; i < 10; i++) {
		phycr = readl(&ftgmac100->phycr);

		if ((phycr & FTGMAC100_PHYCR_MIIWR) == 0)
			return 0;

		mdelay(1);
	}

	debug("mdio write timed out\n");
	return -1;
}

static int ftgmac100_phy_read(struct eth_device *dev, int addr, int reg,
			      u16 *value)
{
	int data = ftgmac100_mdiobus_read(dev, addr, reg);

	if (data < 0)
		return -1;
	*value = data;
	return 0;
}

static int ftgmac100_phy_write(struct eth_device *dev, int addr, int reg,
			       u16 value)
{
	return ftgmac100_mdiobus_write(dev, addr, reg, value);
}

static int ftgmac100_phy_reset(struct eth_device *dev)
{
	struct ftgmac100_data *priv = dev->priv;
	int i;
	u16 status = 0;

	ftgmac100_phy_write(dev, priv->phy_addr, MII_ADVERTISE,
			    ADVERTISE_CSMA | ADVERTISE_ALL);

	printf("%s: Starting autonegotiation...\n", dev->name);

	ftgmac100_phy_write(dev, priv->phy_addr, MII_BMCR,
			    BMCR_ANENABLE | BMCR_ANRESTART);

	for (i = 0; i < 100000 / 100; i++) {
		ftgmac100_phy_read(dev, priv->phy_addr, MII_BMSR, &status);

		if (status & BMSR_ANEGCOMPLETE)
			break;
		mdelay(1);
	}

	if (status & BMSR_ANEGCOMPLETE) {
		printf("%s: Autonegotiation complete\n", dev->name);
	} else {
		printf("%s: Autonegotiation timed out (status=0x%04x)\n",
		       dev->name, status);
		return 0;
	}

	return 1;
}

/* Find the PHY and wait for its link: 1 when the link is up */
static int ftgmac100_phy_init(struct eth_device *dev)
{
	struct ftgmac100_data *priv = dev->priv;
	int phy_addr;
	u16 phy_id = 0xffff, status = 0;
	int i;

	for (phy_addr = 0; phy_addr < PHY_MAX_ADDR; phy_addr++) {
		if (ftgmac100_phy_read(dev, phy_addr, MII_PHYSID1, &phy_id))
			phy_id = 0xffff;

		/*
		 * When it is unable to found PHY,
		 * the interface usually return 0xffff or 0x0000
		 */
		if (phy_id != 0xffff && phy_id != 0x0) {
			debug("%s: found PHY at 0x%02x\n", dev->name, phy_addr);
			priv->phy_addr = phy_addr;
			break;
		}
	}

	if (phy_id == 0xffff || phy_id == 0x0) {
		printf("%s: no PHY present\n", dev->name);
		return 0;
	}

	ftgmac100_phy_read(dev, priv->phy_addr, MII_BMSR, &status);

	if (!(status & BMSR_LSTATUS)) {
		/* Try to re-negotiate if we don't have link already. */
		ftgmac100_phy_reset(dev);

		for (i = 0; i < 100000 / 100; i++) {
			ftgmac100_phy_read(dev, priv->phy_addr,
					   MII_BMSR, &status);
			if (status & BMSR_LSTATUS)
				break;
			udelay(100);
		}
	}

	if (!(status & BMSR_LSTATUS)) {
		printf("%s: link down\n", dev->name);
		return 0;
	}

	return 1;
}

/* Set the MAC's speed and duplex to what the PHY negotiated */
static void ftgmac100_update_link_speed(struct eth_device *dev)
{
	struct ftgmac100 *ftgmac100 = (struct ftgmac100 *)dev->iobase;
	struct ftgmac100_data *priv = dev->priv;
	u16 adv = 0, lpa = 0;
	unsigned int maccr;
	int media, speed, duplex;

	ftgmac100_phy_read(dev, priv->phy_addr, MII_ADVERTISE, &adv);
	ftgmac100_phy_read(dev, priv->phy_addr, MII_LPA, &lpa);

	media = mii_nway_result(lpa & adv);
	speed = media & (ADVERTISE_100FULL | ADVERTISE_100HALF) ? 100 : 10;
	duplex = media & ADVERTISE_FULL ? 1 : 0;

	maccr = readl(&ftgmac100->maccr) &
		~(FTGMAC100_MACCR_GIGA_MODE |
		  FTGMAC100_MACCR_FAST_MODE |
		  FTGMAC100_MACCR_FULLDUP);
	if (speed == 100)
		maccr |= FTGMAC100_MACCR_FAST_MODE;
	if (duplex)
		maccr |= FTGMAC100_MACCR_FULLDUP;
	writel(maccr, &ftgmac100->maccr);

	printf("%s: link up, %dMbps %s-duplex\n",
	       dev->name, speed, duplex ? "full" : "half");
}

/*
 * Reset MAC
 */
static void ftgmac100_reset(struct eth_device *dev)
{
	struct ftgmac100 *ftgmac100 = (struct ftgmac100 *)dev->iobase;

	debug("%s()\n", __func__);

	writel(FTGMAC100_MACCR_SW_RST, &ftgmac100->maccr);

	while (readl(&ftgmac100->maccr) & FTGMAC100_MACCR_SW_RST)
		;
}

/*
 * Set MAC address
 */
static void ftgmac100_set_mac(struct eth_device *dev,
			      const unsigned char *mac)
{
	struct ftgmac100 *ftgmac100 = (struct ftgmac100 *)dev->iobase;
	unsigned int maddr = mac[0] << 8 | mac[1];
	unsigned int laddr = mac[2] << 24 | mac[3] << 16 | mac[4] << 8 | mac[5];

	debug("%s(%x %x)\n", __func__, maddr, laddr);

	writel(maddr, &ftgmac100->mac_madr);
	writel(laddr, &ftgmac100->mac_ladr);
}

static int ftgmac100_write_hwaddr(struct eth_device *dev)
{
	ftgmac100_set_mac(dev, dev->enetaddr);
	return 0;
}

/*
 * disable transmitter, receiver
 */
static void ftgmac100_halt(struct eth_device *dev)
{
	struct ftgmac100 *ftgmac100 = (struct ftgmac100 *)dev->iobase;

	debug("%s()\n", __func__);

	writel(0, &ftgmac100->maccr);
}

static int ftgmac100_init(struct eth_device *dev, struct bd_info *bd)
{
	struct ftgmac100 *ftgmac100 = (struct ftgmac100 *)dev->iobase;
	struct ftgmac100_data *priv = dev->priv;
	struct ftgmac100_txdes *txdes;
	struct ftgmac100_rxdes *rxdes;
	unsigned int maccr;
	void *buf;
	int i;

	debug("%s()\n", __func__);

	if (!priv->txdes) {
		txdes = dma_alloc_coherent(sizeof(*txdes) * PKTBUFSTX,
					   &priv->txdes_dma);
		if (!txdes)
			panic("ftgmac100: out of memory\n");
		memset(txdes, 0, sizeof(*txdes) * PKTBUFSTX);
		priv->txdes = txdes;
	}
	txdes = priv->txdes;

	if (!priv->rxdes) {
		rxdes = dma_alloc_coherent(sizeof(*rxdes) * PKTBUFSRX,
					   &priv->rxdes_dma);
		if (!rxdes)
			panic("ftgmac100: out of memory\n");
		memset(rxdes, 0, sizeof(*rxdes) * PKTBUFSRX);
		priv->rxdes = rxdes;
	}
	rxdes = priv->rxdes;

	/*
	 * Each session starts from a reset MAC, as the driver model driver's
	 * does: the reset puts the MAC's ring pointers back at the first
	 * descriptor, where tx_index and rx_index start.
	 */
	ftgmac100_reset(dev);

	/* set the ethernet address */
	ftgmac100_set_mac(dev, dev->enetaddr);

	/* disable all interrupts */
	writel(0, &ftgmac100->ier);

	/* initialize descriptors */
	priv->tx_index = 0;
	priv->rx_index = 0;

	for (i = 0; i < PKTBUFSTX; i++) {
		/* TXBUF_BADR */
		if (!txdes[i].txdes2) {
			buf = memalign(ARCH_DMA_MINALIGN, CFG_XBUF_SIZE);
			if (!buf)
				panic("ftgmac100: out of memory\n");
			txdes[i].txdes3 = virt_to_phys(buf);
			txdes[i].txdes2 = (uint)buf;
		}
		/* owned by software, and nothing left of the last session */
		txdes[i].txdes0 = 0;
		txdes[i].txdes1 = 0;
	}
	txdes[PKTBUFSTX - 1].txdes0 = FTGMAC100_TXDES0_EDOTR;

	for (i = 0; i < PKTBUFSRX; i++) {
		/* RXBUF_BADR */
		if (!rxdes[i].rxdes2) {
			buf = net_rx_packets[i];
			rxdes[i].rxdes3 = virt_to_phys(buf);
			rxdes[i].rxdes2 = (uint)buf;
		}
		/* free for the MAC */
		rxdes[i].rxdes0 = 0;
	}
	rxdes[PKTBUFSRX - 1].rxdes0 = FTGMAC100_RXDES0_EDORR;

	/* transmit ring */
	writel(priv->txdes_dma, &ftgmac100->txr_badr);

	/* receive ring */
	writel(priv->rxdes_dma, &ftgmac100->rxr_badr);

	/* poll receive descriptor automatically */
	writel(FTGMAC100_APTC_RXPOLL_CNT(1), &ftgmac100->aptc);

	/* config receive buffer size register */
	writel(FTGMAC100_RBSR_SIZE(RBSR_DEFAULT_VALUE), &ftgmac100->rbsr);

	/* enable transmitter, receiver */
	maccr = FTGMAC100_MACCR_TXMAC_EN |
		FTGMAC100_MACCR_RXMAC_EN |
		FTGMAC100_MACCR_TXDMA_EN |
		FTGMAC100_MACCR_RXDMA_EN |
		FTGMAC100_MACCR_CRC_APD |
		FTGMAC100_MACCR_FULLDUP |
		FTGMAC100_MACCR_RX_RUNT |
		FTGMAC100_MACCR_RX_BROADPKT;

	writel(maccr, &ftgmac100->maccr);

	if (!ftgmac100_phy_init(dev))
		return -1;
	ftgmac100_update_link_speed(dev);

	return 0;
}

/*
 * Get a data block via Ethernet
 */
static int ftgmac100_recv(struct eth_device *dev)
{
	struct ftgmac100_data *priv = dev->priv;
	struct ftgmac100_rxdes *curr_des;
	unsigned short rxlen;

	curr_des = &priv->rxdes[priv->rx_index];

	if (!(curr_des->rxdes0 & FTGMAC100_RXDES0_RXPKT_RDY))
		return -1;

	if (curr_des->rxdes0 & (FTGMAC100_RXDES0_RX_ERR |
				FTGMAC100_RXDES0_CRC_ERR |
				FTGMAC100_RXDES0_FTL |
				FTGMAC100_RXDES0_RUNT |
				FTGMAC100_RXDES0_RX_ODD_NB)) {
		/* drop it, and give the descriptor back */
		curr_des->rxdes0 &= ~FTGMAC100_RXDES0_RXPKT_RDY;
		priv->rx_index = (priv->rx_index + 1) % PKTBUFSRX;
		return -1;
	}

	rxlen = FTGMAC100_RXDES0_VDBC(curr_des->rxdes0);

	debug("%s(): RX buffer %d, %x received\n",
	      __func__, priv->rx_index, rxlen);

	/* invalidate d-cache */
	dma_map_single((void *)curr_des->rxdes2, rxlen, DMA_FROM_DEVICE);

	/* pass the packet up to the protocol layers. */
	net_process_received_packet((void *)curr_des->rxdes2, rxlen);

	/* release buffer to DMA */
	curr_des->rxdes0 &= ~FTGMAC100_RXDES0_RXPKT_RDY;

	priv->rx_index = (priv->rx_index + 1) % PKTBUFSRX;

	return 0;
}

/*
 * Send a data block via Ethernet
 */
static int ftgmac100_send(struct eth_device *dev, void *packet, int length)
{
	struct ftgmac100 *ftgmac100 = (struct ftgmac100 *)dev->iobase;
	struct ftgmac100_data *priv = dev->priv;
	struct ftgmac100_txdes *curr_des = &priv->txdes[priv->tx_index];

	if (curr_des->txdes0 & FTGMAC100_TXDES0_TXDMA_OWN) {
		debug("%s(): no TX descriptor available\n", __func__);
		return -1;
	}

	debug("%s(%x, %x)\n", __func__, (int)packet, length);

	length = (length < ETH_ZLEN) ? ETH_ZLEN : length;

	memcpy((void *)curr_des->txdes2, (void *)packet, length);
	dma_map_single((void *)curr_des->txdes2, length, DMA_TO_DEVICE);

	/* only one descriptor on TXBUF */
	curr_des->txdes0 &= FTGMAC100_TXDES0_EDOTR;
	curr_des->txdes0 |= FTGMAC100_TXDES0_FTS |
			    FTGMAC100_TXDES0_LTS |
			    FTGMAC100_TXDES0_TXBUF_SIZE(length) |
			    FTGMAC100_TXDES0_TXDMA_OWN;

	/* start transmit */
	writel(1, &ftgmac100->txpd);

	debug("%s(): packet sent\n", __func__);

	priv->tx_index = (priv->tx_index + 1) % PKTBUFSTX;

	return 0;
}

int ftgmac100_initialize(ulong base_addr)
{
	struct eth_device *dev;
	struct ftgmac100_data *priv;

	dev = calloc(1, sizeof(*dev));
	if (!dev) {
		printf("%s(): failed to allocate dev\n", __func__);
		return -ENOMEM;
	}

	/* Transmit and receive descriptors should align to 16 bytes */
	priv = memalign(16, sizeof(struct ftgmac100_data));
	if (!priv) {
		printf("%s(): failed to allocate priv\n", __func__);
		free(dev);
		return -ENOMEM;
	}
	memset(priv, 0, sizeof(*priv));

	strcpy(dev->name, "FTGMAC100");
	dev->iobase	= base_addr;
	dev->init	= ftgmac100_init;
	dev->halt	= ftgmac100_halt;
	dev->send	= ftgmac100_send;
	dev->recv	= ftgmac100_recv;
	dev->write_hwaddr = ftgmac100_write_hwaddr;
	dev->priv	= priv;

	ftgmac100_reset(dev);

	return eth_register(dev);
}
