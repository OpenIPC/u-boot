/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include <linux/delay.h>

#include "aiao_hal_common.h"
#include "aiao_hal.h"
#include "mpp_board.h"
#include "aiao_reg.h"

#define CHN_CNT_1 1
#define CHN_CNT_2 2

static td_ulong g_aio_base;

aiao_clk_info g_aio_clk_info = {
    .mclk_div_48k_256fs = AIO_MCLK_48K_256FS_786M,
    .mclk_div_44p1k_256fs = AIO_MCLK_44P1K_256FS_786M,
    .mclk_div_32k_256fs = AIO_MCLK_32K_256FS_786M,

    .mclk_div_48k_320fs = AIO_MCLK_48K_320FS_786M,
    .mclk_div_44p1k_320fs = AIO_MCLK_44P1K_320FS_786M,
    .mclk_div_32k_320fs = AIO_MCLK_32K_320FS_786M,

    .mclk_div_96k_256fs = AIO_MCLK_96K_256FS_786M,
    .mclk_div_88p2k_256fs = AIO_MCLK_88P2K_256FS_786M,
    .mclk_div_64k_256fs = AIO_MCLK_64K_256FS_786M,

    .mclk_div_192k_256fs = AIO_MCLK_192K_256FS_786M,
    .mclk_div_176p4k_256fs = AIO_MCLK_176P4K_256FS_786M,
    .mclk_div_128k_256fs = AIO_MCLK_128K_256FS_786M,
};

static td_u32 aiao_hal_read_reg(td_u32 offset)
{
    return (*(volatile td_u32 *)(g_aio_base + (td_u32)offset));
}

static td_void aiao_hal_write_reg(td_u32 offset, td_u32 value)
{
    *(volatile td_u32 *)(g_aio_base + (td_u32)(offset)) = value;
}

static td_void aiao_get_fs_bit(const aio_attr *attr, td_u32 *fs_bit)
{
    if (attr->chn_cnt == CHN_CNT_1) {
        if ((attr->bit_width == AUDIO_BIT_WIDTH_16) && (attr->expand_flag == 2)) { /* 2: cut */
            *fs_bit = CHN_CNT_2 * aio_get_bit_cnt(AUDIO_BIT_WIDTH_24);
        } else {
            *fs_bit = CHN_CNT_2 * aio_get_bit_cnt(attr->bit_width);
        }
    } else {
        if ((attr->bit_width == AUDIO_BIT_WIDTH_16) && (attr->expand_flag == 2)) { /* 2: cut */
            *fs_bit = attr->chn_cnt * aio_get_bit_cnt(AUDIO_BIT_WIDTH_24);
        } else {
            *fs_bit = attr->chn_cnt * aio_get_bit_cnt(attr->bit_width);
        }
    }
}

static td_s32 aiao_get_bclk_sel(td_u32 bclk_div, td_u32 *bclk_sel)
{
    switch (bclk_div) {
        case AIO_MCLK_TO_BCLK_RATIO_1: /* mclk is bclk*1 */
            *bclk_sel = SYS_AIO_BS_CLK1;
            break;
        case AIO_MCLK_TO_BCLK_RATIO_2: /* mclk is bclk*2 */
            *bclk_sel = SYS_AIO_BS_CLK2;
            break;
        case AIO_MCLK_TO_BCLK_RATIO_3: /* mclk is bclk*3 */
            *bclk_sel = SYS_AIO_BS_CLK3;
            break;
        case AIO_MCLK_TO_BCLK_RATIO_4: /* mclk is bclk*4 */
            *bclk_sel = SYS_AIO_BS_CLK4;
            break;
        case AIO_MCLK_TO_BCLK_RATIO_6: /* mclk is bclk*6 */
            *bclk_sel = SYS_AIO_BS_CLK6;
            break;
        case AIO_MCLK_TO_BCLK_RATIO_8: /* mclk is bclk*8 */
            *bclk_sel = SYS_AIO_BS_CLK8;
            break;
        case AIO_MCLK_TO_BCLK_RATIO_12: /* mclk is bclk*12 */
            *bclk_sel = SYS_AIO_BS_CLK12;
            break;
        case AIO_MCLK_TO_BCLK_RATIO_16: /* mclk is bclk*16 */
            *bclk_sel = SYS_AIO_BS_CLK16;
            break;
        case AIO_MCLK_TO_BCLK_RATIO_24: /* mclk is bclk*24 */
            *bclk_sel = SYS_AIO_BS_CLK24;
            break;
        case AIO_MCLK_TO_BCLK_RATIO_32: /* mclk is bclk*32 */
            *bclk_sel = SYS_AIO_BS_CLK32;
            break;
        case AIO_MCLK_TO_BCLK_RATIO_48: /* mclk is bclk*48 */
            *bclk_sel = SYS_AIO_BS_CLK48;
            break;
        case AIO_MCLK_TO_BCLK_RATIO_64: /* mclk is bclk*64 */
            *bclk_sel = SYS_AIO_BS_CLK64;
            break;
        default:
            printf("not support this bclk_division ratio\n");
            return TD_FAILURE;
    }
    return TD_SUCCESS;
}

