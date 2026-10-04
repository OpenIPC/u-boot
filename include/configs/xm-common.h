
#ifdef VENDOR_HISILICON
#define SFC "hi_sfc"
#define VENDOR "hisilicon"
#else
#define SFC "sfc"
#define VENDOR "goke"
#endif

#ifndef PRODUCT_SOCMODEL
#define PRODUCT_SOCMODEL
#endif

#undef CONFIG_SYS_CBSIZE
#undef CONFIG_SYS_LOAD_ADDR

#define CONFIG_SYS_INIT_RAM_ADDR 0x04000000
#ifndef CONFIG_SYS_NAND_MAX_CHIPS
#define CONFIG_SYS_NAND_MAX_CHIPS 1
#endif
#define CONFIG_SYS_INIT_RAM_SIZE 0x14000
#undef CONFIG_SYS_PROMPT
#undef CONFIG_BOOTARGS

#define CONFIG_SYS_PROMPT "OpenIPC # "
#define CONFIG_SYS_CBSIZE 1024
#define CONFIG_SYS_LOAD_ADDR 0x42000000

#define CONFIG_SERVERIP 192.168.1.1
#define CONFIG_GATEWAYIP 192.168.1.1
#define CONFIG_IPADDR 192.168.1.107
#define CONFIG_NETMASK 255.255.255.0

#define CONFIG_CMD_ECHO
#define CONFIG_CMD_LOADB 1
#define CONFIG_CMD_SOURCE
#define CONFIG_VERSION_VARIABLE

#ifdef CONFIG_MMC
#define CONFIG_CMD_MMC
#endif

#undef CONFIG_ENV_OFFSET
#define CONFIG_ENV_OFFSET 0x40000

#define CONFIG_ENV_KERNADDR 0x50000
#define CONFIG_ENV_KERNSIZE 0x200000
#define CONFIG_ENV_ROOTADDR 0x250000
#define CONFIG_ENV_ROOTSIZE 0x500000

#define CONFIG_BOOTARGS "mem=\\${osmem} console=ttyAMA0,115200 panic=20 root=/dev/mtdblock3 rootfstype=squashfs init=/init mtdparts=" SFC ":256k(boot),64k(env),2048k(kernel),\\${rootmtd}(rootfs),-(rootfs_data) \\${extras}"
#define CONFIG_BOOTCOMMAND "setenv bootcmd ${bootnor}; sf probe 0; saveenv; run bootcmd"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"baseaddr=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"kernaddr=" __stringify(CONFIG_ENV_KERNADDR) "\0" \
	"kernsize=" __stringify(CONFIG_ENV_KERNSIZE) "\0" \
	"rootaddr=" __stringify(CONFIG_ENV_ROOTADDR) "\0" \
	"rootsize=" __stringify(CONFIG_ENV_ROOTSIZE) "\0" \
	"rootmtd=5120k\0" \
	"cmdnor=sf probe 0; setenv setargs setenv bootargs ${bootargs}; run setargs; sf read ${baseaddr} ${kernaddr} ${kernsize}; bootm ${baseaddr}\0" \
	"bootnor=run fwrecovery; sf probe 0; setenv setargs setenv bootargs ${bootargs}; run setargs; sf read ${baseaddr} ${kernaddr} ${kernsize}; bootm ${baseaddr}\0" \
	"fwupd=${fetchcmd} ${baseaddr} firmware.${soc} && run fwwrite\0" \
	"fwrecovery=if env exists bootfail; then run fwupd; sleep 5; reset; fi\0" \
	"fwwrite=sf probe 0; sf erase ${kernaddr} ${filesize}; sf write ${baseaddr} ${kernaddr} ${filesize}\0" \
	"ubnor=${updatetool} ${baseaddr} u-boot-${soc}-nor.bin && run ubwrite\0" \
	"uknor=${updatetool} ${baseaddr} uImage.${soc} && run ukwrite\0" \
	"urnor=${updatetool} ${baseaddr} rootfs.squashfs.${soc} && run urwrite\0" \
	"ubwrite=sf probe 0; sf erase 0x0 ${kernaddr}; sf write ${baseaddr} 0x0 ${kernaddr}\0" \
	"ukwrite=sf probe 0; sf erase ${kernaddr} ${kernsize}; sf write ${baseaddr} ${kernaddr} ${filesize}\0" \
	"urwrite=sf probe 0; sf erase ${rootaddr} ${rootsize}; sf write ${baseaddr} ${rootaddr} ${filesize}\0" \
	"nfsroot=/srv/nfs/" __stringify(PRODUCT_SOC) "\0" \
	"bootargsnfs=mem=\${osmem} console=ttyAMA0,115200 panic=20 root=/dev/nfs rootfstype=nfs ip=${ipaddr}:::255.255.255.0::eth0 nfsroot=${serverip}:${nfsroot},v3,nolock rw \${extras}\0" \
	"bootnfs=setenv setargs setenv bootargs ${bootargsnfs}; run setargs; tftpboot ${baseaddr} uImage.${soc}; bootm ${baseaddr}\0" \
	"sdload=setenv fetchcmd fatload mmc 0\0" \
	"norload=sf probe 0; sf read\0" \
	"fetchcmd=tftpboot\0" \
	"sdcard=setenv updatetool fatload mmc 0\0" \
	"updatetool=tftpboot\0" \
	"osmem=32M\0" \
	"bootargs=" CONFIG_BOOTARGS"\0" \
	"board_name\0" \
	"board=" __stringify(PRODUCT_SOC) "\0" \
	"vendor=" VENDOR"\0" \
	"soc=" __stringify(PRODUCT_SOC) "\0" \
	"socmodel=" __stringify(PRODUCT_SOCMODEL)

