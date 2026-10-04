/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
*/
#include "crypto_drv_common.h"

#include "cipher_romable.h"

#define SYMC_NODE_DEPTH         2
#define CRYPTO_AES_MAX_DATA_SIZE (128 * 1024 * 1024) // max length: 128 MB
#define CRYPTO_DMA_MAX_DATA_SIZE (128 * 1024 * 1024) // max length: 128 MB

/* reg offset. */
#define SPACC_SYM_CHN_LOCK                    0x0020
#define IN_SYM_CHN_NODE_START_ADDR_L(id)      (0x4124 + ((id) - 1) * 0x100)
#define IN_SYM_CHN_NODE_LENGTH(id)            (0x412c + ((id) - 1) * 0x100)
#define OUT_SYM_CHN_NODE_START_ADDR_L(id)     (0xD024 + ((id) - 1) * 0x100)
#define OUT_SYM_CHN_NODE_LENGTH(id)           (0xD028 + ((id) - 1) * 0x100)
#define IN_SYM_CHN_CTRL(id)                   (0x4100 + ((id) - 1) * 0x100)
#define IN_SYM_CHN_KEY_CTRL(id)               (0x4110 + ((id) - 1) * 0x100)
#define OUT_SYM_CHN_NODE_WR_POINT(id)         (0xD030 + ((id) - 1) * 0x100)
#define IN_SYM_CHN_NODE_WR_POINT(id)          (0x4130 + ((id) - 1) * 0x100)
#define OUT_SYM_CHAN_RAW_LAST_NODE_INT        0xc000
#define SPACC_SYM_CHN_CLEAR_REQ               0x0060
#define SPACC_INT_RAW_SYM_CLEAR_FINISH        0x0008
#define IN_SYM_OUT_CTRL(id)                   (0x4104 + ((id) - 1) * 0x100)

/* ! spacc symc int entry struct which is defined by hardware, you can't change it */
typedef struct {
    td_u32 sym_first_node : 1;         /* !<  Indicates whether the node is the first node */
    td_u32 sym_last_node : 1;          /* !<  Indicates whether the node is the last node */
    td_u32 rev1 : 7;                   /* !<  reserve */
    td_u32 odd_even : 1;               /* !<  Indicates whether the key is an odd key or even key */
    td_u32 rev2 : 22;                  /* !<  reserve */
    td_u32 sym_alg_length;             /* !<  symc data length */
    td_u32 sym_start_addr;             /* !<  symc start low addr */
    td_u32 sym_start_high;             /* !<  symc start high addr */
    td_u32 iv[4];                      /* !<  symc IV */
} crypto_symc_entry_in;

/* ! spacc symc out entry struct which is defined by hardware, you can't change it */
typedef struct {
    td_u32 rev1;           /* !<  reserve */
    td_u32 sym_alg_length; /* !<  syma data length */
    td_u32 sym_start_addr; /* !<  syma start low addr */
    td_u32 sym_start_high; /* !<  syma start high addr */
} crypto_symc_entry_out;

/* Define the union in_sym_chn_node_wr_point */
typedef union {
    /* Define the struct bits */
    struct {
        td_u32 sym_chn_node_wr_point : 8; /* [7..0] */
        td_u32 reserved_1 : 24;           /* [31..8] */
    } bits;
    /* Define an unsigned member */
    td_u32 u32;
} in_sym_chn_node_wr_point;

/* Define the union out_sym_chn_node_wr_point */
typedef union {
    /* Define the struct bits */
    struct {
        td_u32 sym_chn_node_wr_point : 8; /* [7..0] */
        td_u32 reserved_1 : 24;               /* [31..8] */
    } bits;
    /* Define an unsigned member */
    td_u32 u32;
} out_sym_chn_node_wr_point;

/* Define the union out_sym_chan_raw_int */
typedef union {
    /* Define the struct bits */
    struct {
        td_u32 out_sym_chan_raw_int : 16; /* [15..0] */
        td_u32 reserved_1 : 16;           /* [31..16] */
    } bits;
    /* Define an unsigned member */
    td_u32 u32;
} out_sym_chan_raw_int;

/* Define the union spacc_sym_chn_clear_req */
typedef union {
    /* Define the struct bits */
    struct {
        td_u32 sym_chn_clear_req : 16; /* [15..0] */
        td_u32 reserved_0 : 16;        /* [31..16] */
    } bits;
    /* Define an unsigned member */
    td_u32 u32;
} spacc_sym_chn_clear_req;

/* Define the union spacc_int_raw_sym_clear_finish */
typedef union {
    /* Define the struct bits */
    struct {
        td_u32 int_raw_sym_clear_finish : 16; /* [15..0] */
        td_u32 reserved_0 : 16;               /* [31..16] */
    } bits;
    /* Define an unsigned member */
    td_u32 u32;
} spacc_int_raw_sym_clear_finish;

