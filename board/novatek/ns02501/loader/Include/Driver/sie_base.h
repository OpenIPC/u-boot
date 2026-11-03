/*
    Public header file for SIE module.

    Copyright Novatek Microelectronics Corp. 2023.  All rights reserved.
*/

#ifndef _SIE_BASE_H_
#define _SIE_BASE_H_

#include "constant.h"

#ifndef ENUM_DUMMY4WORD
#define ENUM_DUMMY4WORD(name)   E_##name = 0x10000000
#endif

typedef enum {
    FUNC_PATGEN_EN        = (1<<1),
    FUNC_DVI_EN           = (1<<2),
    FUNC_OB_BYPASS_EN     = (1<<5),
    FUNC_DVS_DECOMP_EN    = (1<<10),    ///< only SIE3
    FUNC_DPC_EN           = (1<<11),    ///< only SIE1/2
    FUNC_ACT_DATAEXT_EN   = (1<<13),    ///< only SIE1/2
    FUNC_ECS_EN           = (1<<15),
    FUNC_DGAIN_EN         = (1<<16),
    FUNC_BS_H_EN          = (1<<17),
    FUNC_BS_V_EN          = (1<<18),
    FUNC_RAWENC_EN        = (1<<19),    ///< only SIE1/2
    FUNC_CGAIN_EN         = (1<<20),
    FUNC_STCS_HISTO_Y_EN  = (1<<22),
    FUNC_STCS_LA_EN       = (1<<23),
    FUNC_STCS_CA_EN       = (1<<24),
    FUNC_STCS_DGAIN_EN    = (1<<25),
    FUNC_EVS_EN           = (1<<26),    ///< only SIE3
    FUNC_DRAM_OUT0_EN     = (1<<27),
    FUNC_COMPANDING_EN    = (1<<28)     ///< only SIE1/2
} SIE_FUNCTION_SEL;

typedef enum {
    MAIN_IN_PARA_MSTR_SNR = 0,  ///< Parallel Master Sensor
    MAIN_IN_CSI_1,              ///< Serial Sensor from CSI-1
    MAIN_IN_CSI_2,              ///< Serial Sensor from CSI-2
    MAIN_IN_2PPATGEN,           ///< Self Pattern-Generator(2-pixel)
    MAIN_IN_DRAM_IN,            ///< from DRAM IN1
    MAIN_IN_MAX,
    ENUM_DUMMY4WORD(SIE_MAIN_INPUT_SEL)
} SIE_MAIN_INPUT_SEL;

typedef enum {
    DATA_PHASE_RISING  = 0,     ///< rising  edge latch/trigger
    DATA_PHASE_FALLING,         ///< falling edge latch/trigger
    DATA_PHASE_MAX,
    ENUM_DUMMY4WORD(SIE_PHASE_SEL)
} SIE_PHASE_SEL;

typedef enum {
    /* for RGGB 2x2 */
    CFAPAT_R  = 0,              ///< start pixel is R
    CFAPAT_Gr = 1,              ///< start pixel is GR
    CFAPAT_Gb = 2,              ///< start pixel is GB
    CFAPAT_B  = 3,              ///< start pixel is B

    /* for RGBIR 4x4 */
    CFAPAT_RG_GI = 0,
    CFAPAT_GB_IG = 1,
    CFAPAT_GI_BG = 2,
    CFAPAT_IG_GR = 3,
    CFAPAT_BG_GI = 4,
    CFAPAT_GR_IG = 5,
    CFAPAT_GI_RG = 6,
    CFAPAT_IG_GB = 7,

    CFAPAT_MAX,
    ENUM_DUMMY4WORD(SIE_CFAPAT_SEL)
} SIE_CFAPAT_SEL;

typedef enum {
    PATGEN_MODE_COLORBAR_V = 1, ///< Color bar, Vertical
    PATGEN_MODE_RANDOM,         ///< Random
    PATGEN_MODE_FIXED,          ///< Fixed
    PATGEN_MODE_HINCREASE,      ///< 1D Increment
    PATGEN_MODE_HVINCREASE,     ///< 2D increment
    PATGEN_MODE_MAX,
    ENUM_DUMMY4WORD(SIE_PATGEN_MODE)
} SIE_PATGEN_MODE;

typedef enum {
    DATA_PACKBUS_8 = 0,         ///<  8 bits packing
    DATA_PACKBUS_10,            ///< 10 bits packing
    DATA_PACKBUS_12,            ///< 12 bits packing
    DATA_PACKBUS_16,            ///< 16 bits packing
    DATA_PACKBUS_MAX,
    ENUM_DUMMY4WORD(SIE_DATA_PACKBUS_SEL)
} SIE_DATA_PACKBUS_SEL;

typedef enum {
    PARAL_RAW_BIT_12 = 0,       ///< 12 bits input
    PARAL_RAW_BIT_10,           ///< 10 bits input
    PARAL_RAW_BIT_MAX,
    ENUM_DUMMY4WORD(SIE_PARAL_RAW_BITDEPTH)
} SIE_PARAL_RAW_BITDEPTH;

typedef enum {
    DRAM_OUT_MODE_NORMAL = 0,
    DRAM_OUT_MODE_SINGLE,
    DRAM_OUT_MODE_MAX,
    ENUM_DUMMY4WORD(SIE_DRAM_OUT_MODE)
} SIE_DRAM_OUT_MODE;

