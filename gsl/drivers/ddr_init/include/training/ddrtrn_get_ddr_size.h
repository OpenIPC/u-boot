/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
 */

#ifndef DDRTRN_GET_DDR_SIZE_H
#define DDRTRN_GET_DDR_SIZE_H

#define WINDING_STEP                 0x4000 /* unit:KB, equal to 16MB */
#define DDR_MAX_CAPAT                0x80000 /* unit: KB, equal to 512MB */

#define DDR_STORE_NUM  4 /* 1:4Byte */

#define DDR_WINDING_NUM1  0x12345678
#define DDR_WINDING_NUM2  0xedcba987

struct ddr_data {
	unsigned int reg_val[DDR_STORE_NUM];
};
#endif /* DDRTRN_GET_DDR_SIZE_H */
