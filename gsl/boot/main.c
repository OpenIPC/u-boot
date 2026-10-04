/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
 */
#include "types.h"
#include "common.h"
#include "lib.h"
#include "platform.h"
#include "flash_map.h"
#include "checkup.h"
#include "board_type.h"
#include "svb.h"
#include "backup.h"
#include "cipher.h"
#include "securecutil.h"
#include "share_drivers.h"
#include "err_print.h"
#include "../drivers/otp/otp.h"
#include "../drivers/ddr_init/init_regs.h"
#include "../drivers/ddr_init/include/ddrtrn_api.h"
#include "../drivers/uart/uart.h"
#include "../drivers/tzasc/tzasc.h"
#include "../drivers/otp/otp.h"
#include "../share_drivers/include/soc_errno.h"
#include "cipher_romable.h"

#ifdef CFG_DEBUG_INFO
#include "../debug/debug.h"
#endif
#include <time_stamp_index.h>
#include "smc_entry.h"

#define EMMC_READ_BLOCK_MAX  10
#define SHIFT_OFF	     32
#define REG_LOW_16BIT	     0x0000ffff

#define TEGRA_SMC_GSL_FUNCID 0xc2fffa00

unsigned long jump_addr;
static backup_img_params_s g_backup_params;
unsigned long __stack_chk_guard;
typedef td_void (*func)(td_void);

#ifndef LLVM_COMPILER
#pragma GCC push_options
#pragma GCC optimize("-fno-stack-protector")
void __stack_chk_fail(void)
{
	err_print(STACK_CHK_FAIL);
	call_reset();
}

#ifdef __LP64__
void stack_chk_guard_setup()
{
	unsigned random = 0;
	(void)uapi_drv_cipher_trng_get_random(&random);
	__stack_chk_guard = random;
	__stack_chk_guard <<= SHIFT_OFF;
	(void)uapi_drv_cipher_trng_get_random(&random);
	__stack_chk_guard |= random;
}
#endif

#ifdef __LP32__
void stack_chk_guard_setup()
{
	unsigned random = 0;
	(void)uapi_drv_cipher_trng_get_random(&random);
	__stack_chk_guard = random;
}
#endif

#pragma GCC pop_options
#else
__attribute__((no_stack_protector)) void __stack_chk_fail(void)
{
	err_print(STACK_CHK_FAIL);
	call_reset();
}

__attribute__((no_stack_protector)) void stack_chk_guard_setup()
{
	unsigned random = 0;
	(void)uapi_drv_cipher_trng_get_random(&random);
	__stack_chk_guard = random;
	__stack_chk_guard <<= SHIFT_OFF;
}
#endif

__attribute__((naked))
__attribute__((__optimize__("-fno-stack-protector")))
void start_u_boot_at_sec_mode(void)
{
	td_func jump_func = (void *)jump_addr;
	jump_func();
}

__attribute__((naked)) __attribute__((__section__(".text.ddr")))
__attribute__((__optimize__("-fno-stack-protector")))
void start_u_boot_at_nsmode(void *gsl_ctx)
{
	// thumb32 instruction
	asm("str sp, [%1, %[sp_mon_offset]]\n\t"
	    "mov sp, %1\n\t"
	    "ldr %1, =0\n\t"

	    "mrc p15, 0, r0, c1, c1, 0\n\t"
	    "orr r0, r0, #1\n\t" // set ns bit
	    "mcr p15, 0, r0, c1, c1, 0\n\t"

	    "cps #" xstr(ARM_SVC_MOD) "\n\t"
				      "mov r14, %0\n\t"
				      "subs pc, lr, #0" ::"r"(jump_addr),
	    "r"(gsl_ctx), [sp_mon_offset] "I"(CTX_SP_MON_OFFSET));
}

static u8 get_self_boot_flag(void)
{
	return (u8)reg_getbits(SELF_BOOT_FLAG_REG_ADDR,
			       SELF_BOOT_DEV_REG_BIT_OFFSET,
			       SELF_BOOT_DEV_REG_BIT_WIDTH);
}

void err_print(u8 err_idx)
{
	if (err_idx >= MAX_ERR_BITS)
		return;
	/* print to uart */
	s8 err_str[MAX_STR_LEN + 1] = { (err_idx / DIV_100) % DIV_10 + '0',
					(err_idx / DIV_10) % DIV_10 + '0',
					(err_idx % DIV_10 + '0'), '\0' };
	u8 i = 0;
	for (i = 0; i < MAX_STR_LEN; i++) {
		if (err_str[i] != '0')
			break;
	}
	serial_putc(REG_UART0_BASE, '\n');
	if (i == MAX_STR_LEN)
		serial_putc(REG_UART0_BASE, '0');
	else
		log_serial_puts(&err_str[i]);
	mdelay(ERR_PRINT_DELAY_MS);
}

