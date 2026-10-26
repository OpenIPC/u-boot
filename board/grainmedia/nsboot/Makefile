# Makefile for nandboot image
#
# Written by: 
#		Harry Hsu <harry_hs@faraday-tech.com>
#
# Copyright (c) 2009 FARADAY, ALL Rights Reserve.
#
DODEBUG = 

TARGET = nsboot

# These two variables control where the zboot image is 
# stored in flash and loaded in memory.
#
TEXT_START_ADDR = 0x92000000
#TEXT_START_ADDR = 0x08000000

HEAD = head.S
HEADOBJ = $(patsubst %.S,%.o, $(HEAD))

SSRC =
SOBJS = $(patsubst %.S,%.o, $(SSRC))

CSRC = serial.c kprint.c string.c ahb_dma.c entry.c spi_flash.c spi020.c
COBJS = $(patsubst %.c,%.o, $(CSRC))

LINK_SCRIPT = ld.script

#CROSS_COMPILE = arm-linux-
#CROSS_COMPILE = /usr/src/arm-linux-3.3/toolchain_gnueabi-4.4.0_ARMv5TE/usr/bin/arm-unknown-linux-uclibcgnueabi-

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
NM = $(CROSS_COMPILE)nm
OBJCOPY = $(CROSS_COMPILE)objcopy

CFLAGS = -Wall -O2 -ffunction-sections -fdata-sections -fno-exceptions -fno-builtin\
	 -march=armv4 -msoft-float -fno-stack-protector -fno-pie

LDFLAGS = -Wl,-T,$(LINK_SCRIPT) -Wl,-Ttext,$(TEXT_START_ADDR) -Wl,--gc-sections -nostartfiles

INCLUDE_PATH = -I./include

PREDEFINE =

ifeq ($(DODEBUG),y)
CFLAGS += -g
endif
#
# MEM_TEST, PLL3_594, DDR_SZ_8136_2Gbx1, DDR_SZ_8136_1Gbx1, DDR_SZ_8136_512Mbx1, DDR_SZ_8135_512Mbx1
#

#=== 8136 ==================================
#CPU 885, default setting
#MFLAGS = -DSYSTEM_EVB -DGM8136 -DDDR_1180 -DMODE_3 -DDDR_SZ_8136_2Gbx1 #-DPLL3_594

#CPU 860
#MFLAGS = -DSYSTEM_EVB -DGM8136 -DDDR_1140 -DMODE_5 -DDDR_SZ_8136_2Gbx1

#CPU 600 for test
#MFLAGS = -DSOCKET_EVB -DGM8136 -DDDR_800 -DMODE_1 -DDDR_SZ_8136_2Gbx1
#CPU 712 for test
#MFLAGS = -DSOCKET_EVB -DGM8136 -DDDR_800 -DMODE_4 -DDDR_SZ_8136_2Gbx1
#CPU 500 for test
#MFLAGS = -DSOCKET_EVB -DGM8136 -DDDR_1000 -DMODE_0 -DDDR_SZ_8136_2Gbx1
#CPU 762 for test
#MFLAGS = -DSOCKET_EVB -DGM8136 -DDDR_1000 -DMODE_6 -DDDR_SZ_8136_2Gbx1
#CPU 810 for test
#MFLAGS = -DSOCKET_EVB -DGM8136 -DDDR_1080 -DNORMAL -DDDR_SZ_8136_2Gbx1
#MFLAGS = -DSYSTEM_EVB -DGM8136 -DDDR_1080 -DNORMAL -DDDR_SZ_8136_2Gbx1

#=== 8136S ==================================
#CPU 885, default setting
#MFLAGS = -DGM8136S -DDDR_1180 -DMODE_4 -DDDR_SZ_8136_1Gbx1

#CPU 860
#MFLAGS = -DGM8136S -DDDR_1140 -DMODE_3 -DDDR_SZ_8136_1Gbx1

#CPU 762, DDR 1016
#MFLAGS = -DGM8136S -DDDR_1000 -DMODE_0 -DDDR_SZ_8136_1Gbx1

#CPU 712
#MFLAGS = -DGM8136S -DDDR_950 -DMODE_7 -DDDR_SZ_8136_1Gbx1

#CPU 590
#MFLAGS = -DGM8136S -DDDR_790 -DMODE_2 -DDDR_SZ_8136_1Gbx1

#CPU 442
#MFLAGS = -DGM8136S -DDDR_590 -DMODE_8 -DDDR_SZ_8136_1Gbx1

#CPU 600 for test
#MFLAGS = -DGM8136S -DDDR_800 -DMODE_1 -DDDR_SZ_8136_1Gbx1
#CPU 810 for test
#MFLAGS = -DGM8136S -DDDR_1080 -DNORMAL -DDDR_SZ_8136_1Gbx1
#MFLAGS = -DGM8136S -DDDR_1400 -DMODE_9 -DDDR_SZ_8136_1Gbx1

