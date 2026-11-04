#ifndef __KDRV_VIDEOYDCC_H__
#define __KDRV_VIDEOYDCC_H__

/* Limit
 */
#define YDCC12_MAX_WIDTH	2880
#define YDCC12_MAX_HEIGHT	2880

#define YDCC8_MAX_WIDTH		640
#define YDCC8_MAX_HEIGHT	640

#define YDCC_ALIGN8(x)			((((x) + 7) >> 3) << 3)
#define YDCC8_DEC_W(x)			YDCC_ALIGN8(x)
#define YDCC8_DEC_H(x)			YDCC_ALIGN8(x)
#define YDCC8_DEC_RGGB_LOS(x)	YDCC_ALIGN8(x)			/* 8x */
#define YDCC8_DEC_Y_WIDTH(x)	YDCC_ALIGN8((x)/2)
#define YDCC8_DEC_Y_HEIGHT(x)	(YDCC_ALIGN8(x)/2)
#define YDCC8_DEC_Y_LOS(x)		YDCC_ALIGN8(YDCC8_DEC_Y_WIDTH(x))				/* frm width/2 and align8 */

#define YDCC8_ADDR_SIZE_MASK(x)	((x) & 0x3F)

#define YDCC12_DEC_W(x)			YDCC_ALIGN8(x)
#define YDCC12_DEC_H(x)			YDCC_ALIGN8(x)
#define YDCC12_DEC_RGGB_LOS(x)	YDCC_ALIGN8(((YDCC12_DEC_W(x) * 3) >> 1))		/* 12bits, final align8 */
#define YDCC12_DEC_Y_WIDTH(x)	YDCC_ALIGN8((x)/2)
#define YDCC12_DEC_Y_HEIGHT(x)	(YDCC_ALIGN8(x)/2)
#define YDCC12_DEC_Y_LOS(x)		YDCC_ALIGN8(YDCC12_DEC_Y_WIDTH(x))
#define YDCC12_ADDR_SIZE_MASK(x)	((x) & 0x7F)

/* no align need */
#define YDCC12_ENC_W(x)			(x)
#define YDCC12_ENC_H(x)			(x)

typedef struct {
	unsigned int 		addr_pa;
	unsigned int 		addr_va;
	unsigned int	size;
} YDCC_ADDR_INFO;

typedef enum {
	VDOYDCC_SET_INIT = 0,		///< set init info, YDCC_INIT_PARAM. Must be set first.
	VDOYDCC_SET_ENC_ADDR,		///< YDCC_ENC_ADDR
	VDOYDCC_SET_DEC_ADDR,		///< YDCC_DEC_ADDR
	VDOYDCC_SET_ENC_PARAM,		///< set encode parameters, YDCC_ENCODE_PARAM
	VDOYDCC_SET_DEC_PARAM,		///< set decode parameters, YDCC_DECODE_PARAM
	VDOYDCC_SET_DEC_LOS,		///< set decode line offset. YDCC_DEC_LOS. It is optional.
	VDOYDCC_SET_DITH_SEED,		///< for emulation purpose. YDCC_DEC_DITH_SEED
	VDOYDCC_SET_SRAM_BUF_INFO,	///< for sram start address and its size. SRAM_BUF_INFO. It is optional.
	VDOYDCC_SET_QTBL,			///< for q-table. YDCC_QTBL. used by enc12.
	VDOYDCC_SET_GAMMA_TBL,		///< GAMMA_TBL. rggb2mcu(). m0 = (((gamma_tbl[tmp+1] - gamma_tbl[tmp]) * (m0 % 128)) >> 7) + gamma_tbl[tmp];
	VDOYDCC_SET_SIE_FIRE_CB_FUNC,	///< YDCC_CALLBACK_FUNC. used for emulation code that tigger encode followed by sie trigger
	VDOYDCC_SET_TIMEOUT_VAL,	///< set timeout value for all encoder/decoder. YDCC_TIMEOUT_VAL
	YDOYDCC_SET_AUTOGATING,		///< set auto gating. 1 indicate ON, 0:OFF
	YDOYDCC_SET_QP_VALUE,		///< set qp value, from 1 ~ 100, ENC12_QP_VAL
	VDOYDCC_GET_WH,				///< get engine type, YDCC_CODEC_ENG
	VDOYDCC_GET_DEC_PARAM,		///< get decode param, YDCC_DECODE_PARAM
	VDOYDCC_GET_DBG_INFO,		///< get checksum, YDCC_DBG_INFO
	VDOYDCC_GET_DEC_LOS,		///< get current line-offset, YDCC_DEC_LOS
	VDOYDCC_GET_ENC12_BS_SZ,	///< get encode bs size
} KDRV_VDOYDCC_PARAM_ID;

typedef enum {
	YDCC12_FMT_RGGB = 0,
	YDCC12_FMT_GRGB,
	YDCC12_FMT_GBRG,
	YDCC12_FMT_BGGR,
} YDCC12_FMT;

