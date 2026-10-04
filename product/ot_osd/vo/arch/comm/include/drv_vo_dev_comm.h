/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#ifndef DRV_VO_DEV_COMM_H
#define DRV_VO_DEV_COMM_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#if vo_desc("UBOOT_VO")
typedef enum {
    VO_HD_HW_DEV = 0,  /* HD dev */
    VO_SD_HW_DEV,  /* SD dev */
    VO_UHD_HW_DEV,  /* UHD dev */
    VO_CAS_DEV,  /* cascade dev */
    VO_VIRT_DEV,  /* virtual dev */
    VO_DEV_TYPE_BUTT,
} vo_dev_type;

#define VO_DEV_REG_UP_MS_TIMEOUT 50
#define VO_KHZ_CLK_UNIT 1000

#define VO_MIN_DIV_MODE 1        /* Minimum clock frequency divider */
#define VO_MAX_DIV_MODE 4        /* Maximum clock frequency divider */
#define VO_INTF_HDMI_DIV_MODE 1  /* Clock divider of the HDMI interface. The clock divider of the BT.1120 and VGA \
                                    interfaces must be the same as that of the HDMI interface. */
#define VO_INTF_CVBS_DIV_MODE 4  /* Clock divider of the CVBS interface */
#define VO_INTF_BT656_DIV_MODE 2 /* Clock divider of the BT.656 interface */
#define VO_INTF_RGB_DIV_MODE_1 1 /* Clock divider of the RGB_16/18/24-bit parallel interface */
#define VO_INTF_RGB_DIV_MODE_3 3 /* RGB_6/8bit interface clock divider */
#define VO_INTF_RGB_DIV_MODE_4 4 /* RGB_6/8bit interface clock divider */

#define VO_MIN_PRE_DIV_MODE 1
#define VO_MAX_PRE_DIV_MODE 32
#define VO_INTF_NO_HDMI_PRE_DIV_MODE 1 /* The front-screen split ratio can only be set to 1 when the non-HDMI \
                                          interface is used. */
#define VO_INTF_HDMI_PRE_DIV_MODE 1    /* The front-screen split ratio can only be set to 1 for the HDMI interface \
                                           (the front-screen divider is not supported). */

#define VO_MIN_PLL_REF_DIV 1 /* Minimum ref_div value of the PLL */
#define VO_DEF_PLL_REF_DIV 1 /* Default ref_div value of the PLL */
#define VO_MAX_PLL_REF_DIV 4 /* Maximum ref_div value of the PLL */

#define VO_MAX_USER_SYNC_INTFB 255
#define VO_MAX_USER_SYNC_VACT  4096
#define VO_MIN_USER_SYNC_VACT  100
#define VO_MAX_USER_SYNC_VBB   256
#define VO_MIN_USER_SYNC_VBB   1
#define VO_MAX_USER_SYNC_VFB   256
#define VO_MIN_USER_SYNC_VFB   1
#define VO_MAX_USER_SYNC_HACT  4096
#define VO_MIN_USER_SYNC_HACT  1
#define VO_MAX_USER_SYNC_HBB   65535
#define VO_MIN_USER_SYNC_HBB   1
#define VO_MAX_USER_SYNC_HFB   65535
#define VO_MIN_USER_SYNC_HFB   1
#define VO_MAX_USER_SYNC_BVACT 4096
#define VO_MAX_USER_SYNC_BVBB  256
#define VO_MAX_USER_SYNC_BVFB  256
#define VO_MAX_USER_SYNC_HPW   65535
#define VO_MIN_USER_SYNC_HPW   1
#define VO_MAX_USER_SYNC_VPW   256
#define VO_MIN_USER_SYNC_VPW   1

#if vo_desc("dev drv api")
td_u32 vo_drv_get_dev_bg_color(ot_vo_dev dev);
ot_vo_intf_type vo_drv_get_dev_intf_type(ot_vo_dev dev);
ot_vo_intf_sync vo_drv_get_dev_intf_sync(ot_vo_dev dev);
td_void vou_drv_set_disp_max_size(ot_vo_dev dev, td_u32 max_width, td_u32 max_height);
#endif

