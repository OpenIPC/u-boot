/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include <common.h>
#include "drv_vo.h"
#include "hal_vo_video.h"
#include "hal_vo_dev_comm.h"
#include "hal_vo_dev.h"
#include "ot_math.h"
#include "securec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#if vo_desc("UBOOT_VO")

#define VO_VGA_GAIN_DEF                 0xB
#define VO_CVBS_DAC_GC_DEF_PAL          0x3B
#define VO_CVBS_DAC_GC_DEF_NTSC         0x3C

static td_u32 g_bg_color[OT_VO_MAX_PHYS_DEV_NUM] = {0x00ffff}; /* default value is CYAN */

td_u32 vo_dev_get_bg_color(ot_vo_dev dev)
{
    return g_bg_color[dev];
}

td_void vo_dev_set_bg_color(ot_vo_dev dev, td_u32 bg_color)
{
    g_bg_color[dev] = bg_color;
}

#if vo_desc("dev user sync timing")
/* dhd0: 1080P60 */
static const ot_vo_sync_info g_vo_user_sync_timing[OT_VO_MAX_PHYS_DEV_NUM] = {
    /*
     * |--INTFACE---||-----TOP-----||----HORIZON--------||----BOTTOM-----||-PULSE-||-INVERSE-|
     * syncm, iop, itf,   vact, vbb,  vfb,  hact,  hbb,  hfb, hmid,bvact,bvbb, bvfb, hpw, vpw,idv, ihs, ivs
     */
    { 0, 1, 1, 1080, 41, 4, 1920, 192, 88,  1,    1,   1,  1,  44, 5, 0, 0, 0 }, /* 1080P@60_hz */
};

const ot_vo_sync_info *vo_drv_get_dev_user_sync_timing(ot_vo_dev dev)
{
    if ((dev < 0) || (dev >= OT_VO_MAX_PHYS_DEV_NUM)) {
        return TD_NULL;
    }
    return &(g_vo_user_sync_timing[dev]);
}
#endif

#if vo_desc("dev user sync info")
/* dhd0: 1080P60 */
static const ot_vo_user_sync_info g_vo_user_sync_info[OT_VO_MAX_PHYS_DEV_NUM] = {
    {
        .manual_user_sync_info = {
            .user_sync_attr = {
                .clk_src = OT_VO_CLK_SRC_PLL,
                .vo_pll = { /* if hdmi, set it by pixel clk and div mode */
                    .fb_div = 99, /* 99 fb div */
                    .frac = 0,
                    .ref_div = 1, /* 1 ref div */
                    .post_div1 = 4, /* 4 post div1 */
                    .post_div2 = 4, /* 4 post div2 */
                },
            },
            .pre_div = 1, /* if hdmi, set it by pixel clk */
            .dev_div = 1, /* if rgb, set it by serial mode */
        },
        .op_mode = OT_OP_MODE_MANUAL,
        .clk_reverse_en = TD_TRUE,
    },
};

const ot_vo_user_sync_info *vo_drv_get_dev_user_sync_info(ot_vo_dev dev)
{
    if ((dev < 0) || (dev >= OT_VO_MAX_PHYS_DEV_NUM)) {
        return TD_NULL;
    }
    return &g_vo_user_sync_info[dev];
}
#endif

#if vo_desc("dev")

td_u32 vo_drv_get_fixed_clk_sel(ot_vo_fixed_clk fixed_clk)
{
    static const td_u32 fixed_clk_sel[OT_VO_FIXED_CLK_BUTT] = {
        0x6, 0x5, 0x4, 0x3, 0x2, 0x1, 0x0
    };

    if (fixed_clk >= OT_VO_FIXED_CLK_BUTT) {
        vo_err_trace("fixed_clk(%d) invalid!\n", fixed_clk);
        return 0;
    }
    return fixed_clk_sel[fixed_clk];
}

td_void vo_drv_set_dev_user_intf_sync_attr(ot_vo_dev dev, const ot_vo_manual_user_sync_info *sync_info)
{
    td_u32 hd_clk_sel;
    td_u32 clk_en;
    td_u32 lcd_mclk_div;
    ot_vo_pll vo_pll;
    ot_mpp_chn mpp_chn;

    vo_drv_set_dev_mpp_chn(dev, &mpp_chn);
    if (dev == VO_DEV_DHD0) {
            /* clk source */
            if (sync_info->user_sync_attr.clk_src == OT_VO_CLK_SRC_PLL) {
            hd_clk_sel = 8; /* 8:bpll */

            (td_void)memcpy_s(&vo_pll, sizeof(ot_vo_pll), &sync_info->user_sync_attr.vo_pll, sizeof(ot_vo_pll));
            call_sys_drv_ioctrl(&mpp_chn, SYS_VO_HD_CLK_SEL, &hd_clk_sel);
            /* clk value */
            call_sys_drv_ioctrl(&mpp_chn, SYS_VO_PLL_FBDIV_SET, &vo_pll.fb_div);
            call_sys_drv_ioctrl(&mpp_chn, SYS_VO_PLL_FRAC_SET, &vo_pll.frac);
            call_sys_drv_ioctrl(&mpp_chn, SYS_VO_PLL_REFDIV_SET, &vo_pll.ref_div);
            call_sys_drv_ioctrl(&mpp_chn, SYS_VO_PLL_POSTDIV1_SET, &vo_pll.post_div1);
            call_sys_drv_ioctrl(&mpp_chn, SYS_VO_PLL_POSTDIV2_SET, &vo_pll.post_div2);
        } else if (sync_info->user_sync_attr.clk_src == OT_VO_CLK_SRC_LCDMCLK) {
            hd_clk_sel = 7; /* 7: lcdmclk */
            call_sys_drv_ioctrl(&mpp_chn, SYS_VO_HD_CLK_SEL, &hd_clk_sel);
            lcd_mclk_div = sync_info->user_sync_attr.lcd_m_clk_div;
            call_sys_drv_ioctrl(&mpp_chn, SYS_VO_LCD_MCLK_DIV, &lcd_mclk_div);
            clk_en = TD_TRUE;
            call_sys_drv_ioctrl(&mpp_chn, SYS_VO_LCD_CLK_EN, &clk_en);
        } else {
            hd_clk_sel = vo_drv_get_fixed_clk_sel(sync_info->user_sync_attr.fixed_clk);
            call_sys_drv_ioctrl(&mpp_chn, SYS_VO_HD_CLK_SEL, &hd_clk_sel);
        }
    }
}

td_u32 vo_drv_get_dev_div(ot_vo_intf_type intf_type, td_bool rgb_dummy_out_en)
{
    if (intf_type & OT_VO_INTF_BT656) {
        return VO_INTF_BT656_DIV_MODE;
    }

    if ((intf_type & OT_VO_INTF_RGB_6BIT) || (intf_type & OT_VO_INTF_RGB_8BIT)) {
        if (rgb_dummy_out_en == TD_FALSE) {
            return VO_INTF_RGB_DIV_MODE_3;
        } else {
            return VO_INTF_RGB_DIV_MODE_4;
        }
    }
    return 1;
}
td_void vo_drv_set_dev_div(ot_vo_dev dev, td_u32 dev_div)
{
    ot_mpp_chn mpp_chn = {0};
    td_u32 div_mode = dev_div - 1;
    vo_drv_set_dev_mpp_chn(dev, &mpp_chn);
    call_sys_drv_ioctrl(&mpp_chn, SYS_VO_DEV_DIV_MOD, &div_mode);
}

td_void vo_drv_set_hdmi_div(ot_vo_dev dev, td_u32 pre_div)
{
    ot_unused(dev);
    ot_unused(pre_div);
}

static ot_vo_intf_type vou_drv_get_dev_supported_intf(ot_vo_dev dev)
{
    ot_vo_intf_type dev_both_support_intf =
        OT_VO_INTF_BT1120 |
        OT_VO_INTF_BT656 |
        OT_VO_INTF_MIPI |
        OT_VO_INTF_RGB_6BIT |
        OT_VO_INTF_RGB_8BIT |
        OT_VO_INTF_RGB_16BIT |
        OT_VO_INTF_RGB_18BIT |
        OT_VO_INTF_RGB_24BIT |
        OT_VO_INTF_MIPI_SLAVE;
    return dev_both_support_intf;
}

static ot_vo_intf_type g_dhd0_homologous_intf[] = {
    OT_VO_INTF_MIPI,
    OT_VO_INTF_BT1120,
    OT_VO_INTF_BT656,
    OT_VO_INTF_RGB_6BIT,
    OT_VO_INTF_RGB_8BIT,
    OT_VO_INTF_RGB_16BIT,
    OT_VO_INTF_RGB_18BIT,
    OT_VO_INTF_RGB_24BIT,
    OT_VO_INTF_MIPI_SLAVE,
};


static td_void vou_drv_get_dev_homologous_intf(ot_vo_dev dev,
    ot_vo_intf_type **intf_type,
    td_u32 *intf_num)
{
    ot_unused(dev);
        *intf_type = g_dhd0_homologous_intf;
        *intf_num = sizeof(g_dhd0_homologous_intf) / sizeof(ot_vo_intf_type);
        return;
}

static td_s32 vou_drv_check_dhd_homologous_intf(ot_vo_dev dev, ot_vo_intf_type intf_type)
{
    td_u32 homologous_intf_num = 0;
    ot_vo_intf_type *homologous_intf = TD_NULL;
    td_u32 loop;
    td_bool is_intf_found = TD_TRUE;
    vou_drv_get_dev_homologous_intf(dev, &homologous_intf, &homologous_intf_num);

    for (loop = 0; loop < homologous_intf_num; loop++) {
        is_intf_found = is_intf_found && ((intf_type) & (~(homologous_intf[loop])));
        if (is_intf_found == TD_FALSE) {
            return TD_SUCCESS;
        }
    }

    vo_err_trace("device HD%d, does not support homologous intf(%d)!\n", dev, intf_type);
    return OT_ERR_VO_NOT_SUPPORT;
}

