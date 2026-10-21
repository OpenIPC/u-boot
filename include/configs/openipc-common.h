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

/*
 * The NOR layout: the boot partition, the environment after it at
 * OPENIPC_ENV_OFFSET, and the firmware after that at OPENIPC_KERNADDR, with
 * the rootfs slots of the fixed lite (2 MiB kernel) and ultimate (3 MiB
 * kernel) layouts. A 256 KiB boot partition unless the SoC's header gives
 * its own -- all of them, as literals: they are spelled into the default
 * environment. The vendor headers' CONFIG_ENV_* are overridden.
 */
#ifndef OPENIPC_ENV_OFFSET
#define OPENIPC_ENV_OFFSET 0x40000
#define OPENIPC_ENV_SIZE 0x10000
#endif
#undef CONFIG_ENV_OFFSET
#undef CONFIG_ENV_SIZE
#undef CONFIG_ENV_SECT_SIZE
#define CONFIG_ENV_OFFSET OPENIPC_ENV_OFFSET
#define CONFIG_ENV_SIZE OPENIPC_ENV_SIZE
#define CONFIG_ENV_SECT_SIZE 0x10000

#ifndef OPENIPC_KERNADDR
#define OPENIPC_KERNADDR 0x50000
#define OPENIPC_ROOTADDR_LITE 0x250000
#define OPENIPC_ROOTADDR_ULTIMATE 0x350000
#endif
#define CONFIG_ENV_KERNADDR OPENIPC_KERNADDR
#define CONFIG_ENV_KERNSIZE 0x200000
#define CONFIG_ENV_ROOTADDR OPENIPC_ROOTADDR_LITE
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
#ifndef OPENIPC_OSMEM
#define OPENIPC_OSMEM "32M"
#endif
#else /* XMedia / Goke */
#undef CONFIG_SYS_LOAD_ADDR
#define CONFIG_SYS_LOAD_ADDR 0x42000000
#define OPENIPC_OSMEM "32M"
#endif

#ifndef OPENIPC_BASEADDR
#define OPENIPC_BASEADDR CONFIG_SYS_LOAD_ADDR
#endif

#undef CONFIG_SYS_BOOTCOUNT_ADDR
#define CONFIG_SYS_BOOTCOUNT_ADDR (CONFIG_SYS_SDRAM_BASE + 0x1f20000)

/*
 * The whole-firmware image of the split NOR layout, as the firmware build
 * names it: firmware.bin.<its SoC model>. That is the SoC, unless the
 * firmware builds one image for a family (hi3516cv6xx); the header of such
 * a SoC names the family.
 */
#ifndef OPENIPC_FW_SOC
#define OPENIPC_FW_SOC CONFIG_PRODUCT_SOC
#endif
#define OPENIPC_FW_FILE "firmware.bin." OPENIPC_FW_SOC

#define OPENIPC_BOOTARGSNFS "mem=\\${osmem} console=ttyAMA0,115200 panic=20 root=/dev/nfs rootfstype=nfs ip=${ipaddr}:::255.255.255.0::eth0 nfsroot=${serverip}:${nfsroot},v3,nolock rw \\${extras}"

/* NOR partition list; the reserved 'mtdparts' variable holds the full
 * "<mtd-id>:<list>" string, the bootargs compose it from mtdids/mtds. */
#ifndef OPENIPC_MTDPARTS
#define OPENIPC_MTDPARTS "256k(boot),64k(env),2048k(kernel),5120k(rootfs),7168k@0x50000(firmware),-(rootfs_data)"
#endif

/* NAND partition list; composed the same way. */
#define OPENIPC_NAND_MTDPARTS "768k(boot),256k(env),-(ubi)"

