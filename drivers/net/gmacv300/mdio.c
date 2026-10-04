/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include <errno.h>
#include <phy.h>
#include "gmac.h"
#include "ctrl.h"
#include "util.h"
#include "mdio.h"

static void mdio_set_phywrite_val(struct gmac_netdev_local const *ld, unsigned short val)
{
	u32 reg;

	gmac_writel_bits(ld, val, REG_MDIO_SINGLE_DATA, MDIO_WR_DATA);
	reg = gmac_readl(ld, REG_MDIO_SINGLE_DATA);
	gmac_trace(GMAC_TRACE_MDIO, "write reg 0x%x, bits:0x%x= 0x%x, then read = 0x%x",
			REG_MDIO_SINGLE_DATA, MDIO_WR_DATA, val, reg);
}

static int mdio_wait_ready(struct gmac_netdev_local const *ld)
{
	int timeout_us = 1000;

	while (--timeout_us && !test_mdio_ready(ld))
		udelay(1);

	return timeout_us;
}

static int gmac_mdio_read(struct gmac_netdev_local const *ld,
		  char const *devname, unsigned char phy,
		  unsigned char reg, unsigned short *value)
{
	int timeout = 1000;

	if (!mdio_wait_ready(ld))
		return -1;

	mdio_start_phyread(ld, phy, reg);

	while (!mdio_wait_ready(ld) && timeout-- > 0)
		udelay(1);

	if (timeout <= 0 || !test_mdio_read_data_done(ld)) {
		*value = 0;
		/* it should return Error(-1), but miiphy_read() will
		 * print error info, it's annoying
		 */
		return 0;
	}

	*value = mdio_get_phyread_val(ld);

	gmac_trace(GMAC_TRACE_MDIO, "mdio read phy:%x, reg:%x = %x\n",
		phy, reg, *value);

	return 0;
}

static int gmac_mdio_write(struct gmac_netdev_local *ld,
		  char const *devname, unsigned char phy,
		  unsigned char reg, unsigned short val)
{
	if (!mdio_wait_ready(ld))
		return -1;

	gmac_trace(GMAC_TRACE_MDIO, "mdio write phy:%x, reg:%x = %x\n",
		phy, reg, val);

	mdio_set_phywrite_val(ld, val);
	mdio_phywrite(ld, phy, reg);

	return 0;
}

static struct gmac_netdev_local *gmac_get_netdev_by_name(const char *devname)
{
	int i;

	for (i = 0; i < CONFIG_GMAC_NUMS; i++) {
		if (!strcmp(devname, g_gmac_board_info[i].mii_name))
			return &g_gmac_board_info[i].gmac_ld;
	}

	return NULL;
}

int gmac_mdiobus_read(struct mii_dev* const bus, int addr, int devad, int reg)
{
	struct gmac_netdev_local *ld = NULL;
	unsigned short value;
	int ret;

	if (bus == NULL)
		return -EINVAL;
	ld = gmac_get_netdev_by_name(bus->name);
	if (ld == NULL)
		return -EINVAL;
	ret = gmac_mdio_read(ld, bus->name, addr, reg, &value);

	return ret ? -1 : (int)value;
}

int gmac_mdiobus_write(struct mii_dev* const bus, int addr, int devad,
		  int reg, unsigned short val)
{
	struct gmac_netdev_local *ld = NULL;
	if (bus == NULL)
		return -EINVAL;
	ld = gmac_get_netdev_by_name(bus->name);
	if (ld == NULL)
		return -EINVAL;

	return gmac_mdio_write(ld, bus->name, addr, reg, val);
}
