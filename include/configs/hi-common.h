/*
 * Common configuration for Hisilicon Hi35xx targets.
 *
 * The environment is defined here once per family and must stay universal
 * for all targets; per-SoC config headers may only override the parameter
 * macros below before including this file.
 */

#ifndef __HI_COMMON_H
#define __HI_COMMON_H

#define GZIP_HEAD_SIZE   0X10
#define HEAD_MAGIC_NUM0 0X70697A67 /* 'g''z''i''p' */
#define HEAD_MAGIC_NUM0_OFFSET 0X8
#define HEAD_MAGIC_NUM1 0X64616568 /* 'h''e''a''d' */
#define HEAD_MAGIC_NUM1_OFFSET 0XC
#define COMPRESSED_SIZE_OFFSET      0X0
#define UNCOMPRESSED_SIZE_OFFSET    0X4

#define CONFIG_ENV_OFFSET       0x40000
#define CONFIG_ENV_SIZE         0x10000

#ifndef CONFIG_SYS_NAND_MAX_CHIPS
#define CONFIG_SYS_NAND_MAX_CHIPS 1
#endif

#ifdef CONFIG_ARCH_BSP

/* CV6xx family (hi3516cv610 / hi3516cv608 / hi3519dv500) */
#define CONFIG_SYS_INIT_RAM_ADDR    0x41700000
#define CONFIG_SYS_INIT_RAM_SIZE    0x4000

#define MTDIDS "sfc"
#define VENDOR "hisilicon"

#define CONFIG_SYS_CBSIZE 1024

#define CONFIG_IPADDR 192.168.1.108
#define CONFIG_NETMASK 255.255.255.0
#define CONFIG_GATEWAYIP 192.168.1.1
#define CONFIG_SERVERIP 192.168.1.107

#ifdef CONFIG_MMC
#define CONFIG_CMD_MMC
#endif

#define CONFIG_BOOTARGS "mem=\\${osmem} console=ttyAMA0,115200 panic=20 root=/dev/mtdblock3 rootfstype=squashfs init=/init mtdparts=\\${mtdids}:\\${mtdparts} \\${extras}"
#define CONFIG_BOOTCOMMAND "setenv bootcmd ${bootnor}; sf probe 0; saveenv; run bootcmd"
#define MTDPARTS "256k(boot),64k(env),2048k(kernel),5120k(rootfs),7168k@0x50000(firmware),-(rootfs_data)"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"baseaddr=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"kernaddr=0x50000\0" \
	"kernsize=0x200000\0" \
	"rootaddr=0x250000\0" \
	"rootsize=0x500000\0" \
	"bootnor=run fwrecovery; sf probe 0; setenv setargs setenv bootargs ${bootargs}; run setargs; sf read ${baseaddr} ${kernaddr} ${kernsize}; bootm ${baseaddr}\0" \
	"ubnor=${fetchcmd} ${baseaddr} u-boot-${soc}-nor.bin && run ubwrite\0" \
	"ubwrite=sf probe 0; sf erase 0x0 ${kernaddr}; sf write ${baseaddr} 0x0 ${kernaddr}\0" \
	"fwupd=${fetchcmd} ${baseaddr} firmware.${soc} && run fwwrite\0" \
	"fwrecovery=if env exists bootfail; then run fwupd; sleep 5; reset; fi\0" \
	"fwwrite=sf probe 0; sf erase ${kernaddr} ${filesize}; sf write ${baseaddr} ${kernaddr} ${filesize}\0" \
	"nfsroot=/srv/nfs/" __stringify(PRODUCT_SOC) "\0" \
	"bootargsnfs=mem=\${osmem} console=ttyAMA0,115200 panic=20 root=/dev/nfs rootfstype=nfs ip=${ipaddr}:::255.255.255.0::eth0 nfsroot=${serverip}:${nfsroot},v3,nolock rw \${extras}\0" \
	"bootnfs=setenv setargs setenv bootargs ${bootargsnfs}; run setargs; tftpboot ${baseaddr} uImage.${soc}; bootm ${baseaddr}\0" \
	"sdload=setenv fetchcmd fatload mmc 0\0" \
	"norload=sf probe 0; sf read\0" \
	"fetchcmd=tftpboot\0" \
	"osmem=32M\0" \
	"mtdids=" MTDIDS "\0" \
	"mtdparts=\${mtdids}:" MTDPARTS "\0" \
	"bootargs=" CONFIG_BOOTARGS "\0" \
	"board_name\0" \
	"board=" __stringify(PRODUCT_SOC) "\0" \
	"vendor=" VENDOR "\0" \
	"soc=" __stringify(PRODUCT_SOC) "\0" \
	"socmodel=" __stringify(PRODUCT_SOCMODEL)

#ifdef CONFIG_FMC_SPI_NAND1

#define MTDIDS "nand"

