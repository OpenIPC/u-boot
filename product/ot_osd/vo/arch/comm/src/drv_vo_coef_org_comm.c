/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include "drv_vo_coef_org_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#if vo_desc("UBOOT_VO")
const csc_coef g_csc_identity_limit = {
    1024, 0, 0, 0, 1024, 0, 0, 0, 1024,
    -16, -128, -128,
    16, 128, 128
};

const csc_coef g_csc_identity_full = {
    1024, 0, 0, 0, 1024, 0, 0, 0, 1024,
    0, -128, -128,
    0, 128, 128
};

const csc_coef g_csc_yuv601full_to_yuv601limit = {
    879, 0, 0, 0, 900, 0, 0, 0, 900,
    0, -128, -128,
    16, 128, 128
};

const csc_coef g_csc_yuv709limit_to_yuv601limit = {
    1024, 102, 196, 0, 1014, -113, 0, -74, 1007,
    -16, -128, -128,
    16, 128, 128
};

const csc_coef g_csc_yuv709full_to_yuv601limit = {
    879, 89, 172, 0, 890, -100, 0, -65, 885,
    0, -128, -128,
    16, 128, 128
};

const csc_coef g_csc_yuv601limit_to_yuv709limit = {
    1024, -118, -213, 0, 1043, 117, 0, 77, 1050,
    -16, -128, -128,
    16, 128, 128
};

const csc_coef g_csc_yuv601full_to_yuv709limit = {
    879, -104, -187, 0, 916, 103, 0, 68, 922,
    0, -128, -128,
    16, 128, 128
};

const csc_coef g_csc_yuv709full_to_yuv709limit = {
    879, 0, 0, 0, 900, 0, 0, 0, 900,
    0, -128, -128,
    16, 128, 128
};

const csc_coef g_csc_yuv601limit_to_yuv601full = {
    1192, 0, 0, 0, 1166, 0, 0, 0, 1166,
    -16, -128, -128,
    0, 128, 128
};

const csc_coef g_csc_yuv709limit_to_yuv601full = {
    1192, 118, 229, 0, 1154, -129, 0, -84, 1146,
    -16, -128, -128,
    0, 128, 128
};

const csc_coef g_csc_yuv709full_to_yuv601full = {
    1024, 104, 201, 0, 1014, -113, 0, -74, 1007,
    0, -128, -128,
    0, 128, 128
};

const csc_coef g_csc_yuv601limit_to_yuv709full = {
    1192, -138, -248, 0, 1187, 134, 0, 87, 1195,
    -16, -128, -128,
    0, 128, 128
};

const csc_coef g_csc_yuv601full_to_yuv709full = {
    1024, -121, -218, 0, 1043, 117, 0, 77, 1050,
    0, -128, -128,
    0, 128, 128
};

const csc_coef g_csc_yuv709limit_to_yuv709full = {
    1192, 0, 0, 0, 1166, 0, 0, 0, 1166,
    -16, -128, -128,
    0, 128, 128
};

const csc_coef g_csc_yuv601limit_to_rgbfull = {
    1192, 0, 1634, 1192, -401, -832, 1192, 2066, 0,
    -16, -128, -128,
    0, 0, 0
};

const csc_coef g_csc_yuv601full_to_rgbfull = {
    1024, 0, 1436, 1024, -352, -731, 1024, 1815, 0,
    0, -128, -128,
    0, 0, 0
};

const csc_coef g_csc_yuv709limit_to_rgbfull = {
    1192, 0, 1836, 1192, -218, -546, 1192, 2163, 0,
    -16, -128, -128,
    0, 0, 0
};

const csc_coef g_csc_yuv709full_to_rgbfull = {
    1024, 0, 1613, 1024, -192, -479, 1024, 1900, 0,
    0, -128, -128,
    0, 0, 0
};

const csc_coef g_csc_yuv601limit_to_rgblimit = {
    1024, 0, 1404, 1024, -345, -715, 1024, 1774, 0,
    -16, -128, -128,
    16, 16, 16
};

const csc_coef g_csc_yuv601full_to_rgblimit = {
    879, 0, 1233, 879, -303, -628, 879, 1558, 0,
    0, -128, -128,
    16, 16, 16
};

const csc_coef g_csc_yuv709limit_to_rgblimit = {
    1024, 0, 1577, 1024, -188, -469, 1024, 1858, 0,
    -16, -128, -128,
    16, 16, 16
};

