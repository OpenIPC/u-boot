# SPDX-License-Identifier: GPL-2.0+
#
# Ingenic XBurst2 (T40): the vendor arch/mips/cpu/xburst2 of
# OpenIPC/u-boot-t40.

obj-y += cpu.o gost_timer.o
# The T41 has the SDK's GPIO layer and DDR code (t41/), not u-boot-t40's
ifndef CONFIG_T41
obj-y += gpio.o
endif
obj-y += jz_serial.o

ifdef CONFIG_SPL_BUILD
obj-y += spl_sfc_nor.o
ifndef CONFIG_T41
obj-y += ddr_innophy.o

# The DDR controller and PHY register values, computed on the build host
# from the DDR chip's timings by the XBurst2 vendor tools/ingenic-tools
hostprogs-y := ddr_params_creator
ddr_params_creator-objs := ddr_params_creator.o
ddr_params_creator-objs-$(CONFIG_DDR_TYPE_DDR2) += ddr2_params.o
ddr_params_creator-objs-$(CONFIG_DDR_TYPE_DDR3) += ddr3_params.o
ddr_params_creator-objs += $(ddr_params_creator-objs-y)
# The target's headers after the host's, so <stdio.h> is the host's
HOST_EXTRACFLAGS += -idirafter $(srctree)/include -idirafter $(objtree)/include \
	-idirafter $(srctree)/arch/mips/include \
	-idirafter $(srctree)/arch/mips/mach-ingenic/include \
	-idirafter $(srctree)/arch/mips/mach-ingenic/xburst2/include -DUSE_HOSTCC

quiet_cmd_ddr_values = GEN     $@
      cmd_ddr_values = $(obj)/ddr_params_creator > $@

$(obj)/ddr_reg_values.h: $(obj)/ddr_params_creator FORCE
	$(call if_changed,ddr_values)

targets += ddr_reg_values.h
ccflags-y += -I$(obj)
$(obj)/ddr_innophy.o: $(obj)/ddr_reg_values.h
endif
endif
