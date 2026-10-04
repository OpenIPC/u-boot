/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
 */

#include "ddrtrn_training.h"

/*
 * Do some prepare before copy code from DDR to SRAM.
 * Keep empty when nothing to do.
 */
void ddrtrn_hal_cmd_prepare_copy(void)
{
	return;
}

/*
 * Save site before DDR training command execute .
 * Keep empty when nothing to do.
 */
void ddrtrn_hal_cmd_site_save(void)
{
	return;
}

/*
 * Restore site after DDR training command execute.
 * Keep empty when nothing to do.
 */
void ddrtrn_hal_cmd_site_restore(void)
{
	return;
}

static void ddrtrn_hal_anti_aging_disable(struct ddrtrn_hal_training_custom_reg *custom_reg,
										  unsigned int base_phy, unsigned int phy_idx)
{
	/* disable rdqs anti-aging */
	custom_reg->age_compst_en[phy_idx] =
		ddrtrn_reg_read(base_phy + DDR_PHY_PHYRSCTRL);
	ddrtrn_reg_write(custom_reg->age_compst_en[phy_idx] &
					 (~((unsigned int)0x1 << DDR_CFG_RX_AGE_COMPST_EN_BIT)),
					 base_phy + DDR_PHY_PHYRSCTRL);
}

/*
 * Save site before DDR training:include boot and command execute.
 * Keep empty when nothing to do.
 */
int ddrtrn_hal_boot_cmd_save(struct ddrtrn_hal_training_custom_reg *custom_reg)
{
	if (custom_reg == NULL)
		return -1;
	custom_reg->ddrt_ctrl = ddrtrn_reg_read(DDR_REG_BASE_SYSCTRL + MISC_DDRT_CTRL);
	ddrtrn_reg_write(custom_reg->ddrt_ctrl | (0x1 << DDRT_MST_SEL_BIT), DDR_REG_BASE_SYSCTRL + MISC_DDRT_CTRL);
	/* turn on ddrt clock */
	custom_reg->ddrt_clk_reg = ddrtrn_reg_read(CRG_REG_BASE_ADDR + CRG_DDRT);
	/* enable ddrt0 clock */
	ddrtrn_reg_write(custom_reg->ddrt_clk_reg | (0x1 << DDR_TEST0_CKEN_BIT), CRG_REG_BASE_ADDR + CRG_DDRT);
	ddr_asm_nop();
	/* disable ddrt0 soft reset */
	ddrtrn_reg_write(ddrtrn_reg_read(CRG_REG_BASE_ADDR + CRG_DDRT) & (~(0x1 << DDR_TEST0_SRST_REQ_BIT)),
					 CRG_REG_BASE_ADDR + CRG_DDRT);
	/* disable rdqs anti-aging */
	ddrtrn_hal_anti_aging_disable(custom_reg, DDR_REG_BASE_PHY0, 0); /* 0: phy0 */

	return 0;
}

/*
 * Restore site after DDR training:include boot and command execute.
 * Keep empty when nothing to do.
 */
void ddrtrn_hal_boot_cmd_restore(const struct ddrtrn_hal_training_custom_reg *custom_reg)
{
	if (custom_reg == NULL)
		return;
	ddrtrn_reg_write(custom_reg->ddrt_ctrl, DDR_REG_BASE_SYSCTRL + MISC_DDRT_CTRL);
	/* restore ddrt clock */
	ddrtrn_reg_write(custom_reg->ddrt_clk_reg, CRG_REG_BASE_ADDR + CRG_DDRT);
	/* restore rdqs anti-aging */
	ddrtrn_reg_write(custom_reg->age_compst_en[0], DDR_REG_BASE_PHY0 + DDR_PHY_PHYRSCTRL);
}

int ddrtrn_hw_training_init(unsigned int hw_item_mask, int low_freq_flag)
{
	int result = 0;
	unsigned int dram_type;
	struct ddrtrn_hal_training_custom_reg reg;
	ddrtrn_set_data(&reg, 0, sizeof(struct ddrtrn_hal_training_custom_reg));
	dram_type = ddrtrn_reg_read(DDR_REG_BASE_PHY0 + DDR_PHY_DRAMCFG) & PHY_DRAMCFG_TYPE_MASK;

	if (ddrtrn_hal_boot_cmd_save(&reg) != 0)
		return -1;
	if ((low_freq_flag == DDR_LOW_FREQ_START_TRUE) && (dram_type == PHY_DRAMCFG_TYPE_LPDDR4)) {
		result = ddrtrn_low_freq_start(hw_item_mask);
	} else {
		ddrtrn_hal_hw_item_cfg(hw_item_mask);
		result = ddrtrn_hw_training_if();
	}
	ddrtrn_hal_boot_cmd_restore(&reg);
	return result;
}

