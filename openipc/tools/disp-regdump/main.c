/*
 * disp-regdump: the register program SigmaStar's own boot logo code
 * (boot_621_release common/cmd_bootlogo.c, the HDMI path) writes for one
 * output timing, by running the vendor display and HDMI TX sources against
 * the recorder. Sections are marked "# name" in the log.
 *
 *   regdump <1080p60|1080p50|720p60|720p50|1024x768|1280x1024|...>
 *
 * SPDX-License-Identifier: MIT
 */
#include <common.h>
#include "mhal_common.h"
#include "mhal_disp_datatype.h"
#include "mhal_disp.h"
#include "mhal_hdmitx_datatype.h"
#include "mhal_hdmitx.h"

extern FILE *rec_out;

/* cmd_bootlogo.c's stTimingTable: hsync w/bp, vsync w/bp, hstart, vstart,
 * hactive, vactive, htotal, vtotal, dclk MHz */
static const struct tm {
    const char *name;
    MHAL_DISP_DeviceTiming_e disp;
    MhaHdmitxTimingResType_e hdmi;
    u16 hsw, hbp, vsw, vbp, hst, vst, ha, va, ht, vt, dclk;
} tms[] = {
    { "1080p60", E_MHAL_DISP_OUTPUT_1080P60, E_MHAL_HDMITX_RES_1920X1080P_60HZ,
      44, 148, 5, 36, 192, 41, 1920, 1080, 2200, 1125, 148 },
    { "1080p50", E_MHAL_DISP_OUTPUT_1080P50, E_MHAL_HDMITX_RES_1920X1080P_50HZ,
      44, 148, 5, 36, 192, 41, 1920, 1080, 2640, 1125, 148 },
    { "720p50", E_MHAL_DISP_OUTPUT_720P50, E_MHAL_HDMITX_RES_1280X720P_50HZ,
      40, 220, 5, 20, 260, 25, 1280, 720, 1980, 750, 74 },
    { "720p60", E_MHAL_DISP_OUTPUT_720P60, E_MHAL_HDMITX_RES_1280X720P_60HZ,
      40, 220, 5, 20, 260, 25, 1280, 720, 1650, 750, 74 },
    { "1024x768", E_MHAL_DISP_OUTPUT_1024x768_60, E_MHAL_HDMITX_RES_1024X768P_60HZ,
      136, 160, 6, 29, 296, 35, 1024, 768, 1344, 806, 65 },
    { "1280x1024", E_MHAL_DISP_OUTPUT_1280x1024_60, E_MHAL_HDMITX_RES_1280X1024P_60HZ,
      112, 248, 3, 38, 360, 41, 1280, 1024, 1688, 1066, 108 },
    { "1440x900", E_MHAL_DISP_OUTPUT_1440x900_60, E_MHAL_HDMITX_RES_1440X900P_60HZ,
      152, 232, 6, 25, 384, 31, 1440, 900, 1904, 934, 106 },
};

static MS_S32 mem_alloc(MS_U8 *name, MS_U32 size, unsigned long long *phys)
{
    *phys = 0x2F700000;             /* never touched: the recorder has no RAM */
    return 0;
}

static MS_S32 mem_free(unsigned long long phys)
{
    return 0;
}

int main(int argc, char **argv)
{
    const struct tm *t = NULL;
    MHAL_DISP_PanelConfig_t pnl[sizeof(tms) / sizeof(tms[0])];
    MHAL_DISP_AllocPhyMem_t mem = { .alloc = mem_alloc, .free = mem_free };
    MHAL_DISP_DeviceTimingInfo_t ti;
    MHAL_DISP_InputPortAttr_t pa;
    MHAL_DISP_VideoFrameData_t fr;
    void *dev = NULL, *lay = NULL, *port = NULL, *hdmi = NULL;
    MhalHdmitxAttrConfig_t attr;
    MhalHdmitxSignalConfig_t sig;
    MhalHdmitxMuteConfig_t mute;
    MhalHdmitxHpdConfig_t hpd;
    u32 lvl = 0;
    unsigned i;

    rec_out = stdout;
    setvbuf(stdout, NULL, _IOLBF, 0);
    for (i = 0; argc > 1 && i < sizeof(tms) / sizeof(tms[0]); i++)
        if (!strcmp(argv[1], tms[i].name))
            t = &tms[i];
    if (!t) {
        fprintf(stderr, "usage: regdump <timing>\n");
        return 2;
    }

    /* _BootLogoDispPnlInit */
    memset(pnl, 0, sizeof(pnl));
    for (i = 0; i < sizeof(tms) / sizeof(tms[0]); i++) {
        pnl[i].bValid = 1;
        pnl[i].eTiming = tms[i].disp;
        pnl[i].stPanelAttr.m_ucPanelHSyncWidth = tms[i].hsw;
        pnl[i].stPanelAttr.m_ucPanelHSyncBackPorch = tms[i].hbp;
        pnl[i].stPanelAttr.m_ucPanelVSyncWidth = tms[i].vsw;
        pnl[i].stPanelAttr.m_ucPanelVBackPorch = tms[i].vbp;
        pnl[i].stPanelAttr.m_wPanelHStart = tms[i].hst;
        pnl[i].stPanelAttr.m_wPanelVStart = tms[i].vst;
        pnl[i].stPanelAttr.m_wPanelWidth = tms[i].ha;
        pnl[i].stPanelAttr.m_wPanelHeight = tms[i].va;
        pnl[i].stPanelAttr.m_wPanelHTotal = tms[i].ht;
        pnl[i].stPanelAttr.m_wPanelVTotal = tms[i].vt;
        pnl[i].stPanelAttr.m_dwPanelDCLK = tms[i].dclk;
    }
    MHAL_DISP_DbgLevel(&lvl);
    MHAL_DISP_InitPanelConfig(pnl, sizeof(tms) / sizeof(tms[0]));

    rec_mark("disp_create");
    MHAL_DISP_DeviceCreateInstance(&mem, 0, &dev);
    MHAL_DISP_VideoLayerCreateInstance(&mem, 0, &lay);
    MHAL_DISP_InputPortCreateInstance(&mem, lay, 0, &port);
    rec_mark("disp_setup");
    MHAL_DISP_DeviceSetBackGroundColor(dev, 0x800080);
    MHAL_DISP_DeviceEnable(dev, 0);
    /* an NVR drives both: HDMI and VGA from one timing */
    MHAL_DISP_DeviceAddOutInterface(dev, MHAL_DISP_INTF_HDMI);
    MHAL_DISP_DeviceAddOutInterface(dev, MHAL_DISP_INTF_VGA);
    rec_mark("disp_timing");
    ti.eTimeType = t->disp;
    ti.pstSyncInfo = NULL;
    MHAL_DISP_DeviceSetOutputTiming(dev, MHAL_DISP_INTF_HDMI, &ti);
    rec_mark("disp_enable");
    MHAL_DISP_DeviceEnable(dev, 1);

    rec_mark("port");                   /* MOP: done by hand in U-Boot */
    memset(&pa, 0, sizeof(pa));
    pa.u16SrcWidth = t->ha;
    pa.u16SrcHeight = t->va;
    pa.stDispWin.u16Width = t->ha;
    pa.stDispWin.u16Height = t->va;
    memset(&fr, 0, sizeof(fr));
    fr.ePixelFormat = E_MHAL_PIXEL_FRAME_YUV_MST_420;
    fr.aPhyAddr[0] = 0x2F700000;
    fr.aPhyAddr[1] = 0x2F700000 + t->ha * t->va;
    fr.au32Stride[0] = t->ha;
    MHAL_DISP_InputPortSetAttr(port, &pa);
    MHAL_DISP_InputPortFlip(port, &fr);
    MHAL_DISP_InputPortEnable(port, TRUE);

    rec_mark("hdmi_create");
    MhalHdmitxCreateInstance(&hdmi, 0);
    hpd.u8GpioNum = 26;
    MhalHdmitxSetHpdConfig(hdmi, &hpd);
    rec_mark("hdmi_off");
    mute.enType = E_MHAL_HDMITX_MUTE_AUDIO | E_MHAL_HDMITX_MUTE_VIDEO |
                  E_MHAL_HDMITX_MUTE_AVMUTE;
    mute.bMute = 1;
    MhalHdmitxSetMute(hdmi, &mute);
    sig.bEn = 0;
    MhalHdmitxSetSignal(hdmi, &sig);
    rec_mark("hdmi_attr");
    memset(&attr, 0, sizeof(attr));
    attr.bVideoEn = 1;
    attr.enInColor = E_MHAL_HDMITX_COLOR_RGB444;
    attr.enOutColor = E_MHAL_HDMITX_COLOR_RGB444;
    attr.enOutputMode = E_MHAL_HDMITX_OUTPUT_MODE_HDMI;
    attr.enColorDepth = E_MHAL_HDMITX_CD_24_BITS;
    attr.enTiming = t->hdmi;
    attr.bAudioEn = 1;
    attr.enAudioFreq = E_MHAL_HDMITX_AUDIO_FREQ_48K;
    attr.enAudioCh = E_MHAL_HDMITX_AUDIO_CH_2;
    attr.enAudioFmt = E_MHAL_HDMITX_AUDIO_FORMAT_PCM;
    attr.enAudioCode = E_MHAL_HDMITX_AUDIO_CODING_PCM;
    MhalHdmitxSetAttr(hdmi, &attr);
    rec_mark("hdmi_on");
    sig.bEn = 1;
    MhalHdmitxSetSignal(hdmi, &sig);
    mute.bMute = 0;
    MhalHdmitxSetMute(hdmi, &mute);
    rec_mark("end");
    return 0;
}
