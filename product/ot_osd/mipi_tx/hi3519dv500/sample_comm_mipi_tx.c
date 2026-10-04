/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include <linux/delay.h>
#include "ot_mipi_tx.h"
#include "mipi_tx.h"
#include "ot_mipi_tx_mod_init.h"
#include "securec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#if CONFIG_OT_MIPI_TX_SUPPORT

#ifdef OT_FPGA
#define SAMPLE_COMM_MIPI_TX_MAX_PHY_DATA_RATE 594
#else
#define SAMPLE_COMM_MIPI_TX_MAX_PHY_DATA_RATE 945
#endif

#define SAMPLE_COMM_MIPI_TX_DEFAULT_SCREEN_ENABLE 0

static const combo_dev_cfg_t g_sample_comm_mipi_tx_720x576_50_config = {
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .out_mode = OUT_MODE_DSI_VIDEO,
    .out_format = OUT_FORMAT_RGB_24BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .hpw  = 64, /* 64 pixel */
        .hbp  = 68, /* 68 pixel */
        .hact = 720, /* 720 pixel */
        .hfp  = 12, /* 12 pixel */
        .vpw   = 5, /* 5 line */
        .vbp   = 39, /* 39 line */
        .vact  = 576, /* 576 line */
        .vfp   = 5, /* 5 line */
    },
    .phy_data_rate = 459, /* 459 Mbps */
    .pixel_clk = 27000, /* 27000 KHz */
};

static const combo_dev_cfg_t g_sample_comm_mipi_tx_1024x768_60_config = {
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .out_mode = OUT_MODE_DSI_VIDEO,
    .out_format = OUT_FORMAT_RGB_24BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .hpw  = 136, /* 136 pixel */
        .hbp  = 160, /* 160 pixel */
        .hact = 1024, /* 1024 pixel */
        .hfp  = 24, /* 24 pixel */
        .vpw   = 6, /* 6 line */
        .vbp   = 29, /* 29 line */
        .vact  = 768, /* 768 line */
        .vfp   = 3, /* 3 line */
    },
    .phy_data_rate = 390, /* 390 Mbps */
    .pixel_clk = 65000, /* 65000 KHz */
};

static const combo_dev_cfg_t g_sample_comm_mipi_tx_1280x720_50_config = {
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .out_mode = OUT_MODE_DSI_VIDEO,
    .out_format = OUT_FORMAT_RGB_24BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .hpw  = 40, /* 40 pixel */
        .hbp  = 220, /* 220 pixel */
        .hact = 1280, /* 1280 pixel */
        .hfp  = 440, /* 440 pixel */
        .vpw   = 5, /* 5 line */
        .vbp   = 20, /* 20 line */
        .vact  = 720, /* 720 line */
        .vfp   = 5, /* 5 line */
    },
    .phy_data_rate = 446, /* 446 Mbps */
    .pixel_clk = 74250, /* 74250 KHz */
};

static const combo_dev_cfg_t g_sample_comm_mipi_tx_1280x720_60_config = {
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .out_mode = OUT_MODE_DSI_VIDEO,
    .out_format = OUT_FORMAT_RGB_24BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .hpw  = 40, /* 40 pixel */
        .hbp  = 220, /* 220 pixel */
        .hact = 1280, /* 1280 pixel */
        .hfp  = 110, /* 110 pixel */
        .vpw   = 5, /* 5 line */
        .vbp   = 20, /* 20 line */
        .vact  = 720, /* 720 line */
        .vfp   = 5, /* 5 line */
    },
    .phy_data_rate = 446, /* 446 Mbps */
    .pixel_clk = 74250, /* 74250 KHz */
};

static const combo_dev_cfg_t g_sample_comm_mipi_tx_1280x1024_60_config = {
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .out_mode = OUT_MODE_DSI_VIDEO,
    .out_format = OUT_FORMAT_RGB_24BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .hpw  = 112, /* 112 pixel */
        .hbp  = 248, /* 248 pixel */
        .hact = 1280, /* 1280 pixel */
        .hfp  = 48, /* 48 pixel */
        .vpw   = 3, /* 3 line */
        .vbp   = 38, /* 38 line */
        .vact  = 1024, /* 1024 line */
        .vfp   = 1, /* 1 line */
    },
    .phy_data_rate = 648, /* 648 Mbps */
    .pixel_clk = 108000, /* 108000 KHz */
};

static const combo_dev_cfg_t g_sample_comm_mipi_tx_1920x1080_24_config = {
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .out_mode = OUT_MODE_DSI_VIDEO,
    .out_format = OUT_FORMAT_RGB_24BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .hpw  = 44, /* 44 pixel */
        .hbp  = 148, /* 148 pixel */
        .hact = 1920, /* 1920 pixel */
        .hfp  = 638, /* 638 pixel */
        .vpw   = 5, /* 5 line */
        .vbp   = 36, /* 36 line */
        .vact  = 1080, /* 1080 line */
        .vfp   = 4, /* 4 line */
    },
    .phy_data_rate = 446, /* 446 Mbps */
    .pixel_clk = 74250, /* 74250 KHz */
};

