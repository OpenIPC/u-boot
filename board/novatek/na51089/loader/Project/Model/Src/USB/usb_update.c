
//#include "board.h"
#include "fuart.h"
#include "usb_update.h"
#include "debug.h"
#include "timer.h"

#define IOADDR_USB_REG_BASE         (0xFF600000)
#define _THUMB2 __attribute__((target("thumb2")))


_THUMB2 void fLib_USB_Update_FW(void);

int usbrom_exit = 0;
volatile DEV *ptusbdev;
#define ptotg ((volatile OTG200 *)IOADDR_USB_REG_BASE)

#define FUSB200_MAX_EP           8   // 1..10
#define FUSB200_MAX_FIFO         4   // 0.. 9

#define mUsbEPMap(EPn, MAP)	(ptusbdev->ep_map[EPn-1] = MAP)
#define mUsbEPMapRd(EPn)		(ptusbdev->ep_map[EPn-1])

#define mUsbFIFOMap(FIFOn, MAP)	(ptusbdev->fifo_map[FIFOn] = MAP)
#define mUsbFIFOMapRd(FIFOn)		(ptusbdev->fifo_map[FIFOn])
#define mUsbFIFOConfigRd(FIFOn)		(ptusbdev->fifo_cfg[FIFOn])

static USB_st tusb;
static MassStorageState eUsbMassStorageState = MS_STATE_CBW;
CBW tCBW;
CSW tCSW;
SCSISense tSCSIsense;
SCSIDeviceResp tSCSIDeviceResp;

MSDC_Verify_CB      guiU2MsdcCheck_cb;
MSDC_VenDone_CB     guiU2MsdcVendorDone_cb;

int trigger_done_cb = 0;

static const unsigned char u8RequestSenseData[DATA_LENGTH_REQUEST_SENSE] __attribute__((aligned(4))) =
{
	0x70,    // 0, response code
	0x00,    // 1, obsolete
	0x00,    // 2, sense key
	0x00,    // 3-6, information

	0x00,
	0x00,
	0x00,
	0x0A,    // 7, additional length (n - 7), n = 17

	0x00,    // 8-11, information
	0x00,
	0x00,
	0x00,

	0x00,    // 12, additional sense code
	0x00,    // 13, additional sense code qualifier
	0x00,    // 14, field replaceable unit code
	0x00,    // 15-17, sense-key specific

	0x00,
	0x00
};


