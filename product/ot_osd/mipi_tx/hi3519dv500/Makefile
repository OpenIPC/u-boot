#MIPI_TX_PATH := $(srctree)/product/ot_osd/mipi_tx

ARCH_NAME := hi3519dv500

cflags-y += -I$(srctree)/product/ot_osd/mipi_tx/$(ARCH_NAME)
cflags-y += -I$(srctree)/product/ot_osd/mipi_tx/$(ARCH_NAME)/include
cflags-y += -I$(srctree)/product/ot_osd/mipi_tx/$(ARCH_NAME)/hal
cflags-y += -I$(srctree)/product/ot_osd/vo/include
cflags-y += -I$(srctree)/product/ot_osd/include

cflags-y     += -I$(srctree)/include

ccflags-y  += $(cflags-y)
HOSTCFLAGS += $(cflags-y)
CPPFLAGS   += $(cflags-y)

obj-y += ./mipi_tx_hal.o
obj-y += ./sample_comm_mipi_tx.o
obj-y += ./mipi_tx.o

ifeq ($(CONFIG_PRODUCTNAME), "hi3519dv500")
ccflags-y += -DCONFIG_OT_MIPI_TX_SUPPORT
endif

