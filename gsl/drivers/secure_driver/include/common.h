/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
 */
#ifndef __COMMON_H_
#define __COMMON_H_
#include "types.h"
#include "td_type.h"
#define NO_CHECK_WORD	0

/*-----------------------------------------------------------------
 * set cpu mode interface
------------------------------------------------------------------*/
void set_mod_normal(void);
void excute_boot(void);
void write_version(void);

#define ALWAYS_INLINE inline __attribute__((always_inline))

typedef union {
	struct {
		u32 bootrom_hide_size : 10; // [9:0]
		u32 reserved : 6; // [15:10]
		u32 bootrom_hide_addr : 10; // [25:16]
	} bits;
	u32 u32;
} rom_hide_cfg;

extern u32 boot_core_reset;

/*-r----------------------------------------------------------------
 * error print interface
------------------------------------------------------------------*/
int calc_hash(uint32_t src_addr, uint32_t src_len, uint8_t *data_hash,
		uint32_t data_hash_len, uint32_t check_word);
/*-----------------------------------------------------------------
 * failure process wfi_handle
------------------------------------------------------------------*/
void entry_wfi_handle(void);

void rom_hide_config(void);

void set_scs_finish(void);

void save_cur_point_syscnt(unsigned int index);

void invalidate_icache_all(void);
#endif /* __COMMON_H_ */
