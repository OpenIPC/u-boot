// SPDX-License-Identifier: GPL-2.0+
/*
 * AX620U cameras: the SPI NAND on the SSI at 0x08000000 is nand0, for
 * the environment, the nand command and the OpenIPC layer; misc_init_r()
 * runs the OpenIPC recovery and firmware scan; the Ethernet PHY is set
 * up for RMII as the stock U-Boot does.
 */
#include <common.h>
#include <dm.h>
#include <init.h>
#include <nand.h>
#include <openipc.h>
#include <phy.h>
#include <linux/mtd/mtd.h>

int board_init(void)
{
	return 0;
}

#ifdef CONFIG_SYS_NAND_SELF_INIT
/*
 * Mainline's spi-nand is a DM MTD device, registered by its probe as
 * spi-nand0. The environment and the nand command want it as nand0:
 * take it out of the MTD table and let nand_register() put it back
 * under that name (mtdids maps nand0 to the kernel's spi4.0_nand).
 */
void board_nand_init(void)
{
	struct udevice *dev;
	struct mtd_info *mtd;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_MTD,
					  DM_DRIVER_GET(spinand), &dev);
	if (ret) {
		printf("SPI NAND: not found (%d)\n", ret);
		return;
	}
	mtd = dev_get_uclass_priv(dev);
	del_mtd_device(mtd);
	ret = nand_register(0, mtd);
	if (ret)
		printf("SPI NAND: nand_register: %d\n", ret);
}
#endif

#ifdef CONFIG_PHYLIB
#define RTL8201F_UID		0x001cc816
#define RTL8201F_PAGE_SELECT	0x1f
#define RTL8201F_RMSR		0x10	/* page 7: RMII mode setting */
#define  RMSR_RMII_TXC_IN	BIT(12)	/* clear: TXC an output, the 50 MHz */
#define  RMSR_RMII_MODE		BIT(3)

/*
 * The RTL8201F the stock kernel's tree names: RMII mode, the PHY's TXC
 * an output, driving the 50 MHz reference clock (the MAC's
 * "rmii-phy-dir" "output"), as the stock U-Boot's driver for it sets
 * before autonegotiation; mainline's leaves both to the strap pins.
 */
static int rtl8201f_rmii(struct phy_device *phydev)
{
	int ret, rmsr;

	ret = phy_write(phydev, MDIO_DEVAD_NONE, RTL8201F_PAGE_SELECT, 7);
	if (ret < 0)
		return ret;
	rmsr = phy_read(phydev, MDIO_DEVAD_NONE, RTL8201F_RMSR);
	ret = rmsr < 0 ? rmsr :
	      phy_write(phydev, MDIO_DEVAD_NONE, RTL8201F_RMSR,
			(rmsr & ~RMSR_RMII_TXC_IN) | RMSR_RMII_MODE);
	/* back to page 0 whatever happened on page 7 */
	rmsr = phy_write(phydev, MDIO_DEVAD_NONE, RTL8201F_PAGE_SELECT, 0);
	return ret < 0 ? ret : rmsr;
}

int board_phy_config(struct phy_device *phydev)
{
	if ((phydev->phy_id & 0xffffff) == RTL8201F_UID) {
		int ret = rtl8201f_rmii(phydev);

		if (ret < 0) {
			printf("RTL8201F: RMII setup failed (%d)\n", ret);
			return ret;
		}
	}
	if (phydev->drv->config)
		return phydev->drv->config(phydev);
	return 0;
}
#endif

int misc_init_r(void)
{
	return openipc_helper();
}
