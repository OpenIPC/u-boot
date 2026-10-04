SECURITY_SUBSYS_DIR = $(srctree)/product/security_subsys
ccflags-y += -I$(SECURITY_SUBSYS_DIR)/security_subsys_common/ot_mpi_api/
ccflags-y += -I$(SECURITY_SUBSYS_DIR)/ext_inc
ccflags-y += $(WRITE_OTP_CFLAGS)

obj-y	+= $(WRITE_OTP_SRCS:%.c=%.o)
