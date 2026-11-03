/**
    Loader project header file

    Loader project header file

    @file       prj_main.h
    @note       Nothing
*/

#ifndef _PRJ_MAIN_H
#define _PRJ_MAIN_H

#include "constant.h"
#include "loader.h"
//#include "GPIO.h"

// Define SPECIAL_KEY_UPDATE == ENABLE will require user to press special key
// to update firmware
#ifndef SPECIAL_KEY_UPDATE
#define SPECIAL_KEY_UPDATE          0
#endif

#ifndef SPI_PINMUX_NORMAL
#define SPI_PINMUX_NORMAL           (0)
#endif

#ifndef SPI_PINMUX_4BITS
#define SPI_PINMUX_4BITS            (1)
#endif

#ifndef SPI_PINMUX_SETTING
#define SPI_PINMUX_SETTING          (SPI_PINMUX_4BITS)
#endif

// Define MACROs for external storage
//
#ifndef STORAGE_EXT_UNKOWN
#define STORAGE_EXT_UNKOWN          (0)
#endif

#ifndef STORAGE_EXT_SDIO1
#define STORAGE_EXT_SDIO1           (1)
#endif

#ifndef STORAGE_EXT_UART
#define STORAGE_EXT_UART           	(2)
#endif

#ifndef STORAGE_EXT_USB
#define STORAGE_EXT_USB             (3)
#endif

#ifndef STORAGE_EXT_ETH
#define STORAGE_EXT_ETH             (4)
#endif

#ifndef STORAGE_EXT_TYPE
#define STORAGE_EXT_TYPE            (STORAGE_EXT_SDIO1)
#endif


// Card detect GPIO offset
#define DGPIO_CARD_DETECT           (0)// | GPIO_IS_DGPIO)

#define CHECK_CARD_EXIST            (0)

//check recovery trigger condition
#define CHECK_RECOVERY_TRIGGER      (0)

#define DEBUG_MSG					DISABLE

#endif
