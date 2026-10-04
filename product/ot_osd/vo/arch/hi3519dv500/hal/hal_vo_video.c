/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "hal_vo_video.h"
#include "hal_vo.h"
#include "ot_common_vo_dev.h"
#include "ot_math.h"
#include "drv_vo.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#if vo_desc("UBOOT_VO")

static td_void vo_hal_layer_set_v0_cvfir_cfg(td_u32 vratio, const vdp_v1_cvfir_cfg *cfg,
    const vo_zme_comm_pq_cfg *pq_cfg)
{
    volatile reg_vdp_regs *vo_reg = vo_hal_get_reg();
    volatile reg_v0_cvfir_vinfo v0_cvfir_vinfo;
    volatile reg_v0_cvfir_vsp v0_cvfir_vsp;
    volatile reg_v0_cvfir_voffset v0_cvfir_voffset;
    volatile reg_v0_cvfir_vboffset v0_cvfir_vboffset;
    volatile td_ulong addr_reg;

    addr_reg = (td_ulong)(td_uintptr_t)&(vo_reg->v0_cvfir_vinfo.u32);
    v0_cvfir_vinfo.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    v0_cvfir_vinfo.bits.out_height = (td_u32)(cfg->out_height - 1);
    v0_cvfir_vinfo.bits.out_fmt = cfg->out_fmt;
    v0_cvfir_vinfo.bits.out_pro = cfg->out_pro;
    v0_cvfir_vinfo.bits.vzme_ck_gt_en = cfg->ck_gt_en;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, v0_cvfir_vinfo.u32);

    addr_reg = (td_ulong)(td_uintptr_t)&(vo_reg->v0_cvfir_vsp.u32);
    v0_cvfir_vsp.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    v0_cvfir_vsp.bits.vratio = vratio;
    v0_cvfir_vsp.bits.cvfir_mode = cfg->cvfir_mode;
    v0_cvfir_vsp.bits.cvmid_en = cfg->cvmid_en;
    v0_cvfir_vsp.bits.cvfir_en = cfg->cvfir_en;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, v0_cvfir_vsp.u32);

    addr_reg = (td_ulong)(td_uintptr_t)&(vo_reg->v0_cvfir_voffset.u32);
    v0_cvfir_voffset.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    v0_cvfir_voffset.bits.vchroma_offset = pq_cfg->vchroma_offset;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, v0_cvfir_voffset.u32);

    addr_reg = (td_ulong)(td_uintptr_t)&(vo_reg->v0_cvfir_vboffset.u32);
    v0_cvfir_vboffset.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    v0_cvfir_vboffset.bits.vbchroma_offset = pq_cfg->vbchroma_offset;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, v0_cvfir_vboffset.u32);
}

static td_void vo_hal_layer_set_v0_cvfir_coef(const cvfir_coef *coef)
{
    volatile reg_vdp_regs *vo_reg = vo_hal_get_reg();
    volatile reg_v0_cvfir_vcoef0 v0_cvfir_vcoef0;
    volatile reg_v0_cvfir_vcoef1 v0_cvfir_vcoef1;
    volatile reg_v0_cvfir_vcoef2 v0_cvfir_vcoef2;
    volatile td_ulong addr_reg;

    addr_reg = (td_ulong)(td_uintptr_t)&(vo_reg->v0_cvfir_vcoef0.u32);
    v0_cvfir_vcoef0.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    v0_cvfir_vcoef0.bits.vccoef00 = coef->vccoef00;
    v0_cvfir_vcoef0.bits.vccoef01 = coef->vccoef01;
    v0_cvfir_vcoef0.bits.vccoef02 = coef->vccoef02;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, v0_cvfir_vcoef0.u32);

    addr_reg = (td_ulong)(td_uintptr_t)&(vo_reg->v0_cvfir_vcoef1.u32);
    v0_cvfir_vcoef1.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    v0_cvfir_vcoef1.bits.vccoef03 = coef->vccoef03;
    v0_cvfir_vcoef1.bits.vccoef10 = coef->vccoef10;
    v0_cvfir_vcoef1.bits.vccoef11 = coef->vccoef11;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, v0_cvfir_vcoef1.u32);

    addr_reg = (td_ulong)(td_uintptr_t)&(vo_reg->v0_cvfir_vcoef2.u32);
    v0_cvfir_vcoef2.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    v0_cvfir_vcoef2.bits.vccoef12 = coef->vccoef12;
    v0_cvfir_vcoef2.bits.vccoef13 = coef->vccoef13;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, v0_cvfir_vcoef2.u32);
}

td_void vo_hal_layer_set_cvfir_cfg(hal_disp_layer layer, td_u32 vratio, const vdp_v1_cvfir_cfg *cfg,
    const vo_zme_comm_pq_cfg *pq_cfg)
{
    if (layer == HAL_DISP_LAYER_VHD0) {
        vo_hal_layer_set_v0_cvfir_cfg(vratio, cfg, pq_cfg);
    }
}

td_void vo_hal_layer_set_cvfir_coef(hal_disp_layer layer, const cvfir_coef *coef)
{
    if (layer == HAL_DISP_LAYER_VHD0) {
        vo_hal_layer_set_v0_cvfir_coef(coef);
    }
}

td_void hal_cbm_set_cbm_attr(hal_disp_layer layer, ot_vo_dev dev)
{
    ot_unused(layer);
    ot_unused(dev);
}

td_void hal_cbm_set_cbm_bkg(hal_cbmmix mixer, const hal_disp_bkcolor *bkg)
{
    volatile reg_vdp_regs *vo_reg = vo_hal_get_reg();
    volatile reg_cbm_bkg1 cbm_bkg1;
    volatile reg_cbm_bkg2 cbm_bkg2;

    if (mixer == HAL_CBMMIX1) {
        cbm_bkg1.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)&(vo_reg->cbm_bkg1.u32));
        cbm_bkg1.bits.cbm_bkgy1 = (bkg->bkg_y);
        cbm_bkg1.bits.cbm_bkgcb1 = (bkg->bkg_cb);
        cbm_bkg1.bits.cbm_bkgcr1 = (bkg->bkg_cr);
        hal_write_reg((td_u32 *)(td_uintptr_t)&(vo_reg->cbm_bkg1.u32), cbm_bkg1.u32);
    } else if (mixer == HAL_CBMMIX2) {
        cbm_bkg2.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)&(vo_reg->cbm_bkg2.u32));
        cbm_bkg2.bits.cbm_bkgy2 = (bkg->bkg_y);
        cbm_bkg2.bits.cbm_bkgcb2 = (bkg->bkg_cb);
        cbm_bkg2.bits.cbm_bkgcr2 = (bkg->bkg_cr);
        hal_write_reg((td_u32 *)(td_uintptr_t)&(vo_reg->cbm_bkg2.u32), cbm_bkg2.u32);
    }
}

static td_bool hal_cbm_get_cbm1_mixer_layer_id(ot_vo_layer layer, td_u8 *layer_id)
{
    const vo_hal_cbm_mixer cbm1_mixer[] = {
        { OT_VO_LAYER_V0, 0x1 },
        { OT_VO_LAYER_G0, 0x2 },
        { VO_LAYER_BUTT,  0x0 }
    };

    td_u32 cbm1_len = sizeof(cbm1_mixer) / sizeof(vo_hal_cbm_mixer);
    return hal_cbm_get_cbm_mixer_layer_id(cbm1_mixer, cbm1_len, layer, layer_id);
}

static td_void hal_cbm_set_cbm1_mixer_prio(ot_vo_layer layer, td_u8 prio)
{
    volatile reg_vdp_regs *vo_reg = vo_hal_get_reg();
    volatile reg_cbm_mix1 cbm_mix1;
    td_u8 layer_id = 0;

    /* check layer availability */
    if (hal_cbm_get_cbm1_mixer_layer_id(layer, &layer_id) != TD_TRUE) {
        return;
    }

     /* set mixer prio */
    cbm_mix1.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)&(vo_reg->cbm_mix1.u32));

    switch (prio) {
        case 0:  /* 0: prio 0 */
            cbm_mix1.bits.mixer_prio0 = layer_id;
            break;

        case 1:  /* 1: prio 1 */
            cbm_mix1.bits.mixer_prio1 = layer_id;
            break;

        case 2:  /* 2: prio 2 */
            cbm_mix1.bits.mixer_prio2 = layer_id;
            break;

        case 3:  /* 3: prio 3 */
            cbm_mix1.bits.mixer_prio3 = layer_id;
            break;

        default:
            vo_err_trace("error priority id %d found\n", prio);
            return;
    }

    hal_write_reg((td_u32 *)(td_uintptr_t)&(vo_reg->cbm_mix1.u32), cbm_mix1.u32);
}

td_void hal_cbm_set_cbm_mixer_prio(ot_vo_layer layer, td_u8 prio, td_u8 mixer_id)
{
    if (mixer_id == HAL_CBMMIX1) {
        hal_cbm_set_cbm1_mixer_prio(layer, prio);
    }
}

#endif /* #if vo_desc("UBOOT_VO") */
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */
