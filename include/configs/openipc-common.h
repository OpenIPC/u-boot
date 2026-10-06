/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Common OpenIPC configuration for all targets.
 *
 * This is the single place for the default environment. Per-SoC config
 * headers may override the OPENIPC_* parameter macros below before
 * including this file, but must not define CONFIG_EXTRA_ENV_SETTINGS,
 * CONFIG_BOOTARGS or CONFIG_BOOTCOMMAND themselves.
 */

#ifndef __OPENIPC_COMMON_H
#define __OPENIPC_COMMON_H

/* Default network configuration */
#define CONFIG_IPADDR       192.168.1.10
#define CONFIG_SERVERIP     192.168.1.107
#define CONFIG_NETMASK      255.255.255.0
#define CONFIG_GATEWAYIP    192.168.1.1

/* Console */
#undef CONFIG_SYS_PROMPT
#define CONFIG_SYS_PROMPT "OpenIPC # "

#ifndef CONFIG_SYS_NAND_MAX_CHIPS
#define CONFIG_SYS_NAND_MAX_CHIPS 1
#endif

/* Vendor naming */
#ifdef VENDOR_HISILICON
#define VENDOR "hisilicon"
#else
#define VENDOR "goke"
#endif

#define CONFIG_ENV_OFFSET 0x40000
#define CONFIG_ENV_SIZE 0x10000
#define CONFIG_ENV_SECT_SIZE 0x10000

/* Kernel/rootfs offsets (shared by all NOR layouts) */
#define CONFIG_ENV_KERNADDR 0x50000
#define CONFIG_ENV_KERNSIZE 0x200000
#define CONFIG_ENV_ROOTADDR 0x250000
#define CONFIG_ENV_ROOTSIZE 0x500000

/* gzip image header */
#define GZIP_HEAD_SIZE   0x10
#define HEAD_MAGIC_NUM0 0x70697a67 /* 'g''z''i''p' */
#define HEAD_MAGIC_NUM0_OFFSET 0x8
#define HEAD_MAGIC_NUM1 0x64616568 /* 'h''e''a''d' */
#define HEAD_MAGIC_NUM1_OFFSET 0xc
#define COMPRESSED_SIZE_OFFSET      0x0
#define UNCOMPRESSED_SIZE_OFFSET    0x4

/* Per-family defaults */
#if defined(CONFIG_TARGET_HI3516CV200_FAMILY) || \
	defined(CONFIG_TARGET_HI3516CV300_FAMILY) || \
	defined(CONFIG_TARGET_HI3518EV100_FAMILY) || \
	defined(CONFIG_TARGET_HI3516AV100_FAMILY) || \
	defined(CONFIG_TARGET_HI3536DV100_FAMILY) || \
	defined(CONFIG_TARGET_HI3536CV100_FAMILY) || \
	defined(CONFIG_TARGET_HI3520DV100_FAMILY) || \
	defined(CONFIG_TARGET_HI3519V101_FAMILY)
#define OPENIPC_BASEADDR 0x82000000
#define OPENIPC_OSMEM "32M"
#elif defined(CONFIG_HI35XX_FAMILY_V500)
#define OPENIPC_BASEADDR 0x82000000
#define OPENIPC_OSMEM "128M"
#elif defined(CONFIG_ARCH_BSP)
#define OPENIPC_OSMEM "32M"
#else /* XMedia / Goke */
#undef CONFIG_SYS_LOAD_ADDR
#define CONFIG_SYS_LOAD_ADDR 0x42000000
#define OPENIPC_OSMEM "32M"
#endif

#ifndef OPENIPC_BASEADDR
#define OPENIPC_BASEADDR CONFIG_SYS_LOAD_ADDR
#endif

/* Firmware image name for the recovery update */
#ifdef CONFIG_ARCH_BSP
#define OPENIPC_FW_FILE "rootfs.${soc}"
#else
#define OPENIPC_FW_FILE "firmware.${soc}"
#endif

#define OPENIPC_BOOTARGSNFS "mem=\\${osmem} console=ttyAMA0,115200 panic=20 root=/dev/nfs rootfstype=nfs ip=${ipaddr}:::255.255.255.0::eth0 nfsroot=${serverip}:${nfsroot},v3,nolock rw \\${extras}"

/* NOR partition list; the runtime mtdparts value is this list, the bootargs
 * compose it as ${mtdids}:${mtdparts} (same as CV610). */
#define OPENIPC_MTDPARTS "256k(boot),64k(env),2048k(kernel),5120k(rootfs),7168k@0x50000(firmware),-(rootfs_data)"

#ifndef CONFIG_FMC_SPI_NAND

/* ---------- SPI NOR ---------- */

