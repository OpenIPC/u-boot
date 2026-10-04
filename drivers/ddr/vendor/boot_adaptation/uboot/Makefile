# Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.

ifndef CONFIG_MINI_BOOT
ifeq ($(TOPDIR),)
TOPDIR   := $(realpath $(dir $(lastword $(MAKEFILE_LIST)))/../../../../..)
endif
DDR_TOP_DIR := $(TOPDIR)/drivers/ddr/vendor

EXTRA_CFLAGS += -I$(TOPDIR) \
	-I$(DDR_TOP_DIR)/boot_adaptation/uboot/ \
	-I$(DDR_TOP_DIR)/boot_adaptation/common/ \
	-I$(DDR_TOP_DIR)/include \
	-I$(DDR_TOP_DIR)/include/hal/chip/$(SOC)/ \
	-I$(DDR_TOP_DIR)/include/hal/ddrc \
	-I$(DDR_TOP_DIR)/include/hal/ddrt \
	-I$(DDR_TOP_DIR)/include/hal/phy

obj-y   := ddrtrn_uboot_cmd_loc.o
obj-y   += ddrtrn_uboot_cmd_ctl.o
obj-y   += ddrtrn_cmd_parsing.o
obj-y   += ../common/ddrtrn_boot.o
obj-y   += ../../api/ddrtrn_api_low_power.o
obj-y   += ../../training/ddrtrn_phy.o
obj-y   += ../../training/ddrtrn_low_power.o
obj-y   += ../../hal/ddrtrn_hal_context.o
obj-y   += ../../hal/phy/common/ddrtrn_hal_phy.o
obj-y   += ../../hal/phy/common/ddrtrn_hal_phy_common.o
obj-y   += ../../hal/ddrc/common/ddrtrn_hal_ddrc.o

obj-y   += ../../hal/chip/$(SOC)/ddrtrn_training_custom.o

################################################################################
# Project adaptation
# In practice, only the files required by this project need to be included.

LIST_PHY_S14="hi3516cv610"
LIST_PHY_C28_V200="hi3519dv500"

ifeq ($(findstring $(SOC), $(LIST_PHY_S14)), $(SOC))
obj-y   += ../../hal/phy/ddrtrn_phy_s14.o
else ifeq ($(findstring $(SOC), $(LIST_PHY_C28_V200)), $(SOC))
obj-y   += ../../hal/phy/ddrtrn_hal_phy_c28_v200.o
else
$(error "Not support SOC !!!" $(SOC) "end")
endif
endif

