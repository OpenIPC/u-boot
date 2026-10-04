/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#ifndef JPEGD_MSG_H
#define JPEGD_MSG_H

#include "jpegd_comm.h"

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

/* Define the union u_jpeg_dec_start */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    jpeg_dec_start        : 1   ; /* [0] */
        unsigned int    rst_busy              : 1   ; /* [1] */
        unsigned int    axi_4k_bypass         : 1   ; /* [2] */
        unsigned int    jcfg2idct_lowdly_en   : 1   ; /* [3] */
        unsigned int    axi_sep_typ           : 1   ; /* [4] */
        unsigned int    reserved_0            : 27  ; /* [31..5] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_jpeg_dec_start;

/* Define the union u_jpeg_resume_start */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    jpeg_resume_start     : 1   ; /* [0] */
        unsigned int    last_resume_in_pic    : 1   ; /* [1] */
        unsigned int    reserved_0            : 30  ; /* [31..2] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_jpeg_resume_start;

/* Define the union u_pic_vld_num */
typedef union {
    /* Define the struct bits  */
    struct {
        unsigned int pic_vld_num            : 32  ; /* [31..0] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_pic_vld_num;
/* Define the union u_jpeg_stride */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    y_stride              : 16  ; /* [15..0] */
        unsigned int    uv_stride             : 16  ; /* [31..16] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_jpeg_stride;

/* Define the union u_picture_size */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    pic_width_in_mcu      : 12  ; /* [11..0] */
        unsigned int    reserved_0            : 4   ; /* [15..12] */
        unsigned int    pic_height_in_mcu     : 12  ; /* [27..16] */
        unsigned int    reserved_1            : 4   ; /* [31..28] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_picture_size;

/* Define the union u_picture_type */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    pic_type              : 3   ; /* [2..0] */
        unsigned int    reserved_0            : 29  ; /* [31..3] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_picture_type;

/* Define the union u_picdec_time */
typedef union {
    /* Define the struct bits  */
    struct {
        unsigned int picdec_time            : 32  ; /* [31..0] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_picdec_time;
/* Define the union u_rgb_out_stride */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    jcfg2jdo_out_stride   : 16  ; /* [15..0] */
        unsigned int    reserved_0            : 16  ; /* [31..16] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_rgb_out_stride;

/* Define the union u_bitbuffer_staddr */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    reserved_0            : 6   ; /* [5..0] */
        unsigned int    bb_staddr             : 26  ; /* [31..6] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_bitbuffer_staddr;

/* Define the union u_bitbuffer_endaddr */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    reserved_0            : 6   ; /* [5..0] */
        unsigned int    bb_endaddr            : 26  ; /* [31..6] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_bitbuffer_endaddr;

/* Define the union u_bitstreams_staddr */
typedef union {
    /* Define the struct bits  */
    struct {
        unsigned int bs_staddr              : 32  ; /* [31..0] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_bitstreams_staddr;
/* Define the union u_bitstreams_endaddr */
typedef union {
    /* Define the struct bits  */
    struct {
        unsigned int bs_endaddr             : 32  ; /* [31..0] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_bitstreams_endaddr;
/* Define the union u_picture_ystaddr */
typedef union {
    /* Define the struct bits  */
    struct {
        unsigned int pic_ystaddr            : 32  ; /* [31..0] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_picture_ystaddr;
/* Define the union u_picture_uvstaddr */
typedef union {
    /* Define the struct bits  */
    struct {
        unsigned int pic_uvstaddr           : 32  ; /* [31..0] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_picture_uvstaddr;
/* Define the union u_pd_sum_cfg_cbcr */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    jcfg2jvld_pd_cb       : 16  ; /* [15..0] */
        unsigned int    jcfg2jvld_pd_cr       : 16  ; /* [31..16] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_pd_sum_cfg_cbcr;

/* Define the union u_pd_sum_cfg_y */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    jcfg2jvld_pd_y        : 16  ; /* [15..0] */
        unsigned int    jcfg2jbs_flag_cur     : 1   ; /* [16] */
        unsigned int    reserved_0            : 15  ; /* [31..17] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_pd_sum_cfg_y;

/* Define the union u_freq_scale */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    freq_scale            : 2   ; /* [1..0] */
        unsigned int    jidct_emar_en         : 1   ; /* [2] */
        unsigned int    ck_gt_en              : 1   ; /* [3] */
        unsigned int    outstanding           : 4   ; /* [7..4] */
        unsigned int    axi_id                : 4   ; /* [11..8] */
        unsigned int    reserved_0            : 20  ; /* [31..12] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_freq_scale;

/* Define the union u_out_type */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    out_rgb_type_old      : 2   ; /* [1..0] */
        unsigned int    jidct_yuv420_en       : 1   ; /* [2] */
        unsigned int    hor_med_en            : 1   ; /* [3] */
        unsigned int    ver_med_en            : 1   ; /* [4] */
        unsigned int    dither_en             : 1   ; /* [5] */
        unsigned int    reserved_0            : 2   ; /* [7..6] */
        unsigned int    out_rgb_type          : 3   ; /* [10..8] */
        unsigned int    reserved_1            : 21  ; /* [31..11] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_out_type;

/* Define the union u_jpgd_alpha */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    jpgd_a                : 8   ; /* [7..0] */
        unsigned int    reserved_0            : 24  ; /* [31..8] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_jpgd_alpha;

/* Define the union u_hor_phase0_coef01 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    hor_phase0_coef0      : 10  ; /* [9..0] */
        unsigned int    reserved_0            : 6   ; /* [15..10] */
        unsigned int    hor_phase0_coef1      : 10  ; /* [25..16] */
        unsigned int    reserved_1            : 6   ; /* [31..26] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_hor_phase0_coef01;

/* Define the union u_hor_phase0_coef23 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    hor_phase0_coef2      : 10  ; /* [9..0] */
        unsigned int    reserved_0            : 6   ; /* [15..10] */
        unsigned int    hor_phase0_coef3      : 10  ; /* [25..16] */
        unsigned int    reserved_1            : 6   ; /* [31..26] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_hor_phase0_coef23;

/* Define the union u_hor_phase0_coef45 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    hor_phase0_coef4      : 10  ; /* [9..0] */
        unsigned int    reserved_0            : 6   ; /* [15..10] */
        unsigned int    hor_phase0_coef5      : 10  ; /* [25..16] */
        unsigned int    reserved_1            : 6   ; /* [31..26] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_hor_phase0_coef45;

/* Define the union u_hor_phase0_coef67 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    hor_phase0_coef6      : 10  ; /* [9..0] */
        unsigned int    reserved_0            : 6   ; /* [15..10] */
        unsigned int    hor_phase0_coef7      : 10  ; /* [25..16] */
        unsigned int    reserved_1            : 6   ; /* [31..26] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_hor_phase0_coef67;

/* Define the union u_hor_phase2_coef01 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    hor_phase2_coef0      : 10  ; /* [9..0] */
        unsigned int    reserved_0            : 6   ; /* [15..10] */
        unsigned int    hor_phase2_coef1      : 10  ; /* [25..16] */
        unsigned int    reserved_1            : 6   ; /* [31..26] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_hor_phase2_coef01;

/* Define the union u_hor_phase2_coef23 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    hor_phase2_coef2      : 10  ; /* [9..0] */
        unsigned int    reserved_0            : 6   ; /* [15..10] */
        unsigned int    hor_phase2_coef3      : 10  ; /* [25..16] */
        unsigned int    reserved_1            : 6   ; /* [31..26] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_hor_phase2_coef23;

/* Define the union u_hor_phase2_coef45 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    hor_phase2_coef4      : 10  ; /* [9..0] */
        unsigned int    reserved_0            : 6   ; /* [15..10] */
        unsigned int    hor_phase2_coef5      : 10  ; /* [25..16] */
        unsigned int    reserved_1            : 6   ; /* [31..26] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_hor_phase2_coef45;

/* Define the union u_hor_phase2_coef67 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    hor_phase2_coef6      : 10  ; /* [9..0] */
        unsigned int    reserved_0            : 6   ; /* [15..10] */
        unsigned int    hor_phase2_coef7      : 10  ; /* [25..16] */
        unsigned int    reserved_1            : 6   ; /* [31..26] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_hor_phase2_coef67;

/* Define the union u_ver_phase0_coef01 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    ver_phase0_coef0      : 10  ; /* [9..0] */
        unsigned int    reserved_0            : 6   ; /* [15..10] */
        unsigned int    ver_phase0_coef1      : 10  ; /* [25..16] */
        unsigned int    reserved_1            : 6   ; /* [31..26] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_ver_phase0_coef01;

/* Define the union u_ver_phase0_coef23 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    ver_phase0_coef2      : 10  ; /* [9..0] */
        unsigned int    reserved_0            : 6   ; /* [15..10] */
        unsigned int    ver_phase0_coef3      : 10  ; /* [25..16] */
        unsigned int    reserved_1            : 6   ; /* [31..26] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_ver_phase0_coef23;

/* Define the union u_ver_phase2_coef01 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    ver_phase2_coef0      : 10  ; /* [9..0] */
        unsigned int    reserved_0            : 6   ; /* [15..10] */
        unsigned int    ver_phase2_coef1      : 10  ; /* [25..16] */
        unsigned int    reserved_1            : 6   ; /* [31..26] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_ver_phase2_coef01;

/* Define the union u_ver_phase2_coef23 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    ver_phase2_coef2      : 10  ; /* [9..0] */
        unsigned int    reserved_0            : 6   ; /* [15..10] */
        unsigned int    ver_phase2_coef3      : 10  ; /* [25..16] */
        unsigned int    reserved_1            : 6   ; /* [31..26] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_ver_phase2_coef23;

/* Define the union u_csc_in_dc_coef */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    csc_in_dc_coef0       : 9   ; /* [8..0] */
        unsigned int    reserved_0            : 1   ; /* [9] */
        unsigned int    csc_in_dc_coef1       : 9   ; /* [18..10] */
        unsigned int    reserved_1            : 1   ; /* [19] */
        unsigned int    csc_in_dc_coef2       : 9   ; /* [28..20] */
        unsigned int    reserved_2            : 3   ; /* [31..29] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_csc_in_dc_coef;

/* Define the union u_csc_out_dc_coef */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    csc_out_dc_coef0      : 9   ; /* [8..0] */
        unsigned int    reserved_0            : 1   ; /* [9] */
        unsigned int    csc_out_dc_coef1      : 9   ; /* [18..10] */
        unsigned int    reserved_1            : 1   ; /* [19] */
        unsigned int    csc_out_dc_coef2      : 9   ; /* [28..20] */
        unsigned int    reserved_2            : 3   ; /* [31..29] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_csc_out_dc_coef;

/* Define the union u_csc_trans_coef0 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    csc_p_00              : 15  ; /* [14..0] */
        unsigned int    reserved_0            : 1   ; /* [15] */
        unsigned int    csc_p_01              : 15  ; /* [30..16] */
        unsigned int    reserved_1            : 1   ; /* [31] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_csc_trans_coef0;

/* Define the union u_csc_trans_coef1 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    csc_p_02              : 15  ; /* [14..0] */
        unsigned int    reserved_0            : 1   ; /* [15] */
        unsigned int    csc_p_10              : 15  ; /* [30..16] */
        unsigned int    reserved_1            : 1   ; /* [31] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_csc_trans_coef1;

/* Define the union u_csc_trans_coef2 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    csc_p_11              : 15  ; /* [14..0] */
        unsigned int    reserved_0            : 1   ; /* [15] */
        unsigned int    csc_p_12              : 15  ; /* [30..16] */
        unsigned int    reserved_1            : 1   ; /* [31] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_csc_trans_coef2;

/* Define the union u_csc_trans_coef3 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    csc_p_20              : 15  ; /* [14..0] */
        unsigned int    reserved_0            : 1   ; /* [15] */
        unsigned int    csc_p_21              : 15  ; /* [30..16] */
        unsigned int    reserved_1            : 1   ; /* [31] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_csc_trans_coef3;

/* Define the union u_csc_trans_coef4 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    csc_p_22              : 15  ; /* [14..0] */
        unsigned int    reserved_0            : 17  ; /* [31..15] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_csc_trans_coef4;

/* Define the union u_mtn_address0 */
typedef union {
    /* Define the struct bits  */
    struct {
        unsigned int mtn_address0           : 32  ; /* [31..0] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_mtn_address0;
/* Define the union u_mtn_address1 */
typedef union {
    /* Define the struct bits  */
    struct {
        unsigned int mtn_address1           : 32  ; /* [31..0] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_mtn_address1;
/* Define the union u_mcu_pro_startpos */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    jcfg2jvld_mcu_starty  : 10  ; /* [9..0] */
        unsigned int    reserved_0            : 22  ; /* [31..10] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_mcu_pro_startpos;

/* Define the union u_piccut_startpos */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    jcfg2jytr_pic_startx  : 13  ; /* [12..0] */
        unsigned int    reserved_0            : 3   ; /* [15..13] */
        unsigned int    jcfg2jytr_pic_starty  : 13  ; /* [28..16] */
        unsigned int    reserved_1            : 3   ; /* [31..29] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_piccut_startpos;

/* Define the union u_piccut_endpos */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    jcfg2jytr_pic_endx    : 13  ; /* [12..0] */
        unsigned int    reserved_0            : 3   ; /* [15..13] */
        unsigned int    jcfg2jytr_pic_endy    : 13  ; /* [28..16] */
        unsigned int    reserved_1            : 3   ; /* [31..29] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_piccut_endpos;

/* Define the union u_mcu_pro_curpos */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    jvld2jcfg_mcu_y       : 10  ; /* [9..0] */
        unsigned int    reserved_0            : 22  ; /* [31..10] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_mcu_pro_curpos;

/* Define the union u_bs_consu */
typedef union {
    /* Define the struct bits  */
    struct {
        unsigned int jvld2jcfg_bs_consu     : 32  ; /* [31..0] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_bs_consu;
/* Define the union u_bs_resume_data0 */
typedef union {
    /* Define the struct bits  */
    struct {
        unsigned int jbs2jcfg_bs_data0      : 32  ; /* [31..0] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_bs_resume_data0;
/* Define the union u_bs_resume_data1 */
typedef union {
    /* Define the struct bits  */
    struct {
        unsigned int jbs2jcfg_bs_data1      : 32  ; /* [31..0] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_bs_resume_data1;
/* Define the union u_bs_resume_bit */
typedef union {
    /* Define the struct bits  */
    struct {
        unsigned int jbs2jcfg_bs_remain     : 32  ; /* [31..0] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_bs_resume_bit;
/* Define the union u_bs_res_data_cfg0 */
typedef union {
    /* Define the struct bits  */
    struct {
        unsigned int jcfg2jbs_bs_data0      : 32  ; /* [31..0] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_bs_res_data_cfg0;
/* Define the union u_bs_res_data_cfg1 */
typedef union {
    /* Define the struct bits  */
    struct {
        unsigned int jcfg2jbs_bs_data1      : 32  ; /* [31..0] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_bs_res_data_cfg1;
/* Define the union u_bs_res_bit_cfg */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    jcfg2jbs_bs_remain    : 7   ; /* [6..0] */
        unsigned int    reserved_0            : 25  ; /* [31..7] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_bs_res_bit_cfg;

/* Define the union u_over_time_thd */
typedef union {
    /* Define the struct bits  */
    struct {
        unsigned int over_time_thr          : 32  ; /* [31..0] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_over_time_thd;
/* Define the union u_pd_sum_y */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    jvld2jcfg_pd_y        : 16  ; /* [15..0] */
        unsigned int    jbs2jcfg_flag_cur     : 1   ; /* [16] */
        unsigned int    reserved_0            : 15  ; /* [31..17] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_pd_sum_y;

/* Define the union u_pd_sum_cbcr */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    jvld2jcfg_pd_cb       : 16  ; /* [15..0] */
        unsigned int    jvld2jcfg_pd_cr       : 16  ; /* [31..16] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_pd_sum_cbcr;

/* Define the union u_luma_pix_sum0 */
typedef union {
    /* Define the struct bits  */
    struct {
        unsigned int luma_pix_sum0          : 32  ; /* [31..0] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_luma_pix_sum0;
/* Define the union u_luma_pix_sum1 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    luma_pix_sum1         : 4   ; /* [3..0] */
        unsigned int    reserved_0            : 27  ; /* [30..4] */
        unsigned int    jidct_luma_sum_en     : 1   ; /* [31] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_luma_pix_sum1;

/* Define the union u_axi_4k_cnt */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    wr_over_4k_cnt        : 16  ; /* [15..0] */
        unsigned int    rd_over_4k_cnt        : 16  ; /* [31..16] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_axi_4k_cnt;

/* Define the union u_axi_16m_cnt */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    srcw_cur_fsm          : 2   ; /* [1..0] */
        unsigned int    srcr_cur_fsm          : 2   ; /* [3..2] */
        unsigned int    wr_cnt                : 4   ; /* [7..4] */
        unsigned int    rd_cnt                : 4   ; /* [11..8] */
        unsigned int    wr_over_16m_cnt       : 10  ; /* [21..12] */
        unsigned int    rd_over_16m_cnt       : 10  ; /* [31..22] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_axi_16m_cnt;

/* Define the union u_sampling_factor */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    v_fac                 : 8   ; /* [7..0] */
        unsigned int    u_fac                 : 8   ; /* [15..8] */
        unsigned int    y_fac                 : 8   ; /* [23..16] */
        unsigned int    reserved_0            : 8   ; /* [31..24] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_sampling_factor;

/* Define the union u_dri */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    jcfg2jvld_ri          : 16  ; /* [15..0] */
        unsigned int    reserved_0            : 16  ; /* [31..16] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_dri;

/* Define the union u_dri_cnt_cfg */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    jcfg2jvld_dri_cnt     : 16  ; /* [15..0] */
        unsigned int    reserved_0            : 16  ; /* [31..16] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_dri_cnt_cfg;

/* Define the union u_dri_cnt */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    jvld2jcfg_dri_cnt     : 16  ; /* [15..0] */
        unsigned int    reserved_0            : 16  ; /* [31..16] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_dri_cnt;

/* Define the union u_mmu_bypass_chn */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    ar_bs_bypass          : 1   ; /* [0] */
        unsigned int    aw_idct_y_bypass      : 1   ; /* [1] */
        unsigned int    aw_idct_c_bypass      : 1   ; /* [2] */
        unsigned int    ar_mtn0_bypass        : 1   ; /* [3] */
        unsigned int    ar_mtn1_bypass        : 1   ; /* [4] */
        unsigned int    aw_mtn0_bypass        : 1   ; /* [5] */
        unsigned int    aw_mtn1_bypass        : 1   ; /* [6] */
        unsigned int    aw_rgb_bypass         : 1   ; /* [7] */
        unsigned int    aw_tunl_bypass        : 1   ; /* [8] */
        unsigned int    reserved_0            : 23  ; /* [31..9] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_mmu_bypass_chn;

/* Define the union u_jpgd_line_num */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    jcfg2idct_line_num    : 10  ; /* [9..0] */
        unsigned int    reserved_0            : 22  ; /* [31..10] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_jpgd_line_num;

/* Define the union u_jpgd_line_cnt_addr */
typedef union {
    /* Define the struct bits  */
    struct {
        unsigned int jcfg2idct_line_cnt_addr : 32  ; /* [31..0] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_jpgd_line_cnt_addr;
/* Define the union u_mem_ctrl_ras_rfs */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    mem_ctrl_rfs          : 16  ; /* [15..0] */
        unsigned int    mem_ctrl_ras          : 16  ; /* [31..16] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_mem_ctrl_ras_rfs;

/* Define the union u_mem_ctrl_rft */
typedef union {
    /* Define the struct bits  */
    struct {
        unsigned int mem_ctrl_rft           : 32  ; /* [31..0] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_mem_ctrl_rft;
/* Define the union u_bitbuffer_staddr_msb */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    bb_staddr_msb         : 2   ; /* [1..0] */
        unsigned int    reserved_0            : 30  ; /* [31..2] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_bitbuffer_staddr_msb;

/* Define the union u_bitbuffer_endaddr_msb */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    bb_endaddr_msb        : 2   ; /* [1..0] */
        unsigned int    reserved_0            : 30  ; /* [31..2] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_bitbuffer_endaddr_msb;

/* Define the union u_bitstream_staddr_msb */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    bs_staddr_msb         : 2   ; /* [1..0] */
        unsigned int    reserved_0            : 30  ; /* [31..2] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_bitstream_staddr_msb;

/* Define the union u_bitstream_endaddr_msb */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    bs_endaddr_msb        : 2   ; /* [1..0] */
        unsigned int    reserved_0            : 30  ; /* [31..2] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_bitstream_endaddr_msb;

/* Define the union u_picture_ystaddr_msb */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    pic_ystaddr_msb       : 2   ; /* [1..0] */
        unsigned int    reserved_0            : 30  ; /* [31..2] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_picture_ystaddr_msb;

/* Define the union u_picture_uvstaddr_msb */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    pic_uvstaddr_msb      : 2   ; /* [1..0] */
        unsigned int    reserved_0            : 30  ; /* [31..2] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_picture_uvstaddr_msb;

/* Define the union u_mtn_address0_msb */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    mtn_address0_msb      : 2   ; /* [1..0] */
        unsigned int    reserved_0            : 30  ; /* [31..2] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_mtn_address0_msb;

/* Define the union u_mtn_address1_msb */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    mtn_address1_msb      : 2   ; /* [1..0] */
        unsigned int    reserved_0            : 30  ; /* [31..2] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_mtn_address1_msb;

/* Define the union u_lowdly_line_num_addr_msb */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    line_cnt_addr_msb     : 2   ; /* [1..0] */
        unsigned int    reserved_0            : 30  ; /* [31..2] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_lowdly_line_num_addr_msb;

/* Define the union u_quant_table */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    qtab_y                : 8   ; /* [7..0] */
        unsigned int    qtab_cb               : 8   ; /* [15..8] */
        unsigned int    qtab_cr               : 8   ; /* [23..16] */
        unsigned int    reserved_0            : 8   ; /* [31..24] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_quant_table;

/* Define the union u_hdc_table */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    hdc_lu                : 12  ; /* [11..0] */
        unsigned int    hdc_ch                : 12  ; /* [23..12] */
        unsigned int    reserved_0            : 8   ; /* [31..24] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_hdc_table;

/* Define the union u_hac_min_table */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    hac_lu_mincode_even   : 8   ; /* [7..0] */
        unsigned int    hac_lu_mincode_odd    : 8   ; /* [15..8] */
        unsigned int    hac_ch_mincode_even   : 8   ; /* [23..16] */
        unsigned int    hac_ch_mincode_odd    : 8   ; /* [31..24] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_hac_min_table;

/* Define the union u_hac_base_table */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    hac_lu_base_even      : 8   ; /* [7..0] */
        unsigned int    hac_lu_base_odd       : 8   ; /* [15..8] */
        unsigned int    hac_ch_base_even      : 8   ; /* [23..16] */
        unsigned int    hac_ch_base_odd       : 8   ; /* [31..24] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_hac_base_table;

/* Define the union u_hac_symbol_table */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    rs_luma               : 8   ; /* [7..0] */
        unsigned int    rs_chroma             : 8   ; /* [15..8] */
        unsigned int    reserved_0            : 16  ; /* [31..16] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_hac_symbol_table;

/* Define the union u_reg0_start */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    start_0               : 1   ; /* [0] */
        unsigned int    reserved_0            : 31  ; /* [31..1] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_reg0_start;

/* Define the union u_reg0_cancel */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    cancel_0              : 1   ; /* [0] */
        unsigned int    reserved_0            : 31  ; /* [31..1] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_reg0_cancel;

/* Define the union u_reg0_frame_cnt */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    frame_cnt_0           : 8   ; /* [7..0] */
        unsigned int    reserved_0            : 24  ; /* [31..8] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_reg0_frame_cnt;

/* Define the union u_reg0_rawint */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    int_raw_dec_fnsh_0    : 1   ; /* [0] */
        unsigned int    int_raw_dec_err_0     : 1   ; /* [1] */
        unsigned int    int_raw_bs_res_0      : 1   ; /* [2] */
        unsigned int    reserved_0            : 1   ; /* [3] */
        unsigned int    int_raw_over_time_0   : 1   ; /* [4] */
        unsigned int    int_raw_line_eql_0    : 1   ; /* [5] */
        unsigned int    int_raw_cancel_0      : 1   ; /* [6] */
        unsigned int    reserved_1            : 25  ; /* [31..7] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_reg0_rawint;

/* Define the union u_reg0_intstate */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    int_dec_fnsh_0        : 1   ; /* [0] */
        unsigned int    int_dec_err_0         : 1   ; /* [1] */
        unsigned int    int_bs_res_0          : 1   ; /* [2] */
        unsigned int    reserved_0            : 1   ; /* [3] */
        unsigned int    int_over_time_0       : 1   ; /* [4] */
        unsigned int    int_line_eql_0        : 1   ; /* [5] */
        unsigned int    int_cancel_0          : 1   ; /* [6] */
        unsigned int    reserved_1            : 25  ; /* [31..7] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_reg0_intstate;

/* Define the union u_reg0_intmask */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    intm_dec_fnsh_0       : 1   ; /* [0] */
        unsigned int    intm_dec_err_0        : 1   ; /* [1] */
        unsigned int    intm_bs_res_0         : 1   ; /* [2] */
        unsigned int    reserved_0            : 1   ; /* [3] */
        unsigned int    intm_over_time_0      : 1   ; /* [4] */
        unsigned int    intm_line_eql_0       : 1   ; /* [5] */
        unsigned int    intm_cancel_0         : 1   ; /* [6] */
        unsigned int    reserved_1            : 25  ; /* [31..7] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_reg0_intmask;

/* Define the union u_reg0_intclr */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    intc_dec_fnsh_0       : 1   ; /* [0] */
        unsigned int    intc_dec_err_0        : 1   ; /* [1] */
        unsigned int    intc_bs_res_0         : 1   ; /* [2] */
        unsigned int    reserved_0            : 1   ; /* [3] */
        unsigned int    intc_over_time_0      : 1   ; /* [4] */
        unsigned int    intc_line_eql_0       : 1   ; /* [5] */
        unsigned int    intc_cancel_0         : 1   ; /* [6] */
        unsigned int    reserved_1            : 25  ; /* [31..7] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_reg0_intclr;

/* Define the union u_reg0_prio */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    prio_lvl_0            : 4   ; /* [3..0] */
        unsigned int    reserved_0            : 28  ; /* [31..4] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_reg0_prio;

/* Define the union u_reg0_addr_l */
typedef union {
    /* Define the struct bits  */
    struct {
        unsigned int cfg_addr_low_0         : 32  ; /* [31..0] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_reg0_addr_l;
/* Define the union u_reg0_addr_h */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    cfg_addr_high_0       : 2   ; /* [1..0] */
        unsigned int    reserved_0            : 30  ; /* [31..2] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_reg0_addr_h;

/* Define the union u_reg0_state */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    state_0               : 4   ; /* [3..0] */
        unsigned int    reserved_0            : 28  ; /* [31..4] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_reg0_state;

/* Define the union u_reg0_cstate */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    rg_cancel_state_0     : 4   ; /* [3..0] */
        unsigned int    cancel_state_vld_0    : 1   ; /* [4] */
        unsigned int    reserved_0            : 27  ; /* [31..5] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_reg0_cstate;

/* Define the union u_reg0_dec_time */
typedef union {
    /* Define the struct bits  */
    struct {
        unsigned int pic_dec_time_0         : 32  ; /* [31..0] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_reg0_dec_time;
/* Define the union u_reg0_luma_sum_l */
typedef union {
    /* Define the struct bits  */
    struct {
        unsigned int dec_luma_sum_l_0       : 32  ; /* [31..0] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_reg0_luma_sum_l;
/* Define the union u_reg0_luma_sum_h */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    dec_luma_sum_h_0      : 4   ; /* [3..0] */
        unsigned int    reserved_0            : 28  ; /* [31..4] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_reg0_luma_sum_h;

/* Define the union u_reg0_dbg_mode */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    dbg_mode_0            : 1   ; /* [0] */
        unsigned int    reserved_0            : 31  ; /* [31..1] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_reg0_dbg_mode;

typedef struct {
    u_reg0_start                    reg0_start                       ;
    u_reg0_cancel                   reg0_cancel                      ;
    u_reg0_frame_cnt                reg0_frame_cnt                   ;
    u_reg0_rawint                   reg0_rawint                      ;
    u_reg0_intstate                 reg0_intstate                    ;
    u_reg0_intmask                  reg0_intmask                     ;
    u_reg0_intclr                   reg0_intclr                      ;
    u_reg0_prio                     reg0_prio                        ;
    u_reg0_addr_l                   reg0_addr_l                      ;
    u_reg0_addr_h                   reg0_addr_h                      ;
    u_reg0_state                    reg0_state                       ;
    u_reg0_cstate                   reg0_cstate                      ;
    u_reg0_dec_time                 reg0_dec_time                    ;
    u_reg0_luma_sum_l               reg0_luma_sum_l                  ;
    u_reg0_luma_sum_h               reg0_luma_sum_h                  ;
    u_reg0_dbg_mode                 reg0_dbg_mode                    ;
} u_reg0_config;

/* Define the union u_jpgd_lowpower */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    jpgd_ck_en            : 1   ; /* [0] */
        unsigned int    reserved_0            : 31  ; /* [31..1] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_jpgd_lowpower;

/* Define the union u_jpgd_msg_outstd */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    outstanding           : 4   ; /* [3..0] */
        unsigned int    axi_id                : 4   ; /* [7..4] */
        unsigned int    reserved_0            : 24  ; /* [31..8] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_jpgd_msg_outstd;

/* Define the union u_jpeg_axi_sep */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    axi_4k_bypass         : 1   ; /* [0] */
        unsigned int    axi_sep_typ           : 1   ; /* [1] */
        unsigned int    reserved_0            : 30  ; /* [31..2] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_jpeg_axi_sep;

/* Define the union u_jpgd_rfs_ras */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int    mem_ctrl_rfs          : 16  ; /* [15..0] */
        unsigned int    mem_ctrl_ras          : 16  ; /* [31..16] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_jpgd_rfs_ras;

/* Define the union u_jpgd_rft */
typedef union {
    /* Define the struct bits  */
    struct {
        unsigned int mem_ctrl_rft           : 32  ; /* [31..0] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_jpgd_rft;
/* Define the union u_soft_use_0 */
typedef union {
    /* Define the struct bits  */
    struct {
        unsigned int reserved_0             : 32  ; /* [31..0] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_soft_use_0;
/* Define the union u_soft_use_1 */
typedef union {
    /* Define the struct bits  */
    struct {
        unsigned int reserved_0             : 32  ; /* [31..0] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_soft_use_1;
/* Define the union u_soft_use_2 */
typedef union {
    /* Define the struct bits  */
    struct {
        unsigned int reserved_0             : 32  ; /* [31..0] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_soft_use_2;
/* Define the union u_soft_use_3 */
typedef union {
    /* Define the struct bits  */
    struct {
        unsigned int reserved_0             : 32  ; /* [31..0] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_soft_use_3;

/* Define the union u_err_type */
typedef union {
    /* Define the struct bits */
    struct {
        // Comment of field: reg0_err_type
        unsigned int    reg0_err_type         : 3   ; /* [2..0] */
        unsigned int    reserved_0            : 5   ; /* [7..3] */
        // Comment of field: reg1_err_type
        unsigned int    reg1_err_type         : 3   ; /* [10..8] */
        unsigned int    reserved_1            : 5   ; /* [15..11] */
        // Comment of field: reg2_err_type
        unsigned int    reg2_err_type         : 3   ; /* [18..16] */
        unsigned int    reserved_2            : 5   ; /* [23..19] */
        // Comment of field: reg3_err_type
        unsigned int    reg3_err_type         : 3   ; /* [26..24] */
        unsigned int    reserved_3            : 5   ; /* [31..27] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_err_type;

/* Define the global struct */
typedef struct {
    volatile u_jpeg_dec_start                jpeg_dec_start                   ; /* 0x0 */
    volatile u_jpeg_resume_start             jpeg_resume_start                ; /* 0x4 */
    volatile u_pic_vld_num                   pic_vld_num                      ; /* 0x8 */
    volatile u_jpeg_stride                   jpeg_stride                      ; /* 0xc */
    volatile u_picture_size                  picture_size                     ; /* 0x10 */
    volatile u_picture_type                  picture_type                     ; /* 0x14 */
    volatile u_picdec_time                   picdec_time                      ; /* 0x18 */
    volatile u_rgb_out_stride                rgb_out_stride                   ; /* 0x1c */
    volatile u_bitbuffer_staddr              bitbuffer_staddr                 ; /* 0x20 */
    volatile u_bitbuffer_endaddr             bitbuffer_endaddr                ; /* 0x24 */
    volatile u_bitstreams_staddr             bitstreams_staddr                ; /* 0x28 */
    volatile u_bitstreams_endaddr            bitstreams_endaddr               ; /* 0x2c */
    volatile u_picture_ystaddr               picture_ystaddr                  ; /* 0x30 */
    volatile u_picture_uvstaddr              picture_uvstaddr                 ; /* 0x34 */
    volatile u_pd_sum_cfg_cbcr               pd_sum_cfg_cbcr                  ; /* 0x38 */
    volatile u_pd_sum_cfg_y                  pd_sum_cfg_y                     ; /* 0x3c */
    volatile u_freq_scale                    freq_scale                       ; /* 0x40 */
    volatile u_out_type                      out_type                         ; /* 0x44 */
    volatile u_jpgd_alpha                    jpgd_alpha                       ; /* 0x48 */
    volatile u_hor_phase0_coef01             hor_phase0_coef01                ; /* 0x4c */
    volatile u_hor_phase0_coef23             hor_phase0_coef23                ; /* 0x50 */
    volatile u_hor_phase0_coef45             hor_phase0_coef45                ; /* 0x54 */
    volatile u_hor_phase0_coef67             hor_phase0_coef67                ; /* 0x58 */
    volatile unsigned int                    reserved_0[4]                    ; /* 0x5c~0x68 */
    volatile u_hor_phase2_coef01             hor_phase2_coef01                ; /* 0x6c */
    volatile u_hor_phase2_coef23             hor_phase2_coef23                ; /* 0x70 */
    volatile u_hor_phase2_coef45             hor_phase2_coef45                ; /* 0x74 */
    volatile u_hor_phase2_coef67             hor_phase2_coef67                ; /* 0x78 */
    volatile unsigned int                    reserved_1[4]                    ; /* 0x7c~0x88 */
    volatile u_ver_phase0_coef01             ver_phase0_coef01                ; /* 0x8c */
    volatile u_ver_phase0_coef23             ver_phase0_coef23                ; /* 0x90 */
    volatile unsigned int                    reserved_2[2]                    ; /* 0x94~0x98 */
    volatile u_ver_phase2_coef01             ver_phase2_coef01                ; /* 0x9c */
    volatile u_ver_phase2_coef23             ver_phase2_coef23                ; /* 0xa0 */
    volatile unsigned int                    reserved_3[2]                    ; /* 0xa4~0xa8 */
    volatile u_csc_in_dc_coef                csc_in_dc_coef                   ; /* 0xac */
    volatile u_csc_out_dc_coef               csc_out_dc_coef                  ; /* 0xb0 */
    volatile u_csc_trans_coef0               csc_trans_coef0                  ; /* 0xb4 */
    volatile u_csc_trans_coef1               csc_trans_coef1                  ; /* 0xb8 */
    volatile u_csc_trans_coef2               csc_trans_coef2                  ; /* 0xbc */
    volatile u_csc_trans_coef3               csc_trans_coef3                  ; /* 0xc0 */
    volatile u_csc_trans_coef4               csc_trans_coef4                  ; /* 0xc4 */
    volatile u_mtn_address0                  mtn_address0                     ; /* 0xc8 */
    volatile u_mtn_address1                  mtn_address1                     ; /* 0xcc */
    volatile unsigned int                    reserved_4                       ; /* 0xd0 */
    volatile u_mcu_pro_startpos              mcu_pro_startpos                 ; /* 0xd4 */
    volatile u_piccut_startpos               piccut_startpos                  ; /* 0xd8 */
    volatile u_piccut_endpos                 piccut_endpos                    ; /* 0xdc */
    volatile u_mcu_pro_curpos                mcu_pro_curpos                   ; /* 0xe0 */
    volatile u_bs_consu                      bs_consu                         ; /* 0xe4 */
    volatile u_bs_resume_data0               bs_resume_data0                  ; /* 0xe8 */
    volatile u_bs_resume_data1               bs_resume_data1                  ; /* 0xec */
    volatile u_bs_resume_bit                 bs_resume_bit                    ; /* 0xf0 */
    volatile u_bs_res_data_cfg0              bs_res_data_cfg0                 ; /* 0xf4 */
    volatile u_bs_res_data_cfg1              bs_res_data_cfg1                 ; /* 0xf8 */
    volatile u_bs_res_bit_cfg                bs_res_bit_cfg                   ; /* 0xfc */
    volatile unsigned int                    reserved_5[2]                    ; /* 0x100~0x104 */
    volatile u_over_time_thd                 over_time_thd                    ; /* 0x108 */
    volatile u_pd_sum_y                      pd_sum_y                         ; /* 0x10c */
    volatile u_pd_sum_cbcr                   pd_sum_cbcr                      ; /* 0x110 */
    volatile u_luma_pix_sum0                 luma_pix_sum0                    ; /* 0x114 */
    volatile u_luma_pix_sum1                 luma_pix_sum1                    ; /* 0x118 */
    volatile u_axi_4k_cnt                    axi_4k_cnt                       ; /* 0x11c */
    volatile u_axi_16m_cnt                   axi_16m_cnt                      ; /* 0x120 */
    volatile u_sampling_factor               sampling_factor                  ; /* 0x124 */
    volatile u_dri                           dri                              ; /* 0x128 */
    volatile u_dri_cnt_cfg                   dri_cnt_cfg                      ; /* 0x12c */
    volatile u_dri_cnt                       dri_cnt                          ; /* 0x130 */
    volatile u_mmu_bypass_chn                mmu_bypass_chn                   ; /* 0x134 */
    volatile u_jpgd_line_num                 jpgd_line_num                    ; /* 0x138 */
    volatile u_jpgd_line_cnt_addr            jpgd_line_cnt_addr               ; /* 0x13c */
    volatile u_mem_ctrl_ras_rfs              mem_ctrl_ras_rfs                 ; /* 0x140 */
    volatile u_mem_ctrl_rft                  mem_ctrl_rft                     ; /* 0x144 */
    volatile u_bitbuffer_staddr_msb          bitbuffer_staddr_msb             ; /* 0x148 */
    volatile u_bitbuffer_endaddr_msb         bitbuffer_endaddr_msb            ; /* 0x14c */
    volatile u_bitstream_staddr_msb          bitstream_staddr_msb             ; /* 0x150 */
    volatile u_bitstream_endaddr_msb         bitstream_endaddr_msb            ; /* 0x154 */
    volatile u_picture_ystaddr_msb           picture_ystaddr_msb              ; /* 0x158 */
    volatile u_picture_uvstaddr_msb          picture_uvstaddr_msb             ; /* 0x15c */
    volatile u_mtn_address0_msb              mtn_address0_msb                 ; /* 0x160 */
    volatile u_mtn_address1_msb              mtn_address1_msb                 ; /* 0x164 */
    volatile u_lowdly_line_num_addr_msb      lowdly_line_num_addr_msb         ; /* 0x168 */
    volatile unsigned int                    reserved_6[37]                   ; /* 0x16c~0x1fc */
    volatile u_quant_table                   quant_table[64]                  ; /* 0x200~0x2fc */
    volatile u_hdc_table                     hdc_table[12]                    ; /* 0x300~0x32c */
    volatile unsigned int                    reserved_7[4]                    ; /* 0x330~0x33c */
    volatile u_hac_min_table                 hac_min_table[8]                 ; /* 0x340~0x35c */
    volatile u_hac_base_table                hac_base_table[8]                ; /* 0x360~0x37c */
    volatile unsigned int                    reserved_8[32]                   ; /* 0x380~0x3fc */
    volatile u_hac_symbol_table              hac_symbol_table[176]            ; /* 0x400~0x6bc */
    volatile unsigned int                    reserved_9[16]                   ; /* 0x6c0~0x6fc */
    volatile u_reg0_config                   reg0_config[4]                   ; /* 0x700~0x7fc */
    volatile unsigned int                    reserved_10[64]                  ; /* 0x800~0x8fc */
    volatile u_jpgd_lowpower                 jpgd_lowpower                    ; /* 0x900 */
    volatile u_jpgd_msg_outstd               jpgd_msg_outstd                  ; /* 0x904 */
    volatile u_jpeg_axi_sep                  jpeg_axi_sep                     ; /* 0x908 */
    volatile u_jpgd_rfs_ras                  jpgd_rfs_ras                     ; /* 0x90c */
    volatile u_jpgd_rft                      jpgd_rft                         ; /* 0x910 */
    volatile unsigned int                    reserved_11[4]                   ; /* 0x914~0x920 */
    volatile u_soft_use_0                    soft_use_0                       ; /* 0x924 */
    volatile u_soft_use_1                    soft_use_1                       ; /* 0x928 */
    volatile u_soft_use_2                    soft_use_2                       ; /* 0x92c */
    volatile u_soft_use_3                    soft_use_3                       ; /* 0x930 */
    volatile u_err_type                      err_type                         ; /* 0x934 */
} jpgd_regs_type;

typedef struct {
    u_jpeg_dec_start                jpeg_dec_start                   ; /* 0x0 */
    u_jpeg_resume_start             jpeg_resume_start                ; /* 0x4 */
    unsigned int                    reserved_0                       ; /* 0x8 */
    u_jpeg_stride                   jpeg_stride                      ; /* 0xc */
    u_picture_size                  picture_size                     ; /* 0x10 */
    u_picture_type                  picture_type                     ; /* 0x14 */
    unsigned int                    reserved_1[2]                    ; /* 0x18~0x1c */
    u_bitbuffer_staddr              bitbuffer_staddr                 ; /* 0x20 */
    u_bitbuffer_endaddr             bitbuffer_endaddr                ; /* 0x24 */
    u_bitstreams_staddr             bitstreams_staddr                ; /* 0x28 */
    u_bitstreams_endaddr            bitstreams_endaddr               ; /* 0x2c */
    u_picture_ystaddr               picture_ystaddr                  ; /* 0x30 */
    u_picture_uvstaddr              picture_uvstaddr                 ; /* 0x34 */
    u_pd_sum_cfg_cbcr               pd_sum_cfg_cbcr                  ; /* 0x38 */
    u_pd_sum_cfg_y                  pd_sum_cfg_y                     ; /* 0x3c */
    u_freq_scale                    freq_scale                       ; /* 0x40 */
    u_out_type                      out_type                         ; /* 0x44 */
    u_mcu_pro_startpos              mcu_pro_startpos                 ; /* 0x48 */
    u_bs_res_data_cfg0              bs_res_data_cfg0                 ; /* 0x4c */
    u_bs_res_data_cfg1              bs_res_data_cfg1                 ; /* 0x50 */
    u_bs_res_bit_cfg                bs_res_bit_cfg                   ; /* 0x54 */
    u_over_time_thd                 over_time_thd                    ; /* 0x58 */
    u_luma_pix_sum1                 luma_pix_sum1                    ; /* 0x5c */
    u_sampling_factor               sampling_factor                  ; /* 0x60 */
    u_dri                           dri                              ; /* 0x64 */
    u_dri_cnt_cfg                   dri_cnt_cfg                      ; /* 0x68 */
    u_mmu_bypass_chn                mmu_bypass_chn                   ; /* 0x6c */
    u_jpgd_line_num                 jpgd_line_num                    ; /* 0x70 */
    u_jpgd_line_cnt_addr            jpgd_line_cnt_addr               ; /* 0x74 */
    unsigned int                    reserved_2[2]                    ; /* 0x78~0x7c */
    u_bitbuffer_staddr_msb          bitbuffer_staddr_msb             ; /* 0x80 */
    u_bitbuffer_endaddr_msb         bitbuffer_endaddr_msb            ; /* 0x84 */
    u_bitstream_staddr_msb          bitstream_staddr_msb             ; /* 0x88 */
    u_bitstream_endaddr_msb         bitstream_endaddr_msb            ; /* 0x8c */
    u_picture_ystaddr_msb           picture_ystaddr_msb              ; /* 0x90 */
    u_picture_uvstaddr_msb          picture_uvstaddr_msb             ; /* 0x94 */
    u_lowdly_line_num_addr_msb      lowdly_line_num_addr_msb         ; /* 0x98 */
    unsigned int                    reserved_3[25]                   ; /* 0x9c~0xfc */
    u_quant_table                   quant_table[64]                  ; /* 0x100~0x1fc */
    u_hdc_table                     hdc_table[12]                    ; /* 0x200~0x22c */
    unsigned int                    reserved_4[4]                    ; /* 0x230~0x23c */
    u_hac_min_table                 hac_min_table[8]                 ; /* 0x240~0x25c */
    u_hac_base_table                hac_base_table[8]                ; /* 0x260~0x27c */
    u_rgb_out_stride                rgb_out_stride                   ; /* 0x280 */
    u_jpgd_alpha                    jpgd_alpha                       ; /* 0x284 */
    u_hor_phase0_coef01             hor_phase0_coef01                ; /* 0x288 */
    u_hor_phase0_coef23             hor_phase0_coef23                ; /* 0x28c */
    u_hor_phase0_coef45             hor_phase0_coef45                ; /* 0x290 */
    u_hor_phase0_coef67             hor_phase0_coef67                ; /* 0x294 */
    u_hor_phase2_coef01             hor_phase2_coef01                ; /* 0x298 */
    u_hor_phase2_coef23             hor_phase2_coef23                ; /* 0x29c */
    u_hor_phase2_coef45             hor_phase2_coef45                ; /* 0x2a0 */
    u_hor_phase2_coef67             hor_phase2_coef67                ; /* 0x2a4 */
    u_ver_phase0_coef01             ver_phase0_coef01                ; /* 0x2a8 */
    u_ver_phase0_coef23             ver_phase0_coef23                ; /* 0x2ac */
    u_ver_phase2_coef01             ver_phase2_coef01                ; /* 0x2b0 */
    u_ver_phase2_coef23             ver_phase2_coef23                ; /* 0x2b4 */
    u_csc_in_dc_coef                csc_in_dc_coef                   ; /* 0x2b8 */
    u_csc_out_dc_coef               csc_out_dc_coef                  ; /* 0x2bc */
    u_csc_trans_coef0               csc_trans_coef0                  ; /* 0x2c0 */
    u_csc_trans_coef1               csc_trans_coef1                  ; /* 0x2c4 */
    u_csc_trans_coef2               csc_trans_coef2                  ; /* 0x2c8 */
    u_csc_trans_coef3               csc_trans_coef3                  ; /* 0x2cc */
    u_csc_trans_coef4               csc_trans_coef4                  ; /* 0x2d0 */
    u_mtn_address0                  mtn_address0                     ; /* 0x2d4 */
    u_mtn_address1                  mtn_address1                     ; /* 0x2d8 */
    u_piccut_startpos               piccut_startpos                  ; /* 0x2dc */
    u_piccut_endpos                 piccut_endpos                    ; /* 0x2e0 */
    u_mtn_address0_msb              mtn_address0_msb                 ; /* 0x2e4 */
    u_mtn_address1_msb              mtn_address1_msb                 ; /* 0x2e8 */
    unsigned int                    reserved_5[5]                    ; /* 0x2ec~0x2fc */
    u_hac_symbol_table              hac_symbol_table[176]            ; /* 0x300~0x5bc */
} jpegd_down_msg;

int jpegd_write_msg_info(jpegd_down_msg *msg, jpegd_vpu_config *config);

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif /* End of JPEGD_MSG_H */