typedef enum {
    TIMING_DELAY_0T = 0,
    TIMING_DELAY_4T,
    TIMING_DELAY_8T,
    TIMING_DELAY_16T,
    TIMING_DELAY_MAX,
    ENUM_DUMMY4WORD(SIE_TIMING_DELAY)
} SIE_TIMING_DELAY;

typedef enum {
    SIE_LOAD_DISABLE = 0,
    SIE_LOAD_FROM_SIE1,
    SIE_LOAD_FROM_SIE2,
    SIE_LOAD_FROM_MAX,
    ENUM_DUMMY4WORD(SIE_LOAD_SRC)
} SIE_LOAD_SRC;

typedef enum {
    SIE_OUTBIT_12 = 0,          ///< 12 output bits
    SIE_OUTBIT_10,              ///< 10 output bits
    SIE_OUTBIT_8,               ///< 8  output bits
    SIE_OUTBIT_MAX,
    ENUM_DUMMY4WORD(SIE_OUTBIT_SEL)
} SIE_OUTBIT_SEL;

typedef enum {
    SIE_PACK_AT_MSB  = 0,       ///< data output at MSB
    SIE_PACK_AT_LSB,            ///< data output at LSB
    SIE_PACK_AT_MAX,
    ENUM_DUMMY4WORD(SIE_PACK_FORMAT)
} SIE_PACK_FORMAT;

typedef enum {
    SIE_FMT_BAYER = 0,          ///< Bayer  2x2 format
    SIE_FMT_RGBIR,              ///< RGB-IR 4x4 format
    SIE_FMT_MAX,
    ENUM_DUMMY4WORD(SIE_BAYER_FMT)
} SIE_BAYER_FMT;

typedef enum {
    ECS_BAYER_3CH_10BIT = 0,    ///< Bayer  2x2 format
    ECS_BAYER_4CH_8BIT,         ///< RGB-IR 4x4 format
    ECS_BAYER_MAX,
    ENUM_DUMMY4WORD(SIE_ECS_BAYER_FMT)
} SIE_ECS_BAYER_FMT;

typedef enum {
    SIE_OB_SUB_OFS = 0,         ///< ob sub use ofs
    SIE_OB_SUB_OBAVG,           ///< ob sub use ob avg result
    SIE_OB_SUB_MAX,
    ENUM_DUMMY4WORD(SIE_OB_SUB_MODE)
} SIE_OB_SUB_MODE;              ///< not supported

typedef enum {
    ECS_MAPSIZE_65X65  = 0,     ///< 65x65
    ECS_MAPSIZE_49X49,          ///< 49x49
    ECS_MAPSIZE_33X33,          ///< 33x33
    ECS_MAPSIZE_MAX,
    ENUM_DUMMY4WORD(SIE_ECS_MSIZE_SEL)
} SIE_ECS_MSIZE_SEL;

typedef enum {
    STCS_HISTO_POST_GAMMA = 0,
    STCS_HISTO_PRE_GAMMA,
    STCS_HISTO_SRC_MAX,
    ENUM_DUMMY4WORD(SIE_HISTO_SRC_SEL)
} SIE_HISTO_SRC_SEL;

typedef enum {
    STCS_CA_SRC_AFTER_IRSUB = 0,
    STCS_CA_SRC_BEFORE_OB,
    STCS_CA_SRC_SEL_MAX,
    ENUM_DUMMY4WORD(SIE_CA_SRC_SEL)
} SIE_CA_SRC_SEL;

typedef enum {
    STCS_LA1_POST_CG = 0,       ///< LA1 source from data right after  color-gain
    STCS_LA1_PRE_CG,            ///< LA1 source from data right before color-gain
    STCS_LA1_SRC_MAX,
    ENUM_DUMMY4WORD(SIE_STCS_LA1_SRC_SEL)
} SIE_STCS_LA1_SRC_SEL;

typedef enum {
    STCS_LA1_RGB = 0,
    STCS_LA1_G,
    STCS_LA1_MAX,
    ENUM_DUMMY4WORD(SIE_STCS_LA_RGB2Y1_MOD_SEL)
} SIE_STCS_LA_RGB2Y1_MOD_SEL;

typedef enum {
    STCS_LA2_RGB = 0,
    STCS_LA2_G,
    STCS_LA2_MAX,
    ENUM_DUMMY4WORD(SIE_STCS_LA_RGB2Y2_MOD_SEL)
} SIE_STCS_LA_RGB2Y2_MOD_SEL;

typedef enum {
    DEF_FACT_50 = 0,            ///< defect concealment cross-channel weighting = 50%
    DEF_FACT_25,                ///< defect concealment cross-channel weighting = 25%; recommanded
    DEF_FACT_12,                ///< defect concealment cross-channel weighting = 12.5%
    DEF_FACT_06,                ///< defect concealment cross-channel weighting = 6.25%
    DEF_FACT_MAX,
    ENUM_DUMMY4WORD(SIE_DEF_FACT_SEL)
} SIE_DEF_FACT_SEL;

typedef enum {
    GAIN_FIELD_2_8 = 0,         ///< 2bits decimal, 8bits fraction
    GAIN_FIELD_3_7,             ///< 3bits decimal, 7bits fraction
    GAIN_FIELD_MAX,
    ENUM_DUMMY4WORD(SIE_GAIN_FIELD)
} SIE_GAIN_FIELD;

typedef enum {
    EV_FMT_16BIT = 0,           ///< 16bit mode
    EV_FMT_20BIT,               ///< 20bit mode
    EV_FMT_MAX,
    ENUM_DUMMY4WORD(SIE_COMPANDING_EV_FMT_SEL)
} SIE_COMPANDING_EV_FMT_SEL;

