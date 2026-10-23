/**
    I/O Access header file

    I/O Access header file

    @file       IOReg.h
    @ingroup    mISYSUtil
    @note       Nothing
*/

#ifndef __IO_REG_H
#define __IO_REG_H

#include "constant.h"
#include "CMacro.h"
#include "Memory.h"

/**
    @addtogroup mISYSUtil
*/
//@{

/**
    @name Input 8bit IO register

    Input 8bit IO register

    @param[in] x        I/O address

    @return register value
*/
//@{
#define INREG8(x)       (*((volatile UINT8*)(x)))               ///< Read 8bits IO register
//@}

/**
    @name Output 8bit IO register

    Output to 8bit IO register

    @param[in] x        I/O address
    @param[in] y        Value to be output

    @return void
*/
//@{
#define OUTREG8(x, y)       (*((volatile UINT8*)(x)) = (y))     ///< Write 8bits IO register
//@}

/**
    @name Set bits to 8bit IO register

    Set bits to 8bit IO register

    @param[in] x        I/O address
    @param[in] y        Value to be set. y will be ORed to this address

    @return register value
*/
//@{
#define SETREG8(x, y)       OUTREG8((x), INREG8(x) | (y))       ///< Set 8bits IO register
//@}

/**
    @name Clear bits from 8bit IO register

    Clear bits from 8bit IO register

    @param[in] x        I/O address
    @param[in] y        Value to be clear. ~(y) will be ANDed to this address

    @return register value
*/
//@{
#define CLRREG8(x, y)       OUTREG8((x), INREG8(x) & ~(y))      ///< Clear 8bits IO register
//@}

/**
    @name Input 16bit IO register

    Input 16bit IO register

    @param[in] x        I/O address (should be 2 bytes alignment)

    @return register value
*/
//@{
#define INREG16(x)      (*((volatile UINT16*)(x)))              ///< Read 16bits IO register
//@}

/**
    @name Output 16bit IO register

    Output to 16bit IO register

    @param[in] x        I/O address (should be 2 bytes alignment)
    @param[in] y        Value to be output

    @return void
*/
//@{
#define OUTREG16(x, y)      (*((volatile UINT16*)(x)) = (y))    ///< Write 16bits IO register
//@}

/**
    @name Set bits to 16bit IO register

    Set bits to 16bit IO register

    @param[in] x        I/O address (should be 2 bytes alignment)
    @param[in] y        Value to be set. y will be ORed to this address

    @return register value
*/
//@{
#define SETREG16(x, y)      OUTREG16((x), INREG16(x) | (y))     ///< Set 16bits IO register
//@}

/**
    @name Clear bits from 16bit IO register

    Clear bits from 16bit IO register

    @param[in] x        I/O address (should be 2 bytes alignment)
    @param[in] y        Value to be clear. ~(y) will be ANDed to this address

    @return register value
*/
//@{
#define CLRREG16(x, y)      OUTREG16((x), INREG16(x) & ~(y))    ///< Clear 16bits IO register
//@}

/**
    @name Input 32bit IO register

    Input 32bit IO register

    @param[in] x        I/O address (should be 4 bytes alignment)

    @return register value
*/
//@{
#define INREG32(x)          (*((volatile UINT32*)(x)))          ///< Read 32bits IO register
//@}

/**
    @name Output 32bit IO register

    Output to 32bit IO register

    @param[in] x        I/O address (should be 4 bytes alignment)
    @param[in] y        Value to be output

    @return void
*/
//@{
#define OUTREG32(x, y)      (*((volatile UINT32*)(x)) = (y))    ///< Write 32bits IO register
//@}

/**
    @name Set bits to 32bit IO register

    Set bits to 32bit IO register

    @param[in] x        I/O address (should be 4 bytes alignment)
    @param[in] y        Value to be set. y will be ORed to this address

    @return register value
*/
//@{
#define SETREG32(x, y)      OUTREG32((x), INREG32(x) | (y))     ///< Set 32bits IO register
//@}

/**
    @name Clear bits from 32bit IO register

    Clear bits from 32bit IO register

    @param[in] x        I/O address (should be 4 bytes alignment)
    @param[in] y        Value to be clear. ~(y) will be ANDed to this address

    @return register value
*/
//@{
#define CLRREG32(x, y)      OUTREG32((x), INREG32(x) & ~(y))    ///< Clear 32bits IO register
//@}

//@}

#endif  // __IO_REG_H
