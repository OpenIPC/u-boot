/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Common OpenIPC configuration for the Ingenic T-series targets.
 *
 * The single place for the default environment, kept in step with
 * include/configs/openipc-common.h on the openipc-hisilicon branch: the
 * update, recovery and boot-limit commands are the same, only the per-SoC
 * parameters below differ. Per-SoC config headers include this last and must
 * not define CONFIG_EXTRA_ENV_SETTINGS, CONFIG_BOOTARGS or
 * CONFIG_BOOTCOMMAND themselves.
 */

#ifndef __OPENIPC_COMMON_H
#define __OPENIPC_COMMON_H

#include <linux/stringify.h>

/* Default network configuration */
#define CONFIG_IPADDR		192.168.1.10
#define CONFIG_SERVERIP		192.168.1.107
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_GATEWAYIP	192.168.1.1

#define VENDOR			"ingenic"

/*
 * Kernel memory (mem=) from the start of the DDR, and above it the media
 * drivers' reserved memory (rmem=), as u-boot-ingenic set them up; on the
 * T40 also the NPU's (nmem), as u-boot-t40 set them up, which left the
 * T40A at the 64 MiB defaults; on the T41 as the T41 SDK's bootargs have
 * them, but for the T41A: the SDK gives it the 256 MiB parts' split, which
 * reaches past its 128 MiB (W631GU6NG, as the T41N), so it has the T41N's.
 */
#if defined(CONFIG_T41L) || defined(CONFIG_T41LQ)
#define OPENIPC_OSMEM		"32M"
#define OPENIPC_RMEM		"32M@0x2000000"
#elif defined(CONFIG_T41N) || defined(CONFIG_T41NQ) || defined(CONFIG_T41A)
#define OPENIPC_OSMEM		"48M"
#define OPENIPC_RMEM		"64M@0x3000000"
#define OPENIPC_NMEM		"16M@0x7000000"
#elif defined(CONFIG_T41ZX) || defined(CONFIG_T41XQ)
#define OPENIPC_OSMEM		"100M"
#define OPENIPC_RMEM		"128M@0x6400000"
#define OPENIPC_NMEM		"28M@0xE400000"
#elif defined(CONFIG_T40N)
#define OPENIPC_OSMEM		"48M"
#define OPENIPC_RMEM		"64M@0x3000000"
#define OPENIPC_NMEM		"16M@0x7000000"
#elif defined(CONFIG_T40XP)
#define OPENIPC_OSMEM		"100M"
#define OPENIPC_RMEM		"128M@0x6400000"
#define OPENIPC_NMEM		"28M@0xE400000"
#elif PHYS_SDRAM_1_SIZE == 0x08000000
#define OPENIPC_OSMEM		"64M"
#define OPENIPC_RMEM		"64M@0x4000000"
#else
#define OPENIPC_OSMEM		"40M"
#define OPENIPC_RMEM		"24M@0x2800000"
#endif

#ifdef OPENIPC_NMEM
#define OPENIPC_ENV_NMEM	"nmem=" OPENIPC_NMEM "\0"
#else
#define OPENIPC_ENV_NMEM
#endif

#define OPENIPC_BASEADDR	0x80600000

/* NOR layout, shared with every OpenIPC NOR camera */
#define OPENIPC_KERNADDR	0x50000
#define OPENIPC_KERNSIZE	0x200000
#define OPENIPC_ROOTADDR	0x250000
#define OPENIPC_ROOTSIZE	0x500000

/* the split package's image, firmware.bin.<soc>; spelled out here, as
 * urfile is: U-Boot does not expand a variable inside another's value */
#define OPENIPC_FW_FILE		"firmware.bin." CONFIG_PRODUCT_SOC

#define OPENIPC_BOOTARGSNFS "mem=\\${osmem} rmem=\\${rmem} console=ttyS1,115200 panic=20 root=/dev/nfs rootfstype=nfs ip=${ipaddr}:::255.255.255.0::eth0 nfsroot=${serverip}:${nfsroot},v3,nolock rw \\${extras}"

