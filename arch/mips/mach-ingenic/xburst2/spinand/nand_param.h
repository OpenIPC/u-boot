/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * The chip lists sfc_params.c registers, in the order the vendor's
 * deal_nand.sh lists them (its `ls *.c | grep nand`).
 */
#ifndef __NAND_PARAM_H
#define __NAND_PARAM_H
int ato_nand_register_func(void);
int dosilicon_nand_register_func(void);
int esmt_mid2c_nand_register_func(void);
int esmt_midc8_nand_register_func(void);
int fm25s_nand_register_func(void);
int foresee_nand_register_func(void);
int gd_nand_register_func(void);
int micron_nand_register_func(void);
int mxic_nand_register_func(void);
int winbond_nand_register_func(void);
int xtx_mid0b_nand_register_func(void);
int xtx_nand_register_func(void);
static void *nand_param[] = {
	(void *)ato_nand_register_func,
	(void *)dosilicon_nand_register_func,
	(void *)esmt_mid2c_nand_register_func,
	(void *)esmt_midc8_nand_register_func,
	(void *)fm25s_nand_register_func,
	(void *)foresee_nand_register_func,
	(void *)gd_nand_register_func,
	(void *)micron_nand_register_func,
	(void *)mxic_nand_register_func,
	(void *)winbond_nand_register_func,
	(void *)xtx_mid0b_nand_register_func,
	(void *)xtx_nand_register_func,
};
#endif
