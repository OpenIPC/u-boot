include ../cfg.mk

CSRC  = $(wildcard *.c)
CSRC  += $(wildcard common/*.c)
CSRC  += $(wildcard otp/*.c)
CSRC  += $(wildcard securelib/*.c)
CSRC  += $(wildcard uart/*.c)
CSRC  += $(wildcard tzasc/*.c)
CSRC  += $(wildcard share_drivers/*.c)
CSRC += $(wildcard ddr_init/*.c)
CSRC += $(wildcard ddr_init/boot_adaptation/common/*.c)
CSRC += $(wildcard ddr_init/hal/*.c)
CSRC += $(wildcard ddr_init/hal/chip/hi3516cv610/*.c)
CSRC += $(wildcard ddr_init/hal/ddrc/common/*.c)
CSRC += $(wildcard ddr_init/hal/ddrc/*.c)
CSRC += $(wildcard ddr_init/hal/ddrt/common/*.c)
CSRC += $(wildcard ddr_init/hal/ddrt/*.c)
CSRC += $(wildcard ddr_init/hal/phy/common/*.c)
CSRC += $(wildcard ddr_init/hal/phy/*.c)
CSRC += $(wildcard ddr_init/hal/uart/*.c)
CSRC += $(wildcard ddr_init/hal/sysctrl/*.c)
CSRC += $(wildcard ddr_init/training/*.c)
CSRC += $(wildcard ddr_init/api/*.c)

ifeq ($(CFG_SM_SUPPORT),1)
include $(PWD)/drivers/secure_driver/build.mak
endif

# Flash FMC driver source files
ifeq ($(CONFIG_GSL_FMC_READ),1)
CSRC  += $(wildcard fmc/spi/*.c)
CSRC  += fmc/spi/fmc100/fmc100.c
CSRC  += fmc/spi/fmc100/fmc100_os.c
CSRC  += fmc/spi/fmc100/fmc_spi_nor_ids.c
endif

OBJS := $(patsubst %.c,%.o,$(CSRC) )

#.SILENT:

CFLAGS += -I./
CFLAGS += -I./otp/
CFLAGS += -I./ddr_init/
CFLAGS += -I./ddr_init/boot_adaptation/common/
CFLAGS += -I./ddr_init/include/
CFLAGS += -I./ddr_init/include/hal/
CFLAGS += -I./ddr_init/include/hal/chip/hi3516cv610/
CFLAGS += -I./ddr_init/include/hal/ddrc/
CFLAGS += -I./ddr_init/include/hal/ddrt/
CFLAGS += -I./ddr_init/include/hal/phy/
CFLAGS += -I./ddr_init/include/hal/uart/
CFLAGS += -I./ddr_init/include/training/
CFLAGS += -I./uart/
ifeq ($(CFG_SM_SUPPORT),1)
CFLAGS += -I./secure_driver/include/common_include/
endif
CFLAGS += -I./tzasc/
CFLAGS += -I./share_drivers/include/
ifeq ($(CONFIG_GSL_FMC_READ),1)
CFLAGS += -I./fmc/include
CFLAGS += -I./fmc/include/arch/arm/include
endif

default: libdrv.a

libdrv.a: $(OBJS)
	echo "  AR    libdrv.a"
	$(AR) r libdrv.a $(OBJS)
	echo "  RL    $@"
	$(RANLIB) $@

%.o: %.c
	$(CC) -c $< $(CFLAGS) -o $@
#.c.o:
#	echo "  CC    $<"
#	$(CC) $(CFLAGS) $<

clean:
	find . -name '*.o' -type f | xargs rm -f
	find . -name '*.a' -type f | xargs rm -f
