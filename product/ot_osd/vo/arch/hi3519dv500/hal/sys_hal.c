/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <net.h>
#include <asm/io.h>

#include "ot_type.h"
#include "ot_board.h"
#include "sys_hal.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#define io_crg_address(x) (x)
#define io_crg_pll_address(x) (x)

static td_void vo_reg_set_bit(unsigned long value, unsigned long offset, unsigned long addr)
{
    unsigned long t, mask;

    mask = 1 << offset;
    t = readl((const volatile void *)(td_uintptr_t)addr);
    t &= ~mask;
    t |= (value << offset) & mask;
    writel(t, (volatile void *)(td_uintptr_t)addr);
}

static td_void vo_reg_write32(unsigned long value, unsigned long mask, unsigned long addr)
{
    unsigned long t;

    t = readl((const volatile void *)(td_uintptr_t)addr);
    t &= ~mask;
    t |= value & mask;
    writel(t, (volatile void *)(td_uintptr_t)addr);
}

td_s32 sys_hal_vo_bus_reset_sel(td_bool reset)
{
    const td_u32 tmp = (reset == TD_TRUE) ? 1 : 0;
    const td_u32 bit = 0;

    vo_reg_set_bit(tmp, bit, io_crg_address(CRG_PERCTL8336_ADDR));
    return TD_SUCCESS;
}

td_s32 sys_hal_vo_apb_clk_en(td_bool clk_en)
{
    const td_u32 tmp = (clk_en == TD_TRUE) ? 1 : 0;
    const td_u32 bit = 8;

    vo_reg_set_bit(tmp, bit, io_crg_address(CRG_PERCTL8336_ADDR));
    return TD_SUCCESS;
}

td_s32 sys_hal_vo_bus_clk_en(td_bool clk_en)
{
    const td_u32 tmp = (clk_en == TD_TRUE) ? 1 : 0;
    const td_u32 bit = 9;

    vo_reg_set_bit(tmp, bit, io_crg_address(CRG_PERCTL8336_ADDR));
    return TD_SUCCESS;
}

td_s32 sys_hal_vo_out_hd_phase_ctrl(td_s32 vo_dev, td_bool reverse)
{
    const td_u32 tmp = (reverse == TD_TRUE) ? 1 : 0;
    const td_u32 bit = 20;

    if (vo_dev != 0) {
        return TD_FAILURE;
    }

    vo_reg_set_bit(tmp, bit, io_crg_address(CRG_PERCTL8338_ADDR));
    return TD_SUCCESS;
}

td_s32 sys_hal_vo_hd0_div_mode(td_s32 dev, td_u32 hd0_div_mod)
{
    const td_u32 bit = 24;
    const td_u32 mask = 0x3;

    if (dev != 0) {
        return TD_FAILURE;
    }

    vo_reg_write32(hd0_div_mod << bit, mask << bit, io_crg_address(CRG_PERCTL8340_ADDR));
    return TD_SUCCESS;
}

td_s32 sys_hal_vo_ppc_cfg_clk_sel(td_s32 dev, td_u32 clk_sel)
{
    const td_u32 bit = 15;

    if (dev != 0) {
        return TD_FAILURE;
    }

    vo_reg_set_bit(clk_sel, bit, io_crg_address(CRG_PERCTL8340_ADDR));
    return TD_SUCCESS;
}

td_s32 sys_hal_vo_ppc_clk_sel(td_s32 dev, td_u32 clk_sel)
{
    const td_u32 bit = 12;
    const td_u32 mask = 0x7;

    if (dev != 0) {
        return TD_FAILURE;
    }

    vo_reg_write32(clk_sel << bit, mask << bit, io_crg_address(CRG_PERCTL8341_ADDR));
    return TD_SUCCESS;
}

td_s32 sys_hal_vo_ppc_sc_clk_sel(td_s32 dev, td_u32 clk_sel)
{
    const td_u32 bit = 13;

    if (dev != 0) {
        return TD_FAILURE;
    }

    vo_reg_set_bit(clk_sel, bit, io_crg_address(CRG_PERCTL8340_ADDR));
    return TD_SUCCESS;
}

