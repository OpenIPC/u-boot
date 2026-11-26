#
# (C) Copyright 2000-2004
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-$(CONFIG_TARGET_HI3519DV500_FAMILY)	:= hi3519dv500.o

# bootm starts Linux through the secure loader and BL31 (bl31_handoff.c,
# bl31.README).
# A NAND build reads BL31 from the boot partition (build.sh appends it to the
# boot image); a NOR build links it in.
obj-$(CONFIG_TARGET_HI3519DV500_FAMILY)	+= bl31_handoff.o
ifndef CONFIG_FMC_SPI_NAND
obj-$(CONFIG_TARGET_HI3519DV500_FAMILY)	+= bl31_blob.o
AFLAGS_bl31_blob.o := -I$(srctree)/$(src)
$(obj)/bl31_blob.o: $(src)/bl31.bin
endif