static td_s32 vou_drv_check_dhd_intf(ot_vo_dev dev, ot_vo_intf_type intf_type)
{
    ot_vo_intf_type dev_support_intf = vou_drv_get_dev_supported_intf(dev);
    if ((intf_type & ~(dev_support_intf)) || (intf_type == 0)) {
        vo_err_trace("device HD%d does not support intf(%d)!\n", dev, intf_type);
        return OT_ERR_VO_NOT_SUPPORT;
    }

    return vou_drv_check_dhd_homologous_intf(dev, intf_type);
}

td_s32 vo_drv_check_other_dev_exclution_intf(ot_vo_dev other_dev, ot_vo_intf_type intf_type)
{
    td_u32 digital_intf;
    vo_drv_dev *drv_dev_ctx = vo_drv_get_dev_ctx(other_dev);

    digital_intf = OT_VO_INTF_MIPI | OT_VO_INTF_MIPI_SLAVE | OT_VO_INTF_BT1120 | OT_VO_INTF_BT656 |
        OT_VO_INTF_RGB_6BIT | OT_VO_INTF_RGB_8BIT | OT_VO_INTF_RGB_16BIT | OT_VO_INTF_RGB_18BIT |
        OT_VO_INTF_RGB_24BIT;

    if ((drv_dev_ctx->vou_attr.intf_type & digital_intf) && (intf_type & digital_intf)) {
        vo_err_trace("intf(%d, %d) can only use one at the same time!\n", drv_dev_ctx->vou_attr.intf_type,
            intf_type);
        return OT_ERR_VO_NOT_SUPPORT;
    }
    return TD_SUCCESS;
}

td_s32 vou_drv_check_dev_intf(ot_vo_dev dev, ot_vo_intf_type intf_type)
{
    td_s32 ret = OT_ERR_VO_INVALID_DEV_ID;

    if (dev == VO_DEV_DHD0) {
        ret = vou_drv_check_dhd_intf(dev, intf_type);
    }

    if (ret != TD_SUCCESS) {
        return ret;
    }

    return vo_drv_check_intf_share(dev, intf_type);
}

ot_vo_intf_type vo_drv_rgb_get_csc_intf(td_void)
{
    return OT_VO_INTF_MIPI;
}
td_s32 vou_drv_check_hdmi_sync(ot_vo_dev dev, ot_vo_intf_type intf_type, ot_vo_intf_sync intf_sync)
{
    ot_unused(dev);
    ot_unused(intf_type);
    ot_unused(intf_sync);

    return OT_ERR_VO_NOT_SUPPORT;
}

static td_s32 vou_drv_check_bt1120_sync(ot_vo_dev dev, ot_vo_intf_sync intf_sync)
{
    if (dev == VO_DEV_DHD0) {
        if ((intf_sync < OT_VO_OUT_640x480_60) || (intf_sync > OT_VO_OUT_1080P60)) {
            vo_err_trace("for BT.1120 interface, vo%d's intfsync %d illegal!\n", dev, intf_sync);
            return OT_ERR_VO_NOT_SUPPORT;
        }
    } else {
        if ((intf_sync < OT_VO_OUT_640x480_60) ||
            (intf_sync > OT_VO_OUT_1080P60) ||
            (intf_sync == OT_VO_OUT_1280x800_60) ||
            (intf_sync == OT_VO_OUT_1366x768_60) ||
            (intf_sync == OT_VO_OUT_1400x1050_60) ||
            (intf_sync == OT_VO_OUT_1440x900_60) ||
            (intf_sync == OT_VO_OUT_1680x1050_60)) {
            vo_err_trace("for BT.1120 interface, vo%d's intfsync %d illegal!\n", dev, intf_sync);
            return OT_ERR_VO_NOT_SUPPORT;
        }
    }

    return TD_SUCCESS;
}

td_s32 vou_drv_check_bt_sync(ot_vo_dev dev, ot_vo_intf_type intf_type, ot_vo_intf_sync intf_sync)
{
    td_s32 ret;

    if (intf_type & OT_VO_INTF_BT1120) {
        ret = vou_drv_check_bt1120_sync(dev, intf_sync);
        if (ret != TD_SUCCESS) {
            return ret;
        }
    }

    if (intf_type & OT_VO_INTF_BT656) {
        if ((intf_sync != OT_VO_OUT_PAL) && (intf_sync != OT_VO_OUT_NTSC)) {
            vo_err_trace("for BT.656 interface, vo%d's intfsync %d illegal!\n", dev, intf_sync);
            return OT_ERR_VO_NOT_SUPPORT;
        }
    }

    return TD_SUCCESS;
}

td_s32 vou_drv_check_rgb_sync(ot_vo_dev dev, ot_vo_intf_type intf_type, ot_vo_intf_sync intf_sync)
{
    if (vo_drv_is_rgb_intf(intf_type)) {
        if (intf_sync != OT_VO_OUT_USER) {
            vo_err_trace("for RGB interface(%d), vo%d's intfsync %d illegal!\n", intf_type, dev, intf_sync);
            return OT_ERR_VO_NOT_SUPPORT;
        }
    }

    return TD_SUCCESS;
}

td_s32 vou_drv_check_mipi_sync(ot_vo_dev dev, ot_vo_intf_type intf_type, ot_vo_intf_sync intf_sync)
{
    td_bool is_intf_sync_illegal = TD_FALSE;
    td_bool all_dev_intf_sync_illegal =
        (intf_sync != OT_VO_OUT_576P50) &&
        (intf_sync != OT_VO_OUT_1024x768_60) &&
        (intf_sync != OT_VO_OUT_720P50) &&
        (intf_sync != OT_VO_OUT_720P60) &&
        (intf_sync != OT_VO_OUT_1280x1024_60) &&
        (intf_sync != OT_VO_OUT_1080P24) &&
        (intf_sync != OT_VO_OUT_1080P25) &&
        (intf_sync != OT_VO_OUT_1080P30) &&
        (intf_sync != OT_VO_OUT_1080P50) &&
        (intf_sync != OT_VO_OUT_1080P60) &&
        (intf_sync != OT_VO_OUT_720x1280_60) &&
        (intf_sync != OT_VO_OUT_1080x1920_60);

    if (!((intf_type & OT_VO_INTF_MIPI) || (intf_type & OT_VO_INTF_MIPI_SLAVE))) {
        return TD_SUCCESS;
    }

    if (dev == VO_DEV_DHD0) {
        is_intf_sync_illegal = all_dev_intf_sync_illegal &&
            (intf_sync != OT_VO_OUT_3840x2160_24) &&
            (intf_sync != OT_VO_OUT_3840x2160_25) &&
            (intf_sync != OT_VO_OUT_3840x2160_30);
    }

    if (is_intf_sync_illegal == TD_TRUE) {
        vo_err_trace("for MIPI interface, vo%d's intfsync %d illegal!\n", dev, intf_sync);
        return OT_ERR_VO_NOT_SUPPORT;
    }

    return TD_SUCCESS;
}

#endif

#if vo_desc("dev")

static td_void vo_drv_dev_cfg_init(ot_vo_dev dev)
{
    vo_drv_dev *drv_dev_ctx = vo_drv_get_dev_ctx(dev);
    vo_dev_capability *dev_cap = &drv_dev_ctx->dev_cap;
    if (dev == VO_DEV_DHD0) {
        dev_cap->dev_type = VO_UHD_HW_DEV;
    }
}

static td_void vo_drv_dev_layer_info_init(ot_vo_dev dev)
{
    vo_drv_dev *drv_dev_ctx = vo_drv_get_dev_ctx(dev);

    if (dev == VO_DEV_DHD0) {
        /* 1 video layer and 1 graphic layer in DHD0 */
        drv_dev_ctx->layer_num = 1;         /* 1: 1 video layer V0 */
    }
}

td_void vo_drv_dev_info_init(td_void)
{
    ot_vo_dev dev;
    td_u32 ctx_len;
    vo_drv_dev *drv_dev_ctx = vo_drv_get_dev_ctx(0);

    ctx_len = sizeof(vo_drv_dev) * OT_VO_MAX_DEV_NUM;
    (td_void)memset_s(drv_dev_ctx, ctx_len, 0, ctx_len);

    for (dev = 0; dev < OT_VO_MAX_DEV_NUM; dev++) {
        vo_drv_dev_cfg_init(dev);
        vo_drv_dev_layer_info_init(dev);
    }
}

#endif

#if vo_desc("dev")

ot_vo_layer vo_drv_get_layer(ot_vo_dev dev)
{
    hal_disp_layer hal_layer = HAL_DISP_LAYER_VHD0;

    if (dev == VO_DEV_DHD0) {
        hal_layer = HAL_DISP_LAYER_VHD0;
    }

    return hal_layer;
}

td_void vo_drv_set_dev_clk_sel(ot_vo_dev dev, td_u32 clk_sel)
{
    ot_mpp_chn mpp_chn;
    td_u32 hd_clk_sel = clk_sel;
    vo_drv_dev *dev_ctx = vo_drv_get_dev_ctx(dev);

    vo_drv_set_dev_mpp_chn(dev, &mpp_chn);

    if ((dev_ctx->vou_attr.intf_sync > OT_VO_OUT_720P60) && (dev_ctx->vou_attr.intf_sync != OT_VO_OUT_1280x1024_60) &&
        (dev_ctx->vou_attr.intf_sync != OT_VO_OUT_1080P24) && (dev_ctx->vou_attr.intf_sync != OT_VO_OUT_1080P25) &&
        (dev_ctx->vou_attr.intf_sync != OT_VO_OUT_1080P30) && (dev_ctx->vou_attr.intf_sync != OT_VO_OUT_1080I50) &&
        (dev_ctx->vou_attr.intf_sync != OT_VO_OUT_1080I60) && (dev_ctx->vou_attr.intf_sync != OT_VO_OUT_1080P50) &&
        (dev_ctx->vou_attr.intf_sync != OT_VO_OUT_1080P60) && (dev_ctx->vou_attr.intf_sync != OT_VO_OUT_3840x2160_24) &&
        (dev_ctx->vou_attr.intf_sync != OT_VO_OUT_3840x2160_25) &&
        (dev_ctx->vou_attr.intf_sync != OT_VO_OUT_3840x2160_30)) {
        call_sys_drv_ioctrl(&mpp_chn, SYS_VO_HD_CLK_SEL, &hd_clk_sel);
    }
}

