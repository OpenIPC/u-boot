/*
    Public header file for SIE module.

    Copyright Novatek Microelectronics Corp. 2023.  All rights reserved.
*/
#ifndef _SIE_LIMIT_H_
#define _SIE_LIMIT_H_

/***********************************************************************************************
 * SIE Master Clock Max Frequency Definition
 ***********************************************************************************************/
#define SIE1_MAX_CLK_FREQ                       320000000   ///< 320MHz, 2-pixel base, BCC_2P
#define SIE2_MAX_CLK_FREQ                       320000000   ///< 320MHz, 2-pixel base, BCC_SIE1(Shared)
#define SIE3_MAX_CLK_FREQ                       320000000   ///< 320MHz, 2-pixel base

/***********************************************************************************************
 * SIE Global Hardware Limitation Definition
 ***********************************************************************************************/
#define SIE_MAX_ENG_NUM                         3           ///< SIE1~3

#define SIE_MAX_OUTPUT_NUM                      3           ///< OUT0:IMG,     OUT1:CA/UV, OUT2:LA
#define SIE_MAX_INPUT_NUM                       2           ///< IN1 :DPC/IMG, IN2 :ECS

#define SIE_CA_MAX_WIN_NUM                      32
#define SIE_LA_MAX_WIN_NUM                      32
#define SIE_LA_HISTO_BIN_NUM                    64

#define SIE_PATGEN_SRC_WIN_W_ALIGN              2
#define SIE_PATGEN_SRC_WIN_H_ALIGN              2
#define SIE_PATGEN_SRC_WIN_MIN_W                16
#define SIE_PATGEN_SRC_WIN_MIN_H                16
#define SIE_PATGEN_SRC_WIN_MAX_W                16382
#define SIE_PATGEN_SRC_WIN_MAX_H                16382

#define SIE_ACT_ECS_WIN_MIN_W                   256         ///< for ECS table size 65x65
#define SIE_ACT_ECS_WIN_MIN_H                   256         ///< for ECS table size 65x65

#define SIE_ACT_WIN_W_ALIGN                     2
#define SIE_ACT_WIN_H_ALIGN                     2
#define SIE_ACT_WIN_X_ALIGN                     1
#define SIE_ACT_WIN_Y_ALIGN                     1
#define SIE_ACT_WIN_MAX_X                       16383
#define SIE_ACT_WIN_MAX_Y                       16383
#define SIE_ACT_WIN_MIN_W                       16
#define SIE_ACT_WIN_MIN_H                       2
#define SIE_ACT_WIN_MAX_W                       16382
#define SIE_ACT_WIN_MAX_H                       16382

#define SIE_CROP_WIN_W_ALIGN                    2
#define SIE_CROP_WIN_H_ALIGN                    2
#define SIE_CROP_WIN_X_ALIGN                    1
#define SIE_CROP_WIN_Y_ALIGN                    1
#define SIE_CROP_WIN_MAX_X                      16383
#define SIE_CROP_WIN_MAX_Y                      16383
#define SIE_CROP_WIN_MIN_W                      16
#define SIE_CROP_WIN_MIN_H                      2
#define SIE_CROP_WIN_MAX_W                      16382
#define SIE_CROP_WIN_MAX_H                      16382

#define SIE_CROP_BS_WIN_H_ALIGN                 2           ///< reserve for backward compatiable, will be remove
#define SIE_CROP_BS_IN_WIN_W_ALIGN              2           ///< when bayer-scaling horizontial on, crop width  should be 2x
#define SIE_CROP_BS_IN_WIN_H_ALIGN              2           ///< when bayer-scaling vertical    on, crop height should be 2x

#define SIE_CROP_WIN_MAX_W_8BIT                 SIE_CROP_WIN_MAX_W
#define SIE_CROP_WIN_MAX_W_10BIT                SIE_CROP_WIN_MAX_W
#define SIE_CROP_WIN_MAX_W_12BIT                SIE_CROP_WIN_MAX_W
#define SIE_CROP_WIN_MAX_W_16BIT                SIE_CROP_WIN_MAX_W

#define SIE_ACT_DATAEXT_SKIPLEN_MIN             1
#define SIE_ACT_DATAEXT_LEN_MIN                 1
#define SIE_ACT_DATAEXT_INILEN_MIN              0
#define SIE_ACT_DATAEXT_SKIPLEN_MAX             255
#define SIE_ACT_DATAEXT_LEN_MAX                 255
#define SIE_ACT_DATAEXT_INILEN_MAX              4095

#define SIE_SCALE_WIN_W_ALIGN                   2
#define SIE_SCALE_WIN_H_ALIGN                   2
#define SIE_SCALE_INPUT_WIN_MAX_W               4096
#define SIE_SCALE_INPUT_WIN_MAX_H               16382
#define SIE_SCALE_OUTPUT_WIN_MAX_W              2880
#define SIE_SCALE_OUTPUT_WIN_MAX_H              8190
#define SIE_SCALE_W_RATIO                       8
#define SIE_SCALE_H_RATIO                       8

