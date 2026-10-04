/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#ifndef JPEGD_DRV_H
#define JPEGD_DRV_H
#include <linux/delay.h>
#ifdef MSG_LOAD_ENABLE
#include "jpegd_msg.h"
#else
#include "jpegd_reg.h"
#endif
#include "jpegd_comm.h"
#include "ot_type.h"

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

#ifdef CHIP_HI3519DV500
#define JPEGD_RST_CRG_REG_ADDR (0x11017640)
#define JPEGD_CLK_CRG_REG_ADDR (0x11017644)
#define JPEGD_REGS_ADDR        (0x17180000)
#define JPEGD_CKEN_OFFSET   4
#define JPEGD_ADDR_ALIGN    256
#endif

#define JPEGD_REGS_SIZE       0x6BF

#define JPEGD_IP_NUM          1

#define ZIGZAG_TABLE_SIZE     64

typedef struct {
    td_bool int_dec_finish;
    td_bool int_dec_err;
    td_bool int_over_time;
    td_bool int_bs_res;
} jpegd_vpu_status;

void jpegd_drv_write_regs(jpgd_regs_type *reg_base, const jpegd_vpu_config *config);

void jpegd_drv_read_regs(const jpgd_regs_type *reg_base, jpegd_vpu_status *vpu_status);

void jpegd_set_clock_en(int vpu_id, td_bool is_run);
void jpegd_reset_select(int vpu_id, td_bool is_run);

void jpegd_set_outstanding(int vpu_id, int outstanding);
void *jpegd_get_reg_addr(int vpu_id);
unsigned int jpegd_read_int(int vpu_id);
void jpegd_clear_int(int vpu_id);
void jpegd_set_int_mask(int vpu_id);
void jpegd_reset(int vpu_id);
void jpegd_start_vpu(int vpu_id, jpegd_vpu_config *config);
int jpegd_set_time_out(int vpu_id, int time_out);

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif /* End of JPEGD_DRV_H */