td_s32 sys_hal_vo_hd_clk_sel(td_s32 dev, td_u32 clk_sel)
{
    const td_u32 bit = 12;
    const td_u32 mask = 0xf;

    if (dev == 0) {
        vo_reg_write32(clk_sel << bit, mask << bit, io_crg_address(CRG_PERCTL8340_ADDR));
    } else {
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

td_s32 sys_hal_vo_cfg_clk_en(td_bool clk_en)
{
    const td_u32 tmp = (clk_en == TD_TRUE) ? 1 : 0;
    const td_u32 bit = 6;

    vo_reg_set_bit(tmp, bit, io_crg_address(CRG_PERCTL8340_ADDR));
    return TD_SUCCESS;
}

td_s32 sys_hal_vo_core_clk_en(td_s32 dev, td_bool clk_en)
{
    const td_u32 bit = 5;
    const td_u32 tmp = (clk_en == TD_TRUE) ? 1 : 0;

    if ((dev == 0) || (dev == 1)) {
        vo_reg_set_bit(tmp, bit, io_crg_address(CRG_PERCTL8340_ADDR));
        return TD_SUCCESS;
    }
    return TD_FAILURE;
}

td_s32 sys_hal_vo_dev_clk_en(td_s32 vo_dev, td_bool clk_en)
{
    const td_u32 bit = 4;
    const td_u32 tmp = (clk_en == TD_TRUE) ? 1 : 0;

    if (vo_dev == 0) {
        vo_reg_set_bit(tmp, bit, io_crg_address(CRG_PERCTL8340_ADDR));
    } else {
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

td_s32 sys_hal_vo_dev_div_mode(td_s32 vo_dev, td_u32 div_mod)
{
    const td_u32 bit = 24;
    const td_u32 mask = 0x3;

    if (vo_dev == 0) {
        vo_reg_write32(div_mod << bit, mask << bit, io_crg_address(CRG_PERCTL8340_ADDR));
    } else {
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

td_s32 sys_hal_vo_sd_date_clk_en(td_s32 vo_dev, td_bool clk_en)
{
    ot_unused(vo_dev);
    ot_unused(clk_en);

    return TD_SUCCESS;
}

td_s32 sys_hal_vo_lcd_clk_en(td_s32 vo_dev, td_bool clk_en)
{
    const td_u32 tmp = (clk_en == TD_TRUE) ? 1 : 0;
    const td_u32 bit = 27;
    ot_unused(vo_dev);

    vo_reg_set_bit(tmp, bit, io_crg_address(CRG_PERCTL8346_ADDR));

    return TD_SUCCESS;
}

td_s32 sys_hal_vo_lcd_mclk_div(td_s32 vo_dev, td_u32 mclk_div)
{
    const td_u32 mask = 0x7ffffff;
    ot_unused(vo_dev);

    vo_reg_write32(mclk_div, mask, io_crg_address(CRG_PERCTL8346_ADDR));

    return TD_SUCCESS;
}

td_s32 sys_hal_vo_bt_clk_sel(td_s32 vo_dev, td_u32 bt_clk_ch_sel)
{
    ot_unused(vo_dev);
    ot_unused(bt_clk_ch_sel);

    return TD_SUCCESS;
}

td_s32 sys_hal_vo_bt_clk_en(td_s32 vo_dev, td_bool bt_clk_en)
{
    const td_u32 bit_bt = 5;
    const td_u32 bit_bt_bp = 4;
    const td_u32 tmp = (bt_clk_en == TD_TRUE) ? 1 : 0;
    ot_unused(vo_dev);

    vo_reg_set_bit(tmp, bit_bt, io_crg_address(CRG_PERCTL8348_ADDR));
    vo_reg_set_bit(tmp, bit_bt_bp, io_crg_address(CRG_PERCTL8348_ADDR));
    return TD_SUCCESS;
}

td_s32 sys_hal_vo_vdac_phase_ctrl(td_s32 vo_dev, td_bool clk_reverse)
{
    ot_unused(vo_dev);
    ot_unused(clk_reverse);

    return TD_SUCCESS;
}

td_s32 sys_hal_vo_vdac_clk_en(td_s32 vo_dev, td_bool clk_en)
{
    ot_unused(vo_dev);
    ot_unused(clk_en);

    return TD_SUCCESS;
}

td_s32 sys_hal_vo_hdmi_clk_en(td_s32 vo_dev, td_bool hdmi_clk_en)
{
    ot_unused(vo_dev);
    ot_unused(hdmi_clk_en);

    return TD_SUCCESS;
}

td_s32 sys_hal_vo_hdmi_clk_sel(td_s32 vo_dev, td_u32 clk_ch_sel)
{
    ot_unused(vo_dev);
    ot_unused(clk_ch_sel);

    return TD_FAILURE;
}

td_s32 sys_hal_vo_mipi_clk_en(td_s32 vo_dev, td_bool mipi_clk_en)
{
    const td_u32 bit_pixel = 4;
    const td_u32 bit_vdp = 5;
    const td_u32 tmp = (mipi_clk_en == TD_TRUE) ? 1 : 0;
    ot_unused(vo_dev);

    vo_reg_set_bit(tmp, bit_pixel, io_crg_address(CRG_PERCTL8352_ADDR));
    vo_reg_set_bit(tmp, bit_vdp, io_crg_address(CRG_PERCTL8352_ADDR));
    return TD_SUCCESS;
}

td_s32 sys_hal_vo_mipi_clk_sel(td_s32 vo_dev, td_u32 clk_ch_sel)
{
    ot_unused(vo_dev);
    ot_unused(clk_ch_sel);

    return TD_SUCCESS;
}

td_s32 sys_hal_vo_set_pll_postdiv2(td_s32 pll, td_u32 bits_set)
{
    const td_u32 bit = 28;
    const td_u32 mask = 0x7;

    if (pll != 0) {
        return TD_FAILURE;
    }

    vo_reg_write32(bits_set << bit, mask << bit, io_crg_pll_address(CRG_PERCTL32_ADDR));

    return TD_SUCCESS;
}

td_s32 sys_hal_vo_set_pll_postdiv1(td_s32 pll, td_u32 bits_set)
{
    const td_u32 bit = 24;
    const td_u32 mask = 0x7;

    if (pll != 0) {
        return TD_FAILURE;
    }

    vo_reg_write32(bits_set << bit, mask << bit, io_crg_pll_address(CRG_PERCTL32_ADDR));

    return TD_SUCCESS;
}

td_s32 sys_hal_vo_set_pll_frac(td_s32 pll, td_u32 bits_set)
{
    const td_u32 bit = 0;
    const td_u32 bit_dsmpd = 25;
    const td_u32 mask = 0xffffff;

    if (pll != 0) {
        return TD_FAILURE;
    }

    vo_reg_write32(bits_set << bit, mask << bit, io_crg_pll_address(CRG_PERCTL32_ADDR));

    if (bits_set == 0) {
        vo_reg_set_bit(1, bit_dsmpd, io_crg_pll_address(CRG_PERCTL33_ADDR));
    } else {
        vo_reg_set_bit(0, bit_dsmpd, io_crg_pll_address(CRG_PERCTL33_ADDR));
    }

    return TD_SUCCESS;
}

td_s32 sys_hal_vo_set_pll_refdiv(td_s32 pll, td_u32 bits_set)
{
    const td_u32 bit = 12;
    const td_u32 mask = 0x3f;

    if (pll != 0) {
        return TD_FAILURE;
    }

    vo_reg_write32(bits_set << bit, mask << bit, io_crg_pll_address(CRG_PERCTL33_ADDR));

    return TD_SUCCESS;
}

td_s32 sys_hal_vo_set_pll_fbdiv(td_s32 pll, td_u32 bits_set)
{
    const td_u32 bit = 0;
    const td_u32 mask = 0xfff;

    if (pll != 0) {
        return TD_FAILURE;
    }

    vo_reg_write32(bits_set << bit, mask << bit, io_crg_pll_address(CRG_PERCTL33_ADDR));

    return TD_SUCCESS;
}

td_s32 sys_hal_vo_set_pll_power_ctrl(td_s32 pll, td_bool power_down)
{
    const td_u32 bit = 20;
    const td_u32 tmp = (power_down == TD_TRUE) ? 1 : 0;

    if (pll != 0) {
        return TD_FAILURE;
    }

    vo_reg_set_bit(tmp, bit, io_crg_pll_address(CRG_PERCTL33_ADDR));

    return TD_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */
