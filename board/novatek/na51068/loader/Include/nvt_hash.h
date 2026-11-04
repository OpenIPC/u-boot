#ifndef _NVT_HASH_H_
#define _NVT_HASH_H_

typedef enum {
    NVT_HASH_MODE_SHA1 = 0,             ///< block size=> 64 bytes, digest size 20 bytes
    NVT_HASH_MODE_SHA256,               ///< block size=> 64 bytes, digest size 32 bytes
    NVT_HASH_MODE_MAX
} NVT_HASH_MODE_T;

#define NVT_HASH_SHA1_DIGEST_SIZE       20
#define NVT_HASH_SHA1_BLOCK_SIZE        64
#define NVT_HASH_SHA256_DIGEST_SIZE     32
#define NVT_HASH_SHA256_BLOCK_SIZE      64

struct nvt_hash_pio_t {
    u32  mode;                          ///< hash mode
    u8   *src;                          ///< input  data buffer
    u8   *digest;                       ///< digest data buffer
    u32  src_size;                      ///< input  data buffer length
    u32  digest_size;                   ///< digest data buffer length
};

/*************************************************************************************
 *  Public Function Prototype
 *************************************************************************************/
void nvt_hash_close(void);
int  nvt_hash_open(void);
int  nvt_hash_pio_sha(struct nvt_hash_pio_t *p_hash);

#endif /* _NVT_HASH_H_  */