static const combo_dev_cfg_t g_sample_comm_mipi_tx_1920x1080_25_config = {
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .out_mode = OUT_MODE_DSI_VIDEO,
    .out_format = OUT_FORMAT_RGB_24BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .hpw  = 44, /* 44 pixel */
        .hbp  = 148, /* 148 pixel */
        .hact = 1920, /* 1920 pixel */
        .hfp  = 528, /* 528 pixel */
        .vpw   = 5, /* 5 line */
        .vbp   = 36, /* 36 line */
        .vact  = 1080, /* 1080 line */
        .vfp   = 4, /* 4 line */
    },
    .phy_data_rate = 446, /* 446 Mbps */
    .pixel_clk = 74250, /* 74250 KHz */
};

static const combo_dev_cfg_t g_sample_comm_mipi_tx_1920x1080_30_config = {
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .out_mode = OUT_MODE_DSI_VIDEO,
    .out_format = OUT_FORMAT_RGB_24BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .hpw  = 44, /* 44 pixel */
        .hbp  = 148, /* 148 pixel */
        .hact = 1920, /* 1920 pixel */
        .hfp  = 88, /* 88 pixel */
        .vpw   = 5, /* 5 line */
        .vbp   = 36, /* 36 line */
        .vact  = 1080, /* 1080 line */
        .vfp   = 4, /* 4 line */
    },
    .phy_data_rate = 446, /* 446 Mbps */
    .pixel_clk = 74250, /* 74250 KHz */
};

static const combo_dev_cfg_t g_sample_comm_mipi_tx_1920x1080_50_config = {
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .out_mode = OUT_MODE_DSI_VIDEO,
    .out_format = OUT_FORMAT_RGB_24BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .hpw  = 44, /* 44 pixel */
        .hbp  = 148, /* 148 pixel */
        .hact = 1920, /* 1920 pixel */
        .hfp  = 528, /* 528 pixel */
        .vpw   = 5, /* 5 line */
        .vbp   = 36, /* 36 line */
        .vact  = 1080, /* 1080 line */
        .vfp   = 4, /* 4 line */
    },
    .phy_data_rate = 891, /* 891 Mbps */
    .pixel_clk = 148500, /* 148500 KHz */
};

static const combo_dev_cfg_t g_sample_comm_mipi_tx_1920x1080_60_config = {
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .out_mode = OUT_MODE_DSI_VIDEO,
    .out_format = OUT_FORMAT_RGB_24BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .hpw  = 44, /* 44 pixel */
        .hbp  = 148, /* 148 pixel */
        .hact = 1920, /* 1920 pixel */
        .hfp  = 88, /* 88 pixel */
        .vpw   = 5, /* 5 line */
        .vbp   = 36, /* 36 line */
        .vact  = 1080, /* 1080 line */
        .vfp   = 4, /* 4 line */
    },
    .phy_data_rate = 891, /* 891 Mbps */
    .pixel_clk = 148500, /* 148500 KHz */
};

static const combo_dev_cfg_t g_sample_comm_mipi_tx_3840x2160_24_config = {
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .out_mode = OUT_MODE_DSI_VIDEO,
    .out_format = OUT_FORMAT_RGB_24BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .hpw  = 88, /* 88 pixel */
        .hbp  = 296, /* 296 pixel */
        .hact = 3840, /* 3840 pixel */
        .hfp  = 1276, /* 1276 pixel */
        .vpw   = 10, /* 10 line */
        .vbp   = 72, /* 72 line */
        .vact  = 2160, /* 2160 line */
        .vfp   = 8, /* 8 line */
    },
    .phy_data_rate = 1782, /* 1782 Mbps */
    .pixel_clk = 297000, /* 297000 KHz */
};

static const combo_dev_cfg_t g_sample_comm_mipi_tx_3840x2160_25_config = {
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .out_mode = OUT_MODE_DSI_VIDEO,
    .out_format = OUT_FORMAT_RGB_24BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .hpw  = 88, /* 88 pixel */
        .hbp  = 296, /* 296 pixel */
        .hact = 3840, /* 3840 pixel */
        .hfp  = 1056, /* 1056 pixel */
        .vpw   = 10, /* 10 line */
        .vbp   = 72, /* 72 line */
        .vact  = 2160, /* 2160 line */
        .vfp   = 8, /* 8 line */
    },
    .phy_data_rate = 1782, /* 1782 Mbps */
    .pixel_clk = 297000, /* 297000 KHz */
};

static const combo_dev_cfg_t g_sample_comm_mipi_tx_3840x2160_30_config = {
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .out_mode = OUT_MODE_DSI_VIDEO,
    .out_format = OUT_FORMAT_RGB_24BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .hpw  = 88, /* 88 pixel */
        .hbp  = 296, /* 296 pixel */
        .hact = 3840, /* 3840 pixel */
        .hfp  = 176, /* 176 pixel */
        .vpw   = 10, /* 10 line */
        .vbp   = 72, /* 72 line */
        .vact  = 2160, /* 2160 line */
        .vfp   = 8, /* 8 line */
    },
    .phy_data_rate = 1782, /*  1782 Mbps */
    .pixel_clk = 297000, /* 297000 KHz */
};

