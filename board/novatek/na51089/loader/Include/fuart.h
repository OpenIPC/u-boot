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
#ifndef ENUM_DUMMY4WORD
#define ENUM_DUMMY4WORD(name)   E_##name = 0x10000000
#endif

typedef enum {
	CONSOLE_UART0,
	CONSOLE_UART1,

	ENUM_DUMMY4WORD(UART_CONSOLE_CH)
} UART_CONSOLE_CH;


typedef struct _CONSOLE {
	void (*hook)(void); 			///< start this object
	void (*putc)(char c); 			///< console input funciton pointer
	unsigned char (*getc)(void); 	///< console output function point
} CONSOLE_OBJ, *PCONSOLE_OBJ;



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

void serial_init(void);
unsigned char serial_getc(void);
void serial_putc(char c);
void serial2_init(void);
unsigned char serial2_getc(void);
void serial2_putc(char c);
PCONSOLE_OBJ get_uart_object(UART_CONSOLE_CH uart_ch);

//@}

#endif
