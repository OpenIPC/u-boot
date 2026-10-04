/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
 */
#ifndef __COMMON_H_
#define __COMMON_H_
#include "td_type.h"
#include "platform.h"
#include "share_drivers.h"
#define BYTE_NUMS 4
#define NO_CHECK_WORD	0
#define DELAY_TIME_MS	2

extern unsigned char _ddr_load_start[];
extern unsigned char _ddr_text_start[];
extern unsigned char _ddr_text_end[];

typedef struct {
	const td_u8 *r; /* r/x component of the signature. */
	const td_u8 *s; /* s/y component of the signature. */
	td_u32 length;
} ext_pke_sig;

#define TEE_VERSION_BOUND 64

#define ALWAYS_INLINE inline __attribute__((always_inline))

void call_reset(void);
/*-----------------------------------------------------------------
 * timer interface
------------------------------------------------------------------*/
unsigned long timer_get_divider(void);

#endif /* __COMMON_H_ */
