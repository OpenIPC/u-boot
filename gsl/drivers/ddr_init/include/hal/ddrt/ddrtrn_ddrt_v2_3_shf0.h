/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
 */

#ifndef DDR_DDRT_V2_3_SHF0_H
#define DDR_DDRT_V2_3_SHF0_H

/* register offset address */
/* base address: DDR_REG_BASE_DDRT */
#define DDRT_OP            0x0  /* DDRT operation config */
#define DDRT_STATUS        0x4  /* DDRT status indicating */
#define DDRT_BURST_CONFIG  0x8  /* DDRT burst transfer config */
typedef union {
	struct {
		unsigned int ddrt_burst_len : 4;  /* [0-3] */
		unsigned int ddrt_burst_size : 3; /* [4-6] */
		unsigned int reserved : 25;       /* [7-31] */
	} bits;
	unsigned int val;
} ddrt_burst_config_desc;
typedef enum {
	DDRT_DATA_8BYTE = 0x3,
	DDRT_DATA_16BYTE = 0x4,
	DDRT_DATA_32BYTE = 0x5,
} ddrt_data_width;

#define DDRT_MEM_CONFIG    0xc  /* DDRT SDRAM config */
#define DDRT_LP4_MEM_CFG   0x1152
#define DDRT_NOLP4_MEM_CFG 0x1052
#define DDRT_BURST_NUM     0x10 /* DDRT burst number config */
/* DDRT burst number config register while testing address */
#define DDRT_ADDR_NUM      0x14
#define DDRT_LOOP_NUM      0x18 /* DDRT loop number config */
/* This register specified the system DDR starting address */
#define DDRT_DDR_BASE_ADDR 0x1c
#define DDRT_ADDR          0x20  /* DDRT test start address config  */
#define DDRT_REVERSED_DQ   0x30  /* DDRT reversed DQ indicating */
#define DDRT_SEED          0x38  /* DDRT starting random seed */
#define DDRT_KDATA         0x3c  /* DDRT kdata config */
#define DDRT_PRBS_SEED0    0x40  /* DDRT PRBS seed config register0 */
#define DDRT_PRBS_SEED1    0x44  /* DDRT PRBS seed config register1 */
#define DDRT_PRBS_SEED2    0x48  /* DDRT PRBS seed config register2 */
#define DDRT_PRBS_SEED3    0x4c  /* DDRT PRBS seed config register3 */
#define DDRT_PRBS_SEED4    0x50  /* DDRT PRBS seed config register4 */
#define DDRT_PRBS_SEED5    0x54  /* DDRT PRBS seed config register5 */
#define DDRT_PRBS_SEED6    0x58  /* DDRT PRBS seed config register6 */
#define DDRT_PRBS_SEED7    0x5c  /* DDRT PRBS seed config register7 */
#define DDRT_DQ_ERR_CNT0   0x60  /* DDRT error number indicator register of DQ3~DQ0 */
#define DDRT_DQ_ERR_CNT1   0x64  /* DDRT error number indicator register of DQ7~DQ4 */
#define DDRT_DQ_ERR_CNT2   0x68  /* DDRT error number indicator register of DQ11~DQ8 */
#define DDRT_DQ_ERR_CNT3   0x6c  /* DDRT error number indicator register of DQ15~DQ12 */
#define DDRT_DQ_ERR_CNT4   0x70  /* DDRT error number indicator register of DQ19~DQ16 */
#define DDRT_DQ_ERR_CNT5   0x74  /* DDRT error number indicator register of DQ23~DQ20 */
#define DDRT_DQ_ERR_CNT6   0x78  /* DDRT error number indicator register of DQ27~DQ24 */
#define DDRT_DQ_ERR_CNT7   0x7c  /* DDRT error number indicator register of DQ31~DQ28 */
#define DDRT_DQ_ERR_OFFSET 0x10

/* DQ3~DQ0 error number indicator, every 8bit for each DQ */
static inline unsigned int ddrt_dq_err_cnt(unsigned int dq)
{
	return (0x60 + (dq << 2)); /* shift left 2 : 4 DQ */
}
/* DQ31~DQ0 error number overflow indicator, every bit for each DQ. */
#define DDRT_DQ_ERR_OVFL 0x80

/* register mask */
#define DDRT_TEST_MODE_MASK 0x300 /* DDRT Test Mode */
#define DDRT_TEST_DONE_MASK 0x1   /* [0] DDRT operation finish signal */
/* [1] DDRT Test result indicator. No error occurred, test pass. */
#define DDRT_TEST_PASS_MASK 0x2

