/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
 */
#include <types.h>
#include <platform.h>
#include "common.h"
#include "td_type.h"
#include "soc_errno.h"
#include "checkup.h"
#include "../drivers/otp/otp.h"
#include "cipher.h"
#include "smc_entry.h"

static struct gsl_smc_ctx g_gsl_smc_context;

__attribute__((__section__(".data.ddr")))
static struct gsl_smc_ctx g_ns_ctx;

void run_optee(void);

struct gsl_smc_ctx *get_gsl_smc_ctx(void)
{
	return &g_gsl_smc_context;
}

struct gsl_smc_ctx *get_ns_ctx(void)
{
	return &g_ns_ctx;
}

void tee_img_failure_process(void)
{
	u32 val;
	val = reg_get(TEE_FAILURE_SOURCE);
	val = set_tee_failure_source_type(val, VERIFY_TEE_OS_FAILED);
	reg_set(TEE_FAILURE_SOURCE, val);
	call_reset();
}

#define save_regs(arm_mode)  \
	"cps	#" xstr(arm_mode) "\n\t" \
	"mrs	r2, spsr\n\t" \
	"str	r2, [r0], #4\n\t" \
	"str	sp, [r0], #4\n\t" \
	"str	lr, [r0], #4\n\t" \

// writed for armv7 t32 and a32 isa
__attribute__((naked)) __attribute__((__optimize__ ("-fno-stack-protector")))
void handle_smc(void)
{
	asm(
	"stm sp, {r0-r12}\n\t"
	"ldr r3, [sp, %[sp_mon_offset]]\n\t"

	"mrc p15, 0, r0, c1, c1, 0\n\t"
	"bic r0, r0, #1\n\t" // clear ns bit
	"mcr p15, 0, r0, c1, c1, 0\n\t"
	"isb\n\t"

	// save callee saved regs; spsr_svc; lr_svc; sp_svc; lr_mon; spsr_mon;
	"mov r0, sp\n\t"
	"add r0, r0, %[spsr_svc_offset]\n\t"

	// save no sec contex
	// prevent optee rewriting them, and prevent TEE data from being exposed on REE
	save_regs(ARM_SVC_MOD)
	save_regs(ARM_MON_MOD)

	"isb\n\t"

	"mov r0, sp\n\t"
	"mov sp, r3\n\t"

	"bl run_optee\n\t" \
	:: [spsr_svc_offset] "I"(CTX_SPSR_SVC_OFFSET), \
		[sp_mon_offset] "I"(CTX_SP_MON_OFFSET));
}

struct optee_header {
	u32 magic;
	u8 version;
	u8 arch;
	u16 flags;
	u32 init_size;
	u32 init_load_addr_hi;
	u32 init_load_addr_lo;
	u32 init_mem_usage;
	u32 paged_size;
};

__attribute__((__section__(".text.ddr")))
__attribute__((naked)) __attribute__((__optimize__ ("-fno-stack-protector")))
void tee_os_return(void)
{
	asm(
	"cps	#" xstr(ARM_SVC_MOD) "\n\t"

	// restore ctx
	"mov sp, r1\n\t"
	"ldmia sp!, {r0-r12}\n\t"

	"mov r0, sp\n\t"

	"add r0, r0, #4\n\t"
	// restore sp
	"ldr sp, [r0], #4\n\t"

	// restore spsr
	"ldr r1, [r0, #4]\n\t"
	"msr spsr, r1\n\t"

	// restore lr_svc to lr
	"ldr r1, [r0]\n\t"
	"mov lr, r1\n\t"

	// exception return
	"add r1, r0, #0xc\n\t"
	"mov r0, #0x0\n\t"
	"ldm r1, {pc}\n\t");
}

__attribute__((naked)) __attribute__((__optimize__ ("-fno-stack-protector")))
void start_tee_os(td_func optee_os)
{
	struct gsl_smc_ctx *gsl_ctx = get_gsl_smc_ctx();
	struct gsl_smc_ctx *ns_ctx = get_ns_ctx();
	int ret;
	gsl_ctx->sp_mon = 0;
	ret = memcpy_ss(ns_ctx, sizeof(*ns_ctx), gsl_ctx, sizeof(*gsl_ctx), NO_CHECK_WORD);
	if (ret != EXT_SEC_SUCCESS)
		tee_img_failure_process();

	asm(
	"mov r1, %1\n\t"
	"mov r3, %2\n\t"
	"ldr r2, =0x0\n\t"
	"blx %0\n\t"
	:: "r" (optee_os), "r" (&g_ns_ctx), "r" (tee_os_return));

	return;
}

void run_optee(void)
{
	u32 optee_addr = get_gsl_smc_ctx()->r1;
	int ret;

	if (is_soc_tee_enable() == AUTH_FAILURE) {
		tee_img_failure_process();
	} else {
		ret = load_tee_img(optee_addr);
		if (ret != EXT_SEC_SUCCESS)
			tee_img_failure_process();

		log_serial_puts((const s8 *)"core0 start TEE Image ...\n");
		start_tee_os((td_func)BL32_BASE);
	}
}
