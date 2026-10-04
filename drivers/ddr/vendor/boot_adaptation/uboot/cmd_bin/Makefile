# Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.

ifndef CONFIG_MINI_BOOT
CUR_PATH := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))
################################################################################

ifeq ($(TOPDIR),)
TOPDIR   := $(realpath $(dir $(lastword $(MAKEFILE_LIST)))/../../../../../../)
endif
BOOT_OUTDIR ?= $(TOPDIR)
sinclude $(TOPDIR)/arch/$(ARCH)/config.mk       # include architecture dependend rules

PWD      := $(shell pwd)
OPPDIR   := $(subst $(TOPDIR),,$(PWD))
DDR_TOP_DIR = $(realpath $(CUR_PATH)/../../..)
################################################################################
DDR_CMD     := ddr_cmd
sinclude $(BOOT_OUTDIR)/include/config/auto.conf
ifneq ("$(MAKECMDGOALS)","clean")
CMD_TEXT_BASE := $(shell grep '^\#define.*DDR_TRAINING_RUN_STACK' $(DDR_TOP_DIR)/include/hal/chip/$(SOC)/ddrtrn_training_custom.h|awk '{print $$3}')
endif
STACK_POINT   := $(CMD_TEXT_BASE)

INCLUDE_DIR := -I$(BOOT_OUTDIR)/include \
	-I$(TOPDIR)/include \
	-I$(TOPDIR)/arch/$(ARCH)/include \
	-I$(DDR_TOP_DIR)/include \
	-I$(DDR_TOP_DIR)/include/training \
	-I$(DDR_TOP_DIR)/include/hal/chip/$(SOC)/ \
	-I$(DDR_TOP_DIR)/include/hal/ddrc \
	-I$(DDR_TOP_DIR)/include/hal/ddrt \
	-I$(DDR_TOP_DIR)/include/hal/phy \
	-I$(DDR_TOP_DIR)/boot_adaptation/uboot/cmd_bin/ \
	-I$(DDR_TOP_DIR)/boot_adaptation/common