static const uint32_t g_symc_chn = 2;
static uint32_t g_node_idx = 0;
static crypto_symc_entry_in __attribute__((aligned(4))) g_entry_in[SYMC_NODE_DEPTH];
static crypto_symc_entry_out __attribute__((aligned(4))) g_entry_out[SYMC_NODE_DEPTH];

int32_t inner_symc_romable_init(void)
{
    /* 1. Lock one symc channel gloably. */
    spacc_reg_write(SPACC_SYM_CHN_LOCK, 0x200);

    /* 2. set channel node start addr & node depth. */
    spacc_reg_write(IN_SYM_CHN_NODE_START_ADDR_L(g_symc_chn), (uint32_t)crypto_get_phys_addr(g_entry_in));
    spacc_reg_write(IN_SYM_CHN_NODE_LENGTH(g_symc_chn), SYMC_NODE_DEPTH);
    spacc_reg_write(OUT_SYM_CHN_NODE_START_ADDR_L(g_symc_chn), (uint32_t)crypto_get_phys_addr(g_entry_out));
    spacc_reg_write(OUT_SYM_CHN_NODE_LENGTH(g_symc_chn), SYMC_NODE_DEPTH);

    /* 3. set the chn ctrl. */
    /*
     * 0x80005500:
     * bit[31] is 1'b1, means channel enable.
     * bit[15:12] is 4'h5, means symc_chn_ds is secure.
     * bit[11:8] is 4'h5, means symc_chn_ss is secure.
     */
    spacc_reg_write(IN_SYM_CHN_CTRL(g_symc_chn), 0x80005500);   // 0x80005500: refer to the above.
    return CRYPTO_SUCCESS;
}

int32_t inner_symc_romable_deinit(void)
{
    /* 1. Clear symc channel. */
    /* 2. Unock symc channel. */
    spacc_reg_write(SPACC_SYM_CHN_LOCK, 0);
    return CRYPTO_SUCCESS;
}

static int32_t inner_symc_romable_wait_done(void)
{
    uint32_t i;
    out_sym_chan_raw_int last_raw;

    for (i = 0; i < CONFIG_SYMC_WAIT_TIMEOUT_IN_US; i++) {
        last_raw.u32 = spacc_reg_read(OUT_SYM_CHAN_RAW_LAST_NODE_INT);
        last_raw.bits.out_sym_chan_raw_int &= (0x01 << g_symc_chn);
        if (last_raw.bits.out_sym_chan_raw_int != 0) {
            spacc_reg_write(OUT_SYM_CHAN_RAW_LAST_NODE_INT, last_raw.u32);
            break;
        }
        crypto_udelay(1);
    }
    if (i >= CONFIG_SYMC_WAIT_TIMEOUT_IN_US) {
        return ERROR_SYMC_CALC_TIMEOUT;
    }

    return CRYPTO_SUCCESS;
}

int32_t gsl_drv_cipher_dma_copy(uint32_t src, uint32_t dst, uint32_t length)
{
    in_sym_chn_node_wr_point in_node_wr_ptr;
    out_sym_chn_node_wr_point out_node_wr_ptr;
    uint32_t ptr;

    /* 1. set the chn_key_ctrl and out_ctrl for dma_copy. */
    spacc_reg_write(IN_SYM_CHN_KEY_CTRL(g_symc_chn), 0);
    spacc_reg_write(IN_SYM_OUT_CTRL(g_symc_chn), 1);    // 1: enable dma copy.
    /* 2. set input node. */
    g_entry_in[g_node_idx].sym_first_node = 1;
    g_entry_in[g_node_idx].sym_last_node = 1;
    g_entry_in[g_node_idx].odd_even = 0;
    g_entry_in[g_node_idx].sym_alg_length = length;
    g_entry_in[g_node_idx].sym_start_addr = (uint32_t)src;
    g_entry_in[g_node_idx].sym_start_high = 0;
    /* 3. set output node. */
    g_entry_out[g_node_idx].sym_alg_length = length;
    g_entry_out[g_node_idx].sym_start_addr = (uint32_t)dst;
    g_entry_out[g_node_idx].sym_start_high = 0;

    g_node_idx = (g_node_idx + 1) % SYMC_NODE_DEPTH;
    /* 4. start. */
    /* configure out node wr_point. */
    out_node_wr_ptr.u32 = spacc_reg_read(OUT_SYM_CHN_NODE_WR_POINT(g_symc_chn));
    ptr = out_node_wr_ptr.bits.sym_chn_node_wr_point + 1;
    out_node_wr_ptr.bits.sym_chn_node_wr_point = ptr % SYMC_NODE_DEPTH;
    spacc_reg_write(OUT_SYM_CHN_NODE_WR_POINT(g_symc_chn), out_node_wr_ptr.u32);

    /* configure in node wr_point. */
    in_node_wr_ptr.u32 = spacc_reg_read(IN_SYM_CHN_NODE_WR_POINT(g_symc_chn));
    ptr = in_node_wr_ptr.bits.sym_chn_node_wr_point + 1;
    in_node_wr_ptr.bits.sym_chn_node_wr_point = ptr % SYMC_NODE_DEPTH;
    spacc_reg_write(IN_SYM_CHN_NODE_WR_POINT(g_symc_chn), in_node_wr_ptr.u32);

    /* 5. wait done. */
    return inner_symc_romable_wait_done();
}