static const combo_dev_cfg_t g_sample_comm_mipi_tx_3840x2160_50_config = {
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .out_mode = OUT_MODE_CSI,
    .out_format = OUT_FORMAT_RAW_16BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .hpw  = 88, /* 88 pixel */
        .hbp  = 296, /* 296 pixel */
        .hact = 3840, /* 3840 pixel */
        .hfp  = 1056, /* 1056 pixel */
        .vpw   = 10, /* 10 line */
        .vbp   = 72, /* 72 line */
        .vact  = 2160, /* 2160 line */
        .vfp   = 8, /* 8 line */
    },
    .phy_data_rate = 2259, /* 2259 Mbps, 2376 is larger than the max 2259, set to 2259 */
    .pixel_clk = 594000, /* 594000 KHz */
};

static const combo_dev_cfg_t g_sample_comm_mipi_tx_3840x2160_60_config = {
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .out_mode = OUT_MODE_CSI,
    .out_format = OUT_FORMAT_RAW_16BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .hpw  = 88, /* 88 pixel */
        .hbp  = 296, /* 296 pixel */
        .hact = 3840, /* 3840 pixel */
        .hfp  = 176, /* 176 pixel */
        .vpw   = 10, /* 10 line */
        .vbp   = 72, /* 72 line */
        .vact  = 2160, /* 2160 line */
        .vfp   = 8, /* 8 line */
    },
    .phy_data_rate = 2259, /* 2259 Mbps, 2376 is larger than the max 2259, set to 2259 */
    .pixel_clk = 594000, /* 594000KHz */
};

static const combo_dev_cfg_t g_sample_comm_mipi_tx_720x1280_60_config = {
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .out_mode = OUT_MODE_DSI_VIDEO,
    .out_format = OUT_FORMAT_RGB_24BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .hpw  = 24, /* 24 pixel */
        .hbp  = 99, /* 99 pixel */
        .hact = 720, /* 720 pixel */
        .hfp  = 99, /* 99 pixel */
        .vpw   = 4, /* 4 line */
        .vbp   = 20, /* 20 line */
        .vact  = 1280, /* 1280 line */
        .vfp   = 8, /* 8 line */
    },
    .phy_data_rate = 459, /* 459Mbps */
    .pixel_clk = 74250, /* 74250KHz */
};

static const combo_dev_cfg_t g_sample_comm_mipi_tx_1080x1920_60_config = {
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .out_mode = OUT_MODE_DSI_VIDEO,
    .out_format = OUT_FORMAT_RGB_24BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .hpw = 8, /* 8 pixel */
        .hbp = 20, /* 20 pixel */
        .hact = 1080, /* 1080 pixel */
        .hfp = 130, /* 130 pixel */
        .vpw = 10, /* 10 line */
        .vbp = 26, /* 26 line */
        .vact = 1920, /* 1920 line */
        .vfp = 16, /* 16 line */
    },
    .phy_data_rate = 891, /* 891 Mbps */
    .pixel_clk = 148500, /* 148500 KHz */
};

typedef struct {
    mipi_tx_intf_sync index;
    const combo_dev_cfg_t *mipi_tx_combo_dev_cfg;
} mipi_tx_intf_sync_cfg;

typedef struct {
    ot_vo_intf_sync intf_sync;
    mipi_tx_intf_sync mipi_tx_sync;
    td_char *mipi_fmt_name;
} vo_mst_sync_mipi_tx;

static const mipi_tx_intf_sync_cfg g_sample_mipi_tx_timing[OT_MIPI_TX_OUT_USER] = {
    {OT_MIPI_TX_OUT_576P50,       &g_sample_comm_mipi_tx_720x576_50_config},
    {OT_MIPI_TX_OUT_1024X768_60,  &g_sample_comm_mipi_tx_1024x768_60_config},
    {OT_MIPI_TX_OUT_720P50,       &g_sample_comm_mipi_tx_1280x720_50_config},
    {OT_MIPI_TX_OUT_720P60,       &g_sample_comm_mipi_tx_1280x720_60_config},
    {OT_MIPI_TX_OUT_1280X1024_60, &g_sample_comm_mipi_tx_1280x1024_60_config},
    {OT_MIPI_TX_OUT_1080P24,      &g_sample_comm_mipi_tx_1920x1080_24_config},
    {OT_MIPI_TX_OUT_1080P25,      &g_sample_comm_mipi_tx_1920x1080_25_config},
    {OT_MIPI_TX_OUT_1080P30,      &g_sample_comm_mipi_tx_1920x1080_30_config},
    {OT_MIPI_TX_OUT_1080P50,      &g_sample_comm_mipi_tx_1920x1080_50_config},
    {OT_MIPI_TX_OUT_1080P60,      &g_sample_comm_mipi_tx_1920x1080_60_config},
    {OT_MIPI_TX_OUT_3840X2160_24, &g_sample_comm_mipi_tx_3840x2160_24_config},
    {OT_MIPI_TX_OUT_3840X2160_25, &g_sample_comm_mipi_tx_3840x2160_25_config},
    {OT_MIPI_TX_OUT_3840X2160_30, &g_sample_comm_mipi_tx_3840x2160_30_config},
    {OT_MIPI_TX_OUT_3840X2160_50, &g_sample_comm_mipi_tx_3840x2160_50_config},
    {OT_MIPI_TX_OUT_3840X2160_60, &g_sample_comm_mipi_tx_3840x2160_60_config},

    {OT_MIPI_TX_OUT_720X1280_60,  &g_sample_comm_mipi_tx_720x1280_60_config},
    {OT_MIPI_TX_OUT_1080X1920_60, &g_sample_comm_mipi_tx_1080x1920_60_config},
    {0},
};

