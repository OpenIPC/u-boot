/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
*/
#if defined(CONFIG_PKE_ECC_SUPPORT)

#include "drv_pke_inner.h"
#include "crypto_drv_common.h"
#if defined(CONFIG_HASH_SUPPORT)
#include "crypto_hash_struct.h"
#endif

#define MAX_LOW_2BITS              3
#define MAX_LOW_3BITS              7
#define MAX_LOW_4BITS              0xF
#define MAX_LOW_8BITS              0xFF

#define SHIFT_4BITS                4
#define SHIFT_8BITS                8
#define SHIFT_16BITS               16
#define SHIFT_24BITS               24

#define BOUND_VALUE_1              1

td_bool inner_drv_is_zero(const td_u8 *val, td_u32 length)
{
    unsigned int i;
    for (i = 0; i < length; i++) {
        if (val[i] != 0) {
            return 0;
        }
    }
    return 1;
}

int memcmp(const void *cs, const void *ct, size_t count)
{
	const unsigned char *su1, *su2;
	int res = 0;

	for (su1 = cs, su2 = ct; 0 < count; ++su1, ++su2, count--)
		if ((res = *su1 - *su2) != 0)
			break;
	return res;
}

td_bool inner_drv_is_in_range(const uint8_t *value, const uint8_t *range, uint32_t len)
{
    if (memcmp(value, range, len) < 0 && inner_drv_is_zero(value, len) == TD_FALSE) {
        return TD_TRUE;
    }
    return TD_FALSE;
}

#endif