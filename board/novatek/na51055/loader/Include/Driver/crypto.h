/**
    Header file for CC (Core Communicator) module.

    This file is the header file that define the API and data type
    for CC module.

    @file       CC.h
    @ingroup    mIDrvSys_CC
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2015.  All rights reserved.
*/

#include "constant.h"

#ifndef _CRYPTO_H
#define _CRYPTO_H

#define ROM_AES_SIZE                    (16)
#define ROM_SHA_SIZE                    (32)    // unit: byte
#define ROM_RSA_SIZE                    (256)   // RSA-2048
#define ROM_RSA_EKEY_SIZE               (32)
#define ROM_RSA_CRC32_SIZE              (4)
/**
    Crypto engine crypto mode
*/
typedef enum {
    CRYPTO_RSV0,
    CRYPTO_RSV1,
	CRYPTO_AES,                         ///< Select Crypto mode AES
	CRYPTO_MODE_NUM,
} CRYPTO_MODE;

/**
    Crypto engine crypto Operating mode
*/
typedef enum {
	CRYPTO_EBC = 0x00,                  ///< Select Crypto opmode EBC
	CRYPTO_CBC,                         ///< Select Crypto opmode CBC
	CRYPTO_OPMODE_NUM,
} CRYPTO_OPMODE;

/**
    Crypto engine encrypt or decrypt
*/
typedef enum {
	CRYPTO_ENCRYPT = 0x00,              ///< Select Crypto engine encrypt
	CRYPTO_DECRYPT,                     ///< Select Crypto engine decrypt
	CRYPTO_TYPE_NUM,

} CRYPTO_TYPE;

typedef enum {
	EFUSE_OTP_1ST_KEY_SET_FIELD = 0x0,        // This if for secure boot
	EFUSE_OTP_2ND_KEY_SET_FIELD,
	EFUSE_OTP_3RD_KEY_SET_FIELD,
	EFUSE_OTP_4TH_KEY_SET_FIELD,

	EFUSE_OTP_TOTAL_KEY_SET_FIELD,
} EFUSE_OTP_KEY_SET_FIELD;

typedef struct _CRYPT_OP {
    CRYPTO_OPMODE   op_mode;            ///< Operation Mode (now support ECB only)
    CRYPTO_TYPE     en_de_crypt;        ///< Encrypt or decrypt (CRYPTO_ENCRYPT or CRYPTO_DECRYPT)
    UINT32          src_addr;           ///< Source address
    UINT32          dst_addr;           ///< Destination address
    UINT32          length;             ///< length
} CRYPT_OP;

/**
    Crypto engine check
*/
typedef enum {
	SECUREBOOT_SECURE_EN = 0x00,       	///< Quary if secure enable or not
	SECUREBOOT_DATA_AREA_ENCRYPT,       ///< Quary if data area encrypt to cypher text or not
	SECUREBOOT_SIGN_RSA,				///< Quary if Signature methed is RSA or not(AES)
	SECUREBOOT_SIGN_RSA_CHK,			///< Quary if Signature hash checksum RSA key correct or not
	SECUREBOOT_STATUS_NUM,

} SECUREBOOT_STATUS;

#define is_secure_enable()				quary_secure_boot(SECUREBOOT_SECURE_EN)			//For backward compatitable
#define is_data_area_encrypted()		quary_secure_boot(SECUREBOOT_DATA_AREA_ENCRYPT)	//For backward compatitable
#define is_signature_rsa()				quary_secure_boot(SECUREBOOT_SIGN_RSA)			//For backward compatitable
#define is_signature_aes()				!quary_secure_boot(SECUREBOOT_SIGN_RSA)			//For backward compatitable
BOOL quary_secure_boot(SECUREBOOT_STATUS scu_status);
UINT32 crypto_data_operation(EFUSE_OTP_KEY_SET_FIELD key_set, CRYPT_OP crypt_op_param);
void rsa_setConfig(UINT32 mode);
void rsa_setkey_n(UINT8* key, UINT32 len, UINT32 sram_size);
void rsa_setkey_ed(UINT8* key, UINT32 len, UINT32 sram_size);
INT32 rsa_pio_enable(UINT8* data, UINT32 len, UINT32 sram_size);
void rsa_getOutput(UINT8 * Output, UINT32 len, UINT32 sram_size);

void shahw( const unsigned char *input, int ilen, UINT32 output[8]);
void rsa_decrypt(UINT32 *input ,UINT32 input_len, UINT32 *pRSAN, UINT32 RSAN_len,  UINT32 *pRSAED, UINT32 RSAED_len , UINT32 *pShaOut);
BOOL rsa_keycheck(UINT32 *input , BOOL efuseCheck);

#endif
