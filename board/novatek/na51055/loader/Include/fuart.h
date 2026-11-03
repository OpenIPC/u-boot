/**
    Header file for UART

    This file is the header file for UART driver

    @file       fuart.h
    @ingroup    mIIOUART
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/


#ifndef _FUART_H
#define _FUART_H

#include "constant.h"

/**
    @addtogroup mIIOUART
*/
//@{

/**
    @name Put system UART string

    Print string to UART

    @param[in] m        printed string
*/
//@{
#define uart_putSystemUARTStr(m)    fLib_PutSerialStr(m)    ///< Print string MACRO
//@}

/**
    @name Put system UART character

    Print character to UART

    @param[in] m        printed character
*/
//@{
#define uart_putSystemUARTChar(m)   fLib_PutSerialChar(m)   ///< Print character MACRO
//@}

extern void fLib_PutSerialChar(char Ch)    __attribute__ ((section (".part1")));
extern void fLib_PutSerialStr(char *Str)   __attribute__ ((section (".part1")));
extern void uart_openSystemUART(void);
extern void uart_openSystemUART2(void);

extern void uart_getChar_polling(char * c);
extern void uart_chkChar(char * c);
extern void uart_getStr_polling(char * c);
extern void uart_getBinary(char *pcString, UINT32 BufferLen);

//@}

#endif
