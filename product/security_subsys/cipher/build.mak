# Common
obj-y	+= security_subsys_common/common/crypto_drv_common.o
obj-y	+= security_subsys_common/common/crypto_pke_common.o
obj-y	+= security_subsys_common/common/crypto_hash_common.o
obj-y	+= security_subsys_common/common/ecc_curve_param.o
obj-y	+= security_subsys_common/common/curve_param/brainpool_256r1.o
obj-y	+= security_subsys_common/common/curve_param/brainpool_384r1.o
obj-y	+= security_subsys_common/common/curve_param/brainpool_512r1.o
obj-y	+= security_subsys_common/common/curve_param/nist_p256r.o
obj-y	+= security_subsys_common/common/curve_param/nist_p384r.o
obj-y	+= security_subsys_common/common/curve_param/nist_p521r.o
obj-y	+= security_subsys_common/common/curve_param/sm2.o
# DRV
obj-y	+= security_subsys_common/drv_code/trng/drv_trng.o
obj-y	+= security_subsys_common/drv_code/symc/drv_symc_ccm.o
obj-y	+= security_subsys_common/drv_code/symc/drv_symc_ctr_non_align.o
obj-y	+= security_subsys_common/drv_code/symc/drv_symc_gcm.o
obj-y	+= security_subsys_common/drv_code/symc/drv_symc_get_config.o
obj-y	+= security_subsys_common/drv_code/symc/drv_symc_mac.o
obj-y	+= security_subsys_common/drv_code/symc/drv_symc.o
obj-y	+= security_subsys_common/drv_code/symc/drv_symc_proc.o

# TRNG HAL
obj-y	+= security_subsys_common/hal_code/trng_v4/hal_trng.o

# SPACC HAL
obj-y	+= security_subsys_common/hal_code/spacc_v4/hal_symc.o
obj-y	+= security_subsys_common/hal_code/spacc_v4/hal_symc_proc.o
obj-y	+= security_subsys_common/hal_code/spacc_v4/hal_symc_cbc_mac.o
obj-y	+= security_subsys_common/hal_code/spacc_v4/hal_symc_ccm.o
obj-y	+= security_subsys_common/hal_code/spacc_v4/hal_symc_cmac.o
obj-y	+= security_subsys_common/hal_code/spacc_v4/hal_symc_gcm.o
obj-y	+= security_subsys_common/hal_code/spacc_v4/hal_hash.o

# PKE HAL
ifdef CONFIG_HI3519DV500
obj-y	+= security_subsys_common/hal_code/pke_v4/hal_pke.o
endif
ifdef CONFIG_HI3516CV610
obj-y	+= security_subsys_common/hal_code/pke_v5/ecc_data_ram_addr.o
obj-y	+= security_subsys_common/hal_code/pke_v5/rsa_data_ram_addr.o
obj-y	+= security_subsys_common/hal_code/pke_v5/hal_pke.o
obj-y	+= security_subsys_common/hal_code/pke_v5/rom_lib.o
endif

# drv_pke
ifdef CONFIG_HI3519DV500
obj-y	+= security_subsys_common/drv_code/pke/v4/drv_pke_ecc_curve.o
obj-y	+= security_subsys_common/drv_code/pke/v4/drv_pke_ecc.o
obj-y	+= security_subsys_common/drv_code/pke/v4/drv_pke.o
endif

ifdef CONFIG_HI3516CV610
obj-y	+= security_subsys_common/drv_code/pke/ecc/drv_pke_ecdsa_check_dot_on_curve.o
obj-y	+= security_subsys_common/drv_code/pke/ecc/drv_pke_ecdsa_sign.o
obj-y	+= security_subsys_common/drv_code/pke/ecc/drv_pke_sm2_sign.o
obj-y	+= security_subsys_common/drv_code/pke/ecc/drv_pke_sm2_dsa_hash.o
obj-y	+= security_subsys_common/drv_code/pke/ecc/drv_pke_ecdsa_verify.o
obj-y	+= security_subsys_common/drv_code/pke/ecc/drv_pke_sm2_verify.o
obj-y	+= security_subsys_common/drv_code/pke/ecc/drv_pke_ecdsa_ecdh.o
obj-y	+= security_subsys_common/drv_code/pke/ecc/drv_pke_ecdsa_gen_key.o
obj-y	+= security_subsys_common/drv_code/pke/ecc/drv_pke_sm2_pub_enc.o
obj-y	+= security_subsys_common/drv_code/pke/ecc/drv_pke_sm2_priv_dec.o
obj-y	+= security_subsys_common/drv_code/pke/drv_pke_inner.o
obj-y	+= security_subsys_common/drv_code/pke/drv_pke_api.o
obj-y	+= security_subsys_common/drv_code/pke/drv_pke_exp_mod.o
obj-y	+= security_subsys_common/drv_code/pke/drv_pke_mod.o
endif

