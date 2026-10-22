/**
    string header

    string header

    @file       string.h
    @ingroup    mISYSClib
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/

#ifndef __NVT_DEBUG_H__
#define __NVT_DEBUG_H__

#include "fuart.h"
/** \addtogroup mISYSClib */
//@{

extern void debug_set_console(CONSOLE_OBJ *p_obj);
extern void debug_err(char *str);
extern void debug_err_var(char *str, int var);
extern void debug_msg(char *str) __attribute__ ((section (".part1")));
extern void debug_msg_var(char *str, int var) __attribute__ ((section (".part1")));
extern void debug_dump_addr(UINT32 addr, UINT32 size);


//@}

#endif

