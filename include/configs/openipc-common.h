/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Common OpenIPC configuration for the Fullhan targets.
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

#define VENDOR			"fullhan"

/*
 * Kernel memory (mem=). The media pool (vmm.ko's mmz) and the ARC's
 * firmware go above it: OpenIPC's load_fullhan places the pool at
 * 0xA2000000 on V200 and at 0xA1D00000 on V100, and the SDKs recommend the
 * same for the others.
 */
#if defined(CONFIG_FULLHAN_V200) || defined(CONFIG_FULLHAN_865X)
#define OPENIPC_OSMEM		"32M"
#else
#define OPENIPC_OSMEM		"29M"
#endif

/* Where the vendor U-Boots load the kernel, clear of it as it unpacks */
#define OPENIPC_BASEADDR	0xA1000000

/* NOR layout, shared with every OpenIPC NOR camera */
#define OPENIPC_KERNADDR	0x50000
#define OPENIPC_KERNSIZE	0x200000
#define OPENIPC_ROOTADDR	0x250000
#define OPENIPC_ROOTSIZE	0x500000

#define OPENIPC_FW_FILE		"firmware.${soc}"

#define OPENIPC_BOOTARGSNFS "mem=\\${osmem} console=ttyS0,115200 panic=20 root=/dev/nfs rootfstype=nfs ip=${ipaddr}:::255.255.255.0::eth0 nfsroot=${serverip}:${nfsroot},v3,nolock rw \\${extras}"

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

/*
 * The flash's MTD name in the vendor kernels: the SPI device under 4.9's
 * spi-nor, the m25p80 platform data's name under 3.0.8
 */
#if defined(CONFIG_FULLHAN_V200) || defined(CONFIG_FULLHAN_865X)
#define SFC "spi0.0"
#else
#define SFC "spi_flash"
#endif

#define CONFIG_BOOTARGS "mem=\\${osmem} console=ttyS0,115200 panic=20 root=/dev/mtdblock3 rootfstype=squashfs init=/init mtdparts=\\${mtdparts} \\${extras}"

/*
 * Recovery: the rootfs is fetched first so its size selects the NOR schema.
 * It starts from the lite layout, not from what the scan found: on a camera
 * that still holds its vendor firmware the scan reports the vendor's
 * offsets, and writing OpenIPC there could leave the kernel write erasing
 * the start of the new rootfs.
 */
#define OPENIPC_FWUPD "fwupd=run mtdslite; run ur; run uk\0"

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
	"fwwrite=sf probe 0; sf erase ${kernaddr} ${filesize}; sf write ${baseaddr} ${kernaddr} ${filesize}\0" \
	"loadkernel=" OPENIPC_LOADKERNEL "\0" \
	"mtdids=" SFC "\0" \
	"mtds=" OPENIPC_MTDPARTS "\0" \
	"mtdparts=" SFC ":" OPENIPC_MTDPARTS "\0" \
	"bootargs=" CONFIG_BOOTARGS

#define CONFIG_BOOTCOMMAND "run bootflash"

#define CONFIG_EXTRA_ENV_SETTINGS \
	OPENIPC_ENV_COMMON \
	OPENIPC_ENV_MEDIUM

#endif /* __OPENIPC_COMMON_H */