td_void vo_drv_set_dev_clk_en(ot_vo_dev dev, td_bool clk_en)
{
    ot_mpp_chn mpp_chn;
    td_bool vo_clk_en = clk_en;

    vo_drv_set_dev_mpp_chn(dev, &mpp_chn);

    if (dev == VO_DEV_DHD0) {
        call_sys_drv_ioctrl(&mpp_chn, SYS_VO_DEV_CLK_EN, &vo_clk_en);
        call_sys_drv_ioctrl(&mpp_chn, SYS_VO_APB_CLK_EN, &vo_clk_en);
        call_sys_drv_ioctrl(&mpp_chn, SYS_VO_CFG_CLK_EN, &vo_clk_en);
        call_sys_drv_ioctrl(&mpp_chn, SYS_VO_LCD_CLK_EN, &vo_clk_en);
    }
}

static td_void vo_drv_set_cbm_bkg(ot_vo_dev dev)
{
    hal_disp_bkcolor bkg;
    td_u32 bg_color;
    td_u32 yuv_bk_grd;
    hal_cbmmix mixer = dev;

    bg_color = vo_drv_get_dev_bg_color(dev);
    yuv_bk_grd = rgb_to_yuv_full(bg_color);
    bkg.bkg_y = YUV_Y(yuv_bk_grd);
    bkg.bkg_cb = YUV_U(yuv_bk_grd);
    bkg.bkg_cr = YUV_V(yuv_bk_grd);

    hal_cbm_set_cbm_bkg(mixer, &bkg);
}

static td_void vo_drv_set_intf_mipi_cfg_comm(ot_vo_dev dev)
{
    ot_mpp_chn mpp_chn;
    td_bool mipi_clk_en = TD_TRUE;
    td_bool mipi_clk_sel = dev;

    vo_drv_set_dev_mpp_chn(dev, &mpp_chn);

    call_sys_drv_ioctrl(&mpp_chn, SYS_VO_MIPI_CLK_EN, &mipi_clk_en);
    call_sys_drv_ioctrl(&mpp_chn, SYS_VO_MIPI_CLK_SEL, &mipi_clk_sel);

    vo_hal_intf_set_mux_sel(dev, OT_VO_INTF_MIPI);
}

static td_void vo_drv_set_intf_mipi_cfg(ot_vo_dev dev)
{
    vo_drv_set_intf_mipi_cfg_comm(dev);
    vo_hal_set_dev_precharge_threshold(dev, TD_FALSE);
}

static td_void vo_drv_set_intf_mipi_slave_cfg(ot_vo_dev dev)
{
    vo_drv_set_intf_mipi_cfg_comm(dev);
    vo_hal_set_dev_precharge_threshold(dev, TD_TRUE);
}

static td_void vo_drv_set_intf_bt1120_cfg(ot_vo_dev dev)
{
    td_bool bt_clk_en = TD_TRUE;
    td_u32 bt_sel = dev;
    td_u32 bt_clk_reverse = TD_TRUE;
    ot_mpp_chn mpp_chn;
    td_bool dfir_en = TD_TRUE;

    vo_drv_set_dev_mpp_chn(dev, &mpp_chn);

    call_sys_drv_ioctrl(&mpp_chn, SYS_VO_BT1120_CLK_EN, &bt_clk_en);  /*  [8][9] bt clk en ,bt bp en. */
    call_sys_drv_ioctrl(&mpp_chn, SYS_VO_BT1120_CLK_SEL, &bt_sel);  /* [25] bt selection ch 0 or ch 1. */

    call_sys_drv_ioctrl(&mpp_chn, SYS_VO_HD_CLKOUT_PHASIC_REVERSE_EN, &bt_clk_reverse);

    vo_hal_intf_set_mux_sel(dev, OT_VO_INTF_BT1120);

    hal_intf_bt_set_dfir_en(dfir_en);
    hal_intf_bt_set_data_width(0x1);
}

static td_void vo_drv_set_intf_bt656_cfg(ot_vo_dev dev)
{
    ot_mpp_chn mpp_chn;
    td_bool bt_clk_en = TD_TRUE;
    td_u32 bt_sel = dev;

    vo_drv_set_dev_mpp_chn(dev, &mpp_chn);
    call_sys_drv_ioctrl(&mpp_chn, SYS_VO_BT1120_CLK_EN, &bt_clk_en);
    call_sys_drv_ioctrl(&mpp_chn, SYS_VO_BT1120_CLK_SEL, &bt_sel);
    vo_hal_intf_set_mux_sel(dev, OT_VO_INTF_BT656);
    hal_intf_bt_set_dfir_en(0x1);
}
td_bool vo_drv_get_dft_rgb_dummy_out_en(td_u32 div_mod, ot_vo_intf_type intf_type)
{
    if ((OT_VO_INTF_RGB_6BIT & intf_type) || (OT_VO_INTF_RGB_8BIT & intf_type)) {
        if (div_mod == VO_INTF_RGB_DIV_MODE_3) {
            return TD_FALSE;
        } else {
            return TD_TRUE;
        }
    }
    return TD_FALSE;
}

static td_void vo_drv_get_intf_rgb_ctrl(ot_vo_intf_type intf_type, reg_intf_lcd_ctrl *lcd_ctrl)
{
    if (OT_VO_INTF_RGB_6BIT & intf_type) {
        lcd_ctrl->bits.lcd_format = 4; /* 4: serial */
        lcd_ctrl->bits.lcd_serial_perd = 0; /* 0: 3 period */
        lcd_ctrl->bits.dfir_en = 0;
        lcd_ctrl->bits.hdmi_mode = 0;
    } else if (OT_VO_INTF_RGB_8BIT & intf_type) {
        lcd_ctrl->bits.lcd_format = 3; /* 3: serial 8bit */
        lcd_ctrl->bits.lcd_serial_perd = 1; /* 0: 4 period */
        lcd_ctrl->bits.dfir_en = 0;
        lcd_ctrl->bits.hdmi_mode = 0;
    } else if (OT_VO_INTF_RGB_16BIT & intf_type) {
        lcd_ctrl->bits.lcd_format = 1; /* 1: parallel 16bit,RGB656 */
        lcd_ctrl->bits.lcd_serial_perd = 0;
        lcd_ctrl->bits.dfir_en = 0;
        lcd_ctrl->bits.hdmi_mode = 0;
    } else if (OT_VO_INTF_RGB_18BIT & intf_type) {
        lcd_ctrl->bits.lcd_format = 2; /* 2: parallel 18bit,RGB666 */
        lcd_ctrl->bits.lcd_serial_perd = 0;
        lcd_ctrl->bits.dfir_en = 0;
        lcd_ctrl->bits.hdmi_mode = 0;
    } else if (OT_VO_INTF_RGB_24BIT & intf_type) {
        lcd_ctrl->bits.lcd_format = 0; /* 0: parallel 24bit,RGB888 */
        lcd_ctrl->bits.lcd_serial_perd = 0;
        lcd_ctrl->bits.dfir_en = 0;
        lcd_ctrl->bits.hdmi_mode = 0;
    }
}

static td_u32 vo_drv_get_rgb_dither_out_width(ot_vo_intf_type intf_type)
{
    if ((intf_type == OT_VO_INTF_RGB_16BIT) ||
        (intf_type == OT_VO_INTF_RGB_18BIT)) {
        return 6; /* 6bit */
    } else {
        return 8; /* 8bit */
    }
}

static td_void vo_drv_set_intf_rgb_cfg(ot_vo_dev dev)
{
    td_u32 bt_sel = dev;
    td_bool bt_clk_en = TD_TRUE;
    ot_vo_intf_type intf_type;
    reg_intf_lcd_ctrl lcd_ctrl = {0};
    ot_mpp_chn mpp_chn;

    intf_type = vo_drv_get_dev_intf_type(dev);

    vo_drv_set_dev_mpp_chn(dev, &mpp_chn);

    call_sys_drv_ioctrl(&mpp_chn, SYS_VO_BT1120_CLK_EN, &bt_clk_en);
    call_sys_drv_ioctrl(&mpp_chn, SYS_VO_BT1120_CLK_SEL, &bt_sel);

    vo_hal_intf_set_mux_sel(dev, intf_type);

    vo_drv_get_intf_rgb_ctrl(intf_type, &lcd_ctrl);

    vo_hal_set_intf_ctrl(intf_type, &(lcd_ctrl.u32));

    /* dither config */
    vo_drv_set_intf_dither(intf_type, vo_drv_get_rgb_dither_out_width(intf_type));
}

static td_void vo_drv_set_intf_cfg(ot_vo_dev dev)
{
    ot_vo_intf_type intf_type;

    intf_type = vo_drv_get_dev_intf_type(dev);
    if (OT_VO_INTF_MIPI & intf_type) {
        vo_drv_set_intf_mipi_cfg(dev);
    }

    if (OT_VO_INTF_MIPI_SLAVE & intf_type) {
        vo_drv_set_intf_mipi_slave_cfg(dev);
    }

    if (OT_VO_INTF_BT1120 & intf_type) {
        vo_drv_set_intf_bt1120_cfg(dev);
    }

    if (OT_VO_INTF_BT656 & intf_type) {
        vo_drv_set_intf_bt656_cfg(dev);
    }

    if (vo_drv_is_rgb_intf(intf_type)) {
        vo_drv_set_intf_rgb_cfg(dev);
    }
}

static td_void vo_drv_get_sync_inv(ot_vo_dev dev, hal_disp_syncinv *inv)
{
    hal_disp_syncinfo sync_info;
    ot_vo_intf_type intf_type;
    ot_vo_intf_sync intf_sync;

    intf_type = vo_drv_get_dev_intf_type(dev);
    intf_sync = vo_drv_get_dev_intf_sync(dev);

    vo_drv_get_sync_info(dev, &sync_info);

    inv->hs_inv = sync_info.ihs ? 1 : 0;
    inv->vs_inv = sync_info.ivs ? 1 : 0;
    inv->dv_inv = sync_info.idv ? 1 : 0;

    if (OT_VO_INTF_HDMI & intf_type) {
        if ((intf_sync == OT_VO_OUT_576P50) || (intf_sync == OT_VO_OUT_480P60)) {
            inv->hs_inv = 1 - inv->hs_inv;
            inv->vs_inv = 1 - inv->vs_inv;
        }
    }

    if (OT_VO_INTF_BT656 & intf_type) {
        inv->hs_inv = 1;
    }

    if (OT_VO_INTF_CVBS & intf_type) {
        inv->hs_inv = 1;
    }
}

