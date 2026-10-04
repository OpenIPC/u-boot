/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#ifndef __AIAO_REG_H__
#define __AIAO_REG_H__

#define BUFFER_ADDR_ALIGN     128 /* buffer base 128 bytes align */
#define BUFFER_ADDR_SIZE_BASE 128 /* buffer base 128 bytes */

/******************************************************************************/
/*                                   AIO reg define                           */
/******************************************************************************/
/* AIO mod reg base adddr */
#define AIAO_INT_ENA_REG    0x0 /* AIO mod int enable reg */
#define AIAO_INT_STATUS_REG 0x4 /* AIO mod int status reg */
#define AIAO_INT_RAW_REG    0x8 /* AIO mod orgin int reg */

#define AIP_SWITCH_RX_BCLK    0x0028 /* AIAO I2S RX BCLK SWITCH config reg, internal bclk select */
#define AOP_SWITCH_TX_BCLK    0x002c /* AIAO I2S TX BCLK SWITCH config reg, internal bclk select */
#define AIO_SOFT_RESET_STATUS 0x0030 /* AIO soft reset finish status */

/* aip0/aop0,aip1/aop1,aop2 */
#define AIAO_I2S08_REG_CFG0  0x0140 /* I2S08 CRG config reg0 */
#define AIAO_I2S08_REG_CFG1  0x0144 /* I2S08 CRG config reg1 */
#define AIAO_I2S09_REG_CFG0  0x0148 /* I2S09 CRG config reg0 */
#define AIAO_I2S09_REG_CFG1  0x014c /* I2S09 CRG config reg1 */

#define AUDIO_AIO_I2S_MUX_REG 0x6c  /* I2S select control reg */

#define aip_i2s_reg_cfg0(n) (0x0100 + 8 * (n))
#define aip_i2s_reg_cfg1(n) (0x0104 + 8 * (n))
#define aop_i2s_reg_cfg0(n) (0x0140 + 8 * (n))
#define aop_i2s_reg_cfg1(n) (0x0144 + 8 * (n))

/* crg id range [0, 15] */
#define aio_i2s_reg_cfg0(crg_id) (0x0100 + 8 * (crg_id))
#define aio_i2s_reg_cfg1(crg_id) (0x0104 + 8 * (crg_id))

/* aop */
#define aop_inf_attri_reg(m) (0x2000 + 0x100 * (m)) /* send chn interface attr reg */
#define aop_ctrl_reg(m)      (0x2004 + 0x100 * (m)) /* send chn control reg */

#define aop_buff_saddr_reg_h(m)   (0x207C + 0x100 * (m)) /* send chn DDR buffer origin addr reg(high 32bit) */

#define aop_buff_saddr_reg(m)      (0x2080 + 0x100 * (m)) /* send chn DDR buffer origin addr reg(low 32bit) */
#define aop_buff_size_reg(m)       (0x2084 + 0x100 * (m)) /* send chn DDR buffer size reg */
#define aop_buff_wptr_reg(m)       (0x2088 + 0x100 * (m)) /* send chn DDR buffer write addr reg */
#define aop_buff_rptr_reg(m)       (0x208C + 0x100 * (m)) /* send chn DDR buffer read addr reg */
#define aop_buff_alempty_th_reg(m) (0x2090 + 0x100 * (m))
#define aop_trans_size_reg(m)      (0x2094 + 0x100 * (m)) /* send chn data trans len reg */
#define aop_rptr_tmp_reg(m)        (0x2098 + 0x100 * (m))

#define aop_int_ena_reg(m)    (0x20A0 + 0x100 * (m)) /* send chn int enable reg */
#define aop_int_raw_reg(m)    (0x20A4 + 0x100 * (m)) /* send chn raw int reg */
#define aop_int_status_reg(m) (0x20A8 + 0x100 * (m)) /* send chn int status reg */
#define aop_int_clr_reg(m)    (0x20AC + 0x100 * (m)) /* send chn int clr reg */