#if defined(VENDOR_HISILICON) && !defined(CONFIG_ARCH_BSP)
#define SFC "hi_sfc"
#else
#define SFC "sfc"
#endif

#define CONFIG_BOOTARGS "mem=\\${osmem} console=ttyAMA0,115200 panic=20 root=/dev/mtdblock3 rootfstype=squashfs init=/init mtdparts=\\${mtdids}:\\${mtdparts} \\${extras}"

#define CONFIG_BOOTCOMMAND "run bootnor"

#ifdef CONFIG_TARGET_HI3516CV610_FAMILY
#define OPENIPC_FWUPD \
	"fwupd=${fetchcmd} ${baseaddr} " OPENIPC_FW_FILE " && run fwwrite\0"
#else
#define OPENIPC_FWUPD "fwupd=run uknor; run urnor\0"
#endif

#define CONFIG_EXTRA_ENV_SETTINGS \
	"baseaddr=" __stringify(OPENIPC_BASEADDR) "\0" \
	"kernaddr=" __stringify(CONFIG_ENV_KERNADDR) "\0" \
	"kernsize=" __stringify(CONFIG_ENV_KERNSIZE) "\0" \
	"rootaddr=" __stringify(CONFIG_ENV_ROOTADDR) "\0" \
	"rootsize=" __stringify(CONFIG_ENV_ROOTSIZE) "\0" \
	"bootnor=run fwrecovery; sf probe 0; setenv setargs setenv bootargs ${bootargs}; run setargs; sf read ${baseaddr} ${kernaddr} ${kernsize}; bootm ${baseaddr}\0" \
	OPENIPC_FWUPD \
	"fwrecovery=if env exists bootfail; then run fwupd; sleep 5; reset; fi\0" \
	"fwwrite=sf probe 0; sf erase ${kernaddr} ${filesize}; sf write ${baseaddr} ${kernaddr} ${filesize}\0" \
	"ubnor=${updatetool} ${baseaddr} u-boot-${soc}-nor.bin && run ubwrite\0" \
	"uknor=${updatetool} ${baseaddr} uImage.${soc} && run ukwrite\0" \
	"urnor=${updatetool} ${baseaddr} rootfs.squashfs.${soc} && run urwrite\0" \
	"ubwrite=sf probe 0; sf erase 0x0 ${kernaddr}; sf write ${baseaddr} 0x0 ${kernaddr}\0" \
	"ukwrite=sf probe 0; sf erase ${kernaddr} ${kernsize}; sf write ${baseaddr} ${kernaddr} ${filesize}\0" \
	"urwrite=sf probe 0; sf erase ${rootaddr} ${rootsize}; sf write ${baseaddr} ${rootaddr} ${filesize}\0" \
	"nfsroot=/srv/nfs/" CONFIG_PRODUCT_SOC "\0" \
	"bootargsnfs=" OPENIPC_BOOTARGSNFS "\0" \
	"bootnfs=setenv setargs setenv bootargs ${bootargsnfs}; run setargs; tftpboot ${baseaddr} uImage.${soc}; bootm ${baseaddr}\0" \
	"sdload=setenv fetchcmd fatload mmc 0\0" \
	"norload=sf probe 0; sf read\0" \
	"fetchcmd=tftpboot\0" \
	"sdcard=setenv updatetool fatload mmc 0\0" \
	"updatetool=tftpboot\0" \
	"osmem=" OPENIPC_OSMEM "\0" \
	"mtdids=" SFC "\0" \
	"mtdparts=" OPENIPC_MTDPARTS "\0" \
	"bootargs=" CONFIG_BOOTARGS "\0" \
	"board_name\0" \
	"board=" CONFIG_PRODUCT_SOC "\0" \
	"vendor=" VENDOR "\0" \
	"soc=" CONFIG_PRODUCT_SOC "\0" \
	"socmodel=" CONFIG_PRODUCT_SOCMODEL

#else /* CONFIG_FMC_SPI_NAND */

/* ---------- SPI NAND ---------- */

#if defined(VENDOR_HISILICON) && !defined(CONFIG_ARCH_BSP)
#define SFC "hinand"
#define OPENIPC_NAND_MTDPARTS "\\${mtdids}:768k(boot),256k(env),-(ubi)"
#else
#define SFC "nand"
#define OPENIPC_NAND_MTDPARTS "mtdparts=" SFC ":768k(boot),256k(env),-(ubi)"
#ifndef VENDOR_HISILICON
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET 0xc0000
#define CONFIG_ENV_SIZE 0x40000
#define CONFIG_ENV_SECT_SIZE 0x20000
#endif
#endif

