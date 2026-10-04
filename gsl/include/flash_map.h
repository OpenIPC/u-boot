/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
 */
#ifndef _FLASH_MAP_H

#define _FLASH_MAP_H
#include <td_type.h>
/* Image id definition */
#define REE_FLASH_RPK_IMAGE_ID	   0x4BA5C31E
#define TEE_FLASH_RPK_IMAGE_ID	   0x4B96B41E
#define TP_FLASH_RPK_IMAGE_ID	   0x4B2D4B1E

#define GSL_KEY_AREA_IMAGE_ID	   0x4BB4D21E
#define TP_EXT_KEY_IMAGE_ID	   0x4B3C5A3C
#define GSL_CODE_INFO_IMAGE_ID	   0x4BB4D22D

#define PARAMS_INFO_IMAGE_ID	   0x4B87A52D
#define REE_BOOT_KEY_IMAGE_ID	   0x4B1E3C1E
#define REE_BOOT_INFO_IMAGE_ID	   0x4BF01E2D
#define REE_BOOTARGS_INFO_IMAGE_ID 0x4B69872D
#define REE_APP_CODEINFO_IMAGE_ID  0x4B0F2D2D

#define TEE_OS_KEY_IMAGE_ID	   0x4BE10F1E
#define TEE_OS_INFO_IMAGE_ID	   0x4BE10F2D

#define REE_KEY_AREA_IMG_ID           0x4B1E3C1E
#define PARAMS_INFO_IMG_ID            0x4B87A52D
#define UBOOT_INFO_IMG_ID             0x4BF01E2D

#define TEE_KEY_AREA_IMG_ID           0x4BE10F1E
#define TEE_CODE_INFO_IMG_ID          0x4BE10F2D

/* structure version */
#define STRUCTURE_VERSION               0x00010000

/* Algorithm select definition */
#define ALG_SEL_ECC256                  0x2A13C812
#define ALG_SEL_SM2                     0x2A13C823
#define ECC_CURVE_RFC5639               0x2A13C812

/* Maintenance flag definition */
#define MAINTENACE_MODE_ENABLE          0x3C7896E1

/* Image encrypt flag */
#define CODE_ENC_DISABLE  0x3C7896E1
#define REE_DEC_DISABLE	      0x0
#define TEE_DEC_DISABLE	      0x0

/* Compress flag definition */
#define APP_COMPRESS_ENABLE             0x3C7896E1

#define DIE_ID_LEN                      16
#define HASH_LEN                        32
#define MX_2				2
#define RSA_SIG_LEN			384
#define SM_SIG_LEN			0x40
#define ECC_256_KEY_LEN                 64
#define PROTECT_KEY_LEN                 16
#define IV_LEN                          16
#define PARAMS_STRUCTURE_HEAD_LEN       15

#define MAX_PARAMS_NUM                  8
#define RSA_KEY_LEN 384
#define RSA_KEY_OFFSET 384
#define RSA_SIGN_LEN 384
#define SHA_256_LEN   32

#define RSA_KEY_N_E_LEN 388
#define SM_KEY_LEN		       0x40
/* Flash rootkey definition */
#define FLSH_ROOTKEY_VALID              0x5
#define FLSH_ROOTKEY_INVALID            0xA

#define MAX_DDR_PARA_NUM                8
#define SIZE_1K                         1024
#define SIZE_15K                        (15 * 1024)
#define ECC_KEY_OFFSET 32

#define INVALID_PAM_HASH_VAL	0xff

/* for uart or usb driver recive data check */
/* ree key(fix 0x100) + param info(fix 0x200) + fill(fix 0x100) + param(max 15k) */
#define FIRST_RECIVE_MAX_LEN	0x4000
/* uboot info(fix 0x200) + uboot code(max 1M) */
#define SECOND_RECIVE_MAX_LEN	0x100000


/* TEE/REE/TP root public key area, size is 0x80 */
typedef struct {
	unsigned int      img_id;
	unsigned int      structure_version;  /* currently version is 0x00010000 */
	unsigned int      structure_length;
	unsigned int      key_owner_id;
	unsigned int      key_id;
	unsigned int      key_alg;            /* 0x2A13C812: ECC256;  0x2A13C823: SM2 */
	unsigned int      ecc_curve_type;     /* 0x2A13C812: RFC 5639, BrainpoolP256r1 */
	unsigned int      key_length;
	unsigned int verify_enable_ext;
	unsigned char     reserved[0x58]; /* 32 bytes reserved */
	unsigned char     root_key_area[RSA_KEY_N_E_LEN];
} flash_root_public_key_s;

