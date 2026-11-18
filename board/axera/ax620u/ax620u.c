// SPDX-License-Identifier: GPL-2.0+
/*
 * AX620U cameras: the SPI NAND on the SSI at 0x08000000 is nand0, for
 * the environment, the nand command and the OpenIPC layer; misc_init_r()
 * runs the OpenIPC recovery and firmware scan.
 */
#include <common.h>
#include <dm.h>
#include <init.h>
#include <nand.h>
#include <openipc.h>
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

int misc_init_r(void)
{
	return openipc_helper();
}