typedef enum {
    DATAEXT_ORDER_EXT_SKIP = 0,
    DATAEXT_ORDER_SKIP_EXT,
    DATAEXT_ORDER_MAX,
    ENUM_DUMMY4WORD(SIE_DATAEXT_ORDER)
} SIE_DATAEXT_ORDER;

typedef enum {
    DATAEXT_INI_OP_DROP_INI_LINE = 0,
    DATAEXT_INI_OP_KEEP_INI_LINE,
    DATAEXT_INI_OP_MAX,
    ENUM_DUMMY4WORD(SIE_DATAEXT_INI_OP)
} SIE_DATAEXT_INI_OP;

typedef enum {
    DVI_CCIR_MODE_SD = 0,       ///< SD mode (8 bits)
    DVI_CCIR_MODE_HD,           ///< HD mode (16bits)
    DVI_CCIR_MODE_MAX,
    ENUM_DUMMY4WORD(SIE_DVI_CCIR_MODE_SEL)
} SIE_DVI_CCIR_MODE_SEL;

typedef enum {
    DVI_CCIR_FMT_601 = 0,       ///< CCIR 601
    DVI_CCIR_FMT_656_EAV,       ///< CCIR 656 EAV code recognition
    DVI_CCIR_FMT_656_ACT,       ///< CCIR 656 ACT size setting (ACT_WIDTH and ACT_HEIGHT are used to determine active region size)
    DVI_CCIR_FMT_MAX,
    ENUM_DUMMY4WORD(SIE_DVI_CCIR_FMT_SEL)
} SIE_DVI_CCIR_FMT_SEL;

typedef enum {
    DVI_CCIR_VD_MODE_FIELD  = 0,///< interlace
    DVI_CCIR_VD_MODE_SAV,       ///< progressive
    DVI_CCIR_VD_MODE_MAX,
    ENUM_DUMMY4WORD(SIE_DVI_CCIR_VD_MODE_SEL)
} SIE_DVI_CCIR_VD_MODE_SEL;

typedef enum {
    DVI_OUT_SWAP_YUYV = 0,
    DVI_OUT_SWAP_YVYU,
    DVI_OUT_SWAP_UYVY,
    DVI_OUT_SWAP_VYUY,
    DVI_OUT_SWAP_MAX,
    ENUM_DUMMY4WORD(SIE_DVI_OUT_SWAP_SEL)
} SIE_DVI_OUT_SWAP_SEL;         ///< output swap for DVI_IN_FMT_YUV422

typedef enum {
	ENC_ADJ_MODE_NORMAL = 0,    ///< rawenc parameters for normal    mode
	ENC_ADJ_MODE_AGGRESIVE,     ///< rawenc parameters for aggresive mode
	ENC_ADJ_MODE_MAX,
	ENUM_DUMMY4WORD(SIE_RAWENC_ADJ_MODE_SEL)
} SIE_RAWENC_ADJ_MODE_SEL;

typedef enum {
    ENC_RATE_50 = 0,
    ENC_RATE_58,
    ENC_RATE_66,            ///< not supported
    ENC_RATE_41,            ///< not supported
    ENC_RATE_MAX,
    ENUM_DUMMY4WORD(SIE_RAWENC_RATE_SEL)
} SIE_RAWENC_RATE_SEL;

typedef enum {
    ENC_SHARE_NONE = 0,
    ENC_SHARE_SHARED,
    ENC_SHARE_MAX,
    ENUM_DUMMY4WORD(SIE_RAWENC_SHARE_SEL)
} SIE_RAWENC_SHARE_SEL;

typedef enum {
    ENC_FRM_RST_ID_0 = 0,
    ENC_FRM_RST_ID_1,
    ENC_FRM_RST_ID_MAX,
    ENUM_DUMMY4WORD(SIE_RAWENC_FRM_RST_ID_SEL)
} SIE_RAWENC_FRM_RST_ID_SEL;                ///< only supported for SIE1

typedef struct {
    UINT8                   tbl_idx[8];     ///< Raw encoder - Quality DCT Table
    UINT8                   lvl_thres[4];   ///< Raw encoder - Quality DCT Level Threshold
} SIE_RAWENC_DCT_PARAM;

typedef struct {
    SIE_RAWENC_ADJ_MODE_SEL adj_mode;       ///< Raw encoder - Table parameter adjust mode
    SIE_RAWENC_RATE_SEL     rate;           ///< Raw encoder - Target rate of encoded bitstream per segment
    BOOL                    gamma_en;       ///< Raw encoder - Gamma
    SIE_RAWENC_DCT_PARAM    dct;            ///< Raw encoder - DCT  Param
} SIE_RAWENC_PARAM;

typedef struct {
    SIE_PHASE_SEL           vd_phase;       ///< VD phase
    SIE_PHASE_SEL           hd_phase;       ///< HD phase
    SIE_PHASE_SEL           data_phase;     ///< Data phase
    BOOL                    vd_inv;         ///< VD Invert
    BOOL                    hd_inv;         ///< HD Invert
    SIE_PARAL_RAW_BITDEPTH  raw_bitdepsel;
} SIE_PARAL_SIGNAL_PARAM;

typedef struct {
    SIE_PATGEN_MODE patgen_mode;
    UINT32          patgen_val;
} SIE_PATGEN_PARAM;

typedef struct {
    UINT32  sz_x;               ///< horizontal size
    UINT32  sz_y;               ///< vertical size
} SIE_SRC_WIN_PARAM;