/* Key area, size is 0x100 */
typedef struct {
	unsigned int      img_id;
	unsigned int      structure_version;
	unsigned int      structure_length;
	unsigned int      signature_length;
	unsigned int      key_owner_id;
	unsigned int      key_id;
	unsigned int      key_alg;            /* 0x2A13C812: ECC256;  0x2A13C823: SM2 */
	unsigned int      ecc_curve_type;     /* 0x2A13C812: RFC 5639, BrainpoolP256r1 */
	unsigned int      key_length;
	unsigned int      version_ext;
	unsigned int      mask_version_ext;
	unsigned int      msid_ext;
	unsigned int      mask_msid_ext;
	unsigned int      maintenance_mode;   /* 0x3C7896E1: enable */
	unsigned char     die_id[DIE_ID_LEN];
	unsigned char     reserved[0xb4]; /* 52 bytes reserved */
	unsigned char     ext_pulic_key_area[RSA_KEY_N_E_LEN];
	unsigned char     signature[RSA_SIGN_LEN];
} gsl_key_area_s;

typedef struct {
	unsigned int      img_id;
	unsigned int      structure_version;
	unsigned int      structure_length;
	unsigned int      signature_length;
	unsigned int      key_owner_id;
	unsigned int      key_id;
	unsigned int      key_alg;            /* 0x2A13C812: ECC256;  0x2A13C823: SM2 */
	unsigned int      ecc_curve_type;     /* 0x2A13C812: RFC 5639, BrainpoolP256r1 */
	unsigned int      key_length;
	unsigned int      version_ext;
	unsigned int      mask_version_ext;
	unsigned int      msid_ext;
	unsigned int      mask_msid_ext;
	unsigned int      maintenance_mode;   /* 0x3C7896E1: enable */
	unsigned char     die_id[DIE_ID_LEN];
	union {
		struct {
			td_u8 reserved[0xb4]; /* 180 bytes reserved */
			td_u8 ext_pulic_key_area[RSA_KEY_N_E_LEN]; /* 388 byte is reserved */
			td_u8 signature[RSA_SIG_LEN]; /* 384 byte is reserved */
		} rsa;
		struct {
			td_u8 reserved[824]; /* 824 bytes reserved */
			td_u8 ext_pulic_key_area[SM_KEY_LEN]; /* 64 byte is reserved */
			td_u8 signature[SM_SIG_LEN]; /* 64 byte is reserved */
		} sm;
		td_u8 reserved_and_key_sig[952]; /* 952 bytes reserved */
	};
} ree_key_area_s;

/* Code area info, size is 0x200 */
typedef struct {
	unsigned int      img_id;
	unsigned int      structure_version;
	unsigned int      structure_length;
	unsigned int      signature_length;
	unsigned int      version_ext;
	unsigned int      mask_version_ext;
	unsigned int      msid_ext;
	unsigned int      mask_msid_ext;
	unsigned int      code_area_addr;
	unsigned int      code_area_len;
	unsigned char     code_area_hash[HASH_LEN];
	unsigned int      code_enc_flag;
	unsigned char     key_material_gsl[PROTECT_KEY_LEN];
	unsigned char     reaserved2[PROTECT_KEY_LEN];
	unsigned char     iv[IV_LEN];
	unsigned int      code_compress_flag; /* 0x3C7896E1: is compressed */
	unsigned int      code_uncompress_len;
	unsigned int      text_segment_size;
	unsigned char 	  fmc_data[0x16];
	unsigned char 	  reserved[0x1e2]; /* 244 byte is reserved */
	unsigned char     sig_code_info[RSA_SIG_LEN];
} gsl_code_info;

typedef struct {
	unsigned int      img_id;
	unsigned int      structure_version;
	unsigned int      structure_length;
	unsigned int      signature_length;
	unsigned int      version_ext;
	unsigned int      mask_version_ext;
	unsigned int      msid_ext;
	unsigned int      mask_msid_ext;
	unsigned int      para_area_addr;
	unsigned int      para_area_len;
	unsigned int      para_area_num;
	unsigned char     para_area_hash[MAX_PARAMS_NUM][HASH_LEN];
	unsigned char     board_index_hash_table[MAX_PARAMS_NUM];
	union {
		struct {
			td_u8 reserved[0x14c];  /* 332 byte is reserved */
			td_u8 signature[RSA_SIG_LEN]; /* 384 byte is reserved */
		} rsa;
		struct {
			td_u8 reserved[652];  /* 652 byte is reserved */
			td_u8 signature[SM_SIG_LEN]; /* 64 byte is reserved */
		} sm;
		td_u8 reserved_and_sig[716]; /* 716 bytes reserved */
	};
} para_area_info;

typedef struct {
	unsigned int      code_enc_flag;
	unsigned char     key_material[PROTECT_KEY_LEN];
	unsigned char     reserve[PROTECT_KEY_LEN];
	unsigned char     iv[IV_LEN];
} para_enc_info;

/* Code area info, size is 0x200 */
typedef struct {
	unsigned int      img_id;
	unsigned int      structure_version;
	unsigned int      structure_length;
	unsigned int      signature_length;
	unsigned int      version_ext;
	unsigned int      mask_version_ext;
	unsigned int      msid_ext;
	unsigned int      mask_msid_ext;
	unsigned int      code_area_addr;
	unsigned int      code_area_len;
	unsigned char     code_area_hash[HASH_LEN];
	para_enc_info     enc_info;
	unsigned int      code_compress_flag; /* 0x3C7896E1: is compressed */
	unsigned int      code_uncompress_len;
	unsigned int      text_segment_size;
	unsigned int      uboot_entry_point;
	union {
		struct {
			td_u8 reserved[0x1f4];  /* 500 byte is reserved */
			td_u8 signature[RSA_SIG_LEN]; /* 384 byte is reserved */
		} rsa;
		struct {
			td_u8 reserved[820];  /* 820 byte is reserved */
			td_u8 signature[SM_SIG_LEN]; /* 64 byte is reserved */
		} sm;
		td_u8 reserved_and_sig[884]; /* 884 bytes reserved */
	};
} uboot_code_info;

