# SPDX-License-Identifier: GPL-2.0+

extra-$(CONFIG_SPL_BUILD) := start.o

SOC := $(CONFIG_SYS_SOC:"%"=%)

obj-y += $(SOC)/clk.o

ifdef CONFIG_CPU_XBURST2
# XBurst2: the generation's own CPU, timer, GPIO, SFC and DDR code
obj-y += xburst2/
obj-$(CONFIG_SPL_BUILD) += spl.o $(SOC)/pll.o $(SOC)/ddr_set_dll.o
else
obj-y += cpu.o timer.o gpio.o jz_serial.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o spl_sfc_nor.o
obj-y += $(SOC)/pll.o $(SOC)/ddr_set_dll.o
obj-$(CONFIG_DDR_INNOPHY) += ddr_innophy.o
ifndef CONFIG_DDR_INNOPHY
obj-y += ddr_dwc.o
endif
obj-$(CONFIG_SPL_DDR_SOFT_TRAINING) += ddr_dqs_training.o

# The DDR controller and PHY register values, computed on the build host
# from the DDR chip's timings, as the vendor tools/ingenic-tools did
hostprogs-y := ddr_params_creator
ddr_params_creator-objs := ddr_params_creator.o
ddr_params_creator-objs-$(CONFIG_DDR_TYPE_DDR2) += ddr2_params.o
ddr_params_creator-objs-$(CONFIG_DDR_TYPE_DDR3) += ddr3_params.o
ddr_params_creator-objs += $(ddr_params_creator-objs-y)
# The target's headers after the host's, so <stdio.h> is the host's
HOST_EXTRACFLAGS += -idirafter $(srctree)/include -idirafter $(objtree)/include \
	-idirafter $(srctree)/arch/mips/include \
	-idirafter $(srctree)/arch/mips/mach-ingenic/include \
	-idirafter $(srctree)/arch/mips/mach-ingenic/xburst/include -DUSE_HOSTCC

quiet_cmd_ddr_values = GEN     $@
      cmd_ddr_values = $(obj)/ddr_params_creator > $@

$(obj)/ddr_reg_values.h: $(obj)/ddr_params_creator FORCE
	$(call if_changed,ddr_values)

targets += ddr_reg_values.h
ccflags-y += -I$(obj)
$(obj)/ddr_innophy.o $(obj)/ddr_dwc.o $(obj)/ddr_dqs_training.o: \
	$(obj)/ddr_reg_values.h
endif
endif
