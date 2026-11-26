# disp-regdump: run SigmaStar's infinity2m display + HDMI TX driver sources
# (the vendor U-Boot's, boot_621_release_20200522 drivers/mstar/{disp,hdmitx})
# against a register recorder, and print the register program a boot logo
# on HDMI needs, per output timing. Build for ARM, run on any ARM Linux
# (the vendor code assumes 32-bit pointers):
#   make BOOT_TGZ=.../boot_621_release_20200522.tar.gz
#   ./regdump 1080p60 > 1080p60.log
BOOT_TGZ ?= /build/shared/mstar/boot_621_release_20200522.tar.gz
CROSS_COMPILE ?= arm-none-linux-gnueabihf-
CC := $(CROSS_COMPILE)gcc
V := build/boot/drivers/mstar
DH := $(V)/disp/hal/infinity2m
HH := $(V)/hdmitx/hal/infinity2m

CFLAGS := -O0 -g -w -fcommon -fgnu89-inline -static -Imock -I. \
	-DCONFIG_ARCH_INFINITY2M -DHDMITX_VGA_SUPPORTED -DHDMITX_OS_TYPE_UBOOT \
	-DCONFIG_SSTAR_DISP -DCONFIG_SSTAR_HDMITX \
	-I$(V)/disp/drv/disp/inc -I$(V)/disp/drv/disp/inc/uboot -I$(V)/disp/drv/disp/pub \
	-I$(V)/disp/drv/midisp/pub -I$(DH)/inc -I$(DH)/pub -I$(V)/disp/hal/pub \
	-I$(V)/hdmitx/drv/hdmitx/inc -I$(V)/hdmitx/drv/hdmitx/inc/uboot \
	-I$(V)/hdmitx/drv/hdmitx/pub -I$(V)/hdmitx/drv/hdmitx/src/api/hdmitx \
	-I$(V)/hdmitx/drv/hdmitx/src/drv/hdmitx/include -I$(V)/hdmitx/drv/mihdmitx/pub \
	-I$(HH)/hdmitx/include -I$(HH)/hdmitx -I$(HH)/hdcp -I$(HH)/pub -I$(HH)/inc \
	-I$(V)/hdmitx/hal/pub -I$(V)/common

DISP_SRC := $(V)/disp/drv/disp/src/uboot/drv_disp_os.c \
	$(V)/disp/drv/disp/src/drv_disp_ctx.c $(V)/disp/drv/disp/src/drv_disp_irq.c \
	$(V)/disp/drv/disp/src/drv_disp_if.c $(V)/disp/drv/midisp/src/mhal_disp.c \
	$(DH)/src/hal_disp_if.c $(DH)/src/hal_disp_irq.c $(DH)/src/hal_disp_scaler_e.c \
	$(DH)/src/hal_disp_pq.c $(DH)/src/hal_disp_color.c $(DH)/src/hal_disp_mop.c \
	$(DH)/src/hal_disp_vga_timing_tbl.c
HDMI_SRC := $(V)/hdmitx/drv/hdmitx/src/uboot/drv_hdmitx_os.c \
	$(V)/hdmitx/drv/hdmitx/src/drv/hdmitx/drvHDMITx.c \
	$(V)/hdmitx/drv/hdmitx/src/api/hdmitx/apiHDMITx.c \
	$(V)/hdmitx/drv/hdmitx/src/drv_hdmitx_if.c $(V)/hdmitx/drv/hdmitx/src/drv_hdmitx_ctx.c \
	$(V)/hdmitx/drv/mihdmitx/src/mhal_hdmitx.c \
	$(HH)/hdmitx/halHDMIUtilTx.c $(HH)/hdmitx/hal_hdmitx_top_tbl.c \
	$(HH)/hdmitx/halHDMITx.c $(HH)/hdmitx/halHDCPTx.c $(HH)/hdcp/halHDCP.c \
	$(HH)/src/hal_hdmitx_if.c

regdump: build/.patched main.c rec.c hdmi_rec.c rec.h
	$(CC) $(CFLAGS) -o $@ main.c rec.c hdmi_rec.c $(DISP_SRC) $(HDMI_SRC)

# the vendor sources, with their register accessors pointed at the recorder
build/.patched: patch.sh
	rm -rf build && mkdir -p build
	tar xzf $(BOOT_TGZ) -C build boot/drivers/mstar/disp boot/drivers/mstar/hdmitx \
		boot/drivers/mstar/common
	sh patch.sh $(V)
	touch $@

clean:
	rm -rf build regdump
