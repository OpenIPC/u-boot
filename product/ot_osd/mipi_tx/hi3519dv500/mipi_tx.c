/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "mipi_tx.h"
#include <common.h>
#include <linux/types.h>
#include "mipi_tx_hal.h"
#include "ot_mipi_tx_mod_init.h"
#include "type.h"
#include "securec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define MIPI_TX_DEV_NAME "ot_mipi_tx"

#define mipi_tx_down_sem_return()

#define mipi_tx_up_sem()
#if mipi_tx_desc("pub")
static int g_en_dev = FALSE;
static int g_en_dev_cfg = FALSE;

static mipi_tx_dev_ctx_t g_mipi_tx_dev_ctx;
void mipi_tx_set_work_mode(mipi_tx_work_mode_t work_mode)
{
    g_mipi_tx_dev_ctx.work_param.work_mode = work_mode;
}

mipi_tx_work_mode_t mipi_tx_get_work_mode(void)
{
    return g_mipi_tx_dev_ctx.work_param.work_mode;
}

void mipi_tx_set_lp_clk_en(unsigned char lp_clk_en)
{
    g_mipi_tx_dev_ctx.work_param.lp_clk_en = lp_clk_en;
}

unsigned char mipi_tx_get_lp_clk_en(void)
{
    return g_mipi_tx_dev_ctx.work_param.lp_clk_en;
}

int mipi_tx_get_lane_num(const short lane_id[], int lane_id_len)
{
    int lane_num = 0;
    int i;

    if ((lane_id == NULL) || (lane_id_len > LANE_MAX_NUM)) {
        return lane_num;
    }

    for (i = 0; i < lane_id_len; i++) {
        if (lane_id[i] != MIPI_TX_DISABLE_LANE_ID) {
            lane_num++;
        }
    }
    return lane_num;
}

static int mipi_tx_check_temp_lane_id(int lane_index, int valid_lane_num, const short lane_id[], int lane_id_len)
{
    int j;
    int temp_id;
    temp_id = lane_id[lane_index];

    if ((temp_id < MIPI_TX_DISABLE_LANE_ID) || (temp_id >= LANE_MAX_NUM)) {
        mipi_tx_err("lane_id[%d] is invalid value %d.\n", lane_index, temp_id);
        return -1;
    }

    if (temp_id == MIPI_TX_DISABLE_LANE_ID) {
        return 0;
    }

    /*
     * lanex_id and lane num:
     * 1 lane: lane0_id, lane1/2/3_id must be different from lane0_id
     * 2 lane: lane0/1_id, lane2/3_id must be different from lane0/1_id
     * 3 lane: lane0/1/2_id, lane3_id must be different from lane0/1/2_id
     */
    if (temp_id > valid_lane_num) {
        mipi_tx_err("lane_id[%d]=%d is illegal, lane%d_id can't be set in %d lane mode\n", lane_index, temp_id,
            temp_id, valid_lane_num);
        return -1;
    }

    /* valid lane id must be different. */
    for (j = lane_index + 1; j < lane_id_len; j++) {
        if (temp_id == lane_id[j]) {
            mipi_tx_err("lane_id[%d] can't be same value %d as lane_id[%d]\n", lane_index, temp_id, j);
            return -1;
        }
    }

    return 0;
}

static int mipi_tx_check_dev_cfg_lane_id(const combo_dev_cfg_t *dev_cfg)
{
    int i;
    const int max_lane_num = LANE_MAX_NUM;
    int lane_num;

    lane_num = mipi_tx_get_lane_num(dev_cfg->lane_id, LANE_MAX_NUM);
    if (lane_num == 0) {
        mipi_tx_err("all lane_id is invalid!\n");
        return -1;
    }

    for (i = 0; i < max_lane_num; i++) {
        if (mipi_tx_check_temp_lane_id(i, lane_num, dev_cfg->lane_id, LANE_MAX_NUM) != 0) {
            return -1;
        }
    }

    return 0;
}

static int mipi_tx_check_dev_cfg_out_mode(const combo_dev_cfg_t *dev_cfg)
{
    if (dev_cfg->out_mode >= OUT_MODE_BUTT) {
        mipi_tx_err("mipi_tx dev output_mode(%d) err!\n", dev_cfg->out_mode);
        return -1;
    }
    return 0;
}

