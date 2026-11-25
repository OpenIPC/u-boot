/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Common OpenIPC configuration for the Axera targets.
 *
 * The single place for the default environment, kept in step with
 * include/configs/openipc-common.h on the openipc-hisilicon and
 * openipc-sigmastar branches: the update, recovery and boot-limit commands
 * are the same, only the per-SoC parameters differ. A SoC's config header
 * defines these, then includes this file last, and does not define
 * CONFIG_EXTRA_ENV_SETTINGS, CONFIG_BOOTARGS or CONFIG_BOOTCOMMAND itself:
 *
 *   OPENIPC_BASEADDR      where kernels and images are loaded
 *   OPENIPC_OSMEM         the kernel's mem=, e.g. "87M"; without it the
 *                         board sets osmem itself, from the DRAM size
 *   OPENIPC_SFC           the flash's MTD name in the kernel (mtdparts=)
 *   SPI NAND (CONFIG_ENV_IS_IN_NAND):
 *     OPENIPC_NAND_MTDPARTS  partition list, ending in "-(ubi)"
 *     OPENIPC_NAND_UBI_MTD   the ubi partition's index, for ubi.mtd=
 *     OPENIPC_NAND_BOOT_SIZE the boot partition's size: ubwrite's limit
 *     OPENIPC_NAND_SPL_SIZE  (optional) the first stage's blocks at the
 *                            boot partition's start, the image carrying
 *                            it before U-Boot (AX620U)
 *   SPI NOR: the layout every OpenIPC NOR camera shares (below); the boot
 *     partition ends at CONFIG_ENV_OFFSET, ubwrite's limit.
 *   eMMC and SD card (CONFIG_ENV_IS_IN_MMC):
 *     OPENIPC_EMMC_DEV, OPENIPC_SD_DEV  their U-Boot mmc numbers, the same
 *                            as the kernel's mmcblk ones
 *     OPENIPC_MMC_PARTS      the eMMC's partition list (blkdevparts=)
 *     OPENIPC_MMC_KERNEL_BLK, OPENIPC_MMC_KERNEL_BLKS, OPENIPC_MMC_KERNEL_SIZE
 *                            its kernel partition, in blocks and bytes
 *     OPENIPC_MMC_ROOT_PART, OPENIPC_MMC_ROOT_BLK
 *                            its root partition: number in the list, block
 *     OPENIPC_KERNEL_ADDR, OPENIPC_FDT_ADDR, OPENIPC_FDTFILE
 *                            the card's Image and device tree (booti)
 */

#ifndef __OPENIPC_COMMON_H
#define __OPENIPC_COMMON_H

#include <linux/stringify.h>

/* Default network configuration */
#define CONFIG_IPADDR		192.168.1.10
#define CONFIG_SERVERIP		192.168.1.107
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_GATEWAYIP	192.168.1.1

#define VENDOR			"axera"

/* NOR layout, shared with every OpenIPC NOR camera */
#define OPENIPC_KERNADDR	0x50000
#define OPENIPC_KERNSIZE	0x200000
#define OPENIPC_ROOTADDR	0x250000
#define OPENIPC_ROOTSIZE	0x500000

#define OPENIPC_BOOTARGSNFS "mem=\\${osmem} console=ttyS0,115200 panic=20 root=/dev/nfs rootfstype=nfs ip=${ipaddr}:::255.255.255.0::eth0 nfsroot=${serverip}:${nfsroot},v3,nolock rw \\${extras}"

/* NOR partition list; the reserved 'mtdparts' variable holds the full
 * "<mtd-id>:<list>" string, the bootargs compose it from mtdids/mtds. */
#define OPENIPC_MTDPARTS "256k(boot),64k(env),2048k(kernel),5120k(rootfs),-(rootfs_data)"