// define the union u_aiao_int_ena
typedef union {
    // define the struct bits
    struct {
        unsigned int rx_ch0_int_ena : 1;       // [0]
        unsigned int rx_ch1_int_ena : 1;       // [1]
        unsigned int rx_ch2_int_ena : 1;       // [2]
        unsigned int rx_ch3_int_ena : 1;       // [3]
        unsigned int rx_ch4_int_ena : 1;       // [4]
        unsigned int rx_ch5_int_ena : 1;       // [5]
        unsigned int rx_ch6_int_ena : 1;       // [6]
        unsigned int rx_ch7_int_ena : 1;       // [7]
        unsigned int reserved_1 : 8;           // [15..8]
        unsigned int tx_ch0_int_ena : 1;       // [16]
        unsigned int tx_ch1_int_ena : 1;       // [17]
        unsigned int tx_ch2_int_ena : 1;       // [18]
        unsigned int tx_ch3_int_ena : 1;       // [19]
        unsigned int tx_ch4_int_ena : 1;       // [20]
        unsigned int tx_ch5_int_ena : 1;       // [21]
        unsigned int tx_ch6_int_ena : 1;       // [22]
        unsigned int tx_ch7_int_ena : 1;       // [23]
        unsigned int spdiftx_ch0_int_ena : 1;  // [24]
        unsigned int spdiftx_ch1_int_ena : 1;  // [25]
        unsigned int spdiftx_ch2_int_ena : 1;  // [26]
        unsigned int spdiftx_ch3_int_ena : 1;  // [27]
        unsigned int reserved_0 : 4;           // [31..28]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_aiao_int_ena;

// define the union u_i2s_crg_cfg0
typedef union {
    // define the struct bits
    struct {
        unsigned int aiao_mclk_div : 27;  // [26..0]
        unsigned int reserved_0 : 5;      // [31..27]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_i2s_crg_cfg0;

// define the union u_i2s_crg_cfg1
typedef union {
    // define the struct bits
    struct {
        unsigned int aiao_bclk_div : 4;       // [3..0]
        unsigned int aiao_fsclk_div : 3;      // [6..4]
        unsigned int reserved_1 : 1;          // [7]
        unsigned int aiao_cken : 1;           // [8]
        unsigned int aiao_srst_req : 1;       // [9]
        unsigned int aiao_bclk_oen : 1;       // [10]
        unsigned int aiao_bclk_sel : 1;       // [11]
        unsigned int aiao_bclkin_pctrl : 1;   // [12]
        unsigned int aiao_bclkout_pctrl : 1;  // [13]
        unsigned int aiao_bclk_en : 1;        // [14]
        unsigned int aiao_ws_en : 1;          // [15]
        unsigned int reserved_0 : 16;         // [31..16]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_i2s_crg_cfg1;

// define the union u_aiao_txswitch_cfg
typedef union {
    // define the struct bits
    struct {
        unsigned int inner_bclk_ws_sel_tx_00 : 4;  // [3..0]
        unsigned int inner_bclk_ws_sel_tx_01 : 4;  // [7..4]
        unsigned int inner_bclk_ws_sel_tx_02 : 4;  // [11..8]
        unsigned int inner_bclk_ws_sel_tx_03 : 4;  // [15..12]
        unsigned int inner_bclk_ws_sel_tx_04 : 4;  // [19..16]
        unsigned int inner_bclk_ws_sel_tx_05 : 4;  // [23..20]
        unsigned int inner_bclk_ws_sel_tx_06 : 4;  // [27..24]
        unsigned int inner_bclk_ws_sel_tx_07 : 4;  // [31..28]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_aiao_switch_tx_bclk;

// define the union u_tx_if_attri
typedef union {
    // define the struct bits
    struct {
        unsigned int tx_mode : 2;            // [1..0]
        unsigned int tx_i2s_precision : 2;   // [3..2]
        unsigned int tx_ch_num : 2;          // [5..4]
        unsigned int tx_underflow_ctrl : 1;  // [6]
        unsigned int tx_multislot_en : 1;    // [7]
        unsigned int tx_sd_offset : 8;       // [15..8]
        unsigned int tx_trackmode : 3;       // [18..16]
        unsigned int reserved_0 : 1;         // [19]
        unsigned int tx_sd_source_sel : 4;   // [23..20]
        unsigned int tx_sd0_sel : 2;         // [25..24]
        unsigned int tx_sd1_sel : 2;         // [27..26]
        unsigned int tx_sd2_sel : 2;         // [29..28]
        unsigned int tx_sd3_sel : 2;         // [31..30]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_tx_if_attri;

// define the union u_tx_dsp_ctrl
typedef union {
    // define the struct bits
    struct {
        unsigned int mute_en : 1;          // [0]
        unsigned int mute_fade_en : 1;     // [1]
        unsigned int reserved_3 : 6;       // [7..2]
        unsigned int volume : 7;           // [14..8]
        unsigned int reserved_2 : 1;       // [15]
        unsigned int fade_in_rate : 4;     // [19..16]
        unsigned int fade_out_rate : 4;    // [23..20]
        unsigned int reserved_1 : 3;       // [26..24]
        unsigned int bypass_en : 1;        // [27]
        unsigned int tx_enable : 1;        // [28]
        unsigned int tx_disable_done : 1;  // [29]
        unsigned int reserved_0 : 2;       // [31..30]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_tx_dsp_ctrl;

// define the union u_tx_buff_size
typedef union {
    // define the struct bits
    struct {
        unsigned int tx_buff_size : 24;  // [23..0]
        unsigned int reserved_0 : 8;     // [31..24]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_tx_buff_size;

// define the union u_tx_buff_wptr
typedef union {
    // define the struct bits
    struct {
        unsigned int tx_buff_wptr : 24;  // [23..0]
        unsigned int reserved_0 : 8;     // [31..24]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_tx_buff_wptr;

// define the union u_tx_buff_rptr
typedef union {
    // define the struct bits
    struct {
        unsigned int tx_buff_rptr : 24;  // [23..0]
        unsigned int reserved_0 : 8;     // [31..24]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_tx_buff_rptr;

// define the union u_tx_trans_size
typedef union {
    // define the struct bits
    struct {
        unsigned int tx_trans_size : 24;  // [23..0]
        unsigned int reserved_0 : 8;      // [31..24]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_tx_trans_size;

// define the union u_tx_int_ena
typedef union {
    // define the struct bits
    struct {
        unsigned int tx_trans_int_ena : 1;        // [0]
        unsigned int tx_empty_int_ena : 1;        // [1]
        unsigned int tx_alempty_int_ena : 1;      // [2]
        unsigned int tx_bfifo_empty_int_ena : 1;  // [3]
        unsigned int tx_ififo_empty_int_ena : 1;  // [4]
        unsigned int tx_stop_int_ena : 1;         // [5]
        unsigned int tx_mfade_int_ena : 1;        // [6]
        unsigned int tx_dat_break_int_ena : 1;    // [7]
        unsigned int reserved_0 : 24;             // [31..8]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_tx_int_ena;

// define the union u_tx_int_clr
typedef union {
    // define the struct bits
    struct {
        unsigned int tx_trans_int_clear : 1;        // [0]
        unsigned int tx_empty_int_clear : 1;        // [1]
        unsigned int tx_alempty_int_clear : 1;      // [2]
        unsigned int tx_bfifo_empty_int_clear : 1;  // [3]
        unsigned int tx_ififo_empty_int_clear : 1;  // [4]
        unsigned int tx_stop_int_clear : 1;         // [5]
        unsigned int tx_mfade_int_clear : 1;        // [6]
        unsigned int tx_dat_break_int_clear : 1;    // [7]
        unsigned int reserved_0 : 24;               // [31..8]
    } bits;

    // define an unsigned member
    unsigned int u32;
} u_tx_int_clr;

typedef union {
    struct {
        unsigned int pad_tx_en : 1;          /* [0] */
        unsigned int audio_rx_bclk_sel : 1;  /* [1] */
        unsigned int rx_sd_sel : 1;          /* [2] */
        unsigned int audio_mclk_sel : 1;     /* [3] */
        unsigned int codec_tx_sel : 1;       /* [4] */
        unsigned int reserved0 : 5;          /* [9..5] */
        unsigned int i2s1_tx_sd0_sel : 2;    /* [11..10] */
        unsigned int i2s1_tx_sd1_sel : 2;    /* [13..12] */
        unsigned int i2s1_tx_sd2_sel : 2;    /* [15..14] */
        unsigned int i2s1_tx_sd3_sel : 2;    /* [17..16] */
        unsigned int i2s1_tx_sd_src_sel : 4; /* [21..18] */
        unsigned int crg9_mode_sel : 1;      /* [22] */
        unsigned int reserved1 : 9;          /* [31..23] */
    } bits;
    unsigned int ul32;
} u_aiao_mux_sel;

#endif /* AIAO_REG_H */
