include cfg.mk

export CROSS_COMPILE ?= arm-v01c02-linux-musleabi-
ifneq ($(LLVM),)
export CC := clang
export LD := ld.lld
export AS := clang
export AR := llvm-ar
export NM := llvm-nm
export STRIP := llvm-strip
export OBJCOPY := llvm-objcopy
export OBJDUMP := llvm-objdump
export READELF := llvm-readelf
export RANLIB := llvm-ranlib
else
export CC := $(CROSS_COMPILE)gcc
export AR := $(CROSS_COMPILE)ar
export RANLIB := $(CROSS_COMPILE)ranlib
export LD := $(CROSS_COMPILE)ld
export OBJCOPY := $(CROSS_COMPILE)objcopy
export OBJDUMP := $(CROSS_COMPILE)objdump
endif
CSRC  = $(wildcard boot/*.c wildcard common/*.c)

ifdef CFG_DEBUG_INFO
CSRC  += $(wildcard debug/*.c)
endif

SSRC  = $(wildcard boot/*.S)

ifdef CFG_DEBUG_INFO
SSRC  += $(wildcard debug/*.S)
endif

OBJS := $(patsubst %.c,%.o,$(CSRC) )
OBJS += $(patsubst %.S,%.o,$(SSRC) )

export CFLAGS := -fno-builtin -fno-common -fdata-sections -ffunction-sections
CFLAGS += -mthumb -mno-unaligned-access -g
CFLAGS += -Wall
CFLAGS += -I$(PWD)/include/
CFLAGS += -I$(PWD)/drivers/share_drivers/
CFLAGS += -I$(PWD)/drivers/share_drivers/include/
CFLAGS += -I$(PWD)/drivers/uart/

ifdef CFG_SM_SUPPORT
CFLAGS += -DCFG_SM_SUPPORT
endif

ifdef CFG_DEBUG_INFO
CFLAGS += -DCFG_DEBUG_INFO
endif

ifdef DDR_SCRAMB_ENABLE
CFLAGS += -DDDR_SCRAMB_ENABLE
endif

CFLAGS += -DCFG_RAM_BASE_ADDR=$(RAM_BASE)

ifneq ($(LLVM),)
CFLAGS += -DLLVM_COMPILER
endif

CFLAGS += -mcpu=cortex-a7+nofp

CFLAGS += -c

CFLAGS += -Os
CFLAGS += -Os -fomit-frame-pointer -fno-exceptions -fno-asynchronous-unwind-tables -fno-unwind-tables -fstack-protector-strong

ifdef CFG_EDA_VERIFY
CFLAGS += -DCFG_EDA_VERIFY
endif

ifdef CFG_FPGA_VERIFY
CFLAGS += -DCFG_FPGA_VERIFY
endif

ifeq ($(CONFIG_GSL_FMC_READ),1)
CFLAGS += -DCONFIG_GSL_FMC_READ
endif

TARGET = gsl
BIN_PATH = pub

LDSCRIPT := linker.lds
LDFLAGS := -Bstatic -T $(LDSCRIPT) -Ttext $(TEXT_BASE) --gc-sections
LDFLAGS += drivers/libdrv.a

.SILENT:

.DEFAULT_GOAL := all

all:
	make -C drivers/
	make -C boot/
	make -C common/
ifdef CFG_DEBUG_INFO
	make -C debug/
endif
	sed -e 's/TEXT_BASE/$(TEXT_BASE)/' \
		-e 's/TEXT_SIZE/$(TEXT_SIZE)/' \
		-e 's/DDR_BASE/$(DDR_BASE)/' \
		-e 's/DDR_SIZE/$(DDR_SIZE)/' \
		-e 's/RAM_BASE/$(RAM_BASE)/' \
		$(LDSCRIPT).mk > $(LDSCRIPT)
	echo "  $(LD) $(OBJS) $(LDFLAGS) -Map $(TARGET).map -o $(TARGET) $(OBJCOPY) -O binary $(TARGET) $(TARGET).bin"
	mkdir -p $(BIN_PATH)
	$(LD) $(OBJS) $(LDFLAGS) -Map $(BIN_PATH)/$(TARGET).map -o $(BIN_PATH)/$(TARGET)
	$(OBJCOPY) -O binary $(BIN_PATH)/$(TARGET) $(BIN_PATH)/$(TARGET).bin
	$(OBJDUMP) -D $(BIN_PATH)/$(TARGET) > $(BIN_PATH)/$(TARGET).asm

clean:
	make -C drivers/ clean
	make -C boot/ clean
	make -C common/ clean
ifdef CFG_DEBUG_INFO
	make -C debug clean
endif
	rm -rf $(BIN_PATH)
	rm -rf $(LDSCRIPT)