typedef struct {
    UINT16          st_x;       ///< horizontal start
    UINT16          st_y;       ///< vertical start
    UINT32          sz_x;       ///< horizontal size
    UINT32          sz_y;       ///< vertical size
    SIE_CFAPAT_SEL  cfa_pat;    ///< CFA start pattern
} SIE_ACT_WIN_PARAM;

typedef struct {
    UINT16          st_x;       ///< horizontal start
    UINT16          st_y;       ///< vertical start
    UINT32          sz_x;       ///< horizontal size
    UINT32          sz_y;       ///< vertical size
    SIE_CFAPAT_SEL  cfa_pat;    ///< CFA start pattern
} SIE_CRP_WIN_PARAM;

typedef struct {
    SIE_TIMING_DELAY vd_delay;
    SIE_TIMING_DELAY hd_delay;
} SIE_VDHD_DELAY_PARAM;

typedef enum {
    DRAM_BURST_LEN_64 = 0,
    DRAM_BURST_LEN_48,
    DRAM_BURST_LEN_32,
    DRAM_BURST_LEN_16,
    DRAM_BURST_LEN_MAX,
    ENUM_DUMMY4WORD(SIE_DRAM_BURST_LEN_SEL)
} SIE_DRAM_BURST_LEN_SEL;

typedef enum {
    DRAM_OUT0_BURST_LEN_128 = 0,
    DRAM_OUT0_BURST_LEN_96,
    DRAM_OUT0_BURST_LEN_64,
    DRAM_OUT0_BURST_LEN_32,
    DRAM_OUT0_BURST_LEN_MAX,
    ENUM_DUMMY4WORD(SIE_DRAM_OUT0_BURST_LEN_SEL)
} SIE_DRAM_OUT0_BURST_LEN_SEL;

typedef enum {
    DRAM_OUT2_BURST_LEN_32 = 0,
    DRAM_OUT2_BURST_LEN_16,
    DRAM_OUT2_BURST_LEN_MAX,
    ENUM_DUMMY4WORD(SIE_DRAM_OUT2_BURST_LEN_SEL)
} SIE_DRAM_OUT2_BURST_LEN_SEL;

typedef struct {
    SIE_DRAM_OUT0_BURST_LEN_SEL out0;
    SIE_DRAM_BURST_LEN_SEL      out1;
    SIE_DRAM_OUT2_BURST_LEN_SEL out2;
    SIE_DRAM_BURST_LEN_SEL      in1;
    SIE_DRAM_BURST_LEN_SEL      in2;
} SIE_DRAM_BURST_LEN_PARAM;

typedef struct {
    UINT32  addr_pa;
} SIE_DRAM_ADDR;

typedef struct {
    UINT32                lofs;             ///< line-offset
    SIE_DATA_PACKBUS_SEL  pack_bus;         ///< pack bus bit number
} SIE_DRAM_IN1_PARAM;

typedef struct {
    UINT32                lofs;             ///< line-offset
    SIE_DATA_PACKBUS_SEL  pack_bus;         ///< pack bus bit number
    BOOL                  hflip;            ///< mirror
    BOOL                  vflip;            ///< flip
    BOOL                  ringbuf_en;
    UINT16                ringbuf_len;
    SIE_OUTBIT_SEL        out_bitdepth;
    SIE_PACK_FORMAT       pack_fmt;
} SIE_DRAM_OUT0_PARAM;

typedef struct {
    UINT32                lofs;             ///< line-offset
    BOOL                  vflip;            ///< H flip
    BOOL                  hflip;            ///< V flip
} SIE_DRAM_OUT1_PARAM;

typedef struct {
    UINT8   pos_code;                       ///< positive code value
    UINT8   neg_code;                       ///< negative code value
    UINT8   nochg_code;                     ///< no changed code value
} SIE_DVS_CODE_PARAM;

typedef struct {
    UINT32  sz_x;
    UINT32  sz_y;
} SIE_DVS_WIN_PARAM;

typedef struct {
    UINT32  mot_out_sz;             ///< must > 0 and align 4
} SIE_EVS_OUT_PARAM;

typedef struct {
    UINT16          r_gain;         ///< Color Gain, R  Gain
    UINT16          gr_gain;        ///< Color Gain, Gr Gain
    UINT16          gb_gain;        ///< Color Gain, Gb Gain
    UINT16          b_gain;         ///< Color Gain, B  Gain
    UINT16          ir_gain;        ///< Color Gain, B  Gain
    SIE_GAIN_FIELD  gain_sel;       ///< gain filed selection
} SIE_CGAIN_PARAM;

typedef struct {
    UINT32  gain_8p8bit;            ///< digital gain in 8.8 bits format
} SIE_DGAIN_PARAM;

typedef struct {
    UINT16 decomp_kp[11];          ///< 0 ~ 0xfff
    UINT16 decomp_sp[12];          ///< 0 ~ 0xfff
    UINT8  decomp_sb[12];          ///< 0 ~ 0xf
} SIE_DECOMPANDING_PARAM;

typedef struct {
    UINT8  comp_kp[11];            ///< 0 ~ 0x1f
    UINT16 comp_sp[12];            ///< 0 ~ 0xfff
    UINT8  comp_sb[12];            ///< 0 ~ 0xf
} SIE_COMPANDING_PARAM;

typedef struct {
    UINT16  ofs;
} SIE_OB_OFS_PARAM;

