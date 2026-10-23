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
	bl_func.c \
	main.c \
	nand_ids.c\
	spi_nor_ids.c\
	bl_u2.c

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
	$(BL_STORAGEINT_PARAM) \

CFLAGS_R = $(C_DEFINE_EXT)

#----------------------------------------------------------------------
# include common parts of the makefile
#----------------------------------------------------------------------
MAKE_COMMON_DIR = ../../../MakeCommon
include $(MAKE_COMMON_DIR)/OutputLib.txt
