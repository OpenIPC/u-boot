/*
    Public header file for SIE module.

    Copyright Novatek Microelectronics Corp. 2023.  All rights reserved.
*/
#ifndef _SIE_ENG_H_
#define _SIE_ENG_H_

#include "sie_base.h"

/*************************************************************************************
 *  SIE Driver Version Definition
 *************************************************************************************/
#define SIE_DRV_VERSION                     "0.01.01"

/*************************************************************************************
 *  SIE Engine Index Definition
 *************************************************************************************/
typedef enum {
    SIE_ENG_ID_1  = 0,                      ///< SIE1
    SIE_ENG_ID_2,                           ///< SIE2
    SIE_ENG_ID_3,                           ///< SIE3
    SIE_ENG_ID_MAX
} SIE_ENG_ID_T;

/*************************************************************************************
 *  SIE Engine Clock Source Definition
 *************************************************************************************/
typedef enum {
    SIE_ENG_CLKSRC_480 = 0,
    SIE_ENG_CLKSRC_RSVD1,
    SIE_ENG_CLKSRC_PLL5,
    SIE_ENG_CLKSRC_RSVD3,
    SIE_ENG_CLKSRC_PLL12,
    SIE_ENG_CLKSRC_320,
    SIE_ENG_CLKSRC_192,
    SIE_ENG_CLKSRC_RSVD7,
    SIE_ENG_CLKSRC_MAX
} SIE_ENG_CLKSRC;

/*************************************************************************************
 *  SIE Internal Clock Source Definition
 *************************************************************************************/
typedef enum {
    SIE_ENG_INTCLKSRC_SIE1 = 0,             ///< engine clock from SIE1_CLK, available for SIE2
    SIE_ENG_INTCLKSRC_SIE2,
    SIE_ENG_INTCLKSRC_SIE3,
    SIE_ENG_INTCLKSRC_MAX
} SIE_ENG_INTCLKSRC;

/*************************************************************************************
 *  SIE Interrupt Definition
 *************************************************************************************/
#define SIE_INT_VD                          0x00000001
#define SIE_INT_BP1                         0x00000002
#define SIE_INT_BP2                         0x00000004
#define SIE_INT_BP3                         0x00000008
#define SIE_INT_ACTST                       0x00000010
#define SIE_INT_CRPST                       0x00000020
#define SIE_INT_DRAM_IN_OUT_ERR             0x00000040
#define SIE_INT_DRAM_OUT0_END               0x00000080
#define SIE_INT_DRAM_OUT1_END               0x00000100
#define SIE_INT_DRAM_OUT2_END               0x00000200
#define SIE_INT_DPCF                        0x00002000
#define SIE_INT_SIECLK_ERR                  0x00004000
#define SIE_INT_RAWENC_OVFL                 0x00008000
#define SIE_INT_ACTEND                      0x00010000
#define SIE_INT_CROPEND                     0x00020000
#define SIE_INT_BEHAVIOR_ERR                0x00040000
#define SIE_INT_HISTO_END                   0x00100000
#define SIE_INT_RAWENC_DATA_OVLAP           0x00800000     ///< BCC share and process data overlap
#define SIE_INT_CCIR656_HEADER_HIT          0x01000000
#define SIE_INT_ALL                         0x0197e3ff

/*************************************************************************************
 *  SIE Common Definition
 *************************************************************************************/
typedef struct {
    SIE_ENG_CLKSRC      clk_src;            ///< engine operation clock source
    UINT32              clk_rate;           ///< engine operation clock rate(Hz)
    SIE_ENG_INTCLKSRC   int_clk_src;        ///< engine internal  clock source for bcc share used
} SIE_INIT_CFG;

typedef struct {
    BOOL    act_en;
} SIE_START_CFG;

/*************************************************************************************
 *  SIE Public Function Prototype
 *************************************************************************************/
void  *sie_eng_get_handle(UINT32 id);
int    sie_eng_open(UINT32 id, SIE_INIT_CFG *init_cfg);
int    sie_eng_close(UINT32 id);
int    sie_eng_trig_hw_reg(void *handle, SIE_START_CFG *trig_cfg);
int    sie_eng_stop_hw_reg(void *handle);
int    sie_eng_set_clock(void *handle, SIE_ENG_CLKSRC clk_src, UINT32 clk_rate);
int    sie_eng_set_intclock(void *handle, SIE_ENG_INTCLKSRC clk_src);

int    sie_eng_write_hw_reg(void *handle, UINT32 reg_ofs, UINT32 val);
UINT32 sie_eng_read_hw_reg(void *handle, UINT32 reg_ofs);

int    sie_eng_patgen_simple_test(UINT32 id, UINT32 out0_enb, UINT32 ydcc_direct);
int    sie_eng_dramin_simple_test(UINT32 id, UINT32 out0_enb, UINT32 ydcc_direct);

#endif  /* _SIE_ENG_H_ */