static void failure_process(failure_source_type type)
{
	u32 val;

	if (get_self_boot_flag() == SELF_BOOT_FLASH) {
		if (opt_get_boot_backup_enable() == BACKUP_ENABLE) {
			if (get_verify_backup_img_reg() == BOOT_FROM_PRIME) {
				/* Set verify_backup_img flag to BOOT_FROM_BACKUP */
				set_verify_backup_img_reg(BOOT_FROM_BACKUP);
				goto clean_boot_img_addr_size_reg;
			}
		}
	}

	/* Set failure_source */
	log_serial_puts((const s8 *)"\n\rG:Boot failed");
	val = reg_get(TEE_FAILURE_SOURCE);
	val = set_tee_failure_source_type(val, type);
	reg_set(TEE_FAILURE_SOURCE, val);

	set_verify_backup_img_reg(BOOT_FROM_PRIME);
clean_boot_img_addr_size_reg:
	clean_boot_img_addr_size();
	log_serial_puts((const s8 *)"\n\rG:soft rst");
	call_reset();
}

static ALWAYS_INLINE void fun_redun_check(int ret, int err,
					  failure_source_type type)
{
	for (int i = 0; i < REDUN_CHECK_CNT; i++) {
		if (ret != EXT_SEC_SUCCESS) {
			err_print(err);
			failure_process(type);
		}
	}
}

static void sec_module_init(void)
{
	bootrom_drv_func_list func_list;

	func_list.malloc = (bootrom_func_malloc)malloc;
	func_list.free = free;
	func_list.serial_putc = (bootrom_func_serial_putc)serial_putc;

	bootrom_drv_register_func(&func_list);

	ext_drv_cipher_deinit();
	ext_drv_cipher_init();
}

static void start_flow_prepare(void)
{
	malloc_init(get_gsl_heap_addr(), MALLOC_SIZE);
	timer_init();
	sec_module_init();
	return;
}

static void configure_ddr_parameters(void)
{
	u32 tables_base = get_ddr_param_data_addr();
	ddr_table_info *boot_table_head_addr =
		(ddr_table_info *)(uintptr_t)tables_base;
	if (boot_table_head_addr->magic == BOOT_TABLE_MAGIC) {
		log_serial_puts((const s8 *)"\nboot table version     :");
		log_serial_puts(
			(const s8 *)(boot_table_head_addr->ddr_table_version));
		log_serial_puts((const s8 *)"\nboot table build time  :");
		log_serial_puts(
			(const s8 *)boot_table_head_addr->ddr_table_gen_time);
		log_serial_puts((const s8 *)"\nboot table file name   :");
		log_serial_puts(
			(const s8 *)boot_table_head_addr->ddr_table_name);
		tables_base += HEADINFO_LENTH;
	}
	init_registers(tables_base, 0);
}

#ifdef DDR_SCRAMB_ENABLE
static u32 get_random_num(void)
{
	u32 reg_val;
	u32 data_cnt;

	do {
		reg_val = reg_get(TRNG_DATA_ST_REG);
		data_cnt = reg_val & TRNG_FIFO_DATA_CNT_MASK;
	} while (data_cnt == 0);

	reg_val = reg_get(TRNG_DSTA_FIFO_DATA_REG);
	return reg_val;
}

static void ddr_scramb_update(void)
{
	reg_set(DDRCA_REE_UPDATE_REG, 0x0);
	reg_set(DDRCA_REE_UPDATE_REG, 0x1);
}

static void ddr_scramb_start(void)
{
	reg_set(DDRCA_REE_RANDOM_L_REG, get_random_num());
	reg_set(DDRCA_REE_RANDOM_H_REG, get_random_num());
	reg_set(DDRCA_EN_REG, SCRAMB_BYPASS_DIS);
	ddr_scramb_update();
	reg_set(DDRCA_LOCK_REG, SCRAMB_LOCK);

	/* clear the old random value */
	get_random_num();
}

#define DDR_SCRUB_AND_GPLL_SWITHC_OFFSET       5
#define DDR_SCRUB_AND_GPLL_SWITHC_WIDTH                1
#define OTP_SHADOWED_ATE_CHIP_VER              (OTP_SHADOW_BASE + 0x0124)

#define OPEN                                   1

static void ddr_scrambling(void)
{
	u32 status;
	u32 dmc0_isvalid = 0;

	/*
	 * read ddrc_cfg_ddrmode register,
	 * if bit[3:0] is not 0x0 ,
	 * the channel is valid.
	 */
	dmc0_isvalid = (reg_get(DMC0_DDRC_CFG_DDRMODE_REG) & 0xf) != 0;
	/* set ddrc to enter self-refresh */
	if (dmc0_isvalid)
		reg_set(DMC0_DDRC_CTRL_SREF_REG, DDRC_SREF_REQ);

	/* wait the status of ddrc to be self-refresh (status == 1). */
	do {
		status = dmc0_isvalid ? (reg_get(DMC0_DDRC_CURR_FUNC_REG) & 0x1) : 1;
	} while (status != 1);

	/* start ddr scrambling */
	ddr_scramb_start();

	/* set ddrc to exit self-refresh */
	if (dmc0_isvalid)
		reg_set(DMC0_DDRC_CTRL_SREF_REG, DDRC_SREF_DONE);
	/* wait the status of ddrc to be normal (status == 0). */
	do {
		status = 0;
		status |= dmc0_isvalid ? (reg_get(DMC0_DDRC_CURR_FUNC_REG) & 0x1) : 0;
	} while (status != 0);

	return;
}
#endif

