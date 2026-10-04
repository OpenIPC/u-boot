/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "aiao_hal.h"
#include "mpp_board.h"
#include "aiao_reg.h"

td_void sys_hal_aio_clk_en(td_bool clk_en)
{
    const td_u32 tmp = (clk_en == TD_TRUE) ? 1 : 0;

    ot_reg_set_bit(tmp, 4, io_address(CRG_PERCTL10784_ADDR)); /* 4: bit */
    ot_reg_set_bit(tmp, 5, io_address(CRG_PERCTL10784_ADDR)); /* 5: bit */
}

td_void sys_hal_aio_reset_sel(td_bool reset)
{
    const td_u32 tmp = (reset == TD_TRUE) ? 1 : 0;

    ot_reg_set_bit(tmp, 0, io_address(CRG_PERCTL10784_ADDR));
}

td_void sys_hal_acodec_apb_clk_en(td_bool clk_en)
{
    return;
}

td_void sys_hal_acodec_reset_sel(td_bool reset)
{
    const td_u32 tmp = (reset == TD_TRUE) ? 1 : 0;

    ot_reg_set_bit(tmp, 0, io_address(CRG_PERCTL10912_ADDR));
    ot_reg_set_bit(tmp, 1, io_address(CRG_PERCTL10912_ADDR));
}

static td_void aiao_hal_clk_sel_reg_set(td_bool clk_sel)
{
    td_u32 tmp;

    tmp = (clk_sel == TD_TRUE) ? 1 : 0;
    ot_reg_set_bit(tmp, 12, io_address(CRG_PERCTL10784_ADDR)); /* 12: bit */
}

static td_void clk_select_base(aiao_clk_info *audio_clk_info)
{
    audio_clk_info->mclk_div_48k_256fs = AIO_MCLK_48K_256FS_786M;
    audio_clk_info->mclk_div_44p1k_256fs = AIO_MCLK_44P1K_256FS_786M;
    audio_clk_info->mclk_div_32k_256fs = AIO_MCLK_32K_256FS_786M;

    audio_clk_info->mclk_div_48k_320fs = AIO_MCLK_48K_320FS_786M;
    audio_clk_info->mclk_div_44p1k_320fs = AIO_MCLK_44P1K_320FS_786M;
    audio_clk_info->mclk_div_32k_320fs = AIO_MCLK_32K_320FS_786M;

    audio_clk_info->mclk_div_96k_256fs = AIO_MCLK_96K_256FS_786M;
    audio_clk_info->mclk_div_88p2k_256fs = AIO_MCLK_88P2K_256FS_786M;
    audio_clk_info->mclk_div_64k_256fs = AIO_MCLK_64K_256FS_786M;

    audio_clk_info->mclk_div_192k_256fs = AIO_MCLK_192K_256FS_786M;
    audio_clk_info->mclk_div_176p4k_256fs = AIO_MCLK_176P4K_256FS_786M;
    audio_clk_info->mclk_div_128k_256fs = AIO_MCLK_128K_256FS_786M;
}

static td_void clk_select_spare(aiao_clk_info *audio_clk_info)
{
    audio_clk_info->mclk_div_48k_256fs = AIO_MCLK_48K_256FS_1188M;
    audio_clk_info->mclk_div_44p1k_256fs = AIO_MCLK_44P1K_256FS_1188M;
    audio_clk_info->mclk_div_32k_256fs = AIO_MCLK_32K_256FS_1188M;

    audio_clk_info->mclk_div_48k_320fs = AIO_MCLK_48K_320FS_1188M;
    audio_clk_info->mclk_div_44p1k_320fs = AIO_MCLK_44P1K_320FS_1188M;
    audio_clk_info->mclk_div_32k_320fs = AIO_MCLK_32K_320FS_1188M;

    audio_clk_info->mclk_div_96k_256fs = AIO_MCLK_96K_256FS_1188M;
    audio_clk_info->mclk_div_88p2k_256fs = AIO_MCLK_88P2K_256FS_1188M;
    audio_clk_info->mclk_div_64k_256fs = AIO_MCLK_64K_256FS_1188M;

    audio_clk_info->mclk_div_192k_256fs = AIO_MCLK_192K_256FS_1188M;
    audio_clk_info->mclk_div_176p4k_256fs = AIO_MCLK_176P4K_256FS_1188M;
    audio_clk_info->mclk_div_128k_256fs = AIO_MCLK_128K_256FS_1188M;
}

td_s32 aiao_hal_set_clk_sel_private(audio_clk_select clk_sel, aiao_clk_info *clk_info)
{
    switch (clk_sel) {
        case AUDIO_CLK_SELECT_BASE:
            clk_select_base(clk_info);
            break;
        case AUDIO_CLK_SELECT_SPARE:
            clk_select_spare(clk_info);
            break;
        default:
            return TD_FAILURE;
    }

    aiao_hal_clk_sel_reg_set(clk_sel);

    return 0;
}

td_void aop_hal_buffer_saddr_reg_h(td_ulong aio_base, td_u32 chn_id, td_u32 value)
{
    td_u32 temp;

    temp = 0;
    *(volatile td_u32 *)(aio_base + (td_u32)(aop_buff_saddr_reg_h(chn_id))) = temp;
}

td_s32 aop_get_bclk_id_private(td_void)
{
    return AIO_CRG_ID_8;
}

td_u8 aop_get_tx_sd_private(td_void)
{
    return TX_SD_ID_0;
}

td_void aop_set_mask_private(td_u32 chn_id, td_ulong aio_base)
{
    u_aiao_mux_sel i2s_mux;

    i2s_mux.ul32 = *(volatile td_u32 *)(aio_base + (td_u32)(AUDIO_AIO_I2S_MUX_REG));

    if (chn_id == AIO_CHN_ID_0) {
            i2s_mux.bits.audio_rx_bclk_sel = 0x1;
            i2s_mux.bits.rx_sd_sel = 0x1;
            i2s_mux.bits.codec_tx_sel = 0x0;
            i2s_mux.bits.audio_mclk_sel = 0x1;
            i2s_mux.bits.crg9_mode_sel = 0x0;

            i2s_mux.bits.i2s1_tx_sd_src_sel = 0x0;
            i2s_mux.bits.i2s1_tx_sd0_sel = 0x0;
            i2s_mux.bits.i2s1_tx_sd1_sel = 0x1;
            i2s_mux.bits.i2s1_tx_sd2_sel = 0x2;
            i2s_mux.bits.i2s1_tx_sd3_sel = 0x3;
    }

    *(volatile td_u32 *)(aio_base + (td_u32)(AUDIO_AIO_I2S_MUX_REG)) = i2s_mux.ul32;
}
