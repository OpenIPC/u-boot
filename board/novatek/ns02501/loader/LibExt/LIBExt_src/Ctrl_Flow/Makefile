#----------------------------------------------------------------------
# include make config file
#----------------------------------------------------------------------
include ../../../Project/Model/ModelConfig.txt
include ../../../Project/Model/ModelConfig_$(MODEL).txt
include ../../../Project/Model/MakeConfig.txt

#----------------------------------------------------------------------
# set the library name here
#----------------------------------------------------------------------
PRJ_NAME = Ctrl_Flow


#----------------------------------------------------------------------
# add/delete source files here
#----------------------------------------------------------------------
SRC = \
	main.c \
	nand_ids.c \
	spi_nor_ids.c \
	time_util.c \
	bl_u2.c\
	bl_lowlevel_init.c \
	bl_func.c \
	bl_linux.c \
	arch_timer.c \
	bl_lzma.c \
	LzmaDec.c \

ASM =

#----------------------------------------------------------------------
# set the include directory here
#----------------------------------------------------------------------
INC_DIR = . \
	../../../Include

#----------------------------------------------------------------------
# set the image output directory here
#----------------------------------------------------------------------
IMG_BASE_DIR = ../../../ARC/Lib

#----------------------------------------------------------------------
# add additional C and assembly flags here
#----------------------------------------------------------------------
CFLAGS_D = -O2
ASMFLAGS_D =

CFLAGS_R = -O2
ASMFLAGS_R =

C_DEFINE_EXT = \
	$(EXTSTRG_PARAM) \
	$(SECURE_DECRYPT_UBOOT_PARAM) \
	$(SECURE_DECRYPT_OPTEE_PARAM) \
	$(OPTEE_UBOOT_SIGNATURE_PARAM) \
	$(SECURE_DECRYPT_RTOS_PARAM) \
	$(SECURE_DECRYPT_FDT_PARAM) \
	$(SECURE_DECRYPT_KERNEL_PARAM) \
	$(BL_STORAGEINT_PARAM) \
	$(SENSOR_PARAM) \
	$(NOR_DTR_PARAM) \
	$(FASTBOOT_PARAM) \
	$(Low_Pwr_PARAM) \
	$(DDR_SIZE_ALL_IN_ON_PARAM) \
	$(SPIPINMUX_PARAM) \
	$(EMMCPINMUX_PARAM) \
	$(DIRECT_BOOT_PARAM) \

# Flash Frequency provided externally (make parameter)
ifneq ($(FLASH_FREQ_MHZ),)
$(info Flash Frequency: $(shell echo -e "\033[1;33m$(FLASH_FREQ_MHZ) MHz\033[0m"))
C_DEFINE_EXT += -D_FLASH_FREQ_MHZ_=$(FLASH_FREQ_MHZ)
endif

CFLAGS_R = $(C_DEFINE_EXT)

#----------------------------------------------------------------------
# include common parts of the makefile
#----------------------------------------------------------------------
MAKE_COMMON_DIR = ../../../MakeCommon
include $(MAKE_COMMON_DIR)/OutputLib.txt
