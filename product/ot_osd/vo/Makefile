ifeq ($(CONFIG_PRODUCTNAME), "hi3519dv500")
VO_ARCH_NAME = hi3519dv500
endif

cflags-y     += -I$(srctree)/product/ot_osd/include
cflags-y     += -I$(srctree)/product/ot_osd/vo/ext_inc
cflags-y     += -I$(srctree)/product/ot_osd/vo/include
cflags-y     += -I$(srctree)/product/ot_osd/vo/mkp/include
cflags-y     += -I$(srctree)/product/ot_osd/vo/mkp/src
cflags-y     += -I$(srctree)/product/ot_osd/vo/mkp/src/dev
cflags-y     += -I$(srctree)/product/ot_osd/vo/mkp/src/intf
cflags-y     += -I$(srctree)/product/ot_osd/vo/mkp/src/layer
cflags-y     += -I$(srctree)/product/ot_osd/vo/mkp/src/common
cflags-y     += -I$(srctree)/product/ot_osd/vo/arch/include
cflags-y     += -I$(srctree)/product/ot_osd/vo/arch/comm/src
cflags-y     += -I$(srctree)/product/ot_osd/vo/arch/comm/include
cflags-y     += -I$(srctree)/product/ot_osd/vo/arch/$(VO_ARCH_NAME)/include
cflags-y     += -I$(srctree)/product/ot_osd/vo/arch/include
cflags-y     += -I$(srctree)/product/ot_osd/vo/arch/$(VO_ARCH_NAME)/include/$(VO_SUB_ARCH_NAME)

ccflags-y  += $(cflags-y)
#HOSTCFLAGS += $(cflags-y)
#CPPFLAGS   += $(cflags-y)

CBB_VO_ARCH_COMM_SRC = arch/comm/src
CBB_VO_ARCH_HAL = arch/$(VO_ARCH_NAME)/hal

#enum mkp src files
obj-y += mkp/src/vo.o
obj-y += mkp/src/mkp_vo_user.o
obj-y += mkp/src/dev/mkp_vo_dev.o
obj-y += mkp/src/intf/mkp_vo_intf.o
obj-y += mkp/src/layer/mkp_vo_video.o
obj-y += mkp/src/layer/mkp_vo_gfx.o
obj-y += mkp/src/common/mkp_vo_comm.o
obj-y += mkp/src/common/mkp_vo_init.o
obj-y += mkp/src/mkp_vo_bmp.o

#enum arch comm src files
obj-y += $(CBB_VO_ARCH_COMM_SRC)/drv_vo_comm.o
obj-y += $(CBB_VO_ARCH_COMM_SRC)/drv_vo_dev_comm.o
obj-y += $(CBB_VO_ARCH_COMM_SRC)/drv_vo_layer_comm.o
obj-y += $(CBB_VO_ARCH_COMM_SRC)/drv_vo_video_comm.o
obj-y += $(CBB_VO_ARCH_COMM_SRC)/drv_vo_gfx_comm.o
obj-y += $(CBB_VO_ARCH_COMM_SRC)/drv_vo_coef_org_comm.o
obj-y += $(CBB_VO_ARCH_COMM_SRC)/hal_vo_comm.o
obj-y += $(CBB_VO_ARCH_COMM_SRC)/hal_vo_dev_comm.o
obj-y += $(CBB_VO_ARCH_COMM_SRC)/hal_vo_layer_comm.o
obj-y += $(CBB_VO_ARCH_COMM_SRC)/hal_vo_video_comm.o
obj-y += $(CBB_VO_ARCH_COMM_SRC)/hal_vo_gfx_comm.o

#enum vo hal src files
obj-y += $(CBB_VO_ARCH_HAL)/drv_vo.o
obj-y += $(CBB_VO_ARCH_HAL)/drv_vo_dev.o
obj-y += $(CBB_VO_ARCH_HAL)/drv_vo_video.o
obj-y += $(CBB_VO_ARCH_HAL)/drv_vo_gfx.o
obj-y += $(CBB_VO_ARCH_HAL)/hal_vo.o
obj-y += $(CBB_VO_ARCH_HAL)/hal_vo_dev.o
obj-y += $(CBB_VO_ARCH_HAL)/hal_vo_video.o
obj-y += $(CBB_VO_ARCH_HAL)/hal_vo_gfx.o

#enum vo sys hal src files
obj-y += $(CBB_VO_ARCH_HAL)/sys_drv.o
obj-y += $(CBB_VO_ARCH_HAL)/sys_hal.o

ifeq ($(CONFIG_PRODUCTNAME), "hi3519dv500")
ccflags-y += -DCHIP_HI3519DV500
endif

# module macro


# function macro
ifeq ($(CONFIG_PRODUCTNAME), "hi3519dv500")
ccflags-y += -DCONFIG_OT_VO_RGB
ccflags-y += -DCONFIG_OT_VO_MIPI
ccflags-y += -DCONFIG_OT_VO_BT1120
# DCONFIG_OT_VO_CVBS is not set
# DCONFIG_OT_VO_VGA is not set
# CONFIG_OT_VO_HDMI is not set
# CONFIG_OT_VO_MIPI is not set
# CONFIG_OT_VO_HDMI1 is not set
endif
