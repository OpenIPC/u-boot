SECURITY_SUBSYS_DIR := $(srctree)/product/security_subsys

ifdef CONFIG_TARGET_HI3516CV610_FAMILY
ccflags-y 	+= -DCONFIG_HI3516CV610
CONFIG_HI3516CV610 = y
else
ccflags-y 	+= -DCONFIG_HI3519DV500
CONFIG_HI3519DV500 = y
endif

include $(SECURITY_SUBSYS_DIR)/km/build.mak
include $(SECURITY_SUBSYS_DIR)/otp/build.mak
include $(SECURITY_SUBSYS_DIR)/cipher/build.mak
include $(SECURITY_SUBSYS_DIR)/crypto_osal/build.mak
include $(SECURITY_SUBSYS_DIR)/init/build.mak

ccflags-y 	+= -I${SECURITY_SUBSYS_DIR}
ccflags-y 	+= -I${SECURITY_SUBSYS_DIR}/init
ccflags-y 	+= -I${SECURITY_SUBSYS_DIR}/ext_inc
ccflags-y 	+= -I${SECURITY_SUBSYS_DIR}/crypto_osal
ccflags-y 	+= -I${SECURITY_SUBSYS_DIR}/security_subsys_common/common
ccflags-y 	+= -I${SECURITY_SUBSYS_DIR}/security_subsys_common/include/common_include
ccflags-y 	+= -I${SECURITY_SUBSYS_DIR}/security_subsys_common/include/hal_include
ccflags-y 	+= -I${SECURITY_SUBSYS_DIR}/security_subsys_common/include/drv_include
ccflags-y 	+= -I${SECURITY_SUBSYS_DIR}/security_subsys_common/ot_mpi_api
ccflags-y 	+= -I${SECURITY_SUBSYS_DIR}/security_subsys_common/drv_code
ccflags-y 	+= -I${SECURITY_SUBSYS_DIR}/security_subsys_common/drv_code/hash
ccflags-y 	+= -I${SECURITY_SUBSYS_DIR}/security_subsys_common/drv_code/symc
ifdef CONFIG_HI3519DV500
ccflags-y 	+= -I${SECURITY_SUBSYS_DIR}/security_subsys_common/drv_code/pke/v4
endif
ifdef CONFIG_HI3516CV610
ccflags-y 	+= -I${SECURITY_SUBSYS_DIR}/security_subsys_common/drv_code/pke
ccflags-y 	+= -I${SECURITY_SUBSYS_DIR}/security_subsys_common/drv_code/pke/ecc
ccflags-y 	+= -I${SECURITY_SUBSYS_DIR}/security_subsys_common/hal_code/pke_v5
ccflags-y 	+= -I${SECURITY_SUBSYS_DIR}/security_subsys_common/hal_code/pke_alg
endif

ccflags-y  += -include crypto_features.h