/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
 */
#ifndef __CIPHER_H_
#define __CIPHER_H_
#include "common.h"
#include "soc_drv_common.h"
#include "flash_map.h"

int decrypt_data(u32 type, para_enc_info *enc_info, u8 *code_dest, u8 *code_src,
		 u32 code_len, u32 sym_key_type);
td_s32 verify_signature(const td_u8 *pub_key, const ext_data *data, const td_u8 *sign);
td_s32 calc_hash(td_u32 src_addr, td_u32 src_len, td_u8 *data_sha,
				 td_u32 data_sha_len, td_u32 check_word);
int dma_copy(uintptr_t dest, u32 count, uintptr_t src);

void lock_ree_bootkey(void);

void lock_tee_bootkey(void);
#endif /* __CIPHER_H_ */
