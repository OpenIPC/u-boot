#ifndef _NVT_RSA_H_
#define _NVT_RSA_H_

typedef enum {
    NVT_RSA_MODE_NORMAL = 0,                                ///< normal mode, encryption or decryption
    NVT_RSA_MODE_MAX
} NVT_RSA_MODE_T;

typedef enum {
    NVT_RSA_KEY_WIDTH_256 = 0,                              ///< key width 256  bits
    NVT_RSA_KEY_WIDTH_512,                                  ///< key width 512  bits
    NVT_RSA_KEY_WIDTH_1024,                                 ///< key width 1024 bits
    NVT_RSA_KEY_WIDTH_2048,                                 ///< key width 2048 bits
    NVT_RSA_KEY_WIDTH_4096,                                 ///< key width 4096 bits
    NVT_RSA_KEY_WIDTH_MAX
} NVT_RSA_KEY_WIDTH_T;

#define NVT_RSA_256_KEY_SIZE        32                      ///< bytes
#define NVT_RSA_512_KEY_SIZE        64                      ///< bytes
#define NVT_RSA_1024_KEY_SIZE       128                     ///< bytes
#define NVT_RSA_2048_KEY_SIZE       256                     ///< bytes
#define NVT_RSA_4096_KEY_SIZE       512                     ///< bytes

#define NVT_RSA_256_OUT_SIZE        NVT_RSA_256_KEY_SIZE    ///< bytes
#define NVT_RSA_512_OUT_SIZE        NVT_RSA_512_KEY_SIZE    ///< bytes
#define NVT_RSA_1024_OUT_SIZE       NVT_RSA_1024_KEY_SIZE   ///< bytes
#define NVT_RSA_2048_OUT_SIZE       NVT_RSA_2048_KEY_SIZE   ///< bytes
#define NVT_RSA_4096_OUT_SIZE       NVT_RSA_4096_KEY_SIZE   ///< bytes

struct nvt_rsa_pio_t {
    NVT_RSA_KEY_WIDTH_T key_w;                              ///< key width
    u8                  *src;                               ///< input  data buffer
    u8                  *dst;                               ///< output data buffer
    u8                  *key_n;                             ///< key_n  data buffer
    u8                  *key_ed;                            ///< key_ed data buffer, (N, e) as public key, (N, d) as private key
    u32                 src_size;                           ///< input  data buffer length
    u32                 dst_size;                           ///< output data buffer length
    u32                 key_n_size;                         ///< key_n  data buffer length
    u32                 key_ed_size;                        ///< key_ed data buffer length
};

/*************************************************************************************
 *  Public Function Prototype
 *************************************************************************************/
void nvt_rsa_close(void);
int  nvt_rsa_open(void);
int  nvt_rsa_pio_normal(struct nvt_rsa_pio_t *p_rsa);

#endif /* _NVT_RSA_H_  */
