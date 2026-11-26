/* Host stand-in for U-Boot's common.h: what the vendor display and HDMI TX
 * U-Boot sources use. */
#ifndef REGDUMP_COMMON_H
#define REGDUMP_COMMON_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "rec.h"
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef unsigned long ulong;
typedef uint32_t phys_addr_t;
#define udelay(us)      rec_delay_us(us)
#define mdelay(ms)      rec_delay_us((ms) * 1000)
#define __iomem
#define CONFIG_SYS_HZ   1000
static inline unsigned long get_timer(unsigned long base) { return rec_time_ms() - base; }
#endif
