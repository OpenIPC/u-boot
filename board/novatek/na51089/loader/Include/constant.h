#ifndef _CONSTANT_H
#define _CONSTANT_H

//Size MNEMONIC

#define SZ_1                    (1 << 0)    /* 0x00000001 */
#define SZ_2                    (1 << 1)    /* 0x00000002 */
#define SZ_4                    (1 << 2)    /* 0x00000004 */
#define SZ_8                    (1 << 3)    /* 0x00000008 */
#define SZ_16                   (1 << 4)    /* 0x00000010 */
#define SZ_32                   (1 << 5)    /* 0x00000020 */
#define SZ_64                   (1 << 6)    /* 0x00000040 */
#define SZ_128                  (1 << 7)    /* 0x00000080 */
#define SZ_256                  (1 << 8)    /* 0x00000100 */
#define SZ_512                  (1 << 9)    /* 0x00000200 */

#define SZ_1K                   (1 << 10)   /* 0x00000400 */
#define SZ_2K                   (1 << 11)   /* 0x00000800 */
#define SZ_4K                   (1 << 12)   /* 0x00001000 */
#define SZ_8K                   (1 << 13)   /* 0x00002000 */
#define SZ_16K                  (1 << 14)   /* 0x00004000 */
#define SZ_32K                  (1 << 15)   /* 0x00008000 */
#define SZ_64K                  (1 << 16)   /* 0x00010000 */
#define SZ_128K                 (1 << 17)   /* 0x00020000 */
#define SZ_256K                 (1 << 18)   /* 0x00040000 */
#define SZ_512K                 (1 << 19)   /* 0x00080000 */

#define SZ_1M                   (1 << 20)   /* 0x00100000 */
#define SZ_2M                   (1 << 21)   /* 0x00200000 */
#define SZ_4M                   (1 << 22)   /* 0x00400000 */
#define SZ_8M                   (1 << 23)   /* 0x00800000 */
#define SZ_16M                  (1 << 24)   /* 0x01000000 */
#define SZ_32M                  (1 << 25)   /* 0x02000000 */
#define SZ_64M                  (1 << 26)   /* 0x04000000 */
#define SZ_128M                 (1 << 27)   /* 0x08000000 */
#define SZ_256M                 (1 << 28)   /* 0x10000000 */
#define SZ_512M                 (1 << 29)   /* 0x20000000 */

#define SZ_1G                   (1 << 30)   /* 0x40000000 */
#define SZ_2G                   (1 << 31)   /* 0x80000000 */

#define SZ_30M                  (SZ_32M - SZ_2M)

// ASCII code

#define CR                      0x0D
#define LF                      0x0A
#define BS                      0x08
#define ESC                     27

// Boolean constant definition

#if 0
#ifndef FALSE
#define FALSE                   0
#endif
#ifndef TRUE
#define TRUE                    1
#endif
#endif

#ifndef NULL
#define NULL                    0
#endif

#ifndef ON
#define ON                      1
#endif

#ifndef OFF
#define OFF                     0
#endif

#ifndef ENABLE
#define ENABLE                  1
#endif

#ifndef DISABLE
#define DISABLE                 0
#endif

#ifndef DISK_FULL
#define DISK_FULL               (-1)
#endif

// type declaration

typedef int                     BOOLEAN;
typedef unsigned long long      UINT64;
typedef long long               INT64;
typedef unsigned long           UINT32;
typedef unsigned long           INT32U;
typedef unsigned                INT32S;
typedef unsigned char           INT8U;
typedef long                    INT32;
typedef unsigned short          UINT16;
typedef short                   INT16;
typedef unsigned char           UINT8;
typedef char                    INT8;
typedef unsigned long long  u64;
typedef unsigned int        u32;
typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        uint;
typedef unsigned short      ushort;
typedef unsigned char       uchar;
typedef unsigned int            UINT;
typedef int                     INT;

typedef enum {FALSE, TRUE} BOOL;

// error codes
typedef long                    ER;

#endif