#ifdef VENDOR_HISILICON
#define CONFIG_BOOTARGS "mem=\\${osmem} console=ttyAMA0,115200 panic=20 init=/init root=/dev/ubiblock0_1 ubi.mtd=2,2048 ubi.block=0,1 mtdparts=\\${mtdids}:\\${mtdparts} \\${extras}"
#else
#define CONFIG_BOOTARGS "mem=\\${osmem} console=ttyAMA0,115200 panic=20 init=/init root=/dev/ubiblock0_1 ubi.mtd=2,2048 ubi.block=0,1 \\${mtdparts} \\${extras}"
#endif

#define CONFIG_BOOTCOMMAND "setenv setargs setenv bootargs ${bootargs}; run setargs; ubi part ubi; ubi read ${baseaddr} kernel; bootm ${baseaddr}; reset"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"baseaddr=" __stringify(OPENIPC_BASEADDR) "\0" \
	"fwupd=${fetchcmd} ${baseaddr} firmware.ubi.${soc} && nand erase 0x100000 0x7f00000; nand write ${baseaddr} 0x100000 ${filesize}\0" \
	"fwrecovery=if env exists bootfail; then run fwupd; sleep 5; reset; fi\0" \
	"urnand=${updatetool} ${baseaddr} rootfs.ubi.${soc} && nand erase 0x100000 0x7f00000; nand write ${baseaddr} 0x100000 ${filesize}\0" \
	"sdload=setenv fetchcmd fatload mmc 0\0" \
	"fetchcmd=tftpboot\0" \
	"sdcard=setenv updatetool fatload mmc 0\0" \
	"updatetool=tftpboot\0" \
	"nfsroot=/srv/nfs/" CONFIG_PRODUCT_SOC "\0" \
	"bootargsnfs=" OPENIPC_BOOTARGSNFS "\0" \
	"bootargs=" CONFIG_BOOTARGS "\0" \
	"bootnfs=setenv setargs setenv bootargs ${bootargsnfs}; run setargs; tftpboot ${baseaddr} uImage.${soc}; bootm ${baseaddr}\0" \
	"osmem=" OPENIPC_OSMEM "\0" \
	"mtdids=nand0=" SFC "\0" \
	"mtdparts=" OPENIPC_NAND_MTDPARTS "\0" \
	"board_name\0" \
	"board=" CONFIG_PRODUCT_SOC "\0" \
	"vendor=" VENDOR "\0" \
	"soc=" CONFIG_PRODUCT_SOC "\0" \
	"socmodel=" CONFIG_PRODUCT_SOCMODEL

#define CONFIG_SYS_MALLOC_LEN (32 * SZ_128K)

#endif /* CONFIG_FMC_SPI_NAND */

/* Common non-environment settings */
#if defined(CONFIG_ARCH_BSP)
#define CONFIG_SYS_INIT_RAM_ADDR 0x41700000
#define CONFIG_SYS_INIT_RAM_SIZE 0x4000
#elif !defined(CONFIG_HI35XX_FAMILY_V500) && \
	!defined(CONFIG_TARGET_HI3516CV200_FAMILY) && \
	!defined(CONFIG_TARGET_HI3516CV300_FAMILY) && \
	!defined(CONFIG_TARGET_HI3518EV100_FAMILY) && \
	!defined(CONFIG_TARGET_HI3516AV100_FAMILY) && \
	!defined(CONFIG_TARGET_HI3536DV100_FAMILY) && \
	!defined(CONFIG_TARGET_HI3536CV100_FAMILY) && \
	!defined(CONFIG_TARGET_HI3520DV100_FAMILY) && \
	!defined(CONFIG_TARGET_HI3519V101_FAMILY)
#define CONFIG_SYS_INIT_RAM_ADDR 0x04000000
#define CONFIG_SYS_INIT_RAM_SIZE 0x14000
#endif

#ifdef CONFIG_HI35XX_FAMILY_V500
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

#define CONFIG_SD_BOOT 1
#define CONFIG_SPI_BOOT 1
#endif

#if defined(CONFIG_HI35XX_FAMILY_V500) || \
	defined(CONFIG_TARGET_HI3516CV200_FAMILY) || \
	defined(CONFIG_TARGET_HI3516CV300_FAMILY) || \
	defined(CONFIG_TARGET_HI3518EV100_FAMILY) || \
	defined(CONFIG_TARGET_HI3516AV100_FAMILY) || \
	defined(CONFIG_TARGET_HI3536DV100_FAMILY) || \
	defined(CONFIG_TARGET_HI3536CV100_FAMILY) || \
	defined(CONFIG_TARGET_HI3520DV100_FAMILY) || \
	defined(CONFIG_TARGET_HI3519V101_FAMILY)
#define CONFIG_BOOTDELAY 1
#endif

#endif /* __OPENIPC_COMMON_H */
