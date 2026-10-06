################################################################################
# Boot0 (mini-boot) for Hi3518EV100, ARM926EJS
################################################################################
PWD                = $(shell pwd)
HW_CROSS_COMPILE   = $(CROSS_COMPILE)
BOOT_TOPDIR       ?= $(PWD)/../../../../../../
BOOT_OUTDIR       ?= $(BOOT_TOPDIR)
LZMA              ?= lzma -fkzc -7
SOC               ?= hi3518ev100
TEXTBASE          ?= 0x80700000

CC       := $(HW_CROSS_COMPILE)gcc
LD       := $(HW_CROSS_COMPILE)ld
OBJCOPY  := $(HW_CROSS_COMPILE)objcopy
OBJDUMP  := $(HW_CROSS_COMPILE)objdump

################################################################################
BOOT     := u-boot-$(SOC)

CFLAGS   := -O2 -g -fno-builtin -ffreestanding \
	-D__KERNEL__ -DTEXT_BASE=$(TEXTBASE) \
	-I$(BOOT_TOPDIR)/include \
	-I$(BOOT_OUTDIR)/include \
	-I$(BOOT_TOPDIR)/arch/arm/include \
	-I$(BOOT_TOPDIR)/arch/arm/cpu/arm926ejs/$(SOC) \
	-fno-pic -fno-common -ffixed-r8 \
	-pipe -march=armv5te -marm -mno-thumb-interwork -mabi=aapcs-linux \
	-mno-unaligned-access -msoft-float -fno-aggressive-loop-optimizations \
	-Wall -Wstrict-prototypes -fno-stack-protector \
	-D__LINUX_ARM_ARCH__=5 -D__ARM__ -DCONFIG_ARM \
	-include $(BOOT_TOPDIR)/include/linux/kconfig.h \
	-fno-strict-aliasing

UBOOT_Z_LDS := u-boot.lds

################################################################################
START := start.o
COBJS := uart.o timer.o mmu.o cache-cp15.o div0.o _divsi3.o _udivsi3.o reset.o \
	startup.o image_data.o

SSRC  := arch/arm/cpu/arm926ejs/$(SOC)/start.S \
	arch/arm/cpu/arm926ejs/$(SOC)/uart.S \
	arch/arm/cpu/arm926ejs/$(SOC)/timer.c \
	arch/arm/cpu/arm926ejs/$(SOC)/mmu.S \
	arch/arm/cpu/arm926ejs/$(SOC)/cache-cp15.c \
	arch/arm/cpu/arm926ejs/$(SOC)/div0.c \
	arch/arm/cpu/arm926ejs/$(SOC)/_divsi3.S \
	arch/arm/cpu/arm926ejs/$(SOC)/_udivsi3.S \
	arch/arm/cpu/arm926ejs/$(SOC)/reset.S \
	arch/arm/cpu/arm926ejs/$(SOC)/hw_compressed/startup.c \
	arch/arm/cpu/arm926ejs/$(SOC)/hw_compressed/image_data.S

SRC := $(notdir $(SSRC))

REG := $(wildcard *.reg .reg)
IMAGE_DATA_Z := image_data.lzma

# reg_info is spliced at offset 64 padded to a 4K boundary; the U-Boot code
# must stay at its linked offset, so the skip depends on the reg_info size.
REG_SIZE := $(shell stat -c%s $(REG) 2>/dev/null || echo 4096)
REG_PAD  := $(shell echo $$(( ($(REG_SIZE) + 4095) / 4096 * 4096 )))
REG_SKIP := $(shell echo $$(( 64 + $(REG_PAD) )))

################################################################################
.PHONY: all $(BOOT).bin
all: $(BOOT).bin
$(BOOT).bin: $(BOOT).tmp regfile
	$(Q)dd if=$(BOOT).tmp of=./tmp1 bs=1 count=64 2>/dev/null
	$(Q)dd if=$(REG) of=./tmp2 bs=4096 conv=sync 2>/dev/null
	$(Q)dd if=$(BOOT).tmp of=./tmp3 bs=1 skip=$(REG_SKIP) 2>/dev/null
	$(Q)cat tmp1 tmp2 tmp3 > $(BOOT).bin
	$(Q)rm -f tmp1 tmp2 tmp3
	$(Q)chmod 754 $(BOOT).bin
	$(Q)cp -fv $@ $(BOOT_OUTDIR)
	$(Q)echo $(BOOT).bin is Ready.

$(BOOT).tmp: $(BOOT).elf
	$(OBJCOPY) -O srec $< $(BOOT).srec
	$(OBJCOPY) -j .text -O binary $< $(BOOT).text
	$(OBJCOPY) --gap-fill=0xff -O binary $< $@

$(BOOT).elf: $(IMAGE_DATA_Z) $(SRC) $(START) $(COBJS)
	$(LD) -Bstatic -T $(UBOOT_Z_LDS) -Ttext $(TEXTBASE) $(START) \
		$(COBJS) -Map $(BOOT).map -o $@
	$(OBJDUMP) -d $@ > $@.asm

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

$(IMAGE_DATA_Z): $(BINIMAGE)
	$(LZMA) $< > $@

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

TMPS := $(COBJS) $(START) $(SRC) \
	$(BOOT).map $(BOOT).elf $(BOOT).elf.asm $(BOOT).srec $(BOOT).bin $(BOOT).text $(BOOT).tmp \
	$(IMAGE_DATA_Z)

clean:
	-rm -f $(TMPS)
	-rm -f $(REG)