#=== 8135S ==================================
#CPU 712, default setting
#MFLAGS = -DGM8135S -DDDR_950 -DMODE_4 -DDDR_SZ_8135_512Mbx1

#CPU 590
#MFLAGS = -DGM8135S -DDDR_790 -DMODE_1 -DDDR_SZ_8135_512Mbx1

#CPU 442
#MFLAGS = -DGM8135S -DDDR_590 -DMODE_5 -DDDR_SZ_8135_512Mbx1

#CPU 375
#MFLAGS = -DGM8135S -DDDR_590 -DMODE_6 -DDDR_SZ_8135_512Mbx1

#CPU 337
#MFLAGS = -DGM8135S -DDDR_450 -DMODE_7 -DDDR_SZ_8135_512Mbx1

#CPU 300
#MFLAGS = -DGM8135S -DDDR_450 -DMODE_8 -DDDR_SZ_8135_512Mbx1

#CPU 260
#MFLAGS = -DGM8135S -DDDR_450 -DMODE_9 -DDDR_SZ_8135_512Mbx1

#CPU 760 for test
#MFLAGS = -DGM8135S -DDDR_1000 -DMODE_2 -DDDR_SZ_8135_512Mbx1 #-DMODE_SPECIAL
#CPU 500 for test
#MFLAGS = -DGM8135S -DDDR_1000 -DMODE_0 -DDDR_SZ_8135_512Mbx1
#CPU 860 for test
#MFLAGS = -DGM8135S -DDDR_1140 -DMODE_3 -DDDR_SZ_8135_512Mbx1
#CPU 810 for test
#MFLAGS = -DGM8135S -DDDR_1080 -DNORMAL -DDDR_SZ_8135_512Mbx1
#CPU 810 for test
#MFLAGS = -DGM8135S -DDDR_1080 -DMODE_CPU -DDDR_SZ_8135_512Mbx1

#============================================
#debug
#MFLAGS = -DSOCKET_EVB -DGM8136 -DDDR_800 -DUSE_SCP -DDDR_SZ_8136_1Gbx1 -DMEM_TEST
#MFLAGS = -DSYSTEM_EVB -DGM8136 -DDDR_1333 -DMODE_2 -DDDR_SZ_8136_2Gbx1
#MFLAGS = -DGM8136S -DDDR_1240 -DMODE_5 -DDDR_SZ_8136_1Gbx1
#MFLAGS = -DGM8136S -DDDR_1333 -DMODE_6 -DMODE_CPU -DDDR_SZ_8136_1Gbx1

#============================================
# One nsboot for all three packages: package.c is built once per package
# with that package's flags -- the default setting of each list above --
# and entry.c picks the one matching the chip at run time. MFLAGS is left
# for the debug options (MEM_TEST, USE_SCP, ...).
MFLAGS =
MFLAGS_gm8136 = -DSYSTEM_EVB -DGM8136 -DDDR_1180 -DMODE_3 -DDDR_SZ_8136_2Gbx1
MFLAGS_gm8136s = -DGM8136S -DDDR_1180 -DMODE_4 -DDDR_SZ_8136_1Gbx1
MFLAGS_gm8135s = -DGM8135S -DDDR_950 -DMODE_4 -DDDR_SZ_8135_512Mbx1
PKGOBJS = package_gm8136.o package_gm8136s.o package_gm8135s.o

CFLAGS += $(INCLUDE_PATH) $(PREDEFINE) $(MFLAGS)
ASFLAGS = $(CFLAGS) -D__ASSEMBLY__ $(MFLAGS)

GCCLIB = $(shell $(CC) $(CFLAGS) --print-libgcc-file-name)

OBJS = $(SOBJS) $(COBJS) $(PKGOBJS)
LIBS = $(GCCLIB)

all: $(TARGET)

$(TARGET): $(HEADOBJ) $(OBJS)
	$(LD) -O2 -G 0 --gc-sections -T $(LINK_SCRIPT) -Ttext $(TEXT_START_ADDR) \
		$(HEADOBJ) $(OBJS) $(LIBS) -o $@
	$(OBJCOPY) -O binary $@ $@.bin
	$(NM) $@ | grep -v '\(compiled\)\|\(\.o$$\)\|\( [aUw] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)' | sort > System.map

clean:
	rm -f *.o *.map *.bak $(TARGET)*

$(SOBJS): %.o : %.S
	$(CC) $(ASFLAGS) -c $< -o $@

$(PKGOBJS): package_%.o : package.c
	$(CC) $(CFLAGS) $(MFLAGS_$*) -DPACKAGE=$*_package \
		-DDDR_arg=$*_ddr_arg -DDDR_arg_AR=$*_ddr_arg_ar -c $< -o $@

$(COBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(HEADOBJ): $(HEAD)
	$(CC) $(ASFLAGS) -c $< -o $@