static td_void vo_drv_set_sync_inv(ot_vo_dev dev, const hal_disp_syncinv *inv)
{
    ot_vo_intf_type intf_type = vo_drv_get_dev_intf_type(dev);
    if (vo_drv_is_bt_intf(intf_type)) {
        vo_hal_intf_set_bt_sync_inv(inv);
    }

    if (vo_drv_is_cvbs_intf(intf_type)) {
        vo_hal_intf_set_cvbs_sync_inv(inv);
    }

    if (vo_drv_is_mipi_intf(intf_type)) {
        vo_hal_intf_set_mipi_sync_inv(inv);
    }

    if (vo_drv_is_rgb_intf(intf_type)) {
        vo_hal_intf_set_rgb_sync_inv(inv);
    }
}

static td_void vo_drv_get_dev_out_reverse(ot_vo_dev dev, td_bool *clk_reverse)
{
    ot_vo_intf_type intf_type = vo_drv_get_dev_intf_type(dev);
    if (intf_type & OT_VO_INTF_RGB_24BIT) {
        *clk_reverse = TD_FALSE;
    } else {
        *clk_reverse = TD_TRUE;
    }
}

static td_void vo_drv_set_dev_out_reverse(ot_vo_dev dev, td_bool clk_reverse_en)
{
    ot_mpp_chn mpp_chn;
    ot_vo_intf_sync intf_sync;

    intf_sync = vo_drv_get_dev_intf_sync(dev);

    vo_drv_set_dev_mpp_chn(dev, &mpp_chn);

    if (intf_sync != OT_VO_OUT_USER) {
        call_sys_drv_ioctrl(&mpp_chn, SYS_VO_HD_CLKOUT_PHASIC_REVERSE_EN, &clk_reverse_en);
    }
}

static td_void vo_drv_get_div_mod_by_rgb_intf(ot_vo_intf_type intf_type, td_u32 *div_mode)
{
    if (OT_VO_INTF_RGB_8BIT & intf_type) {
        *div_mode = 3; /* 3: 4div */
    } else if (OT_VO_INTF_RGB_6BIT & intf_type) {
        *div_mode = 2; /* 2: 3div */
    } else if ((OT_VO_INTF_RGB_16BIT & intf_type) ||
               (OT_VO_INTF_RGB_18BIT & intf_type) ||
               (OT_VO_INTF_RGB_24BIT & intf_type)) {
        *div_mode = 0; /* 0: 1div */
    }
}

static td_void vo_drv_get_div_mod(ot_vo_dev dev, td_u32 *div_mode)
{
    ot_vo_intf_type intf_type;

    intf_type = vo_drv_get_dev_intf_type(dev);
    if (OT_VO_INTF_BT1120 & intf_type) {
        *div_mode = 0; /* 0: 1div */
    }

    if (OT_VO_INTF_BT656 & intf_type) {
        *div_mode = 1; /* 1: 2div */
    }

    if (OT_VO_INTF_CVBS & intf_type) {
        *div_mode = 3; /* 0: 1div, 1: 2div, 2: 3div, 3: 4div */
    }

    if (vo_drv_is_rgb_intf(intf_type)) {
        vo_drv_get_div_mod_by_rgb_intf(intf_type, div_mode);
    }
}

static td_void vo_drv_set_div_mod(ot_vo_dev dev, td_u32 div_mode)
{
    ot_mpp_chn mpp_chn;
    ot_vo_intf_sync intf_sync;

    intf_sync = vo_drv_get_dev_intf_sync(dev);
    if (intf_sync == OT_VO_OUT_USER) {
        return;
    }

    vo_drv_set_dev_mpp_chn(dev, &mpp_chn);
    call_sys_drv_ioctrl(&mpp_chn, SYS_VO_DEV_DIV_MOD, &div_mode);
}

static td_void vo_drv_set_intf_clip(ot_vo_dev dev)
{
    ot_vo_intf_type intf_type;

    intf_type = vo_drv_get_dev_intf_type(dev);
    if (OT_VO_INTF_BT1120 & intf_type) {
        hal_disp_clip clip_data = {0x40, 0x40, 0x40, 0x3ac, 0x3c0, 0x3c0};
        vo_hal_intf_set_clip_cfg(OT_VO_INTF_BT1120, TD_TRUE, &clip_data);
    }

    if (OT_VO_INTF_CVBS & intf_type) {
        hal_disp_clip clip_data = {0x40, 0x40, 0x40, 0x3ac, 0x3c0, 0x3c0};
        vo_hal_intf_set_clip_cfg(OT_VO_INTF_CVBS, TD_TRUE, &clip_data);
    }

    if (OT_VO_INTF_BT656 & intf_type) {
        hal_disp_clip clip_data = {0x40, 0x40, 0x40, 0x3ac, 0x3c0, 0x3c0};
        vo_hal_intf_set_clip_cfg(OT_VO_INTF_BT656, TD_TRUE, &clip_data);
    }
}

td_void vo_drv_open(ot_vo_dev dev)
{
    hal_disp_syncinfo sync_info;
    hal_disp_syncinv inv = {0};
    td_bool clk_reverse_en = TD_TRUE;
    td_u32 div_mode = 0;

    vo_drv_set_cbm_bkg(dev);

    vo_drv_set_intf_cfg(dev);

    vo_drv_get_sync_inv(dev, &inv);
    vo_drv_set_sync_inv(dev, &inv);
    vo_drv_get_sync_info(dev, &sync_info);
    vo_drv_set_sync_info(dev, &sync_info);

    vo_drv_get_div_mod(dev, &div_mode);
    vo_drv_set_div_mod(dev, div_mode);

    vo_drv_get_dev_out_reverse(dev, &clk_reverse_en);
    vo_drv_set_dev_out_reverse(dev, clk_reverse_en);

    hal_disp_set_dev_multi_chn_en(dev, HAL_MULTICHN_EN_1P1C);

    vo_drv_set_intf_clip(dev);

    hal_disp_set_intf_enable(dev, TD_TRUE);

    hal_disp_set_reg_up(dev);
}

td_void vo_drv_close(ot_vo_dev dev)
{
    ot_mpp_chn mpp_chn;
    td_bool clk_en = TD_FALSE;
    ot_vo_intf_type intf_type;

    hal_disp_set_intf_enable(dev, TD_FALSE);
    hal_disp_set_reg_up(dev);

    vo_drv_set_dev_mpp_chn(dev, &mpp_chn);

    vo_drv_get_dev_reg_up(dev);

    intf_type = vo_drv_get_dev_intf_type(dev);
    if (vo_drv_is_mipi_intf(intf_type)) {
        call_sys_drv_ioctrl(&mpp_chn, SYS_VO_MIPI_CLK_EN, &clk_en);
    }

    if (OT_VO_INTF_BT1120 & intf_type) {
        call_sys_drv_ioctrl(&mpp_chn, SYS_VO_BT1120_CLK_EN, &clk_en);
    }

    if (OT_VO_INTF_BT656 & intf_type) {
        call_sys_drv_ioctrl(&mpp_chn, SYS_VO_BT1120_CLK_EN, &clk_en);
    }

    if (vo_drv_is_rgb_intf(intf_type)) {
        call_sys_drv_ioctrl(&mpp_chn, SYS_VO_BT1120_CLK_EN, &clk_en);
    }
    
    call_sys_drv_ioctrl(&mpp_chn, SYS_VO_LCD_CLK_EN, &clk_en);
}

static td_s32 vo_drv_check_dev_id_support(ot_vo_dev dev)
{
    ot_unused(dev);
    return TD_SUCCESS;
}

static td_s32 vo_drv_check_layer_id_support(ot_vo_layer layer)
{
    ot_unused(layer);
    return TD_SUCCESS;
}

td_s32 vo_drv_check_dev_id(ot_vo_dev dev)
{
    td_s32 ret;
    ret =  vo_drv_check_dev_id_pub(dev);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    return vo_drv_check_dev_id_support(dev);
}

td_s32 vo_drv_check_layer_id(ot_vo_layer layer)
{
    td_s32 ret;
    ret = vo_drv_check_layer_id_pub(layer);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    return vo_drv_check_layer_id_support(layer);
}
#endif

