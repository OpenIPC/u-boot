/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
 */

#ifndef DDRTRN_CONSOLE_H
#define DDRTRN_CONSOLE_H

#define ALIGN_COUNT 4

static inline int isprint(char c)
{
	return (c >= ' ' && c <= '~');
}
static inline int isspace(char c)
{
	return (c == ' ' || (c >= '\t' && c <= '\r'));
}
static inline int isxdigit(char c)
{
	return ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'));
}
#endif /* DDRTRN_CONSOLE_H */