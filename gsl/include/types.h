/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
 */
#ifndef __TYPES_H__
#define __TYPES_H__

typedef unsigned long int uintptr_t;

typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

typedef signed long long s64;
typedef unsigned long long u64;

typedef unsigned long size_t;
typedef int ptrdiff_t;

typedef int errno_t;
typedef errno_t                 bool;

#define BITS_PER_LONG 32

/* Dma addresses are 32-bits wide.  */
typedef u32 dma_addr_t;

#undef NULL
#if defined(__cplusplus)
#define NULL 0
#else
#define NULL ((void *)0)
#endif

#define TRUE    1
#define FALSE   0

#define ERROR   (-1)
#define OK      0

#define DATA_EMPTY	(-2)

#define AUTH_SUCCESS	(0x3CA5965A)
#define AUTH_FAILURE	(0xC35A69A5)

#define FOREVER         while (1)
#define IN
#define OUT

#define PRIVATE static

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef   long long      int64_t;
typedef   unsigned   long long       uint64_t;

typedef unsigned char u_char;
typedef unsigned long u_long;
typedef unsigned int u_int;
typedef unsigned char u_int8_t;
typedef unsigned short u_int16_t;
typedef unsigned int u_int32_t;
typedef int ssize_t;
typedef long loff_t;

#endif