static int mipi_tx_check_dev_cfg_video_mode(const combo_dev_cfg_t *dev_cfg)
{
    if (dev_cfg->video_mode >= VIDEO_DATA_MODE_BUTT) {
        mipi_tx_err("mipi_tx dev video_mode(%d) err!\n", dev_cfg->video_mode);
        return -1;
    }
    return 0;
}

static int mipi_tx_check_dev_cfg_out_format(const combo_dev_cfg_t *dev_cfg)
{
    if (dev_cfg->out_format >= OUT_FORMAT_BUTT) {
        mipi_tx_err("mipi_tx dev format(%d) err!\n", dev_cfg->out_format);
        return -1;
    }
    return 0;
}

static int mipi_tx_check_dev_vertical_sync_info(const sync_info_t *sync_info)
{
    if ((sync_info->vact < MIPI_TX_MIN_SYNC_VACT) || (sync_info->vact > MIPI_TX_MAX_SYNC_VACT)) {
        mipi_tx_err("mipi tx sync's vact(%u) should be [%u, %u]!\n", sync_info->vact, MIPI_TX_MIN_SYNC_VACT,
            MIPI_TX_MAX_SYNC_VACT);
        return -1;
    }

    if ((sync_info->vbp < MIPI_TX_MIN_SYNC_VBP) || (sync_info->vbp > MIPI_TX_MAX_SYNC_VBP)) {
        mipi_tx_err("mipi tx sync's vbb(%u) should be [%u, %u]!\n", sync_info->vbp, MIPI_TX_MIN_SYNC_VBP,
            MIPI_TX_MAX_SYNC_VBP);
        return -1;
    }

    if ((sync_info->vfp < MIPI_TX_MIN_SYNC_VFP) || (sync_info->vfp > MIPI_TX_MAX_SYNC_VFP)) {
        mipi_tx_err("mipi tx sync's vfb(%u) should be [%u, %u]!\n",  sync_info->vfp, MIPI_TX_MIN_SYNC_VFP,
            MIPI_TX_MAX_SYNC_VFP);
        return -1;
    }

    return 0;
}

static int mipi_tx_check_dev_horizontal_sync_info(const sync_info_t *sync_info)
{
    if ((sync_info->hact < MIPI_TX_MIN_SYNC_HACT) || (sync_info->hact > MIPI_TX_MAX_SYNC_HACT)) {
        mipi_tx_err("mipi tx sync's hact(%u) should be [%u, %u]!\n", sync_info->hact, MIPI_TX_MIN_SYNC_HACT,
            MIPI_TX_MAX_SYNC_HACT);
        return -1;
    }

    if (sync_info->hbp < MIPI_TX_MIN_SYNC_HBP) {
        mipi_tx_err("mipi tx sync's hbb(%u) should be [%u, %u]!\n", sync_info->hbp, MIPI_TX_MIN_SYNC_HBP,
            MIPI_TX_MAX_SYNC_HBP);
        return -1;
    }

    if (sync_info->hfp < MIPI_TX_MIN_SYNC_HFP) {
        mipi_tx_err("mipi tx sync's hfb(%u) should be [%u, %u]!\n", sync_info->hfp, MIPI_TX_MIN_SYNC_HFP,
            MIPI_TX_MAX_SYNC_HFP);
        return -1;
    }

    return 0;
}

static int mipi_tx_check_dev_pulse_sync_info(const sync_info_t *sync_info)
{
    if (sync_info->hpw < MIPI_TX_MIN_SYNC_HPW) {
        mipi_tx_err("mipi tx sync's hpw(%u) should be [%u, %u]!\n", sync_info->hpw, MIPI_TX_MIN_SYNC_HPW,
            MIPI_TX_MAX_SYNC_HPW);
        return -1;
    }

    if ((sync_info->vpw < MIPI_TX_MIN_SYNC_VPW) || (sync_info->vpw > MIPI_TX_MAX_SYNC_VPW)) {
        mipi_tx_err("mipi tx sync's vpw(%u) should be [%u, %u]!\n", sync_info->vpw, MIPI_TX_MIN_SYNC_VPW,
            MIPI_TX_MAX_SYNC_VPW);
        return -1;
    }

    return 0;
}