static td_s32 aiao_get_lrclk_sel(td_u32 lrclk_div, td_u32 *lrclk_sel)
{
    switch (lrclk_div) {
        case AIO_BCLK_TO_FSCLK_RATIO_384:
            *lrclk_sel = SYS_AIO_SAMPLE_CLK384;
            break;
        case AIO_BCLK_TO_FSCLK_RATIO_320:
            *lrclk_sel = SYS_AIO_SAMPLE_CLK320;
            break;
        case AIO_BCLK_TO_FSCLK_RATIO_256: /* bclk is fsclk*256 */
            *lrclk_sel = SYS_AIO_SAMPLE_CLK256;
            break;
        case AIO_BCLK_TO_FSCLK_RATIO_128: /* bclk is fsclk*128 */
            *lrclk_sel = SYS_AIO_SAMPLE_CLK128;
            break;
        case AIO_BCLK_TO_FSCLK_RATIO_64: /* bclk is fsclk*64 */
            *lrclk_sel = SYS_AIO_SAMPLE_CLK64;
            break;
        case AIO_BCLK_TO_FSCLK_RATIO_32: /* bclk is fsclk*32 */
            *lrclk_sel = SYS_AIO_SAMPLE_CLK32;
            break;
        case AIO_BCLK_TO_FSCLK_RATIO_16: /* bclk is fsclk*16 */
            *lrclk_sel = SYS_AIO_SAMPLE_CLK16;
            break;
        default:
            printf("not support this fsclk_division ratio!\n");
            return TD_FAILURE;
    }
    return TD_SUCCESS;
}

static td_void aop_hal_int_en(td_u32 chn_id, td_bool en)
{
    u_aiao_int_ena tmp;

    if (chn_id >= AO_DEV_MAX_NUM) {
        printf("invalid chn_id(%d)!\n", chn_id);
    }

    tmp.u32 = aiao_hal_read_reg(AIAO_INT_ENA_REG);

    switch (chn_id) {
        case 0:
            tmp.bits.tx_ch0_int_ena = en;
            break;
        default:
            break;
    }

    aiao_hal_write_reg(AIAO_INT_ENA_REG, tmp.u32);
}

static td_void aop_hal_buffer_saddr_reg(td_u32 chn_id, td_u32 value)
{
    td_u32 temp;

    temp = (td_u32)(value & 0xffffffff);
    *(volatile td_u32 *)(g_aio_base + (td_u32)(aop_buff_saddr_reg(chn_id))) = temp;
}

td_void aop_hal_set_buffer_addr(td_u32 chn_id, td_u32 value)
{
    if (chn_id >= AO_DEV_MAX_NUM) {
        printf("invalid chn_id(%d)!\n", chn_id);
    }
    aop_hal_buffer_saddr_reg(chn_id, value);
    aop_hal_buffer_saddr_reg_h(g_aio_base, chn_id, value);
}

td_void aop_hal_set_buffer_size(td_u32 chn_id, td_u32 value)
{
    u_tx_buff_size tmp;

    if (chn_id >= AO_DEV_MAX_NUM) {
        printf("invalid chn_id(%d)!\n", chn_id);
    }

    tmp.u32 = aiao_hal_read_reg(aop_buff_size_reg(chn_id));

    tmp.bits.tx_buff_size = value;

    aiao_hal_write_reg(aop_buff_size_reg(chn_id), tmp.u32);
}

td_void aop_hal_set_buff_wptr(td_u32 chn_id, td_u32 value)
{
    u_tx_buff_wptr tmp;

    if (chn_id >= AO_DEV_MAX_NUM) {
        printf("invalid chn_id(%d)!\n", chn_id);
    }

    tmp.u32 = aiao_hal_read_reg(aop_buff_wptr_reg(chn_id));

    tmp.bits.tx_buff_wptr = value;

    aiao_hal_write_reg(aop_buff_wptr_reg(chn_id), tmp.u32);
}

