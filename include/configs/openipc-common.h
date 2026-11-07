/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Common OpenIPC configuration for the SigmaStar targets.
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

#define VENDOR			"sigmastar"

/*
 * infinity3 kernel memory (mem=): all of it, as the vendor U-Boot passes.
 * The media drivers take their buffers from the kernel. On the later dies
 * ss_set_mem_env() sets memlx and memsz at every boot instead.
 */
#ifdef CONFIG_SS_MMA_BOOTARGS
#define OPENIPC_ENV_OSMEM	""
#elif PHYS_SDRAM_1_SIZE == 0x08000000
#define OPENIPC_ENV_OSMEM	"osmem=128M\0"
#else
#define OPENIPC_ENV_OSMEM	"osmem=64M\0"
#endif

/*
 * Where the vendor U-Boot reads the kernel: 16 MiB above its load address,
 * 0x20008000
 */
#define OPENIPC_BASEADDR	0x21000000

/* NOR layout, shared with every OpenIPC NOR camera */
#define OPENIPC_KERNADDR	0x50000
#define OPENIPC_KERNSIZE	0x200000
#define OPENIPC_ROOTADDR	0x250000
#define OPENIPC_ROOTSIZE	0x500000

#define OPENIPC_FW_FILE		"firmware.${soc}"

/*
 * What the kernel is told about memory: mem= on infinity3; LX_MEM and the
 * MMA heap on the later dies, as OpenIPC's vendor U-Boot passes them.
 */
#ifdef CONFIG_SS_MMA_BOOTARGS
#define OPENIPC_MEMARGS_HEAD ""
#define OPENIPC_MEMARGS_TAIL " LX_MEM=\\${memlx} mma_heap=mma_heap_name0,miu=0,sz=\\${memsz} cma=2M"
#else
#define OPENIPC_MEMARGS_HEAD "mem=\\${osmem} "
#define OPENIPC_MEMARGS_TAIL ""
#endif

#define OPENIPC_BOOTARGSNFS OPENIPC_MEMARGS_HEAD "console=ttyS0,115200 panic=20 root=/dev/nfs rootfstype=nfs ip=${ipaddr}:::255.255.255.0::eth0 nfsroot=${serverip}:${nfsroot},v3,nolock rw" OPENIPC_MEMARGS_TAIL " \\${extras}"

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

#ifdef CONFIG_ENV_IS_IN_NAND

/*
 * SPI NAND, in the layout of OpenIPC/u-boot-sigmastar, whose saved
 * environments have to keep working: the boot partitions, the environment
 * and one UBI device, with the kernel and a squashfs rootfs in volumes of
 * their own. "mtdparts" holds the kernel argument whole.
 */
#ifdef CONFIG_SS_INFINITY2M
/* Its PNI puts the stages further up: env in blocks 34-36, then ubi */
#define OPENIPC_NAND_MTDPARTS "nand0:4352k(boot),384k(env),-(ubi)"
#define OPENIPC_NAND_BOOTSIZE 0x440000
#define OPENIPC_NAND_ROOTADDR 0x4A0000
#define OPENIPC_NAND_ROOTSIZE 0x7B60000
#define OPENIPC_NAND_ROOTDEV "mtdblock4"
#elif defined(CONFIG_SS_IFLIEGEN)
/* The Ifliegen SDK's: CIS, BOOT and BOOT_BAK, the environment at 0x500000 */
#define OPENIPC_NAND_MTDPARTS "nand0:1280k(cis),1920k(boot0),1920k(boot1),256k(env),-(ubi)"
#define OPENIPC_NAND_BOOTSIZE 0x500000
#define OPENIPC_NAND_ROOTADDR 0x540000
#define OPENIPC_NAND_ROOTSIZE 0x7AC0000
#define OPENIPC_NAND_ROOTDEV "mtdblock6"
#elif defined(CONFIG_SS_IFORD)
/* The Iford SDK's: CIS, BOOT and its copy, the environment at 0x580000, ubi */
#define OPENIPC_NAND_MTDPARTS "nand0:1280k(cis),2176k(boot0),2176k(boot1),256k(env),-(ubi)"
#define OPENIPC_NAND_BOOTSIZE 0x580000
#define OPENIPC_NAND_ROOTADDR 0x5C0000
#define OPENIPC_NAND_ROOTSIZE 0x7A40000
#define OPENIPC_NAND_ROOTDEV "mtdblock6"
#elif defined(CONFIG_SS_I6C_FAMILY)
/* The SDK's: the CIS, BOOT and BOOT_BAK, the environment, ubi */
#define OPENIPC_NAND_MTDPARTS "nand0:1280k(cis),1792k(boot0),1792k(boot1),256k(env),-(ubi)"
#define OPENIPC_NAND_BOOTSIZE 0x4C0000
#define OPENIPC_NAND_ROOTADDR 0x500000
#define OPENIPC_NAND_ROOTSIZE 0x7B00000
#define OPENIPC_NAND_ROOTDEV "mtdblock6"
#else
#define OPENIPC_NAND_MTDPARTS "nand0:1024k(boot0),1024k(boot1),256k(env),-(ubi)"
#define OPENIPC_NAND_BOOTSIZE 0x200000
#define OPENIPC_NAND_ROOTADDR 0x240000
#define OPENIPC_NAND_ROOTSIZE 0x7DC0000
#define OPENIPC_NAND_ROOTDEV "mtdblock5"
#endif
/*
 * The rootfs volume as the OpenIPC kernels number it: gluebi makes each
 * UBI volume an MTD after the partitions, kernel then rootfs. A kernel
 * without gluebi takes extras="ubi.block=0,rootfs root=/dev/ubiblock0_1
 * rootfstype=squashfs", the later root= winning.
 */

