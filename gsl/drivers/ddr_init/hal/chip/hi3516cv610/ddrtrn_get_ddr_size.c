/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
 */

#include "ddrtrn_get_ddr_size.h"
#include "ddrtrn_training_custom.h"
#include "ddrtrn_capacity_adapt.h"
#include "ddrtrn_training.h"
#include "hal/ddrtrn_hal_context.h"

#ifdef DDR_CAPAT_ADAPT_CFG
static void ddrtrn_dmc_rank_cfg(void)
{
	unsigned int rank_num, dmc_num, dmc_idx;

	rank_num = ddrtrn_hal_get_cur_phy_rank_num();
	dmc_num = ddrtrn_hal_get_cur_phy_dmc_num();

	for (dmc_idx = 0; dmc_idx < dmc_num; dmc_idx++) {
		unsigned int base_dmc = ddrtrn_hal_get_cur_dmc_addr(dmc_idx);
		unsigned int ddrmode_val = ddrtrn_reg_read(base_dmc + DDR_DMC_CFG_DDRMODE);
		if (rank_num == DDR_1_RANK) {
			/* Rank set 1 */
			ddrtrn_reg_write((ddrmode_val & (~(DMC_DDRMODE_RANK_MASK << DMC_DDRMODE_RANK_BIT))) |
				(DMC_DDRMODE_RANK_1 << DMC_DDRMODE_RANK_BIT), base_dmc + DDR_DMC_CFG_DDRMODE);
		} else if (rank_num == DDR_2_RANK) {
			/* Rank set 2 */
			ddrtrn_reg_write((ddrmode_val & (~(DMC_DDRMODE_RANK_MASK << DMC_DDRMODE_RANK_BIT))) |
				(DMC_DDRMODE_RANK_2 << DMC_DDRMODE_RANK_BIT), base_dmc + DDR_DMC_CFG_DDRMODE);
		}
	}
}

/* Identify DDR capacity by winding
 * return: ddr size (KB)
 */
static unsigned int ddrtrn_winding_identification(unsigned int step, unsigned int max_capat, unsigned int bit_mask)
{
#define DDR_10_BIT  10 /* Move 10 bits */
	unsigned int ddr_size, status, cnt = 0;

	/* dmc rank config */
	ddrtrn_dmc_rank_cfg();

	do {
		cnt++;

		/* winding */
		ddrtrn_ddrt_process_dword(DDR_WINDING_NUM1, DDRT_CFG_BASE_ADDR, DDRT_WRITE_READ, DDRT_MASK_32BIT, &status);
		if (status != DDRT_NORMAL) {
			ddr_serial_puts("\r[ddr] ddrtrn_winding_identification1 error status: 0x");
			ddr_serial_put_hex(status);
			ddr_serial_puts("\n");
			return 0;
		}

		while ((step * cnt) <= max_capat) {
			ddrtrn_ddrt_process_dword(DDR_WINDING_NUM1, (unsigned long long)DDRT_CFG_BASE_ADDR +
				(((unsigned long long)step * (unsigned long long)cnt) << DDR_10_BIT),
				DDRT_READ_ONLY, bit_mask, &status);
			if (status == DDRT_NORMAL) {
				break;
			} else if (status != DDRT_ERROR) {
				ddr_serial_puts("\r[ddr] ddrtrn_winding_identification2\n");
				return 0;
			}
			cnt++;
		}

		if ((step * cnt) > max_capat) {
			ddr_serial_puts("\r[ddr] DDR capacity identification error\n");
			return DDR_MAX_CAPAT; /* The return value is determined based on the project */
		}

		/* check winding */
		ddrtrn_ddrt_process_dword(DDR_WINDING_NUM2, DDRT_CFG_BASE_ADDR, DDRT_WRITE_READ, DDRT_MASK_32BIT, &status);
		if (status != DDRT_NORMAL) {
			ddr_serial_puts("\r[ddr] ddrtrn_winding_identification3 error status: 0x\n");
			ddr_serial_put_hex(status);
			ddr_serial_puts("\n");
			return 0;
		}

		ddrtrn_ddrt_process_dword(DDR_WINDING_NUM2, (unsigned long long)DDRT_CFG_BASE_ADDR +
			(((unsigned long long)step * (unsigned long long)cnt) << DDR_10_BIT),
			DDRT_READ_ONLY, bit_mask, &status);
		if (status == DDRT_NORMAL && cnt == 1) {
			ddr_serial_puts("\r[ddr] check winding fail\n");
			return 0; /* widing fail */
		}
	} while (status != DDRT_NORMAL);

	ddr_size = step * cnt;

	return ddr_size;
}

