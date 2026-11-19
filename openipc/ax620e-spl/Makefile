# SPDX-License-Identifier: GPL-2.0+
#
# The AX620E first stage (the SDK's BL1 "spl"), SPI NOR boot, AArch64,
# as the AX620E Xiongmai cameras run it: DDR (the eFuse chip type picks
# the LPDDR4 part), then U-Boot from flash 0x10000, unpacked by GZIPD to
# 0x5C000400 and entered there in EL1. Replaces the SDK's build/ and
# boot/bl1/spl/Makefile, which need the whole SDK tree; the values below
# are the ones those take from build/projects/AX620Q_nor_arm32_k419
# (project.mak) for this layout, with ARCH=arm64.
#
#   make -C openipc/ax620e-spl O=<dir> CROSS_COMPILE=aarch64-linux-gnu-
#
# gives <dir>/spl.bin, the code the boot ROM loads to 0x03000400;
# openipc/ax620e-image.py puts the boot ROM's header in front of it.

O ?= .
CROSS_COMPILE ?= aarch64-linux-gnu-
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy

SRC := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))/bl1

# The SDK's spl/Makefile object list for FLASH_TYPE=nor, no ATF/RISC-V;
# start.o first (bl1.lds puts *start.o at the entry)
SRCS := board/arch/arm64/start.S board/board.c board/arch/arm64/stack.S \
	common/cmn.c board/meminit.c spl/spl_main.c \
	driver/uart/uart.c driver/timer/timer.c driver/axdma/axdma_checksum.c \
	driver/secure/efuse_drv.c driver/secure/eip130_drv.c driver/adc/ax_adc.c \
	driver/ddr/ddr_init.c driver/ddr/ddrmc_train_flow_lp4.c \
	driver/ddr/ddrmc_train_flow_no_lp.c driver/ddr/ddr_scan.c \
	driver/usb/printf.c core/trace/trace.c driver/secure/secure.c \
	driver/atf/atf.c driver/pwm/pwm.c driver/gzipd/ax_gzipd_drv.c \
	core/boot/boot.c driver/spi/spinor.c driver/spi/dw_spi_mst.c \
	driver/dmaper/dmaper.c
OBJS := $(addprefix $(O)/obj/,$(patsubst %.S,%.o,$(SRCS:.c=.o)))

CFLAGS := -std=gnu11 -Os -Wall -Wno-array-bounds -ffreestanding -fno-builtin \
	-fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables \
	-fomit-frame-pointer -fno-stack-protector -fno-pic -fno-pie \
	-mstrict-align -mgeneral-regs-only -MMD -MP
CFLAGS += $(addprefix -I$(SRC)/,driver/include board/include common/include \
	core/include driver/usb driver/ddr/ddr driver/adc core/ax_env)
CFLAGS += -DAX620E_CFG=1 -DCONFIG_ARM64=1
# FLASH_TYPE=nor; SUPPPORT_GZIPD=TRUE (the U-Boot image is packed);
# SPINOR_SINGLE_IO: 0x03 reads on one line, as the stock first stage
CFLAGS += -DAX620E_NOR=1 -DSUPPPORT_GZIPD -DSPINOR_SINGLE_IO
# The image layout: U-Boot's header at flash 0x10000 (no backup copy),
# loaded to 0x5C000000; the rest is what spl/Makefile always defines
CFLAGS += -DUBOOT_HEADER_FLASH_BASE=0x10000 -DUBOOT_HEADER_BAK_FLASH_BASE=0x0 \
	-DUBOOT_IMG_HEADER_BASE=0x5C000000 -DDDRINIT_PARAM_HEADER_BASE=0x03200000 \
	-DATF_IMG_HEADER_BASE=0x4003FC00 -DATF_HEADER_FLASH_BASE=0x0 \
	-DATF_HEADER_BAK_FLASH_BASE=0x0 -DDTB_IMG_HEADER_ADDR=0x42007C00 \
	-DDTB_HEADER_FLASH_BASE=0x0 -DDTB_HEADER_BAK_FLASH_BASE=0x0 \
	-DKERNEL_IMG_HEADER_ADDR=0x40007C00 -DKERNEL_HEADER_FLASH_BASE=0x0 \
	-DKERNEL_HEADER_BAK_FLASH_BASE=0x0 -DRAMDISK_START_ADDR=0x0 \
	-DRAMDISK_FLASH_POS=0x0 -DRAWDATA_FLASH_POS=0x0

all: $(O)/spl.bin

$(O)/spl.bin: $(O)/spl.elf
	$(OBJCOPY) -O binary $< $@

$(O)/spl.elf: $(OBJS) $(SRC)/board/arch/arm64/bl1.lds
	$(LD) --entry=_start -T $(SRC)/board/arch/arm64/bl1.lds \
		--no-warn-rwx-segments -Map $(O)/spl.map $(OBJS) -o $@ \
		$$($(CC) -print-libgcc-file-name)

$(O)/obj/%.o: $(SRC)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/obj/%.o: $(SRC)/%.S
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(O)/obj $(O)/spl.elf $(O)/spl.bin $(O)/spl.map

-include $(OBJS:.o=.d)

.PHONY: all clean
