sinclude $(TOPDIR)/config.mk

ifeq ($(CONFIG_PRODUCTNAME), "hi3516dv500")
	cflags-y  += -DAO_PRODUCT_HI3516DV500
	CONFIG_PRODUCT_NAME=hi3519dv500
else ifeq ($(CONFIG_PRODUCTNAME), "hi3519dv500")
	cflags-y  += -DAO_PRODUCT_HI3519DV500
	CONFIG_PRODUCT_NAME=hi3519dv500
else ifeq ($(CONFIG_PRODUCTNAME), "hi3516cv610")
	cflags-y  += -DAO_PRODUCT_HI3516CV610
	CONFIG_PRODUCT_NAME=hi3516cv610
endif

cflags-y += -I$(srctree)/product/audio/ao/include
cflags-y += -I$(srctree)/product/audio/ao/arch/include
cflags-y += -I$(srctree)/product/audio/ao/arch/$(CONFIG_PRODUCT_NAME)/include

ccflags-y  += $(cflags-y)
HOSTCFLAGS += $(cflags-y)
CPPFLAGS   += $(cflags-y)

obj-y += arch/$(CONFIG_PRODUCT_NAME)/aiao_hal.o
obj-y += arch/$(CONFIG_PRODUCT_NAME)/amp.o
obj-y += arch/common/aiao_hal_common.o
obj-y += ao.o