/*
 * Past bootlimit -- a boot loop: the image is intact, but userspace never
 * comes up healthy, so nothing clears the DRAM counter -- boot the same
 * kernel with "failsafe". openipc-failsafe then brings up network and SSH
 * only, so the camera stays reachable. Firmware without that package ignores
 * the word and boots as usual. Firmware the scan found broken (bootfail)
 * goes to recovery first, as bootflash does. A kernel that does not load
 * leaves the camera at this prompt rather than in a reset loop.
 * OPENIPC_ALTBOOTCMD_RETIRED is the reflash this replaced on the
 * openipc-hisilicon branch; openipc_helper() migrates an environment that
 * still carries it.
 */
#define OPENIPC_ALTBOOTCMD "run fwrecovery; echo \"*** Boot limit exceeded -- entering FAILSAFE (network + SSH only) ***\"; setenv setargs setenv bootargs ${bootargs} failsafe; run setargs; if " OPENIPC_LOADKERNEL "; then " OPENIPC_BOOTKERNEL "; fi"
#define OPENIPC_ALTBOOTCMD_RETIRED "echo \"*** Boot limit exceeded -- forcing firmware recovery ***\"; run fwupd"

/*
 * Firmware the scan found broken (bootfail): fetch a new one (fwupd) and
 * reset, retrying until the server has it. A U-Boot with no network, MMC
 * or USB cannot fetch anything, and that reset would loop forever: there it
 * says so and stays at the prompt, where an image loaded over the serial
 * line (loady ${baseaddr}) goes in with run urwrite or run ubwrite. A
 * kernel that loads but never boots still comes to altbootcmd past
 * bootlimit, which does not reset.
 */
#define OPENIPC_RECOVERY_FETCH \
	"fwrecovery=if env exists bootfail; then run fwupd; sleep 5; reset; fi\0" \
	"bootflash=run fwrecovery; setenv setargs setenv bootargs ${bootargs}; run setargs; run loadkernel; bootm ${baseaddr}; reset\0"
#define OPENIPC_RECOVERY_PROMPT \
	"fwrecovery=if env exists bootfail; then echo \"*** Firmware broken, and this U-Boot has no network, MMC or USB to recover it from: staying at the prompt (loady ${baseaddr}, then run urwrite) ***\"; fi\0" \
	"bootflash=if env exists bootfail; then run fwrecovery; else setenv setargs setenv bootargs ${bootargs}; run setargs; run loadkernel; bootm ${baseaddr}; reset; fi\0"

/*
 * What earlier builds of this tree saved for commands since replaced, as
 * "name=value\0" pairs: openipc_helper() puts today's default in place of a
 * saved value that is exactly one of these (an edited one stays), so a
 * board that saved its environment under an older U-Boot gets the fixes.
 * Both recovery variants are listed: a board whose U-Boot gained (or lost)
 * a medium to recover from takes the other one; the one that is today's
 * default is left as it is.
 */
#define OPENIPC_RETIRED_COMMON OPENIPC_RECOVERY_FETCH OPENIPC_RECOVERY_PROMPT

#if defined(CONFIG_ENV_IS_IN_MMC)
#define OPENIPC_RECOVERY OPENIPC_RECOVERY_MMC
#elif defined(CONFIG_CMD_NET) || defined(CONFIG_MMC) || defined(CONFIG_USB_STORAGE)
#define OPENIPC_RECOVERY OPENIPC_RECOVERY_FETCH
#else
#define OPENIPC_RECOVERY OPENIPC_RECOVERY_PROMPT
#endif

/* Environment shared by all boot media: identity, network boot, update
 * source selection and the medium-agnostic update commands. The selected
 * medium provides ubfile/ubwrite/ukwrite/urwrite/loadkernel/mtdids/
 * mtdparts/bootargs. */
/* What boots the loaded kernel (the MMC medium has its own) */
#ifndef OPENIPC_BOOTKERNEL
#define OPENIPC_BOOTKERNEL "bootm ${baseaddr}"
#endif

/* The card recovery and srcmmc read from (mmc 0 unless the board says) */
#ifndef OPENIPC_SD_DEV
#define OPENIPC_SD_DEV 0
#endif

#ifdef OPENIPC_OSMEM
#define OPENIPC_ENV_OSMEM "osmem=" OPENIPC_OSMEM "\0"
#else
#define OPENIPC_ENV_OSMEM ""
#endif