static const unsigned char u8ModeSenseData[DATA_LENGTH_MODE_SENSE] __attribute__((aligned(4))) =
{
	0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


#define USB_BUFF_SZ 128
unsigned int u32Usb_buffer[USB_BUFF_SZ/4]; // may less than USB_BUFF_SZ

static INT32U nvtdev_opened = 1;
#define mCxType(cmd)            ((cmd[0] & (BIT6 | BIT5)) >> 5)
#define mCxRequest(cmd)         ((unsigned char)(cmd[0] >> 8))
#define mCxValue(cmd)           ((unsigned short)(cmd[0] >> 16)) // get 3rd byte, 4th byte
#define mCxIndex(cmd)           ((unsigned short)(cmd[1]))       // get 5th byte, 6th byte
#define mCxLength(cmd)          ((unsigned short)(cmd[1] >> 16)) // get 7th byte, 8th byte


#define mUsbRmWkupST()		(ptusbdev->main_ctl & BIT0)
#define mUsbRmWkupClr()		(ptusbdev->main_ctl &= ~BIT0)
#define mUsbRmWkupSet()		(ptusbdev->main_ctl |= BIT0)
#define mUsbTsMdWr(item)		(ptusbdev->phy_tms = item)
#define mUsbIntF2OUTDis()		(ptusbdev->int_mgrp1 |= (BIT5 | BIT4))
#define mUsbIntF2OUTEn()		(ptusbdev->int_mgrp1 &= ~(BIT5 | BIT4))
#define mUsbIntF0INDis()		(ptusbdev->int_mgrp1 |= BIT16)
#define mUsbIntF0INEn()		(ptusbdev->int_mgrp1 &= ~BIT16)
// Endpoint & FIFO Configuration
#define mUsbEPMxPtSz(EPn, dir, size)	(ptusbdev->ep_xpsz[dir][EPn - 1] = size)
#define mUsbEPStallST(EPn, dir)		(((ptusbdev->ep_xpsz[dir][EPn - 1]) & BIT11) >> 11)
#define mUsbEPRsTgSet(EPn, dir)	(ptusbdev->ep_xpsz[dir][EPn-1] |=  BIT12)
#define mUsbEPRsTgClr(EPn, dir)		(ptusbdev->ep_xpsz[dir][EPn-1] &= ~BIT12)
#define mUsbEPStallClr(EPn, dir)		(ptusbdev->ep_xpsz[dir][EPn-1] &= ~BIT11)
#define mUsbEPStallSet(EPn, dir)		(ptusbdev->ep_xpsz[dir][EPn-1] |=  BIT11)

extern void CPUflushReadCache(UINT32 uiStartAddr, UINT32 uiLength);
extern void CPUflushWriteCache(UINT32 uiStartAddr, UINT32 uiLength);

INT32S my_SCSICmd(CBW * ptcbw);

_THUMB2 signed int usb_DxFifoRdWr(unsigned char * buf, unsigned int count, unsigned int Rd)
{
	signed int ret = -1;	// assume fail

	//#ifdef USBROM_RTOS_DBG
	//debug_msg("MSG: enter usb_DxFifoRdWr\r\n");
	//#endif

	CPUflushWriteCache((UINT32) buf, (UINT32) count);
	CPUflushReadCache((UINT32) buf, (UINT32) count);

	//#ifdef __FREERTOS
	//if(buf == (unsigned char *)0xFE080000)
	//	ptusbdev->dma_addr = (INT32U)buf;
	//else
	//	ptusbdev->dma_addr = dma_getPhyAddr(buf);

	//dma_flushWriteCache((UINT32)buf, count);
	//dma_flushReadCache((UINT32)buf, count);
	//#else
	ptusbdev->dma_addr = (INT32U)buf;//dma_getPhyAddr(buf);

	__asm__ __volatile__("dsb\n\t");
	//#endif


	if (Rd) {
		// fifo select
		ptusbdev->dma_fifo = BIT2;
		// ctrl setting
		ptusbdev->dma_ctl = (count << 8) |		// dma byte count
							(0 << 2) |		    // not dma io to io
							(0 << 1);			// fifo 2 memory

		#ifdef USBROM_RTOS_DBG
		debug_msg("MSG: count = %d, Read CTRL = %x\r\n", count, ptusbdev->dma_ctl);
		#endif

	} else {
		// fifo select
		ptusbdev->dma_fifo = BIT0;
		// ctrl setting
		ptusbdev->dma_ctl = (count << 8) |	// dma byte count
							(0 << 2) |		// not dma io to io
							(1 << 1);		// memory 2 fifo

		#ifdef USBROM_RTOS_DBG
		debug_msg("MSG: Write count = %d, Write CTRL = %x\r\n", count, ptusbdev->dma_ctl);
		#endif
	}

	// dma start
	ptusbdev->dma_ctl |= BIT0;

	while (1) {
		INT32U temp;

		temp = ptusbdev->int_grp2 & ~ ptusbdev->int_mgrp2;

		if (temp & BIT7) {	// dma completion
			ret = 0;		// ok
			break;
		}
		if (temp & (BIT2 | BIT1)) {	// bus condiction
			// clear cx fifo
			ptusbdev->cx_cf |= BIT3;
			break;
		}
		if (temp & BIT8) {	// dma error
			// clear cx fifo
			ptusbdev->cx_cf |= BIT3;
			break;
		}
	}

	// clear (dma error & dma completion) interrupt
	ptusbdev->int_grp2 |= (BIT8 | BIT7);
	CPUflushReadCache((UINT32) buf, (UINT32) count);

	//debug_msg("done\r\n");

	//#ifdef __FREERTOS
	//dma_flushReadCache((UINT32)buf, count);
	//#endif

	return ret;
}

_THUMB2 void vSCSIHWError(void)
{
	tCSW.u8Status = CSW_STATUS_CMD_FAIL;
	if ((tCBW.CB0_cblen_lun_flag&0xFF) == CBW_FLAG_OUT) {
		mUsbEPStallSet(OUT_EP, DIRECTION_OUT);
	} else {
		//debug_msg("S1\r\n");
		mUsbEPStallSet(IN_EP, DIRECTION_IN);
	}

	//#ifdef USBROM_RTOS_DBG
	//DBG_DUMP("MSG: vSCSIHWError\r\n");
	//#endif

	tSCSIsense.u8Key = KEY_NOT_READY;
	tSCSIsense.u8KeyAdd = ADDKEY_MEDIUM_NOT_PRESENT;
}


//////////////////////////////////////////////////////////////////////////////////
_THUMB2 MassStorageState eScsiIn(void)
{

	switch (tSCSIDeviceResp.u8MemIndex) {
		case CARD_INDEX_SRAM:
			// unit: byte
			if (usb_DxFifoRdWr((INT8U *)u32Usb_buffer, tSCSIDeviceResp.u16DataResidue, 0) >= 0) {
				tCSW.u32DataResidue -= tSCSIDeviceResp.u16DataResidue;
				if (tCSW.u32DataResidue) {
					// wait for fifo empty
					while ((ptusbdev->cx_cf & (BIT8 | BIT9)) != (BIT8 | BIT9)) {
						// if (VBUS == 0)
						//	return MS_STATE_CSW;
					}
					//debug_msg("S2\r\n");
					//mUsbEPStallSet(IN_EP, DIRECTION_IN); // case MS13Case_5:
				}
				return MS_STATE_CSW;
			}
			break;

		case CARD_INDEX_FSM_SS:
			if (usb_DxFifoRdWr((INT8U *)tSCSIDeviceResp.u32IOAddr, tSCSIDeviceResp.u16DataResidue, 0) >= 0) {
				tCSW.u32DataResidue -= tSCSIDeviceResp.u16DataResidue;
			    return MS_STATE_CSW;
			} else {
			    mUsbEPStallSet(OUT_EP, DIRECTION_IN);        // case MS13Case_11:
			}
	        break;


		default:
			break;
	}
	vSCSIHWError();
	return MS_STATE_CSW;
}


//////////////////////////////////////////////////////////////////////////////////
_THUMB2 MassStorageState eScsiOut(void)
{
	switch (tSCSIDeviceResp.u8MemIndex) {
		case CARD_INDEX_SRAM:
			if (usb_DxFifoRdWr((INT8U *)u32Usb_buffer, tSCSIDeviceResp.u16DataResidue, 1) >= 0) {
				tCSW.u32DataResidue -= tSCSIDeviceResp.u16DataResidue;
				if (tCSW.u32DataResidue)
					mUsbEPStallSet(OUT_EP, DIRECTION_OUT);        // case MS13Case_11:
				return MS_STATE_CSW;
			}
			break;

		case CARD_INDEX_FSM_SS:
			if (usb_DxFifoRdWr((INT8U *)tSCSIDeviceResp.u32IOAddr, tSCSIDeviceResp.u16DataResidue, 1) >= 0) {
				tCSW.u32DataResidue -= tSCSIDeviceResp.u16DataResidue;
			    return MS_STATE_CSW;
			} else {
			    mUsbEPStallSet(OUT_EP, DIRECTION_OUT);        // case MS13Case_11:
			}
	        break;
		default:
			break;
	}
	vSCSIHWError();
	return MS_STATE_CSW;
}

typedef unsigned int size_t;
extern void *utl_memcpy(void *dest, const void *src, size_t count);

_THUMB2 void vSCSICmd_RequestSense(void)
{
	// Device intend to send data
	tSCSIDeviceResp.u8Flags = CBW_FLAG_IN;
	tSCSIDeviceResp.u8MemIndex = CARD_INDEX_SRAM;
	tSCSIDeviceResp.u16DataResidue = sizeof(u8RequestSenseData);

	// copy response to sram
	utl_memcpy ((UINT32 *)u32Usb_buffer, (UINT32 *)u8RequestSenseData, (UINT32)sizeof(u8RequestSenseData));

	*((unsigned int *)u32Usb_buffer + (SENSE_OFFSET_KEY>>2)) += (tSCSIsense.u8Key&0xFF)<<16;
	*((unsigned int *)u32Usb_buffer + (SENSE_OFFSET_ADD>>2)) += (tSCSIsense.u8KeyAdd&0xFF);

	// all key disappear
	tSCSIsense.u8Key = KEY_NO_SENSE;
	tSCSIsense.u8KeyAdd = ADDKEY_NO_ADDITIONAL;
}

_THUMB2 void vSCSICmd_ModeSense(void)
{

	//switch (tCBW.u8CB[2]) {
	switch ((tCBW.CB1_CB15[0]>>8)&0xFF) {
		case 0x00:     // return mode parameter header and block descriptor
		//debug_msg("MODE_SENSE 00\r\n");
			tSCSIDeviceResp.u16DataResidue = 0x0C;
			break;
		case 0x3F:     // all pages
			//debug_msg("MODE_SENSE 3F\r\n");
         		tSCSIDeviceResp.u16DataResidue = sizeof(u8ModeSenseData);
			break;
		default:
			//debug_msg("MODE_SENSE ERR\r\n");
			tSCSIsense.u8Key = KEY_ILLEGAL_REQUEST;
			tSCSIsense.u8KeyAdd = ADDKEY_INVALID_FIELD_IN_CMD;
			tCSW.u8Status = CSW_STATUS_CMD_FAIL;
			return;
	}
	// Device intend to send data
	tSCSIDeviceResp.u8Flags = CBW_FLAG_IN;
	tSCSIDeviceResp.u8MemIndex = CARD_INDEX_SRAM;

	// copy response to sram
	utl_memcpy((UINT32 *)u32Usb_buffer, (UINT32 *)u8ModeSenseData, (UINT32)tSCSIDeviceResp.u16DataResidue);
}



_THUMB2 void vSCSICmd_Unsupport(void)
{
	// Device intend to transfer no data
	tSCSIsense.u8Key = KEY_ILLEGAL_REQUEST;
	tSCSIsense.u8KeyAdd = ADDKEY_INVALID_CMD_OP_CODE;
	tCSW.u8Status = CSW_STATUS_CMD_FAIL;

	//#ifdef DISPLAY_USB_INFORMATION
	//DBG_DUMP("MSG: UnSupported SCSI Command (op code = 0x%x)\r\n", tCBW.u8CB[0]);
	//#endif
}

_THUMB2 MassStorageState eMassStorage13case(void)
{
	MassStorage13Case eCase = 0x0;
	MassStorageState estate;
	INT32U u32DeviceRespDataLength = 0x0;

	switch(tSCSIDeviceResp.u8MemIndex) {
		case CARD_INDEX_SRAM:
			u32DeviceRespDataLength = tSCSIDeviceResp.u16DataResidue;
			tSCSIDeviceResp.u32IOAddr += tSCSIDeviceResp.u16TfSzCurrent;
			break;
		case CARD_INDEX_FSM_SS:
		    u32DeviceRespDataLength =  tSCSIDeviceResp.u16DataResidue;
		    break;
		default:// SPI
			u32DeviceRespDataLength =  tSCSIDeviceResp.u16DataResidue * 2112;
			break;
	}

	// Mass Storage the thirteen case
	if (tCBW.u32DataTransferLength == u32DeviceRespDataLength) {
		// normal case
		if (tCBW.u32DataTransferLength == 0)
			eCase = MS13Case_1;                       // MassStorage case (1)
		else {
			if ((tCBW.CB0_cblen_lun_flag&0xFF) == tSCSIDeviceResp.u8Flags) {
				if ((tCBW.CB0_cblen_lun_flag&0xFF) == CBW_FLAG_IN)
					eCase = MS13Case_6;                 // MassStorage case (6)
				else
					eCase = MS13Case_12;                // MassStorage case (12)
			}
		}

		//#ifdef DISPLAY_USB_INFORMATION
		//	vShowSCSI();
		//#endif

	} else {
		if (tCBW.u32DataTransferLength == 0)
			eCase = MS13Case_2_3;                     // MassStorage case (2)(3)
		else if ((tCBW.CB0_cblen_lun_flag&0xFF) == CBW_FLAG_IN) {
			if (u32DeviceRespDataLength == 0)
				eCase = MS13Case_4;                    // MassStorage case (4)
			else if (tSCSIDeviceResp.u8Flags == CBW_FLAG_IN) {
				if (tCBW.u32DataTransferLength > u32DeviceRespDataLength)
					eCase = MS13Case_5;                 // MassStorage case (5)
				else
					eCase = MS13Case_7_8;               // MassStorage case (7)(8)
			}
		}
		else { //tCBW.u8Flags == CBW_FLAG_OUT
			if (u32DeviceRespDataLength == 0)
				eCase = MS13Case_9;                    // MassStorage case (9)
			else if (tSCSIDeviceResp.u8Flags == CBW_FLAG_OUT) {
				if (tCBW.u32DataTransferLength > u32DeviceRespDataLength)
					eCase = MS13Case_11;                // MassStorage case (11)
				else
					eCase = MS13Case_10_13;             // MassStorage case (10)(13)
			}
		}

//#ifdef DISPLAY_USB_INFORMATION
//		{
			//vShowUsb();
//			DBG_DUMP("MSG: MS13Case = 0x%x\r\n", eCase);
//			DBG_DUMP("MSG: , u32DeviceRespDataLength  = 0x%x\r\n", u32DeviceRespDataLength);
//		}
//#endif

	}

	// assume the next state is STATE_CSW
	estate = MS_STATE_CSW;
	switch (eCase) {
		case MS13Case_2_3:
			//debug_msg("S3\r\n");
			mUsbEPStallSet(IN_EP, DIRECTION_IN);
			tCSW.u8Status = CSW_STATUS_PHASE_ERROR;
			break;
		case MS13Case_4:
			//debug_msg("S4\r\n");
			mUsbEPStallSet(IN_EP, DIRECTION_IN);
			break;
		case MS13Case_5:
		case MS13Case_6:
			estate = eScsiIn();
			break;
		case MS13Case_11:
		case MS13Case_12:
			estate = eScsiOut();
			break;
		case MS13Case_7_8:
			//debug_msg("S5\r\n");
			mUsbEPStallSet(IN_EP, DIRECTION_IN);
			tCSW.u8Status = CSW_STATUS_PHASE_ERROR;
			break;
		case MS13Case_9:
			mUsbEPStallSet(OUT_EP, DIRECTION_OUT);
			break;
		case MS13Case_10_13:
			mUsbEPStallSet(OUT_EP, DIRECTION_OUT);
			tCSW.u8Status = CSW_STATUS_PHASE_ERROR;
			//debug_msg("S6\r\n");
			mUsbEPStallSet(IN_EP, DIRECTION_IN);
			break;
		default:    // MS13Case_1;
			// do nothing here
			break;
	}
	return estate;
}


_THUMB2 MassStorageState eSCSI_CmdDecode(void)    //trace, different from FUSB100-CardReader
{
	//debug_msg("eSCSI_CmdDecode\r\n");
	// Assume SCSI Response data length will be 0
	tSCSIDeviceResp.u16DataResidue = 0;
	if (((tCBW.CB0_cblen_lun_flag >> 8)&0xFF) >= CARD_TYPE_MAX_REPORT) {
		tCSW.u8Status = CSW_STATUS_CMD_FAIL;
		tSCSIsense.u8Key = KEY_NOT_READY;
		tSCSIsense.u8KeyAdd = ADDKEY_LOGICAL_UNIT_NOT_SUPPORT;
	} else {
		// Parse SCSI command
		//switch (tCBW.u8CB[0]) {                    // Operation Code
		switch ((tCBW.CB0_cblen_lun_flag >> 24)&0xFF) {                    // Operation Code
			case SCSI_OP_REQUEST_SENSE:
				vSCSICmd_RequestSense();
				//debug_msg("vSCSICmd_RequestSense\r\n");
				break;
			case SCSI_OP_MEDIUM_REMOVAL:
				//vSCSICmd_MediumRemoval();
				debug_msg("SCSI_OP_MEDIUM_REMOVAL\r\n");
				break;
			case SCSI_OP_INQUIRY:
				//vSCSICmd_Inquiry();
				debug_msg("SCSI_OP_INQUIRY\r\n");
				break;
			case SCSI_OP_MODE_SENSE:
				vSCSICmd_ModeSense();
				//debug_msg("SCSI_OP_MODE_SENSE 22\r\n");
				break;

			case SCSI_OP_TEST_UNIT_READY:
			case SCSI_OP_READ_CAPACITY:
			case SCSI_OP_READ_10:
			case SCSI_OP_WRITE_10:
			case SCSI_OP_VERIFY:
				//debug_msg("SCSI_OP_TEST_UNIT_READY\r\n");
				tSCSIsense.u8Key = KEY_NOT_READY;
				tSCSIsense.u8KeyAdd = ADDKEY_MEDIUM_NOT_PRESENT;
				tCSW.u8Status = CSW_STATUS_CMD_FAIL;
				break;

			default:
				if (my_SCSICmd(&tCBW) < 0) {
					//#ifdef USBROM_RTOS_DBG
					debug_msg("MSG: SCSI cmd: UNSUPPOR\r\n");
					//#endif
					vSCSICmd_Unsupport();
				}
				break;
		}
	}
	return eMassStorage13case();
}





_THUMB2 MassStorageState eUsbProessCBW(void)
{
	usb_DxFifoRdWr((unsigned char *)&tCBW, 31, 1);

	if(tCBW.u32Signature != CBW_SIGNATE) {
		return MS_STATE_CBW;
	} else {
		// pass u32DataTransferLength to u32DataResidue
		tCSW.u32DataResidue = tCBW.u32DataTransferLength;

		#ifdef USBROM_RTOS_DBG
		//DBG_DUMP("MSG: tCSW.u32DataResidue = %d, dir = %d\r\n", tCBW.u32DataTransferLength, tCBW.u8Flags);
		#endif

		// pass Tag from CBW to CSW
		tCSW.u32Tag = tCBW.u32Tag;
		// Assume Status is CMD_PASS
		tCSW.u8Status = CSW_STATUS_CMD_PASS;

		return eSCSI_CmdDecode();
	}
}

_THUMB2 void vUsbUnplug(void)
{
	//#ifdef USBROM_RTOS_DBG
	//debug_msg("MSG: Unplug.. pls check\r\n");
	//#endif
	// mUsbUnPlug();
	ptusbdev->phy_tms |= BIT0;
}

_THUMB2 void vUsb_BulkIntOnOff(void)
{
	//#ifdef USBROM_RTOS_DBG
	//DBG_DUMP("MSG: st 0x%x", eUsbMassStorageState);
	//#endif

	switch(eUsbMassStorageState)
	{
		case MS_STATE_CBW:
			mUsbIntF2OUTEn();
			mUsbIntF0INDis();
			break;
		case MS_STATE_CB_DMA_IN:
			if (mUsbEPStallST (IN_EP, DIRECTION_IN)) {
				// wait for clear feature, and then enable F0
			}
			else
				mUsbIntF0INEn();
			break;
		case MS_STATE_CB_DMA_OUT:
			mUsbIntF2OUTEn();
			break;
		case MS_STATE_CSW:
			mUsbIntF2OUTDis();
			if (mUsbEPStallST (IN_EP, DIRECTION_IN)) {
				// wait for clear feature, and then enable F0
			}
			else {
				mUsbIntF0INEn();
			}
			break;
		case MS_STATE_BGD:
			mUsbIntF2OUTDis();
			mUsbIntF0INDis();
			break;
		default:
			//#ifdef USBROM_RTOS_DBG
			debug_msg("MSG: Error MS_STATE\r\n");
			//#endif
			vUsbUnplug();
			break;
	}
}

_THUMB2 void vUsb_F0_In(void)
{
	switch(eUsbMassStorageState)
	{
		case MS_STATE_CSW:
			//#ifdef USBROM_RTOS_DBG
			//debug_msg("MSG: EP1 IN CSW\r\n");
			//#endif

			if (trigger_done_cb) {
				trigger_done_cb = 0;

				if (guiU2MsdcVendorDone_cb != NULL) {
					if (guiU2MsdcVendorDone_cb((unsigned int)&tCBW) != 0) {
						tCSW.u8Status = CSW_STATUS_CMD_FAIL;
					}
				}
			}

			usb_DxFifoRdWr((unsigned char *)&tCSW, 13, 0);

			//mUsbFIFODone(FIFO0);
			eUsbMassStorageState = MS_STATE_CBW;
			if(usbrom_exit == 1)
				usbrom_exit = 2;
			break;
		case MS_STATE_CB_DMA_IN:
			//#ifdef USBROM_RTOS_DBG
			//debug_msg("MSG: EP1 CS DMA IN\r\n");
			//#endif
			eUsbMassStorageState = eScsiOut();
			break;
		default:
			//#ifdef USBROM_RTOS_DBG
			debug_msg("MSG: Error FIFO0_IN interrupt.\r\n");
			//#endif

			break;
	}
	vUsb_BulkIntOnOff();
}

_THUMB2 void vUsb_F2_Out(unsigned int u32FIFOByteCount)
{
	switch(eUsbMassStorageState) {
		case MS_STATE_CBW:

			//#ifdef USBROM_RTOS_DBG
			//debug_msg("MSG: EP2 Out CBW\r\n");
			//#endif

			if (u32FIFOByteCount == 31) {
				eUsbMassStorageState = eUsbProessCBW();
			}
			else{
				//#ifdef USBROM_RTOS_DBG
				//debug_msg("MSG: EP out STAL SET\r\n");
				//#endif

				mUsbEPStallSet(OUT_EP, DIRECTION_OUT);
			}
			break;
		case MS_STATE_CB_DMA_OUT:

			//#ifdef USBROM_RTOS_DBG
			//debug_msg("MSG: EP2 Out CB DMA Out\r\n");
			//#endif

			eUsbMassStorageState = eScsiOut();
			break;
		default:

			//#ifdef USBROM_RTOS_DBG
			//debug_msg("MSG: Error FIFO2_OUT interrupt.\r\n");
			//#endif

			break;
	}

	vUsb_BulkIntOnOff();
}




///////////////////////////////////////////////////////////////////////////////
//		bClear_feature()
//		Description:
//			1. Send 2 bytes status to host.
//		input: none
//		output: TRUE or FALSE (BOOLEAN)
///////////////////////////////////////////////////////////////////////////////
_THUMB2 signed int usb_Clear_feature(void)
{
	unsigned int ep_n;
	unsigned int fifo_n;
	signed int bdir;

	switch (mCxValue (tusb.u32UsbCmd)) {		// FeatureSelector
	    	case 0:		// ENDPOINT_HALE
			// Clear "Endpoint_Halt", Turn off the "STALL" bit in Endpoint Control Function Register
			if(mCxIndex (tusb.u32UsbCmd) == 0x00)
				tusb.bUsbEP0HaltSt = FALSE;
			else {
				ep_n = mCxIndex (tusb.u32UsbCmd) & 0x7F;		// which ep will be clear
				// over the Max. ep count ?
				if (ep_n > FUSB200_MAX_EP)
					return -1;
				// the direction of this ep
				// bdir =0 if OUT
				bdir = mCxIndex (tusb.u32UsbCmd) & BIT7;;
				// get the relatived FIFO number
				if (bdir)
					fifo_n = mUsbEPMapRd(ep_n) & 0x0F;
				else
					fifo_n = mUsbEPMapRd(ep_n)  >> 4;
				// over the Max. fifo count ?
				if (fifo_n >= FUSB200_MAX_FIFO)
					return -1;

				// Check the FIFO had been enable ?
				if ((mUsbFIFOConfigRd(fifo_n) & BIT5) == 0)
					return -1;
				// bdir =0 if IN, 1 if OUT
				bdir = bdir ? 0: 1;
				mUsbEPRsTgSet(ep_n, bdir);		// Set Rst_Toggle Bit
				mUsbEPRsTgClr(ep_n, bdir);		// Clear Rst_Toggle Bit
				mUsbEPStallClr(ep_n, bdir);		// Clear Stall Bit
				vUsb_BulkIntOnOff();
			}
			break;
 		case 1 :   		// Device Remote Wakeup
			// Clear "Device_Remote_Wakeup", Turn off the"RMWKUP" bit in Main Control Register
			mUsbRmWkupClr();
			break;
		case 2 :   		// Test Mode
			// do not break here
		default :
			return -1;
	}
	tusb.eUsbCxAction = ACT_DONE;
	return 0;
}



_THUMB2 signed int usb_StandardCommand(void)
{
	switch (mCxRequest (tusb.u32UsbCmd)) { // by Standard Request codes
   		//case 0:		// get status
   		//	return (usb_Get_status());

		case 1:		// clear feature
			return (usb_Clear_feature());

		//case 3:		// set feature
		//	return (usb_Set_feature());

   		//case 5:		// set address
		//	if (!tusb.bUsbEP0HaltSt)
		//		return(usb_Set_address());
   		//	break;

		//case 6:		// get descriptor
		//	if (!tusb.bUsbEP0HaltSt)
		//		return(usb_Get_descriptor());
		//	break;

		//case 8:		// get configuration
		//	if (!tusb.bUsbEP0HaltSt)
		//		vGet_configuration();
		//	return 0;

		//case 9:		// set configuration
		//	if (!tusb.bUsbEP0HaltSt)
		//		return(usb_Set_configuration());
		//	break;

		//case 10:	// get interface
		//	if (!tusb.bUsbEP0HaltSt)
		//		return(usb_Get_interface());
	     //     	break;

	     // 	case 11:	// set interface
		//	if (!tusb.bUsbEP0HaltSt)
		//		return(usb_Set_interface());
		//	break;

		case 2:		// Reserved for further use
		case 4:		// Reserved for further use
		case 7:		// set descriptor, not support
		case 12:		// synch frame, not support
		default:
			break;
   	}
   	return -1;
}

///////////////////////////////////////////////////////////////////////////////
//		vUsb_ep0setup()
//		Description:
//			1. Read 8-byte setup packet.
//			2. Decode command as Standard, Class, Vendor or NOT support command
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
_THUMB2 void vUsb_ep0setup(void)
{
	/*
	if(tusb.bUsbChirpFinish == 0)
	{
		// first ep0 command after usb reset, means we can check usb speed right now.
		tusb.bUsbChirpFinish = 1;

		if (ptusbdev->main_ctl & BIT6) {
			tusb.bHighSpeed = 1;
			//DBG_DUMP("MSG: L%x, high speed mode\r\n", tusb.u8LineCount ++);
		}
		else {
			tusb.bHighSpeed = 0;
			//DBG_DUMP("MSG: L%x, full speed mode\r\n", tusb.u8LineCount ++);
		}
		// Init AP
		vUsbMassStorageInit();
	}
	*/

	// select dma target fifo no.:
	//ptusbdev->dma_fifo = BIT4;

	// Read 8-byte setup packet from FIFO
	tusb.u32UsbCmd[0] = ptusbdev->dma_data;
	tusb.u32UsbCmd[1] = ptusbdev->dma_data;

	#if 0//def USBROM_RTOS_DBG
	DBG_DUMP("MSG: L%x, EP0Cmd: %x %x %x %x %x %x %x %x\r\n",
		tusb.u8LineCount ++, (INT8U)(tusb.u32UsbCmd[0] >> 0)
					, (INT8U)(tusb.u32UsbCmd[0] >> 8)
					, (INT8U)(tusb.u32UsbCmd[0] >> 16)
					, (INT8U)(tusb.u32UsbCmd[0] >> 24)
					, (INT8U)(tusb.u32UsbCmd[1] >> 0)
					, (INT8U)(tusb.u32UsbCmd[1] >> 8)
					, (INT8U)(tusb.u32UsbCmd[1] >> 16)
					, (INT8U)(tusb.u32UsbCmd[1] >> 24));
	#endif

 	//  Command Decode
	switch (mCxType(tusb.u32UsbCmd)) {
		case 0:						// standard command

			//#ifdef USBROM_RTOS_DBG
			//DBG_DUMP("MSG: Standard command\r\n");
			//#endif

			if (usb_StandardCommand() < 0)
				tusb.eUsbCxAction = ACT_STALL;
			break;
/*
		case 1:						// class command

			#ifdef USBROM_RTOS_DBG
			DBG_DUMP("MSG: class command\r\n");
			#endif

			if (usb_ClassCommand() < 0)
				tusb.eUsbCxAction = ACT_STALL;
			break;

		case 2:						// vendor command
			#ifdef USBROM_RTOS_DBG
			DBG_DUMP("MSG: vendor command\r\n");
			#endif
//			if (usb_UsbVendorCommand() < 0
				tusb.eUsbCxAction = ACT_STALL;
			break;
*/
		default:
			// Invalid(bad) command, Return EP0_STALL flag
			tusb.eUsbCxAction = ACT_STALL;
			break;
	}
}




_THUMB2 void vUsbInit(void)
{
	ptusbdev = ((volatile DEV *)(IOADDR_USB_REG_BASE + 0x100));

	// init variables
	tusb.eUsbCxAction = ACT_IDLE;
	tusb.bUsbChirpFinish = 0;

	tusb.u16TxRxCounter = 0;
	tusb.eUsbCxCommand = CMD_VOID;
	tusb.u8UsbConfigValue = 0;
	tusb.u8UsbInterfaceValue = 0;
	tusb.u8UsbInterfaceAlternateSetting = 0;
	tusb.bUsbEP0HaltSt = FALSE;

	// init hardware
	//vFUSB200Init();
	tusb.u8LineCount = 1;

	tCSW.u32Signature = CSW_SIGNATE;
	eUsbMassStorageState = MS_STATE_CBW;
	mUsbIntF2OUTEn ();
	mUsbIntF0INDis();

}



_THUMB2 void vUsbIsr2(void)
{
	INT32U level1;
	INT32U level2;

	level1 = ptusbdev->int_grp & ~ ptusbdev->int_mgrp;

	if (0 == level1)
		return;

	if (level1 & BIT2) {				//Group 2

		level2 = ptusbdev->int_grp2 & ~ ptusbdev->int_mgrp2;

		if (level2) {

			//debug_msg("MSG: IntSCR2\r\n");

			if (level2 & BIT0) {
				//debug_msg("MSG: BUS RST\r\n");
			}

			if (level2 & BIT1) {
				//debug_msg("MSG: BUS SUSPEND\r\n");
				ptusbdev->int_grp2 |= BIT1;
			}

			if (level2 & BIT2) {
				//debug_msg("MSG: BUS RESUME\r\n");
				ptusbdev->int_grp2 |= BIT2;
			}

			if (level2 & BIT5) {
				//debug_msg("MSG: TX0B\r\n");

				ptusbdev->tx0byte |= 0;
				ptusbdev->int_grp2 |= BIT5;
			}

			if (level2 & BIT6) {
				//debug_msg("MSG: RX0B\r\n");
				ptusbdev->rx0byte |= 0;
				ptusbdev->int_grp2 |= BIT6;
			}

		}
	}


	if (level1 & BIT0) {				//Group 0
		level2 = ptusbdev->int_grp0 & ~ ptusbdev->int_mgrp0;
		if (level2) {
			//debug_msg("MSG: IntSCR0\r\n");

			if (level2 & BIT0) {
				//debug_msg("MSG: USB ep0 Setup\r\n");
				vUsb_ep0setup();
			}
			else if (level2 & BIT3) {
				//debug_msg("MSG: USB ep0 end\r\n");
				//vUsb_ep0end();
			}
			if (level2 & BIT1) {
				//debug_msg("MSG: USB ep0 TX\r\n");
				//vUsb_ep0tx();
			}
			if (level2 & BIT2) {
				//#ifdef USBROM_RTOS_DBG
				//debug_msg("MSG: USB ep0 RX\r\n");
				//#endif
				//vUsb_ep0rx();
			}
			if (level2 & BIT4) {

				//#ifdef USBROM_RTOS_DBG
				//debug_msg("MSG: USB ep0 fail\r\n");
				//#endif
				// stall cx: mUsbEP0StallSet();
				//ptusbdev->cx_cf |= BIT2;
			}
			if (level2 & BIT5) {

				//#ifdef USBROM_RTOS_DBG
				//debug_msg("MSG: cmd abort\r\n");
				//#endif
				ptusbdev->int_grp0 |= BIT5;
			}
		}


		if (tusb.eUsbCxAction == ACT_STALL) {
			ptusbdev->cx_cf |= BIT2;

			//#ifdef USBROM_RTOS_DBG
			//DBG_DUMP("MSG: EP0 STALL\r\n");
			//#endif
		}
		else if (tusb.eUsbCxAction == ACT_DONE) {
			ptusbdev->cx_cf |= BIT0;

			//#ifdef USBROM_RTOS_DBG
			//DBG_DUMP("MSG: EP0 DONE\r\n");
			//#endif
		}
		// Clear Action
		tusb.eUsbCxAction = ACT_IDLE;


	}

	if (level1 & BIT1) {				//Group Byte 1
		level2 = ptusbdev->int_grp1 & ~ ptusbdev->int_mgrp1;

		//#ifdef USBROM_RTOS_DBG
		//debug_msg("MSG: IntSCR1\r\n");
		//#endif

		if (level2 & (BIT5 | BIT4)) {		// F2 out (full or short)
			vUsb_F2_Out(ptusbdev->fifo_bc[2]);
		}

		if (level2 & BIT16)				// F0 in
			vUsb_F0_In();
	}
}



_THUMB2 static INT32U calc_sum_v2(INT32U dat0, INT32U dat1, INT32U dat2, INT32U dat3)
{
	INT32U sum=0;

	sum = (dat0&0xFFFF)+((dat0>>16))+(dat1&0xFFFF)+((dat1>>16)) +
		  (dat2&0xFFFF)+((dat2>>16))+(dat3&0xFFFF)+((dat3>>16)) + 28;

	//DBG_DUMP("SUM = 0x%08X\r\n",sum);
	return  sum&0xFFFF;
}

_THUMB2 INT32S my_SCSICmd(CBW * ptcbw)
{
	INT32U temp1, temp2;
	INT32U CDB[4];

	CDB[0] = ((ptcbw->CB0_cblen_lun_flag >> 24) & 0xFF) + ((ptcbw->CB1_CB15[0]&0xFFFFFF)<<8);
	CDB[1] = ((ptcbw->CB1_CB15[0] >> 24) & 0xFF) + ((ptcbw->CB1_CB15[1]&0xFFFFFF)<<8);
	CDB[2] = ((ptcbw->CB1_CB15[1] >> 24) & 0xFF) + ((ptcbw->CB1_CB15[2]&0xFFFFFF)<<8);
	CDB[3] = ((ptcbw->CB1_CB15[2] >> 24) & 0xFF) + ((ptcbw->CB1_CB15[3]&0xFFFFFF)<<8);

	// Parse SCSI command
	if (!(((CDB[0]&0xFF) == SCSI_OP_SET0)||((CDB[0]&0xFF) == SCSI_OP_SET1)||((CDB[0]&0xFF) == SCSI_OP_SET2)))
		return -1;

	// check sum
	if(calc_sum_v2(CDB[0],CDB[1],CDB[2],CDB[3])) {
		tCSW.u8Status = CSW_STATUS_CMD_FAIL;
		u32Usb_buffer[0] = 0;
		#ifdef USBROM_RTOS_DBG
		DBG_DUMP("CHKSUM FAILED\r\n");
		#endif
		return -1;
	}

	tCSW.u8Status = CSW_STATUS_CMD_PASS;

	switch (((CDB[0] >> 8) &0xFF)) {

	case SCSIOP_OUT_OPEN_DEVICE: {
		nvtdev_opened = 1;
		#ifdef USBROM_RTOS_DBG
		DBG_DUMP("SCSIOP_OUT_OPEN_DEVICE\r\n");
		#endif
	}
	break;

	case SCSIOP_IN_WAIT_DONE: {
		usbrom_exit = 1;
		#ifdef USBROM_RTOS_DBG
		DBG_DUMP("SCSIOP_IN_WAIT_DONE\r\n");
		#endif
	}
	break;


	case SCSIOP_IN_IS_NVT: {
		tSCSIDeviceResp.u8Flags = CBW_FLAG_IN;
		tSCSIDeviceResp.u8MemIndex = CARD_INDEX_SRAM;
		tSCSIDeviceResp.u16DataResidue = 4;

		tCSW.u8Status = CSW_STATUS_CMD_PASS;
		u32Usb_buffer[0] = 0x70210000;
		#ifdef USBROM_RTOS_DBG
		DBG_DUMP("SCSIOP_IN_IS_NVT\r\n");
		#endif
	}
	break;

	case SCSIOP_OUT_ADDR_WRITE: {

		if(nvtdev_opened == 0) {
			tCSW.u8Status = CSW_STATUS_CMD_FAIL;
		} else {
			// address
			temp1 = ((CDB[0]>>16)&0xFFFF)+((CDB[1]&0xFFFF)<<16);//u8to32(&ptcbw->u8CB[2]);
			// data
			temp2 = ((CDB[1]>>16)&0xFFFF)+((CDB[2]&0xFFFF)<<16);//u8to32(&ptcbw->u8CB[6]);

			*(INT32U *)temp1 = temp2;
			__asm__ __volatile__("dsb\n\t");

			#ifdef USBROM_RTOS_DBG
			DBG_DUMP("SCSIOP_OUT_ADDR_WRITE 0x%08X 0x%08X\r\n", temp1, temp2);
			#endif
		}
	}
	break;

	case SCSIOP_IN_ADDR_READ: {

		if(nvtdev_opened == 0) {
			tCSW.u8Status = CSW_STATUS_CMD_FAIL;
		} else {
			// address
			temp1 = ((CDB[0]>>16)&0xFFFF)+((CDB[1]&0xFFFF)<<16);//u8to32(&ptcbw->u8CB[2]);

			tSCSIDeviceResp.u8Flags = CBW_FLAG_IN;
			tSCSIDeviceResp.u8MemIndex = CARD_INDEX_SRAM;
			tSCSIDeviceResp.u16DataResidue = 8;
			u32Usb_buffer[0] = temp1;
			u32Usb_buffer[1] = *((INT32U *)temp1);

			#ifdef USBROM_RTOS_DBG
			DBG_DUMP("SCSIOP_IN_ADDR_READ 0x%08X 0x%08X\r\n", temp1, u32Usb_buffer[1]);
			#endif
		}
	}
	break;
#if 0 //only rom code support
	case SCSIOP_OUT_WRLOADER: {

		if(nvtdev_opened == 0) {
			tCSW.u8Status = CSW_STATUS_CMD_FAIL;
		} else {
			// address
			temp1 = ((CDB[0]>>16)&0xFFFF)+((CDB[1]&0xFFFF)<<16);//u8to32(&ptcbw->u8CB[2]);

		    tSCSIDeviceResp.u32IOAddr = temp1;
			tSCSIDeviceResp.u8Flags = CBW_FLAG_OUT;
			tSCSIDeviceResp.u8MemIndex = CARD_INDEX_FSM_SS;
			tSCSIDeviceResp.u16DataResidue = ptcbw->u32DataTransferLength;

			#ifdef USBROM_RTOS_DBG
			DBG_DUMP("SCSIOP_OUT_WRLOADER 0x%08X 0x%08X\r\n", temp1, tSCSIDeviceResp.u16DataResidue);
			#endif

		}
	}
	break;
#endif
	default: {
		if(nvtdev_opened == 0) {
			tCSW.u8Status = CSW_STATUS_CMD_FAIL;
		} else {
			int ret = -1;
			unsigned int OutDataBuf=0, insize=0;

			if (guiU2MsdcCheck_cb != NULL) {
				ret = guiU2MsdcCheck_cb((unsigned int)ptcbw, (unsigned int *) &OutDataBuf, (unsigned int *)&insize);
			} else {
				ret = -1;
			}

			if(ret == 0) {

				if (ptcbw->CB0_cblen_lun_flag & CBW_FLAG_IN) {
				    tSCSIDeviceResp.u32IOAddr 		= OutDataBuf;
					tSCSIDeviceResp.u8Flags 		= CBW_FLAG_IN;
					tSCSIDeviceResp.u8MemIndex 		= CARD_INDEX_FSM_SS;
					tSCSIDeviceResp.u16DataResidue 	= insize;

				} else {
				    tSCSIDeviceResp.u32IOAddr 		= OutDataBuf;
					tSCSIDeviceResp.u8Flags 		= CBW_FLAG_OUT;
					tSCSIDeviceResp.u8MemIndex 		= CARD_INDEX_FSM_SS;
					tSCSIDeviceResp.u16DataResidue 	= ptcbw->u32DataTransferLength;
				}

				trigger_done_cb = 1;
			} else {
				return ret;
			}
		}
	}
	break;

	}

	return 0;

}



_THUMB2 void fLib_USB_Update_FW(void)
{
	debug_msg("USBFW UPDATE\r\n");

	vUsbInit();

	while (1) {

		vUsbIsr2();

		if(usbrom_exit > 1) {
			//mdelay(300); //make sure that pc recevie scsi status
			timer_delay(300000);
			vUsbUnplug();
			timer_delay(300000);
			//mdelay(300); //make sure that pc recevie usb plug-out single
			break;
		}

	}

	usbrom_exit = 0;

}