static int mipi_tx_check_dev_cfg_sync_info(const combo_dev_cfg_t *dev_cfg)
{
    int ret;

    ret = mipi_tx_check_dev_vertical_sync_info(&dev_cfg->sync_info);
    if (ret != 0) {
        return ret;
    }

    ret = mipi_tx_check_dev_horizontal_sync_info(&dev_cfg->sync_info);
    if (ret != 0) {
        return ret;
    }

    return mipi_tx_check_dev_pulse_sync_info(&dev_cfg->sync_info);
}

static int mipi_tx_check_dev_cfg_phy_data_rate(const combo_dev_cfg_t *dev_cfg)
{
    if ((dev_cfg->phy_data_rate < MIPI_TX_MIN_PHY_DATA_RATE) ||
        (dev_cfg->phy_data_rate > MIPI_TX_MAX_PHY_DATA_RATE)) {
        mipi_tx_err("mipi_tx dev phy data rate(%u) should be in [%d, %d]\n", dev_cfg->phy_data_rate,
            MIPI_TX_MIN_PHY_DATA_RATE, MIPI_TX_MAX_PHY_DATA_RATE);
        return -1;
    }
    return 0;
}

static int mipi_tx_check_dev_cfg_pixel_clk(const combo_dev_cfg_t *dev_cfg)
{
    if ((dev_cfg->pixel_clk < MIPI_TX_MIN_PIXEL_CLK) ||
        (dev_cfg->pixel_clk > MIPI_TX_MAX_PIXEL_CLK)) {
        mipi_tx_err("mipi_tx dev pixel clk(%u) should be in [%d, %d]\n", dev_cfg->pixel_clk,
            MIPI_TX_MIN_PIXEL_CLK, MIPI_TX_MAX_PIXEL_CLK);
        return -1;
    }
    return 0;
}

static int mipi_tx_is_format_for_dsi_mode(out_format_t out_format)
{
    if ((out_format >= OUT_FORMAT_RGB_16BIT) && (out_format <= OUT_FORMAT_YUV422_16BIT)) {
        return 1;
    }
    return 0;
}

static int mipi_tx_is_format_for_csi_mode(out_format_t out_format)
{
    if ((out_format >= OUT_FORMAT_YUV420_8BIT_NORMAL) && (out_format <= OUT_FORMAT_RAW_16BIT)) {
        return 1;
    }
    return 0;
}

static int mipi_tx_is_dsi_mode(out_mode_t out_mode)
{
    if ((out_mode == OUT_MODE_DSI_VIDEO) || (out_mode == OUT_MODE_DSI_CMD)) {
        return 1;
    }
    return 0;
}

static int mipi_tx_is_csi_mode(out_mode_t out_mode)
{
    if (out_mode == OUT_MODE_CSI) {
        return 1;
    }
    return 0;
}

static int mipi_tx_check_dev_out_mode_and_out_format(const combo_dev_cfg_t *dev_cfg)
{
    if ((mipi_tx_is_dsi_mode(dev_cfg->out_mode) == 1) &&
        (mipi_tx_is_format_for_dsi_mode(dev_cfg->out_format) != 1)) {
        mipi_tx_err("mipi_tx dev out mode(%d) does not match the out data format(%d)\n",
            dev_cfg->out_mode, dev_cfg->out_format);
        return -1;
    }

    if ((mipi_tx_is_csi_mode(dev_cfg->out_mode) == 1) &&
        (mipi_tx_is_format_for_csi_mode(dev_cfg->out_format) != 1)) {
        mipi_tx_err("mipi_tx dev out mode(%d) does not match the out data format(%d)\n",
            dev_cfg->out_mode, dev_cfg->out_format);
        return -1;
    }
    return 0;
}

static int mipi_tx_check_dev_clklane_continue_mode(const combo_dev_cfg_t *dev_cfg)
{
    if ((dev_cfg->clklane_continue_mode < MIPI_TX_CLK_LANE_CONTINUE) ||
        (dev_cfg->clklane_continue_mode >= MIPI_TX_CLK_LANE_BUTT)) {
        mipi_tx_err("mipi_tx dev continue mode (%u) should be in [%d, %d]\n", dev_cfg->clklane_continue_mode,
            MIPI_TX_CLK_LANE_CONTINUE, MIPI_TX_CLK_LANE_NON_CONTINUE);
        return -1;
    }

    return 0;
}