typedef struct {
    BOOL                dither_en;      ///< dithering enable
    BOOL                dither_rst;     ///< dithering reset
    UINT32              dither_lvl;     ///< dithering level; 0x0=bit[1:0], 0x7=bit[8:7]
    SIE_ECS_MSIZE_SEL   map_size_sel;   ///< ecs table size
    UINT32              map_shift;      ///< multiplication shift bit number
} SIE_ECS_PARAM;

typedef struct {
    UINT32  hscl_fctr;                  ///< map horizontal scaling factor
    UINT32  vscl_fctr;                  ///< map vertical scaling factor
} SIE_ECS_SCAL_PARAM;

typedef struct {
    SIE_DEF_FACT_SEL    fact;
    SIE_CFAPAT_SEL      cfa_pat;
} SIE_DPC_PARAM;

typedef struct {
    SIE_DATAEXT_ORDER   ext_order;
    SIE_DATAEXT_INI_OP  ini_line_op;
    UINT8               ext_data_skip;
    UINT8               ext_data_len;
    UINT16              ini_line_len;
} SIE_DATAEXT_PARAM;

typedef struct {
    BOOL    src_intpV;      ///< vertical interpolation, only for input width<=8188 or 5504
    UINT32  iv;             ///< bayer scaling - integration value
    UINT32  sv;             ///< bayer scaling - skipping value
    UINT32  bvR;            ///< bayer scaling - begin value for R/Gb channel
    UINT32  bvB;            ///< bayer scaling - begin value for B/Gr channel
    UINT32  out_sz;         ///< bayer scaling - output size
    UINT32  divM[2];        ///< bayer scaling - division parameter M
    UINT32  divS[2];        ///< bayer scaling - division parameter S
} SIE_BS_H_PARAM;

typedef struct {
    UINT32  iv;             ///< bayer scaling - integration value
    UINT32  sv;             ///< bayer scaling - skipping value
    UINT32  bvR;            ///< bayer scaling - begin value for R/Gb channel
    UINT32  bvB;            ///< bayer scaling - begin value for B/Gr channel
    UINT32  out_sz;         ///< bayer scaling - output size
    UINT32  divM[2];        ///< bayer scaling - division parameter M
    UINT32  divS[2];        ///< bayer scaling - division parameter S
    UINT32  in_shift;       ///< bayer scaling - input shift
    UINT32  divMRb[16];     ///< bayer scaling - division parameter M for R/B
    UINT32  divSRb[16];     ///< bayer scaling - division parameter S for R/B
} SIE_BS_V_PARAM;

typedef struct {
    UINT32  in_sz_x;        ///< input  horizontal size
    UINT32  in_sz_y;        ///< input  vertical size
    UINT32  out_sz_x;       ///< output horizontal size
    UINT32  out_sz_y;       ///< output vertical size
    UINT32  lpf;            ///< low-pass valumn, 0~100, (suggestion: 50)
    UINT32  bin_pwr;        ///< binning, 0~2, (suggestion: 0)
    BOOL    adaptivelpf_en; ///< 0: LPF is absolute value; 1: LPF is adaptive value (suggestion: 1)
} SIE_BS_PARAM;

typedef struct {
    BOOL                       la_cg_en;
    BOOL                       la_gamma_en;
    BOOL                       la_th_en;            ///< not supported
    BOOL                       ca_th_en;
    UINT16                     companding_shift;    ///< must setup when companding enable and roi_acc or ca/la enable
    SIE_HISTO_SRC_SEL          histo_src_sel;
    SIE_STCS_LA1_SRC_SEL       la1_src_sel;
    SIE_STCS_LA_RGB2Y1_MOD_SEL la_rgb2y1_mod;
    SIE_STCS_LA_RGB2Y2_MOD_SEL la_rgb2y2_mod;
    SIE_CA_SRC_SEL             ca_accm_src;
} SIE_STCS_PATH_PARAM;

typedef struct {
    SIE_CFAPAT_SEL  ca_cfa_pat;
    UINT32          ca_roi_stx;
    UINT32          ca_roi_sty;
    UINT32          ca_roi_szx;
    UINT32          ca_roi_szy;
    UINT32          ca_win_numx;
    UINT32          ca_win_numy;
    UINT32          la_roi_stx;
    UINT32          la_roi_sty;
    UINT32          la_roi_szx;
    UINT32          la_roi_szy;
    UINT32          la_win_numx;
    UINT32          la_win_numy;
} SIE_STCS_CALASIZE_INFO;

typedef struct {
    UINT16  ca_ob_ofs;          ///< OB ofs for CA
    UINT16  la_ob_ofs;          ///< OB ofs for LA
} SIE_STCS_OB_PARAM;

typedef struct {
    UINT32  fctr_x;             ///< horizontal scaling factor
    UINT32  fctr_y;             ///< vertical   scaling factor
} SIE_CA_SCALE_PARAM;

typedef struct {
    UINT32  ca_irsub_rwet;      ///< IR weight for subtraction, R channel
    UINT32  ca_irsub_gwet;      ///< IR weight for subtraction, G channel
    UINT32  ca_irsub_bwet;      ///< IR weight for subtraction, B channel
} SIE_CA_IRSUB_PARAM;

typedef struct {
    UINT32  la_irsub_rwet;      ///< IR weight for subtraction, R channel
    UINT32  la_irsub_gwet;      ///< IR weight for subtraction, G channel
    UINT32  la_irsub_bwet;      ///< IR weight for subtraction, B channel
} SIE_LA_IRSUB_PARAM;

