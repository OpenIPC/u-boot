/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
*/
#include "crypto_drv_common.h"
#include "soc_drv_common.h"
#include "lib.h"

static crypto_romable_drv_func g_drv_func = {0};

void *crypto_malloc(unsigned int size)
{
    if (g_drv_func.malloc != NULL) {
        return g_drv_func.malloc(size);
    }
    return malloc(size);
}

void crypto_free(const void *ptr)
{
    if (g_drv_func.free != NULL) {
        g_drv_func.free(ptr);
    }
    return free((void *)ptr);
}

void crypto_udelay(unsigned int us)
{
    if (g_drv_func.udelay != NULL) {
        return g_drv_func.udelay(us);
    }
    udelay(us);
}

int crypto_drv_func_register(const crypto_romable_drv_func *drv_func)
{
    if (drv_func == NULL) {
        return CRYPTO_FAILURE;
    }
    g_drv_func.malloc = drv_func->malloc;
    g_drv_func.free = drv_func->free;
    g_drv_func.udelay = drv_func->udelay;
    return CRYPTO_SUCCESS;
}