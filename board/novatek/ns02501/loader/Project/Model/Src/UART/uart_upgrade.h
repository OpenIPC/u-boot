#ifndef __UART_UPGRADE_H
#define __UART_UPGRADE_H

/**
	Open NVT Device

	The commands except SCSIOP_IN_IS_NVT can be run if open device success.
	If not open first, the operation commands got CSW Status 0x1 means command failed.

	@DATA_PHASE: None.
	@CSW_PHASE status code: 0 is success. 0x1 means checksum failed.
*/
#define UARTOP_OUT_OPEN_DEVICE    0x01C0


/**
	Send Write Loader

	The Write Address @ CDB[2]~[5];
	For example address 0xFE080000 shall fill CDB[2]=0x00 CDB[3]=0x00 CDB[4]=0x08 CDB[5]=0xFE
	CDB[6]~[16] shall follow checksum rule. Content is dont care.

	@DATA_PHASE: OUT. Size is at CBW[8]~[11] DataTransferLength.
	@CSW_PHASE status code: 0 is success. 0x1 means checksum failed.
*/
#define UARTOP_OUT_WRLOADER       0x02C0

/**
	Address WRITE WORD (DRAM/REG/SRAM  dont care)

	The Write Address @ CDB[2]~[5];
	For example address 0xFE080000 shall fill CDB[2]=0x00 CDB[3]=0x00 CDB[4]=0x08 CDB[5]=0xFE
	The Write DATA @ CDB[6]~[9].
	For example address 0x12345678 shall fill CDB[6]=0x78 CDB[7]=0x56 CDB[8]=0x34 CDB[9]=0x12
	CDB[10]~[15] shall follow checksum rule. Content is dont care.

	@DATA_PHASE: None.
	@CSW_PHASE status code: 0 is success. 0x1 means checksum failed.
*/
#define UARTOP_OUT_ADDR_WRITE     0x03C0

/**
	Address READ WORD (DRAM/REG/SRAM  dont care)

	The READ Address @ CDB[2]~[5];
	For example address 0xFE080000 shall fill CDB[2]=0x00 CDB[3]=0x00 CDB[4]=0x08 CDB[5]=0xFE
	CDB[6]~[15] shall follow checksum rule. Content is dont care.

	@DATA_PHASE: IN 8 bytes return. DATA[0]~[3] is repeat CDB address. Read out DATA is [3] ~[7].
	@CSW_PHASE status code: 0 is success. 0x1 means checksum failed.
*/
#define UARTOP_IN_ADDR_READ     0x83C0


/**
	Ask if NVT Device

	CDB[2]~[15] shall follow checksum rule

	@DATA_PHASE: IN 4 bytes return; The value 0x81200000 woule be return for 98321. <8321 = 0x2081> If checksum fail, the 0x0 would be returned.
	@CSW_PHASE status code: 0 is success. 0x1 means checksum failed.

*/
#define UARTOP_IN_IS_NVT        0x81C0 // ???O???O???a?? device (?o?????n??  SCSIOP_IN_OPEN_DEVICE)


/**
	End USB ROM CODE.

	Exit USB ROM code after receiving this command. The checksum must be correct.

	CDB[2]~[15] shall follow checksum rule

	@DATA_PHASE: None.
	@CSW_PHASE status code: 0 is success and end usb rom.  0x1 means checksum failed.
*/
#define UARTOP_IN_WAIT_DONE     0x82C0

#define TOP_CTRL_VERSION_REG        (0xF00100F0)


typedef int  (*UART_Verify_CB)(unsigned int pCmdBuf, unsigned int *pDataBuf, unsigned int *IN_size);   ///< Callback for verify the Vendor Command is supported or not
typedef int  (*UART_Vendor_CB)(unsigned int pCmdBuf);                 ///< Callback for Vendor Command processing


extern void loader_installUARTCB(UART_Verify_CB callback);
extern void uart_upgrade_procedure(void);

static __inline unsigned int uart_read_reg(unsigned int reg)
{
	return *((volatile unsigned int *)reg);
}

static __inline void uart_write_reg(unsigned int reg, unsigned int value)
{
	*((volatile unsigned int *)reg) = value;
}



#endif