/* As the firmware's own volume holds it, 34 LEBs of 126976 bytes */
#define OPENIPC_KERNEL_VOLUME_SIZE "0x400000"

#define CONFIG_BOOTARGS "console=ttyS0,115200 panic=20 root=/dev/" OPENIPC_NAND_ROOTDEV " init=/init ubi.mtd=ubi \\${mtdparts}" OPENIPC_MEMARGS_TAIL " \\${extras}"

/* rootfs.ubi carries the kernel volume too */
#define OPENIPC_FWUPD "fwupd=run ur\0"

#define OPENIPC_LOADKERNEL "ubi part ubi; ubi read ${baseaddr} kernel"
#define OPENIPC_ENV_MEDIUM \
	"ubfile=u-boot-" CONFIG_PRODUCT_SOC "-nand.bin\0" \
	"urfile=rootfs.ubi." CONFIG_PRODUCT_SOC "\0" \
	"rootaddr=" __stringify(OPENIPC_NAND_ROOTADDR) "\0" \
	"rootsize=" __stringify(OPENIPC_NAND_ROOTSIZE) "\0" \
	"ubwrite=nand erase 0x0 " __stringify(OPENIPC_NAND_BOOTSIZE) "; nand write ${baseaddr} 0x0 ${filesize}\0" \
	"ukwrite=ubi part ubi; ubi write ${baseaddr} kernel ${filesize}\0" \
	"urwrite=nand erase ${rootaddr} ${rootsize}; nand write ${baseaddr} ${rootaddr} ${filesize}\0" \
	"loadkernel=" OPENIPC_LOADKERNEL "\0" \
	"mtdids=nand0=nand0\0" \
	"mtdparts=mtdparts=" OPENIPC_NAND_MTDPARTS "\0" \
	"bootargs=" CONFIG_BOOTARGS

#else /* !CONFIG_ENV_IS_IN_NAND */

/*
 * The flash's MTD name in the vendor kernels: drivers/mstar/flash_isp's
 * (the vendor U-Boot passes mtdparts=NOR_FLASH:...); the SGS kernels, which
 * carry Iford, Ifliegen and Ifado (drivers/sgs_common/flash/nor), call it
 * nor0.
 */
#if defined(CONFIG_SS_IFORD_FAMILY) || defined(CONFIG_SS_IFADO)
#define SFC "nor0"
#else
#define SFC "NOR_FLASH"
#endif

#define CONFIG_BOOTARGS OPENIPC_MEMARGS_HEAD "console=ttyS0,115200 panic=20 root=/dev/mtdblock3 rootfstype=squashfs init=/init mtdparts=\\${mtdparts}" OPENIPC_MEMARGS_TAIL " \\${extras}"

/* Recovery: the rootfs is fetched first so its size selects the NOR schema */
#define OPENIPC_FWUPD "fwupd=run ur; run uk\0"

#define OPENIPC_LOADKERNEL "sf probe 0; sf read ${baseaddr} ${kernaddr} ${kernsize}"
#define OPENIPC_ENV_MEDIUM \
	"ubfile=u-boot-" CONFIG_PRODUCT_SOC "-nor.bin\0" \
	"urfile=rootfs.squashfs." CONFIG_PRODUCT_SOC "\0" \
	"kernaddr=" __stringify(OPENIPC_KERNADDR) "\0" \
	"kernsize=" __stringify(OPENIPC_KERNSIZE) "\0" \
	"rootaddr=" __stringify(OPENIPC_ROOTADDR) "\0" \
	"rootsize=" __stringify(OPENIPC_ROOTSIZE) "\0" \
	"mtdslite=setenv kernsize 0x200000; setenv rootaddr 0x250000; setenv rootsize 0x500000\0" \
	"mtdsultimate=setenv kernsize 0x300000; setenv rootaddr 0x350000; setenv rootsize 0xa00000\0" \
	"ubwrite=sf probe 0; sf erase 0x0 ${kernaddr}; sf write ${baseaddr} 0x0 ${filesize}\0" \
	"ukwrite=sf probe 0; sf erase ${kernaddr} ${kernsize}; sf write ${baseaddr} ${kernaddr} ${filesize}\0" \
	"urwrite=sf probe 0; if itest ${filesize} -gt 500000; then run mtdsultimate; fi; sf erase ${rootaddr} ${rootsize}; sf write ${baseaddr} ${rootaddr} ${filesize}\0" \
	"fwwrite=sf probe 0; sf erase ${kernaddr} ${filesize}; sf write ${baseaddr} ${kernaddr} ${filesize}\0" \
	"loadkernel=" OPENIPC_LOADKERNEL "\0" \
	"mtdids=" SFC "\0" \
	"mtds=" OPENIPC_MTDPARTS "\0" \
	"mtdparts=" SFC ":" OPENIPC_MTDPARTS "\0" \
	"bootargs=" CONFIG_BOOTARGS

#endif /* CONFIG_ENV_IS_IN_NAND */

#define CONFIG_BOOTCOMMAND "run bootflash"

#define CONFIG_EXTRA_ENV_SETTINGS \
	OPENIPC_ENV_COMMON \
	OPENIPC_ENV_MEDIUM

#endif /* __OPENIPC_COMMON_H */