td_void aop_hal_set_buff_rptr(td_u32 chn_id, td_u32 value)
{
    u_tx_buff_rptr tmp;

    if (chn_id >= AO_DEV_MAX_NUM) {
        printf("invalid chn_id(%d)!\n", chn_id);
    }

    tmp.u32 = aiao_hal_read_reg(aop_buff_rptr_reg(chn_id));

    tmp.bits.tx_buff_rptr = value;

    aiao_hal_write_reg(aop_buff_rptr_reg(chn_id), tmp.u32);
}

td_void aop_hal_set_trans_size(td_u32 chn_id, td_u32 value)
{
    u_tx_trans_size tmp;

    if (chn_id >= AO_DEV_MAX_NUM) {
        printf("invalid chn_id(%d)!\n", chn_id);
    }

    tmp.u32 = aiao_hal_read_reg(aop_trans_size_reg(chn_id));

    tmp.bits.tx_trans_size = value;

    aiao_hal_write_reg(aop_trans_size_reg(chn_id), tmp.u32);
}

static td_void aop_hal_set_tx_start(td_u32 chn_id, td_bool en)
{
    u_tx_dsp_ctrl tmp;

    if (chn_id >= AO_DEV_MAX_NUM) {
        printf("invalid chn_id(%d)!\n", chn_id);
    }

    tmp.u32 = aiao_hal_read_reg(aop_ctrl_reg(chn_id));

    tmp.bits.tx_enable = en;

    aiao_hal_write_reg(aop_ctrl_reg(chn_id), tmp.u32);
}

static td_u32 aop_hal_get_dis_done(td_u32 chn_id)
{
    u_tx_dsp_ctrl tmp;

    td_u32 status;

    if (chn_id >= AO_DEV_MAX_NUM) {
        printf("invalid chn_id(%d)!\n", chn_id);
        return TD_FAILURE;
    }

    tmp.u32 = aiao_hal_read_reg(aop_ctrl_reg(chn_id));

    status = tmp.bits.tx_disable_done;

    return status;
}

static td_void aop_hal_clr_child_int_all_status(td_u32 chn_id)
{
    u_tx_int_clr int_mask;

    if (chn_id >= AO_DEV_MAX_NUM) {
        printf("invalid chn_id(%d)!\n", chn_id);
    }

    int_mask.bits.tx_trans_int_clear = 1;
    int_mask.bits.tx_empty_int_clear = 1;
    int_mask.bits.tx_alempty_int_clear = 1;
    int_mask.bits.tx_bfifo_empty_int_clear = 1;
    int_mask.bits.tx_ififo_empty_int_clear = 1;
    int_mask.bits.tx_stop_int_clear = 1;
    int_mask.bits.tx_mfade_int_clear = 1;
    int_mask.bits.tx_dat_break_int_clear = 1;

    aiao_hal_write_reg(aop_int_clr_reg(chn_id), int_mask.u32);
}

td_s32 aop_hal_set_volume(td_u32 chn_id, td_s32 volume_db)
{
    u_tx_dsp_ctrl tmp;

    if (chn_id >= AO_DEV_MAX_NUM) {
        printf("invalid chn_id(%d)!\n", chn_id);
        return TD_FAILURE;
    }

    /* 0x7F->6db 0x7E->5db 0x29->-80db 0~0x28 mute */
    if ((volume_db < -121) || (volume_db > 6)) { /* -121, 6: volume */
        printf("invalid volume(%d) for chn_id(%d)!\n", volume_db, chn_id);
        return TD_FAILURE;
    }

    tmp.u32 = aiao_hal_read_reg(aop_ctrl_reg(chn_id));

    tmp.bits.volume = 0x7F - (6 - volume_db); /* 6: max volume */

    aiao_hal_write_reg(aop_ctrl_reg(chn_id), tmp.u32);

    return TD_SUCCESS;
}

static td_s32 aop_set_switch_tx_bclk(td_u32 chn_id, td_u32 crg_id)
{
    u_aiao_switch_tx_bclk switch_tx_bclk;

    /* internal bclk selection, for clock sharing */
    switch_tx_bclk.u32 = aiao_hal_read_reg(AOP_SWITCH_TX_BCLK);
    switch (chn_id) {
        case AIO_CHN_ID_0:
            switch_tx_bclk.bits.inner_bclk_ws_sel_tx_00 = crg_id;
            break;
        case AIO_CHN_ID_1:
            switch_tx_bclk.bits.inner_bclk_ws_sel_tx_01 = crg_id;
            break;
        case AIO_CHN_ID_2:
            switch_tx_bclk.bits.inner_bclk_ws_sel_tx_02 = crg_id;
            break;
        default:
            printf("invalid chn_id:%d\n", chn_id);
            return TD_FAILURE;
    }
    aiao_hal_write_reg(AOP_SWITCH_TX_BCLK, switch_tx_bclk.u32);

    return TD_SUCCESS;
}

static td_bool check_attr_sample_rate(audio_sample_rate sample_rate)
{
    if ((sample_rate != AUDIO_SAMPLE_RATE_8000) && (sample_rate != AUDIO_SAMPLE_RATE_12000) &&
        (sample_rate != AUDIO_SAMPLE_RATE_11025) && (sample_rate != AUDIO_SAMPLE_RATE_16000) &&
        (sample_rate != AUDIO_SAMPLE_RATE_22050) && (sample_rate != AUDIO_SAMPLE_RATE_24000) &&
        (sample_rate != AUDIO_SAMPLE_RATE_32000) && (sample_rate != AUDIO_SAMPLE_RATE_44100) &&
        (sample_rate != AUDIO_SAMPLE_RATE_48000)) {
        return TD_FALSE;
    }
    return TD_TRUE;
}