#define CONFIG_BOOTARGS "mem=\\${osmem} console=ttyAMA0,115200 panic=20 init=/init root=/dev/ubiblock0_1 ubi.mtd=2,2048 ubi.block=0,1 mtdparts=\\${mtdids}:\\${mtdparts} \\${extras}"
#define CONFIG_BOOTCOMMAND "setenv setargs setenv bootargs ${bootargs}; run setargs; ubi part ubi; ubi read ${baseaddr} kernel; bootm ${baseaddr}; reset"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"baseaddr=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"fwupd=${fetchcmd} ${baseaddr} firmware.ubi.${soc} && nand erase 0x100000 0x7f00000; nand write ${baseaddr} 0x100000 ${filesize}\0" \
	"mtdparts=mtdparts="MTDIDS":768k(boot),256k(env),-(ubi)\0" \
	"nfsroot=/srv/nfs/" __stringify(PRODUCT_SOC) "\0" \
	"bootargsnfs=mem=\${osmem} console=ttyAMA0,115200 panic=20 root=/dev/nfs rootfstype=nfs ip=${ipaddr}:::255.255.255.0::eth0 nfsroot=${serverip}:${nfsroot},v3,nolock rw \${extras}\0" \
	"bootargs="CONFIG_BOOTARGS"\0" \
	"bootnfs=setenv setargs setenv bootargs ${bootargsnfs}; run setargs; tftpboot ${baseaddr} uImage.${soc}; bootm ${baseaddr}\0" \
	"osmem=32M\0" \
	"mtdids=nand0="MTDIDS"\0" \
	"mtdparts=\${mtdids}:" MTDPARTS "\0" \
	"board_name\0" \
	"board=" __stringify(PRODUCT_SOC) "\0" \
	"vendor=" VENDOR "\0" \
	"soc=" __stringify(PRODUCT_SOC) "\0" \
	"socmodel=" __stringify(PRODUCT_SOCMODEL)

#define CONFIG_SYS_MALLOC_LEN (32 * SZ_128K)
#endif

#else /* !CONFIG_ARCH_BSP */

/* V500 family (hi3516cv500 / hi3516dv300 / hi3516av300) */

#ifndef HI_ENV_BASEADDR
#define HI_ENV_BASEADDR 0x82000000
#endif

#ifndef HI_ENV_OSMEM
#define HI_ENV_OSMEM "128M"
#endif

#undef CONFIG_BOOTARGS
#undef CONFIG_BOOTCOMMAND
#undef CONFIG_BOOTDELAY
#undef CONFIG_EXTRA_ENV_SETTINGS
#undef CONFIG_SYS_PROMPT

#define CONFIG_SYS_PROMPT "OpenIPC # "

#ifndef PRODUCT_SOCMODEL
#define PRODUCT_SOCMODEL
#endif

#ifndef CONFIG_FMC_SPI_NAND

#define MTDIDS "hi_sfc"
#define VENDOR "hisilicon"
#define MTDPARTS "256k(boot),64k(env),2048k(kernel),5120k(rootfs),7168k@0x50000(firmware),-(rootfs_data)"

#define CONFIG_BOOTARGS "mem=\\${osmem} console=ttyAMA0,115200 panic=20 root=/dev/mtdblock3 rootfstype=squashfs init=/init mtdparts=\\${mtdids}:\\${mtdparts} \\${extras}"
#define CONFIG_BOOTCOMMAND "setenv bootcmd ${bootnor}; sf probe 0; saveenv; run bootcmd"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"baseaddr=" __stringify(HI_ENV_BASEADDR) "\0" \
	"kernaddr=0x50000\0" \
	"kernsize=0x200000\0" \
	"rootaddr=0x250000\0" \
	"rootsize=0x500000\0" \
	"bootnor=run fwrecovery; sf probe 0; setenv setargs setenv bootargs ${bootargs}; run setargs; sf read ${baseaddr} ${kernaddr} ${kernsize}; bootm ${baseaddr}\0" \
	"ubnor=${fetchcmd} ${baseaddr} u-boot-${soc}-nor.bin && run ubwrite\0" \
	"ubwrite=sf probe 0; sf erase 0x0 ${kernaddr}; sf write ${baseaddr} 0x0 ${kernaddr}\0" \
	"fwupd=${fetchcmd} ${baseaddr} firmware.${soc} && run fwwrite\0" \
	"fwrecovery=if env exists bootfail; then run fwupd; sleep 5; reset; fi\0" \
	"fwwrite=sf probe 0; sf erase ${kernaddr} ${filesize}; sf write ${baseaddr} ${kernaddr} ${filesize}\0" \
	"nfsroot=/srv/nfs/" __stringify(PRODUCT_SOC) "\0" \
	"bootargsnfs=mem=\\${osmem} console=ttyAMA0,115200 panic=20 root=/dev/nfs rootfstype=nfs ip=${ipaddr}:::255.255.255.0::eth0 nfsroot=${serverip}:${nfsroot},v3,nolock rw \\${extras}\0" \
	"bootnfs=setenv setargs setenv bootargs ${bootargsnfs}; run setargs; tftpboot ${baseaddr} uImage.${soc}; bootm ${baseaddr}\0" \
	"sdload=setenv fetchcmd fatload mmc 0\0" \
	"norload=sf probe 0; sf read\0" \
	"fetchcmd=tftpboot\0" \
	"osmem=" HI_ENV_OSMEM "\0" \
	"mtdids=" MTDIDS "\0" \
	"mtdparts=\\${mtdids}:" MTDPARTS "\0" \
	"bootargs=" CONFIG_BOOTARGS "\0" \
	"board_name\0" \
	"board=" __stringify(PRODUCT_SOC) "\0" \
	"vendor=" VENDOR "\0" \
	"soc=" __stringify(PRODUCT_SOC) "\0" \
	"socmodel=" __stringify(PRODUCT_SOCMODEL)

