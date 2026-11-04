#ifndef __SDHCI_H__
#define __SDHCI_H__

#define   SDHCI_SPEC_300    2
/* SDHCI_BLOCK_SIZE 0x04 */
#define SDHCI_MAKE_BLKSZ(dma, blksz) (((dma & 0x7) << 12) | (blksz & 0xFFF))

/* SDHCI_TRANSFER_MODE  0x0C */
#define  SDHCI_TRNS_DMA         0x01
#define  SDHCI_TRNS_BLK_CNT_EN  0x02
#define  SDHCI_TRNS_ACMD12      0x04
#define  SDHCI_TRNS_READ        0x10
#define  SDHCI_TRNS_MULTI       0x20

/* SDHCI_COMMAND    0x0E */
#define  SDHCI_CMD_RESP_MASK    0x03
#define  SDHCI_CMD_CRC          0x08
#define  SDHCI_CMD_INDEX        0x10
#define  SDHCI_CMD_DATA         0x20
#define  SDHCI_CMD_ABORTCMD     0xC0

#define  SDHCI_CMD_RESP_NONE    0x00
#define  SDHCI_CMD_RESP_LONG    0x01
#define  SDHCI_CMD_RESP_SHORT   0x02
#define  SDHCI_CMD_RESP_SHORT_BUSY 0x03

#define SDHCI_MAKE_CMD(c, f) (((c & 0xff) << 8) | (f & 0xff))
#define SDHCI_GET_CMD(c) ((c>>8) & 0x3f)

/* SDHCI_PRESENT_STATE  0x24 */
#define  SDHCI_CMD_INHIBIT              0x00000001
#define  SDHCI_DATA_INHIBIT             0x00000002
#define  SDHCI_DOING_WRITE              0x00000100
#define  SDHCI_DOING_READ               0x00000200
#define  SDHCI_SPACE_AVAILABLE          0x00000400
#define  SDHCI_DATA_AVAILABLE           0x00000800
#define  SDHCI_CARD_PRESENT             0x00010000
#define  SDHCI_CARD_STATE_STABLE        0x00020000
#define  SDHCI_CARD_DETECT_PIN_LEVEL    0x00040000
#define  SDHCI_WRITE_PROTECT            0x00080000

/* SDHCI_HOST_CONTROL   0x28 */
//#define  SDHCI_CTRL_LED       0x01
#define  SDHCI_CTRL_4BITBUS     0x02
#define  SDHCI_CTRL_HISPD       0x04
#define  SDHCI_CTRL_8BITBUS     0x20
#define  SDHCI_CTRL_CD_TEST_INS 0x40
#define  SDHCI_CTRL_CD_TEST     0x80

/* SDHCI_POWER_CONTROL  0x29 */
#define  SDHCI_POWER_ON         0x01
//#define  SDHCI_POWER_180      0x0A
//#define  SDHCI_POWER_300      0x0C
#define  SDHCI_POWER_330        0x0E

/* SDHCI_CLOCK_CONTROL  0x2C */
#define  SDHCI_DIVIDER_SHIFT    8
#define  SDHCI_DIVIDER_HI_SHIFT 6
#define  SDHCI_DIV_MASK         0xFF
#define  SDHCI_DIV_MASK_LEN     8
#define  SDHCI_DIV_HI_MASK      0x300
#define  SDHCI_CLOCK_CARD_EN    0x0004
#define  SDHCI_CLOCK_INT_STABLE 0x0002
#define  SDHCI_CLOCK_INT_EN     0x0001

/* SDHCI_SOFTWARE_RESET 0x2F */
#define  SDHCI_RESET_ALL        0x01
#define  SDHCI_RESET_CMD        0x02
#define  SDHCI_RESET_DATA       0x04

/* SDHCI_INT_STATUS 0x30 */
/* SDHCI_INT_ENABLE 0x34 */
/* SDHCI_SIGNAL_ENABLE  0x38 */
#define  SDHCI_INT_RESPONSE     0x00000001
#define  SDHCI_INT_DATA_END     0x00000002
#define  SDHCI_INT_DMA_END      0x00000008
#define  SDHCI_INT_SPACE_AVAIL  0x00000010
#define  SDHCI_INT_DATA_AVAIL   0x00000020
#define  SDHCI_INT_CARD_INSERT  0x00000040
#define  SDHCI_INT_CARD_REMOVE  0x00000080
#define  SDHCI_INT_CARD_INT     0x00000100
#define  SDHCI_INT_ERROR        0x00008000
#define  SDHCI_INT_TIMEOUT      0x00010000
#define  SDHCI_INT_CRC          0x00020000
#define  SDHCI_INT_END_BIT      0x00040000
#define  SDHCI_INT_INDEX        0x00080000
#define  SDHCI_INT_DATA_TIMEOUT 0x00100000
#define  SDHCI_INT_DATA_CRC     0x00200000
#define  SDHCI_INT_DATA_END_BIT 0x00400000
#define  SDHCI_INT_BUS_POWER    0x00800000
#define  SDHCI_INT_ACMD12ERR    0x01000000
#define  SDHCI_INT_ADMA_ERROR   0x02000000

#define  SDHCI_INT_NORMAL_MASK  0x00007FFF
#define  SDHCI_INT_ERROR_MASK   0xFFFF8000

#define  SDHCI_INT_CMD_MASK (SDHCI_INT_RESPONSE | SDHCI_INT_TIMEOUT | \
        SDHCI_INT_CRC | SDHCI_INT_END_BIT | SDHCI_INT_INDEX)
#define  SDHCI_INT_DATA_MASK    (SDHCI_INT_DATA_END | SDHCI_INT_DMA_END | \
        SDHCI_INT_DATA_AVAIL | SDHCI_INT_SPACE_AVAIL | \
        SDHCI_INT_DATA_TIMEOUT | SDHCI_INT_DATA_CRC | \
        SDHCI_INT_DATA_END_BIT | SDHCI_INT_ADMA_ERROR)
