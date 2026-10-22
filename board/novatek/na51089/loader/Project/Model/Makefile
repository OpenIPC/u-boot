#----------------------------------------------------------------------
# set the ROOT directory here
#----------------------------------------------------------------------
ROOT_DIR = ../..

#----------------------------------------------------------------------
# include make config file
#----------------------------------------------------------------------
include ModelConfig.txt
include ModelConfig_$(MODEL).txt
include MakeConfig.txt

#----------------------------------------------------------------------
# set the project name here
#----------------------------------------------------------------------
PRJ_NAME = Loader$(CHIP)

#----------------------------------------------------------------------
# set loader execution DRAM address
#----------------------------------------------------------------------
ifeq "$(SDRAM_SIZE)" "1024"
# 1Gb*2 => 256MB => 0x1000_0000
LD_BASE_ADDR = 0xF000000
else ifeq "$(SDRAM_SIZE)" "2048"
# 2Gb*2 => 512MB => 0x2000_0000
LD_BASE_ADDR = 0x1F000000
else
# 4Gb*2 => 1024MB => 0x4000_0000
LD_BASE_ADDR = 0x3F000000
endif

ifeq "$(LOADER_DRAM_ADDR)" ""
LD_BASE_ADDR = 0x1000000
else
LD_BASE_ADDR = $(LOADER_DRAM_ADDR)
endif

#----------------------------------------------------------------------
# set the include directory here
#----------------------------------------------------------------------
INC_DIR = . \
	../../Include

#----------------------------------------------------------------------
# add/delete the project assembly files here
#----------------------------------------------------------------------
#ASM = Src/reset.S \
#Src/remap.S




#----------------------------------------------------------------------
# add/delete the project C source files here
#----------------------------------------------------------------------
SRC = \
	Src/prj_main.c \
	Src/USB/usb_update.c \
	Src/UART/uart_upgrade.c




#----------------------------------------------------------------------
# set the image output directory here
#----------------------------------------------------------------------
IMG_BASE_DIR = $(PRJ_NAME)_Data

#----------------------------------------------------------------------
# set the library directory here
#----------------------------------------------------------------------
LIB_BASE_DIR = ../../LIB

#----------------------------------------------------------------------
# set the library directory here
#----------------------------------------------------------------------
ARC_BASE_DIR = $(ROOT_DIR)/ARC

#----------------------------------------------------------------------
# add additional C and assembly flags here, for example, -O2
#----------------------------------------------------------------------
C_DEFINE_EXT = \
	$(MODEL_PARAM) \
	$(BL_UPDATE_NAME_PARAM) \
	$(BL_STRINGS_PARAM) \
	$(BL_COMPRESS_PARAM) \
	$(CHKFWIMAGE_PARAM) \
	$(FWCOMPRESS_PARAM) \
	$(BL_LOADER_VER_PARAM) \
	$(BL_LDADDR_PARAM) \
	$(EXTSTRG_PARAM) \
	$(STORAGE_INT_TYPE)\
	$(SPIPINMUX_PARAM)\
	$(EMMCPINMUX_PARAM)\
	$(DRAMSCAN_PARAM)\
	$(BL_DUTY_CALIBRATION_PARAM)\
	$(BL_DUTY_CALIBRATION_TYPE_PARAM)\
	$(BL_LOADER_FLOW_PARAM) \
	$(BL_STORAGEINT_PARAM) \
	$(SECURE_DECRYPT_UBOOT_PARAM) \
	$(SECURE_DECRYPT_OPTEE_PARAM) \
	$(CARDDET_PIN_PARAM)

CFLAGS_R = $(C_DEFINE_EXT)
CFLAGS_D = $(C_DEFINE_EXT)

ASMFLAGS_R = \
	$(BL_COMPRESS_PARAM_ASM)

ASMFLAGS_D = \
	$(BL_COMPRESS_PARAM_ASM)

GCCFLAGS = \
	$(MODEL_PARAM) \
	$(BL_UPDATE_NAME_PARAM) \
	$(BL_STRINGS_PARAM) \
	$(BL_COMPRESS_PARAM) \
	$(CHKFWIMAGE_PARAM) \
	$(FWCOMPRESS_PARAM) \
	$(BL_LOADER_VER_PARAM) \
	$(STORAGE_INT_TYPE)\
	$(DRAMSCAN_PARAM)\
	$(BL_DUTY_CALIBRATION_PARAM)\
	$(BL_DUTY_CALIBRATION_TYPE_PARAM)\
	$(BL_LOADER_FLOW_PARAM)

#----------------------------------------------------------------------
# include common parts of the makefile
#----------------------------------------------------------------------
MAKE_COMMON_DIR = ../../MakeCommon
include $(MAKE_COMMON_DIR)/OutputImg.txt