#ifdef DDR_SCRAMB_ENABLE
static void ddr_scramb_turn_off(void)
{
	reg_set(DDRCA_EN_REG, SCRAMB_BYPASS);
	ddr_scramb_update();
}
#endif

#define DDR_MEM_RET_REG 0x11148508
#define STR_MEM_RET 0x46
#define END_MEM_RET 0x47

#define INVALID_PARAM 0xffffffff
#define END_OF_PARA_LIST  0x0

#ifndef CFG_FPGA_VERIFY
#define MAX_PARAM 20
const unsigned int params_list[2][4][MAX_PARAM][2] = {
	[0b0] = {
		[0x0] = {
			{END_OF_PARA_LIST, 0x0},
		},
		[0x1] = {
			{0x11150070, 0x20400A09},
			{0x11150074, 0x80000000},
			{0x11151030, 0xE8000401},
			{END_OF_PARA_LIST, 0x0},
		},
		[0x2] = {
			{INVALID_PARAM, 0x0},
		},
		[0x3] = {
			{INVALID_PARAM, 0x0}
		},
	},
	[0b1] = {
		[0x0] = {
			{END_OF_PARA_LIST, 0x0}
		},
		[0x1] = {
			{0x11148100, 0x34294A10},
			{0x11148104, 0x19400607},
			{0x11148108, 0x3300F028},
			{0x1114810c, 0x00000023},
			{0x11148110, 0x00000000},
			{0x11148114, 0x00000005},
			{0x11148118, 0x00000000},
			{0x11148120, 0x01600005},
			{0x11150030, 0x28883321},
			{0x11150064, 0x00000173},
			{0x11150070, 0x10200305},
			{0x11150274, 0x00000014},
			{0x111502f4, 0x00000014},
			{0x1115125c, 0x00000080},
			{END_OF_PARA_LIST, 0x0},
		},
		[0x2] = {
			{INVALID_PARAM, 0x0}
		},
		[0x3] = {
			{INVALID_PARAM, 0x0}
		},
	},
};

#define PARAM_ADDR 0x0
#define PARAM_VAL 0x1

#define OTP_REG_DDR_PARAM_EXT_ADDR 0x101e0020

typedef union {
	struct {
		unsigned int reserved0 : 16;  /* [15:0] */
		unsigned int otp_param0 : 1;  /* [16] */
		unsigned int otp_param1 : 3;  /* [19:17] */
		unsigned int otp_param2 : 4;  /* [23:20] */
	} bits;
	unsigned int val;
} ddr_parameter;

void add_ddr_parameters(void)
{
	ddr_parameter param = (ddr_parameter)reg_get(OTP_REG_DDR_PARAM_EXT_ADDR);
	unsigned int p1 = param.bits.otp_param1;
	unsigned int p2 = param.bits.otp_param2;

	if (param.bits.otp_param0 == 1)
		return;

	for (int i = 0; i < MAX_PARAM; i++) {
		unsigned int addr = params_list[p1][p2][i][PARAM_ADDR];
		unsigned int val = params_list[p1][p2][i][PARAM_VAL];
		if (addr == INVALID_PARAM) {
			log_serial_puts((const s8 *)"\nddr param err, update gsl!!!");
			call_reset();
		}

		if (addr == END_OF_PARA_LIST)
			return;

		reg_set(addr, val);
	}
}
#endif

static void system_init(u32 channel_type)
{
	unsigned int ddr_size = 0;

	reg_set(DDR_MEM_RET_REG, STR_MEM_RET);

	/* config ddr table params */
	configure_ddr_parameters();

#ifndef CFG_FPGA_VERIFY
	add_ddr_parameters();
#endif

#ifdef DDR_SCRAMB_ENABLE
	/* turn off ddr scramb */
	 ddr_scramb_turn_off();
#endif

	/* SVB */
	start_svb();

	reg_set(DDR_MEM_RET_REG, END_MEM_RET);

#ifndef CFG_FPGA_VERIFY
	/* ddr tranning */
	ddrtrn_start(&ddr_size);
#endif

	end_svb();

#ifdef DDR_SCRAMB_ENABLE
	/* ddr scrambling */
	ddr_scrambling();
#endif

#ifndef CFG_FPGA_VERIFY
	/* ddr enter low power mode */
	bsp_ddrtrn_enter_lp_mode();
#endif
}

static u32 get_data_channel_type(void)
{
	u32 channel_type;
	channel_type = reg_get(REG_SYSCTRL_BASE + DATA_CHANNEL_TYPE_REG);
	switch (channel_type) {
	case BOOT_SEL_SDIO:
	case BOOT_SEL_USB:
	case BOOT_SEL_UART:
	case BOOT_SEL_FLASH:
	case BOOT_SEL_EMMC:
		break;
	default:
		err_print(GET_DATA_CHANNEL_TYPE_UNKNOW);
		channel_type = BOOT_SEL_UNKNOW;
		break;
	}
	return channel_type;
}

