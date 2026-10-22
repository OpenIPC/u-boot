/**
    string header

    string header

    @file       string.h
    @ingroup    mISYSClib
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/

#ifndef __NVT_LIBC_STRING_H__
#define __NVT_LIBC_STRING_H__

/** \addtogroup mISYSClib */
//@{

/**
    size_t data type

    data type to store value return by sizeof keyword
*/
typedef unsigned int size_t;

extern size_t strlen(const char *s);
extern size_t strnlen(const char *s, size_t count);
extern char *strncpy(char *dest, const char *src, size_t count);
extern char *strncat(char *dest, const char *src, size_t count);
extern char *strstr(const char *s1, const char *s2);
extern int strncmp(const char *cs, const char *ct, size_t count);
extern char *strrchr(const char *s, int c);
extern char *strcpy(char *dest, const char *src);
extern char *strcat(char *dest, const char *src);
extern char *strchr(const char *s, int c);
extern char* strtok(char *s, const char *delim);
extern int strcmp(const char *cs, const char *ct);

extern void *utl_memset(void *s, int c, size_t count);
extern void *utl_memcpy(void *dest, const void *src, size_t count);
extern int memcmp(const void *cs, const void *ct, size_t count);
extern void *memmove(void *dest, const void *src, size_t count);
extern void *memchr(const void *ptr, int value, size_t count);
//@}

#endif