# drv_hash
obj-y	+= security_subsys_common/drv_code/hash/drv_hash_inner.o
obj-y	+= security_subsys_common/drv_code/hash/drv_hash_simple.o
obj-y	+= security_subsys_common/drv_code/hash/drv_hash_clone.o
obj-y	+= security_subsys_common/drv_code/hash/drv_pbkdf2_soft.o
obj-y	+= security_subsys_common/drv_code/hash/drv_pbkdf2_hard.o
obj-y	+= security_subsys_common/drv_code/hash/drv_hash_soft_hmac_simple.o

# ECC
ifdef CONFIG_HI3516CV610
obj-y	+= security_subsys_common/hal_code/pke_alg/calc_hash.o
obj-y	+= security_subsys_common/hal_code/pke_alg/ecc_check_dot_on_curve.o
obj-y	+= security_subsys_common/hal_code/pke_alg/ecc_common.o
obj-y	+= security_subsys_common/hal_code/pke_alg/ecc_compute_s.o
obj-y	+= security_subsys_common/hal_code/pke_alg/ecc_compute_u1_and_u2.o
obj-y	+= security_subsys_common/hal_code/pke_alg/ecc_compute_x.o
obj-y	+= security_subsys_common/hal_code/pke_alg/ecc_inner.o
obj-y	+= security_subsys_common/hal_code/pke_alg/ecc_inv_mod.o
obj-y	+= security_subsys_common/hal_code/pke_alg/ecc_mul_naf_inner.o
obj-y	+= security_subsys_common/hal_code/pke_alg/ecc_point_mul.o
obj-y	+= security_subsys_common/hal_code/pke_alg/get_multi_random.o
obj-y	+= security_subsys_common/hal_code/pke_alg/pke_alg_inner.o
obj-y	+= security_subsys_common/hal_code/pke_alg/rsa_exp_mod.o
obj-y	+= security_subsys_common/hal_code/pke_alg/rsa_mod.o
obj-y	+= security_subsys_common/hal_code/pke_alg/rsa_inner.o
obj-y	+= security_subsys_common/hal_code/pke_alg/sm2_add_mod.o
obj-y	+= security_subsys_common/hal_code/pke_alg/sm2_compute_s.o
endif

# RSA
ifdef CONFIG_HI3519DV500
obj-y	+= security_subsys_common/drv_code/pke/v4/drv_pke_rsa.o
endif

ifdef CONFIG_HI3516CV610
obj-y	+= security_subsys_common/drv_code/pke/rsa/drv_pke_rsa_priv_dec.o
obj-y	+= security_subsys_common/drv_code/pke/rsa/drv_pke_rsa_pub_enc.o
obj-y	+= security_subsys_common/drv_code/pke/rsa/drv_pke_rsa_sign.o
obj-y	+= security_subsys_common/drv_code/pke/rsa/drv_pke_rsa_v15.o
obj-y	+= security_subsys_common/drv_code/pke/rsa/drv_pke_rsa_v21.o
obj-y	+= security_subsys_common/drv_code/pke/rsa/drv_pke_rsa_verify.o
endif

# MPI
obj-y	+= cipher/ot_mpi_hash.o
obj-y	+= cipher/ot_mpi_kdf.o
obj-y	+= cipher/ot_mpi_pke.o
obj-y	+= cipher/ot_mpi_symc.o
obj-y	+= cipher/ot_mpi_trng.o