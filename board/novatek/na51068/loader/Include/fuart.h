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

#include "board.h"

/**
    @addtogroup mIIOUART
*/
 /**
    Macro to generate dummy element for enum type to expand enum size to word (4 bytes)
    */
    //@{
    #ifndef ENUM_DUMMY4WORD
    #define ENUM_DUMMY4WORD(name)   E_##name = 0x10000000
    #endif
    //@}
//@}
//@{
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
//#define uart_putSystemUARTStr(m)    fLib_PutSerialStr(m)    ///< Print string MACRO
//@}

/**
    @name Put system UART character

    Print character to UART

    @param[in] m        printed character
*/
//@{
#define fLib_PutSerialChar(m)		serial_putc(m)
#define uart_putSystemUARTChar(m)   fLib_PutSerialChar(m)   ///< Print character MACRO
//@}




void serial_init(void);
unsigned char serial_getc(void);
void serial_putc(char c);
void uart_putSystemUARTStr(char *str);
PCONSOLE_OBJ get_uart_object(UART_CONSOLE_CH uart_ch);


//@}

#endif
