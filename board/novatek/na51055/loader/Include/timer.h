/**
    Public header file for timer module.

    This file is the header file that define the API and data type for timer
    module.

    @file       timer.h
    @ingroup    mIHALTimer
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/

#ifndef _TIMER_H
#define _TIMER_H

#include "IOReg.h"

/**
    @addtogroup mIHALTimer
*/
//@{

extern UINT32 timer_getLdrElapse(void);
extern void timer_delay(UINT32 US);
#define timer_getLdrElapse()	INREG32(0xF0040108)
#define timer_getSysTick()		INREG32(0xF0040108)
//extern UINT32 timer_getSysTick(void)  __attribute__ ((section (".part1")));
 //@}

#endif