static int mipi_tx_check_comb_dev_cfg(const combo_dev_cfg_t *dev_cfg)
{
    int ret;
    if (g_en_dev == TRUE) {
        mipi_tx_err("mipi_tx dev has enable!\n");
        return -1;
    }

    if (dev_cfg->devno != 0) {
        mipi_tx_err("mipi_tx dev devno err!\n");
        return -1;
    }

    ret = mipi_tx_check_dev_cfg_lane_id(dev_cfg);
    if (ret != 0) {
        return ret;
    }

    ret = mipi_tx_check_dev_cfg_out_mode(dev_cfg);
    if (ret != 0) {
        return ret;
    }

    ret = mipi_tx_check_dev_cfg_video_mode(dev_cfg);
    if (ret != 0) {
        return ret;
    }

    ret = mipi_tx_check_dev_cfg_out_format(dev_cfg);
    if (ret != 0) {
        return ret;
    }

    ret = mipi_tx_check_dev_cfg_sync_info(dev_cfg);
    if (ret != 0) {
        return ret;
    }

    ret = mipi_tx_check_dev_cfg_phy_data_rate(dev_cfg);
    if (ret != 0) {
        return ret;
    }

    ret = mipi_tx_check_dev_cfg_pixel_clk(dev_cfg);
    if (ret != 0) {
        return ret;
    }

    ret = mipi_tx_check_dev_out_mode_and_out_format(dev_cfg);
    if (ret != 0) {
        return ret;
    }
    return mipi_tx_check_dev_clklane_continue_mode(dev_cfg);
}

static int mipi_tx_set_combo_dev_cfg(const combo_dev_cfg_t *dev_cfg)
{
    int ret;

    mipi_tx_check_null_ptr_return(dev_cfg);
    mipi_tx_down_sem_return();

    ret = mipi_tx_check_comb_dev_cfg(dev_cfg);
    if (ret < 0) {
        mipi_tx_up_sem();
        mipi_tx_err("mipi_tx check combo_dev config failed!\n");
        return ret;
    }

    /* set controller config */
    mipi_tx_drv_set_controller_cfg(dev_cfg);

    /* set phy config */
    mipi_tx_drv_set_phy_cfg();

    mipi_tx_set_work_mode(MIPI_TX_WORK_MODE_LP);
    mipi_tx_set_lp_clk_en(FALSE);
    (td_void)memcpy_s(&g_mipi_tx_dev_ctx.dev_cfg, sizeof(combo_dev_cfg_t), dev_cfg, sizeof(combo_dev_cfg_t));
    g_en_dev_cfg = TRUE;

    mipi_tx_up_sem();

    return ret;
}

#endif /* #if mipi_tx_desc("pub") */

#if mipi_tx_desc("set & get cmd")

static int mipi_tx_check_pub_info(const mipi_tx_pub_info *pub_info)
{
    if (g_en_dev == TRUE) {
        mipi_tx_err("mipi_tx dev has enabled!\n");
        return -1;
    }

    if (g_en_dev_cfg != TRUE) {
        mipi_tx_err("mipi_tx dev has not configured!\n");
        return -1;
    }

    if (pub_info->devno != 0) {
        mipi_tx_err("mipi_tx devno(%u) illegal!\n", pub_info->devno);
        return -1;
    }

    if ((pub_info->work_param.work_mode != MIPI_TX_WORK_MODE_LP) &&
        (pub_info->work_param.work_mode != MIPI_TX_WORK_MODE_HS)) {
        mipi_tx_err("mipi_tx work_mode(%d) is illegal!\n", pub_info->work_param.work_mode);
        return -1;
    }

    if ((pub_info->work_param.lp_clk_en != 0) && (pub_info->work_param.lp_clk_en != 1)) {
        mipi_tx_err("mipi_tx lp_clk_en(%d) is illegal!\n", pub_info->work_param.lp_clk_en);
        return -1;
    }

    /* lp_clk_en must be 1 when work mode is hs mode */
    if ((pub_info->work_param.work_mode == MIPI_TX_WORK_MODE_HS) && (pub_info->work_param.lp_clk_en != 1)) {
        mipi_tx_err("mipi_tx lp_clk_en(%d) is not support in work_mode(%d)!\n",
            pub_info->work_param.lp_clk_en, pub_info->work_param.work_mode);
        return -1;
    }

    return 0;
}

