# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2012 Henrik Nordstrom <henrik@henriknordstrom.net>
#
# Based on some other board Makefile
#
# (C) Copyright 2000-2003
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
obj-y	+= board.o
obj-$(CONFIG_SUN7I_GMAC)	+= gmac.o
obj-$(CONFIG_MACH_SUN4I)	+= dram_sun4i_auto.o
obj-$(CONFIG_MACH_SUN5I)	+= dram_sun5i_auto.o
obj-$(CONFIG_MACH_SUN7I)	+= dram_sun5i_auto.o
obj-$(CONFIG_CHIP_DIP_SCAN)	+= chip.o

ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_MACH_SUN8I_V831)	+= v83x.o v83x_kernel_dtb.o

# The vendor kernel's device tree, gzipped into U-Boot (v83x.c)
AFLAGS_v83x_kernel_dtb.o += -DV83X_KERNEL_DTB_GZ=\"$(obj)/v83x-kernel.dtb.gz\"
$(obj)/v83x_kernel_dtb.o: $(obj)/v83x-kernel.dtb.gz

quiet_cmd_dtbgz = DTBGZ   $@
      cmd_dtbgz = $(DTC) -q -I dts -O dtb -o - $< | gzip -9n > $@

$(obj)/v83x-kernel.dtb.gz: $(src)/v83x-kernel.dts FORCE
	$(call if_changed,dtbgz)

targets += v83x-kernel.dtb.gz
endif