/* register bit */
#define DDRT_DDR_MEM_WIDTH 12 /* SDRAM total width */

/* register value */
#define DDRT_CFG_START             0x1
#define DDRT_CFG_BURST_CFG_DATAEYE 0x4f
#define DDRT_CFG_BURST_CFG_GATE    0x43
#ifdef CFG_EDA_VERIFY
#define DDRT_CFG_BURST_NUM 0x5 /* ddrt test number */
#else
#define DDRT_CFG_BURST_NUM 0x7f /* ddrt test number */
#endif
#define DDRT_CFG_SEED      0x6d6d6d6d
#define DDRT_CFG_REVERSED  0x55aa55aa
#ifndef DDRT_CFG_BASE_ADDR
/* [CUSTOM] DDR training start address. MEM_BASE_DDR */
#define DDRT_CFG_BASE_ADDR 0x0
#endif
/* [CUSTOM] DDRT test address. 0x800000 = 8M */
#define DDRT_CFG_TEST_ADDR_CMD (DDRT_CFG_BASE_ADDR + 0x800000)
/* [CUSTOM] DDRT test start address. */
#define DDRT_CFG_TEST_ADDR_BOOT DDRT_CFG_BASE_ADDR
#define DDRT_CFG_ADDR_NUM 0xffffffff
#define DDRT_CFG_LOOP_NUM 0x0

/* [2:0]000:8 bit; 001:9 bit; 010:10 bit; 011:11 bit; 100:12 bit.
single SDRAM column number. */
#define DDRT_DDR_COL_WIDTH 0x2
/* [6:4]000:11 bit; 001:12 bit; 010:13 bit; 011:14 bit; 100:15 bit; 101:16 bit.
single SDRAM row number */
#define DDRT_DDR_ROW_WIDTH 0x50
/* [8]0:4 Bank; 1:8 Bank. single SDRAM bank number */
#define DDRT_DDR_BANK_WIDTH 0x100

#define DDRT_WR_COMPRARE_MODE (0 << 8)  /* Write read & compare mode */
#define DDRT_WRITE_ONLY_MODE  (1 << 8)  /* Write only mode */
#define DDRT_READ_ONLY_MODE   (2 << 8)  /* Read only mode */
#define DDRT_RANDOM_WR_MODE   (3 << 8)  /* Random write & read mode */

#define DDRT_PATTERM_PRBS11   (0 << 12)
#define DDRT_PATTERM_PRBS7    (1 << 12)
#define DDRT_PATTERM_PRBS15   (2 << 12)
#define DDRT_PATTERM_PRBS31   (3 << 12)

#define DDRT_PRBS_SEED_DEFAULT_VAL  0x11251986
#define DDRT_PRBS_SEED1_VAL         0xb6112519
#define DDRT_PRBS_SEED2_VAL         0xbab61125
#define DDRT_PRBS_SEED3_VAL         0x01bab611
#define DDRT_PRBS_SEED4_VAL         0xd301bab6
#define DDRT_PRBS_SEED5_VAL         0xe0d301ba
#define DDRT_PRBS_SEED6_VAL         0x8de0d301
#define DDRT_PRBS_SEED7_VAL         0x618de0d3

/* other */
#define DDRT_WAIT_TIMEOUT       1000000
#define DDRT_READ_TIMEOUT       20
#define DDRT_PCODE_WAIT_TIMEOUT 100000

typedef enum {
	DDRT_READ_ONLY = 0x201,
	DDRT_WRITE_ONLY = 0x101,
	DDRT_WRITE_READ = 0x001,
} ddrt_mode;

typedef enum {
	DDRT_RUNNING = 0x0,
	DDRT_ERROR = 0x1,
	DDRT_NORMAL = 0x3,
	DDRT_TIMEOUT = 0x11,
} ddrt_status;

typedef enum {
	DDRT_MASK_LOW_16BIT = 0x0,
	DDRT_MASK_HIGH_16BIT = 0x1,
	DDRT_MASK_32BIT = 0x2,
} ddrt_byte_mask_tag;

/* DDRT test DDR using space */
static inline unsigned int ddrt_get_test_addr(unsigned int addr)
{
	return addr;
}
#endif /* DDR_DDRT_V2_3_SHF0_H */