#if vo_desc("dev")
static vo_std_pixel_clk g_vo_std_pixel_clk[OT_VO_OUT_BUTT] = {
    /* index,                fb_div, frac, ref_div, post_div1,post_div2 */
    /* bt.656 */
    {OT_VO_OUT_PAL,          27000}, /* 27MHz = 13.5MHz * 2div */
    {OT_VO_OUT_NTSC,         27000}, /* 27MHz = 13.5MHz * 2div */

    /* not support */
    {OT_VO_OUT_960H_PAL,     36000}, /* 36MHz */
    {OT_VO_OUT_960H_NTSC,    36000}, /* 36MHz */

    /* hdmi, bt.1120, mipi_tx */
    {OT_VO_OUT_640x480_60,   25200}, /* 25.2MHz */
    {OT_VO_OUT_480P60,       27000}, /* 27MHz */
    {OT_VO_OUT_576P50,       27000}, /* 27MHz */
    {OT_VO_OUT_800x600_60,   40000}, /* 40MHz */
    {OT_VO_OUT_1024x768_60,  65000}, /* 65MHz */
    {OT_VO_OUT_720P50,       74250}, /* 74.25MHz */
    {OT_VO_OUT_720P60,       74250}, /* 74.25MHz */
    {OT_VO_OUT_1280x800_60,  83500}, /* 83.5MHz */
    {OT_VO_OUT_1280x1024_60, 108000}, /* 108MHz */
    {OT_VO_OUT_1366x768_60,  85500}, /* 85.5MHz */
    {OT_VO_OUT_1400x1050_60, 121750}, /* 121.75MHz */
    {OT_VO_OUT_1440x900_60,  106500}, /* 106.5MHz */
    {OT_VO_OUT_1680x1050_60, 146250}, /* 146.25MHz */
    {OT_VO_OUT_1080P24,      74250}, /* 74.25MHz */
    {OT_VO_OUT_1080P25,      74250}, /* 74.25MHz */
    {OT_VO_OUT_1080P30,      74250}, /* 74.25MHz */
    {OT_VO_OUT_1080I50,      74250}, /* 74.25MHz */
    {OT_VO_OUT_1080I60,      74250}, /* 74.25MHz */
    {OT_VO_OUT_1080P50,      148500}, /* 148.5MHz */
    {OT_VO_OUT_1080P60,      148500}, /* 148.5MHz */
    {OT_VO_OUT_1600x1200_60, 162000}, /* 162MHz */
    {OT_VO_OUT_1920x1200_60, 154000}, /* 154MHz */
    {OT_VO_OUT_1920x2160_30, 148500}, /* 148.5MHz */
    {OT_VO_OUT_2560x1440_30, 120900}, /* 120.9MHz */
    {OT_VO_OUT_2560x1440_60, 241500}, /* 241.5MHz */
    {OT_VO_OUT_2560x1600_60, 268500}, /* 268.5MHz */
    {OT_VO_OUT_3840x2160_24, 297000}, /* 297MHz */
    {OT_VO_OUT_3840x2160_25, 297000}, /* 297MHz */
    {OT_VO_OUT_3840x2160_30, 297000}, /* 297MHz */
    {OT_VO_OUT_3840x2160_50, 594000}, /* 594MHz */
    {OT_VO_OUT_3840x2160_60, 594000}, /* 594MHz */

    /* not support */
    {OT_VO_OUT_4096x2160_24, 297000}, /* 297MHz */
    {OT_VO_OUT_4096x2160_25, 297000}, /* 297MHz */
    {OT_VO_OUT_4096x2160_30, 297000}, /* 297MHz */
    {OT_VO_OUT_4096x2160_50, 594000}, /* 594MHz */
    {OT_VO_OUT_4096x2160_60, 594000}, /* 594MHz */
    {OT_VO_OUT_7680x4320_30, 1188000}, /* 1188MHz */
    {OT_VO_OUT_240x320_50,   4500}, /* 4.5MHz */
    {OT_VO_OUT_320x240_50,   4000}, /* 4.0MHz */
    {OT_VO_OUT_240x320_60,   5500}, /* 5.5MHz */
    {OT_VO_OUT_320x240_60,   6200}, /* 6.2MHz */
    {OT_VO_OUT_800x600_50,   33500}, /* 33.5MHz */

    /* mipi_tx */
    {OT_VO_OUT_720x1280_60,  74250}, /* 74.25MHz */
    {OT_VO_OUT_1080x1920_60, 148500}, /* 148.5MHz */
    {} /* user */
};

td_void vo_drv_set_std_ppc_clk(ot_vo_intf_sync intf_sync)
{
    td_u32 index = 0;
    td_u32 pixel_clk_khz = VO_297000_KHZ_CLK;
    td_u32 size = sizeof(g_vo_std_pixel_clk) / sizeof(vo_std_pixel_clk);
    
    for (index = 0; index < size; index++) {
        if (intf_sync == g_vo_std_pixel_clk[index].index) {
            pixel_clk_khz = g_vo_std_pixel_clk[index].pixel_clk;
            break;
        }
    }

    vo_drv_set_ppc_crg_clk_sel(pixel_clk_khz);
}

td_void vo_drv_dev_get_clk_pll_src(ot_vo_dev dev, td_u32 *pll_src)
{
    vo_drv_dev *dev_ctx = vo_drv_get_dev_ctx(dev);

    if (vo_drv_is_rgb_intf(dev_ctx->vou_attr.intf_type)) {
        *pll_src = VO_CLK_PLL_SRC_FOUT4;
    }
}

/*
 * hdmi, bt.1120, mipi_tx: 1div
 * bt656: 2div
 * ref_div: 1
 */
static vo_pll_param g_vo_pll_param_no_div[OT_VO_OUT_BUTT] = {
    /* index,                fb_div, frac, ref_div, post_div1,post_div2 */
    /* bt.656 */
    {OT_VO_OUT_PAL,          {40,  0x800000, 1,  6, 6}, 0}, /* 27MHz = 13.5MHz * 2div */
    {OT_VO_OUT_NTSC,         {40,  0x800000, 1,  6, 6}, 0}, /* 27MHz = 13.5MHz * 2div */

    /* not support */
    {OT_VO_OUT_960H_PAL,     {36,  0,        1,  6, 4}, 0}, /* 36MHz */
    {OT_VO_OUT_960H_NTSC,    {36,  0,        1,  6, 4}, 0}, /* 36MHz */

    /* hdmi, bt.1120, mipi_tx */
    {OT_VO_OUT_640x480_60,   {36,  0xc00000, 1,  7, 5}, 0}, /* 25.2MHz */
    {OT_VO_OUT_480P60,       {40,  0x800000, 1,  6, 6}, 0}, /* 27MHz */
    {OT_VO_OUT_576P50,       {40,  0x800000, 1,  6, 6}, 0}, /* 27MHz */
    {OT_VO_OUT_800x600_60,   {40,  0,        1,  6, 4}, 0}, /* 40MHz */
    {OT_VO_OUT_1024x768_60,  {65,  0,        1,  6, 4}, 0}, /* 65MHz */
    {OT_VO_OUT_720P50,       {49,  0x800000, 1,  4, 4}, 0}, /* 74.25MHz */
    {OT_VO_OUT_720P60,       {49,  0x800000, 1,  4, 4}, 0}, /* 74.25MHz */
    {OT_VO_OUT_1280x800_60,  {83,  0x800000, 1,  6, 4}, 0}, /* 83.5MHz */
    {OT_VO_OUT_1280x1024_60, {108, 0,        1,  6, 4}, 0}, /* 108MHz */
    {OT_VO_OUT_1366x768_60,  {85,  0x800000, 1,  6, 4}, 0}, /* 85.5MHz */
    {OT_VO_OUT_1400x1050_60, {121, 0xc00000, 1,  6, 4}, 0}, /* 121.75MHz */
    {OT_VO_OUT_1440x900_60,  {106, 0x800000, 1,  6, 4}, 0}, /* 106.5MHz */
    {OT_VO_OUT_1680x1050_60, {97,  0x800000, 1,  4, 4}, 0}, /* 146.25MHz */
    {OT_VO_OUT_1080P24,      {49,  0x800000, 1,  4, 4}, 0}, /* 74.25MHz */
    {OT_VO_OUT_1080P25,      {49,  0x800000, 1,  4, 4}, 0}, /* 74.25MHz */
    {OT_VO_OUT_1080P30,      {49,  0x800000, 1,  4, 4}, 0}, /* 74.25MHz */
    {OT_VO_OUT_1080I50,      {49,  0x800000, 1,  4, 4}, 0}, /* 74.25MHz */
    {OT_VO_OUT_1080I60,      {49,  0x800000, 1,  4, 4}, 0}, /* 74.25MHz */
    {OT_VO_OUT_1080P50,      {99,  0,        1,  4, 4}, 0}, /* 148.5MHz */
    {OT_VO_OUT_1080P60,      {99,  0,        1,  4, 4}, 0}, /* 148.5MHz */
    {OT_VO_OUT_1600x1200_60, {81,  0,        1,  6, 2}, 0}, /* 162MHz */
    {OT_VO_OUT_1920x1200_60, {77,  0,        1,  6, 2}, 0}, /* 154MHz */
    {OT_VO_OUT_1920x2160_30, {99,  0,        1,  4, 4}, 0}, /* 148.5MHz */
    {OT_VO_OUT_2560x1440_30, {50,  0x600000, 1,  5, 2}, 0}, /* 120.9MHz */
    {OT_VO_OUT_2560x1440_60, {80,  0x800000, 1,  4, 2}, 0}, /* 241.5MHz */
    {OT_VO_OUT_2560x1600_60, {89,  0x800000, 1,  4, 2}, 0}, /* 268.5MHz */
    {OT_VO_OUT_3840x2160_24, {99,  0,        1,  4, 2}, 0}, /* 297MHz */
    {OT_VO_OUT_3840x2160_25, {99,  0,        1,  4, 2}, 0}, /* 297MHz */
    {OT_VO_OUT_3840x2160_30, {99,  0,        1,  4, 2}, 0}, /* 297MHz */
    {OT_VO_OUT_3840x2160_50, {99,  0,        1,  2, 2}, 0}, /* 594MHz */
    {OT_VO_OUT_3840x2160_60, {99,  0,        1,  2, 2}, 0}, /* 594MHz */

    /* not support */
    {OT_VO_OUT_4096x2160_24, {99,  0,        1,  4, 2}, 0}, /* 297MHz */
    {OT_VO_OUT_4096x2160_25, {99,  0,        1,  4, 2}, 0}, /* 297MHz */
    {OT_VO_OUT_4096x2160_30, {99,  0,        1,  4, 2}, 0}, /* 297MHz */
    {OT_VO_OUT_4096x2160_50, {99,  0,        1,  2, 2}, 0}, /* 594MHz */
    {OT_VO_OUT_4096x2160_60, {99,  0,        1,  2, 2}, 0}, /* 594MHz */
    {OT_VO_OUT_7680x4320_30, {99,  0,        1,  2, 1}, 0}, /* 1188MHz */
    {OT_VO_OUT_240x320_50,   {36,  1207960,  1,  4, 2}, 0}, /* 4.5MHz */
    {OT_VO_OUT_320x240_50,   {36,  13136560, 1,  3, 3}, 0}, /* 4.0MHz */
    {OT_VO_OUT_240x320_60,   {36,  12348031, 1,  5, 4}, 0}, /* 5.5MHz */
    {OT_VO_OUT_320x240_60,   {41,  6630356,  1,  5, 4}, 0}, /* 6.2MHz */
    {OT_VO_OUT_800x600_50,   {33,  8858370,  1,  3, 1}, 0}, /* 33.5MHz */

    /* mipi_tx */
    {OT_VO_OUT_720x1280_60,  {49,  0x800000, 1,  4, 4}, 0}, /* 74.25MHz */
    {OT_VO_OUT_1080x1920_60, {99,  0,        1,  4, 4}, 0}, /* 148.5MHz */
    {} /* user */
};