SRCS := $(wildcard $(DDR_TOP_DIR)/boot_adaptation/uboot/cmd_bin/*.c)
SRCS += $(wildcard $(DDR_TOP_DIR)/boot_adaptation/common/ddrtrn_common_ctl.c)
SRCS += $(wildcard $(DDR_TOP_DIR)/api/ddrtrn_api_low_power.c)
SRCS += $(wildcard $(DDR_TOP_DIR)/training/*.c)
SRCS += $(wildcard $(DDR_TOP_DIR)/hal/*.c)
SRCS += $(wildcard $(DDR_TOP_DIR)/hal/phy/common/*.c)
SRCS += $(wildcard $(DDR_TOP_DIR)/hal/ddrc/common/*.c)
SRCS += $(wildcard $(DDR_TOP_DIR)/hal/ddrt/common/*.c)
SRCS += $(wildcard $(DDR_TOP_DIR)/hal/sysctrl/*.c)
SRCS += $(wildcard $(DDR_TOP_DIR)/hal/uart/*.c)

CFLAGS   := -Os -pipe  \
	-DCMD_TEXT_BASE=$(CMD_TEXT_BASE) \
	-DSTACK_POINT=$(STACK_POINT) \
	-DDDR_TRAINING_CMD \
	-D__KERNEL__ \
	-fno-builtin \
	-ffreestanding \
	-fstack-protector-strong \
	-Wall -Wno-error \
	-include $(TOPDIR)/include/linux/kconfig.h \
	$(INCLUDE_DIR)
ifneq ($(LLVM), )
CFLAGS += -Wno-error=asm-operand-widths
endif

################################################################################
# Project adaptation
# In practice, only the files required by this project need to be included.

LIST_PHY_S14="hi3516cv610"
LIST_PHY_C28_V200="hi3519dv500"

LIST_DDRC_S14="hi3519dv500 hi3516cv610"

LIST_DDRT_V2_3="hi3519dv500 hi3516cv610"

ifneq ("$(MAKECMDGOALS)","clean")
SRCS += $(wildcard $(DDR_TOP_DIR)/hal/chip/$(SOC)/*.c)

ifeq ($(findstring $(SOC), $(LIST_PHY_S14)), $(SOC))
SRCS += $(DDR_TOP_DIR)/hal/phy/ddrtrn_phy_s14.c
else ifeq ($(findstring $(SOC), $(LIST_PHY_C28_V200)), $(SOC))
SRCS += $(DDR_TOP_DIR)/hal/phy/ddrtrn_hal_phy_c28_v200.c
else
$(error "PHY version not support SOC !!!" $(SOC) "end")
endif

ifeq ($(findstring $(SOC), $(LIST_DDRC_S14)), $(SOC))
SRCS += $(DDR_TOP_DIR)/hal/ddrc/ddrtrn_ddrc_s14.c
else
$(error "DMC version not support SOC!!!" $(SOC) "end")
endif

ifeq ($(findstring $(SOC), $(LIST_DDRT_V2_3)), $(SOC))
SRCS += $(DDR_TOP_DIR)/hal/ddrt/ddrtrn_hal_ddrt_v2_3.c
else
$(error "DDRT version not support SOC!!!" $(SOC) "end")
endif
endif
################################################################################
COBJS := $(patsubst %.c,%.o,$(SRCS))
DEPS := $(patsubst %.o,%.d,$(COBJS)) $(patsubst %.o,%.d,$(START))

ifneq ($(OBJTREE),$(SRCTREE))
CFLAGS += -I$(OBJTREE)/include2 -I$(OBJTREE)/include
endif
CFLAGS += $(PLATFORM_RELFLAGS) $(PLATFORM_CPPFLAGS)

ifeq ("$(CONFIG_ARM64)","y")
START       := ddrtrn_cmdbin_entry_64.o
LDS_SCRIPT := ddrtrn_cmdbin_64.lds
CFLAGS += -DCONFIG_ARM64
else
START       := ddrtrn_cmdbin_entry_32.o
LDS_SCRIPT := ddrtrn_cmdbin_32.lds
endif

################################################################################

LINK_FILES = $(SSRC)
.PHONY: $(DDR_CMD).bin
all: $(DDR_CMD).bin
	#remove soft link files
	@rm -f $(LINK_FILES) *.o *.d *.elf *.map *.srec

ifneq ($(LLVM), )
$(DDR_CMD).bin: $(DDR_CMD).elf
	$(OBJCOPY) -O binary $(PWD)/$(DDR_CMD).elf $@
else
$(DDR_CMD).bin: $(DDR_CMD).elf
	$(OBJCOPY) -O srec $(PWD)/$(DDR_CMD).elf $(DDR_CMD).srec
	$(OBJCOPY) --gap-fill=0xff -O binary $(PWD)/$(DDR_CMD).elf $@
endif
$(DDR_CMD).elf: $(START) $(COBJS) $(LDS_SCRIPT)
	$(LD) -Bstatic -T $(LDS_SCRIPT) -Ttext $(CMD_TEXT_BASE) $(START) \
		$(COBJS) $(AOBJS) -Map $(DDR_CMD).map -o $@
endif
.PHONY: clean
clean:
	@rm -vf *.o *.d *.elf *.map *.srec $(LINK_FILES) $(DDR_CMD).bin

%.o : %.S
	$(CC) -D__ASSEMBLY__ $(CFLAGS) -o $@ -c $*.S

%.o : %.c
	$(CC) $(CFLAGS) -Wall -Wstrict-prototypes -fno-stack-protector \
		-o $@ -c $*.c

ifneq ("$(MAKECMDGOALS)","clean")
sinclude $(DEPS)
endif

%.d : %.c
	set -e; $(CC) $(CFLAGS) -MM $< | sed 's,$*.o:,$*.o $*.d:,g' > $@

%.d : %.S
	set -e; $(CC) $(CFLAGS) -MM $< | sed 's,$*.o:,$*.o $*.d:,g' > $@

################################################################################