#else /* CONFIG_FMC_SPI_NAND */

#define MTDIDS "hinand"
#define VENDOR "hisilicon"

#define CONFIG_BOOTARGS "mem=\\${osmem} console=ttyAMA0,115200 panic=20 init=/init root=/dev/ubiblock0_1 ubi.mtd=2,2048 ubi.block=0,1 mtdparts=\\${mtdids}:\\${mtdparts} \\${extras}"
#define CONFIG_BOOTCOMMAND "setenv setargs setenv bootargs ${bootargs}; run setargs; ubi part ubi; ubi read ${baseaddr} kernel; bootm ${baseaddr}; reset"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"baseaddr=" __stringify(HI_ENV_BASEADDR) "\0" \
	"fwupd=${fetchcmd} ${baseaddr} firmware.ubi.${soc} && nand erase 0x100000 0x7f00000; nand write ${baseaddr} 0x100000 ${filesize}\0" \
	"fwrecovery=if env exists bootfail; then run fwupd; sleep 5; reset; fi\0" \
	"nfsroot=/srv/nfs/" __stringify(PRODUCT_SOC) "\0" \
	"bootargsnfs=mem=\\${osmem} console=ttyAMA0,115200 panic=20 root=/dev/nfs rootfstype=nfs ip=${ipaddr}:::255.255.255.0::eth0 nfsroot=${serverip}:${nfsroot},v3,nolock rw \\${extras}\0" \
	"bootnfs=setenv setargs setenv bootargs ${bootargsnfs}; run setargs; tftpboot ${baseaddr} uImage.${soc}; bootm ${baseaddr}\0" \
	"sdload=setenv fetchcmd fatload mmc 0\0" \
	"fetchcmd=tftpboot\0" \
	"osmem=" HI_ENV_OSMEM "\0" \
	"mtdids=" MTDIDS "\0" \
	"mtdparts=\\${mtdids}:768k(boot),256k(env),-(ubi)\0" \
	"bootargs=" CONFIG_BOOTARGS "\0" \
	"board_name\0" \
	"board=" __stringify(PRODUCT_SOC) "\0" \
	"vendor=" VENDOR "\0" \
	"soc=" __stringify(PRODUCT_SOC) "\0" \
	"socmodel=" __stringify(PRODUCT_SOCMODEL)

#define CONFIG_SYS_MALLOC_LEN (32 * SZ_128K)

#endif /* CONFIG_FMC_SPI_NAND */

#undef CONFIG_OSD_ENABLE

#define CONFIG_SYS_MEMTEST_START       CONFIG_SYS_SDRAM_BASE + 0x400
#define CONFIG_SYS_MEMTEST_END         PHYS_SDRAM_1_SIZE - 0x1000000

#if (CONFIG_AUTO_UPDATE == 1)
#define CONFIG_AUTO_UPDATE_ADAPTATION   1
#define CONFIG_AUTO_SD_UPDATE     1

#ifndef CONFIG_MINI_BOOT
#define CONFIG_CMD_FAT          1
#endif
#endif

#define CONFIG_ENV_KERNADDR 0x50000
#define CONFIG_ENV_KERNSIZE 0x200000
#define CONFIG_ENV_ROOTADDR 0x250000
#define CONFIG_ENV_ROOTSIZE 0x500000

#define CONFIG_SD_BOOT 1
#define CONFIG_SPI_BOOT 1
#define CONFIG_BOOTDELAY 1

#define CONFIG_SPI_BLOCK_PROTECT 1
#define CONFIG_NETMASK 255.255.255.0
#define CONFIG_GATEWAYIP 192.168.1.1
#define CONFIG_SERVERIP 192.168.1.254
#define CONFIG_IPADDR 192.168.1.10

#endif /* CONFIG_ARCH_BSP */

#endif /* __HI_COMMON_H */
