
#ifndef __USB_UPDATE_H
#define __USB_UPDATE_H
#include "scsi_op.h"


#define BIT0    0x00000001L
#define BIT1    0x00000002L
#define BIT2    0x00000004L
#define BIT3    0x00000008L
#define BIT4    0x00000010L
#define BIT5    0x00000020L
#define BIT6    0x00000040L
#define BIT7    0x00000080L
#define BIT8	0x00000100L
#define BIT9	0x00000200L
#define BIT10	0x00000400L
#define BIT11	0x00000800L
#define BIT12	0x00001000L
#define BIT13	0x00002000L
#define BIT14	0x00004000L
#define BIT15	0x00008000L
#define BIT16	0x00010000L
#define BIT17	0x00020000L
#define BIT18	0x00040000L
#define BIT19	0x00080000L
#define BIT20	0x00100000L
#define BIT21	0x00200000L
#define BIT22	0x00400000L
#define BIT23	0x00800000L


typedef int 	(*MSDC_Verify_CB)(unsigned int pCmdBuf, unsigned int *pDataBuf, unsigned int *IN_size);   ///< Callback for verify the Vendor Command is supported or not
typedef int 	(*MSDC_VenDone_CB)(unsigned int pCmdBuf);   ///< Callback for verify the Vendor Command is supported or not


// KEY_ILLEGAL_REQUEST: Additional key
#define ADDKEY_INVALID_CMD_OP_CODE       0x20
#define ADDKEY_INVALID_FIELD_IN_CMD      0x24

#define CBW_FLAG_IN                     0x80
#define CBW_FLAG_OUT                    0x00

// Endpoint or FIFO direction define
#define DIRECTION_IN             0 ///1
#define DIRECTION_OUT            1 ///0

#define EP0                      0x00
#define EP1                      0x01
#define EP2                      0x02
#define EP3                      0x03

// FIFO number define
#define FIFO0                    0x0
#define FIFO1                    0x1
#define FIFO2                    0x2
#define FIFO3                    0x3
#define FIFO4                    0x4
#define FIFO5                    0x5
#define FIFO6                    0x6
#define FIFO7                    0x7
#define FIFO8                    0x8
#define FIFO9                    0x9

#define OUT_EP               EP2
#define IN_EP                EP1
#define OUT_FIFO             FIFO2
#define IN_FIFO              FIFO0

typedef enum {
	MS13Case_1 = 0,
	MS13Case_2_3,
	MS13Case_4,
	MS13Case_5,
	MS13Case_6,
	MS13Case_7_8,
	MS13Case_9,
	MS13Case_10_13,
	MS13Case_11,
	MS13Case_12
} MassStorage13Case;

#define CBW_SIGNATE                     0x43425355
#define CSW_SIGNATE                     0x53425355
#define CSW_STATUS_CMD_PASS             0x00
#define CSW_STATUS_CMD_FAIL             0x01
#define CSW_STATUS_PHASE_ERROR          0x02

// Request Sense data format
#define SENSE_OFFSET_KEY              0x02
#define SENSE_OFFSET_ADD              0x0C
#define SENSE_OFFSET_ADD_QUALIFIER    0x0D

// KEY_NO_SENSE: Additional key
#define ADDKEY_NO_ADDITIONAL    0x00

// all response data length
#define DATA_LENGTH_INQUIRY              36
#define DATA_LENGTH_REQUEST_SENSE        18
#define DATA_LENGTH_MODE_SENSE           8

// SCSI command operation code
#define SCSI_OP_TEST_UNIT_READY         0x00
#define SCSI_OP_REQUEST_SENSE           0x03
#define SCSI_OP_INQUIRY                 0x12
#define SCSI_OP_MODE_SELECT             0x15
#define SCSI_OP_MODE_SENSE              0x1A
#define SCSI_OP_MEDIUM_REMOVAL          0x1E
#define SCSI_OP_READ_FORMAT_CAPACITY    0x23 // unsupported
#define SCSI_OP_READ_CAPACITY           0x25
#define SCSI_OP_READ_10                 0x28
#define SCSI_OP_WRITE_10                0x2A
#define SCSI_OP_VERIFY                  0x2F

#define CARD_INDEX_SRAM      7
#define CARD_INDEX_FSM_SS    3
#define CARD_TYPE_MAX_REPORT 1

