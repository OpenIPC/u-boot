/**
    System Clock APIs header file

    System Clock APIs header.

    @file       clock.h
    @ingroup    mIHALSysCG
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/

#ifndef __CLOCK_H
#define __CLOCK_H

#include "IOReg.h"

/** \addtogroup mIHALSysCG */
//@{
// For SM host clock rate
#define GZ_CLOCK_RATE_240_MHZ       (0x00 << 12)    
#define GZ_CLOCK_RATE_320_MHZ       (0x01 << 12)    
#define GZ_CLOCK_RATE_480_MHZ       (0x02 << 12)    
#define GZ_CLOCK_RATE_MASK          (0x03 << 12)


#define enableGZReset()             CLRREG32(CLOCK_GEN_REG_BASE_ADDR + 0x8C, (1 << 7))
#define disableGZReset()            SETREG32(CLOCK_GEN_REG_BASE_ADDR + 0x8C, (1 << 7))

#define enableGZClock()             SETREG32(CLOCK_GEN_REG_BASE_ADDR + 0x78, (1 << 11))
#define disableGZClock()            CLRREG32(CLOCK_GEN_REG_BASE_ADDR + 0x78, (1 << 11))
//@}

#endif  // __CLOCK_H