static td_s32 check_ao_attr_basic(const aio_attr *attr)
{
    if (aio_is_master_mode(attr->work_mode) && (check_attr_sample_rate(attr->sample_rate) == TD_FALSE)) {
        printf("invalid samplerate\n");
        return TD_FAILURE;
    }

    if ((attr->chn_cnt != CHN_CNT_1) && (attr->chn_cnt != CHN_CNT_2)) {
        printf("invalid chn_cnt:%u\n", attr->chn_cnt);
        return TD_FAILURE;
    }

    if ((attr->chn_cnt == CHN_CNT_1) && (attr->snd_mode == AUDIO_SOUND_MODE_STEREO)) {
        printf("can not use stereo mode when chn=1!\n");
        return TD_FAILURE;
    }

    if (attr->bit_width != AUDIO_BIT_WIDTH_16) {
        printf("invalid bit_width:%d\n", attr->bit_width);
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

static td_s32 check_ao_attr_other(const aio_attr *attr)
{
    if (aio_is_i2s_mode(attr->work_mode)) {
        if (attr->chn_cnt * aio_get_bit_cnt(attr->bit_width) > AIO_ONE_FIFO_BITWIDTH * 2) { /* 2 fifo */
            printf("I2S mode, not support chn_cnt(%u) * bit_width(%dbit).\n", attr->chn_cnt,
                aio_get_bit_cnt(attr->bit_width));
            return TD_FAILURE;
        }
    } else if (aio_is_pcm_mode(attr->work_mode)) {
        if (attr->chn_cnt * aio_get_bit_cnt(attr->bit_width) > 256) { /* 256 bit */
            printf("PCM mode, not support chn_cnt(%u) * bit_width(%dbit).\n", attr->chn_cnt,
                aio_get_bit_cnt(attr->bit_width));
            return TD_FAILURE;
        }
    }

    if ((attr->work_mode < 0) || (attr->work_mode >= AIO_MODE_BUTT)) {
        printf("invalid work_mode.\n");
        return TD_FAILURE;
    }
    if ((attr->snd_mode < 0) || (attr->snd_mode >= AUDIO_SOUND_MODE_BUTT)) {
        printf("invalid snd_mode.\n");
        return TD_FAILURE;
    }
    if ((attr->frame_num > MAX_AUDIO_FRAME_NUM) || (attr->frame_num < 2)) { /* 2 frame */
        printf("invalid frame_num.\n");
        return TD_FAILURE;
    }
    if ((attr->point_num_per_frame > MAX_AO_POINT_NUM) || (attr->point_num_per_frame < MIN_AUDIO_POINT_NUM)) {
        printf("invalid points(%d).\n", attr->point_num_per_frame);
        return TD_FAILURE;
    }
    if ((attr->clk_share != 0) && (attr->clk_share != 1)) {
        printf("invalid clk_share.\n");
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

static td_s32 check_ao_attr(td_u32 chn_id, const aio_attr *attr)
{
    td_s32 ret;

    if (chn_id >= AO_DEV_MAX_NUM) {
        printf("invalid chn_id(%d)!\n", chn_id);
        return TD_FAILURE;
    }

    ret = check_ao_attr_basic(attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = check_ao_attr_other(attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return TD_SUCCESS;
}

static td_s32 aop_set_attr_reg_tx_i2s_precision(const aio_attr *attr, u_tx_if_attri *aop_attr_reg)
{
    if (attr->bit_width == AUDIO_BIT_WIDTH_8) {
        aop_attr_reg->bits.tx_i2s_precision = 0; /* 0: 8bit */
    } else if (attr->bit_width == AUDIO_BIT_WIDTH_16) {
        aop_attr_reg->bits.tx_i2s_precision = 1; /* 1: 16bit */
    } else if (attr->bit_width == AUDIO_BIT_WIDTH_24) {
        aop_attr_reg->bits.tx_i2s_precision = 2; /* 2: 24bit */
    } else {
        printf("invalid bit_width!\n");
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

static td_s32 aop_set_attr_reg(td_u32 chn_id, const aio_attr *attr)
{
    td_s32 ret;
    td_u8 tx_sd;
    u_tx_if_attri aop_attr_reg;

    aop_attr_reg.u32 = aiao_hal_read_reg(aop_inf_attri_reg(chn_id));
    aop_attr_reg.bits.tx_underflow_ctrl = 1; /* send the last data */
    aop_attr_reg.bits.reserved_0 = 0;

    if ((attr->work_mode == AIO_MODE_I2S_SLAVE) || (attr->work_mode == AIO_MODE_I2S_MASTER)) {
        aop_attr_reg.bits.tx_mode = 0; /* I2S mode */
        aop_attr_reg.bits.tx_sd_offset = 0;
    } else if ((attr->work_mode == AIO_MODE_PCM_SLAVE_STD) || (attr->work_mode == AIO_MODE_PCM_MASTER_STD)) {
        aop_attr_reg.bits.tx_mode = 1; /* PCM standard mode */
        aop_attr_reg.bits.tx_sd_offset = 1;
    } else if ((attr->work_mode == AIO_MODE_PCM_SLAVE_NSTD) || (attr->work_mode == AIO_MODE_PCM_MASTER_NSTD)) {
        aop_attr_reg.bits.tx_mode = 1; /* PCM non standard mode */
        aop_attr_reg.bits.tx_sd_offset = 0;
    } else {
        printf("invalid work mode!\n");
        return TD_FAILURE;
    }

    if (attr->chn_cnt == CHN_CNT_1) {
        aop_attr_reg.bits.tx_ch_num = 0;
    } else if (attr->chn_cnt == CHN_CNT_2) {
        aop_attr_reg.bits.tx_ch_num = 1;
    } else {
        printf("invalid chn_cnt!\n");
        return TD_FAILURE;
    }

    ret = aop_set_attr_reg_tx_i2s_precision(attr, &aop_attr_reg);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    tx_sd = aop_get_tx_sd_private();
    switch (chn_id) {
        case 0: /* 0: AIO CHN ID */
            aop_attr_reg.bits.tx_sd_source_sel = tx_sd;
            break;
        default:
            return TD_FAILURE;
    }

    aop_attr_reg.bits.tx_sd0_sel = 0;
    aop_attr_reg.bits.tx_sd1_sel = 1;
    aop_attr_reg.bits.tx_sd2_sel = 2; /* 2: sd3 */
    aop_attr_reg.bits.tx_sd3_sel = 3; /* 3: sd4 */

    aop_attr_reg.bits.tx_trackmode = 0;
    aiao_hal_write_reg(aop_inf_attri_reg(chn_id), aop_attr_reg.u32);

    return TD_SUCCESS;
}

static td_void aop_set_ctrl_reg(td_u32 chn_id)
{
    u_tx_dsp_ctrl aop_ctrl_reg;
    aop_ctrl_reg.u32 = aiao_hal_read_reg(aop_ctrl_reg(chn_id));

    aop_ctrl_reg.bits.mute_en = 0;
    aop_ctrl_reg.bits.mute_fade_en = 0;
    aop_ctrl_reg.bits.reserved_3 = 0;
    aop_ctrl_reg.bits.volume = 0x79; /* 0db */
    aop_ctrl_reg.bits.reserved_2 = 0;
    aop_ctrl_reg.bits.fade_in_rate = 0;
    aop_ctrl_reg.bits.fade_out_rate = 0;
    aop_ctrl_reg.bits.reserved_1 = 0;
    aop_ctrl_reg.bits.bypass_en = 0;
    aop_ctrl_reg.bits.tx_enable = 0;
    aop_ctrl_reg.bits.reserved_0 = 0;

    aiao_hal_write_reg(aop_ctrl_reg(chn_id), aop_ctrl_reg.u32);
}

static td_s32 aiao_crg_set(td_u32 chn_id, const aio_attr *attr, td_u32 mclk_sel,
    td_u32 bclk_sel, td_u32 lrclk_sel)
{
    td_s32 ret;
    u_i2s_crg_cfg0 i2s_crg_cfg0;
    u_i2s_crg_cfg1 i2s_crg_cfg1;
    td_s32 crg_id;

    crg_id = aop_get_bclk_id_private();
    i2s_crg_cfg0.u32 = aiao_hal_read_reg(aio_i2s_reg_cfg0(crg_id));
    i2s_crg_cfg0.bits.aiao_mclk_div = mclk_sel;
    i2s_crg_cfg0.bits.reserved_0 = 0;
    aiao_hal_write_reg(aio_i2s_reg_cfg0(crg_id), i2s_crg_cfg0.u32);

    i2s_crg_cfg1.u32 = aiao_hal_read_reg(aio_i2s_reg_cfg1(crg_id));
    i2s_crg_cfg1.bits.aiao_bclk_div = bclk_sel;
    i2s_crg_cfg1.bits.aiao_fsclk_div = lrclk_sel;

    if (aio_is_master_mode(attr->work_mode)) {
        i2s_crg_cfg1.bits.aiao_bclk_oen = 0;
        i2s_crg_cfg1.bits.aiao_bclk_sel = 0;
    } else if (aio_is_slave_mode(attr->work_mode)) {
        i2s_crg_cfg1.bits.aiao_bclk_oen = 1;
        i2s_crg_cfg1.bits.aiao_bclk_sel = 1;
    } else {
        printf("invalid aio work_mode(%d).\n", attr->work_mode);
        return TD_FAILURE;
    }

    i2s_crg_cfg1.bits.aiao_cken = 1;
    i2s_crg_cfg1.bits.aiao_srst_req = 0;
    i2s_crg_cfg1.bits.aiao_bclkin_pctrl = 0;
    i2s_crg_cfg1.bits.aiao_bclkout_pctrl = 0;

    aiao_hal_write_reg(aio_i2s_reg_cfg1(crg_id), i2s_crg_cfg1.u32);

    ret = aop_set_switch_tx_bclk(chn_id, crg_id);
    return ret;
}

static td_s32 aiao_get_bclk_fsclk_div_cfg(const aio_attr *attr, td_u32 mclk_rate_num, td_u32 *bclk_sel,
    td_u32 *lrclk_sel)
{
    td_u32 fs_bit;
    td_s32 ret;

    aiao_get_fs_bit(attr, &fs_bit);

    if ((mclk_rate_num % (fs_bit * attr->sample_rate)) != 0) {
        printf("can not get the bclk_division ratio.\n");
        return TD_FAILURE;
    }

    ret = aiao_get_bclk_sel(mclk_rate_num / (fs_bit * attr->sample_rate), bclk_sel);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return aiao_get_lrclk_sel(fs_bit, lrclk_sel);
}

static td_s32 aiao_get_mclk_cfg(const aio_attr *attr, td_u32 *mclk_sel, td_u32 *mclk_rate_num)
{
    switch (attr->sample_rate) {
        case AUDIO_SAMPLE_RATE_8000:
        case AUDIO_SAMPLE_RATE_16000:
        case AUDIO_SAMPLE_RATE_32000:
            *mclk_sel = g_aio_clk_info.mclk_div_32k_256fs;
            *mclk_rate_num = AUDIO_SAMPLE_RATE_32000 * AIO_FIFO_BIT_WIDTH_256;
            break;
        case AUDIO_SAMPLE_RATE_12000:
        case AUDIO_SAMPLE_RATE_24000:
        case AUDIO_SAMPLE_RATE_48000:
            *mclk_sel = g_aio_clk_info.mclk_div_48k_256fs;
            *mclk_rate_num = AUDIO_SAMPLE_RATE_48000 * AIO_FIFO_BIT_WIDTH_256;
            break;

        case AUDIO_SAMPLE_RATE_11025:
        case AUDIO_SAMPLE_RATE_22050:
        case AUDIO_SAMPLE_RATE_44100:
            *mclk_sel = g_aio_clk_info.mclk_div_44p1k_256fs;
            *mclk_rate_num = AUDIO_SAMPLE_RATE_44100 * AIO_FIFO_BIT_WIDTH_256;
            break;
        default:
            printf("not support this sample rate.\n");
            return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_s32 aiao_get_mclk_bclk_fsclk_div_cfg(const aio_attr *attr, td_u32 *mclk_sel,
    td_u32 *bclk_sel, td_u32 *lrclk_sel)
{
    td_u32 ret;

    td_u32 mclk_rate_num;
    ret = aiao_get_mclk_cfg(attr, mclk_sel, &mclk_rate_num);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }
    return aiao_get_bclk_fsclk_div_cfg(attr, mclk_rate_num, bclk_sel, lrclk_sel);
}

static td_s32 aop_set_sys_ctl(td_u32 chn_id, const aio_attr *attr)
{
    td_u32 mclk_sel = 0;
    td_u32 lrclk_sel = 0;
    td_u32 bclk_sel = 0;
    td_s32 ret;

    /* get clock value */
    ret = aiao_get_mclk_bclk_fsclk_div_cfg(attr, &mclk_sel, &bclk_sel, &lrclk_sel);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

    return aiao_crg_set(chn_id, attr, mclk_sel, bclk_sel, lrclk_sel);
}

static td_void aop_set_mask(td_u32 chn_id, const aio_attr *attr)
{
    aop_set_mask_private(chn_id, g_aio_base);
}

td_s32 aop_hal_set_dev_attr(td_u32 chn_id, const aio_attr *attr)
{
    td_s32 ret;

    /* check AO attr */
    ret = check_ao_attr(chn_id, attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    /* config MUX */
    aop_set_mask(chn_id, attr);

    ret = aop_set_sys_ctl(chn_id, attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = aop_set_attr_reg(chn_id, attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    aop_set_ctrl_reg(chn_id);

    return TD_SUCCESS;
}

static td_void aiao_crg_enable(td_s32 crg_id)
{
    u_i2s_crg_cfg1 i2s_crg_cfg1;

    i2s_crg_cfg1.u32 = aiao_hal_read_reg(aio_i2s_reg_cfg1(crg_id));
    i2s_crg_cfg1.bits.aiao_bclk_en = 1;
    i2s_crg_cfg1.bits.aiao_ws_en = 1;
    i2s_crg_cfg1.bits.aiao_srst_req = 0;
    i2s_crg_cfg1.bits.aiao_cken = 1;
    aiao_hal_write_reg(aio_i2s_reg_cfg1(crg_id), i2s_crg_cfg1.u32);
}

static td_void aiao_crg_disable(td_u32 crg_id)
{
    u_i2s_crg_cfg1 i2s_crg_cfg1;

    i2s_crg_cfg1.u32 = aiao_hal_read_reg(aio_i2s_reg_cfg1(crg_id));
    i2s_crg_cfg1.bits.aiao_srst_req = 1;
    i2s_crg_cfg1.bits.aiao_cken = 0;
    i2s_crg_cfg1.bits.aiao_bclk_en = 0;
    i2s_crg_cfg1.bits.aiao_ws_en = 0;
    aiao_hal_write_reg(aio_i2s_reg_cfg1(crg_id), i2s_crg_cfg1.u32);
}

static td_void aop_hal_set_child_int_mask(td_u32 chn_id)
{
    u_tx_int_ena tmp;

    if (chn_id >= AO_DEV_MAX_NUM) {
        printf("invalid chn_id(%d)!\n", chn_id);
    }

    tmp.u32 = *(volatile td_u32 *)(g_aio_base + (td_u32)(aop_int_ena_reg(chn_id)));

    tmp.bits.tx_trans_int_ena = 1;
    tmp.bits.tx_empty_int_ena = 0;
    tmp.bits.tx_alempty_int_ena = 0;
    tmp.bits.tx_bfifo_empty_int_ena = 0;
    tmp.bits.tx_ififo_empty_int_ena = 0;
    tmp.bits.tx_stop_int_ena = 0;
    tmp.bits.tx_mfade_int_ena = 0;
    tmp.bits.tx_dat_break_int_ena = 0;
    tmp.bits.reserved_0 = 0;

    *(volatile td_u32 *)(g_aio_base + (td_u32)(aop_int_ena_reg(chn_id))) = tmp.u32;
}

td_void aop_hal_dev_enable(td_u32 chn_id)
{
    td_s32 crg_id = aop_get_bclk_id_private();

    aop_hal_set_child_int_mask(chn_id);

    aop_hal_int_en(chn_id, TD_TRUE);

    aiao_crg_enable(crg_id);

    aop_hal_set_tx_start(chn_id, TD_TRUE);
}

td_void aop_hal_dev_disable(td_u32 chn_id)
{
    td_u32 cnt = 0;
    u_i2s_crg_cfg1 i2s_crg_cfg1;
    td_s32 crg_id = aop_get_bclk_id_private();

    aop_hal_set_tx_start(chn_id, TD_FALSE);

    aop_hal_int_en(chn_id, TD_FALSE);

    aop_hal_clr_child_int_all_status(chn_id);

    while (!aop_hal_get_dis_done(chn_id)) {
        if (cnt++ > 100) { /* cnt is 100 */
            i2s_crg_cfg1.u32 = aiao_hal_read_reg(aio_i2s_reg_cfg1(crg_id));
            i2s_crg_cfg1.bits.aiao_srst_req = 1;
            i2s_crg_cfg1.bits.aiao_cken = 0;
            aiao_hal_write_reg(aio_i2s_reg_cfg1(crg_id), i2s_crg_cfg1.u32);

            udelay(10000); /* 10000us */

            i2s_crg_cfg1.u32 = aiao_hal_read_reg(aio_i2s_reg_cfg1(crg_id));
            i2s_crg_cfg1.bits.aiao_srst_req = 0;
            i2s_crg_cfg1.bits.aiao_cken = 1;
            aiao_hal_write_reg(aio_i2s_reg_cfg1(crg_id), i2s_crg_cfg1.u32);

            printf("can't get ao stop flag. ao clock is not config, please check your clock!\n");
            break;
        }
        udelay(10000); /* 10000us */
    }
    aiao_crg_disable(crg_id);
}

static td_s32 aiao_hal_set_clk_sel(audio_clk_select clk_sel)
{
    return aiao_hal_set_clk_sel_private(clk_sel, &g_aio_clk_info);
}

static td_s32 aiao_module_init_clk(td_void)
{
    td_s32 ret;
    td_bool reset = TD_FALSE;
    td_bool clk_en = TD_TRUE;
    audio_clk_select clk_sel = AUDIO_DEFAULT_CLK_SELECT;

    ret = aiao_hal_set_clk_sel(clk_sel);
    if (ret != TD_SUCCESS) {
        printf("set aio src clk failed!\n");
        return ret;
    }

    sys_hal_aio_clk_en(clk_en);

    sys_hal_aio_reset_sel(reset);

    sys_hal_acodec_apb_clk_en(clk_en);

    sys_hal_acodec_reset_sel(reset);

    return TD_SUCCESS;
}

static td_s32 aiao_reg_addr_init(td_void)
{
    g_aio_base = io_address(AIAO_REG_ADDR);
    if (g_aio_base == 0) {
        printf("failed to ioremap aiao reg addr.\n");
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_void aiao_hal_stop_int(td_void)
{
    u_aiao_int_ena un_tmp;

    un_tmp.u32 = 0x0;

    aiao_hal_write_reg(AIAO_INT_ENA_REG, un_tmp.u32);
}

static td_void aiao_init_tx_sd(td_void)
{
    td_u32 tx_port;
    u_tx_if_attri aop_attr_reg;

    for (tx_port = 0; tx_port < AO_DEV_MAX_NUM; tx_port++) {
        aop_attr_reg.u32 = aiao_hal_read_reg(aop_inf_attri_reg(tx_port));
        aop_attr_reg.bits.tx_sd_source_sel = tx_port & 0xff;
        aiao_hal_write_reg(aop_inf_attri_reg(tx_port), aop_attr_reg.u32);
    }
}

td_s32 aiao_module_init(td_void)
{
    td_s32 ret;

    ret = aiao_reg_addr_init();
    if (ret != TD_SUCCESS) {
        return ret;
    }

    aiao_hal_stop_int();

    ret = aiao_module_init_clk();
    if (ret != TD_SUCCESS) {
        return ret;
    }

    aiao_init_tx_sd();

    return TD_SUCCESS;
}

td_void aiao_module_exit(td_void)
{
    td_bool reset = TD_TRUE;
    td_bool clk_en = TD_FALSE;

    sys_hal_aio_reset_sel(reset);
    sys_hal_aio_clk_en(clk_en);
    sys_hal_acodec_apb_clk_en(clk_en);
    sys_hal_acodec_reset_sel(reset);
}
