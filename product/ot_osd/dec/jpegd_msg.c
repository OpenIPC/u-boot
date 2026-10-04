/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#include "jpegd_msg.h"
#include "jpegd_comm.h"
#include "securec.h"

static void jpegd_dec_start(jpegd_down_msg *jpegd_msg, const jpegd_vpu_config *config)
{
    u_jpeg_dec_start d32;

    d32.u32 = 0;
    d32.bits.jpeg_dec_start = 1;
    jpegd_msg->jpeg_dec_start.u32 = d32.u32;

    return;
}

static void jpegd_resume_start(jpegd_down_msg *jpegd_msg)
{
    u_jpeg_resume_start d32;

    d32.u32 = 0;

    d32.bits.jpeg_resume_start = 0;
    d32.bits.last_resume_in_pic = 1;

    jpegd_msg->jpeg_resume_start.u32 = d32.u32;
    return;
}

static void jpegd_stride(jpegd_down_msg *jpegd_msg, const jpegd_vpu_config *config)
{
    u_jpeg_stride d32;

    d32.u32 = 0;

    d32.bits.y_stride = config->y_stride;
    d32.bits.uv_stride = config->c_stride;

    jpegd_msg->jpeg_stride.u32 = d32.u32;
    return;
}

static void jpegd_pic_size(jpegd_down_msg *jpegd_msg, const jpegd_vpu_config *config)
{
    u_picture_size d32;

    d32.u32 = 0;

    d32.bits.pic_width_in_mcu = config->width_in_mcu;
    d32.bits.pic_height_in_mcu = config->height_in_mcu;

    jpegd_msg->picture_size.u32 = d32.u32;
    return;
}

static void jpegd_pic_type(jpegd_down_msg *jpegd_msg, const jpegd_vpu_config *config)
{
    u_picture_type d32;

    d32.u32 = 0;

    d32.bits.pic_type = config->pic_type;

    jpegd_msg->picture_type.u32 = d32.u32;
    return;
}

static void jpegd_bit_buf_addr(jpegd_down_msg *jpegd_msg, const jpegd_vpu_config *config)
{
    /* start low 32 bit */
    jpegd_msg->bitbuffer_staddr.u32 = get_low_addr(config->phy_str_buf_start);
    /* start high 2 bit */
    jpegd_msg->bitbuffer_staddr_msb.u32 = get_high_addr(config->phy_str_buf_start);

    /* end low 32 bit */
    jpegd_msg->bitbuffer_endaddr.u32 = get_low_addr(config->phy_str_buf_end);
    /* end high 2 bit */
    jpegd_msg->bitbuffer_endaddr_msb.u32 = get_high_addr(config->phy_str_buf_end);
    return;
}

static void jpegd_bit_stream_addr(jpegd_down_msg *jpegd_msg, const jpegd_vpu_config *config)
{
    /* start low 32 bit */
    jpegd_msg->bitstreams_staddr.u32 = get_low_addr(config->phy_str_start);
    /* start high 2 bit */
    jpegd_msg->bitstream_staddr_msb.u32 = get_high_addr(config->phy_str_start);

    /* end low 32 bit */
    jpegd_msg->bitstreams_endaddr.u32 = get_low_addr(config->phy_str_end);
    /* end high 2 bit */
    jpegd_msg->bitstream_endaddr_msb.u32 = get_high_addr(config->phy_str_end);
    return;
}

static void jpegd_pic_yuv_addr(jpegd_down_msg *jpegd_msg, const jpegd_vpu_config *config)
{
    /* y low 32 bit */
    jpegd_msg->picture_ystaddr.u32 = get_low_addr(config->y_phy_addr);
    /* y high 2 bit */
    jpegd_msg->picture_ystaddr_msb.u32 = get_high_addr(config->y_phy_addr);

    if (config->out_yuv == TD_TRUE) {
        /* uv low 32 bit */
        jpegd_msg->picture_uvstaddr.u32 = get_low_addr(config->c_phy_addr);
        /* uv high 2 bit */
        jpegd_msg->picture_uvstaddr_msb.u32 = get_high_addr(config->c_phy_addr);
    }
    return;
}

static void jpegd_freq_scal(jpegd_down_msg *jpegd_msg, const jpegd_vpu_config *config)
{
    u_freq_scale d32;

    d32.u32 = jpegd_msg->freq_scale.u32;

    d32.bits.freq_scale = 0;
    d32.bits.ck_gt_en = 1;
    d32.bits.outstanding = 3; /* 3:Recommended Value */
    d32.bits.axi_id = 0;
    if (config->out_yuv == TD_TRUE) {
        d32.bits.jidct_emar_en = 1; /* 1:yuv output */
    } else {
        d32.bits.jidct_emar_en = 0; /* 0:rgb output */
    }
    jpegd_msg->freq_scale.u32 = d32.u32;
    return;
}