/* NOR partition list; the reserved 'mtdparts' variable holds the full
 * "<mtd-id>:<list>" string, the bootargs compose it from mtdids/mtds. */
#define OPENIPC_MTDPARTS "256k(boot),64k(env),2048k(kernel),5120k(rootfs),7168k@0x50000(firmware),-(rootfs_data)"

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
#define OPENIPC_ALTBOOTCMD "run fwrecovery; echo \"*** Boot limit exceeded -- entering FAILSAFE (network + SSH only) ***\"; setenv setargs setenv bootargs ${bootargs} failsafe; run setargs; if " OPENIPC_LOADKERNEL "; then bootm ${baseaddr}; fi"
#define OPENIPC_ALTBOOTCMD_RETIRED "echo \"*** Boot limit exceeded -- forcing firmware recovery ***\"; run fwupd"

/* Environment shared by all boot media: identity, network boot, update
 * source selection and the medium-agnostic update commands. The selected
 * medium provides ubfile/ubwrite/ukwrite/urwrite/loadkernel/mtdids/
 * mtdparts/bootargs. */
#define OPENIPC_ENV_COMMON \
	"baseaddr=" __stringify(OPENIPC_BASEADDR) "\0" \
	"osmem=" OPENIPC_OSMEM "\0" \
	"rmem=" OPENIPC_RMEM "\0" \
	OPENIPC_ENV_NMEM \
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

#ifndef CONFIG_JZ_SFC_NAND

/* ---------- SPI NOR ---------- */

/* The flash's MTD name in the vendor kernel (drivers/mtd/devices/jz_sfc) */
#ifdef CONFIG_T41
/* the T41 kernels' (drivers/mtd/devices/jz_sfc_v1, flash on SFC0) */
#define SFC "sfc0_nor"
#else
#define SFC "jz_sfc"
#endif

#define CONFIG_BOOTARGS "mem=\\${osmem} rmem=\\${rmem} console=ttyS1,115200 panic=20 root=/dev/mtdblock3 rootfstype=squashfs init=/init mtdparts=\\${mtdparts} \\${extras}"

/*
 * Recovery: the rootfs is fetched first so its size selects the NOR schema.
 * It starts from the lite layout, not from what the scan found: on a camera
 * that still holds its vendor firmware the scan reports the vendor's
 * offsets (rootfs at 0x1f0000 on a GK7102C), and writing OpenIPC there
 * left the kernel write erasing the start of the new rootfs.
 */
#ifdef CONFIG_OPENIPC_NOR_SPLIT
/*
 * The split layout: the whole firmware file, written
 * where the firmware starts, when the server has one that fits the firmware
 * area; else -- none, an empty one, or one too big for the chip -- the
 * fixed-slot pair as before. common/openipc.c turns a saved fwupd of exactly the
 * legacy form into this one.
 */
#define OPENIPC_FWUPD_SPLIT_CMD \
	"if ${source} ${baseaddr} ${fwfile} && itest ${filesize} -gt 0 && itest ${filesize} -le ${fwsize}; " \
	"then run fwwrite; else run mtdslite; run ur; run uk; fi"
#define OPENIPC_FWUPD_LEGACY_CMD "run mtdslite; run ur; run uk"
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
#define OPENIPC_FWWRITE "fwwrite=" OPENIPC_FWWRITE_SPLIT_CMD "\0"
#else
#define OPENIPC_FWUPD "fwupd=run mtdslite; run ur; run uk\0"
#define OPENIPC_FWWRITE \
	"fwwrite=sf probe 0; sf erase ${kernaddr} ${filesize}; sf write ${baseaddr} ${kernaddr} ${filesize}\0"
#endif

