################################################################################
PWD                = $(shell pwd)
HW_CROSS_COMPILE   = $(CROSS_COMPILE)
BOOT_TOPDIR       ?= $(PWD)/../../../../../../
BOOT_OUTDIR       ?= $(BOOT_TOPDIR)
GZIP              ?= $(if $(wildcard $(SRCDIR)/gzip), $(SRCDIR)/gzip, $(BOOT_TOPDIR)/../../../tools/utils/bin/gzip)
SRCDIR            := $(BOOT_TOPDIR)/arch/arm/cpu/armv7/$(SOC)/hw_compressed

################################################################################
CC       := $(HW_CROSS_COMPILE)gcc
AR       := $(HW_CROSS_COMPILE)ar
LD       := $(HW_CROSS_COMPILE)ld
OBJCOPY  := $(HW_CROSS_COMPILE)objcopy


################################################################################
BOOT     := u-boot-$(SOC)
TEXTBASE := 0x80700000

CFLAGS   :=-Os -fno-builtin -ffreestanding \
	-D__KERNEL__ -DTEXT_BASE=$(TEXTBASE) \
	-I$(BOOT_TOPDIR)/include \
	-I$(BOOT_OUTDIR)/include \
	-I$(BOOT_TOPDIR)/drivers/ddr/hisilicon/default \
	-I$(BOOT_TOPDIR)/drivers/ddr/hisilicon/$(SOC) \
	-I$(BOOT_TOPDIR)/lib/hw_dec/hisi \
	-I$(BOOT_TOPDIR)/lib/hw_dec \
	-I$(BOOT_TOPDIR)/arch/arm/include \
	-fno-pic -ffunction-sections \
	-fdata-sections -fno-common -ffixed-r9    \
	-fno-common -pipe -march=armv7-a \
	-Wall -Wstrict-prototypes -fno-stack-protector \
	-D__LINUX_ARM_ARCH__=7 -D__ARM__ \
	-include $(BOOT_TOPDIR)/include/linux/kconfig.h \
	$(MKFLAGS) -fno-strict-aliasing

UBOOT_Z_LDS := u-boot.lds

################################################################################

START := start.o
COBJS := lowlevel_init_v300.o \
	init_registers.o \
	uart.o \
	ddr_training_impl.o \
	ddr_training_ctl.o \
	ddr_training_boot.o \
	ddr_training_custom.o \
	ddr_training_console.o \
	pll_trainning.o \
	startup.o \
	image_data.o \
	reset.o \
	emmc_boot.o \
	div0.o

SSRC  := arch/arm/cpu/armv7/$(SOC)/start.S \
	arch/arm/cpu/armv7/$(SOC)/reset.S \
	arch/arm/cpu/armv7/$(SOC)/uart.S \
	arch/arm/cpu/armv7/$(SOC)/init_registers.c \
	arch/arm/cpu/armv7/$(SOC)/lowlevel_init_v300.c \
	arch/arm/cpu/armv7/$(SOC)/pll_trainning.c \
	arch/arm/cpu/armv7/$(SOC)/emmc_boot.c \
	drivers/ddr/hisilicon/default/ddr_training_impl.c \
	drivers/ddr/hisilicon/default/ddr_training_ctl.c \
	drivers/ddr/hisilicon/default/ddr_training_boot.c \
	drivers/ddr/hisilicon/default/ddr_training_console.c \
	drivers/ddr/hisilicon/$(SOC)/ddr_training_custom.c \
	lib/hw_dec/hisi/hw_decompress_hisi.c \
	lib/hw_dec/hisi/hw_decompress_hi3516cv500.c \
	lib/hw_dec/hisi/hw_decompress_hi3516dv300.c \
	lib/hw_dec/hisi/hw_decompress_hisi_v1.c \
	lib/hw_dec/hisi/hw_decompress_hisi_v1.h \
	arch/arm/lib/div0.c

SRC := $(notdir $(SSRC))

REG := $(wildcard *.reg .reg)
IMAGE_DATA_Z := image_data.gzip
################################################################################
.PHONY: all $(BOOT).bin
all: $(BOOT).bin
$(BOOT).bin: $(BOOT).tmp regfile
	$(Q)dd if=$(BOOT).tmp of=./tmp1 bs=1 count=64 2>/dev/null
	$(Q)dd if=$(REG) of=./tmp2 bs=8192 conv=sync 2>/dev/null
	$(Q)dd if=$(BOOT).tmp of=./tmp3 bs=1 skip=8256 2>/dev/null
	$(Q)cat tmp1 tmp2 tmp3 > $(BOOT).bin
	$(Q)rm -f tmp1 tmp2 tmp3
	$(Q)chmod 754 $(BOOT).bin
	$(Q)cp -fv $@ $(BOOT_OUTDIR)
	$(Q)echo $(BOOT).bin is Ready.

$(BOOT).tmp: $(BOOT).elf
	$(OBJCOPY) -O srec $< $(BOOT).srec
	$(OBJCOPY) -j .text -O binary $< $(BOOT).text
	$(OBJCOPY) --gap-fill=0xff -O binary $< $@

$(BOOT).elf: $(IMAGE_DATA_Z) $(SRC) $(START) $(COBJS) $(UBOOT_Z_LDS)
	$(LD) -Bstatic -T $(UBOOT_Z_LDS) -Ttext $(TEXTBASE) $(START) \
		$(COBJS) -Map $(BOOT).map -o $@
	$(OBJDUMP) -d  $@ > $@.asm

.PHONY: regfile
regfile:
	@if [ "$(words $(REG))" = "0" ]; then ( \
		echo '***' Need '.reg' or '*.reg' file in directory $(PWD); \
		exit 1; \
	) fi
	@if [ "$(words $(REG))" != "1" ]; then ( \
		echo '***' Found multi '.reg' or '*.reg' file in directory $(PWD); \
		echo '***' Files: $(notdir $(REG)); \
		exit 1; \
	) fi

################################################################################
$(START): start.S
	$(CC) -D__ASSEMBLY__ $(CFLAGS) -o $@ $< -c

# -1 : --fast      -9 : --best
$(IMAGE_DATA_Z): $(BINIMAGE)
	$(GZIP) -fNqc -7 $< > $@

%.o: %.c
	$(CC) $(CFLAGS) -Wall -Wstrict-prototypes \
		-fno-stack-protector -o $@ $< -c

%.o: %.S
	$(CC) -D__ASSEMBLY__ $(CFLAGS) -o $@ $< -c

image_data.o: image_data.S $(IMAGE_DATA_Z)
	$(CC) -D__ASSEMBLY__ $(CFLAGS) -o $@ $< -c

#############################################################################

$(SRC):
	ln -sf $(BOOT_TOPDIR)/$(filter %/$@,$(SSRC)) $@
################################################################################
TMPS := $(COBJS) $(START) $(SRC) \
	$(BOOT).map $(BOOT).elf $(BOOT).elf.asm $(BOOT).srec $(BOOT).bin $(BOOT).text $(BOOT).tmp \
	$(IMAGE_DATA_Z)

.PHONY: clean distclean

distclean: clean
clean:
	[[ -n "$(REG)" && "$(REG)" != "*" ]] && rm -f $(REG)
	[[ -n "$(TMPS)" && "$(TMPS)" != "*" ]] && rm -f $(TMPS)
	[[ -n "$(SRC)" && "$(SRC)" != "*" ]] && rm -f $(addprefix $(SRCDIR), $(SRC))

################################################################################
.PHONY: clean
FORCE:
################################################################################
