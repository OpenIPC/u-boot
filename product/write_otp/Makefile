CONFIG_SOC_FAMILY ?= $(CONFIG_SYS_SOC)

WRITE_OTP_SRCS_DIR := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

ifneq (,$(findstring UBOOT,$(KBUILD_CPPFLAGS)))
EXCUTE_ENV ?= u_boot
else
EXCUTE_ENV ?= userspace
endif

WRITE_OTP_CFLAGS 	= -I$(WRITE_OTP_SRCS_DIR)/arch/${CONFIG_SOC_FAMILY}/
WRITE_OTP_CFLAGS 	+= -I$(WRITE_OTP_SRCS_DIR)/include/

WRITE_OTP_SRCS += write_otp.c
WRITE_OTP_SRCS += init/cmd_write_otp_init.c

include $(WRITE_OTP_SRCS_DIR)/scripts/Makefile.$(EXCUTE_ENV).mak