static void ddrtrn_store_ddr(struct ddr_data *ddr_reg_val)
{
	unsigned int i;

	for (i = 0; i < DDR_STORE_NUM; i++) {
		ddr_reg_val->reg_val[i] = ddrtrn_read_ddr_in_dword(DDRT_CFG_BASE_ADDR + DDR_REG_OFFSET * i);
	}
}

static void ddrtrn_restore_ddr(struct ddr_data *ddr_reg_val)
{
	unsigned int i;

	for (i = 0; i < DDR_STORE_NUM; i++) {
		ddrtrn_write_ddr_in_dword(ddr_reg_val->reg_val[i], DDRT_CFG_BASE_ADDR + DDR_REG_OFFSET * i);
	}
}

/* Identifying the DDR Capacity in Different Scenarios
 * DDR4: Distinguish upper and lower 16 bits
 */
static void ddrtrn_winding_identy_scence(struct ddr_capat_by_phy *phy_capat)
{
	unsigned int dmc_num = ddrtrn_hal_get_cur_phy_dmc_num();

	if (ddrtrn_hal_get_cur_phy_dram_type() != PHY_DRAMCFG_TYPE_LPDDR4) {
		for (unsigned int dmc_idx = 0; dmc_idx < dmc_num; dmc_idx++) {
			ddrtrn_hal_set_cur_dmc(ddrtrn_hal_get_cur_dmc_addr(dmc_idx));
			unsigned int mem_width = ddrtrn_hal_ddrt_get_mem_width();
			if (mem_width == MEM_WIDTH_32BIT) {
				phy_capat->capat_low16bit = (ddrtrn_winding_identification(WINDING_STEP, DDR_MAX_CAPAT,
					DDRT_MASK_LOW_16BIT) >> 1) >> DDR_10_BIT;
				phy_capat->capat_high16bit = (ddrtrn_winding_identification(WINDING_STEP, DDR_MAX_CAPAT,
					DDRT_MASK_HIGH_16BIT) >> 1) >> DDR_10_BIT;
				phy_capat->capacity = phy_capat->capat_low16bit + phy_capat->capat_high16bit;
			} else {
				phy_capat->capat_low16bit =
					ddrtrn_winding_identification(WINDING_STEP, DDR_MAX_CAPAT, DDRT_MASK_32BIT) >> DDR_10_BIT;
				phy_capat->capat_high16bit = 0;
				phy_capat->capacity = phy_capat->capat_low16bit + phy_capat->capat_high16bit;
			}
		}
	} else {
		phy_capat->capacity = ddrtrn_winding_identification(WINDING_STEP, DDR_MAX_CAPAT, DDRT_MASK_32BIT) >> DDR_10_BIT;
	}
}

/**
  @brief Save axi register status before ddr capacity adaptation.

  @param[in] reg_bak  DDR axi register.
**/
static void ddrtrn_save_reg_status(unsigned int *reg_bak)
{
	unsigned int dmc_num, rank_num, rank_idx, dmc_idx, base_dmc;

	reg_bak[0] = ddrtrn_reg_read(DDR_REG_BASE_AXI + DDR_AXI_REGION0_MAP1); /* register 0 */
	reg_bak[1] = ddrtrn_reg_read(DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB1); /* register 1 */
	reg_bak[2] = ddrtrn_reg_read(DDR_REG_BASE_AXI + DDR_AXI_REGION0_MAP2); /* register 2 */
	reg_bak[3] = ddrtrn_reg_read(DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB2); /* register 3 */
	reg_bak[4] = ddrtrn_reg_read(DDR_REG_BASE_AXI + DDR_AXI_REGION1_MAP1); /* register 4 */

	dmc_num = ddrtrn_hal_get_cur_phy_dmc_num();
	rank_num = ddrtrn_hal_get_cur_phy_rank_num();
	for (rank_idx = 0; rank_idx < rank_num; rank_idx++) {
		for (dmc_idx = 0; dmc_idx < dmc_num; dmc_idx++) {
			base_dmc = ddrtrn_hal_get_cur_dmc_addr(dmc_idx);
			reg_bak[5 + rank_idx * DDR_DMC_PER_PHY_MAX + dmc_idx] = /* from register 5 */
				ddrtrn_reg_read(base_dmc + ddr_dmc_cfg_rnkvol(rank_idx));
			reg_bak[5 + DDR_DMC_PER_PHY_MAX * DDR_RANK_NUM + /* from register 5 */
				rank_idx * DDR_DMC_PER_PHY_MAX + dmc_idx] =
				ddrtrn_reg_read(DDR_REG_BASE_AXI + qos_ddrc_cfg_rnkvol(dmc_idx, rank_idx));
		}
	}
}

