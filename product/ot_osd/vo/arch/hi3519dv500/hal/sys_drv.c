/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "sys_hal.h"
#include "sys_ext.h"
#include "sys_drv.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

td_s32 sys_drv_drv_ioctrl(const ot_mpp_chn *mpp_chn, td_u32 func_id, const td_void *io_args)
{
    td_s32 dev;

    dev = mpp_chn->dev_id;
    switch (func_id) {
        case SYS_VO_BUS_RESET_SEL: {
            td_bool *reset = NULL;
            reset = (td_bool *)io_args;
            sys_hal_vo_bus_reset_sel(*reset);
            break;
        }

        case SYS_VO_CFG_CLK_EN: {
            td_bool *clk_en = NULL;
            clk_en = (td_bool *)io_args;
            sys_hal_vo_cfg_clk_en(*clk_en);
            break;
        }
        case SYS_VO_APB_CLK_EN: {
            td_bool *clk_en = NULL;
            clk_en = (td_bool *)io_args;
            sys_hal_vo_apb_clk_en(*clk_en);
            break;
        }

        case SYS_VO_BUS_CLK_EN: {
            td_bool *clk_en = NULL;
            clk_en = (td_bool *)io_args;
            sys_hal_vo_bus_clk_en(*clk_en);
            break;
        }

        case SYS_VO_HD_CLKOUT_PHASIC_REVERSE_EN: {
            td_bool *clk_reverse_en = NULL;
            clk_reverse_en = (td_bool *)io_args;
            sys_hal_vo_out_hd_phase_ctrl(dev, *clk_reverse_en);
            break;
        }

        case SYS_VO_DEV_DIV_MOD: {
            td_u32 *clk_sel = NULL;
            clk_sel = (td_u32 *)io_args;
            sys_hal_vo_dev_div_mode(dev, *clk_sel);
            break;
        }

        case SYS_VO_PPC_CFG_CLK_SEL: {
            td_u32 *clk_sel = NULL;
            clk_sel = (td_u32 *)io_args;
            sys_hal_vo_ppc_cfg_clk_sel(dev, *clk_sel);
            break;
        }

        case SYS_VO_PPC_CLK_SEL: {
            td_u32 *clk_sel = NULL;
            clk_sel = (td_u32 *)io_args;
            sys_hal_vo_ppc_clk_sel(dev, *clk_sel);
            break;
        }

        case SYS_VO_PPC_SC_CLK_SEL: {
            td_u32 *clk_sel = NULL;
            clk_sel = (td_u32 *)io_args;
            sys_hal_vo_ppc_sc_clk_sel(dev, *clk_sel);
            break;
        }

        case SYS_VO_HD_CLK_SEL: {
            td_u32 *clk_sel = NULL;
            clk_sel = (td_u32 *)io_args;
            sys_hal_vo_hd_clk_sel(dev, *clk_sel);
            break;
        }

        case SYS_VO_CORE_CLK_EN: {
            td_bool *clk_en = NULL;
            clk_en = (td_bool *)io_args;
            sys_hal_vo_core_clk_en(dev, *clk_en);
            break;
        }

        case SYS_VO_DEV_CLK_EN: {
            td_bool *clk_en = NULL;
            clk_en = (td_bool *)io_args;
            sys_hal_vo_dev_clk_en(dev, *clk_en);
            break;
        }

        case SYS_VO_SD_DATE_CLK_EN: {
            td_bool *clk_en = NULL;
            clk_en = (td_bool *)io_args;
            sys_hal_vo_sd_date_clk_en(dev, *clk_en);
            break;
        }

        case SYS_VO_LCD_CLK_EN: {
            td_bool *clk_en = NULL;
            clk_en = (td_bool *)io_args;
            sys_hal_vo_lcd_clk_en(dev, *clk_en);
            break;
        }

        case SYS_VO_LCD_MCLK_DIV: {
            td_u32 *div_mode = NULL;
            div_mode = (td_u32 *)io_args;
            sys_hal_vo_lcd_mclk_div(dev, *div_mode);
            break;
        }

        case SYS_VO_BT1120_CLK_EN: {
            td_bool *bt1120_clk_en = NULL;
            bt1120_clk_en = (td_bool *)io_args;
            sys_hal_vo_bt_clk_en(dev, *bt1120_clk_en);
            break;
        }

        case SYS_VO_BT1120_CLK_SEL: {
            td_u32 *bt1120_clk_ch_sel = NULL;
            bt1120_clk_ch_sel = (td_u32 *)io_args;
            sys_hal_vo_bt_clk_sel(dev, *bt1120_clk_ch_sel);
            break;
        }

        case SYS_VO_DAC_CLK_PHASIC_REVERSE_EN: {
            td_u32 *rev = NULL;
            rev = (td_u32 *)io_args;
            sys_hal_vo_vdac_phase_ctrl(dev, *rev);
            break;
        }

        case SYS_VO_DEV_DAC_EN: {
            td_bool *clk_en = NULL;
            clk_en = (td_bool *)io_args;
            sys_hal_vo_vdac_clk_en(dev, *clk_en);
            break;
        }

        case SYS_VO_HDMI_CLK_EN: {
            td_bool *hdmi_clk_en = NULL;
            hdmi_clk_en = (td_bool *)io_args;
            sys_hal_vo_hdmi_clk_en(dev, *hdmi_clk_en);
            break;
        }

        case SYS_VO_HDMI_CLK_SEL: {
            td_bool *clk_sel = NULL;
            clk_sel = (td_bool *)io_args;
            sys_hal_vo_hdmi_clk_sel(dev, *clk_sel);
            break;
        }

        case SYS_VO_MIPI_CLK_EN: {
            td_bool *mipi_clk_en = NULL;
            mipi_clk_en = (td_bool *)io_args;
            sys_hal_vo_mipi_clk_en(dev, *mipi_clk_en);
            break;
        }

        case SYS_VO_MIPI_CLK_SEL: {
            td_bool *clk_sel = NULL;
            clk_sel = (td_bool *)io_args;
            sys_hal_vo_mipi_clk_sel(dev, *clk_sel);
            break;
        }

        case SYS_VO_PLL_POSTDIV2_SET: {
            td_s32 pll;
            td_u32 bits_set;
            pll = mpp_chn->dev_id;
            bits_set = *(td_u32 *)io_args;
            sys_hal_vo_set_pll_postdiv2(pll, bits_set);
            break;
        }

        case SYS_VO_PLL_POSTDIV1_SET: {
            td_s32 pll;
            td_u32 bits_set;
            pll = mpp_chn->dev_id;
            bits_set = *(td_u32 *)io_args;
            sys_hal_vo_set_pll_postdiv1(pll, bits_set);
            break;
        }

        case SYS_VO_PLL_FRAC_SET: {
            td_s32 pll;
            td_u32 bits_set;
            pll = mpp_chn->dev_id;
            bits_set = *(td_u32 *)io_args;
            sys_hal_vo_set_pll_frac(pll, bits_set);
            break;
        }

        case SYS_VO_PLL_REFDIV_SET: {
            td_s32 pll;
            td_u32 bits_set;
            pll = mpp_chn->dev_id;
            bits_set = *(td_u32 *)io_args;
            sys_hal_vo_set_pll_refdiv(pll, bits_set);
            break;
        }

        case SYS_VO_PLL_FBDIV_SET: {
            td_s32 pll;
            td_u32 bits_set;
            pll = mpp_chn->dev_id;
            bits_set = *(td_u32 *)io_args;
            sys_hal_vo_set_pll_fbdiv(pll, bits_set);
            break;
        }

        case SYS_VO_PLL_POWER_CTRL: {
            td_s32 pll;
            td_bool *power_down = TD_NULL;
            pll = mpp_chn->dev_id;
            power_down = (td_bool *)io_args;
            sys_hal_vo_set_pll_power_ctrl(pll, *power_down);
            break;
        }

        default: {
            return TD_FAILURE;
        }
    }
    return TD_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
        }
#endif
#endif /* end of #ifdef __cplusplus */