typedef enum {
	YDCC_DEC_RGGB 	= 0x1,
	YDCC_DEC_Y_ONLY = 0x2,
	YDCC_DEC_BOTH = (YDCC_DEC_RGGB | YDCC_DEC_Y_ONLY),
} YDCC_DEC_MODE;

typedef struct {
	unsigned int 	sram_buf_addr;
	unsigned int	sram_size;
} SRAM_BUF_INFO;

typedef struct {
	/* source is from SIE */
	YDCC_ADDR_INFO bs_out_addr;			///< Output address of bitstream out (YDCC12 Encode). It is SRAM address. 64 align of enc8/dec8. 128 align for enc12/dec12.
} YDCC_ENC_ADDR;

typedef struct {
	unsigned char bypass_mode;					///< 0: off, 1: enable YDCC12 bypass mode.
	unsigned char gamma_correction;				///< 0: disable gamma correction. 1: enable gamma correction
	YDCC12_FMT	input_fmt;				///< Format of YDCC12 Encode input data
} YDCC_ENCODE_PARAM;

typedef struct {
	unsigned int	dec_lofs_rggb;				///< rggb buffer line offset
	unsigned int	dec_lofs_y;					///< y buffer line offset
} YDCC_DEC_LOS;

typedef struct {
	unsigned short	width;		///< 0xC, max:2880
	unsigned short	height;
} YDCC_INIT_PARAM;

typedef struct {
	unsigned char	qp_val;	/* 1 ~ 100 */
} ENC12_QP_VAL;

typedef struct {
	unsigned int dithering_seed;
} YDCC_DEC_DITH_SEED;

typedef struct {
	unsigned int timeout_value;	/* mclk unit */
} YDCC_TIMEOUT_VAL;

/* change MSB-5 bits,thus totally 32 entries */
typedef struct {
	unsigned int  gm_tbl[16];	/* 11:0 GM_TBL[2N], 23:12 GM_TBL[2N+1], N: 0 ~ 15 */
	unsigned int  gm_tbl_16;	/* 11:0 GM_TBL[32] */
} GAMMA_TBL;

/* don't change order. */
typedef enum {
	YDCC_ENG_ENC12 = 0,		/* ydcc encode 12bits */
	YDCC_ENG_MAX,
} YDCC_ENG_TYPE;

/* VDOYDCC_SET_SIE_FIRE_CB_FUNC */
typedef struct {
	int (*callback)(void *data);
	unsigned int usr_data;
} YDCC_CALLBACK_FUNC;

/*!
 * @fn int kdrv_videoydcc_open(unsigned int chip, unsigned int engine)
 * @brief open hardware engine
 * @param chip		the chip id of hardware
 * @param engine	the engine id YDCC_ENG_TYPE of hardware
 * @return return 0 on success, -1 on error
 */
int kdrv_videoydcc_open(unsigned int chip, unsigned int engine);

/*!
 * @fn int kdrv_videoydcc_close(unsigned int chip, unsigned int engine)
 * @brief close hardware engine
 * @param chip		the chip id of hardware
 * @param engine	the engine id of hardware
 * @return return 0 on success, -1 on error
 */
int kdrv_videoydcc_close(unsigned int handle);

/*!
 * @fn int kdrv_videoydcc_get(unsigned int handle, DEC_ID id, void *param)
 * @brief set parameters to hardware engine
 * @param handler	the handler of hardware
 * @param id 		the id of parameters
 * @param param 	the parameters
 * @return return 0 on success, -1 on error
 */
int kdrv_videoydcc_get(unsigned int handle, KDRV_VDOYDCC_PARAM_ID parm_id, void *param);

/*!
 * @fn int kdrv_videoydcc_set(KDRV_VDODEC_ID handler, DEC_ID id, void *param)
 * @brief get parameters to hardware engine
 * @param handler	the handler of hardware
 * @param id 		the id of parameters
 * @param param 	the parameters
 * @return return 0 on success, -1 on error
 */
int kdrv_videoydcc_set(unsigned int handle, KDRV_VDOYDCC_PARAM_ID parm_id, void *param);

typedef enum {
	YDCC_STS_OK = 0,
	YDCC_STS_FAIL = -1,
	YDCC_BS_OVERFLOW = -2,
	YDCC_BS_VLC_OVERFLOW = -3,
	YDCC_BS_OVERSIZE = -4,
	YDCC_STS_TIMEOUT = -5,
	YDCC_STS_HW_BUSY = -6,
	YDCC_STS_NOT_FRAME_DONE = -7,
	YDCC_STS_SIE_TRIG_FAIL = -8,
	YDCC_OTHERS = -100,
} YDCC_STS;

/*!
 * @fn YDCC_STS kdrv_videoydcc_trigger(KDRV_VDODEC_ID handler)
 * @brief trigger hardware engine
 * @param handler 				the handler of hardware
 * @param p_enc_info		 	the decode one frame settings
 * @param p_cb_func 			the callback function
 * @param user_data 			the private user data
 * @return return 0 on success, -1 on error
 */
YDCC_STS kdrv_videoydcc_trigger(unsigned int handle);

#endif /* __KDRV_VIDEOYDCC_H__ */