#define OPENIPC_ENV_COMMON \
	"baseaddr=" __stringify(OPENIPC_BASEADDR) "\0" \
	OPENIPC_ENV_OSMEM \
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
	"srcmmc=setenv source fatload mmc " __stringify(OPENIPC_SD_DEV) "\0" \
	"srcusb=usb start; setenv source fatload usb 0\0" \
	"ub=${source} ${baseaddr} ${ubfile} && run ubwrite\0" \
	"uk=if ${source} ${baseaddr} uImage.${soc}; then run ukwrite; elif ${source} ${baseaddr} fitImage.${soc}; then run ukwrite; fi\0" \
	"ur=${source} ${baseaddr} ${urfile} && run urwrite\0" \
	"bootlimit=2\0" \
	"altbootcmd=" OPENIPC_ALTBOOTCMD "\0" \
	OPENIPC_RECOVERY

#if defined(CONFIG_ENV_IS_IN_MMC)

/* ---------- eMMC and SD card ---------- */

/*
 * The firmware boots from the card when it carries one (FAT p1 Image, the
 * firmware's SD image), else from the eMMC; firmware_scan()
 * (common/openipc.c) looks every boot and sets, in RAM, medium (sd or
 * emmc) and rootargs (root=, and the eMMC's blkdevparts=). loadkernel and
 * the boot go through ${medium}load and ${medium}boot: booti of the
 * card's Image and device tree, or bootm of the eMMC's kernel partition
 * (a uImage or FIT).
 */
#define CONFIG_BOOTARGS "mem=\\${osmem} console=ttyS0,115200 panic=20 \\${rootargs} rootfstype=ext4 rw rootwait \\${extras}"

#define OPENIPC_LOADKERNEL "run ${medium}load"
#undef OPENIPC_BOOTKERNEL
#define OPENIPC_BOOTKERNEL "run ${medium}boot"

/*
 * No firmware on either: stay at the prompt. Nothing is fetched and
 * written by itself: until the eMMC has OpenIPC's layout, it holds the
 * board's own firmware. A card's firmware that does not load or boot
 * falls back to the eMMC's when the scan found one there too (emmcargs);
 * what does not boot then leaves U-Boot at the prompt, not in a reset
 * loop. setargs keeps ${rootargs} unexpanded, so it is run again for the
 * eMMC.
 */
#define OPENIPC_RECOVERY_MMC \
	"fwrecovery=if env exists bootfail; then echo \"*** No firmware on the SD card (Image and the device tree on its first partition) or the eMMC: staying at the prompt ***\"; fi\0" \
	"bootflash=if env exists bootfail; then run fwrecovery; else setenv setargs setenv bootargs ${bootargs}; run setargs; run loadkernel && " OPENIPC_BOOTKERNEL "; " \
	"if test \"${medium}\" = sd && env exists emmcargs; then echo \"*** The firmware on the SD card did not boot: trying the eMMC ***\"; setenv medium emmc; setenv rootargs ${emmcargs}; run setargs; run loadkernel && " OPENIPC_BOOTKERNEL "; fi; " \
	"echo \"*** The firmware on ${medium} did not boot: staying at the prompt ***\"; fi\0"

/* A write of ${filesize} bytes from ${baseaddr} to the eMMC block <blk>,
 * at most <max> bytes */
#define OPENIPC_MMC_WRITE(what, blk, max) \
	"if itest ${filesize} -gt 0 && itest ${filesize} -le " __stringify(max) "; then " \
	"setexpr mmcblks ${filesize} + 0x1ff; setexpr mmcblks ${mmcblks} / 0x200; " \
	"mmc dev " __stringify(OPENIPC_EMMC_DEV) " && mmc write ${baseaddr} " __stringify(blk) " ${mmcblks}; setenv mmcblks; " \
	"else echo " what ": ${filesize} bytes do not fit, nothing written; fi"

