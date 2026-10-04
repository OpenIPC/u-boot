obj-y += jpegd_entry.o
obj-y += jpegd.o
obj-y += jpegd_drv.o
obj-y += jpegd_image.o
obj-y += jpegd_comm.o
ifeq ($(CONFIG_PRODUCTNAME), "hi3519dv500")
obj-y += jpegd_msg.o
ccflags-y += -DMSG_LOAD_ENABLE
endif

ccflags-y  += -I$(srctree)/product/ot_osd/include
ifeq ($(CONFIG_PRODUCTNAME), "hi3519dv500")
ccflags-y  += -DCHIP_HI3519DV500
endif