#define SDHCI_INT_ALL_MASK  ((unsigned int)-1) & ~0x100

typedef enum sdhci_reg
{
    SDHCI_DMA_ADDRESS       = 0x00,
    SDHCI_BLOCK_SIZE        = 0x04,
    SDHCI_BLOCK_COUNT       = 0x06,
    SDHCI_ARGUMENT          = 0x08,
    SDHCI_TRANSFER_MODE     = 0x0c,
    SDHCI_COMMAND           = 0x0E,
    SDHCI_RESPONSE          = 0x10,
    SDHCI_RESPONSE1         = 0x14,
    SDHCI_RESPONSE2         = 0x18,
    SDHCI_RESPONSE3         = 0x1c,
    SDHCI_BUFFER            = 0x20,
    SDHCI_PRESENT_STATE     = 0x24,
    SDHCI_HOST_CONTROL      = 0x28,
    SDHCI_POWER_CONTROL     = 0x29,
    SDHCI_BLOCK_GAP_CONTROL = 0x2a,
    SDHCI_WAKE_UP_CONTROL   = 0x2b,
    SDHCI_CLOCK_CONTROL     = 0x2c,
    SDHCI_TIMEOUT_CONTROL   = 0x2e,
    SDHCI_SOFTWARE_RESET    = 0x2f,
    SDHCI_INT_STATUS        = 0x30,
    SDHCI_ERRINT_STATUS     = 0x32,
    SDHCI_INT_ENABLE        = 0x34,
    SDHCI_ERRINT_ENABLE     = 0x36,
    SDHCI_SIGNAL_ENABLE     = 0x38,
    SDHCI_ERRSIGNAL_ENABLE  = 0x3a,
    SDHCI_ACMD12_ERR        = 0x3c,
    SDHCI_HOST_CONTROL2     = 0x3e,
    SDHCI_CAPABILITIES      = 0x40,
    SDHCI_CAPABILITIES_1    = 0x44,
    SDHCI_MAX_CURRENT       = 0x48,
    SDHCI_SET_ACMD12_ERROR  = 0x50,
    SDHCI_SET_INT_ERROR     = 0x52,
    SDHCI_ADMA_ERROR        = 0x54,
    SDHCI_ADMA_ADDRESS      = 0x58,
    SDHCI_ADMA_ADDRESS_HI   = 0x5c,
    SDHCI_PRESET_FOR_SDR12  = 0x66,
    SDHCI_PRESET_FOR_SDR25  = 0x68,
    SDHCI_PRESET_FOR_SDR50  = 0x6a,
    SDHCI_PRESET_FOR_SDR104 = 0x6c,
    SDHCI_PRESET_FOR_DDR50  = 0x6e,
    SDHCI_PRESET_FOR_HS400  = 0x74,

    SPEC_VERSION_NUMBER     = 0xfe,
    VENDOR_DEFINE_REG0      = 0x100,
    VENDOR_DEFINE_REG1      = 0x104,
    VENDOR_DEFINE_REG2      = 0x108,
    VENDOR_DEFINE_REG3      = 0x10c,
    VENDOR_DEFINE_REG4      = 0x110,
    VENDOR_DEFINE_REG5      = 0x114,
    VENDOR_DEFINE_REG6      = 0x118,
    VENDOR_DEFINE_REG7      = 0x11c,
    VENDOR_DEFINE_REG8      = 0x120,
    VENDOR_DEFINE_REG9      = 0x124,
    DMA_HANDSHAKE_EN_REG    = 0x128,
    HW_ATTRIBUTES_REG       = 0x178,
    IP_REVISION_REG         = 0x17C,
    SDHCI_REG_MAX           = 0xFFFF
} SDHCI_REG_T;
/*
 * End of controller registers.
 */
//#define SDHCI_MAX_DIV_SPEC_200    256
#define SDHCI_MAX_DIV_SPEC_300  2046
/*
 * Host SDMA buffer boundary. Valid values from 4K to 512K in powers of 2.
 */
#define SDHCI_DEFAULT_BOUNDARY_SIZE  (512 * 1024)
#define SDHCI_DEFAULT_BOUNDARY_ARG   (7)

struct sdhci_host {
    u32 ioaddr;
    u32 quirks;
    u32 host_caps;
    u32 version;
    u32 clock;
    u32 voltages;
    int index;
    struct mmc *mmc;
};

static __inline void sdhci_writel(struct sdhci_host *host, u32 val, SDHCI_REG_T reg)
{
    (*((volatile unsigned int *)(host->ioaddr + reg)) = (unsigned int)(val));
}

static __inline void sdhci_writew(struct sdhci_host *host, u16 val, SDHCI_REG_T reg)
{
    (*((volatile unsigned short *)(host->ioaddr + reg)) = (unsigned short)(val));
}

static __inline void sdhci_writeb(struct sdhci_host *host, u8 val, SDHCI_REG_T reg)
{
    (*((volatile unsigned char *)(host->ioaddr + reg)) = (unsigned char)(val));
}

static __inline u32 sdhci_readl(struct sdhci_host *host, SDHCI_REG_T reg)
{
    return *((volatile unsigned int *)(host->ioaddr + reg));
}

static __inline u16 sdhci_readw(struct sdhci_host *host, SDHCI_REG_T reg)
{
    return *((volatile unsigned short *)(host->ioaddr + reg));
}

static __inline u8 sdhci_readb(struct sdhci_host *host, SDHCI_REG_T reg)
{
    return *((volatile unsigned char *)(host->ioaddr + reg));
}


#endif /* __SDHCI_H__ */