#define OPENIPC_ENV_MEDIUM \
	"fwupd=run ur; run uk\0" \
	"ubfile=u-boot-" CONFIG_PRODUCT_SOC "-emmc.bin\0" \
	"urfile=rootfs.ext4." CONFIG_PRODUCT_SOC "\0" \
	"ubwrite=echo ubwrite: the eMMC boot partition needs the OpenIPC first stage, not built yet; nothing written\0" \
	"ukwrite=" OPENIPC_MMC_WRITE("ukwrite", OPENIPC_MMC_KERNEL_BLK, OPENIPC_MMC_KERNEL_SIZE) "\0" \
	"urwrite=" OPENIPC_MMC_WRITE("urwrite", OPENIPC_MMC_ROOT_BLK, 0x7fffffff) "\0" \
	"kernel_addr_r=" __stringify(OPENIPC_KERNEL_ADDR) "\0" \
	"fdt_addr_r=" __stringify(OPENIPC_FDT_ADDR) "\0" \
	"fdtfile=" OPENIPC_FDTFILE "\0" \
	"sdload=fatload mmc " __stringify(OPENIPC_SD_DEV) ":1 ${kernel_addr_r} Image && fatload mmc " __stringify(OPENIPC_SD_DEV) ":1 ${fdt_addr_r} ${fdtfile}\0" \
	"sdboot=booti ${kernel_addr_r} - ${fdt_addr_r}\0" \
	"emmcload=mmc dev " __stringify(OPENIPC_EMMC_DEV) " && mmc read ${baseaddr} " __stringify(OPENIPC_MMC_KERNEL_BLK) " " __stringify(OPENIPC_MMC_KERNEL_BLKS) "\0" \
	"emmcboot=bootm ${baseaddr}\0" \
	"loadkernel=" OPENIPC_LOADKERNEL "\0" \
	"mmcparts=mmcblk" __stringify(OPENIPC_EMMC_DEV) ":" OPENIPC_MMC_PARTS "\0" \
	"bootargs=" CONFIG_BOOTARGS

#elif !defined(CONFIG_ENV_IS_IN_NAND)

/* ---------- SPI NOR ---------- */

#define SFC OPENIPC_SFC

#define CONFIG_BOOTARGS "mem=\\${osmem} console=ttyS0,115200 panic=20 root=/dev/mtdblock3 rootfstype=squashfs init=/init mtdparts=\\${mtdparts} \\${extras}"

/*
 * Recovery: firmware.bin, the kernel and the rootfs in one image, when the
 * server has one that fits from kernaddr to the end of the chip (fwsize); else
 * the rootfs, then the kernel, the rootfs first so its size selects the NOR
 * schema.
 */

/* ubwrite before it kept to the boot partition: it erased the env too */
#define OPENIPC_RETIRED_MEDIUM \
	"ubwrite=sf probe 0; sf erase 0x0 ${kernaddr}; sf write ${baseaddr} 0x0 ${filesize}\0"