#ifdef CONFIG_FMC_SPI_NAND

#ifdef VENDOR_HISILICON
#define SFC "hinand"
#else
#define SFC "nand"
#endif

#define CONFIG_BOOTARGS "mem=\${osmem} console=ttyAMA0,115200 panic=20 init=/init root=/dev/ubiblock0_1 ubi.mtd=2,2048 ubi.block=0,1 \${mtdparts} \${extras}"
#define CONFIG_BOOTCOMMAND "setenv setargs setenv bootargs ${bootargs}; run setargs; ubi part ubi; ubi read ${baseaddr} kernel; bootm ${baseaddr}; reset"

#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET 0xc0000
#define CONFIG_ENV_SIZE 0x40000
#define CONFIG_ENV_SECT_SIZE 0x20000

#define CONFIG_EXTRA_ENV_SETTINGS \
	"baseaddr=0x42000000\0" \
	"fwupd=${fetchcmd} ${baseaddr} firmware.ubi.${soc} && nand erase 0x100000 0x7f00000; nand write ${baseaddr} 0x100000 ${filesize}\0" \
	"fwrecovery=if env exists bootfail; then run fwupd; sleep 5; reset; fi\0" \
	"sdload=setenv fetchcmd fatload mmc 0\0" \
	"fetchcmd=tftpboot\0" \
	"urnand=tftpboot ${baseaddr} rootfs.ubi.${soc} && nand erase 0x100000 0x7f00000; nand write ${baseaddr} 0x100000 ${filesize}\0" \
	"mtdparts=mtdparts="SFC":768k(boot),256k(env),-(ubi)\0" \
	"nfsroot=/srv/nfs/" __stringify(PRODUCT_SOC) "\0" \
	"bootargsnfs=mem=\${osmem} console=ttyAMA0,115200 panic=20 root=/dev/nfs rootfstype=nfs ip=${ipaddr}:::255.255.255.0::eth0 nfsroot=${serverip}:${nfsroot},v3,nolock rw \${extras}\0" \
	"bootargs="CONFIG_BOOTARGS"\0" \
	"bootnfs=setenv setargs setenv bootargs ${bootargsnfs}; run setargs; tftpboot ${baseaddr} uImage.${soc}; bootm ${baseaddr}\0" \
	"osmem=32M\0" \
	"mtdids=nand0="SFC"\0" \
	"board_name\0" \
	"board=" __stringify(PRODUCT_SOC) "\0" \
	"vendor=" VENDOR"\0" \
	"soc=" __stringify(PRODUCT_SOC) "\0" \
	"socmodel=" __stringify(PRODUCT_SOCMODEL)

#define CONFIG_SYS_MALLOC_LEN (32 * SZ_128K)
#endif