#define SIE_CA_CROP_WIN_W_ALIGN                 2
#define SIE_CA_CROP_WIN_H_ALIGN                 2
#define SIE_CA_CROP_WIN_X_ALIGN                 1
#define SIE_CA_CROP_WIN_Y_ALIGN                 1
#define SIE_CA_CROP_WIN_MAX_X                   16383
#define SIE_CA_CROP_WIN_MAX_Y                   16383
#define SIE_CA_CROP_WIN_MAX_W                   10208       ///< (16383*(640/2-1) / 1024 + 1 )*2 = 10208
#define SIE_CA_CROP_WIN_MAX_H                   8160        ///< (16383*(512/2-1) / 1024 + 1 )*2 = 8160
#define SIE_CA_CROP_WIN_MIN_W                   64
#define SIE_CA_CROP_WIN_MIN_H                   64

#define SIE_CA_SMPL_MAX_W                       640         ///< max horizontal subsample output, line buffer 640x12bit
#define SIE_CA_SMPL_MAX_H                       512         ///< max vertical   subsample output

#define SIE_CA_WIN_W_ALIGN                      2
#define SIE_CA_WIN_H_ALIGN                      2
#define SIE_CA_WIN_MIN_W                        4
#define SIE_CA_WIN_MIN_H                        2
#define SIE_CA_WIN_MAX_W                        254
#define SIE_CA_WIN_MAX_H                        254

#define SIE_LA_CROP_WIN_W_ALIGN                 2
#define SIE_LA_CROP_WIN_H_ALIGN                 2
#define SIE_LA_CROP_WIN_X_ALIGN                 1
#define SIE_LA_CROP_WIN_Y_ALIGN                 1
#define SIE_LA_CROP_WIN_MAX_X                   320         ///< 640/2
#define SIE_LA_CROP_WIN_MAX_Y                   256         ///< 512/2
#define SIE_LA_CROP_WIN_MAX_W                   320         ///< scale down from 640, split to rgb, transfer to Y only
#define SIE_LA_CROP_WIN_MAX_H                   256         ///< scale down from 512, split to rgb, transfer to Y only
#define SIE_LA_CROP_WIN_MIN_W                   32
#define SIE_LA_CROP_WIN_MIN_H                   32

#define SIE_LA_WIN_W_ALIGN                      1
#define SIE_LA_WIN_H_ALIGN                      1
#define SIE_LA_WIN_MIN_W                        4
#define SIE_LA_WIN_MIN_H                        1
#define SIE_LA_WIN_MAX_W                        255
#define SIE_LA_WIN_MAX_H                        255

#define SIE_IN1_LINEOFFSET_ALIGN                4           ///< word align
#define SIE_OUT0_LINEOFFSET_ALIGN               4           ///< word align
#define SIE_OUT1_LINEOFFSET_ALIGN               4           ///< word align
#define SIE_OUT2_LINEOFFSET_ALIGN               4           ///< word align
#define SIE_OUT3_LINEOFFSET_ALIGN               4           ///< word align

#define SIE_IN1_LINEOFFSET_MAX_SIZE             65532       ///< Byte
#define SIE_OUT0_LINEOFFSET_MAX_SIZE            65532       ///< Byte
#define SIE_OUT1_LINEOFFSET_MAX_SIZE            65532       ///< Byte
#define SIE_OUT2_LINEOFFSET_MAX_SIZE            65532       ///< Byte

#define SIE_RING_BUFFER_MAX_LEN                 2047        ///< line count, ring buffer as line base

#define SIE_DVS_WIN_W_ALIGN                     8           ///< dvs 1 group is 8 pixel
#define SIE_DVS_WIN_H_ALIGN                     1
#define SIE_DVS_WIN_MIN_W                       8
#define SIE_DVS_WIN_MIN_H                       1
#define SIE_DVS_WIN_MAX_W                       16376       ///< 16383 align to 8
#define SIE_DVS_WIN_MAX_H                       8191

#define SIE_EVS_MOT_OUT_ALIGN                   4
#define SIE_EVS_MOT_OUT_MIN_SIZE                4
#define SIE_EVS_MOT_OUT_MAX_SIZE                0xfffffffc

/***********************************************************************************************
 * SIE1 Hardware Limitation Definition (2-pixel base)
 ***********************************************************************************************/
#define SIE1_MAX_OUTPUT_NUM                     SIE_MAX_OUTPUT_NUM
#define SIE1_MAX_INPUT_NUM                      SIE_MAX_INPUT_NUM

#define SIE1_LA_MAX_WIN_NUM                     SIE_LA_MAX_WIN_NUM