#define OPENIPC_LOADKERNEL "sf probe 0; sf read ${baseaddr} ${kernaddr} ${kernsize}"
#define OPENIPC_ENV_MEDIUM \
	"ubfile=u-boot-" CONFIG_PRODUCT_SOC "-nor.bin\0" \
	"urfile=rootfs.squashfs." CONFIG_PRODUCT_SOC "\0" \
	"kernaddr=" __stringify(OPENIPC_KERNADDR) "\0" \
	"kernsize=" __stringify(OPENIPC_KERNSIZE) "\0" \
	"rootaddr=" __stringify(OPENIPC_ROOTADDR) "\0" \
	"rootsize=" __stringify(OPENIPC_ROOTSIZE) "\0" \
	"mtdslite=setenv kernaddr " __stringify(OPENIPC_KERNADDR) "; setenv kernsize 0x200000; setenv rootaddr 0x250000; setenv rootsize 0x500000\0" \
	"mtdsultimate=setenv kernaddr " __stringify(OPENIPC_KERNADDR) "; setenv kernsize 0x300000; setenv rootaddr 0x350000; setenv rootsize 0xa00000\0" \
	"ubwrite=if itest ${filesize} -gt 0 && itest ${filesize} -le " __stringify(CONFIG_ENV_OFFSET) "; then sf probe 0 && sf erase 0x0 " __stringify(CONFIG_ENV_OFFSET) " && sf write ${baseaddr} 0x0 ${filesize}; else echo ubwrite: ${filesize} bytes do not fit the boot partition, nothing written; fi\0" \
	"ukwrite=sf probe 0; sf erase ${kernaddr} ${kernsize}; sf write ${baseaddr} ${kernaddr} ${filesize}\0" \
	"urwrite=sf probe 0; run mtdslite; if itest ${filesize} -gt 500000; then run mtdsultimate; fi; sf erase ${rootaddr} ${rootsize}; sf write ${baseaddr} ${rootaddr} ${filesize}\0" \
	"fwfile=firmware.bin." CONFIG_PRODUCT_SOC "\0" \
	"fwsize=0x7b0000\0" \
	"fwwrite=itest ${filesize} -gt 0 && itest ${filesize} -le ${fwsize} && sf probe 0 && sf erase ${kernaddr} ${fwsize} && sf write ${baseaddr} ${kernaddr} ${filesize}\0" \
	"fwupd=if ${source} ${baseaddr} ${fwfile} && run fwwrite; then true; else run ur; run uk; fi\0" \
	"loadkernel=" OPENIPC_LOADKERNEL "\0" \
	"mtdids=" SFC "\0" \
	"mtds=" OPENIPC_MTDPARTS "\0" \
	"mtdparts=" SFC ":" OPENIPC_MTDPARTS "\0" \
	"bootargs=" CONFIG_BOOTARGS

#else /* CONFIG_ENV_IS_IN_NAND */

/* ---------- SPI NAND ---------- */

#define SFC OPENIPC_SFC

#define CONFIG_BOOTARGS "mem=\\${osmem} console=ttyS0,115200 panic=20 init=/init root=ubi0:rootfs rootfstype=ubifs ubi.mtd=" __stringify(OPENIPC_NAND_UBI_MTD) ",2048 mtdparts=\\${mtdparts} \\${extras}"

/*
 * The NAND layout is one UBI device after boot and env, holding the UBIFS
 * rootfs with the kernel inside it -- /boot/fitImage, or /boot/uImage on a
 * build without a FIT -- and rootfs_data on the rest. No flash is set aside
 * for a kernel: the rootfs volume is sized to its image. This is the layout
 * of openipc-hisilicon's NAND images; only the partitions before ubi differ
 * where the vendor's first stage needs them.
 *
 * Recovery rewrites the rootfs volume only, sized to the image, with
 * rootfs_data taking the rest. Only a rootfs that grew beyond what the old
 * volume could give costs the data volume, as U-Boot cannot resize UBI
 * volumes; else rootfs_data is preserved. fwfit (common/openipc.c) checks
 * first that the image fits at all -- the UBI device for urwrite, the boot
 * partition's good blocks for ubwrite -- so an image too big erases and
 * removes nothing.
 */

/*
 * ubwrite: the image goes where it is written from flash 0. Where the
 * first stage comes with U-Boot (OPENIPC_NAND_SPL_SIZE, its blocks at the
 * boot partition's start: AX620U), fwglued (common/openipc.c) checks the
 * image and splits it: U-Boot to the rest of the boot partition, then the
 * first stage to its blocks; an image of U-Boot alone updates U-Boot only.
 * Else the image is boot's.
 */
#ifdef OPENIPC_NAND_SPL_SIZE
#define OPENIPC_NAND_UBWRITE \
	"ubwrite=if fwglued ${baseaddr} ${filesize}; then if nand erase ${ubbootoff} ${ubbootarea} && nand write ${ubbootaddr} ${ubbootoff} ${ubbootsize}; then if itest ${ubsplsize} -gt 0; then nand erase 0 ${ubsplsize} && nand write ${baseaddr} 0 ${ubsplsize}; fi; fi; else echo ubwrite: nothing written; fi; setenv ubbootaddr; setenv ubbootsize; setenv ubbootoff; setenv ubbootarea; setenv ubsplsize\0"