const csc_coef g_csc_yuv709full_to_rgblimit = {
    879, 0, 1385, 879, -165, -412, 879, 1632, 0,
    0, -128, -128,
    16, 16, 16
};

const csc_coef g_csc_rgbfull_to_yuv601limit = {
    263, 516, 100, -152, -298, 450, 450, -377, -73,
    0, 0, 0,
    16, 128, 128
};

const csc_coef g_csc_rgbfull_to_yuv601full = {
    306, 601, 117, -173, -339, 512, 512, -429, -83,
    0, 0, 0,
    0, 128, 128
};

const csc_coef g_csc_rgbfull_to_yuv709limit = {
    187, 629, 63, -103, -347, 450, 450, -409, -41,
    0, 0, 0,
    16, 128, 128
};

const csc_coef g_csc_rgbfull_to_yuv709full = {
    218, 732, 74, -117, -395, 512, 512, -465, -47,
    0, 0, 0,
    0, 128, 128
};

const td_s32 g_sin_table[61] = { /* 61 theta */
    -500, -485, -469, -454, -438, -422, -407, -391, -374, -358,
    -342, -325, -309, -292, -276, -259, -242, -225, -208, -191,
    -174, -156, -139, -122, -104, -87, -70, -52, -35, -17,
    0, 17, 35, 52, 70, 87, 104, 122, 139, 156,
    174, 191, 208, 225, 242, 259, 276, 292, 309, 325,
    342, 358, 374, 391, 407, 422, 438, 454, 469, 485,
    500
};

const td_s32 g_cos_table[61] = { /* 61 theta */
    866, 875, 883, 891, 899, 906, 914, 921, 927, 934,
    940, 946, 951, 956, 961, 966, 970, 974, 978, 982,
    985, 988, 990, 993, 995, 996, 998, 999, 999, 1000,
    1000, 1000, 999, 999, 998, 996, 995, 993, 990, 988,
    985, 982, 978, 974, 970, 966, 961, 956, 951, 946,
    940, 934, 927, 921, 914, 906, 899, 891, 883, 875,
    866
};

const csc_coef *g_csc_coef[OT_VO_CSC_MATRIX_BUTT] = {
    &g_csc_identity_limit,
    &g_csc_yuv601full_to_yuv601limit,
    &g_csc_yuv709limit_to_yuv601limit,
    &g_csc_yuv709full_to_yuv601limit,
    &g_csc_yuv601limit_to_yuv709limit,
    &g_csc_yuv601full_to_yuv709limit,
    &g_csc_identity_limit,
    &g_csc_yuv709full_to_yuv709limit,
    &g_csc_yuv601limit_to_yuv601full,
    &g_csc_identity_full,
    &g_csc_yuv709limit_to_yuv601full,
    &g_csc_yuv709full_to_yuv601full,
    &g_csc_yuv601limit_to_yuv709full,
    &g_csc_yuv601full_to_yuv709full,
    &g_csc_yuv709limit_to_yuv709full,
    &g_csc_identity_full,
    &g_csc_yuv601limit_to_rgbfull,
    &g_csc_yuv601full_to_rgbfull,
    &g_csc_yuv709limit_to_rgbfull,
    &g_csc_yuv709full_to_rgbfull,
    &g_csc_yuv601limit_to_rgblimit,
    &g_csc_yuv601full_to_rgblimit,
    &g_csc_yuv709limit_to_rgblimit,
    &g_csc_yuv709full_to_rgblimit,
    &g_csc_rgbfull_to_yuv601limit,
    &g_csc_rgbfull_to_yuv601full,
    &g_csc_rgbfull_to_yuv709limit,
    &g_csc_rgbfull_to_yuv709full,
};

const csc_coef *vo_get_csc_coef(ot_vo_csc_matrix csc_matrix)
{
    if ((csc_matrix >= OT_VO_CSC_MATRIX_BT601LIMIT_TO_BT601LIMIT) &&
        (csc_matrix < OT_VO_CSC_MATRIX_BUTT)) {
        return g_csc_coef[csc_matrix];
    }

    return TD_NULL;
}

const td_s32 *vo_get_sin_table(td_void)
{
    return g_sin_table;
}

const td_s32 *vo_get_cos_table(td_void)
{
    return g_cos_table;
}
#endif /* #if vo_desc("UBOOT_VO") */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */
