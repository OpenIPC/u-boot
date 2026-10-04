# MPI
obj-y	+= km/ot_mpi_km.o

# DRV
obj-y	+= security_subsys_common/drv_code/km/drv_klad.o
obj-y	+= security_subsys_common/drv_code/km/drv_klad_clear_key.o
obj-y	+= security_subsys_common/drv_code/km/drv_klad_effective_key.o
obj-y	+= security_subsys_common/drv_code/km/drv_klad_content_key.o
obj-y	+= security_subsys_common/drv_code/km/drv_klad_session_key.o
obj-y	+= security_subsys_common/drv_code/km/drv_keyslot.o

# HAL
obj-y	+= security_subsys_common/hal_code/km_v4/hal_rkp.o
obj-y	+= security_subsys_common/hal_code/km_v4/hal_klad.o
obj-y	+= security_subsys_common/hal_code/km_v4/hal_keyslot_soft.o 
obj-y	+= security_subsys_common/hal_code/km_v4/hal_keyslot_hard.o
obj-y	+= security_subsys_common/hal_code/km_v4/hal_klad_clear_key.o