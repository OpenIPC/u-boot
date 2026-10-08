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

/* Legacy `gpio status` prints banks and the live pad-mux function */
#ifdef CONFIG_HI_GPIO
#ifndef __ASSEMBLY__
void hi_gpio_status(void);
#endif
#define gpio_status() hi_gpio_status()
#endif

/* All targets boot legacy kernels via ATAGs (no DTB in the boot flow) */
#define CONFIG_SUPPORT_PASSING_ATAGS 1
#define CONFIG_SETUP_MEMORY_TAGS 1
#define CONFIG_CMDLINE_TAG 1
#define CONFIG_INITRD_TAG 1

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

/* Firmware image name for the whole-firmware update (CV610) */
#ifdef CONFIG_ARCH_BSP
#define OPENIPC_FW_FILE "rootfs.${soc}"
#else
#define OPENIPC_FW_FILE "firmware.${soc}"
#endif

#define OPENIPC_BOOTARGSNFS "mem=\\${osmem} console=ttyAMA0,115200 panic=20 root=/dev/nfs rootfstype=nfs ip=${ipaddr}:::255.255.255.0::eth0 nfsroot=${serverip}:${nfsroot},v3,nolock rw \\${extras}"

/* NOR partition list; the reserved 'mtdparts' variable holds the full
 * "<mtd-id>:<list>" string, the bootargs compose it from mtdids/mtds. */
#define OPENIPC_MTDPARTS "256k(boot),64k(env),2048k(kernel),5120k(rootfs),7168k@0x50000(firmware),-(rootfs_data)"

/* NAND partition list; composed the same way. */
#define OPENIPC_NAND_MTDPARTS "768k(boot),256k(env),-(ubi)"

/* Environment shared by all boot media: identity, network boot, update
 * source selection and the medium-agnostic update commands. The selected
 * medium provides ubfile/ubwrite/ukwrite/urwrite/loadkernel/mtdids/
 * mtdparts/bootargs. */
#define OPENIPC_ENV_COMMON \
	"baseaddr=" __stringify(OPENIPC_BASEADDR) "\0" \
	"osmem=" OPENIPC_OSMEM "\0" \
	"board_name\0" \
	"board=" CONFIG_PRODUCT_SOC "\0" \
	"vendor=" VENDOR "\0" \
	"soc=" CONFIG_PRODUCT_SOC "\0" \
	"socmodel=" CONFIG_PRODUCT_SOCMODEL "\0" \
	"nfsroot=/srv/nfs/" CONFIG_PRODUCT_SOC "\0" \
	"bootargsnfs=" OPENIPC_BOOTARGSNFS "\0" \
	"bootnfs=setenv setargs setenv bootargs ${bootargsnfs}; run setargs; ${source} ${baseaddr} uImage.${soc}; bootm ${baseaddr}\0" \
	"source=tftpboot\0" \
	"srcnet=setenv source tftpboot\0" \
	"srcmmc=setenv source fatload mmc 0\0" \
	"srcusb=usb start; setenv source fatload usb 0\0" \
	"ub=${source} ${baseaddr} ${ubfile} && run ubwrite\0" \
	"uk=if ${source} ${baseaddr} uImage.${soc}; then run ukwrite; elif ${source} ${baseaddr} fitImage.${soc}; then run ukwrite; fi\0" \
	"ur=${source} ${baseaddr} ${urfile} && run urwrite\0" \
	OPENIPC_FWUPD \
	"fwrecovery=if env exists bootfail; then run fwupd; sleep 5; reset; fi\0" \
	"bootflash=run fwrecovery; setenv setargs setenv bootargs ${bootargs}; run setargs; run loadkernel; bootm ${baseaddr}; reset\0"

#ifndef CONFIG_FMC_SPI_NAND

/* ---------- SPI NOR ---------- */

#if defined(VENDOR_HISILICON) && !defined(CONFIG_ARCH_BSP)
#define SFC "hi_sfc"
#else
#define SFC "sfc"
#endif

#define CONFIG_BOOTARGS "mem=\\${osmem} console=ttyAMA0,115200 panic=20 root=/dev/mtdblock3 rootfstype=squashfs init=/init mtdparts=\\${mtdparts} \\${extras}"

/* Recovery: CV610 writes a single firmware blob, the others split images;
 * the rootfs is fetched first so its size selects the NOR schema. */
#ifdef CONFIG_TARGET_HI3516CV610_FAMILY
#define OPENIPC_FWUPD \
	"fwupd=${source} ${baseaddr} " OPENIPC_FW_FILE " && run fwwrite\0"
#else
#define OPENIPC_FWUPD "fwupd=run ur; run uk\0"
#endif