static int mipi_tx_check_set_cmd_info(const cmd_info_t *cmd_info)
{
    mipi_tx_pub_info pub_info = { 0 };
    int ret;

    pub_info.devno = cmd_info->devno;
    pub_info.work_param.work_mode = cmd_info->work_mode;
    pub_info.work_param.lp_clk_en = cmd_info->lp_clk_en;

    ret = mipi_tx_check_pub_info(&pub_info);
    if (ret != 0) {
        return ret;
    }

    /* When cmd is not NULL,cmd_size means the length of cmd or it means cmd and addr */
    if (cmd_info->cmd != NULL) {
        if ((cmd_info->cmd_size > MIPI_TX_SET_DATA_SIZE) || (cmd_info->cmd_size == 0)) {
            mipi_tx_err("mipi_tx dev cmd_size(%d) is illegal, it should be in (%d, %d].\n", cmd_info->cmd_size,
                0, MIPI_TX_SET_DATA_SIZE);
            return -1;
        }
    }

    return 0;
}

static int mipi_tx_set_cmd(const cmd_info_t *cmd_info)
{
    int ret;

    mipi_tx_check_null_ptr_return(cmd_info);
    mipi_tx_down_sem_return();

    ret = mipi_tx_check_set_cmd_info(cmd_info);
    if (ret < 0) {
        mipi_tx_up_sem();
        mipi_tx_err("mipi_tx check set cmd info failed!\n");
        return ret;
    }
    ret = mipi_tx_drv_set_cmd_info(cmd_info);
    mipi_tx_up_sem();
    return ret;
}

static int mipi_tx_check_get_cmd_info(const get_cmd_info_t *get_cmd_info)
{
    mipi_tx_pub_info pub_info = { 0 };
    int ret;

    pub_info.devno = get_cmd_info->devno;
    pub_info.work_param.work_mode = get_cmd_info->work_mode;
    pub_info.work_param.lp_clk_en = get_cmd_info->lp_clk_en;

    ret = mipi_tx_check_pub_info(&pub_info);
    if (ret != 0) {
        return ret;
    }

    if ((get_cmd_info->get_data_size == 0) || (get_cmd_info->get_data_size > MIPI_TX_GET_DATA_SIZE)) {
        mipi_tx_err("mipi_tx dev get_data_size(%d) illegal, it should be in (%d, %d].\n",
            get_cmd_info->get_data_size, 0, MIPI_TX_GET_DATA_SIZE);
        return -1;
    }

    if (get_cmd_info->get_data == NULL) {
        mipi_tx_err("mipi_tx dev get_data is null!\n");
        return -1;
    }

    return 0;
}

static int mipi_tx_get_cmd(const get_cmd_info_t *get_cmd_info)
{
    int ret;

    mipi_tx_check_null_ptr_return(get_cmd_info);
    mipi_tx_down_sem_return();
    ret = mipi_tx_check_get_cmd_info(get_cmd_info);
    if (ret < 0) {
        mipi_tx_up_sem();
        mipi_tx_err("mipi_tx check get cmd info failed!\n");
        return ret;
    }
    ret = mipi_tx_drv_get_cmd_info(get_cmd_info);
    mipi_tx_up_sem();

    return ret;
}

#endif /* #if mipi_tx_desc("set & get cmd") */