/*
 * Past bootlimit -- a boot loop: the image is intact, but userspace never
 * comes up healthy, so nothing clears the DRAM counter -- boot the same
 * kernel with "failsafe". openipc-failsafe then brings up network and SSH
 * only, so the camera stays reachable. Firmware without that package ignores
 * the word and boots as usual. Firmware the scan found broken (bootfail)
 * goes to recovery first, as bootflash does. A kernel that does not load
 * leaves the camera at this prompt rather than in a reset loop. The kernel
 * is loaded inline (OPENIPC_LOADKERNEL), not through the loadkernel
 * variable: an environment saved by an older OpenIPC U-Boot has no such
 * variable, and the scan sets kernaddr/kernsize on every NOR boot anyway. A reflash
 * here instead (OPENIPC_ALTBOOTCMD_RETIRED) needed a TFTP server or SD
 * card the field does not have, and a failed one parked the camera at the
 * prompt with no kernel; openipc_helper() replaces that saved command.
 */
#define OPENIPC_ALTBOOTCMD "run fwrecovery; echo \"*** Boot limit exceeded -- entering FAILSAFE (network + SSH only) ***\"; setenv setargs setenv bootargs ${bootargs} failsafe; run setargs; if " OPENIPC_LOADKERNEL "; then bootm ${baseaddr}; fi"
#define OPENIPC_ALTBOOTCMD_RETIRED "echo \"*** Boot limit exceeded -- forcing firmware recovery ***\"; run fwupd"

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
	"bootlimit=2\0" \
	"altbootcmd=" OPENIPC_ALTBOOTCMD "\0" \
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

/*
 * Recovery. A board that boots the split layout takes the whole firmware
 * image, written where the firmware starts; without one on the server, or
 * on a fixed-slot board, the rootfs and the kernel, the rootfs first so its
 * size selects the NOR schema.
 */
#ifdef CONFIG_OPENIPC_NOR_SPLIT
/* the firmware file when the server has one, else the fixed-slot pair */
#define OPENIPC_FWUPD_SPLIT_CMD \
	"if ${source} ${baseaddr} ${fwfile}; then run fwwrite; else run ur; run uk; fi"
/* fwupd as a saved environment from before the split layout spells it */
#define OPENIPC_FWUPD_LEGACY_CMD "run ur; run uk"
/*
 * The whole firmware area (the scan sets fwsize to the end of the chip), and
 * only for a file that fits it: an oversized download must not erase the
 * firmware it cannot replace.
 */
#define OPENIPC_FWWRITE_SPLIT_CMD \
	"sf probe 0; if itest ${filesize} -gt 0; then if itest ${filesize} -le ${fwsize}; then " \
	"sf erase ${kernaddr} ${fwsize}; sf write ${baseaddr} ${kernaddr} ${filesize}; " \
	"else echo fwwrite: ${filesize} bytes do not fit ${fwsize} from ${kernaddr}, nothing written; fi; " \
	"else echo fwwrite: empty file, nothing written; fi"
#define OPENIPC_FWUPD \
	"fwfile=" OPENIPC_FW_FILE "\0" \
	"fwupd=" OPENIPC_FWUPD_SPLIT_CMD "\0"
#define OPENIPC_FWWRITE \
	"fwwrite=" OPENIPC_FWWRITE_SPLIT_CMD "\0"
#else
#define OPENIPC_FWUPD "fwupd=run ur; run uk\0"
#define OPENIPC_FWWRITE \
	"fwwrite=sf probe 0; sf erase ${kernaddr} +${filesize}; sf write ${baseaddr} ${kernaddr} ${filesize}\0"
#endif