#define SIE1_CROP_WIN_MAX_W_8BIT                SIE_CROP_WIN_MAX_W_8BIT
#define SIE1_CROP_WIN_MAX_W_10BIT               SIE_CROP_WIN_MAX_W_10BIT
#define SIE1_CROP_WIN_MAX_W_12BIT               SIE_CROP_WIN_MAX_W_12BIT
#define SIE1_CROP_WIN_MAX_W_16BIT               SIE_CROP_WIN_MAX_W_16BIT

#define SIE1_OUT0_LINEOFFSET_ALIGN_HFLIP_8BIT   SIE_OUT0_LINEOFFSET_ALIGN
#define SIE1_OUT0_LINEOFFSET_ALIGN_HFLIP_10BIT  SIE_OUT0_LINEOFFSET_ALIGN
#define SIE1_OUT0_LINEOFFSET_ALIGN_HFLIP_12BIT  SIE_OUT0_LINEOFFSET_ALIGN
#define SIE1_OUT0_LINEOFFSET_ALIGN_HFLIP_16BIT  SIE_OUT0_LINEOFFSET_ALIGN

#define SIE1_BCC_HW_ABILITY                     1           ///< support BCC hardware function
#define SIE1_BCC_SHARE_SRC                      0           ///< SIE1

/***********************************************************************************************
 * SIE2 Hardware Limitation Definition (2-pixel base)
 ***********************************************************************************************/
#define SIE2_MAX_OUTPUT_NUM                     SIE_MAX_OUTPUT_NUM
#define SIE2_MAX_INPUT_NUM                      SIE_MAX_INPUT_NUM

#define SIE2_LA_MAX_WIN_NUM                     SIE_LA_MAX_WIN_NUM

#define SIE2_CROP_WIN_MAX_W_8BIT                SIE_CROP_WIN_MAX_W_8BIT
#define SIE2_CROP_WIN_MAX_W_10BIT               SIE_CROP_WIN_MAX_W_10BIT
#define SIE2_CROP_WIN_MAX_W_12BIT               SIE_CROP_WIN_MAX_W_12BIT
#define SIE2_CROP_WIN_MAX_W_16BIT               SIE_CROP_WIN_MAX_W_16BIT

#define SIE2_OUT0_LINEOFFSET_ALIGN_HFLIP_8BIT   SIE_OUT0_LINEOFFSET_ALIGN
#define SIE2_OUT0_LINEOFFSET_ALIGN_HFLIP_10BIT  SIE_OUT0_LINEOFFSET_ALIGN
#define SIE2_OUT0_LINEOFFSET_ALIGN_HFLIP_12BIT  SIE_OUT0_LINEOFFSET_ALIGN
#define SIE2_OUT0_LINEOFFSET_ALIGN_HFLIP_16BIT  SIE_OUT0_LINEOFFSET_ALIGN

#define SIE2_BCC_HW_ABILITY                     0           ///< not support BCC hardware function, share with SIE1
#define SIE2_BCC_SHARE_SRC                      0           ///< SIE1

/***********************************************************************************************
 * SIE3 Hardware Limitation Definition (2-pixel base)
 ***********************************************************************************************/
#define SIE3_MAX_OUTPUT_NUM                     SIE_MAX_OUTPUT_NUM
#define SIE3_MAX_INPUT_NUM                      SIE_MAX_INPUT_NUM

#define SIE3_LA_MAX_WIN_NUM                     8

#define SIE3_CROP_WIN_MAX_W_8BIT                SIE_CROP_WIN_MAX_W_8BIT
#define SIE3_CROP_WIN_MAX_W_10BIT               SIE_CROP_WIN_MAX_W_10BIT
#define SIE3_CROP_WIN_MAX_W_12BIT               SIE_CROP_WIN_MAX_W_12BIT
#define SIE3_CROP_WIN_MAX_W_16BIT               SIE_CROP_WIN_MAX_W_16BIT

#define SIE3_OUT0_LINEOFFSET_ALIGN_HFLIP_8BIT   SIE_OUT0_LINEOFFSET_ALIGN
#define SIE3_OUT0_LINEOFFSET_ALIGN_HFLIP_10BIT  SIE_OUT0_LINEOFFSET_ALIGN
#define SIE3_OUT0_LINEOFFSET_ALIGN_HFLIP_12BIT  SIE_OUT0_LINEOFFSET_ALIGN
#define SIE3_OUT0_LINEOFFSET_ALIGN_HFLIP_16BIT  SIE_OUT0_LINEOFFSET_ALIGN

#define SIE3_BCC_HW_ABILITY                     0           ///< not support BCC hardware function
#define SIE3_BCC_SHARE_SRC                      2           ///< SIE3

#endif  /* _SIE_LIMIT_H_ */