static const combo_dev_cfg_t *sample_mipi_tx_get_combo_dev_config(mipi_tx_intf_sync mipi_intf_sync)
{
    td_s32 loop;
    td_u32 loop_num = sizeof(g_sample_mipi_tx_timing) / sizeof(mipi_tx_intf_sync_cfg);

    for (loop = 0; loop < loop_num; loop++) {
        if (g_sample_mipi_tx_timing[loop].index == mipi_intf_sync) {
            return g_sample_mipi_tx_timing[loop].mipi_tx_combo_dev_cfg;
        }
    }

    return TD_NULL;
}

static td_s32 vo_mst_mipi_tx_send_one_cmd(const cmd_info_t *cmd_info)
{
    td_s32 ret;
    ret = mipi_tx_ioctl(OT_MIPI_TX_SET_CMD, (uintptr_t)cmd_info);
    if (ret != TD_SUCCESS) {
        printf("MIPI_TX SET CMD failed\n");
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_s32 vo_mst_mipi_tx_init_screen(const sample_mipi_tx_config *tx_config)
{
    td_s32 ret;
    cmd_info_t ci = { 0 };
    td_u32 loop;
    td_u32 loop_num = tx_config->cmd_count;

    if (loop_num == 0) {
        printf("info: cmd count is zero! mipi_tx will not send cmd\n");
        return TD_SUCCESS;
    }

    printf("cmd count is %d!\n", loop_num);

    for (loop = 0; loop < loop_num; loop++) {
        (td_void)memcpy_s(&ci, sizeof(cmd_info_t), &(tx_config->cmd_info[loop].cmd_info), sizeof(cmd_info_t));
        ret = vo_mst_mipi_tx_send_one_cmd(&ci);
        if (ret != TD_SUCCESS) {
            printf("loop(%d): MIPI_TX SET CMD failed\n", loop);

            return TD_FAILURE;
        }
        udelay(tx_config->cmd_info[loop].usleep_value);
    }

    return TD_SUCCESS;
}

static td_s32 sample_comm_mipi_tx_check_config(const sample_mipi_tx_config *tx_config)
{
    mipi_tx_intf_sync mipi_intf_sync;

    if (tx_config == NULL) {
        printf("tx_config is null\n");
        return TD_FAILURE;
    }

    mipi_intf_sync = tx_config->intf_sync;
    if ((mipi_intf_sync >= OT_MIPI_TX_OUT_BUTT)) {
        printf("mipi tx sync illegal\n");
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

static td_s32 sample_comm_mipi_tx_get_config(const sample_mipi_tx_config *tx_config,
    const combo_dev_cfg_t **mipi_tx_config)
{
    mipi_tx_intf_sync mipi_intf_sync;
    mipi_intf_sync = tx_config->intf_sync;

    if (mipi_intf_sync == OT_MIPI_TX_OUT_USER) {
        *mipi_tx_config = &tx_config->combo_dev_cfg;
        if ((*mipi_tx_config)->phy_data_rate == 0) {
            printf("error: not set mipi tx user config\n");
            return TD_FAILURE;
        }
    } else {
        *mipi_tx_config = sample_mipi_tx_get_combo_dev_config(mipi_intf_sync);
        if (*mipi_tx_config == TD_NULL) {
            printf("error: mipi tx combo config is null\n");
            return TD_FAILURE;
        }
    }
    return TD_SUCCESS;
}

#if MIPI_TX_DEBUG
static td_void mipi_tx_print_combo_dev_cfg(const combo_dev_cfg_t *mipi_tx_config)
{
    if (mipi_tx_config == NULL) {
        printf("param is null\n");
        return;
    }

    printf("print mipi tx config as follow----------\n");
    printf(".devno = %u\n", mipi_tx_config->devno);
    printf(".lane_id[%d] = %d %d %d %d\n",
        LANE_MAX_NUM,
        mipi_tx_config->lane_id[0], mipi_tx_config->lane_id[1], /* 0: lane0's id, 1: lane1's id */
        mipi_tx_config->lane_id[2], mipi_tx_config->lane_id[3]); /* 2: lane2's id, 3: lane3's id */
    printf(".out_mode = %d\n", mipi_tx_config->out_mode);
    printf(".video_mode = %d\n", mipi_tx_config->video_mode);
    printf(".out_format = %d\n", mipi_tx_config->out_format);
    printf(".sync_info\t= (  hsa,  hbp, hact,  hfp,  vsa,  vbp, vact,  vfp)\n"
           "\t\t= (%5d,%5d,%5d,%5d,%5d,%5d,%5d,%5d)\n",
        mipi_tx_config->sync_info.hpw, mipi_tx_config->sync_info.hbp,
        mipi_tx_config->sync_info.hact, mipi_tx_config->sync_info.hfp,
        mipi_tx_config->sync_info.vpw, mipi_tx_config->sync_info.vbp,
        mipi_tx_config->sync_info.vact, mipi_tx_config->sync_info.vfp);
    printf(".phy_data_rate = %u\n", mipi_tx_config->phy_data_rate);
    printf(".pixel_clk = %u\n", mipi_tx_config->pixel_clk);
    printf("print mipi tx config end.----------\n");
}

static td_void mipi_tx_print_cmd(cmd_info_t *ci)
{
    int i;
    printf("data_type=0x%02x\n", ci->data_type);
    printf("cmd_size=0x%04x\n", ci->cmd_size);
    if (ci->cmd != TD_NULL) {
        for (i = 0; i < ci->cmd_size; i++) {
            printf("0x%02x ", ci->cmd[i]);
        }
        printf("\n");
    }
}

static td_void mipi_tx_print_screen_cfg(const sample_mipi_tx_config *tx_config)
{
    cmd_info_t ci = { 0 };
    td_u32 loop;
    td_u32 loop_num = tx_config->cmd_count;

    printf("cmd_count = 0x%04x\n", loop_num);

    for (loop = 0; loop < loop_num; loop++) {
        (td_void)memcpy_s(&ci, sizeof(cmd_info_t), &(tx_config->cmd_info[loop].cmd_info), sizeof(cmd_info_t));
        mipi_tx_print_cmd(&ci);

        printf("usleep_value=%d\n", tx_config->cmd_info[loop].usleep_value);
    }
}
#endif

static td_s32 sample_comm_start_mipi_tx(const sample_mipi_tx_config *tx_config)
{
    mipi_tx_intf_sync mipi_intf_sync;
    const combo_dev_cfg_t *combo_config = TD_NULL;
    td_s32 ret;

    ret = sample_comm_mipi_tx_check_config(tx_config);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    mipi_tx_module_init();

    mipi_intf_sync = tx_config->intf_sync;
    printf("mipi intf sync = %d\n", mipi_intf_sync);

    ret = sample_comm_mipi_tx_get_config(tx_config, &combo_config);
    if (ret != TD_SUCCESS) {
        printf("%s,%d, get mipi tx config fail\n", __FUNCTION__, __LINE__);
        goto start_mipi_end;
    }

#if MIPI_TX_DEBUG
    /* print combo config */
    mipi_tx_print_combo_dev_cfg(combo_config);
#endif

    /* step1 */
    ret = mipi_tx_ioctl(OT_MIPI_TX_SET_DEV_CFG, (uintptr_t)combo_config);
    if (ret != TD_SUCCESS) {
        printf("%s,%d, ioctl mipi tx fail at ret(%d)\n", __FUNCTION__, __LINE__, ret);
        goto start_mipi_end;
    }

#if MIPI_TX_DEBUG
    /* print screen config */
    mipi_tx_print_screen_cfg(tx_config);
#endif

    /* step2 */
    ret = vo_mst_mipi_tx_init_screen(tx_config);
    if (ret != TD_SUCCESS) {
        printf("%s,%d, init screen failed\n", __FUNCTION__, __LINE__);
        goto start_mipi_end;
    }

    /* step3 */
    ret = mipi_tx_ioctl(OT_MIPI_TX_ENABLE, (uintptr_t)NULL);
    if (ret != TD_SUCCESS) {
        printf("%s,%d, ioctl mipi tx fail at ret(%d)\n", __FUNCTION__, __LINE__, ret);
    }

start_mipi_end:
    return ret;
}

static td_void sample_comm_stop_mipi_tx(ot_vo_intf_type intf_type)
{
    if (!((intf_type & OT_VO_INTF_MIPI) ||
        (intf_type & OT_VO_INTF_MIPI_SLAVE))) {
        printf("intf is not mipi\n");
        return;
    }

    if (mipi_tx_ioctl(OT_MIPI_TX_DISABLE, (uintptr_t)NULL) < 0) {
        printf("ioctl mipi tx fail\n");
    }

    mipi_tx_module_exit();
}

#if SAMPLE_COMM_MIPI_TX_DEFAULT_SCREEN_ENABLE

static const combo_dev_cfg_t g_sample_vo_mipi_tx_1080x1920_60_config = {
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .out_mode = OUT_MODE_DSI_VIDEO,
    .out_format = OUT_FORMAT_RGB_24BIT,
    .video_mode =  NON_BURST_MODE_SYNC_PULSES,
    .sync_info = {
        .hpw = 8, /* 8 pixel */
        .hbp = 20, /* 20 pixel */
        .hact = 1080, /* 1080 pixel */
        .hfp = 130, /* 130 pixel */
        .vpw = 10, /* 10 line */
        .vbp = 26, /* 26 line */
        .vact = 1920, /* 1920 line */
        .vfp = 16, /* 16 line */
    },
    .phy_data_rate = 594, /* 594 bps */
    .pixel_clk = 148500, /* 148500 KHz */
};

td_void set_mipi_tx_combo_cfg_1080x1920_60(combo_dev_cfg_t *combo_dev_cfg)
{
    (td_void)memcpy_s(combo_dev_cfg, sizeof(combo_dev_cfg_t), &g_sample_vo_mipi_tx_1080x1920_60_config,
        sizeof(combo_dev_cfg_t));
}

static const combo_dev_cfg_t g_sample_vo_mipi_tx_720x1280_st_60_config = {
    .devno = 0,
    .lane_id = {0, 1, 2, 3},
    .out_mode = OUT_MODE_DSI_VIDEO,
    .out_format = OUT_FORMAT_RGB_24BIT,
    .video_mode =  BURST_MODE,
    .sync_info = {
        .hpw = 10,
        .hbp = 40,
        .hact = 720,
        .hfp = 40,
        .vpw = 4,
        .vbp = 14,
        .vact = 1280,
        .vfp = 16,
    },
    .phy_data_rate = 453,
    .pixel_clk = 75000,
};

td_void set_mipi_tx_combo_cfg_720x1280_st_60(combo_dev_cfg_t *combo_dev_cfg)
{
    (td_void)memcpy_s(combo_dev_cfg, sizeof(combo_dev_cfg_t), &g_sample_vo_mipi_tx_720x1280_st_60_config,
        sizeof(combo_dev_cfg_t));
}

#define CMD_COUNT_1080x1920 13
#define UDELAY_1000   1000
#define UDELAY_10000  10000
#define UDELAY_20000  20000
#define UDELAY_200000 200000

static mipi_tx_cmd_info g_cmd_info_1080x1920[CMD_COUNT_1080x1920] = {
    /* {devno work_mode lp_clk_en data_type cmd_size cmd}, usleep_value */
    {{0, 0, 0, 0x23, 0xeeff, NULL}, UDELAY_1000},
    {{0, 0, 0, 0x23, 0x4018, NULL}, UDELAY_10000},
    {{0, 0, 0, 0x23, 0x0018, NULL}, UDELAY_20000},
    {{0, 0, 0, 0x23, 0x00ff, NULL}, UDELAY_10000},
    {{0, 0, 0, 0x23, 0x01fb, NULL}, UDELAY_10000},
    {{0, 0, 0, 0x23, 0x0135, NULL}, UDELAY_10000},
    {{0, 0, 0, 0x23, 0xff51, NULL}, UDELAY_1000},
    {{0, 0, 0, 0x23, 0x2c53, NULL}, UDELAY_1000},
    {{0, 0, 0, 0x23, 0x0155, NULL}, UDELAY_1000},
    {{0, 0, 0, 0x23, 0x24d3, NULL}, UDELAY_10000},
    {{0, 0, 0, 0x23, 0x10d4, NULL}, UDELAY_10000},
    {{0, 0, 0, 0x05, 0x0011, NULL}, UDELAY_200000},
    {{0, 0, 0, 0x05, 0x0029, NULL}, 0},
};

td_void set_mipi_tx_config_1080p_screen_user(sample_mipi_tx_config *tx_config)
{
    tx_config->cmd_count = CMD_COUNT_1080x1920;
    tx_config->cmd_info = g_cmd_info_1080x1920;
}

td_void set_mipi_tx_config_1080p_user(sample_mipi_tx_config *tx_config)
{
    if (tx_config->intf_sync == OT_MIPI_TX_OUT_USER) {
        set_mipi_tx_combo_cfg_1080x1920_60(&tx_config->combo_dev_cfg);
    }
}

#define CMD_DATA_COUNT_720X1280_ST 15

static unsigned char g_cmd_data_720x1280_st[CMD_DATA_COUNT_720X1280_ST][0x40] = {
    {
        0xb9, 0xf1, 0x12, 0x83,
    },
    {
        0xb2, 0xc8, 0x22, 0x30,
    },
    {
        0xb3, 0x10, 0x10, 0x28,
        0x28, 0x03, 0xff, 0x00,
        0x00, 0x00, 0x00, 0x00,
    },
    {
        0xb5, 0x0a, 0x0a, 0x00,
    },
    {
        0xb6, 0x50, 0x50, 0x00,
    },
    {
        0xb8, 0x25, 0x22, 0xf0,
        0x63, 0x00, 0x00, 0x00,
    },
    {
        0xba, 0x33, 0x81, 0x05,
        0xf9, 0x0e, 0x0e, 0x20,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x44,
        0x25, 0x00, 0x90, 0x0a,
        0x00, 0x00, 0x01, 0x4f,
        0x01, 0x00, 0x00, 0x37,
    },
    {
        0xbf, 0x02, 0x11, 0x00,
    },
    {
        0xc0, 0x73, 0x73, 0x50,
        0x50, 0x00, 0x00, 0x08,
        0x70, 0x00, 0x00, 0x00,
    },
    {
        0xc6, 0x82, 0x00, 0x3f,
        0xff, 0x00, 0xe0, 0x00,
    },
    {
        0xc1, 0x26, 0x00, 0x32,
        0x32, 0x77, 0xe1, 0xcc,
        0xcc, 0x67, 0x67, 0x33,
        0x33, 0x00, 0x00, 0x00,
    },
    {
        0xe0, 0x00, 0x03, 0x04,
        0x30, 0x30, 0x3f, 0x31,
        0x2f, 0x05, 0x0a, 0x0d,
        0x10, 0x12, 0x10, 0x12,
        0x0f, 0x18, 0x00, 0x03,
        0x04, 0x30, 0x30, 0x3f,
        0x31, 0x2f, 0x05, 0x0a,
        0x0d, 0x10, 0x12, 0x10,
        0x12, 0x0f, 0x18, 0x00,
    },
    {
        0xe3, 0x07, 0x07, 0x0b,
        0x0b, 0x03, 0x0b, 0x00,
        0x00, 0x00, 0x00, 0xff,
        0x80, 0xc0, 0x10, 0x00,
    },
    {
        0xe9, 0xc1, 0x10, 0x0e,
        0x00, 0x00, 0x80, 0x81,
        0x12, 0x31, 0x23, 0x48,
        0x8a, 0x80, 0x81, 0x47,
        0x20, 0x00, 0x00, 0x30,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x30, 0x00, 0x00,
        0x00, 0x02, 0x46, 0x02,
        0x88, 0x88, 0x88, 0x88,
        0x88, 0x88, 0x88, 0xf8,
        0x13, 0x57, 0x13, 0x88,
        0x88, 0x88, 0x88, 0x88,
        0x88, 0x88, 0xf8, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
    },
    {
        0xea, 0x00, 0x1a, 0x00,
        0x00, 0x00, 0x00, 0x02,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x75, 0x31, 0x31,
        0x88, 0x88, 0x88, 0x88,
        0x88, 0x88, 0x88, 0x8f,
        0x64, 0x20, 0x20, 0x88,
        0x88, 0x88, 0x88, 0x88,
        0x88, 0x88, 0x8f, 0x23,
        0x00, 0x00, 0x00, 0xa8,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
    },
};

#define CMD_COUNT_720x1280_ST 20
#define UDELAY_50       50
#define UDELAY_250000   250000
#define UDELAY_50000    50000

static mipi_tx_cmd_info g_cmd_info_720x1280_ST[CMD_COUNT_720x1280_ST] = {
    /* {devno work_mode lp_clk_en data_type cmd_size cmd}, usleep_value */
    {{0, 0, 1, 0x29, 0x04, g_cmd_data_720x1280_st[0]}, UDELAY_50}, /* 0: 0 cmd data index */
    {{0, 0, 1, 0x29, 0x04, g_cmd_data_720x1280_st[1]}, UDELAY_50}, /* 1: 1 cmd data index */
    {{0, 0, 1, 0x29, 0x0b, g_cmd_data_720x1280_st[2]}, UDELAY_50}, /* 2: 2 cmd data index */
    {{0, 0, 1, 0x23, 0x80b4, TD_NULL}, UDELAY_50},
    {{0, 0, 1, 0x29, 0x03, g_cmd_data_720x1280_st[3]}, UDELAY_50}, /* 3: 3 cmd data index */
    {{0, 0, 1, 0x29, 0x03, g_cmd_data_720x1280_st[4]}, UDELAY_50}, /* 4: 4 cmd data index */
    {{0, 0, 1, 0x29, 0x05, g_cmd_data_720x1280_st[5]}, UDELAY_50}, /* 5: 5 cmd data index */
    {{0, 0, 1, 0x29, 0x1c, g_cmd_data_720x1280_st[6]}, UDELAY_50}, /* 6: 6 cmd data index */
    {{0, 0, 1, 0x23, 0x46bc, TD_NULL}, UDELAY_50},
    {{0, 0, 1, 0x29, 0x04, g_cmd_data_720x1280_st[7]}, UDELAY_50}, /* 7: 7 cmd data index */
    {{0, 0, 1, 0x29, 0x0a, g_cmd_data_720x1280_st[8]}, UDELAY_50}, /* 8: 8 cmd data index */
    {{0, 0, 1, 0x29, 0x07, g_cmd_data_720x1280_st[9]}, UDELAY_50}, /* 9: 9 cmd data index */
    {{0, 0, 1, 0x29, 0x0d, g_cmd_data_720x1280_st[10]}, UDELAY_50}, /* 10: 10 cmd data index */
    {{0, 0, 1, 0x23, 0x0bcc, TD_NULL}, UDELAY_50},
    {{0, 0, 1, 0x29, 0x23, g_cmd_data_720x1280_st[11]}, UDELAY_50}, /* 11: 11 cmd data index */
    {{0, 0, 1, 0x29, 0x0f, g_cmd_data_720x1280_st[12]}, UDELAY_50}, /* 12: 12 cmd data index */
    {{0, 0, 1, 0x29, 0x40, g_cmd_data_720x1280_st[13]}, UDELAY_50}, /* 13: 13 cmd data index */
    {{0, 0, 1, 0x29, 0x3e, g_cmd_data_720x1280_st[14]}, UDELAY_50}, /* 14: 14 cmd data index */
    {{0, 0, 1, 0x23, 0x0111, TD_NULL}, UDELAY_250000},
    {{0, 0, 1, 0x23, 0x0129, TD_NULL}, UDELAY_50000},
};

static td_void set_mipi_tx_config_cmd_info(sample_mipi_tx_config *tx_config)
{
    tx_config->cmd_count = CMD_COUNT_720x1280_ST;
    tx_config->cmd_info = g_cmd_info_720x1280_ST;
}

td_void set_mipi_tx_config_720p_user(sample_mipi_tx_config *tx_config)
{
    if (tx_config->intf_sync == OT_MIPI_TX_OUT_USER) {
        set_mipi_tx_combo_cfg_720x1280_st_60(&tx_config->combo_dev_cfg);
    }
}

#else

td_void set_mipi_tx_config_1080p_screen_user(sample_mipi_tx_config *tx_config)
{
    tx_config->cmd_count = 0;
    tx_config->cmd_info = NULL;
}

static td_void set_mipi_tx_config_cmd_info(sample_mipi_tx_config *tx_config)
{
    tx_config->cmd_count = 0;
    tx_config->cmd_info = NULL;
}

static td_void set_mipi_tx_config_user(sample_mipi_tx_config *tx_config)
{
    if (tx_config->intf_sync == OT_MIPI_TX_OUT_USER) {
        (td_void)memcpy_s(&tx_config->combo_dev_cfg, sizeof(combo_dev_cfg_t),
            &g_sample_comm_mipi_tx_1920x1080_60_config, sizeof(combo_dev_cfg_t));
    }
}

#endif

static td_void set_mipi_tx_config_combo_dev_cfg(sample_mipi_tx_config *tx_config)
{
#if SAMPLE_COMM_MIPI_TX_DEFAULT_SCREEN_ENABLE
    /*
     * The combo device configuration of mipi_tx is for 720x1280 resolution screen.
     * It is only an example, and users need to modify it according to the actual situation.
     */
    set_mipi_tx_config_720p_user(tx_config);
#else
    /*
     * This combo device configuration is for 1920x1080 resolution peripherals.
     * It is only an example, and users need to modify it according to the actual situation
     */
    set_mipi_tx_config_user(tx_config);
#endif
}

void mipi_tx_display(unsigned int intftype, unsigned int sync)
{
    sample_mipi_tx_config tx_config = {0};

    if (!((intftype & OT_VO_INTF_MIPI) || (intftype & OT_VO_INTF_MIPI_SLAVE))) {
        return;
    }

    tx_config.intf_sync = sync;

    /*
     * step1: Users do:
     * fill the cmd_count and cmd_info for a peripheral device.
     * If this peripheral device needs to be configured through mipi_tx controller.
     * If not, ignore this step or fill in to 0.
     */
    set_mipi_tx_config_cmd_info(&tx_config);

    /*
     * step2: Users do:
     * fill the combo_dev_cfg for mipi_tx in USER timing.
     * If it is not user timing, the system will automatically fill in this item in step3.
     */
    set_mipi_tx_config_combo_dev_cfg(&tx_config);

    /*
     * step3: System do:
     * If it is not user timing, it is not necessary to fill in the combo_dev_cfg,
     * and the system will adopt a default configuration.
     */
    sample_comm_start_mipi_tx(&tx_config);
}

void mipi_tx_stop(unsigned int intftype)
{
    sample_comm_stop_mipi_tx(intftype);
}

#else

td_s32 sample_comm_start_mipi_tx(const sample_mipi_tx_config *tx_config)
{
    return 0;
}

td_void sample_comm_stop_mipi_tx(ot_vo_intf_type intf_type)
{
}

void mipi_tx_display(unsigned int intftype, unsigned int sync)
{
}

void mipi_tx_stop(unsigned int intftype)
{
}

#endif /* end of #if CONFIG_OT_MIPI_TX_SUPPORT */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */
