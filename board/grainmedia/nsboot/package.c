/*
 *      package.c
 *
 *      One package's DDR controller table and CPU clock, as entry.c chose
 *      them at build time before. The Makefile builds this file once per
 *      package with that package's MFLAGS, PACKAGE naming the result.
 */
#include "board.h"
#include "package.h"

#include "ddr_1400.h"
#include "ddr_1333.h"
#include "ddr_1240.h"
#include "ddr_1180.h"
#include "ddr_1140.h"
#include "ddr_1080.h"
#include "ddr_1000.h"
#include "ddr_950.h"
#include "ddr_850.h"
#include "ddr_800.h"
#include "ddr_790.h"
#include "ddr_700.h"
#include "ddr_666.h"
#include "ddr_590.h"
#include "ddr_450.h"

/* set_fcs() arguments of each mode, from entry.c */
/* ================= GM8136 ================ */
#ifdef GM8136
#define NAME "GM8136"
#ifdef NORMAL
#define FCS 0x6216, 81, 3, "normal"
#endif
#ifdef MODE_0
#define FCS 0x5014, 50, 3, "mode 0"
#endif
#ifdef MODE_1
#define FCS 0x4202, 60, 3, "mode 1"     //CPU600
#endif
#ifdef MODE_2
#define FCS 0x6256, 100, 3, "mode 2"
#endif
#ifdef MODE_3
#define FCS 0x6216, 118, 4, "mode 3"
#endif
#ifdef MODE_4
#define FCS 0x4202, 95, 4, "mode 4"
#endif
#ifdef MODE_5
#define FCS 0x6216, 86, 3, "mode 5"
#endif
#ifdef MODE_6
#define FCS 0x6216, 127, 5, "mode 6"
#endif
#endif  //GM8136
/* ================= GM8136S ================ */
#ifdef GM8136S
#define NAME "GM8136S"
#ifdef NORMAL
#define FCS 0x6216, 81, 3, "normal"
#endif
#ifdef MODE_0
#define FCS 0x5216, 127, 5, "mode 0"
#endif
#ifdef MODE_1
#define FCS 0x4202, 60, 3, "mode 1"     //CPU600
#endif
#ifdef MODE_2
#define FCS 0x3216, 59, 3, "mode 2"
#endif
#ifdef MODE_3
#define FCS 0x6216, 86, 3, "mode 3"
#endif
#ifdef MODE_4
#define FCS 0x6216, 118, 4, "mode 4"
#endif
#ifdef MODE_5
#define FCS 0x5216, 93, 3, "mode 5"
#endif
#ifdef MODE_6
#define FCS 0x7156, 100, 3, "mode 6"
#endif
#ifdef MODE_7
#define FCS 0x5216, 95, 4, "mode 7"
#endif
#ifdef MODE_8
#define FCS 0x1002, 59, 4, "mode 8"
#endif
#ifdef MODE_9
#define FCS 0x6256, 105, 3, "mode 9"
#endif
#endif  //GM8136S
/* ================= GM8135S ================ */
#ifdef GM8135S
#define NAME "GM8135S"
#ifdef NORMAL
#define FCS 0x6216, 81, 3, "normal"
#endif
#ifdef MODE_0
#define FCS 0x5014, 50, 3, "mode 0"
#endif
#ifdef MODE_1
#define FCS 0x3216, 59, 3, "mode 1"
#endif
#ifdef MODE_2
#define FCS 0x5216, 127, 5, "mode 2"
#endif
#ifdef MODE_3
#define FCS 0x6216, 86, 3, "mode 3"
#endif
#ifdef MODE_CPU
#define FCS 0x4201, 81, 3, "mode cpu"   //CPU810/800
#endif
#ifdef MODE_4
#define FCS 0x5216, 95, 4, "mode 4"
#endif
#ifdef MODE_5
#define FCS 0x1016, 59, 4, "mode 5"     //AXI 200
#endif
#ifdef MODE_6
#define FCS 0x1016, 50, 4, "mode 6"
#endif
#ifdef MODE_7
#define FCS 0x1016, 45, 4, "mode 7"
#endif
#ifdef MODE_8
#define FCS 0x1002, 40, 4, "mode 8"     //AXI 100
#endif
#ifdef MODE_9
#define FCS 0x1002, 35, 4, "mode 9"     //AXI 87
#endif
#endif  //GM8135S

#ifndef FCS
#define FCS 0, 0, 0, "debug"            /* USE_JUMP: no clock change */
#endif

#if defined(GM8135S)
#define DDR_AR      (const unsigned int (*)[2])DDR_arg_AR, sizeof(DDR_arg_AR) / 8
#else
#define DDR_AR      0, 0
#endif

const struct gm_package PACKAGE = {
    NAME,
    (const unsigned int (*)[2])DDR_arg, sizeof(DDR_arg) / 8,
    DDR_AR,
    FCS
};
