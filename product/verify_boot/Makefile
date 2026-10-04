sinclude $(TOPDIR)/config.mk

ifdef CONFIG_TARGET_HI3516CV610_FAMILY
ccflags-y 	+= -DCONFIG_HI3516CV610
CONFIG_HI3516CV610 = y
else
ccflags-y 	+= -DCONFIG_HI3519DV500
CONFIG_HI3519DV500 = y
endif

obj-$(CONFIG_VERIFY_BOOT_ENABLE) += verify_boot_cmd.o
obj-$(CONFIG_VERIFY_BOOT_ENABLE) += verify_boot.o
obj-$(CONFIG_VERIFY_BOOT_ENABLE) += crypto_rsa_verify.o
obj-$(CONFIG_VERIFY_BOOT_ENABLE) += crypto_sm2_verify.o
obj-$(CONFIG_VERIFY_BOOT_ENABLE) += crypto_hash_calc.o
obj-$(CONFIG_VERIFY_BOOT_ENABLE) += crypto_sm4_dec.o

ccflags-$(CONFIG_VERIFY_BOOT_ENABLE) += -I$(srctree)/product/security_subsys/security_subsys_common/ot_mpi_api
ccflags-$(CONFIG_VERIFY_BOOT_ENABLE) += -I$(srctree)/product/security_subsys/security_subsys_common/include/common_include
ccflags-$(CONFIG_VERIFY_BOOT_ENABLE) += -I$(srctree)/product/security_subsys/ext_inc
ccflags-$(CONFIG_VERIFY_BOOT_ENABLE) += -I$(srctree)/product/security_subsys/init

# ccflags-y	+= -DCONFIG_VERIFY_KEY_AREA_SUPPORT
ccflags-y	+= -DCONFIG_SM4_DECRYPT_SUPPORT
ccflags-y  += -include $(srctree)/product/security_subsys/crypto_features.h