#ifndef OT_FPGA
#if mipi_tx_desc("pn swap")
static int mipi_tx_check_set_pn_swap(const combo_dev_pn_swap_t *tx_pn_swap)
{
    int i;

    if (g_en_dev_cfg != TRUE) {
        mipi_tx_err("mipi_tx dev has not configured!\n");
        return -1;
    }
    if (g_en_dev == TRUE) {
        mipi_tx_err("mipi_tx dev has enable!\n");
        return -1;
    }
    if (tx_pn_swap->devno != 0) {
        mipi_tx_err("mipi_tx devno(%u) illegal!\n", tx_pn_swap->devno);
        return -1;
    }
    for (i = 0; i < MIPI_LANE_NUM_PER_DEV; i++) {
        if ((tx_pn_swap->pn_swap[i] != TRUE) && (tx_pn_swap->pn_swap[i] != FALSE)) {
            mipi_tx_err("pn_swap (%u) is illegal!\n", tx_pn_swap->pn_swap[i]);
            return -1;
        }
    }

    return 0;
}
static int mipi_tx_set_pn_swap(const combo_dev_pn_swap_t *pn_swap)
{
    int ret;
    mipi_tx_check_null_ptr_return(pn_swap);
    mipi_tx_down_sem_return();
    ret = mipi_tx_check_set_pn_swap(pn_swap);
    if (ret < 0) {
        mipi_tx_up_sem();
        mipi_tx_err("mipi_tx check pn swap failed!\n");
        return ret;
    }
    mipi_tx_drv_set_pn_swap(pn_swap);
    mipi_tx_up_sem();
    return ret;
}
#endif /* #if mipi_tx_desc("pn swap") */
#endif

#if mipi_tx_desc("enable & disable")

static int mipi_tx_enable(void)
{
    out_mode_t output_mode;

    mipi_tx_down_sem_return();
    if (g_en_dev_cfg == FALSE) {
        mipi_tx_up_sem();
        return -1;
    }

    output_mode = g_mipi_tx_dev_ctx.dev_cfg.out_mode;
    mipi_tx_drv_enable_input(output_mode);
    if (output_mode == OUT_MODE_DSI_CMD) {
        mipi_tx_set_work_mode(MIPI_TX_WORK_MODE_HS);
    }
    mipi_tx_set_lp_clk_en(TRUE);
    g_en_dev = TRUE;
    mipi_tx_up_sem();

    return 0;
}

static int mipi_tx_disable(void)
{
    mipi_tx_down_sem_return();
    mipi_tx_drv_disable_input();

    mipi_tx_drv_reset_pn_swap();
    mipi_tx_set_work_mode(MIPI_TX_WORK_MODE_LP);
    mipi_tx_set_lp_clk_en(FALSE);
    mipi_tx_drv_set_ctrl_clk(0);
    g_en_dev = FALSE;
    g_en_dev_cfg = FALSE;
    mipi_tx_up_sem();

    return 0;
}

#endif /* #if mipi_tx_desc("enable & disable") */

#if mipi_tx_desc("proc")
#endif /* #if mipi_tx_desc("proc") */

#if mipi_tx_desc("init & ioctl")
long mipi_tx_ioctl(unsigned int cmd, unsigned long arg)
{
    switch (cmd) {
        case OT_MIPI_TX_SET_DEV_CFG:
            return mipi_tx_set_combo_dev_cfg((combo_dev_cfg_t *)(uintptr_t)arg);

        case OT_MIPI_TX_SET_CMD:
            return mipi_tx_set_cmd((cmd_info_t *)(uintptr_t)arg);

        case OT_MIPI_TX_GET_CMD:
            return mipi_tx_get_cmd((get_cmd_info_t *)(uintptr_t)arg);

        case OT_MIPI_TX_ENABLE:
            return mipi_tx_enable();

        case OT_MIPI_TX_DISABLE:
            return mipi_tx_disable();

        case OT_MIPI_TX_SET_PN_SWAP:
            return mipi_tx_set_pn_swap((combo_dev_pn_swap_t *)(uintptr_t)arg);

        default:
            mipi_tx_err("invalid mipi_tx ioctl cmd: 0x%x\n", cmd);
            return (-1);
    }
}

static int mipi_tx_init(void)
{
    return mipi_tx_drv_init();
}

static void mipi_tx_exit(void)
{
    mipi_tx_drv_exit();
}

int mipi_tx_module_init(void)
{
    int ret;

    ret = mipi_tx_init();
    if (ret != 0) {
        mipi_tx_err("ot_mipi_init failed!\n");
        goto fail0;
    }

    printf("load mipi_tx driver successful!\n");
    return 0;

fail0:
    mipi_tx_err("load mipi_tx driver failed!\n");
    return -1;
}

void mipi_tx_module_exit(void)
{
    mipi_tx_exit();
    printf("unload mipi_tx driver ok!\n");
}

#endif /* #if mipi_tx_desc("init & ioctl") */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
