/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef UAPI_COMMON_H
#define UAPI_COMMON_H

int crypto_cipher_open(void);

void crypto_cipher_close(void);

int crypto_cipher_ioctl(int cmd, void *args);

int crypto_cipher_is_init(void);

#endif