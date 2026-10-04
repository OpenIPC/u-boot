/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
 */

#include "ddrtrn_training.h"
#include "hal/ddrtrn_hal_context.h"

#ifdef DDR_DDRT_SPECIAL_CONFIG
/* Some special DDRT need read register repeatedly */
unsigned int ddrt_reg_read(unsigned int addr)
{
	int times = 0;
	unsigned int data0, data1, data2;
	do {
		data0 = ddrtrn_reg_read(addr);
		data1 = ddrtrn_reg_read(addr);
		data2 = ddrtrn_reg_read(addr);
		times++;
	} while (((data0 != data1) || (data1 != data2)) && (times < DDRT_READ_TIMEOUT));
	if (times >= DDRT_READ_TIMEOUT) {
		ddrtrn_fatal("DDRT wait timeout");
		ddrtrn_hal_training_stat(DDR_ERR_DDRT_TIME_OUT, 0, -1, -1);
	}
	return data0;
}

/* Some special DDRT need write twice register */
void ddrt_reg_write(unsigned int data, unsigned int addr)
{
	unsigned int tmp;
	tmp = ddrtrn_reg_read(addr);
	ddrtrn_reg_write(data, addr);
	ddrtrn_reg_write(data, addr);
}
#else
/* Some special DDRT need read register repeatedly */
unsigned int ddrt_reg_read(unsigned int addr)
{
	return (*(volatile unsigned int *)((uintptr_t)addr));
}
void ddrt_reg_write(unsigned int val, unsigned int addr)
{
	(*(volatile unsigned int *)((uintptr_t)addr)) = val;
}
#endif /* DDR_DDRT_SPECIAL_CONFIG */

unsigned int ddrtrn_hal_ddrt_get_mem_width(void)
{
	return ((ddrtrn_reg_read(ddrtrn_hal_get_cur_dmc() + DDR_DMC_CFG_DDRMODE) >>
			 DMC_MEM_WIDTH_BIT) & DMC_MEM_WIDTH_MASK);
}

unsigned int ddrtrn_hal_ddrt_get_addr(void)
{
	return ddrtrn_reg_read(DDR_REG_BASE_DDRT + DDRT_ADDR);
}

#ifdef DDR_CAPAT_ADAPT_CFG
/**
  @brief Initial ddrt before capacity self-adaptation.
**/
void ddrtrn_ddrt_init_for_capacity(void)
{
	ddrt_burst_config_desc ddrt_burst_config_reg;

	ddrt_burst_config_reg.val = 0;
	ddrt_burst_config_reg.bits.ddrt_burst_size = DDRT_DATA_16BYTE;
	/* set ddrt_burst_len=1 */
	ddrt_burst_config_reg.bits.ddrt_burst_len = 0;
	ddrtrn_reg_write(ddrt_burst_config_reg.val, DDR_REG_BASE_DDRT + DDRT_BURST_CONFIG);
	if (ddrtrn_hal_get_phy_dram_type(0) == PHY_DRAMCFG_TYPE_LPDDR4) {
		ddrtrn_reg_write(DDRT_LP4_MEM_CFG, DDR_REG_BASE_DDRT + DDRT_MEM_CONFIG);
	} else {
		ddrtrn_reg_write(DDRT_NOLP4_MEM_CFG, DDR_REG_BASE_DDRT + DDRT_MEM_CONFIG);
	}
	/* set ddrt_burst_num=1 */
	ddrtrn_reg_write(0x0, DDR_REG_BASE_DDRT + DDRT_BURST_NUM);
	ddrtrn_reg_write(0xffffffff, DDR_REG_BASE_DDRT + DDRT_ADDR_NUM);
	/* set loop=1 */
	ddrtrn_reg_write(0x0, DDR_REG_BASE_DDRT + DDRT_LOOP_NUM);
}

/**
  @brief Process ddr in dword by ddrt.

  @param      addr      Address in ddr.
  @param      val       Value to be checked of writed.
  @param      mode      Ddrt mode, only write/only read/write and read.
  @param      bit_mask  Select high/low 16bit dq to check.
  @param[out] status    Result of check.
                          DDRT_RUNNING : 0x0, ddrt is runing
                          DDRT_ERROR : 0x1, ddrt is done, values ??read and written are different.
                          DDRT_NORMAL : 0x3, ddrt is done, values ??read and written are the same.
                          DDRT_TIMEOUT : 0x11, ddrt timeout.
**/
void ddrtrn_ddrt_process_dword(
	unsigned int val, unsigned long long addr, unsigned int mode, unsigned int bit_mask, unsigned int *status)
{
#define DDRT_CHECK_MAX_LOOP 10000000
	unsigned int cnt = 0;
	addr_in_64bit addr_64;

	addr_64.addr = addr;
	ddrtrn_reg_write((unsigned int)(addr_64.dword.high_32_bit), DDR_REG_BASE_DDRT + DDRT_DDR_BASE_ADDR);
	ddrtrn_reg_write((unsigned int)(addr_64.dword.low_32_bit), DDR_REG_BASE_DDRT + DDRT_ADDR);
	ddrtrn_reg_write(val, DDR_REG_BASE_DDRT + DDRT_REVERSED_DQ);
	ddrtrn_reg_write(mode, DDR_REG_BASE_DDRT + DDRT_OP);

	while (cnt < DDRT_CHECK_MAX_LOOP && ddrtrn_reg_read(DDR_REG_BASE_DDRT + DDRT_STATUS) == 0) {
		cnt++;
	}
	if (cnt >= DDRT_CHECK_MAX_LOOP) {
		*status = DDRT_TIMEOUT;
		return;
	}

	if (bit_mask == DDRT_MASK_LOW_16BIT || bit_mask == DDRT_MASK_HIGH_16BIT) {
		if (ddrtrn_reg_read(DDR_REG_BASE_DDRT + DDRT_DQ_ERR_CNT0 + DDRT_DQ_ERR_OFFSET * bit_mask) != 0 ||
			ddrtrn_reg_read(DDR_REG_BASE_DDRT + DDRT_DQ_ERR_CNT1 + DDRT_DQ_ERR_OFFSET * bit_mask) != 0 ||
			ddrtrn_reg_read(DDR_REG_BASE_DDRT + DDRT_DQ_ERR_CNT2 + DDRT_DQ_ERR_OFFSET * bit_mask) != 0 ||
			ddrtrn_reg_read(DDR_REG_BASE_DDRT + DDRT_DQ_ERR_CNT3 + DDRT_DQ_ERR_OFFSET * bit_mask) != 0) {
			*status = DDRT_ERROR;
		} else {
			*status = DDRT_NORMAL;
		}
	} else {
		*status = ddrtrn_reg_read(DDR_REG_BASE_DDRT + DDRT_STATUS);
	}
	return;
}
#endif /* DDR_CAPAT_ADAPT_CFG */