#define OPENIPC_ENV_MEDIUM \
	"ubfile=u-boot-" CONFIG_PRODUCT_SOC "-nor.bin\0" \
	"urfile=rootfs.squashfs." CONFIG_PRODUCT_SOC "\0" \
	"kernaddr=" __stringify(CONFIG_ENV_KERNADDR) "\0" \
	"kernsize=" __stringify(CONFIG_ENV_KERNSIZE) "\0" \
	"rootaddr=" __stringify(CONFIG_ENV_ROOTADDR) "\0" \
	"rootsize=" __stringify(CONFIG_ENV_ROOTSIZE) "\0" \
	"mtdslite=setenv kernsize 0x200000; setenv rootaddr 0x250000; setenv rootsize 0x500000\0" \
	"mtdsultimate=setenv kernsize 0x300000; setenv rootaddr 0x350000; setenv rootsize 0xa00000\0" \
	"ubwrite=sf probe 0; sf erase 0x0 ${kernaddr}; sf write ${baseaddr} 0x0 ${filesize}\0" \
	"ukwrite=sf probe 0; sf erase ${kernaddr} ${kernsize}; sf write ${baseaddr} ${kernaddr} ${filesize}\0" \
	"urwrite=sf probe 0; if itest ${filesize} -gt 500000; then run mtdsultimate; fi; sf erase ${rootaddr} ${rootsize}; sf write ${baseaddr} ${rootaddr} ${filesize}\0" \
	"fwwrite=sf probe 0; sf erase ${kernaddr} ${filesize}; sf write ${baseaddr} ${kernaddr} ${filesize}\0" \
	"loadkernel=sf probe 0; sf read ${baseaddr} ${kernaddr} ${kernsize}\0" \
	"mtdids=" SFC "\0" \
	"mtds=" OPENIPC_MTDPARTS "\0" \
	"mtdparts=" SFC ":" OPENIPC_MTDPARTS "\0" \
	"bootargs=" CONFIG_BOOTARGS

#else /* CONFIG_FMC_SPI_NAND */

/* ---------- SPI NAND ---------- */

#if defined(VENDOR_HISILICON) && !defined(CONFIG_ARCH_BSP)
#define SFC "hinand"
#else
#define SFC "nand"
#endif

/* NAND releases may ship a FIT kernel (fitImage.${soc}) */
#define CONFIG_FIT 1

/* Environment lives in the NAND 'env' partition */
#undef CONFIG_ENV_OFFSET
#undef CONFIG_ENV_SIZE
#undef CONFIG_ENV_SECT_SIZE
#define CONFIG_ENV_OFFSET 0xc0000
#define CONFIG_ENV_SIZE 0x40000
#define CONFIG_ENV_SECT_SIZE 0x20000

#define CONFIG_BOOTARGS "mem=\\${osmem} console=ttyAMA0,115200 panic=20 init=/init root=ubi0:rootfs rootfstype=ubifs ubi.mtd=2,2048 mtdparts=\\${mtdparts} \\${extras}"

/* Recovery: recreate the system volumes; the kernel is fixed and the
 * rootfs volume is sized to the image, with rootfs_data taking the
 * rest. Only a rootfs that grew beyond what the old volume could give
 * costs the data volume, as U-Boot cannot resize UBI volumes; else
 * rootfs_data is preserved. */
#define OPENIPC_FWUPD "fwupd=run uk; run ur\0"

#define OPENIPC_ENV_MEDIUM \
	"ubfile=u-boot-" CONFIG_PRODUCT_SOC "-nand.bin\0" \
	"urfile=rootfs.ubifs." CONFIG_PRODUCT_SOC "\0" \
	"ubwrite=nand erase 0x0 0xc0000; nand write ${baseaddr} 0x0 ${filesize}\0" \
	"ukwrite=ubi part ubi; ubi check kernel && ubi remove kernel; ubi create kernel 0x300000 d; ubi write ${baseaddr} kernel ${filesize}\0" \
	"urwrite=ubi part ubi; ubi check rootfs && ubi remove rootfs; if ubi create rootfs ${filesize} d; then ubi check rootfs; else ubi remove rootfs_data; ubi create rootfs ${filesize} d; fi; ubi check rootfs_data || ubi create rootfs_data 0 d; ubi write ${baseaddr} rootfs ${filesize}\0" \
	"loadkernel=ubi part ubi; ubi read ${baseaddr} kernel\0" \
	"mtdids=nand0=" SFC "\0" \
	"mtds=" OPENIPC_NAND_MTDPARTS "\0" \
	"mtdparts=" SFC ":" OPENIPC_NAND_MTDPARTS "\0" \
	"bootargs=" CONFIG_BOOTARGS

#define CONFIG_SYS_MALLOC_LEN (32 * SZ_128K)

#endif /* CONFIG_FMC_SPI_NAND */

#define CONFIG_BOOTCOMMAND "run bootflash"

#define CONFIG_EXTRA_ENV_SETTINGS \
	OPENIPC_ENV_COMMON \
	OPENIPC_ENV_MEDIUM

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
