# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

mtd-$(CONFIG_MTD) += mtdcore.o mtd_uboot.o
mtd-$(CONFIG_DM_MTD) += mtd-uclass.o
mtd-$(CONFIG_MTD_PARTITIONS) += mtdpart.o
mtd-$(CONFIG_MTD_CONCAT) += mtdconcat.o
mtd-$(CONFIG_ALTERA_QSPI) += altera_qspi.o
mtd-$(CONFIG_FLASH_CFI_DRIVER) += cfi_flash.o
mtd-$(CONFIG_FLASH_CFI_MTD) += cfi_mtd.o
mtd-$(CONFIG_FLASH_CFI_LEGACY) += jedec_flash.o
mtd-$(CONFIG_STM32_FLASH) += stm32_flash.o
mtd-$(CONFIG_RENESAS_RPC_HF) += renesas_rpc_hf.o
mtd-$(CONFIG_HBMC_AM654) += hbmc-am654.o
obj-$(CONFIG_FMC) += fmc_common.o
obj-$(CONFIG_TARGET_XM72020300) += fmc_xm72020300.o
obj-$(CONFIG_TARGET_XM72010200) += fmc_xm72010200.o
obj-$(CONFIG_TARGET_HI3516CV200_FAMILY) += fmc_hi3518ev200.o
obj-$(CONFIG_TARGET_HI3516CV300_FAMILY) += fmc_hi3516cv300.o
obj-$(CONFIG_TARGET_XM72010300) += fmc_xm72010300.o
obj-$(CONFIG_TARGET_XM72050200) += fmc_xm72050200.o
obj-$(CONFIG_TARGET_XM72050300) += fmc_xm72050300.o
obj-$(CONFIG_TARGET_XM72050500) += fmc_xm72050500.o
obj-$(CONFIG_TARGET_XM720XXX) += fmc_xm720xxx.o
obj-$(CONFIG_TARGET_XM76050100) += fmc_xm76050100.o
obj-$(CONFIG_TARGET_HI3516CV500) += fmc_hi3516cv500.o
obj-$(CONFIG_TARGET_HI3516DV300) += fmc_hi3516dv300.o
obj-$(CONFIG_TARGET_HI3516AV300) += fmc_hi3516av300.o
obj-$(CONFIG_TARGET_HI3516CV610_FAMILY) += fmc_hi3516cv610.o
obj-$(CONFIG_TARGET_HI3519DV500_FAMILY) += fmc_hi3519dv500.o

# U-Boot build
ifeq ($(CONFIG_SPL_BUILD)$(CONFIG_TPL_BUILD),)

ifneq ($(mtd-y),)
obj-y += mtd.o
endif
obj-y += nand/
obj-y += onenand/
obj-y += spi/
obj-$(CONFIG_MTD_UBI) += ubi/

#SPL/TPL build
else

ifneq ($(mtd-y),)
obj-$(CONFIG_SPL_MTD_SUPPORT) += mtd.o
endif
obj-$(CONFIG_$(SPL_TPL_)NAND_SUPPORT) += nand/
obj-$(CONFIG_SPL_ONENAND_SUPPORT) += onenand/
obj-$(CONFIG_$(SPL_TPL_)SPI_FLASH_SUPPORT) += spi/
obj-$(CONFIG_SPL_UBI) += ubispl/

endif
