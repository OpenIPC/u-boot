# SPDX-License-Identifier: GPL-2.0+
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy

CFLAGS = -nostdinc -isystem $(shell $(CC) -print-file-name=include) \
	 -ffreestanding -fno-builtin -Wall -Wextra -Os \
	 -fno-delete-null-pointer-checks -marm -march=armv5te \
	 -msoft-float -fno-stack-protector -fno-pie -ffunction-sections

# GCC 12 on: addresses below 4 KiB are real here
CFLAGS += $(shell $(CC) --param=min-pagesize=0 -E -x c /dev/null \
	 -o /dev/null 2>/dev/null && echo --param=min-pagesize=0)

nsboot.bin: nsboot.elf
	$(OBJCOPY) -O binary $< $@

nsboot.elf: start.o main.o ld.script
	$(LD) -T ld.script --gc-sections -o $@ start.o main.o

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
%.o: %.S
	$(CC) $(CFLAGS) -c -o $@ $<