/* ubwrite before it took the first stage too */
#define OPENIPC_RETIRED_UBWRITE \
	"ubwrite=if itest ${filesize} -gt 0 && itest ${filesize} -le " __stringify(OPENIPC_NAND_BOOT_SIZE) " && fwfit boot ${filesize}; then nand erase.part boot && nand write ${baseaddr} boot ${filesize}; else echo ubwrite: ${filesize} bytes do not fit the boot partition, nothing written; fi\0"
#else
#define OPENIPC_NAND_UBWRITE \
	"ubwrite=if itest ${filesize} -gt 0 && itest ${filesize} -le " __stringify(OPENIPC_NAND_BOOT_SIZE) " && fwfit boot ${filesize}; then nand erase.part boot && nand write ${baseaddr} boot ${filesize}; else echo ubwrite: ${filesize} bytes do not fit the boot partition, nothing written; fi\0"
#define OPENIPC_RETIRED_UBWRITE ""
#endif

/* ubwrite and urwrite before fwfit (#135) */
#define OPENIPC_RETIRED_MEDIUM \
	OPENIPC_RETIRED_UBWRITE \
	"ubwrite=if itest ${filesize} -gt 0 && itest ${filesize} -le " __stringify(OPENIPC_NAND_BOOT_SIZE) "; then nand erase.part boot; nand write ${baseaddr} boot ${filesize}; else echo ubwrite: ${filesize} bytes do not fit the boot partition, nothing written; fi\0" \
	"urwrite=ubi part ubi; ubi check rootfs && ubi remove rootfs; if ubi create rootfs ${filesize} d; then ubi check rootfs; else ubi remove rootfs_data; ubi create rootfs ${filesize} d; fi; ubi check rootfs_data || ubi create rootfs_data 0 d; ubi write ${baseaddr} rootfs ${filesize}\0"

#define OPENIPC_LOADKERNEL "ubi part ubi; ubifsmount ubi0:rootfs; ubifsload ${baseaddr} /boot/fitImage || ubifsload ${baseaddr} /boot/uImage"
#define OPENIPC_ENV_MEDIUM \
	"fwupd=run ur\0" \
	"ubfile=u-boot-" CONFIG_PRODUCT_SOC "-nand.bin\0" \
	"urfile=rootfs.ubifs." CONFIG_PRODUCT_SOC "\0" \
	OPENIPC_NAND_UBWRITE \
	"urwrite=if ubi part ubi && fwfit rootfs ${filesize}; then ubi check rootfs && ubi remove rootfs; if ubi create rootfs ${filesize} d; then ubi check rootfs; else ubi remove rootfs_data; ubi create rootfs ${filesize} d; fi; ubi check rootfs_data || ubi create rootfs_data 0 d; if ubi check rootfs; then ubi write ${baseaddr} rootfs ${filesize}; else echo urwrite: no rootfs volume, nothing written; fi; else echo urwrite: ${filesize} bytes do not fit the UBI device, nothing removed or written; fi\0" \
	"loadkernel=" OPENIPC_LOADKERNEL "\0" \
	"mtdids=nand0=" SFC "\0" \
	"mtds=" OPENIPC_NAND_MTDPARTS "\0" \
	"mtdparts=" SFC ":" OPENIPC_NAND_MTDPARTS "\0" \
	"bootargs=" CONFIG_BOOTARGS

#endif /* CONFIG_ENV_IS_IN_MMC, NAND */

#ifndef OPENIPC_RETIRED_MEDIUM
#define OPENIPC_RETIRED_MEDIUM ""
#endif
#define OPENIPC_RETIRED_ENV OPENIPC_RETIRED_COMMON OPENIPC_RETIRED_MEDIUM

#define CONFIG_BOOTCOMMAND "run bootflash"

#define CONFIG_EXTRA_ENV_SETTINGS \
	OPENIPC_ENV_COMMON \
	OPENIPC_ENV_MEDIUM

#endif /* __OPENIPC_COMMON_H */