int32_t gsl_drv_cipher_symc_decrypt(ext_symc_alg symc_alg, uint32_t keyslot_chn_num, uint8_t *iv, uint32_t iv_length,
    uint32_t src, uint32_t dst, uint32_t length)
{
    in_sym_chn_node_wr_point in_node_wr_ptr;
    out_sym_chn_node_wr_point out_node_wr_ptr;
    uint32_t ptr;
    int32_t ret;
    crypto_unused(keyslot_chn_num);
    if (iv == NULL || iv_length != 16) {   // 16: the valid iv length.
        return ERROR_INVALID_PARAM;
    }

    if ((length % CRYPTO_AES_BLOCK_SIZE_IN_BYTES  != 0) || (length > CRYPTO_AES_MAX_DATA_SIZE)) {
        return ERROR_INVALID_PARAM;
    }

    /* 1. set the chn_key_ctrl and out_ctrl for aes-128-cbc decrypt. */
    /*
     * 0x11320000
     * bit[28] is 1'b1, means sym_alg_decrypt is decrypt.
     * bit[27:26] is 2'b0, means sym_alg_data_witdh is 128bit for AES.
     * bit[25:24] is 2'b1, means sym_alg_key_len is 128bit.
     * bit[23:20] is 4'h3, means sym_alg_mode is CBC.
     * bit[19:16] is 4'h2 or 4'h5. 4'h2 means symc_alg_sel is AES, 4'h5 means symc_alg_sel is SM4.
     * bit[8:0] is 0, means sym_key_chn_id is 0.
     */
     if (symc_alg == EXT_SYMC_ALG_AES) {
        spacc_reg_write(IN_SYM_CHN_KEY_CTRL(g_symc_chn), 0x11320000);   // 0x11320000: refer to the above.
     } else if (symc_alg == EXT_SYMC_ALG_SM4) {
        spacc_reg_write(IN_SYM_CHN_KEY_CTRL(g_symc_chn), 0x11350000);   // 0x11350000: refer to the above.
     } else {
        return ERROR_INVALID_PARAM;
     }
    spacc_reg_write(IN_SYM_OUT_CTRL(g_symc_chn), 0);    // 0: disable dma copy.

    /* 2. set input node. */
    g_entry_in[g_node_idx].sym_first_node = 1;
    g_entry_in[g_node_idx].sym_last_node = 1;
    g_entry_in[g_node_idx].odd_even = 0;
    g_entry_in[g_node_idx].sym_alg_length = length;
    g_entry_in[g_node_idx].sym_start_addr = src;
    g_entry_in[g_node_idx].sym_start_high = 0;
    ret = memcpy_s(g_entry_in[g_node_idx].iv, sizeof(g_entry_in[g_node_idx].iv), iv, iv_length);
    crypto_chk_return(ret != EOK, ERROR_MEMCPY_S, "memcpy_s failed\n");
    /* 3. set output node. */
    g_entry_out[g_node_idx].sym_alg_length = length;
    g_entry_out[g_node_idx].sym_start_addr = dst;
    g_entry_out[g_node_idx].sym_start_high = 0;
    g_node_idx = (g_node_idx + 1) % SYMC_NODE_DEPTH;
    /* 4. start. */
    /* configure out node wr_point. */
    out_node_wr_ptr.u32 = spacc_reg_read(OUT_SYM_CHN_NODE_WR_POINT(g_symc_chn));
    ptr = out_node_wr_ptr.bits.sym_chn_node_wr_point + 1;
    out_node_wr_ptr.bits.sym_chn_node_wr_point = ptr % SYMC_NODE_DEPTH;
    spacc_reg_write(OUT_SYM_CHN_NODE_WR_POINT(g_symc_chn), out_node_wr_ptr.u32);
    /* configure in node wr_point. */
    in_node_wr_ptr.u32 = spacc_reg_read(IN_SYM_CHN_NODE_WR_POINT(g_symc_chn));
    ptr = in_node_wr_ptr.bits.sym_chn_node_wr_point + 1;
    in_node_wr_ptr.bits.sym_chn_node_wr_point = ptr % SYMC_NODE_DEPTH;
    spacc_reg_write(IN_SYM_CHN_NODE_WR_POINT(g_symc_chn), in_node_wr_ptr.u32);
    /* 5. wait done. */
    return inner_symc_romable_wait_done();
}
