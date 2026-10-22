#ifndef _SCSI_OP_H
#define _SCSI_OP_H

/**
Only Support MSDC Vendor command code 0xF0/0xE0/0xC0.
Others vendor code would STALL OUT EP. Sense data would return illegal request sense code.
*/
#define SCSI_OP_SET0				0xF0
#define SCSI_OP_SET1				0xE0
#define SCSI_OP_SET2				0xC0


///////////////////////////////////////////////////////////////////////////////
// Uart's return status: CDB[13]
// Uart's extra data len: CDB[6] ~ CDB[9] except SCSIOP_OUT_ADDR_WRITE
// Checksum rule: The CDB[0] ~ CDB[15]'s sum LSB 8bits shall be 0x00
// Checksum is fixed CDB[14] ~ CDB[15]
// When Checksum failed. CSW Status 0x1 means command failed.
///////////////////////////////////////////////////////////////////////////////
#define IDX_CDB_UART_DATA_LEN_LWLB  6 ///< uart's data len low word low byte
#define IDX_CDB_UART_DATA_LEN_LWHB  7 ///< uart's data len low word high byte
#define IDX_CDB_UART_DATA_LEN_HWLB  8 ///< uart's data len high word low byte
#define IDX_CDB_UART_DATA_LEN_HWHB  9 ///< uart's data len high word low byte
#define IDX_CDB_UART_STATUS_ER     13 ///< uart's csw status
#define IDX_CDB_CHKSUM_LB          14 ///< check sum low bytes
#define IDX_CDB_CHKSUM_HB          15 ///< check sum high bytes

///////////////////////////////////////////////////////////////////////////////
// SCSI CBW
///////////////////////////////////////////////////////////////////////////////
typedef struct _NVT_SCSI_CBW {
	unsigned int  dCBWSignature;
	unsigned int  dCBWTag;
	unsigned int  dCBWDataTransferLength;
	unsigned char bmCBWFlags;
	unsigned char bCBWLUN;
	unsigned char bCBWCBLength;
	unsigned char CBWCB[16];
} NVT_SCSI_CBW;

/**
Open NVT Device

The commands except SCSIOP_IN_IS_NVT can be run if open device success.
If not open first, the operation commands got CSW Status 0x1 means command failed.

@DATA_PHASE: None.
@CSW_PHASE status code: 0 is success. 0x1 means checksum failed.
*/
#define SCSIOP_OUT_OPEN_DEVICE 0x01


/**
Send Write Loader

The Write Address @ CDB[2]~[5];
For example address 0xFE080000 shall fill CDB[2]=0x00 CDB[3]=0x00 CDB[4]=0x08 CDB[5]=0xFE
MSDC: CDB[6]~[15] shall follow checksum rule. Content is dont care.
UART: CDB[6]~[9] is data size (because uart has no CBW to tell transmitting size)

@DATA_PHASE: None.
@CSW_PHASE status code: 0 is success. 0x1 means checksum failed.
*/
#define SCSIOP_OUT_WRLOADER 0x02

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
#define SCSIOP_OUT_ADDR_WRITE 0x03

/**
Write Memory

The Write Starting Address @ CDB[2]~[5];
For example offset 0xFE080000 shall fill CDB[2]=0x00 CDB[3]=0x00 CDB[4]=0x08 CDB[5]=0xFE
MSDC: CDB[6]~[15] shall follow checksum rule. Content is dont care.
UART: CDB[6]~[9] is data size (because uart has no CBW to tell transmitting size)

@DATA_PHASE: None.
@CSW_PHASE status code: 0 is success. 0x1 means checksum failed.
*/
#define SCSIOP_OUT_MEM_WRITE 0x04