#ifdef DDR_READ_DDR_BY_DMA
#ifdef DDR_HSL_ADAPTION
/**
  @brief    Copy dword data with spacc dma.

  @param dest_addr    Destination address.
  @param src_addr     Source address.
  @param len          Length of the copied data.
  @param dma_type     Copy direction.

  @retval HI_SUCCESS  Copy sucess.
  @retval others      Error occur.
**/
static int ddrtrn_copy_data_with_spacc_dma(unsigned int dest_addr, unsigned int src_addr,
	unsigned int len, unsigned int dma_type)
{
	unsigned int check_sum, buffer_addr;
	int ret = HI_FAILURE;

	if (dma_type == ddr2hppram) {
		buffer_addr = dest_addr;
		dma_type = ddr2sram;
	} else if (dma_type == hppram2ddr) {
		buffer_addr = src_addr;
		dma_type = sram2ddr;
	} else {
		return ret;
	}

	/* enable mcipher to the buffer */
	check_sum = ENTRY_2 ^ buffer_addr ^ len;
	set_spacc_access_area(ENTRY_2, buffer_addr, len, check_sum);

	check_sum = dest_addr ^ src_addr ^ len ^ dma_type;
	ret = read_with_spacc_dma(dest_addr, src_addr, len, dma_type, check_sum);
	if (ret != HI_SUCCESS) {
		debug_info("ddrtrn SPACC DMA read error\n", 0);
	}

	/* disable mcipher to the buffer */
	check_sum = ENTRY_2 ^ 0 ^ 0;
	set_spacc_access_area(ENTRY_2, 0, 0, check_sum);

	return 0;
}
#endif /* DDR_HSL_ADAPTION */

#ifdef DDR_HRF_ADAPTION
static int ddrtrn_copy_data_with_spacc_dma(unsigned int dest_addr, unsigned int src_addr,
	unsigned int len, unsigned int dma_type)
{
	unsigned int check_sum;
	int ret = OSAL_FAILURE;
	if (dma_type == ddr2hppram) {
		dma_type = DDR2HPP_RAM;
	} else if (dma_type == hppram2ddr) {
		dma_type = HPP_RAM2DDR;
	} else {
		return ret;
	}

	ret = spacc_init(0);
	if (ret != OSAL_SUCCESS) {
		ddr_serial_puts("\rddrtrn_copy_data_with_spacc_dma spacc_init error.\n");
		return ret;
	}

	/* set dma_type */
	cipher_access_ram_area(src_addr, len);
	set_dma_secure_attribute(TD_FALSE, TD_TRUE);

	/* checksum and dma copy */
	check_sum = src_addr ^ dest_addr ^ dma_type ^ len;

	ret = read_with_spacc_dma(dest_addr, src_addr, len, dma_type, check_sum);
	if (ret != OSAL_SUCCESS) {
		ddr_serial_puts("\rddrtrn_copy_data_with_spacc_dma read_with_spacc_dma error.\n");
	}
	set_dma_secure_attribute(TD_TRUE, TD_TRUE);
	cipher_access_ram_area(0, 0);
	spacc_deinit();

	return 0;
}
#endif /* DDR_HRF_ADAPTION */

/**
  @brief    Read data from ddr by spacc dma.

  @param addr    Address of the data to be read in the DDR.

  @return   Data read from the DDR.
**/
unsigned int ddrtrn_read_ddr_in_dword(unsigned int addr)
{
	unsigned int val;

	if (ddrtrn_copy_data_with_spacc_dma((unsigned int)(&val), addr, sizeof(unsigned int), ddr2hppram) != 0)
		ddr_serial_puts("ddrtrn_read_ddr_with_spacc_dma error\n");

	return val;
}

/**
  @brief    Write data to ddr by spacc dma.

  @param dest_addr    Address of the data to be written in the DDR.
  @param src_addr     Address of the data to be read in the SRAM.
**/
void ddrtrn_write_ddr_in_dword(unsigned int val, unsigned int addr)
{
	if (ddrtrn_copy_data_with_spacc_dma(addr, (unsigned int)(&val), sizeof(unsigned int), hppram2ddr) != 0)
		ddr_serial_puts("ddrtrn_write_ddr_with_spacc_dma error\n");
}
#else
unsigned int ddrtrn_read_ddr_in_dword(unsigned int addr)
{
	return ddrtrn_reg_read(addr);
}
void ddrtrn_write_ddr_in_dword(unsigned int val, unsigned int addr)
{
	ddrtrn_reg_write(val, addr);
}
#endif /* DDR_READ_DDR_BY_DMA */

/*
@brief    DDR ZQ trim resistor config
 */
void ddrtrn_zq_res_trim(void)
{
	otp_rg_res_trim_desc otp_rg_res_trim_reg;

	otp_rg_res_trim_reg.val = ddrtrn_reg_read(OTP_RG_RES_TRIM);
	if ((otp_rg_res_trim_reg.bits.rg_res_trim == 0) ||
		(otp_rg_res_trim_reg.bits.rg_res_trim == DDRPHY0_OTP_RG_RES_TRIM_MASK)) {
		return;
	} else {
		ddr_phy_acioctl_sr1_desc ddr_phy_acioctl_sr1_reg;
		ddr_phy_acioctl_sr1_reg.val = ddrtrn_reg_read(DDR_REG_BASE_PHY0 + DDR_PHY_ACIOCTL_SR1);
		ddr_phy_acioctl_sr1_reg.bits.acctl_trim_res_rg = otp_rg_res_trim_reg.bits.rg_res_trim;
		ddr_phy_acioctl_sr1_reg.bits.acctl_trim_res1_rg = otp_rg_res_trim_reg.bits.rg_res_trim;
		ddrtrn_reg_write(ddr_phy_acioctl_sr1_reg.val, DDR_REG_BASE_PHY0 + DDR_PHY_ACIOCTL_SR1);
	}
}