// KEY_NOT_READY: Additional key
#define ADDKEY_LOGICAL_UNIT_NOT_READY    0x04
#define ADDKEY_LOGICAL_UNIT_NOT_SUPPORT  0x25
#define ADDKEY_MEDIUM_NOT_PRESENT        0x3A

// Sense key
#define KEY_NO_SENSE            0x00
#define KEY_RECOVERED_ERROR     0x01
#define KEY_NOT_READY           0x02
#define KEY_MEDIUM_ERROR        0x03
#define KEY_HARDWARE_ERROR      0x04
#define KEY_ILLEGAL_REQUEST     0x05
#define KEY_UNIT_ATTENTION      0x06
#define KEY_DATA_PROTECT        0x07
#define KEY_BLANK_CHECK         0x08
#define KEY_VENDOR_SPECIFIC     0x09
#define KEY_COPY_ABORTED        0x0A
#define KEY_ABORTED_CMD         0x0B
#define KEY_OBSOLETE            0x0C
#define KEY_VOLUMN_OVERFLOW     0x0D
#define KEY_MISCOMPARE          0x0E
#define KEY_RESERVED            0x0F

typedef struct	// SCSI device response
{
	unsigned int u8MemIndex;       // current access memory type
	unsigned int u8Flags;          // In or Out
	unsigned int u32IOAddr;         // block addr in Card controller
	unsigned int u16DataResidue;  // residue (block count for card content, or byte count for information)
	unsigned int u16TfSzCurrent;  // current transfer length (block count for card content, or byte count for information)
} SCSIDeviceResp;

typedef struct SCSI_Sense_Data
{
	unsigned int u8Key;
	unsigned int u8KeyAdd;
} SCSISense;

typedef enum {
	MS_STATE_IDLE             = 0,
	MS_STATE_CBW              = 1,
	MS_STATE_CB_DMA_IN        = 2,
	MS_STATE_CB_DMA_OUT       = 3,
	MS_STATE_CSW              = 4,
	MS_STATE_STALL            = 5,
	MS_STATE_BGD              = 6
} MassStorageState;


typedef struct CommandBlockWrapper
{
	unsigned int u32Signature;
	unsigned int u32Tag;
	unsigned int u32DataTransferLength;
	unsigned int CB0_cblen_lun_flag;
	unsigned int CB1_CB15[4];
	//unsigned char u8Flags;
	//unsigned char u8LUN;
	//unsigned char u8CBLength;
	//unsigned char u8CB[16];
} CBW;

typedef struct CommandStatusWrapper
{
	unsigned int u32Signature;
	unsigned int u32Tag;
	unsigned int u32DataResidue;
	unsigned int u8Status;
} CSW;

// Host Controller (HC)
typedef struct {
	INT32U cap;             // 0x00: capability
	INT32U ports;           // 0x04: Structural Parameter
	INT32U capp;            // 0x08: capability Parameter
	INT32U reserved1;       // 0X0c
	INT32U cmd;             // 0x10: USB command
	INT32U status;         // 0X14: status
	INT32U int_en;         // 0X18: interrupt enable
	INT32U fm_idx;         // 0X1c: frame Index
	INT32U reserved2;       // 0x20
	INT32U periodic;        // 0X24: periodic Frame List Base Address. must 4KB align.
	INT32U asynch;          // 0X28: Current Asynchronous List Address, must 32B align.
	INT32U reserved3;       // 0X2c
	INT32U port;            // 0x30: port status and Control
	INT32U reserved4[3];    // 0X34 ~ 0X3c
	INT32U misc;            // 0x40
	INT32U reserved5[15];   // 0x44 ~ 0x7C
} HC;

// OTG Controller (OTG)
typedef struct {
	INT32U ctl_st;          // 0x80: control/status
	INT32U int_st;          // 0x84: interrupt status
	INT32U int_en;          // 0x88: interrupt enable
	INT32U reserved[13];    // 0x8C ~ 0xBC
} OTG;

// Global Controller (GL)
typedef struct {
	INT32U int_st;          // 0xC0: interrupt status
	INT32U interrupt;       // 0xC4: interrupt mask
	INT32U reserved[14];    // 0xC8 ~ 0xFC
} GL;

// Device Controller (DEV)
typedef struct {
	INT32U main_ctl;        // 0x100: device main control
	INT32U dev_adr;         // 0x104: device address
	INT32U tst_ep;          // 0x108: device test
	INT32U frm_no;          // 0x10C: device SOF            [13: 11] SOF micro frame number, [10: 0] SOF frame number
	INT32U sof_tmsk;        // 0x110: device SOF mask timer [15: 0] SOF mask timer
	INT32U phy_tms;         // 0x114: PHY test mode selector
	INT32U vnd_ctl;         // 0x118: vendor specific IO control
	INT32U cx_cs;           // 0x11C: CX configuration and status
	INT32U cx_cf;           // 0x120: CX configuration and FIFO empty status
	INT32U idle_cnt;        // 0x124: Device idel counter
	INT32U cx_dataport;     // 0x128: device cx data register
	INT32U reserved1[1];    // 0x128 ~ 0x12C
	INT32U int_mgrp;        // 0x130: mask of interrupt group
	INT32U int_mgrp0;       // 0x134: mask of interrupt source group 0
	INT32U int_mgrp1;       // 0x138: mask of interrupt source group 1
	INT32U int_mgrp2;       // 0x13C: mask of interrupt source group 2
	INT32U int_grp;         // 0x140: interrupt group
	INT32U int_grp0;        // 0x144: interrupt group 0
	INT32U int_grp1;        // 0x148: interrupt group 1
	INT32U int_grp2;        // 0x14C: interrupt group 2
	INT32U rx0byte;         // 0x150: receive zero-length data packet
	INT32U tx0byte;         // 0x154: rtransfer zero-length data packet
	INT32U iso_seq_err;     // 0x158: iso. sequential error/abort
	INT32U reserved2;       // 0x15C
	INT32U ep_xpsz[2][8];   // 0x160: IN endpoint x MaxPacketSize Reg
                                // 0x170: IN endpoint x MaxPacketSize Reg
                                // 0x180: OUT endpoint x MaxPacketSize Reg
                                // 0x190: OUT endpoint x MaxPacketSize Reg
	INT8U ep_map[8];        // 0x1A0: endpoint 1 ~ 8 map Reg
	INT8U fifo_map[4];      // 0x1A8: FIFO 0 ~ 3 map Reg
	INT8U fifo_cfg[4];      // 0x1AC: FIFO 0 ~ 3 configuration
	INT32U fifo_bc[4];      // 0x1B0: FIFO x instruction and byte count
	INT32U dma_fifo;        // 0x1C0: dma target FIFO number
	INT32U reserved3;       // 0x1C4
	INT32U dma_ctl;         // 0x1C8: DMA controller parameter setting 1
	INT32U dma_addr;        // 0x1CC: DMA controller parameter setting 2, address
	INT32U dma_data;        // 0x1D0: DMA controller parameter setting 3, Cx data
} DEV;

// OTG Controller
typedef struct {
	HC hc;                  // 0x00 ~ 0x7F
	OTG otg;                // 0x80 ~ 0xBF
	GL gl;                  // 0xC0 ~ 0xFF
	DEV dev;                // 0x100 ~ 0x1FF
} OTG200;



typedef enum
{
	ACT_IDLE,
	ACT_DONE,
	ACT_STALL,

	ACT_DUMMY = 0x80000000
} Action;

typedef enum
{
	CMD_VOID,              // No command
	CMD_GET_DESCRIPTOR,    // Get_Descriptor command
	CMD_SET_DESCRIPTOR,    // Set_Descriptor command

	CommandType_dummy = 0x80000000
} CommandType;

typedef struct USB_s
{
	// end pointer 0 information
	unsigned int  u32UsbCmd[8/4];
	unsigned int bUsbChirpFinish;
	Action eUsbCxAction;
	unsigned int u16TxRxCounter;
	unsigned char * pu8Descriptor;
	unsigned int u8UsbConfigValue;
	unsigned int u8UsbInterfaceValue;
	unsigned int u8UsbInterfaceAlternateSetting;
	unsigned int bUsbEP0HaltSt;
	unsigned int bHighSpeed;			// high speed
	CommandType eUsbCxCommand;
	// for display
	unsigned int u8LineCount;
} USB_st;







#endif /* __USB_OTG_H  */






