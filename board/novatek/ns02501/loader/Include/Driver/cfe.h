/**
    cfe module driver.

    This file is the driver of config engine module.

    @file       cfe.h
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2023.  All rights reserved.
*/

#ifndef _CFE_DEF_
#define _CFE_DEF_

#include "IOReg.h"

typedef enum {
	I2C 	= 0x10,
	I2C2 	= 0x11,
	I2C3 	= 0x12,
	SPI 	= 0x20,
	SPI2 	= 0x21,
	SPI3 	= 0x22,
	SIF		= 0x40,
	SIF2	= 0x41,
	SIF3	= 0x42,
	SIF4	= 0x43,
} AUTO_DESC_INTF;

typedef enum {
	CFE_RUNNING 	= 0,
	CFE_END,
	CFE_CMD_NOT_SUP,
	CFE_POL_TOUT,
	CFE_I2C_NACK,
	CFE_DES_ERR,
	CFE_NEVER_USED,
} CFE_STATUS;

/*  -------------------------------------------------------------------------------
 *  API
 *  -------------------------------------------------------------------------------
 */
 // code_sram:
 // 0 		-> config engine sram (2KB)
 // other	-> set other sram base address, exp: IFE(0xf07c0000)
void cfe_open(UINT32 code_sram);
void cfe_close(void);
void cfe_trig(void);

// write data to address
void cfe_write_reg(UINT32 address, UINT32 data);

// read data from address
// write (data | read_data) to address
void cfe_mask_set(UINT32 address, UINT32 data);

// read data from address
// write ( !(data) & read_data) to address
void cfe_mask_clear(UINT32 address, UINT32 data);

// delay 1/120M*cycle
void cfe_delay(UINT32 cycle);

// read data from address
// loop if(read_data & mask) neq 0
void cfe_mask_poll_0(UINT32 address, UINT32 mask);

// read data from address
// loop if(read_data | (!mask)) neq 0xffffffff
void cfe_mask_poll_1(UINT32 address, UINT32 mask);


void cfe_auto_desc(UINT32 desc_ddress, AUTO_DESC_INTF interface_sel);

// read address 32bit data to tmp
// tmp: 0~3
void cfe_read_reg(UINT32 tmp, UINT32 address);

// tmp = tmp & value
// tmp: 0~3
void cfe_and(UINT32 tmp, UINT32 value);

// tmp = tmp | value
// tmp: 0~3
void cfe_or(UINT32 tmp, UINT32 value);

// write tmp to address
// tmp: 0~3
void cfe_write_tmp(UINT32 tmp, UINT32 address);

// copy register from source to dest
void cfe_copy_reg(UINT32 source, UINT32 dest);

// Read data from address
// Write (data ^ read_data) to address
void cfe_xor_set(UINT32 address, UINT32 data);

// wait config engine finish
void cfe_wait_end(void);

// get status
// 0: still running
// 1: end
// 2: command not support
// 3: polling timeout
// 4: i2c nack
// 5: des err
// 6: not used
CFE_STATUS cfe_get_status(void);


//for test
void cfe_test(void);
#endif