static void get_img_backup_params(backup_img_params_s *backup_params)
{
	u32 boot_zone;
	memset_s(backup_params, sizeof(backup_img_params_s), 0,
		 sizeof(backup_img_params_s));
	boot_zone = get_verify_backup_img_reg();
	if ((get_self_boot_flag() == SELF_BOOT_FLASH) &&
	    (opt_get_boot_backup_enable() == BACKUP_ENABLE) &&
	    (boot_zone != BOOT_FROM_PRIME)) {
		if (reg_get(BOOT_IMG_ADDR_REG_ADDR) != 0) {
			backup_params->offset_addr =
				reg_get(BOOT_IMG_ADDR_REG_ADDR);
			backup_params->enable = 1;
		}
	}
}

#ifdef CFG_DEBUG_INFO
#ifdef __LP64__
static u32 print_current_el(u32 core)
{
	size_t current_el;
	asm volatile("mrs %0, CurrentEL\n\t" : "=r"(current_el) : : "memory");
	size_t el = (current_el >> CURRENT_EL_SHIFT) & CURRENT_EL_MASK;
	log_serial_puts((const s8 *)"\ncore");
	serial_put_dec(core);
	log_serial_puts((const s8 *)" running:");
	serial_put_dec(el);
	log_serial_puts((const s8 *)"\n");
	return el;
}
#else
static u32 print_current_el(u32 core)
{
	return 0;
}
#endif
#endif

void sram_to_npu_info(void)
{
	log_serial_puts((const s8 *)"\nsram to npu!\n");
}

__attribute__((__section__(".text.ddr"))) static void set_sram_secure_lock(void)
{
	reg_set(SEC_BOOTRAM_SEC_CFG_LOCK1, 1);
}

__attribute__((__section__(".text.ddr"))) static void
set_sram_secure_region(u32 end)
{
	u32 secure_region_size;
	u32 value;
	secure_region_size = end - SRAM_BASE;
	if (secure_region_size % SIZE_1K != 0)
		secure_region_size += SIZE_1K;
	secure_region_size = secure_region_size / SIZE_1K;
	value = reg_get(SEC_BOOTRAM_SEC_CFG) & 0xffffff00;
	value |= secure_region_size;
	reg_set(SEC_BOOTRAM_SEC_CFG, value);
	value = reg_get(SEC_BOOTRAM_SEC_CFG) & 0x000000ff;
	if (value != secure_region_size) {
		log_serial_puts((const s8 *)"secure sram set error\n");
		call_reset();
	}
}

typedef union {
	struct {
		td_u32 soc_tee_verify_enable : 4; /* [3:0] */
		td_u32 reserved		     : 28; /* [4:32] */
	} bits;
	td_u32 u32;
} otp_4bit_aligned_lockable_1; /* 0x8 */

#define OTP_4BIT_ALIGNED_LOCKABLE_1 (OTP_SHADOW_BASE + 0x0008)

#define SCS_FINISH_ADDR			    (REG_BASE_CA_MISC + SCS_CTRL) /* b[3:0]) */
#define SCS_FINISH_ADDR_BIT_OFFSET	    (0x0)
#define SCS_FINISH_ADDR_BIT_WIDTH	    (0x4)
#define SCS_IS_FINISH			    (0x5)
#define SCS_IS_NOT_FINISH		    (0xA)

void set_scs_finish(void)
{
	reg_setbits(SCS_FINISH_ADDR, SCS_FINISH_ADDR_BIT_OFFSET,
		    SCS_FINISH_ADDR_BIT_WIDTH, SCS_IS_FINISH);
}

#define REG_DMA_SEC 0x10270900
#define DMA_SEC 0x0

static void set_dma_lock(void)
{
	reg_set(REG_DMA_SEC, DMA_SEC);
}

#define REG_DDRT_SEC 0x17950800
#define DDRT_SEC 0x00611111

static void set_ddrt_lock(void)
{
	reg_set(REG_DDRT_SEC, DDRT_SEC);
}

static void load_and_start_uboot(void)
{
	otp_bit_aligned_lockable otp_value;
	otp_4bit_aligned_lockable_1 otp_4bit_aligned_lockable_1;

	lock_ree_bootkey();
	otp_value.u32 = reg_get(OTP_BIT_ALIGNED_LOCKABLE);
	otp_4bit_aligned_lockable_1.u32 = reg_get(OTP_BIT_ALIGNED_LOCKABLE);
	if (is_soc_tee_enable() != AUTH_FAILURE) {
		set_dma_lock();
		set_ddrt_lock();
		set_sram_secure_region(SRAM_END - SIZE_1K);
		set_sram_secure_lock();

		if (otp_value.bits.tee_owner_sel == 0x0 &&
		    otp_4bit_aligned_lockable_1.bits.soc_tee_verify_enable !=
			    0x0) {
			set_scs_finish();
		}

		start_u_boot_at_nsmode((void *)get_gsl_smc_ctx());
	} else {
		start_u_boot_at_sec_mode();
	}
}

