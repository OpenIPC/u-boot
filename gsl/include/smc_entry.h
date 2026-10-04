/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
 */
#ifndef _SMC_ENTRY_H
#define _SMC_ENTRY_H
#include "types.h"

#define ARM_USR_MOD	0x10
#define ARM_FIQ_MOD	0x11
#define ARM_IRQ_MOD	0x12
#define ARM_SVC_MOD	0x13
#define ARM_MON_MOD	0x16
#define ARM_ABT_MOD	0x17
#define ARM_UND_MOD	0x1b
#define ARM_SYS_MOD	0x1f

/*
 * The generic structure to save arguments and callee saved registers during
 * an SMC. Also this structure is used to store the result return values after
 * the completion of SMC service.
 */
typedef struct gsl_smc_ctx {
	u32 r0;
	u32 r1;
	u32 r2;
	u32 r3;
	u32 r4;
	u32 r5;
	u32 r6;
	u32 r7;
	u32 r8;
	u32 r9;
	u32 r10;
	u32 r11;
	u32 r12;

	u32 spsr_svc;
	u32 sp_svc;
	u32 lr_svc;

	u32 spsr_mon;
	u32 sp_mon;
	u32 lr_mon;
} gsl_smc_ctx;

#define CTX_SPSR_SVC_OFFSET __builtin_offsetof(struct gsl_smc_ctx, spsr_svc)
#define CTX_SP_MON_OFFSET __builtin_offsetof(struct gsl_smc_ctx, sp_mon)

struct gsl_smc_ctx *get_gsl_smc_ctx(void);

#define xstr(s) str(s)
#define str(s) #s
#endif