#define OPENIPC_LOADKERNEL "sf probe 0; sf read ${baseaddr} ${kernaddr} ${kernsize}"
#define OPENIPC_ENV_MEDIUM \
	"ubfile=u-boot-" CONFIG_PRODUCT_SOC "-nor.bin\0" \
	"urfile=rootfs.squashfs." CONFIG_PRODUCT_SOC "\0" \
	"kernaddr=" __stringify(OPENIPC_KERNADDR) "\0" \
	"kernsize=" __stringify(OPENIPC_KERNSIZE) "\0" \
	"rootaddr=" __stringify(OPENIPC_ROOTADDR) "\0" \
	"rootsize=" __stringify(OPENIPC_ROOTSIZE) "\0" \
	"mtdslite=setenv kernaddr " __stringify(OPENIPC_KERNADDR) "; setenv kernsize 0x200000; setenv rootaddr 0x250000; setenv rootsize 0x500000\0" \
	"mtdsultimate=setenv kernsize 0x300000; setenv rootaddr 0x350000; setenv rootsize 0xa00000\0" \
	"ubwrite=sf probe 0; sf erase 0x0 ${kernaddr}; sf write ${baseaddr} 0x0 ${filesize}\0" \
	"ukwrite=sf probe 0; sf erase ${kernaddr} ${kernsize}; sf write ${baseaddr} ${kernaddr} ${filesize}\0" \
	"urwrite=sf probe 0; if itest ${filesize} -gt 500000; then run mtdsultimate; fi; sf erase ${rootaddr} ${rootsize}; sf write ${baseaddr} ${rootaddr} ${filesize}\0" \
	OPENIPC_FWWRITE \
	"loadkernel=" OPENIPC_LOADKERNEL "\0" \
	"mtdids=" SFC "\0" \
	"mtds=" OPENIPC_MTDPARTS "\0" \
	"mtdparts=" SFC ":" OPENIPC_MTDPARTS "\0" \
	"bootargs=" CONFIG_BOOTARGS

#else /* CONFIG_JZ_SFC_NAND */

/* ---------- SPI NAND ---------- */

/* The flash's MTD name in the vendor kernels (ingenic_sfc_v1/v2 nand) */
#define SFC "sfc_nand"

/* NAND partition list; composed the same way. */
#define OPENIPC_NAND_MTDPARTS "768k(boot),256k(env),-(ubi)"

/* Environment lives in the NAND 'env' partition: CONFIG_ENV_OFFSET 0xc0000,
 * CONFIG_ENV_SIZE and CONFIG_ENV_RANGE 0x40000 (openipc/nand_config), as
 * on openipc-hisilicon. */

#define CONFIG_BOOTARGS "mem=\\${osmem} rmem=\\${rmem} console=ttyS1,115200 panic=20 init=/init root=ubi0:rootfs rootfstype=ubifs ubi.mtd=2,2048 mtdparts=\\${mtdparts} \\${extras}"

/*
 * The NAND layout is one UBI device after boot and env, holding the UBIFS
 * rootfs with the kernel inside it -- /boot/fitImage, or /boot/uImage on a
 * build without a FIT -- and rootfs_data on the rest. No flash is set aside
 * for a kernel: the rootfs volume is sized to its image. This is the layout
 * of openipc-hisilicon's NAND images.
 *
 * Recovery rewrites the rootfs volume only, sized to the image, with
 * rootfs_data taking the rest. Only a rootfs that grew beyond what the old
 * volume could give costs the data volume, as U-Boot cannot resize UBI
 * volumes; else rootfs_data is preserved.
 */
#define OPENIPC_FWUPD "fwupd=run ur\0"

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

#endif /* CONFIG_JZ_SFC_NAND */

#define CONFIG_BOOTCOMMAND "run bootflash"

#define CONFIG_EXTRA_ENV_SETTINGS \
	OPENIPC_ENV_COMMON \
	OPENIPC_ENV_MEDIUM

#endif /* __OPENIPC_COMMON_H */