#if vo_desc("dev")
td_void vo_drv_get_sync_info(ot_vo_dev dev, hal_disp_syncinfo *sync_info);
td_void vo_drv_set_sync_info(ot_vo_dev dev, const hal_disp_syncinfo *sync_info);
td_bool vo_drv_is_progressive(ot_vo_dev dev);
td_bool vou_drv_get_dev_enable(ot_vo_dev dev);
td_s32 vo_drv_check_dev_pll(ot_vo_dev dev, const ot_vo_manual_user_sync_info *sync_info);
td_s32 vou_drv_check_dev_sync(ot_vo_dev dev, ot_vo_intf_type intf_type, ot_vo_intf_sync intf_sync);
td_s32 vou_drv_check_dev_pub_attr(ot_vo_dev dev, const ot_vo_pub_attr *pub_attr);
td_void vo_drv_get_dev_reg_up(ot_vo_dev dev);
#endif

#if vo_desc("dev layer prio")
td_void vou_drv_set_layer_priority(ot_vo_dev dev, ot_vo_layer layer, td_u32 priority);
#endif

#if vo_desc("dev intf")
td_void vou_drv_intf_csc_config(ot_vo_intf_type intf, const ot_vo_csc *csc);
td_void vo_drv_set_vga_param(ot_vo_dev dev, const ot_vo_vga_param *vga_param);
td_void vo_drv_set_hdmi_param(ot_vo_dev dev, const ot_vo_hdmi_param *hdmi_param);
td_void vo_drv_set_rgb_param(ot_vo_dev dev, const ot_vo_rgb_param *rgb_param);
td_void vo_drv_set_bt_param(ot_vo_dev dev, const ot_vo_bt_param *bt_param);
td_void vo_drv_set_mipi_param(ot_vo_dev dev, const ot_vo_mipi_param *mipi_param);
td_void vo_drv_set_dev_mpp_chn(ot_vo_dev dev, ot_mpp_chn *mpp_chn);
td_bool vo_drv_is_hdmi_intf(ot_vo_intf_type intf_type);
td_bool vo_drv_is_hdmi1_intf(ot_vo_intf_type intf_type);
td_bool vo_drv_is_rgb_intf(ot_vo_intf_type intf_type);
td_bool vo_drv_is_vga_intf(ot_vo_intf_type intf_type);
td_bool vo_drv_is_cvbs_intf(ot_vo_intf_type intf_type);
td_bool vo_drv_is_mipi_intf(ot_vo_intf_type intf_type);
td_bool vo_drv_is_bt_intf(ot_vo_intf_type intf_type);
td_bool vo_drv_is_bt1120_intf(ot_vo_intf_type intf_type);
td_bool vo_drv_is_bt656_intf(ot_vo_intf_type intf_type);
td_bool vo_drv_is_rgb_serial_intf(ot_vo_intf_type intf_type);
td_bool vo_drv_is_rgb_parallel_intf(ot_vo_intf_type intf_type);
td_s32 vo_drv_check_intf_share(ot_vo_dev dev, ot_vo_intf_type intf_type);
#endif

#if vo_desc("dev interrupt")
td_void vo_drv_int_set_mode(ot_vo_dev dev, vo_int_mode int_mode);
#endif

#if vo_desc("check id")
td_s32 vo_drv_check_dev_id_pub(ot_vo_dev dev);
td_s32 vo_drv_check_layer_id_pub(ot_vo_layer layer);
#endif

td_void vo_drv_set_pub_attr(ot_vo_dev dev, const ot_vo_pub_attr *pub_attr);
td_void vo_drv_enable(ot_vo_dev dev);
td_void vo_drv_disable(ot_vo_dev dev);
td_u32 vou_drv_get_dev_layer_num(ot_vo_dev dev);
td_void vou_drv_get_dev_layer(ot_vo_dev dev, ot_vo_layer *layer, td_u32 vo_layer_num);

td_bool vo_drv_is_phy_dev_uhd(ot_vo_dev dev);
td_bool vo_drv_is_phy_dev_hd(ot_vo_dev dev);
td_bool vo_drv_is_phy_dev_sd(ot_vo_dev dev);

#endif /* #if vo_desc("UBOOT_VO") */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* end of DRV_VO_DEV_COMM_H */