typedef struct {
    UINT32         st_x;        ///< horizontal start
    UINT32         st_y;        ///< vertical start
    UINT32         sz_x;        ///< horizontal size
    UINT32         sz_y;        ///< vertical size
    SIE_CFAPAT_SEL cfa_pat;     ///< CFA start pattern
} SIE_CA_CROP_PARAM;

typedef struct {
    UINT32  st_x;               ///< horizontal start
    UINT32  st_y;               ///< vertical start
    UINT32  sz_x;               ///< horizontal size
    UINT32  sz_y;               ///< vertical size
} SIE_LA_CROP_PARAM;

typedef struct {
    UINT32  win_numx;           ///< color accumulation - horizontal window number
    UINT32  win_numy;           ///< color accumulation - vertical window number
    UINT32  win_szx;            ///< color accumulation - horizontal window size
    UINT32  win_szy;            ///< color accumulation - vertical window size
} SIE_CA_WIN_PARAM;

typedef struct {
    UINT32  win_numx;           ///< color accumulation - horizontal window number
    UINT32  win_numy;           ///< color accumulation - vertical window number
    UINT32  win_szx;            ///< color accumulation - horizontal window size
    UINT32  win_szy;            ///< color accumulation - vertical window size
} SIE_LA_WIN_PARAM;

typedef struct {
    UINT16  r_gain;             ///< R  Gain
    UINT16  g_gain;             ///< G  Gain
    UINT16  b_gain;             ///< B  Gain
} SIE_LA_CG_PARAM;

typedef struct {
    UINT16  *buf_la1;
    UINT16  *buf_la2;
} SIE_LA_RSLT;

typedef struct {
    UINT16  *buf_r;
    UINT16  *buf_g;
    UINT16  *buf_b;
    UINT16  *buf_ir;
    UINT16  *acc_cnt;
} SIE_CA_RSLT;

typedef struct {
    UINT16  g_th_lower;
    UINT16  g_th_upper;
    UINT16  rg_th_lower;
    UINT16  rg_th_upper;
    UINT16  bg_th_lower;
    UINT16  bg_th_upper;
    UINT16  pg_th_lower;
    UINT16  pg_th_upper;
} SIE_CA_TH_PARAM;

typedef struct {
    SIE_CA_WIN_PARAM  ca_win;
    SIE_STCS_OB_PARAM stcs_ob;
    SIE_CGAIN_PARAM   cgain;
    SIE_BAYER_FMT     bayer_fmt;
} SIE_CA_RSLT_PARAMS;

typedef struct {
    UINT16  *buf_hist;
} SIE_HIST_RSLT;

typedef struct {
    SIE_DVI_CCIR_MODE_SEL       mode;               ///< SD(8bits)/HD(16bits)
    SIE_DVI_CCIR_FMT_SEL        fmt;                ///< CCIR601/CCIR656
    SIE_DVI_CCIR_VD_MODE_SEL    vd_mode;            ///< 0:interlace 1:progressive
    BOOL                        id_auto_align;      ///< demux channel data base on CHID in EAV/SAV, 0:disable 1:enable
    BOOL                        field_enb;          ///< check field_sel and output corresponding frame
    BOOL                        field_sel;          ///< 0:receive data when field=0, 1:receive data when field=1
    BOOL                        hd_in_swap;         ///< byte swap control when in HD(16bit) mode

    UINT8                       data_period;        ///< receive data period used with data_idx
    UINT8                       data_idx;           ///< index of selected pixel, as target channel id if enable ccir_id_auto_align
} SIE_DVI_CCIR_PARAM;

typedef struct {
    SIE_DVI_OUT_SWAP_SEL        out_swap;           ///< swap control for DVI_IN_FMT_YUV422
    BOOL                        out_split;          ///< must enable when DVI_IN_FMT_YUV420 or out_yuv420_en = 1
    BOOL                        out_yuv420_en;      ///< YUV422 to YUV420 control for DVI_IN_FMT_YUV422

    SIE_DVI_CCIR_PARAM          ccir;               ///< ccir receive parameter
} SIE_DVI_PARAM;

/*************************************************************************************
 *  Public Set Function Prototype [Direct apply]
 *************************************************************************************/
int sie_eng_set_soft_reset_hw_reg(void *handle, BOOL enb);
int sie_eng_set_checksum_en_hw_reg(void *handle, BOOL enb);
int sie_eng_set_interrupt_enable_hw_reg(void *handle, UINT32 int_en);
int sie_eng_set_load_hw_reg(void *handle);
int sie_eng_set_dramin_start_hw_reg(void *handle);
int sie_eng_set_main_input_hw_reg(void *handle, SIE_MAIN_INPUT_SEL mainInInfo);
int sie_eng_set_vdhd_delay_hw_reg(void *handle, SIE_VDHD_DELAY_PARAM *p_dly_info);
int sie_eng_set_global_load_src_hw_reg(void *handle, SIE_LOAD_SRC src);
int sie_eng_set_parallel_sig_hw_reg(void *handle, SIE_PARAL_SIGNAL_PARAM *p_paral_info);
int sie_eng_set_dvs_code_hw_reg(void *handle, SIE_DVS_CODE_PARAM *p_dvs);
int sie_eng_set_ccir_header_check_hw_reg(void *handle, UINT32 header);

int sie_eng_set_companding_hw_reg(void *handle, SIE_COMPANDING_PARAM *p_companding);
int sie_eng_set_decompanding_hw_reg(void *handle, SIE_DECOMPANDING_PARAM *p_decompanding);

int sie_eng_set_la_gamma_hw_reg(void *handle, UINT16 *p_gam_tbl);

