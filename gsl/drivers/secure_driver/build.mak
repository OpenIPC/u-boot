# secure_driver
CSRC	+=	\
	$(wildcard secure_driver/hal_code/trng_v4/*.c)	\
	$(wildcard secure_driver/hal_code/spacc_v4/*.c)	\
	$(wildcard secure_driver/hal_code/pke_v5/*.c)	\
	$(wildcard secure_driver/hal_code/pke_alg/*.c)	\
	$(wildcard secure_driver/hal_code/km_v4/*.c)	\
	$(wildcard secure_driver/drv_code/hash/*.c)		\
	$(wildcard secure_driver/drv_code/km/*.c)		\
	$(wildcard secure_driver/drv_code/pke/*.c)	\
	$(wildcard secure_driver/drv_code/otp/*.c)	\
	$(wildcard secure_driver/drv_code/pke/rsa/*.c)	\
	$(wildcard secure_driver/drv_code/pke/ecc/*.c)	\
	$(wildcard secure_driver/romable/*.c) \
	$(wildcard secure_driver/common/*.c) \
	$(wildcard secure_driver/common/curve_param/*.c) \
	$(wildcard secure_driver/crypto_osal/*.c)

CFLAGS	+= \
	-Isecure_driver/include/drv_include	\
	-Isecure_driver/include/hal_include	\
	-Isecure_driver/include/common_include	\
	-Isecure_driver/include/romable_include	\
	-Isecure_driver/common	\
	-Isecure_driver/common/common_include	\
	-Isecure_driver	\
	-Isecure_driver/include/common_include  \
	-Isecure_driver/crypto_osal	\
	-Isecure_driver/drv_code/pke \
	-Isecure_driver/hal_code/pke_v5	\
	-Isecure_driver/hal_code/pke_alg

CFLAGS	+= -include crypto_platform.h
CFLAGS	+= -include $(PWD)/drivers/share_drivers/include/td_type.h