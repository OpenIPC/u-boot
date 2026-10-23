/**
    Header file for RTC module.

    This file is the header file that define the API and data type
    for RTC module.

    @file       rtc.h
    @ingroup    mIHALRTC
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/

#ifndef __RTC_H
#define __RTC_H
#include "IOReg.h"

/**
    @addtogroup mIHALRTC
*/
//@{

/**
    @name RTC power on source

    RTC power on source

    @note For rtc_getPWRONSource()
*/
//@{
#define RTC_PWR_SW_STS      0x00000100              ///< Power on from switch
#define RTC_PWR_SW2_STS     0x00000200              ///< power on from power switch 2
#define RTC_PWR_SW3_STS     0x00000400              ///< power on from power switch 3 (Usually for USB VBUS, depend on project)
#define RTC_PWR_VBUS_STS    (RTC_PWR_SW3_STS)       ///< power on from VBUS (identical to RTC_PWR_SW3_STS)
#define RTC_PWR_SW4_STS     0x00000800              ///< power on from power switch 4 (Usually for VBAT, depend on project)
#define RTC_PWR_VBAT_STS    (RTC_PWR_SW4_STS)       ///< power on from VBAT (identical to RTC_PWR_SW4_STS)
#define RTC_PWR_ALARM_STS   0x00010000              ///< Power on from PWR ALARM
//@}

extern void     rtc_resetShutDownTimer(void);
extern void     rtc_poweroffPWR(void);
extern UINT32   rtc_getPWRONSource(void);
extern UINT32   rtc_chkS3boot_init(void) __attribute__ ((section (".part1")));
extern void     rtc_chkS3boot_wait_init(void) __attribute__ ((section (".part1")));
extern UINT32   rtc_chkS3boot(void) __attribute__ ((section (".part1")));

//@}
#endif  // __RTC_H