static void jpegd_out_type(jpegd_down_msg *jpegd_msg, const jpegd_vpu_config *config)
{
    u_out_type d32;

    d32.u32 = 0;

    if (config->out_yuv == TD_TRUE) {
        if (config->pic_format == PICTURE_FORMAT_YUV400) {
            d32.bits.jidct_yuv420_en = 0;
        } else {
            d32.bits.jidct_yuv420_en = 1;
        }
    }
    jpegd_msg->out_type.u32 = d32.u32;
    return;
}

static void jpegd_over_time_thd(jpegd_down_msg *jpegd_msg)
{
    u_over_time_thd d32;

    d32.u32 = 0;

    d32.bits.over_time_thr = 0xffffffff;
    jpegd_msg->over_time_thd.u32 = d32.u32;
    return;
}

static void jpegd_set_pix_sum_enable(jpegd_down_msg *jpegd_msg)
{
    u_luma_pix_sum1 d32;

    d32.u32 = 0;

    d32.bits.jidct_luma_sum_en = 0;
    jpegd_msg->luma_pix_sum1.u32 = d32.u32;
    return;
}

static void jpegd_sample_factor(jpegd_down_msg *jpegd_msg, const jpegd_vpu_config *config)
{
    u_sampling_factor d32;

    d32.u32 = 0;

    d32.bits.v_fac = config->v_fac;
    d32.bits.u_fac = config->u_fac;
    d32.bits.y_fac = config->y_fac;

    jpegd_msg->sampling_factor.u32 = d32.u32;
    return;
}

static void jpegd_dri(jpegd_down_msg *jpegd_msg, const jpegd_vpu_config *config)
{
    u_dri d32;

    d32.u32 = 0;

    d32.bits.jcfg2jvld_ri = config->dri;

    jpegd_msg->dri.u32 = d32.u32;
    return;
}

static td_void jpegd_line_num(jpegd_down_msg *jpegd_msg, const jpegd_vpu_config *config)
{
    u_jpgd_line_num d32;

    d32.u32 = 0;

    d32.bits.jcfg2idct_line_num = 0x26;

    jpegd_msg->jpgd_line_num.u32 = d32.u32;
    return;
}

static td_void jpegd_line_cnt_addr(jpegd_down_msg *jpegd_msg, const jpegd_vpu_config *config)
{
    /* end low 32 bit */
    jpegd_msg->jpgd_line_cnt_addr.u32 = 0x74d31080;

    /* end high 2 bit */
    jpegd_msg->lowdly_line_num_addr_msb.u32 = 0;
    return;
}

static void jpegd_quant_table(jpegd_down_msg *jpegd_msg, const jpegd_vpu_config *config)
{
    unsigned int i;
    for (i = 0; i < QUANT_TABLE_SIZE; i++) {
        jpegd_msg->quant_table[i].u32 = config->quant_table[i];
    }
    return;
}

static void jpegd_huffman_table(jpegd_down_msg *jpegd_msg, const jpegd_vpu_config *config)
{
    /* hdc_tbl,  huffman table */
    unsigned int i;
    for (i = 0; i < HDC_TABLE_SIZE; i++) {
        jpegd_msg->hdc_table[i].u32 = config->huffman_table[i];
    }

    /* hac_min_tbl */
    for (i = 0; i < HAC_MIN_TABLE_SIZE; i++) {
        jpegd_msg->hac_min_table[i].u32 = config->huffman_min_table[i];
    }

    /* hac_base_tbl, huffman table ac register */
    for (i = 0; i < HAC_BASE_TABLE_SIZE; i++) {
        jpegd_msg->hac_base_table[i].u32 = config->huffman_base_table[i];
    }

    /* hac_symbol_tbl, huffman table ac register */
    for (i = 0; i < HAC_SYMBOL_TABLE_SIZE; i++) {
        jpegd_msg->hac_symbol_table[i].u32 = config->huffman_symbol_table[i];
    }
    return;
}

int jpegd_write_msg_info(jpegd_down_msg *msg, jpegd_vpu_config *config)
{
    if ((msg == TD_NULL) || (config == TD_NULL)) {
        return TD_FAILURE;
    }

    (void)memset_s(msg, sizeof(jpegd_down_msg), 0x0, sizeof(jpegd_down_msg));

    jpegd_dec_start(msg, config);

    jpegd_resume_start(msg);

    jpegd_stride(msg, config);

    jpegd_pic_size(msg, config);

    jpegd_pic_type(msg, config);

    jpegd_bit_buf_addr(msg, config);

    jpegd_bit_stream_addr(msg, config);

    jpegd_pic_yuv_addr(msg, config);

    jpegd_freq_scal(msg, config);

    jpegd_out_type(msg, config);

    jpegd_over_time_thd(msg);

    jpegd_set_pix_sum_enable(msg);

    jpegd_sample_factor(msg, config);

    jpegd_dri(msg, config);

    jpegd_line_num(msg, config);

    jpegd_line_cnt_addr(msg, config);

    jpegd_quant_table(msg, config);

    jpegd_huffman_table(msg, config);

    return TD_SUCCESS;
}