static vo_pll_param *vo_drv_get_pll_param_no_div(ot_vo_intf_sync intf_sync)
{
    return &g_vo_pll_param_no_div[intf_sync];
}

td_void vo_drv_get_pll_cfg_no_div(ot_vo_intf_sync intf_sync, ot_vo_pll *pll)
{
    vo_pll_param pll_init = {OT_VO_OUT_1080P60, {99, 0, 1, 4, 4}, 0};
    vo_pll_param *pll_ret = &pll_init;

    if (intf_sync < OT_VO_OUT_USER) {
        pll_ret = vo_drv_get_pll_param_no_div(intf_sync);
    }

    pll->post_div2 = pll_ret->pll.post_div2;
    pll->post_div1 = pll_ret->pll.post_div1;
    pll->frac = pll_ret->pll.frac;
    pll->fb_div = pll_ret->pll.fb_div;
    pll->ref_div = pll_ret->pll.ref_div;
}

td_void vo_drv_dev_get_pll_cfg(ot_vo_dev dev, ot_vo_pll *pll)
{
    vo_drv_dev *dev_ctx = vo_drv_get_dev_ctx(dev);
    vo_drv_get_pll_cfg_no_div(dev_ctx->vou_attr.intf_sync, pll);
}

static td_void vo_drv_dev_set_pll_power_down(ot_vo_dev dev, td_bool power_down)
{
    ot_mpp_chn mpp_chn = {0};

    vo_drv_set_dev_mpp_chn(dev, &mpp_chn);
    call_sys_drv_ioctrl(&mpp_chn, SYS_VO_PLL_POWER_CTRL, &power_down);
}

td_void vo_drv_dev_set_pll_cfg(ot_vo_dev dev, ot_vo_pll *pll)
{
    ot_mpp_chn mpp_chn = {0};

    vo_drv_set_dev_mpp_chn(dev, &mpp_chn);
    vo_drv_dev_set_pll_power_down(dev, TD_TRUE);
    call_sys_drv_ioctrl(&mpp_chn, SYS_VO_PLL_FBDIV_SET, &pll->fb_div);
    call_sys_drv_ioctrl(&mpp_chn, SYS_VO_PLL_FRAC_SET, &pll->frac);
    call_sys_drv_ioctrl(&mpp_chn, SYS_VO_PLL_REFDIV_SET, &pll->ref_div);
    call_sys_drv_ioctrl(&mpp_chn, SYS_VO_PLL_POSTDIV1_SET, &pll->post_div1);
    call_sys_drv_ioctrl(&mpp_chn, SYS_VO_PLL_POSTDIV2_SET, &pll->post_div2);
    vo_drv_dev_set_pll_power_down(dev, TD_FALSE);
}