/**
Read / Write flash

send SCSIOP_FLASH_OP to loader
MSDC: CDB[6]~[15] shall follow checksum rule. Content is dont care. except CDB[10]
UART: CDB[6]~[9] is SCSIOP_FLASH_OP size (because uart has no CBW to tell transmitting size)

@DATA_PHASE: None.
@CSW_PHASE status code: 0 is success. 0x1 means checksum failed.
*/
#define FLASH_ACCESS_TYPE_AUTO 0
#define FLASH_ACCESS_TYPE_NAND 1
#define FLASH_ACCESS_TYPE_NOR  2
#define FLASH_ACCESS_TYPE_EMMC 3
typedef struct _SCSIOP_FLASH_OP {
	unsigned int type; //FLASH_ACCESS_TYPE
	unsigned int is_write; //indicate write
	unsigned long long offset; //flash r/w offset
	unsigned long long size;   //flash r/w size
	unsigned long long partition_size; //flash partition size
} SCSIOP_FLASH_OP;
#define SCSIOP_OUT_FLASH_ACCESS 0x05


/**
Ask if NVT Device

No need to SCSIOP_IN_OPEN_DEVICE
CDB[2]~[15] shall follow checksum rule

@DATA_PHASE: IN 4 bytes return; The value 0x81200000 woule be return for 98321. <8321 = 0x2081> If checksum fail, the 0x0 would be returned.
@CSW_PHASE status code: 0 is success. 0x1 means checksum failed.

*/
#define SCSIOP_IN_IS_NVT 0x81


/**
End USB ROM CODE.

Exit USB ROM code after receiving this command. The checksum must be correct.
After SCSIOP_IN_WRLOADER is invoked, PC invoked this to confirm write loader is ok.

CDB[2]~[15] shall follow checksum rule

@DATA_PHASE: None.
@CSW_PHASE status code: 0 is success and end usb rom.  0x1 means checksum failed.
*/
#define SCSIOP_IN_WAIT_DONE 0x82


/**
Address READ WORD (DRAM/REG/SRAM  dont care)

The READ Address @ CDB[2]~[5];
For example address 0xFE080000 shall fill CDB[2]=0x00 CDB[3]=0x00 CDB[4]=0x08 CDB[5]=0xFE
CDB[6]~[15] shall follow checksum rule. Content is dont care.

@DATA_PHASE: IN 8 bytes return. DATA[0]~[3] is repeat CDB address. Read out DATA is [4] ~[7].
@CSW_PHASE status code: 0 is success. 0x1 means checksum failed.
*/
#define SCSIOP_IN_ADDR_READ 0x83


/**
The Read Starting Address @ CDB[2]~[5];
For example address 0xFE080000 shall fill CDB[2]=0x00 CDB[3]=0x00 CDB[4]=0x08 CDB[5]=0xFE
CDB[6]~[9] is read data size
CDB[10]~[15] shall follow checksum rule. Content is dont care.

@DATA_PHASE: IN 8 bytes return. DATA[0]~[3] is repeat CDB address. Read out DATA is [3] ~[7].
@CSW_PHASE status code: 0 is success. 0x1 means checksum failed.

*/
#define SCSIOP_IN_MEM_READ 0x84


/**
Get Temp Buffer Address

MSDC: CDB[6]~[15] shall follow checksum rule. Content is dont care.
UART: CDB[6]~[9] is data size (because uart has no CBW to tell transmitting size)

@DATA_PHASE: Read out DATA is [3] ~[7].
@CSW_PHASE status code: 0 is success. 0x1 means checksum failed.
*/
#define SCSIOP_IN_GET_TMP_ADDR 0x85

/**
Notify fdt has send

The fdt has sent into loader's temp buffer. notify loader to check its sanity.
CDB[2]~[5] is status result in word.
MSDC: CDB[6]~[15] shall follow checksum rule. Content is dont care.
UART: CDB[6]~[9] is data size (because uart has no CBW to tell transmitting size)

@DATA_PHASE: None.
@CSW_PHASE status code: 0 is success. 0x1 means checksum failed.
*/
#define SCSIOP_IN_FDT 0x86


/**
Notify uboot or nvtpack has send

The uboot or nvtpack has sent into loader's temp buffer. notify loader to check its sanity.
CDB[2]~[5] is status result in word.
MSDC: CDB[6]~[15] shall follow checksum rule. Content is dont care.
UART: CDB[6]~[9] is data size (because uart has no CBW to tell transmitting size)
*/
#define SCSIOP_IN_UBOOT 0x87

#endif