int sie_eng_set_rawenc_share_sel_hw_reg(void *handle, SIE_RAWENC_SHARE_SEL share);
int sie_eng_set_rawenc_frm_rst_id_hw_reg(void *handle, SIE_RAWENC_FRM_RST_ID_SEL frm_rst_id);

int sie_eng_set_dma_ch_disable_hw_reg(void *handle, BOOL disable);
int sie_eng_set_tcm_wait_b_resp_enb_hw_reg(void *handle, BOOL enb);

/*************************************************************************************
 *  Public Set Function Prototype [VD latched]
 *************************************************************************************/
int sie_eng_set_ipp_ctrl_buf_reg(void *handle, BOOL direct2ipp_en);
int sie_eng_set_ydcc_direct_buf_reg(void *handle, BOOL enb);
int sie_eng_set_function_enable_buf_reg(void *handle, UINT32 func_en);
int sie_eng_set_single_function_enable_buf_reg(void *handle, SIE_FUNCTION_SEL func_item, BOOL enb);
int sie_eng_set_bayer_fmt_buf_reg(void *handle, SIE_BAYER_FMT fmt);
int sie_eng_set_ecs_mode_buf_reg(void *handle, SIE_ECS_BAYER_FMT mode);
int sie_eng_set_dvi_buf_reg(void *handle, SIE_DVI_PARAM *p_dvi);
int sie_eng_set_patgen_buf_reg(void *handle, SIE_PATGEN_PARAM *p_param);

int sie_eng_set_rawenc_rate_buf_reg(void *handle, SIE_RAWENC_RATE_SEL rate);
int sie_eng_set_rawenc_gamma_en_buf_reg(void *handle, BOOL enb);
int sie_eng_set_rawenc_dct_buf_reg(void *handle, SIE_RAWENC_DCT_PARAM *p_dct);
int sie_eng_set_rawenc_param_buf_reg(void *handle, SIE_RAWENC_ADJ_MODE_SEL mode, SIE_RAWENC_RATE_SEL rate);

int sie_eng_set_src_win_buf_reg(void *handle, SIE_SRC_WIN_PARAM *p_src_win);
int sie_eng_set_act_win_buf_reg(void *handle, SIE_ACT_WIN_PARAM *p_act_win);
int sie_eng_set_crop_win_buf_reg(void *handle, SIE_CRP_WIN_PARAM *p_crp_win);

int sie_eng_set_bp1_buf_reg(void *handle, UINT32 bp);
int sie_eng_set_bp2_buf_reg(void *handle, UINT32 bp);
int sie_eng_set_bp3_buf_reg(void *handle, UINT32 bp);

int sie_eng_set_cgain_buf_reg(void *handle, SIE_CGAIN_PARAM *p_cgain);
int sie_eng_set_dgain_buf_reg(void *handle, SIE_DGAIN_PARAM *p_dgain);

int sie_eng_set_ecs_buf_reg(void *handle, SIE_ECS_PARAM *p_ecs, SIE_ACT_WIN_PARAM *p_actwin);
int sie_eng_set_ecs_scale_buf_reg(void *handle, SIE_ECS_SCAL_PARAM *p_ecs_scal);

int sie_eng_set_dpc_buf_reg(void *handle, SIE_DPC_PARAM *p_dpc);

int sie_eng_set_ob_ofs_buf_reg(void *handle, SIE_OB_OFS_PARAM *p_ob_ofs);

int sie_eng_set_dataext_buf_reg(void *handle, SIE_DATAEXT_PARAM *p_dataext);

int sie_eng_set_bs_buf_reg(void *handle, SIE_BS_PARAM *p_bs);
int sie_eng_set_bsh_buf_reg(void *handle, SIE_BS_H_PARAM *p_bsh);
int sie_eng_set_bsv_buf_reg(void *handle, SIE_BS_V_PARAM *p_bsv);

int sie_eng_set_stcs_path_buf_reg(void *handle, SIE_STCS_PATH_PARAM *p_stcs_path);
int sie_eng_set_stcs_ob_buf_reg(void *handle, SIE_STCS_OB_PARAM *p_stcs_ob);

int sie_eng_set_ca_crop_buf_reg(void *handle, SIE_CA_CROP_PARAM *p_ca_crop);
int sie_eng_set_ca_scale_buf_reg(void *handle, SIE_CA_SCALE_PARAM *p_ca_scal);
int sie_eng_set_ca_win_buf_reg(void *handle, SIE_CA_WIN_PARAM *p_la_win);
int sie_eng_set_ca_th_buf_reg(void *handle, SIE_CA_TH_PARAM *p_ca_th);
int sie_eng_set_ca_irsub_buf_reg(void *handle, SIE_CA_IRSUB_PARAM *p_ca_irsub);

int sie_eng_set_la_crop_buf_reg(void *handle, SIE_LA_CROP_PARAM *p_la_crop);
int sie_eng_set_la_win_buf_reg(void *handle, SIE_LA_WIN_PARAM *p_la_win);
int sie_eng_set_la_irsub_buf_reg(void *handle, SIE_LA_IRSUB_PARAM *p_la_irsub);
int sie_eng_set_la_cg_buf_reg(void *handle, SIE_LA_CG_PARAM *p_la_cg);

int sie_eng_set_cala_size_buf_reg(void *handle, SIE_STCS_CALASIZE_INFO *p_sz_info, SIE_LA_WIN_PARAM *p_win_param);

int sie_eng_set_dvs_win_buf_reg(void *handle, SIE_DVS_WIN_PARAM *p_dvs_win);
int sie_eng_set_evs_out_buf_reg(void *handle, SIE_EVS_OUT_PARAM *p_evs_out);

int sie_eng_set_dramin1_addr_buf_reg(void *handle, SIE_DRAM_ADDR *p_in1);
int sie_eng_set_dramin2_addr_buf_reg(void *handle, SIE_DRAM_ADDR *p_in2);
int sie_eng_set_dramout0_addr_buf_reg(void *handle, SIE_DRAM_ADDR *p_dout0);
int sie_eng_set_dramout1_addr_buf_reg(void *handle, SIE_DRAM_ADDR *p_dout1);
int sie_eng_set_dramout2_addr_buf_reg(void *handle, SIE_DRAM_ADDR *p_dout2);

int sie_eng_set_dramin1_buf_reg(void *handle, SIE_DRAM_IN1_PARAM *p_din1);
int sie_eng_set_dramout0_buf_reg(void *handle, SIE_DRAM_OUT0_PARAM *p_dout0);
int sie_eng_set_dramout1_buf_reg(void *handle, SIE_DRAM_OUT1_PARAM *p_dout1);

int sie_eng_set_dramout0_singleout_mode_buf_reg(void *handle, SIE_DRAM_OUT_MODE mode);
int sie_eng_set_dramout1_singleout_mode_buf_reg(void *handle, SIE_DRAM_OUT_MODE mode);
int sie_eng_set_dramout2_singleout_mode_buf_reg(void *handle, SIE_DRAM_OUT_MODE mode);

int sie_eng_set_dramout0_singleout_en_buf_reg(void *handle, BOOL enb);
int sie_eng_set_dramout1_singleout_en_buf_reg(void *handle, BOOL enb);
int sie_eng_set_dramout2_singleout_en_buf_reg(void *handle, BOOL enb);

int sie_eng_set_dram_burst_len_buf_reg(void *handle, SIE_DRAM_BURST_LEN_PARAM *p_burst_len);

/*************************************************************************************
 *  Public Get Function Prototype
 *************************************************************************************/
int sie_eng_get_dramout0_singleout_en_buf_reg(void *handle, BOOL *enb);
int sie_eng_get_dramout1_singleout_en_buf_reg(void *handle, BOOL *enb);
int sie_eng_get_dramout2_singleout_en_buf_reg(void *handle, BOOL *enb);
int sie_eng_get_dramout3_singleout_en_buf_reg(void *handle, BOOL *enb);

int sie_eng_get_dramin1_buf_reg(void *handle, SIE_DRAM_IN1_PARAM *p_din1);
int sie_eng_get_dramout0_buf_reg(void *handle, SIE_DRAM_OUT0_PARAM *p_dout0);
int sie_eng_get_dramout1_buf_reg(void *handle, SIE_DRAM_OUT1_PARAM *p_dout1);

int sie_eng_get_bayer_fmt_buf_reg(void *handle, SIE_BAYER_FMT *p_fmt);

int sie_eng_get_cgain_buf_reg(void *handle, SIE_CGAIN_PARAM *p_cgain);
int sie_eng_get_dgain_buf_reg(void *handle, SIE_DGAIN_PARAM *p_dgain);

int sie_eng_get_ca_rslt(void *handle, UINT32 addr_va, SIE_CA_RSLT *p_ca_rslt, SIE_CA_RSLT_PARAMS *p_ca_rslt_info);
int sie_eng_get_la_rslt(void *handle, UINT32 addr_va, SIE_LA_RSLT *p_la_rslt, SIE_LA_WIN_PARAM *p_la_win);
int sie_eng_get_hist(void *handle, SIE_HIST_RSLT *p_hist_rslt);

int sie_eng_get_function_enable_buf_reg(void *handle, UINT32 *func_en);
int sie_eng_get_crop_win_buf_reg(void *handle, SIE_CRP_WIN_PARAM *p_crp_win);
int sie_eng_get_dvi_buf_reg(void *handle, SIE_DVI_PARAM *p_dvi);
int sie_eng_get_ccir_header_status_hw_reg(void *handle, UINT32 *status);

int sie_eng_get_bsh_buf_reg(void *handle, SIE_BS_H_PARAM *p_bsh);
int sie_eng_get_bsv_buf_reg(void *handle, SIE_BS_V_PARAM *p_bsv);

int sie_eng_get_stcs_path_buf_reg(void *handle, SIE_STCS_PATH_PARAM *p_stcs_path);
int sie_eng_get_stcs_ob_buf_reg(void *handle, SIE_STCS_OB_PARAM *p_stcs_ob);

int sie_eng_get_ca_crop_buf_reg(void *handle, SIE_CA_CROP_PARAM *p_ca_crop);
int sie_eng_get_la_crop_buf_reg(void *handle, SIE_LA_CROP_PARAM *p_la_crop);
int sie_eng_get_ca_win_buf_reg(void *handle, SIE_CA_WIN_PARAM *p_ca_win);
int sie_eng_get_la_win_buf_reg(void *handle, SIE_LA_WIN_PARAM *p_la_win);

int sie_eng_get_evs_total_cnt_hw_reg(void *handle, UINT32 *p_cnt);

int sie_eng_get_ir_level(void *handle, UINT32 *level);
int sie_eng_calc_ir_level(void *handle, SIE_CA_RSLT *ca_rst, SIE_CA_WIN_PARAM *p_ca_win, SIE_CA_IRSUB_PARAM *p_ca_ir_sub);

#endif  /* _SIE_BASE_H_ */