/**
  @brief Restore axi register status after ddr capacity adaptation.

  @param[in] reg_bak  DDR axi register.
**/
static void ddrtrn_restore_reg_status(unsigned int *reg_bak, unsigned int reg_bak_length)
{
	unsigned int dmc_num, rank_num, rank_idx, dmc_idx, base_dmc;
 
	dmc_num = ddrtrn_hal_get_cur_phy_dmc_num();
	rank_num = ddrtrn_hal_get_cur_phy_rank_num();
	if (reg_bak_length <= 5 + DDR_RANK_NUM * DDR_DMC_PER_PHY_MAX + /* from register 5 */
							(rank_num - 1) * DDR_DMC_PER_PHY_MAX +
							(dmc_num - 1) || reg_bak == NULL) {
		ddr_serial_puts("\rreg_bak error!\n");
		return;
	}
	ddrtrn_reg_write(reg_bak[0], DDR_REG_BASE_AXI + DDR_AXI_REGION0_MAP1); /* register 0 */
	ddrtrn_reg_write(reg_bak[1], DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB1); /* register 1 */
	ddrtrn_reg_write(reg_bak[2], DDR_REG_BASE_AXI + DDR_AXI_REGION0_MAP2); /* register 2 */
	ddrtrn_reg_write(reg_bak[3], DDR_REG_BASE_AXI + DDR_AXI_REGION0_ATTRIB2); /* register 3 */
	ddrtrn_reg_write(reg_bak[4], DDR_REG_BASE_AXI + DDR_AXI_REGION1_MAP1); /* register 4 */

	for (rank_idx = 0; rank_idx < rank_num; rank_idx++) {
		for (dmc_idx = 0; dmc_idx < dmc_num; dmc_idx++) {
			base_dmc = ddrtrn_hal_get_cur_dmc_addr(dmc_idx);
			ddrtrn_reg_write(reg_bak[5 + rank_idx * DDR_DMC_PER_PHY_MAX + dmc_idx], /* from register 5 */
				base_dmc + ddr_dmc_cfg_rnkvol(rank_idx));
			ddrtrn_reg_write(reg_bak[5 + DDR_DMC_PER_PHY_MAX * DDR_RANK_NUM + /* from register 5 */
				rank_idx * DDR_DMC_PER_PHY_MAX + dmc_idx],
				DDR_REG_BASE_AXI + qos_ddrc_cfg_rnkvol(dmc_idx, rank_idx));
		}
	}
}

/* return DDR size, unit:MB */
static void ddrtrn_get_ddr_size(struct ddr_capat_phy_all *capat_phy_all)
{
	unsigned int phy_idx = ddrtrn_hal_get_phy_id();
	unsigned int reg_bak[5 + DDR_DMC_PER_PHY_MAX * DDR_RANK_NUM * 2]; /* 5 axi registers and 2 special registers */
	struct ddr_data ddr_reg_val;

	ddrtrn_save_reg_status(reg_bak);
	/* Address mapping to PHY */
	ddrtrn_chsel_remap_func();

	/* store ddr_base value */
	ddrtrn_store_ddr(&ddr_reg_val);

	ddrtrn_ddrt_init_for_capacity();

	ddrtrn_winding_identy_scence(&capat_phy_all->phy_capat[phy_idx]);

	/* restore ddr base value */
	ddrtrn_restore_ddr(&ddr_reg_val);

	/* ‌5 AXI registers and 2 DDR logical registers (two per rank per DMC) */
	ddrtrn_restore_reg_status(
		reg_bak, 5 + DDR_DMC_PER_PHY_MAX * DDR_RANK_NUM * 2);
}

/* return DDR capacity (MB) */
unsigned int ddrtrn_capat_adapt(void)
{
	unsigned int i;
	unsigned int phy_num = ddrtrn_hal_get_phy_num();
	struct ddr_capat_phy_all capat_phy_all;
	struct ddrtrn_hal_training_custom_reg reg;
	ddrtrn_set_data(&reg, 0, sizeof(struct ddrtrn_hal_training_custom_reg));
	ddrtrn_set_data((char *)&capat_phy_all, 0, sizeof(struct ddr_capat_phy_all));

	if (ddrtrn_hal_boot_cmd_save(&reg) != 0)
		return -1;
	for (i = 0; i < phy_num; i++) {
		ddrtrn_hal_set_phy_id(i);
		ddrtrn_hal_set_cur_phy(ddrtrn_hal_get_phy_addr(i));
		capat_phy_all.cur_phy_rank_num[i] = ddrtrn_hal_get_cur_phy_rank_num();
		ddrtrn_get_ddr_size(&capat_phy_all);
		capat_phy_all.ddr_capat_sum += capat_phy_all.phy_capat[i].capacity;
	}

	ddrtrn_hal_boot_cmd_restore(&reg);

	ddr_serial_puts("\nDDR size: 0x");
	ddr_serial_put_hex(capat_phy_all.ddr_capat_sum);

	return capat_phy_all.ddr_capat_sum;
}
#endif /* DDR_CAPAT_ADAPT_CFG */