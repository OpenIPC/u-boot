# SPDX-License-Identifier: GPL-2.0+
#
# The AX620U first stage (SPL), SPI NAND boot, AArch32: clocks, DDR, then
# U-Boot from flash 0x100000 to 0x40000400, entered there (README.md).
#
#   make -C openipc/ax620u-spl O=<dir> CROSS_COMPILE=arm-linux-gnueabi-
#
# gives <dir>/spl.bin, the code the boot ROM loads to on-chip RAM 0x800;
# openipc/ax620u-spl-image.py puts the boot ROM's header in front of it
# and lays out the spl partition.

O ?= .
CROSS_COMPILE ?= arm-linux-gnueabi-
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy

SRC := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

# start.o first (bl1.lds puts .text.start at the entry anyway)
SRCS := board/start.S board/board.c common/cmn.c spl/spl_main.c \
	core/boot/boot.c driver/uart/uart.c driver/timer/timer.c \
	driver/efuse/efuse.c driver/dma/axdma.c driver/spi/spi.c \
	driver/spi/spinand.c driver/ddr/ddr_init.c driver/ddr/ddr_tables.c
OBJS := $(addprefix $(O)/obj/,$(patsubst %.S,%.o,$(SRCS:.c=.o)))

# -O0, as the vendor's build: the DDR PHY and PLL waits are counted loops
# (DELAY() in ddr_init.c), timed by the code -O0 gives. Soft float: the
# code has none, and the hard-float toolchains' libgcc then links too.
CFLAGS := -march=armv7-a -mthumb -mfloat-abi=soft -O0 -std=gnu11 -Wall \
	-Wno-main -ffreestanding -fno-builtin -fno-common -fno-pic \
	-fno-stack-protector -fno-delete-null-pointer-checks \
	-ffunction-sections -fdata-sections -MMD -MP
# Bare metal: the compiler's own headers (stdarg.h), no C library's
CFLAGS += -nostdinc -isystem $(shell $(CC) -print-file-name=include)
CFLAGS += -I$(SRC)/include -I$(SRC)/driver/ddr

# GNU ld 2.39+ warns about the single RWX segment; older ld lacks the flag
LDFLAGS_RWX := $(shell $(LD) --no-warn-rwx-segments --version >/dev/null 2>&1 && \
	echo -Wl,--no-warn-rwx-segments)
LDFLAGS := -nostdlib -static -Wl,-T,$(SRC)/board/bl1.lds -Wl,--gc-sections \
	-Wl,--build-id=none $(LDFLAGS_RWX) -Wl,-Map,$(O)/spl.map

all: $(O)/spl.bin

$(O)/spl.bin: $(O)/spl.elf
	$(OBJCOPY) -O binary -R .bss -R .pen $< $@

$(O)/spl.elf: $(OBJS) $(SRC)/board/bl1.lds
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -lgcc -o $@

$(O)/obj/%.o: $(SRC)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/obj/%.o: $(SRC)/%.S
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -marm -c $< -o $@

# The DDR tables are data copied out of the vendor first stage (stock.bin):
# regenerate them with `make tables`; the result is in the tree.
tables:
	python3 $(SRC)/tools/gen_ddr_tables.py $(SRC)/stock.bin \
		$(SRC)/driver/ddr/ddr_tables.c

clean:
	rm -rf $(O)/obj $(O)/spl.elf $(O)/spl.bin $(O)/spl.map

-include $(OBJS:.o=.d)

.PHONY: all clean tables