#define OPENIPC_LOADKERNEL "sf probe 0; sf read ${baseaddr} ${kernaddr} ${kernsize}"
#define OPENIPC_ENV_MEDIUM \
	"ubfile=u-boot-" CONFIG_PRODUCT_SOC "-nor.bin\0" \
	"urfile=rootfs.squashfs." CONFIG_PRODUCT_SOC "\0" \
	"kernaddr=" __stringify(CONFIG_ENV_KERNADDR) "\0" \
	"kernsize=" __stringify(CONFIG_ENV_KERNSIZE) "\0" \
	"rootaddr=" __stringify(CONFIG_ENV_ROOTADDR) "\0" \
	"rootsize=" __stringify(CONFIG_ENV_ROOTSIZE) "\0" \
	"mtdslite=setenv kernsize 0x200000; setenv rootaddr " __stringify(OPENIPC_ROOTADDR_LITE) "; setenv rootsize 0x500000\0" \
	"mtdsultimate=setenv kernsize 0x300000; setenv rootaddr " __stringify(OPENIPC_ROOTADDR_ULTIMATE) "; setenv rootsize 0xa00000\0" \
	"ubwrite=sf probe 0; sf erase 0x0 ${kernaddr}; sf write ${baseaddr} 0x0 ${filesize}\0" \
	"ukwrite=sf probe 0; sf erase ${kernaddr} ${kernsize}; sf write ${baseaddr} ${kernaddr} ${filesize}\0" \
	"urwrite=sf probe 0; if itest ${filesize} -gt 500000; then run mtdsultimate; fi; sf erase ${rootaddr} ${rootsize}; sf write ${baseaddr} ${rootaddr} ${filesize}\0" \
	OPENIPC_FWWRITE \
	"loadkernel=" OPENIPC_LOADKERNEL "\0" \
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

/* Environment lives in the NAND 'env' partition */
#undef CONFIG_ENV_OFFSET
#undef CONFIG_ENV_SIZE
#undef CONFIG_ENV_SECT_SIZE
#define CONFIG_ENV_OFFSET 0xc0000
#define CONFIG_ENV_SIZE 0x40000
#define CONFIG_ENV_SECT_SIZE 0x20000

#define CONFIG_BOOTARGS "mem=\\${osmem} console=ttyAMA0,115200 panic=20 init=/init root=ubi0:rootfs rootfstype=ubifs ubi.mtd=2,2048 mtdparts=\\${mtdparts} \\${extras}"

/*
 * The NAND layout is one UBI device after boot and env, holding the UBIFS
 * rootfs with the kernel inside it -- /boot/fitImage, or /boot/uImage on a
 * build without a FIT -- and rootfs_data on the rest. No flash is set aside
 * for a kernel: the rootfs volume is sized to its image. This is the layout
 * the firmware's NAND packages (rootfs.ubi, rootfs.ubifs) and openipc.org's
 * installs write.
 *
 * Recovery rewrites the rootfs volume only, sized to the image, with
 * rootfs_data taking the rest. Only a rootfs that grew beyond what the old
 * volume could give costs the data volume, as U-Boot cannot resize UBI
 * volumes; else rootfs_data is preserved.
 */
#define OPENIPC_FWUPD "fwupd=run ur\0"
/* bootm_size keeps the relocated FDT inside the kernel's lowmem
 * (mem=${osmem}): bootm otherwise puts it at the top of RAM, which the
 * kernel no longer maps once paging_init() honours mem=, and it dies
 * silently in unflatten_device_tree(). */

#define OPENIPC_LOADKERNEL "ubi part ubi; ubifsmount ubi0:rootfs; ubifsload ${baseaddr} /boot/fitImage || ubifsload ${baseaddr} /boot/uImage"
#define OPENIPC_ENV_MEDIUM \
	"ubfile=u-boot-" CONFIG_PRODUCT_SOC "-nand.bin\0" \
	"urfile=rootfs.ubifs." CONFIG_PRODUCT_SOC "\0" \
	"ubwrite=nand erase 0x0 0xc0000; nand write ${baseaddr} 0x0 ${filesize}\0" \
	"urwrite=ubi part ubi; ubi check rootfs && ubi remove rootfs; if ubi create rootfs ${filesize} d; then ubi check rootfs; else ubi remove rootfs_data; ubi create rootfs ${filesize} d; fi; ubi check rootfs_data || ubi create rootfs_data 0 d; ubi write ${baseaddr} rootfs ${filesize}\0" \
	"loadkernel=" OPENIPC_LOADKERNEL "\0" \
	"bootm_size=0x2000000\0" \
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