typedef struct {
	unsigned int      img_id;
	unsigned int      structure_version;
	unsigned int      structure_length;
	unsigned int      signature_length;
	unsigned int      key_owner_id;
	unsigned int      key_id;
	unsigned int      key_alg;            /* 0x2A13C812: ECC256;  0x2A13C823: SM2 */
	unsigned int      ecc_curve_type;     /* 0x2A13C812: RFC 5639, BrainpoolP256r1 */
	unsigned int      key_length;
	unsigned int      version_ext;
	unsigned int      mask_version_ext;
	unsigned int      msid_ext;
	unsigned int      mask_msid_ext;
	unsigned int      maintenance_mode;   /* 0x3C7896E1: enable */
	unsigned char     die_id[DIE_ID_LEN];
	union {
		struct {
			td_u8 reserved[0xb4]; /* 52 bytes reserved */
			td_u8 ext_pulic_key_area[RSA_KEY_N_E_LEN]; /* 388 byte is reserved */
			td_u8 signature[RSA_SIG_LEN]; /* 384 byte is reserved */
		} rsa;
		struct {
			td_u8 reserved[824]; /* 824 bytes reserved */
			td_u8 ext_pulic_key_area[SM_KEY_LEN]; /* 64 byte is reserved */
			td_u8 signature[SM_SIG_LEN]; /* 64 byte is reserved */
		} sm;
		td_u8 reserved_and_key_sig[952]; /* 952 bytes reserved */
	};
} tee_key_area_s;

/* tee info, size is 0x200 */
typedef struct {
	unsigned int      img_id;
	unsigned int      structure_version;
	unsigned int      structure_length;
	unsigned int      signature_length;
	unsigned int      version_ext;
	unsigned int      mask_version_ext;
	unsigned int      msid_ext;
	unsigned int      mask_msid_ext;
	unsigned int      tee_code_area_addr;
	unsigned int      tee_code_area_len;
	unsigned char     tee_code_area_hash[HASH_LEN];
	para_enc_info     enc_info;
	unsigned int      tee_code_compress_flag; /* 0x3C7896E1: is compressed */
	unsigned int      tee_code_uncompress_len;
	unsigned int      tee_text_segment_size;
	unsigned int      tee_secure_ddr_size;
	union {
		struct {
			td_u8 reserved[0x1f4];  /* 500 byte is reserved */
			td_u8 signature[RSA_SIG_LEN]; /* 384 byte is reserved */
		} rsa;
		struct {
			td_u8 reserved[820];  /* 820 byte is reserved */
			td_u8 signature[SM_SIG_LEN]; /* 64 byte is reserved */
		} sm;
		td_u8 reserved_and_sig[884]; /* 884 bytes reserved */
	};
} tee_code_info;

/* Boot Image BackUp */
typedef struct {
	int enable;
	unsigned int offset_times;
	unsigned int offset_addr;
} backup_img_params_s;
typedef struct aapcs64_params {
	size_t arg0;
	size_t arg1;
	size_t arg2;
	size_t arg3;
	size_t arg4;
	size_t arg5;
	size_t arg6;
	size_t arg7;
} aapcs64_params_t;

typedef struct param_header {
	u8 type;       /* type of the structure */
	u8 version;    /* version of this structure */
	u16 size;      /* size of this structure in bytes */
	u32 attr;      /* attributes: unused bits SBZ */
} param_header_t;

typedef struct entry_point_info {
	param_header_t h;
	size_t pc;
	u32 spsr;
	aapcs64_params_t args;
} entry_point_info_t;

int get_ree_key_and_paras_info_from_device(const backup_img_params_s *backup_params, u32 channel_type);
int get_paras_data_from_flash(const backup_img_params_s *backup_params, u32 board_index, u32 channel_type);
int get_uboot_info_from_flash(const backup_img_params_s *backup_params, u32 channel_type);
int get_uboot_code_from_flash(const backup_img_params_s *backup_params, u32 channel_type);

typedef enum {
	REE = 0x04dd5974,
	TEE = 0x398488f2,
	TP = 0xa9053a2b,
	PARAMS = 0x5a963863,
	INVALID_APP_TYPE
} app_type;

typedef enum {
	GSL_ROOTKEY = 0x2A13C812,
	UBOOT_ROOTKEY = 0x2A13C823,
	TEE_ROOTKEY = 0x2A13C834,
	TA_ROOTKEY = 0x2A13C845,
	REE_APP_ROOTKEY = 0x2A13C856,
} sym_key_type;
#endif
