/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef CRYPTO_ECC_CURVE_H
#define CRYPTO_ECC_CURVE_H

#include "crypto_pke_struct.h"

const drv_pke_ecc_curve *get_ecc_curve(drv_pke_ecc_curve_type curve_type);

#endif