static ALWAYS_INLINE void store_booting_param(void)
{
	u32 boot_code_addr;
	uboot_code_info *uboot_info = NULL;
	uboot_info = (uboot_code_info *)get_uboot_info_download_ddr_addr();
	boot_code_addr = uboot_info->uboot_entry_point;
	/* The DDR is not initialized in the U-boot */
	reg_set(REG_SYSCTRL_BASE + REG_SC_GEN4, 0);
	jump_addr = (long unsigned int)(void *)boot_code_addr;
}

#ifdef CFG_DEBUG_INFO
static void print_addr_size(u32 addr, u32 size, const s8 *s)
{
	log_serial_puts(s);
	serial_put_hex(addr);
	log_serial_puts((const s8 *)" size:0x");
	serial_put_hex(size);
}

static void dump_addr_info()
{
	print_addr_size(get_gsl_code_info_addr(), GSL_CODE_INFO_SIZE,
			(const s8 *)"\n\ngsl info addr  :0x");
	print_addr_size(get_gsl_code_addr(), get_gsl_code_area_len(),
			(const s8 *)"\ngsl code addr  :0x");
	print_addr_size(get_gsl_heap_addr(), get_gsl_heap_len(),
			(const s8 *)"\ngsl heap addr  :0x");
	print_addr_size(get_ree_key_area_addr(), REE_BOOT_KEY_AREA_SIZE,
			(const s8 *)"\nree key  addr  :0x");
	print_addr_size(get_ddr_param_info_addr(), PARM_AREA_INFO_SIZE,
			(const s8 *)"\nparam info addr:0x");
	log_serial_puts((const s8 *)"\nparam max num  :0x");
	serial_put_hex(get_ddr_param_cnt());
	print_addr_size(get_ddr_param_data_addr(), get_ddr_param_len(),
			(const s8 *)"\nparam data addr:0x");
	print_addr_size(get_uboot_info_download_ddr_addr(),
			UBOOT_CODE_INFO_SIZE,
			(const s8 *)"\nuboot info addr:0x");
	print_addr_size(get_uboot_code_ddr_addr(), get_uboot_code_size(),
			(const s8 *)"\nuboot info addr:0x");
	log_serial_puts((const s8 *)"\nuboot entry    :0x");
	serial_put_hex(get_uboot_entrypoint_ddr_addr());
}
#endif

static int copy_uboot_code_to_entry_point(void)
{
	int ret = EXT_SEC_FAILURE;
	u32 boot_img_int_ddr_addr = get_uboot_info_download_ddr_addr();
	uboot_code_info *uboot_info =
		(uboot_code_info *)(uintptr_t)boot_img_int_ddr_addr;
	u32 dst_len = uboot_info->code_area_len;
	ret = dma_copy(uboot_info->uboot_entry_point, dst_len,
		       boot_img_int_ddr_addr + UBOOT_CODE_INFO_SIZE);
	if (ret != EXT_SEC_SUCCESS) {
		log_serial_puts((const s8 *)"\ncopy_uboot_code_to_entry_point fail");
		return EXT_SEC_FAILURE;
	}
	return EXT_SEC_SUCCESS;
}

static int copy_ree_key_to_entry_point(void)
{
	errno_t err;
	u32 boot_img_int_ddr_addr = get_uboot_info_download_ddr_addr();
	uboot_code_info *uboot_info =
		(uboot_code_info *)(uintptr_t)boot_img_int_ddr_addr;
	u32 dst_len;
	u32 dst;
	u32 src_len;

	dst_len = UBOOT_CODE_INFO_SIZE;
	dst = uboot_info->uboot_entry_point - UBOOT_CODE_INFO_SIZE;
	src_len = sizeof(uboot_code_info);
	err = memcpy_s((void *)(uintptr_t)dst, dst_len,
		       (void *)(uintptr_t)(uboot_info), src_len);
	if (err != EOK)
		return EXT_SEC_FAILURE;
	dst_len = REE_BOOT_KEY_AREA_SIZE;
	dst = uboot_info->uboot_entry_point - UBOOT_CODE_INFO_SIZE -
	      REE_BOOT_KEY_AREA_SIZE;
	src_len = sizeof(ree_key_area_s);
	err = memcpy_s((void *)(uintptr_t)dst, dst_len,
		       (void *)(uintptr_t)(get_ree_key_area_addr()), src_len);
	if (err != EOK)
		return EXT_SEC_FAILURE;
	return EXT_SEC_SUCCESS;
}

#define WDG_SYS_TIMEOUT (1000) /* 1s */
#define WDG_CLK_3MHZ	3000000

static int first_recv_len_check(size_t lenth)
{
	para_area_info *para_info =
		(para_area_info *)(uintptr_t)get_ddr_param_info_addr();
	int right_len = REE_BOOT_KEY_AREA_SIZE + PARM_AREA_INFO_SIZE +
			para_info->para_area_addr +
			para_info->para_area_len * 1;
	if (lenth == right_len)
		return EXT_SEC_SUCCESS;
	err_print(FIRST_RECV_LEN_CHECK_ERR);
	return EXT_SEC_FAILURE;
}

static int second_recv_len_check(size_t lenth)
{
	uboot_code_info *uboot_info = (uboot_code_info *)(uintptr_t)
		get_uboot_info_download_ddr_addr();
	int right_len = UBOOT_CODE_INFO_SIZE + uboot_info->code_area_len;
	if (lenth == right_len)
		return EXT_SEC_SUCCESS;
	err_print(SECOND_RECV_LEN_CHECK_ERR);
	return EXT_SEC_FAILURE;
}

static int get_ree_key_and_param_from_uart(void)
{
	size_t uart_rcv_len;
	void *val = (void *)(uintptr_t)get_ree_key_area_addr();
	int ret = EXT_SEC_FAILURE;
	ret = copy_from_uart(val, &uart_rcv_len, FIRST_RECIVE_MAX_LEN);
	if (ret != EXT_SEC_SUCCESS) {
		err_print(GET_REE_KEY_AND_PARAM_FROM_UART_ERR);
		return EXT_SEC_FAILURE;
	}
	if (first_recv_len_check(uart_rcv_len) != EXT_SEC_SUCCESS) {
		return EXT_SEC_FAILURE;
	}
	return EXT_SEC_SUCCESS;
}
#define BOARD_TYPE_FRAME_SEND_LEN   11
#define BOARD_TYPE_RCV_LEN	    14
#define BOARD_TYPE_CRC_LEN	    2
#define BOARD_TYPE_PAYLOAD_SEND_LEN 8
#define FRAM_CRC_H		    0x8
#define FRAM_OFF0		    0x0
#define FTAM_OFF1		    0x1
#define FRAM_OFF2		    0x2
#define FRAM_CRC_H_OFF		    (BOARD_TYPE_RCV_LEN - 1 - 1)
#define FRAM_CRC_L_OFF		    (BOARD_TYPE_RCV_LEN - 1)
#define CRC_LEN			    (0x2)
#define TIMEOUT_USB		    2000
#define CRC_NUMS		    0xFFFF
#define WAIT_USB_ACK_COMPL_DELAY    1

#include "uart.h"
#include "burn_protocol.h"
#include "crc.h"
static int send_board_type_to_usb(u32 type)
{
	frame_info frame;
	unsigned char board_type_frame_send_buf[BOARD_TYPE_FRAME_SEND_LEN];
	unsigned char board_type_frame_rcv_buf[BOARD_TYPE_RCV_LEN];
	int i;
	size_t usb_rcv_len = 0;
	volatile int ret = EXT_SEC_FAILURE;
	unsigned short crc = 0;
	unsigned short rev_crc;
	if (memset_s(&frame, sizeof(frame_info), 0, sizeof(frame_info)) != EOK)
		return EXT_SEC_FAILURE;
	usb3_driver_init(); /* re init */
	int crc_nums = CRC_NUMS;
	while (1) {
		if (crc_nums < 0) {
			log_serial_puts((const s8 *)"connect timeout\n");
			return EXT_SEC_FAILURE;
		}
		crc_nums--;
		ret = copy_from_usb((void *)board_type_frame_rcv_buf,
				    &usb_rcv_len, BOARD_TYPE_RCV_LEN);
		if (ret != EXT_SEC_SUCCESS)
			continue;
		if (usb_rcv_len != BOARD_TYPE_RCV_LEN)
			continue;
		if (board_type_frame_rcv_buf[FRAM_OFF0] != XBOARD ||
				(board_type_frame_rcv_buf[FTAM_OFF1] !=
				(u8)(~board_type_frame_rcv_buf[FRAM_OFF2])))
			continue;
		mdelay(WAIT_USB_ACK_COMPL_DELAY);
		rev_crc = (unsigned short)((board_type_frame_rcv_buf[FRAM_CRC_H_OFF] << FRAM_CRC_H) |
					 (board_type_frame_rcv_buf[FRAM_CRC_L_OFF]));
		crc = 0;
		for (i = 0; i < BOARD_TYPE_RCV_LEN - CRC_LEN; i++) {
			crc = cal_crc_perbyte(board_type_frame_rcv_buf[i], crc);
		}
		if (rev_crc == crc) {
			(void)build_board_type_frame(type, board_type_frame_send_buf,
						     BOARD_TYPE_FRAME_SEND_LEN);
			send_to_usb((char *)board_type_frame_send_buf,
				    BOARD_TYPE_FRAME_SEND_LEN);
			log_serial_puts(
				(const s8 *)"send_board_type_to_usb ok\n");
			return EXT_SEC_SUCCESS;
		}
	}
	return EXT_SEC_FAILURE;
}

static int get_ree_key_and_param_from_usb(void)
{
	size_t usb_rcv_len;
	int ret;
	ret = copy_from_usb((void *)(uintptr_t)(get_ree_key_area_addr()),
			    &usb_rcv_len, FIRST_RECIVE_MAX_LEN);
	if (ret != EXT_SEC_SUCCESS) {
		err_print(GET_REE_KEY_AND_PARAM_FROM_USB_ERR);
		return EXT_SEC_FAILURE;
	}
	if (first_recv_len_check(usb_rcv_len) != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;
	return EXT_SEC_SUCCESS;
}

static int get_uboot_info_and_code_from_usb(void)
{
	u32 img_int_ddr_addr;
	size_t usb_rcv_len;
	int ret = EXT_SEC_FAILURE;
	img_int_ddr_addr = get_uboot_info_download_ddr_addr();
	ret = copy_from_usb((void *)(uintptr_t)img_int_ddr_addr, &usb_rcv_len,
			    SECOND_RECIVE_MAX_LEN);
	if (ret != EXT_SEC_SUCCESS) {
		err_print(GET_UBOOT_INFO_AND_CODE_FROM_USB_ERR);
		return EXT_SEC_FAILURE;
	}
	if ((second_recv_len_check(usb_rcv_len) != EXT_SEC_SUCCESS))
		return EXT_SEC_FAILURE;
	return EXT_SEC_SUCCESS;
}

static int get_uboot_info_and_code_from_uart(void)
{
	u32 img_int_ddr_addr;
	size_t img_total_len = 0;
	int ret = EXT_SEC_FAILURE;
	img_int_ddr_addr = get_uboot_info_download_ddr_addr();
	ret = copy_from_uart((void *)(uintptr_t)img_int_ddr_addr,
			     &img_total_len, SECOND_RECIVE_MAX_LEN);
	if (ret != EXT_SEC_SUCCESS) {
		err_print(GET_UBOOT_INFO_AND_CODE_FROM_UART_ERR);
		return EXT_SEC_FAILURE;
	}
	if ((second_recv_len_check(img_total_len) != EXT_SEC_SUCCESS))
		return EXT_SEC_FAILURE;
	return EXT_SEC_SUCCESS;
}

static void handle_ree_key_and_param(u32 channel_type, u32 board_param_index)
{
	int ret = EXT_SEC_FAILURE;
	if (channel_type == BOOT_SEL_UART || channel_type == BOOT_SEL_USB) {
		if (channel_type == BOOT_SEL_UART) {
			ret = send_board_type_to_uart(board_param_index);
			if (ret != EXT_SEC_SUCCESS)
				failure_process(LOAD_IMG_FROM_DEVICE_FAILED);
			ret = get_ree_key_and_param_from_uart();
		} else if (channel_type == BOOT_SEL_USB) {
			ret = send_board_type_to_usb(board_param_index);
			if (ret != EXT_SEC_SUCCESS)
				failure_process(LOAD_IMG_FROM_DEVICE_FAILED);
			ret = get_ree_key_and_param_from_usb();
		}
		if (ret != EXT_SEC_SUCCESS)
			failure_process(LOAD_IMG_FROM_DEVICE_FAILED);
	} else if (channel_type == BOOT_SEL_FLASH ||
		   channel_type == BOOT_SEL_EMMC ||
		   channel_type == BOOT_SEL_SDIO) {
		get_img_backup_params(&g_backup_params);
		ret = get_ree_key_and_paras_info_from_device(&g_backup_params,
							     channel_type);
		if (ret != EXT_SEC_SUCCESS)
			failure_process(VERIFY_PARAMS_FAILED);
	} else {
		call_reset();
	}

	fun_redun_check(handle_ree_key_area(), HANDLE_REE_KEY_ERR,
			VERIFY_REE_KEY_FAILED);
	fun_redun_check(handle_ddr_param_info(board_param_index),
			HANDLE_PARAM_AREA_INFO_ERR, VERIFY_PARAMS_FAILED);
	if (channel_type == BOOT_SEL_FLASH || channel_type == BOOT_SEL_EMMC ||
	    channel_type == BOOT_SEL_SDIO) {
		ret = get_paras_data_from_flash(
			&g_backup_params, board_param_index, channel_type);
		if (ret != EXT_SEC_SUCCESS)
			failure_process(VERIFY_PARAMS_FAILED);
	}

	fun_redun_check(handle_ddr_param(board_param_index),
			HANDLE_PARAM_AREA_ERR, VERIFY_PARAMS_FAILED);
}

static void handle_uboot(u32 channel_type)
{
	int ret = EXT_SEC_FAILURE;
	if (channel_type == BOOT_SEL_UART || channel_type == BOOT_SEL_USB) {
		if (channel_type == BOOT_SEL_UART)
			ret = get_uboot_info_and_code_from_uart();
		else if (channel_type == BOOT_SEL_USB)
			ret = get_uboot_info_and_code_from_usb();
		if (ret != EXT_SEC_SUCCESS)
			failure_process(VERIFY_UBOOT_FAILED);
	} else if (channel_type == BOOT_SEL_FLASH ||
		   channel_type == BOOT_SEL_EMMC ||
		   channel_type == BOOT_SEL_SDIO) {
		ret = get_uboot_info_from_flash(&g_backup_params, channel_type);
		if (ret != EXT_SEC_SUCCESS)
			failure_process(VERIFY_UBOOT_FAILED);
	} else {
		call_reset();
	}
	fun_redun_check(handle_uboot_info(), HANDLE_UBOOT_CODE_INFO_ERR,
			VERIFY_UBOOT_FAILED);
	if (channel_type == BOOT_SEL_UART || channel_type == BOOT_SEL_USB) {
		ret = copy_uboot_code_to_entry_point();
		if (ret != EXT_SEC_SUCCESS)
			failure_process(VERIFY_UBOOT_FAILED);
	}
	if (channel_type == BOOT_SEL_FLASH || channel_type == BOOT_SEL_EMMC ||
	    channel_type == BOOT_SEL_SDIO) {
		ret = get_uboot_code_from_flash(&g_backup_params, channel_type);
		if (ret != EXT_SEC_SUCCESS)
			failure_process(VERIFY_PARAMS_FAILED);
	}

	fun_redun_check(handle_uboot_code(), HANDLE_UBOOT_CODE_ERR,
			VERIFY_UBOOT_FAILED);
}

#define SCR_EL3_RW_AARCH64		(1 << 10) /* Next lower level is AArch64     */
#define SCR_EL3_RW_AARCH32		(0 << 10) /* Lower lowers level are AArch32  */
#define SCR_EL3_HCE_EN			(1 << 8) /* Hypervisor Call enable          */
#define SCR_EL3_SMD_DIS			(1 << 7) /* Secure Monitor Call disable     */
#define SCR_EL3_RES1			(3 << 4) /* Reserved, RES1                  */
#define SCR_EL3_EA_EN			(1 << 3) /* External aborts taken to EL3    */
#define SCR_NS_BIT			(1 << 0) /* EL0 and EL1 in Non-scure state  */

#define SPSR_DISABLE_IRQ_FIQ_EXCEPTIONS (0b11 << 6)

#define OTP_CUSTOMER_ID_OFFSET		0x130
#define OTP_CUSTOMER_ID_LEN		0x10
#define OTP_WORD_LEN			4
#define CUSTOMER_ID_LOCK		0x1

#define REG_CUSTOMER_ID			(REG_SYSCTRL_BASE + 0x1100)
#define REG_CUSTOMER_ID_LOCK		(REG_SYSCTRL_BASE + 0x1120)

int set_customer_id_lock(void)
{
	volatile td_s32 ret = EXT_FAILURE;
	td_u32 addr = OTP_CUSTOMER_ID_OFFSET;
	u64 custom_id_addr = REG_CUSTOMER_ID;
	td_u32 word_data = 0;
	td_u32 i = 0;

	for (i = 0; i < OTP_CUSTOMER_ID_LEN; i += OTP_WORD_LEN) {
		ret = ext_drv_cipher_otp_read_word(addr, &word_data);
		if (ret != EXT_SUCCESS) {
			return ret;
		}

		reg_set(custom_id_addr, word_data);
		addr += OTP_WORD_LEN;
		custom_id_addr += OTP_WORD_LEN;
	}

	reg_set(REG_CUSTOMER_ID_LOCK, CUSTOMER_ID_LOCK);
	return EXT_SUCCESS;
}

int copy_gsl_ddr_code(void)
{
	u32 ret;
	unsigned long ddr_code_len = (unsigned long)(&_ddr_text_end) -
				     (unsigned long)(&_ddr_text_start);
	ret = memcpy_s(&_ddr_text_start, ddr_code_len, &_ddr_load_start,
		       ddr_code_len);
	if (ret != EOK) {
		return EXT_SEC_FAILURE;
	}
	return EXT_SUCCESS;
}

void main_entry(void)
{
	u32 channel_type;
	u32 board_param_index = 0;
	u32 ret;

	save_cur_point_syscnt(GSL_START_TIME);
	start_flow_prepare();

	ret = set_customer_id_lock();
	if (ret != EXT_SEC_SUCCESS) {
		failure_process(SECURE_MODE_INIT_FAILED);
	}

	board_param_index = get_board_param_index();
	channel_type = get_data_channel_type();
	if (channel_type == BOOT_SEL_UNKNOW)
		failure_process(DETECT_BOOT_DEVICE_FAILED);
#ifdef CFG_SM_SUPPORT
	ext_drv_cipher_deinit();
	gsl_drv_cipher_init();
#endif
	handle_ree_key_and_param(channel_type, board_param_index);

	system_init(channel_type);

	handle_uboot(channel_type);

#ifdef CFG_DEBUG_INFO
	dump_addr_info();
#endif
	copy_ree_key_to_entry_point();

	copy_gsl_ddr_code();

	store_booting_param();

#ifdef CFG_DEBUG_INFO
	print_current_el(0);
#endif
	save_cur_point_syscnt(ENTRY_UBOOT_TIME);
#ifdef CFG_DEBUG_INFO
	print_boot_time();
#endif
	load_and_start_uboot();
}