td_s32 vo_drv_check_dev_pll_ref_div(ot_vo_dev dev, td_u32 ref_div)
{
    /* ref_div should be 1 */
    if (ref_div != VO_DEF_PLL_REF_DIV) {
        vo_err_trace("dev(%d) pll ref_div %d illegal, should be %d\n",
            dev, ref_div, VO_DEF_PLL_REF_DIV);
        return OT_ERR_VO_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

/* bt1120, mipi: 1 div */
static vo_fix_lcd_param g_vo_fix_lcd_param_1_div[] = {
    /* index,                src_clk, fixed, lcd_mclk_div */
    {OT_VO_OUT_PAL,          OT_VO_CLK_SRC_LCDMCLK, .lcd_mclk_div = 0x2e8ba3}, /* 27MHz / 2div */
    {OT_VO_OUT_NTSC,         OT_VO_CLK_SRC_LCDMCLK, .lcd_mclk_div = 0x2e8ba3}, /* 27MHz / 2div */
    {OT_VO_OUT_640x480_60,   OT_VO_CLK_SRC_LCDMCLK, .lcd_mclk_div = 0x2b7143}, /* 25.2MHz */
    {OT_VO_OUT_480P60,       OT_VO_CLK_SRC_LCDMCLK, .lcd_mclk_div = 0x2e8ba3}, /* 27MHz */
    {OT_VO_OUT_576P50,       OT_VO_CLK_SRC_LCDMCLK, .lcd_mclk_div = 0x2e8ba3}, /* 27MHz */
    {OT_VO_OUT_800x600_60,   OT_VO_CLK_SRC_LCDMCLK, .lcd_mclk_div = 0x44f4cc}, /* 40MHz */
    {OT_VO_OUT_1024x768_60,  OT_VO_CLK_SRC_LCDMCLK, .lcd_mclk_div = 0x700dcb}, /* 65MHz */
    {OT_VO_OUT_720P50,       OT_VO_CLK_SRC_FIXED,   .fixed_clk = OT_VO_FIXED_CLK_74_25M}, /* 74.25MHz */
    {OT_VO_OUT_720P60,       OT_VO_CLK_SRC_FIXED,   .fixed_clk = OT_VO_FIXED_CLK_74_25M}, /* 74.25MHz */
    {OT_VO_OUT_1280x1024_60, OT_VO_CLK_SRC_FIXED,   .fixed_clk = OT_VO_FIXED_CLK_108M}, /* 108MHz */
    {OT_VO_OUT_1080P24,      OT_VO_CLK_SRC_FIXED,   .fixed_clk = OT_VO_FIXED_CLK_74_25M}, /* 74.25MHz */
    {OT_VO_OUT_1080P25,      OT_VO_CLK_SRC_FIXED,   .fixed_clk = OT_VO_FIXED_CLK_74_25M}, /* 74.25MHz */
    {OT_VO_OUT_1080P30,      OT_VO_CLK_SRC_FIXED,   .fixed_clk = OT_VO_FIXED_CLK_74_25M}, /* 74.25MHz */
    {OT_VO_OUT_1080I50,      OT_VO_CLK_SRC_FIXED,   .fixed_clk = OT_VO_FIXED_CLK_74_25M}, /* 74.25MHz */
    {OT_VO_OUT_1080I60,      OT_VO_CLK_SRC_FIXED,   .fixed_clk = OT_VO_FIXED_CLK_74_25M}, /* 74.25MHz */
    {OT_VO_OUT_1080P50,      OT_VO_CLK_SRC_FIXED,   .fixed_clk = OT_VO_FIXED_CLK_148_5M}, /* 148.5MHz */
    {OT_VO_OUT_1080P60,      OT_VO_CLK_SRC_FIXED,   .fixed_clk = OT_VO_FIXED_CLK_148_5M}, /* 148.5MHz */
    {OT_VO_OUT_720x1280_60,  OT_VO_CLK_SRC_FIXED,   .fixed_clk = OT_VO_FIXED_CLK_74_25M}, /* 74.25MHz */
    {OT_VO_OUT_1080x1920_60, OT_VO_CLK_SRC_FIXED,   .fixed_clk = OT_VO_FIXED_CLK_148_5M}, /* 148.5MHz */
    {OT_VO_OUT_3840x2160_25, OT_VO_CLK_SRC_FIXED,   .fixed_clk = OT_VO_FIXED_CLK_297M}, /* 297MHz */
    {OT_VO_OUT_3840x2160_24, OT_VO_CLK_SRC_FIXED,   .fixed_clk = OT_VO_FIXED_CLK_297M}, /* 297MHz */
    {OT_VO_OUT_3840x2160_30, OT_VO_CLK_SRC_FIXED,   .fixed_clk = OT_VO_FIXED_CLK_297M}, /* 297MHz */
};

/* bt656: 2 div */
static vo_fix_lcd_param g_vo_fix_lcd_param_2_div[] = {
    /* index,                src_clk, fixed, lcd_mclk_div */
    {OT_VO_OUT_PAL,  OT_VO_CLK_SRC_LCDMCLK, .lcd_mclk_div = 0x2e8ba3}, /* 27MHz / 2div */
    {OT_VO_OUT_NTSC, OT_VO_CLK_SRC_LCDMCLK, .lcd_mclk_div = 0x2e8ba3}, /* 27MHz / 2div */
};

static td_void vo_drv_get_fix_lcd_param_by_intf(ot_vo_intf_type intf_type, vo_fix_lcd_param **fix_lcd_param,
    td_u32 *sync_num)
{
    if (vo_drv_is_bt1120_intf(intf_type) || vo_drv_is_mipi_intf(intf_type)) {
        *fix_lcd_param = g_vo_fix_lcd_param_1_div;
        *sync_num = sizeof(g_vo_fix_lcd_param_1_div) / sizeof(vo_fix_lcd_param);
    } else if (vo_drv_is_bt656_intf(intf_type)) {
        *fix_lcd_param = g_vo_fix_lcd_param_2_div;
        *sync_num = sizeof(g_vo_fix_lcd_param_2_div) / sizeof(vo_fix_lcd_param);
    }
}

static td_void vo_drv_get_fix_lcd_param(ot_vo_intf_sync intf_sync, const vo_fix_lcd_param *fix_lcd_input,
    td_u32 sync_num, vo_fix_lcd_param *fix_lcd_output)
{
    vo_fix_lcd_param pll_init = {OT_VO_OUT_1080P60, OT_VO_CLK_SRC_FIXED, .fixed_clk = OT_VO_FIXED_CLK_148_5M};
    const vo_fix_lcd_param *pll_ret = &pll_init;
    td_u32 loop;

    for (loop = 0; loop < sync_num; loop++) {
        if (intf_sync == fix_lcd_input[loop].index) {
            pll_ret = &(fix_lcd_input[loop]);
        }
    }
    fix_lcd_output->index = pll_ret->index;
    fix_lcd_output->clk_src = pll_ret->clk_src;
    fix_lcd_output->fixed_clk = pll_ret->fixed_clk;
    fix_lcd_output->lcd_mclk_div = pll_ret->lcd_mclk_div;
}

static td_void vo_drv_set_fix_lcd_param(ot_vo_dev dev, const vo_fix_lcd_param *fix_lcd_input)
{
    td_u32 hd_clk_sel;
    td_u32 lcd_mclk_div;
    ot_mpp_chn mpp_chn;
    ot_vo_clk_src clk_src = fix_lcd_input->clk_src;

    vo_drv_set_dev_mpp_chn(dev, &mpp_chn);
    if (clk_src == OT_VO_CLK_SRC_LCDMCLK) {
        hd_clk_sel = 0x7; /* 0x7: lcd_mclk */
        lcd_mclk_div = fix_lcd_input->lcd_mclk_div;
        call_sys_drv_ioctrl(&mpp_chn, SYS_VO_LCD_MCLK_DIV, &lcd_mclk_div);
    } else if (clk_src == OT_VO_CLK_SRC_FIXED) {
        hd_clk_sel = vo_drv_get_fixed_clk_sel(fix_lcd_input->fixed_clk);
    } else {
        return;
    }

    call_sys_drv_ioctrl(&mpp_chn, SYS_VO_HD_CLK_SEL, &hd_clk_sel);
}

td_u32 vo_drv_dev_get_max_clk(ot_vo_dev dev)
{
    ot_unused(dev);

    return VO_297000_KHZ_CLK;
}

td_bool vo_drv_is_support_hdmi_ssc_vdp_div(td_void)
{
    return TD_FALSE;
}

td_void vo_drv_get_hdmi_ssc_vdp_div(ot_vo_intf_sync intf_sync, td_u32 *hdmi_ssc_vdp_div)
{
    ot_unused(intf_sync);
    *hdmi_ssc_vdp_div = 0;
}

static td_void vo_drv_set_std_fix_lcd_clk(td_void)
{
    vo_drv_dev *dev_ctx = vo_drv_get_dev_ctx(VO_DEV_DHD0);
    ot_vo_intf_type intf_type = dev_ctx->vou_attr.intf_type;
    ot_vo_intf_sync intf_sync = dev_ctx->vou_attr.intf_sync;
    vo_fix_lcd_param fix_lcd_param_get = {0};
    vo_fix_lcd_param *fix_lcd_param = TD_NULL;
    td_u32 sync_num = 0;

    vo_drv_get_fix_lcd_param_by_intf(intf_type, &fix_lcd_param, &sync_num);

    vo_drv_get_fix_lcd_param(intf_sync, fix_lcd_param, sync_num, &fix_lcd_param_get);
    vo_drv_set_fix_lcd_param(VO_DEV_DHD0, &fix_lcd_param_get);
}

static td_void vo_drv_set_std_hd_clk(ot_vo_dev dev)
{
    ot_vo_pll pll = { 0 };
    vo_drv_dev *dev_ctx = vo_drv_get_dev_ctx(dev);

    if ((dev_ctx->vou_attr.intf_sync > OT_VO_OUT_720P60) && (dev_ctx->vou_attr.intf_sync != OT_VO_OUT_1280x1024_60) &&
        (dev_ctx->vou_attr.intf_sync != OT_VO_OUT_1080P24) && (dev_ctx->vou_attr.intf_sync != OT_VO_OUT_1080P25) &&
        (dev_ctx->vou_attr.intf_sync != OT_VO_OUT_1080P30) && (dev_ctx->vou_attr.intf_sync != OT_VO_OUT_1080I50) &&
        (dev_ctx->vou_attr.intf_sync != OT_VO_OUT_1080I60) && (dev_ctx->vou_attr.intf_sync != OT_VO_OUT_1080P50) &&
        (dev_ctx->vou_attr.intf_sync != OT_VO_OUT_1080P60) && (dev_ctx->vou_attr.intf_sync != OT_VO_OUT_3840x2160_24) &&
        (dev_ctx->vou_attr.intf_sync != OT_VO_OUT_3840x2160_25) &&
        (dev_ctx->vou_attr.intf_sync != OT_VO_OUT_3840x2160_30)) {
        vo_drv_dev_get_pll_cfg(dev, &pll);
        vo_drv_dev_set_pll_cfg(dev, &pll);
    } else {
        vo_drv_set_std_fix_lcd_clk();
    }
    vo_drv_set_std_ppc_clk(dev_ctx->vou_attr.intf_sync);
}

td_void vo_drv_set_dev_clk(ot_vo_dev dev)
{
    vo_drv_dev *dev_ctx = vo_drv_get_dev_ctx(dev);
    if (dev_ctx->vou_attr.intf_sync == OT_VO_OUT_USER) {
        return;
    }

    if (vo_drv_is_phy_dev_uhd(dev) || vo_drv_is_phy_dev_hd(dev)) {
        vo_drv_set_std_hd_clk(dev);
    }
}

#endif

#if vo_desc("dev clk")
static td_u32 vo_drv_cal_ppc_crg_clk_sel(td_u32 pixel_clk)
{
    td_u32 ppc_clk_sel;
    td_u32 ppc_clk_div = 0;
    ot_mpp_chn mpp_chn;

    vo_drv_set_dev_mpp_chn(0, &mpp_chn);
    if (pixel_clk > VO_136000_KHZ_CLK) {
        ppc_clk_sel = 0x3;
    } else if (pixel_clk > VO_90000_KHZ_CLK) {
        ppc_clk_sel = 0x2;
    } else if (pixel_clk > VO_45000_KHZ_CLK) {
        ppc_clk_sel = 0x1;
    } else if (pixel_clk > VO_12500_KHZ_CLK) {
        ppc_clk_sel = 0x0;
    } else if (pixel_clk > VO_4200_KHZ_CLK) {
        ppc_clk_div = 0x2;
        ppc_clk_sel = 0x0;
    } else if (pixel_clk > VO_1200_KHZ_CLK) {
        ppc_clk_div = 0xA;
        ppc_clk_sel = 0x0;
    } else {
        ppc_clk_div = 0x1F;
        ppc_clk_sel = 0x0;
    }

    call_sys_drv_ioctrl(&mpp_chn, SYS_VO_PPC_CFG_CLK_SEL, &ppc_clk_div);
    return ppc_clk_sel;
}

td_void vo_drv_set_def_hd_crg_clk_sel(td_void)
{
    td_u32 hd_clk_sel = 0x5; /* The hd clock is set to 148.5 MHz before the timing is confirmed. */
    ot_mpp_chn mpp_chn;
    vo_drv_set_dev_mpp_chn(0, &mpp_chn);
    call_sys_drv_ioctrl(&mpp_chn, SYS_VO_HD_CLK_SEL, &hd_clk_sel);
}

td_void vo_drv_set_ppc_crg_clk_sel(td_u32 pixel_clk)
{
    td_u32 ppc_clk_sel;
    ot_mpp_chn mpp_chn;
    vo_drv_set_dev_mpp_chn(0, &mpp_chn);
    ppc_clk_sel = vo_drv_cal_ppc_crg_clk_sel(pixel_clk);
    call_sys_drv_ioctrl(&mpp_chn, SYS_VO_PPC_CLK_SEL, &ppc_clk_sel);
}

td_void vo_drv_set_def_ppc_crg_clk_sel(td_void)
{
    td_u32 def_ppc_clk_sel = 0x3; /* The PPC clock is set to 300 MHz by default. */
    ot_mpp_chn mpp_chn;
    vo_drv_set_dev_mpp_chn(0, &mpp_chn);
    call_sys_drv_ioctrl(&mpp_chn, SYS_VO_PPC_CLK_SEL, &def_ppc_clk_sel);
}

td_void vo_drv_set_all_crg_clk(td_bool clk_en)
{
    ot_mpp_chn mpp_chn;
    td_bool ppc_cfg = TD_TRUE;
    vo_drv_set_dev_mpp_chn(0, &mpp_chn);
    ot_unused(clk_en);
    call_sys_drv_ioctrl(&mpp_chn, SYS_VO_CFG_CLK_EN, &ppc_cfg);
    call_sys_drv_ioctrl(&mpp_chn, SYS_VO_APB_CLK_EN, &ppc_cfg);
    call_sys_drv_ioctrl(&mpp_chn, SYS_VO_BUS_CLK_EN, &ppc_cfg);
    call_sys_drv_ioctrl(&mpp_chn, SYS_VO_CORE_CLK_EN, &ppc_cfg);
    call_sys_drv_ioctrl(&mpp_chn, SYS_VO_DEV_CLK_EN, &ppc_cfg);
}

td_void vo_lpw_bus_reset(td_bool reset)
{
    ot_mpp_chn mpp_chn;

    /* see sys_hal_vo_bus_reset_sel */
    vo_drv_set_dev_mpp_chn(0, &mpp_chn);
    call_sys_drv_ioctrl(&mpp_chn, SYS_VO_BUS_RESET_SEL, &reset);
}

#endif

#if vo_desc("dev intf")

td_bool vo_drv_is_dev_support_rgb(ot_vo_dev dev)
{
    return vo_drv_is_phy_dev_hd(dev);
}

#endif

td_s32 vo_drv_check_dev_clk_src(ot_vo_dev dev, ot_vo_clk_src clk_src)
{
    if (dev == VO_DEV_DHD0) {
        if ((clk_src != OT_VO_CLK_SRC_PLL) &&
            (clk_src != OT_VO_CLK_SRC_LCDMCLK) &&
            (clk_src != OT_VO_CLK_SRC_FIXED)) {
            vo_err_trace("dev(%d) clk source %d illegal, not support the clk source.\n",
                dev, clk_src);
            return OT_ERR_VO_ILLEGAL_PARAM;
        }
    }

    return TD_SUCCESS;
}

static td_s32 vo_drv_check_dev_lcdmclkdiv(ot_vo_dev dev, td_u32 lcd_m_clk_div)
{
    if ((lcd_m_clk_div > VO_LCDMCLK_DIV_MAX) || (lcd_m_clk_div == 0)) {
        vo_err_trace("dev(%d) lcd_mclk value %d illegal, it must be in (0,%d].\n", dev, lcd_m_clk_div,
            VO_LCDMCLK_DIV_MAX);
        return OT_ERR_VO_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

static td_s32 vo_drv_check_dev_fixed_clk(ot_vo_dev dev, ot_vo_fixed_clk fixed_clk)
{
    if (fixed_clk >= OT_VO_FIXED_CLK_BUTT) {
        vo_err_trace("dev(%d) fixed clk value %d illegal.\n", dev, fixed_clk);
        return OT_ERR_VO_ILLEGAL_PARAM;
    }
    return TD_SUCCESS;
}

td_s32 vo_drv_check_dev_clkvalue(ot_vo_dev dev, const ot_vo_user_sync_info *sync_info)
{
    td_s32 ret;
    const ot_vo_manual_user_sync_info *manual_user_sync_info = &sync_info->manual_user_sync_info;
    ot_vo_clk_src clk_src = manual_user_sync_info->user_sync_attr.clk_src;

    if (dev != VO_DEV_DHD0) {
        return TD_SUCCESS;
    }

    ret = vo_drv_check_dev_clk_src(dev, clk_src);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    if (clk_src == OT_VO_CLK_SRC_PLL) {
        return vo_drv_check_dev_pll(dev, manual_user_sync_info);
    } else if (clk_src == OT_VO_CLK_SRC_LCDMCLK) {
        return vo_drv_check_dev_lcdmclkdiv(dev, manual_user_sync_info->user_sync_attr.lcd_m_clk_div);
    } else if (clk_src == OT_VO_CLK_SRC_FIXED) {
        return vo_drv_check_dev_fixed_clk(dev, manual_user_sync_info->user_sync_attr.fixed_clk);
    }

    return OT_ERR_VO_ILLEGAL_PARAM;
}

static td_s32 vo_drv_check_rgb_intf_user_dev_div(ot_vo_intf_type intf_type, ot_vo_dev dev, td_u32 dev_div)
{
    if ((dev_div != VO_INTF_RGB_DIV_MODE_1) &&
        (dev_div != VO_INTF_RGB_DIV_MODE_3) &&
        (dev_div != VO_INTF_RGB_DIV_MODE_4)) {
        vo_err_trace("vo(%d) div mode %d is illegal, it must be %d/%d/%d when intf type is %d.\n", dev, dev_div,
            VO_INTF_RGB_DIV_MODE_1, VO_INTF_RGB_DIV_MODE_3, VO_INTF_RGB_DIV_MODE_4, intf_type);
        return OT_ERR_VO_ILLEGAL_PARAM;
    }

    if ((dev_div != VO_INTF_RGB_DIV_MODE_1) && (intf_type == OT_VO_INTF_RGB_16BIT ||
        intf_type == OT_VO_INTF_RGB_18BIT || intf_type == OT_VO_INTF_RGB_24BIT)) {
        vo_err_trace("vo(%d) div mode %d is illegal, it must be %d when intf type is %d.\n", dev, dev_div,
            VO_INTF_RGB_DIV_MODE_1, intf_type);
        return OT_ERR_VO_ILLEGAL_PARAM;
    }
    if ((dev_div == VO_INTF_RGB_DIV_MODE_1) &&
        (intf_type == OT_VO_INTF_RGB_6BIT || intf_type == OT_VO_INTF_RGB_8BIT)) {
        vo_err_trace("vo(%d) div mode %d is illegal, it must be %d/%d when intf type is %d.\n", dev, dev_div,
            VO_INTF_RGB_DIV_MODE_3, VO_INTF_RGB_DIV_MODE_4, intf_type);
        return OT_ERR_VO_ILLEGAL_PARAM;
    }
    return TD_SUCCESS;
}
td_s32 vo_drv_check_intf_user_dev_div(ot_vo_dev dev, td_u32 dev_div)
{
    ot_vo_intf_type intf_type;
    td_u32 intf_dev_div = 0;
    intf_type = vo_drv_get_dev_intf_type(dev);
    if (vo_drv_is_rgb_intf(intf_type)) {
        return vo_drv_check_rgb_intf_user_dev_div(intf_type, dev, dev_div);
    }

    if (intf_type & (OT_VO_INTF_HDMI | OT_VO_INTF_BT1120 | OT_VO_INTF_MIPI | OT_VO_INTF_MIPI_SLAVE)) {
        intf_dev_div = VO_INTF_HDMI_DIV_MODE;
    }

    if (intf_type & OT_VO_INTF_CVBS) {
        intf_dev_div = VO_INTF_CVBS_DIV_MODE;
    }

    if (intf_type & OT_VO_INTF_BT656) {
        intf_dev_div = VO_INTF_BT656_DIV_MODE;
    }

    if (dev_div != intf_dev_div) {
        vo_err_trace("vo(%d) dev div mode %d is illegal, it must be %d when intf type is %d.\n", dev, dev_div,
            intf_dev_div, intf_type);
        return OT_ERR_VO_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

#if vo_desc("intf dither")
static td_void vo_drv_get_intf_dither_in_out_mode(vdp_dither_cfg *cfg)
{
    td_u32 i_data_width_dither = 0;
    td_u32 o_data_width_dither = 0;
    td_u32 dither_io_md;
    td_u32 dither_md = 0;

    dither_io_md = cfg->io_mode;
    if (dither_io_md == DITHER_IO_MODE_12_8) { /* rgb24bit */
        dither_md = DITHER_MODE_8BIT;
        i_data_width_dither = DITHER_IWIDTH_MODE_12BIT;
        o_data_width_dither = DITHER_OWIDTH_MODE_8BIT;
    } else if (dither_io_md == DITHER_IO_MODE_10_6) { /* rgb16bit, rgb18bit */
        dither_md = DITHER_MODE_8BIT;
        i_data_width_dither = DITHER_IWIDTH_MODE_10BIT;
        o_data_width_dither = DITHER_OWIDTH_MODE_6BIT;
    }
    cfg->dither_mode = dither_md;
    cfg->i_data_width_dither = i_data_width_dither;
    cfg->o_data_width_dither = o_data_width_dither;
}

static td_void vo_drv_get_intf_dither_sed_and_thr(vdp_dither_cfg *cfg)
{
    const td_u32 sed_y0 = 2147483647; /* 2147483647: typical y0 */
    const td_u32 sed_u0 = 2147482647; /* 2147482647: typical u0 */
    const td_u32 sed_v0 = 2147481647; /* 2147481647: typical v0 */
    const td_u32 sed_w0 = 2147480647; /* 2147480647: typical w0 */
    const td_u32 thr_max = 60000; /* 60000: dither threshold max */
    const td_u32 thr_min = 120; /* 120: dither threshold max */

    cfg->dither_sed.dither_sed_y0 = sed_y0;
    cfg->dither_sed.dither_sed_u0 = sed_u0;
    cfg->dither_sed.dither_sed_v0 = sed_v0;
    cfg->dither_sed.dither_sed_w0 = sed_w0;
    cfg->dither_sed.dither_sed_y1 = sed_y0;
    cfg->dither_sed.dither_sed_u1 = sed_u0;
    cfg->dither_sed.dither_sed_v1 = sed_v0;
    cfg->dither_sed.dither_sed_w1 = sed_w0;
    cfg->dither_sed.dither_sed_y2 = sed_y0;
    cfg->dither_sed.dither_sed_u2 = sed_u0;
    cfg->dither_sed.dither_sed_v2 = sed_v0;
    cfg->dither_sed.dither_sed_w2 = sed_w0;
    cfg->dither_sed.dither_sed_y3 = sed_y0;
    cfg->dither_sed.dither_sed_u3 = sed_u0;
    cfg->dither_sed.dither_sed_v3 = sed_v0;
    cfg->dither_sed.dither_sed_w3 = sed_w0;
    cfg->dither_thr_max = thr_max;
    cfg->dither_thr_min = thr_min;
}

static td_void vo_drv_set_intf_rgb_dither_mode(vdp_dither_cfg *cfg)
{
    /* drv transfer */
    vo_drv_get_intf_dither_in_out_mode(cfg);
    vo_drv_get_intf_dither_sed_and_thr(cfg);
    vo_hal_intf_set_lcd_dither(cfg);
}

static td_void vo_drv_set_intf_rgb_dither(td_u32 in_bit_width, td_u32 out_bit_width)
{
    vdp_dither_cfg dither_cfg = {0};

    /* default setting for rgb24bit */
    dither_cfg.dither_en = 1;
    dither_cfg.dither_round = 1;
    dither_cfg.dither_round_unlim = 1;
    dither_cfg.dither_domain_mode = 1;
    dither_cfg.dither_tap_mode = 1;
    dither_cfg.io_mode = DITHER_IO_MODE_12_8;

    if (in_bit_width == VO_INPUT_BIT_WIDTH_10) {
        if (out_bit_width == VO_OUTPUT_BIT_WIDTH_10) {
            dither_cfg.dither_en = 0;
        } else if (out_bit_width == VO_OUTPUT_BIT_WIDTH_6) { /* for rgb16bit, rgb18bit */
            dither_cfg.dither_round = 0;
            dither_cfg.io_mode = DITHER_IO_MODE_10_6;
        }
    }

    vo_drv_set_intf_rgb_dither_mode(&dither_cfg);
}

td_void vo_drv_set_intf_dither(ot_vo_intf_type intf_type, td_u32 out_bit_width)
{
    if (vo_drv_is_rgb_intf(intf_type)) {
        vo_drv_set_intf_rgb_dither(VO_INPUT_BIT_WIDTH_10, out_bit_width); /* it is always 10bit before dither */
    }
}
#endif


ot_vo_layer vo_drv_get_gfx_layer(ot_vo_dev dev)
{
    hal_disp_layer hal_layer;

    switch (dev) {
        case VO_DEV_DHD0:
            hal_layer = HAL_DISP_LAYER_GFX0;
            break;
        default:
            hal_layer = HAL_DISP_LAYER_BUTT;
            break;
    }

    return hal_layer;
}

#endif /* #if vo_desc("UBOOT_VO") */
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */
