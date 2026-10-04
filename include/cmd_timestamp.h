/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved. *
 */
 
#ifndef _CMD_TIMESTAMP_H_
#define _CMD_TIMESTAMP_H_
#include "stdio.h"
void timestamp_mark(const char *name, unsigned int line);
void timestamp_print(void);
bool timestamp_record_once_sram(unsigned int count, unsigned int offset, const char *name, unsigned int line);
unsigned int timestamp_sram_record(void); /* implement in board file */
#endif
