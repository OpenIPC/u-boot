/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#ifndef __MIPI_TX_H__
#define __MIPI_TX_H__

#include "ot_mipi_tx.h"
#include "ot_common_vo_dev.h"

#define MIPI_TX_MAX_SYNC_VACT  4096
#define MIPI_TX_MIN_SYNC_VACT  100
#define MIPI_TX_MAX_SYNC_VBP   256
#define MIPI_TX_MIN_SYNC_VBP   1
#define MIPI_TX_MAX_SYNC_VFP   256
#define MIPI_TX_MIN_SYNC_VFP   1
#define MIPI_TX_MAX_SYNC_HACT  4096
#define MIPI_TX_MIN_SYNC_HACT  1
#define MIPI_TX_MAX_SYNC_HBP   65535
#define MIPI_TX_MIN_SYNC_HBP   1
#define MIPI_TX_MAX_SYNC_HFP   65535
#define MIPI_TX_MIN_SYNC_HFP   1
#define MIPI_TX_MAX_SYNC_HPW   65535
#define MIPI_TX_MIN_SYNC_HPW   1
#define MIPI_TX_MAX_SYNC_VPW   256
#define MIPI_TX_MIN_SYNC_VPW   1

#define MIPI_TX_MAX_PHY_DATA_RATE 1800 /* Mbps */
#define MIPI_TX_MIN_PHY_DATA_RATE 80 /* Mbps */

#define MIPI_TX_MAX_PIXEL_CLK ((MIPI_TX_MAX_PHY_DATA_RATE) * (LANE_MAX_NUM) * 1000 / 8) /* KHz , 8: min 8bit/pixel */
#define MIPI_TX_MIN_PIXEL_CLK ((MIPI_TX_MIN_PHY_DATA_RATE) * (1) * 1000 / 24) /* KHz, 24: max 24bit/pixel */

#define mipi_tx_err(x, ...) printf("%s(%d): " x, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define mipi_tx_check_null_ptr_return(ptr)   \
    do {                                     \
        if ((ptr) == NULL) {                 \
            mipi_tx_err("NULL pointer\r\n"); \
            return (-1);                     \
        }                                    \
    } while (0)

#define mipi_tx_desc(x) 1
#define mipi_tx_unused(x) ((void)(x))

typedef struct {
    mipi_tx_work_mode_t work_mode;
    unsigned char       lp_clk_en;
} mipi_tx_work_param;

typedef struct {
    unsigned int        devno;
    mipi_tx_work_param     work_param;
} mipi_tx_pub_info;

typedef struct {
    combo_dev_cfg_t dev_cfg;
    mipi_tx_work_param work_param;
} mipi_tx_dev_ctx_t;

void mipi_tx_set_work_mode(mipi_tx_work_mode_t work_mode);
mipi_tx_work_mode_t mipi_tx_get_work_mode(void);
void mipi_tx_set_lp_clk_en(unsigned char lp_clk_en);
unsigned char mipi_tx_get_lp_clk_en(void);

typedef enum {
    OT_MIPI_TX_OUT_576P50       = OT_VO_OUT_576P50,
    OT_MIPI_TX_OUT_1024X768_60  = OT_VO_OUT_1024x768_60,
    OT_MIPI_TX_OUT_720P50       = OT_VO_OUT_720P50,
    OT_MIPI_TX_OUT_720P60       = OT_VO_OUT_720P60,
    OT_MIPI_TX_OUT_1280X1024_60 = OT_VO_OUT_1280x1024_60,
    OT_MIPI_TX_OUT_1080P24      = OT_VO_OUT_1080P24,
    OT_MIPI_TX_OUT_1080P25      = OT_VO_OUT_1080P25,
    OT_MIPI_TX_OUT_1080P30      = OT_VO_OUT_1080P30,
    OT_MIPI_TX_OUT_1080P50      = OT_VO_OUT_1080P50,
    OT_MIPI_TX_OUT_1080P60      = OT_VO_OUT_1080P60,
    OT_MIPI_TX_OUT_3840X2160_24 = OT_VO_OUT_3840x2160_24,
    OT_MIPI_TX_OUT_3840X2160_25 = OT_VO_OUT_3840x2160_25,
    OT_MIPI_TX_OUT_3840X2160_30 = OT_VO_OUT_3840x2160_30,
    OT_MIPI_TX_OUT_3840X2160_50 = OT_VO_OUT_3840x2160_50,
    OT_MIPI_TX_OUT_3840X2160_60 = OT_VO_OUT_3840x2160_60,

    OT_MIPI_TX_OUT_720X1280_60  = OT_VO_OUT_720x1280_60,
    OT_MIPI_TX_OUT_1080X1920_60 = OT_VO_OUT_1080x1920_60,

    OT_MIPI_TX_OUT_USER = OT_VO_OUT_USER,

    OT_MIPI_TX_OUT_BUTT = OT_VO_OUT_BUTT,
} mipi_tx_intf_sync;

typedef struct {
    cmd_info_t cmd_info;
    td_u32 usleep_value;
} mipi_tx_cmd_info;

typedef struct {
    /* for combo dev config */
    mipi_tx_intf_sync intf_sync;

    /* for screen cmd */
    td_u32 cmd_count;
    mipi_tx_cmd_info *cmd_info;

    /* for user sync */
    combo_dev_cfg_t combo_dev_cfg;
} sample_mipi_tx_config;

long mipi_tx_ioctl(unsigned int cmd, unsigned long arg);
void mipi_tx_display(unsigned int intftype, unsigned int sync);
void mipi_tx_stop(unsigned int intftype);
int mipi_tx_get_lane_num(const short lane_id[], int lane_id_len);

#endif /* end of #ifndef __MIPI_TX_H__ */
