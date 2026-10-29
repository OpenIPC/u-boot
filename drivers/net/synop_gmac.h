/* SPDX-License-Identifier: MIT */
/*
 * Synopsys DesignWare GMAC core, as the Ingenic T-series has it.
 *
 * From drivers/net/SynopGMAC_Dev.h of OpenIPC/u-boot-ingenic (Synopsys,
 * 2007-2009, with Ingenic changes), trimmed to what drivers/net/jz_gmac.c
 * uses: enhanced, 8-word descriptors (ENH_DESC, ENH_DESC_8W), no AVB, no
 * IEEE 1588 timestamping, no power management.
 */
/*
 * Copyright (c) <2009> Synopsys, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software annotated with this license and associated documentation files
 * (the "Software"), to deal in the Software without restriction, including without
 * limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * =================================================================================== */

#ifndef SYNOP_GMAC_H
#define SYNOP_GMAC_H

#define ENH_DESC
#define ENH_DESC_8W

#include <common.h>
#include <asm/io.h>


//#define SYNOP_DEBUG
static inline void plat_delay(u32 delay)
{
	while (delay--);
	return;
}


#define TR0(fmt, args...) printf("SynopGMAC: " fmt, ##args)
#ifdef SYNOP_DEBUG
#define TR(fmt, args...)						\
	do {								\
		printf("SynopGMAC: " fmt, ##args);			\
	} while(0)
#else
#define TR(fmt, args...)
#endif
#define DEFAULT_DELAY_VARIABLE  10
#define DEFAULT_LOOP_VARIABLE   10000

/* Error Codes */
#define ESYNOPGMACNOERR   0
#define ESYNOPGMACNOMEM   1
#define ESYNOPGMACPHYERR  2
#define ESYNOPGMACBUSY    3

/**
 * The Low level function to read register contents from Hardware.
 *
 * @param[in] pointer to the base of register map
 * @param[in] Offset from the base
 * \return  Returns the register contents
 */
static u32 __inline__ synopGMACReadReg(u32 *RegBase, u32 RegOffset)
{

	u32 addr = (u32)RegBase + RegOffset;
	u32 data = readl((void *)addr);
	//printf("=====>%s: RegBase = 0x%08x RegOffset = 0x%08x RegData = 0x%08x\n", __FUNCTION__, (u32)RegBase, RegOffset, data );
	return data;

}

/**
 * The Low level function to write to a register in Hardware.
 *
 * @param[in] pointer to the base of register map
 * @param[in] Offset from the base
 * @param[in] Data to be written
 * \return  void
 */
static void  __inline__ synopGMACWriteReg(u32 *RegBase, u32 RegOffset, u32 RegData)
{

	u32 addr = (u32)RegBase + RegOffset;
	//TR("=====>%s RegBase = 0x%08x RegOffset = 0x%08x RegData = 0x%08x\n", __FUNCTION__,(u32) RegBase, RegOffset, RegData );
	//writel(RegData,(void *)addr);
	*((volatile unsigned int *)addr) = RegData;
	return;
}

/**
 * The Low level function to set bits of a register in Hardware.
 *
 * @param[in] pointer to the base of register map
 * @param[in] Offset from the base
 * @param[in] Bit mask to set bits to logical 1
 * \return  void
 */
static void __inline__ synopGMACSetBits(u32 *RegBase, u32 RegOffset, u32 BitPos)
{
	u32 addr = (u32)RegBase + RegOffset;
	u32 data = readl((void *)addr);
	data |= BitPos;
	//  TR("%s !!!!!!!!!!!!! RegOffset = 0x%08x RegData = 0x%08x\n", __FUNCTION__, RegOffset, data );
	writel(data,(void *)addr);
	//  TR("%s !!!!!!!!!!!!! RegOffset = 0x%08x RegData = 0x%08x\n", __FUNCTION__, RegOffset, data );
	return;
}


/**
 * The Low level function to clear bits of a register in Hardware.
 *
 * @param[in] pointer to the base of register map
 * @param[in] Offset from the base
 * @param[in] Bit mask to clear bits to logical 0
 * \return  void
 */
static void __inline__ synopGMACClearBits(u32 *RegBase, u32 RegOffset, u32 BitPos)
{
	u32 addr = (u32)RegBase + RegOffset;
	u32 data = readl((void *)addr);
	data &= (~BitPos);
	//  TR("%s !!!!!!!!!!!!!! RegOffset = 0x%08x RegData = 0x%08x\n", __FUNCTION__, RegOffset, data );
	writel(data,(void *)addr);
	//  TR("%s !!!!!!!!!!!!! RegOffset = 0x%08x RegData = 0x%08x\n", __FUNCTION__, RegOffset, data );
	return;
}

/**
 * The Low level function to Check the setting of the bits.
 *
 * @param[in] pointer to the base of register map
 * @param[in] Offset from the base
 * @param[in] Bit mask to set bits to logical 1
 * \return  returns TRUE if set to '1' returns FALSE if set to '0'. Result undefined there are no bit set in the BitPos argument.
 *
 */
static bool __inline__ synopGMACCheckBits(u32 *RegBase, u32 RegOffset, u32 BitPos)
{
	u32 addr = (u32)RegBase + RegOffset;
	u32 data = readl((void *)addr);
	data &= BitPos;
	if(data)  return true;
	else	    return false;

}

/*SynopGMAC can support up to 32 phys*/

enum GMACPhyBase
{
	PHY0  = 0,			//The device can support 32 phys, but we use first phy only
	PHY31 = 31,
};

#define DEFAULT_PHY_BASE (PHY0 + 16)		//We use First Phy
#define MACBASE 0x0000			// The Mac Base address offset is 0x0000
#define DMABASE 0x1000			// Dma base address starts with an offset 0x1000

typedef struct DmaDescStruct
{
  u32   status;         /* Status 									*/
  u32   length;         /* Buffer 1  and Buffer 2 length 						*/
  u32   buffer1;        /* Network Buffer 1 pointer (Dma-able) 							*/
  u32   buffer2;        /* Network Buffer 2 pointer or next descriptor pointer (Dma-able)in chain structure 	*/
			/* This data below is used only by driver					*/
  u32   extstatus;      /* Extended status of a Rx Descriptor                                           */
  u32   reserved1;      /* Reserved word                                                                */
  u32   timestamplow;   /* Lower 32 bits of the 64 bit timestamp value                                  */
  u32   timestamphigh;  /* Higher 32 bits of the 64 bit timestamp value                                  */
  u32   data1;          /* This holds virtual address of buffer1, not used by DMA  			*/
  u32   data2;          /* This holds virtual address of buffer2, not used by DMA  			*/
} DmaDesc;

/* synopGMAC device data */

typedef struct synopGMACDeviceStruct
{
  u32 MacBase; 		         /* base address of MAC registers           */
  u32 DmaBase;         		 /* base address of DMA registers           */
  u32 PhyBase;          	 /* PHY device address on MII interface     */
  u32 Version;	                 /* Gmac Revision version	                */


  dma_addr_t TxDescDma;		 /* Dma-able address of first tx descriptor either in ring or chain mode, this is used by the GMAC device*/
  dma_addr_t RxDescDma; 	 /* Dma-albe address of first rx descriptor either in ring or chain mode, this is used by the GMAC device*/
  DmaDesc *TxDesc;               /* start address of TX descriptors ring or chain, this is used by the driver  */
  DmaDesc *RxDesc;               /* start address of RX descriptors ring or chain, this is used by the driver  */

  u32 BusyTxDesc;	         /* Number of Tx Descriptors owned by DMA at any given time*/
  u32 BusyRxDesc;		 /* Number of Rx Descriptors owned by DMA at any given time*/

  u32  RxDescCount;              /* number of rx descriptors in the tx descriptor queue/pool */
  u32  TxDescCount;              /* number of tx descriptors in the rx descriptor queue/pool */

  u32  TxBusy;                   /* index of the tx descriptor owned by DMA, is obtained by synopGMAC_get_tx_qptr()                */
  u32  TxNext;                   /* index of the tx descriptor next available with driver, given to DMA by synopGMAC_set_tx_qptr() */
  u32  RxBusy;                   /* index of the rx descriptor owned by DMA, obtained by synopGMAC_get_rx_qptr()                   */
  u32  RxNext;                   /* index of the rx descriptor next available with driver, given to DMA by synopGMAC_set_rx_qptr() */

  DmaDesc * TxBusyDesc;          /* Tx Descriptor address corresponding to the index TxBusy */
  DmaDesc * TxNextDesc;          /* Tx Descriptor address corresponding to the index TxNext */
  DmaDesc * RxBusyDesc;          /* Rx Descriptor address corresponding to the index TxBusy */
  DmaDesc * RxNextDesc;          /* Rx Descriptor address corresponding to the index RxNext */


  /*Phy related stuff*/
  u32 ClockDivMdc;		 /* Clock divider value programmed in the hardware           */
  /* The status of the link */
  u32 LinkState;		 /* Link status as reported by the Marvel Phy                */
  u32 DuplexMode;                /* Duplex mode of the Phy				    */
  u32 Speed;			 /* Speed of the Phy					    */
  u32 LoopBackMode; 		 /* Loopback status of the Phy				    */

} synopGMACdevice;

/* Below is "88E1011/88E1011S Integrated 10/100/1000 Gigabit Ethernet Transceiver"
 * Register and their layouts. This Phy has been used in the Dot Aster GMAC Phy daughter.
 * Since the Phy register map is standard, this map hardly changes to a different Ppy
 */

enum MiiRegisters
{
  PHY_CONTROL_REG           = 0x0000,		/*Control Register*/
  PHY_STATUS_REG            = 0x0001,		/*Status Register */
  PHY_ID_HI_REG             = 0x0002,		/*PHY Identifier High Register*/
  PHY_ID_LOW_REG            = 0x0003,		/*PHY Identifier High Register*/
  PHY_AN_ADV_REG            = 0x0004,		/*Auto-Negotiation Advertisement Register*/
  PHY_LNK_PART_ABl_REG      = 0x0005,		/*Link Partner Ability Register (Base Page)*/
  PHY_AN_EXP_REG            = 0x0006,		/*Auto-Negotiation Expansion Register*/
  PHY_AN_NXT_PAGE_TX_REG    = 0x0007,		/*Next Page Transmit Register*/
  PHY_LNK_PART_NXT_PAGE_REG = 0x0008,		/*Link Partner Next Page Register*/
  PHY_1000BT_CTRL_REG       = 0x0009,		/*1000BASE-T Control Register*/
  PHY_1000BT_STATUS_REG     = 0x000a,		/*1000BASE-T Status Register*/
  PHY_SPECIFIC_CTRL_REG     = 0x0010,		/*Phy specific control register*/
  PHY_SPECIFIC_STATUS_REG   = 0x0011,		/*Phy specific status register*/
  PHY_INTERRUPT_ENABLE_REG  = 0x0012,		/*Phy interrupt enable register*/
  PHY_INTERRUPT_STATUS_REG  = 0x0013,		/*Phy interrupt status register*/
  PHY_EXT_PHY_SPC_CTRL	    = 0x0014,		/*Extended Phy specific control*/
  PHY_RX_ERR_COUNTER	    = 0x0015,		/*Receive Error Counter*/
  PHY_EXT_ADDR_CBL_DIAG     = 0x0016,		/*Extended address for cable diagnostic register*/
  PHY_LED_CONTROL	    = 0x0018,		/*LED Control*/
  PHY_MAN_LED_OVERIDE       = 0x0019,		/*Manual LED override register*/
  PHY_EXT_PHY_SPC_CTRL2     = 0x001a,		/*Extended Phy specific control 2*/
  PHY_EXT_PHY_SPC_STATUS    = 0x001b,		/*Extended Phy specific status*/
  PHY_CBL_DIAG_REG	    = 0x001c,		/*Cable diagnostic registers*/
};


/* This is Control register layout. Control register is of 16 bit wide.
*/

enum Mii_GEN_CTRL
{		                      /*	Description	               bits	       R/W	default value  */
	Mii_reset		 = 0x8000,
	Mii_Speed_10   		 = 0x0000,   /* 10   Mbps                    6:13         	RW                      */
	Mii_Speed_100   	 = 0x2000,   /* 100  Mbps                    6:13         	RW                      */
	Mii_Speed_1000 		 = 0x0040,   /* 1000 Mbit/s                  6:13         	RW                      */

	Mii_Duplex     		 = 0x0100,   /* Full Duplex mode             8               	RW                      */

	Mii_Manual_Master_Config = 0x0800,   /* Manual Master Config         11              	RW			*/

	Mii_Loopback   		 = 0x4000,   /* Enable Loop back             14              	RW                      */
	Mii_NoLoopback 		 = 0x0000,   /* Enable Loop back             14              	RW                      */
};

enum Mii_Phy_Status
{
	Mii_phy_status_speed_10	 	= 0x0000,
	Mii_phy_status_speed_100  	= 0x4000,
	Mii_phy_status_speed_1000	= 0x8000,

	Mii_phy_status_full_duplex	= 0x2000,
	Mii_phy_status_half_duplex	= 0x0000,

	Mii_phy_status_link_up		= 0x0400,
};
/* This is Status register layout. Status register is of 16 bit wide.
*/
enum Mii_GEN_STATUS
{
	Mii_AutoNegCmplt     = 0x0020,   /* Autonegotiation completed      5              RW                   */
	Mii_Link             = 0x0004,   /* Link status                    2              RW                   */
};

enum Mii_Link_Status
{
	LINKDOWN	= 0,
	LINKUP		= 1,
};

enum Mii_Duplex_Mode
{
	HALFDUPLEX = 1,
	FULLDUPLEX = 2,
};
enum Mii_Link_Speed
{
	SPEED10     = 1,
	SPEED100    = 2,
	SPEED1000   = 3,
};

enum Mii_Loop_Back
{
	NOLOOPBACK  = 0,
	LOOPBACK    = 1,
};



/**********************************************************
 * GMAC registers Map
 * For Pci based system address is BARx + GmacRegisterBase
 * For any other system translation is done accordingly
 **********************************************************/
enum GmacRegisters
{
  GmacConfig     	  = 0x0000,    /* Mac config Register                       */
  GmacFrameFilter  	  = 0x0004,    /* Mac frame filtering controls              */
  GmacHashHigh     	  = 0x0008,    /* Multi-cast hash table high                */
  GmacHashLow      	  = 0x000C,    /* Multi-cast hash table low                 */
  GmacGmiiAddr     	  = 0x0010,    /* GMII address Register(ext. Phy)           */
  GmacGmiiData     	  = 0x0014,    /* GMII data Register(ext. Phy)              */
  GmacFlowControl  	  = 0x0018,    /* Flow control Register                     */
  GmacVlan         	  = 0x001C,    /* VLAN tag Register (IEEE 802.1Q)           */

  GmacVersion     	  = 0x0020,    /* GMAC Core Version Register                */
  GmacWakeupAddr  	  = 0x0028,    /* GMAC wake-up frame filter adrress reg     */
  GmacPmtCtrlStatus  	  = 0x002C,    /* PMT control and status register           */

  #ifdef LPI_SUPPORT
  GmacLPICtrlSts      = 0x0030,    /* LPI (low power idle) Control and Status Register          */
  GmacLPITimerCtrl    = 0x0034,    /* LPI timer control register               */
  #endif

  GmacInterruptStatus	  = 0x0038,    /* Mac Interrupt ststus register	       */
  GmacInterruptMask       = 0x003C,    /* Mac Interrupt Mask register	       */

  GmacAddr0High    	  = 0x0040,    /* Mac address0 high Register                */
  GmacAddr0Low    	  = 0x0044,    /* Mac address0 low Register                 */
  GmacAddr1High    	  = 0x0048,    /* Mac address1 high Register                */
  GmacAddr1Low     	  = 0x004C,    /* Mac address1 low Register                 */
  GmacAddr2High   	  = 0x0050,    /* Mac address2 high Register                */
  GmacAddr2Low     	  = 0x0054,    /* Mac address2 low Register                 */
  GmacAddr3High    	  = 0x0058,    /* Mac address3 high Register                */
  GmacAddr3Low     	  = 0x005C,    /* Mac address3 low Register                 */
  GmacAddr4High    	  = 0x0060,    /* Mac address4 high Register                */
  GmacAddr4Low     	  = 0x0064,    /* Mac address4 low Register                 */
  GmacAddr5High    	  = 0x0068,    /* Mac address5 high Register                */
  GmacAddr5Low     	  = 0x006C,    /* Mac address5 low Register                 */
  GmacAddr6High    	  = 0x0070,    /* Mac address6 high Register                */
  GmacAddr6Low     	  = 0x0074,    /* Mac address6 low Register                 */
  GmacAddr7High    	  = 0x0078,    /* Mac address7 high Register                */
  GmacAddr7Low     	  = 0x007C,    /* Mac address7 low Register                 */
  GmacAddr8High    	  = 0x0080,    /* Mac address8 high Register                */
  GmacAddr8Low     	  = 0x0084,    /* Mac address8 low Register                 */
  GmacAddr9High    	  = 0x0088,    /* Mac address9 high Register                */
  GmacAddr9Low      	  = 0x008C,    /* Mac address9 low Register                 */
  GmacAddr10High          = 0x0090,    /* Mac address10 high Register               */
  GmacAddr10Low    	  = 0x0094,    /* Mac address10 low Register                */
  GmacAddr11High   	  = 0x0098,    /* Mac address11 high Register               */
  GmacAddr11Low    	  = 0x009C,    /* Mac address11 low Register                */
  GmacAddr12High   	  = 0x00A0,    /* Mac address12 high Register               */
  GmacAddr12Low     	  = 0x00A4,    /* Mac address12 low Register                */
  GmacAddr13High   	  = 0x00A8,    /* Mac address13 high Register               */
  GmacAddr13Low   	  = 0x00AC,    /* Mac address13 low Register                */
  GmacAddr14High   	  = 0x00B0,    /* Mac address14 high Register               */
  GmacAddr14Low        	  = 0x00B4,    /* Mac address14 low Register                */
  GmacAddr15High     	  = 0x00B8,    /* Mac address15 high Register               */
  GmacAddr15Low  	  = 0x00BC,    /* Mac address15 low Register                */

  /*Time Stamp Register Map*/
  GmacTSControl	          = 0x0700,  /* Controls the Timestamp update logic                         : only when IEEE 1588 time stamping is enabled in corekit            */

  GmacTSSubSecIncr     	  = 0x0704,  /* 8 bit value by which sub second register is incremented     : only when IEEE 1588 time stamping without external timestamp input */

  GmacTSHigh  	          = 0x0708,  /* 32 bit seconds(MS)                                          : only when IEEE 1588 time stamping without external timestamp input */
  GmacTSLow   	          = 0x070C,  /* 32 bit nano seconds(MS)                                     : only when IEEE 1588 time stamping without external timestamp input */

  GmacTSHighUpdate        = 0x0710,  /* 32 bit seconds(MS) to be written/added/subtracted           : only when IEEE 1588 time stamping without external timestamp input */
  GmacTSLowUpdate         = 0x0714,  /* 32 bit nano seconds(MS) to be writeen/added/subtracted      : only when IEEE 1588 time stamping without external timestamp input */

  GmacTSAddend            = 0x0718,  /* Used by Software to readjust the clock frequency linearly   : only when IEEE 1588 time stamping without external timestamp input */

  GmacTSTargetTimeHigh 	  = 0x071C,  /* 32 bit seconds(MS) to be compared with system time          : only when IEEE 1588 time stamping without external timestamp input */
  GmacTSTargetTimeLow     = 0x0720,  /* 32 bit nano seconds(MS) to be compared with system time     : only when IEEE 1588 time stamping without external timestamp input */

  GmacTSHighWord          = 0x0724,  /* Time Stamp Higher Word Register (Version 2 only); only lower 16 bits are valid                                                   */
  //GmacTSHighWordUpdate    = 0x072C,  /* Time Stamp Higher Word Update Register (Version 2 only); only lower 16 bits are valid                                            */

  GmacTSStatus            = 0x0728,  /* Time Stamp Status Register                                                                                                       */
  #ifdef AVB_SUPPORT
  GmacAvMacCtrl            = 0x0738,  /* AV mac control Register  */
  #endif

};

/**********************************************************
 * GMAC Network interface registers
 * This explains the Register's Layout

 * FES is Read only by default and is enabled only when Tx
 * Config Parameter is enabled for RGMII/SGMII interface
 * during CoreKit Config.

 * DM is Read only with value 1'b1 in Full duplex only Config
 **********************************************************/

/* GmacConfig              = 0x0000,    Mac config Register  Layout */
enum GmacConfigReg
{
                                           /* Bit description                      Bits         R/W   Reset value  */
  GmacWatchdog		   = 0x00800000,
  GmacWatchdogDisable      = 0x00800000,     /* (WD)Disable watchdog timer on Rx      23           RW                */
  GmacWatchdogEnable       = 0x00000000,     /* Enable watchdog timer                                        0       */

  GmacJabber		   = 0x00400000,
  GmacJabberDisable        = 0x00400000,     /* (JD)Disable jabber timer on Tx        22           RW                */
  GmacJabberEnable         = 0x00000000,     /* Enable jabber timer                                          0       */

  GmacFrameBurst           = 0x00200000,
  GmacFrameBurstEnable     = 0x00200000,     /* (BE)Enable frame bursting during Tx   21           RW                */
  GmacFrameBurstDisable    = 0x00000000,     /* Disable frame bursting                                       0       */

  GmacJumboFrame           = 0x00100000,
  GmacJumboFrameEnable     = 0x00100000,     /* (JE)Enable jumbo frame for Tx         20           RW                */
  GmacJumboFrameDisable    = 0x00000000,     /* Disable jumbo frame                                          0       */

  GmacInterFrameGap7       = 0x000E0000,     /* (IFG) Config7 - 40 bit times          19:17        RW                */
  GmacInterFrameGap6       = 0x000C0000,     /* (IFG) Config6 - 48 bit times                                         */
  GmacInterFrameGap5       = 0x000A0000,     /* (IFG) Config5 - 56 bit times                                         */
  GmacInterFrameGap4       = 0x00080000,     /* (IFG) Config4 - 64 bit times                                         */
  GmacInterFrameGap3       = 0x00040000,     /* (IFG) Config3 - 72 bit times                                         */
  GmacInterFrameGap2       = 0x00020000,     /* (IFG) Config2 - 80 bit times                                         */
  GmacInterFrameGap1       = 0x00010000,     /* (IFG) Config1 - 88 bit times                                         */
  GmacInterFrameGap0       = 0x00000000,     /* (IFG) Config0 - 96 bit times                                 000     */

  GmacDisableCrs	   = 0x00010000,
  GmacMiiGmii		   = 0x00008000,
  GmacSelectMii            = 0x00008000,     /* (PS)Port Select-MII mode              15           RW                */
  GmacSelectGmii           = 0x00000000,     /* GMII mode                                                    0       */

  GmacFESpeed100           = 0x00004000,     /*(FES)Fast Ethernet speed 100Mbps       14           RW                */
  GmacFESpeed10            = 0x00000000,     /* 10Mbps                                                       0       */

  GmacRxOwn		   = 0x00002000,
  GmacDisableRxOwn         = 0x00002000,     /* (DO)Disable receive own packets       13           RW                */
  GmacEnableRxOwn          = 0x00000000,     /* Enable receive own packets                                   0       */

  GmacLoopback		   = 0x00001000,
  GmacLoopbackOn           = 0x00001000,     /* (LM)Loopback mode for GMII/MII        12           RW                */
  GmacLoopbackOff          = 0x00000000,     /* Normal mode                                                  0       */

  GmacDuplex		   = 0x00000800,
  GmacFullDuplex           = 0x00000800,     /* (DM)Full duplex mode                  11           RW                */
  GmacHalfDuplex           = 0x00000000,     /* Half duplex mode                                             0       */

  GmacRxIpcOffload	   = 0x00000400,     /*IPC checksum offload		      10           RW        0       */

  GmacRetry		   = 0x00000200,
  GmacRetryDisable         = 0x00000200,     /* (DR)Disable Retry                      9           RW                */
  GmacRetryEnable          = 0x00000000,     /* Enable retransmission as per BL                              0       */

  GmacLinkUp               = 0x00000100,     /* (LUD)Link UP                           8           RW                */
  GmacLinkDown             = 0x00000100,     /* Link Down                                                    0       */

  GmacPadCrcStrip	   = 0x00000080,
  GmacPadCrcStripEnable    = 0x00000080,     /* (ACS) Automatic Pad/Crc strip enable   7           RW                */
  GmacPadCrcStripDisable   = 0x00000000,     /* Automatic Pad/Crc stripping disable                          0       */

  GmacBackoffLimit	   = 0x00000060,
  GmacBackoffLimit3        = 0x00000060,     /* (BL)Back-off limit in HD mode          6:5         RW                */
  GmacBackoffLimit2        = 0x00000040,     /*                                                                      */
  GmacBackoffLimit1        = 0x00000020,     /*                                                                      */
  GmacBackoffLimit0        = 0x00000000,     /*                                                              00      */

  GmacDeferralCheck	   = 0x00000010,
  GmacDeferralCheckEnable  = 0x00000010,     /* (DC)Deferral check enable in HD mode   4           RW                */
  GmacDeferralCheckDisable = 0x00000000,     /* Deferral check disable                                       0       */

  GmacTx		   = 0x00000008,
  GmacTxEnable             = 0x00000008,     /* (TE)Transmitter enable                 3           RW                */
  GmacTxDisable            = 0x00000000,     /* Transmitter disable                                          0       */

  GmacRx		   = 0x00000004,
  GmacRxEnable             = 0x00000004,     /* (RE)Receiver enable                    2           RW                */
  GmacRxDisable            = 0x00000000,     /* Receiver disable                                             0       */
};

/* GmacFrameFilter    = 0x0004,     Mac frame filtering controls Register Layout*/
enum GmacFrameFilterReg
{
  GmacFilter		   = 0x80000000,
  GmacFilterOff            = 0x80000000,     /* (RA)Receive all incoming packets       31         RW                 */
  GmacFilterOn             = 0x00000000,     /* Receive filtered packets only                                0       */

  GmacHashPerfectFilter	   = 0x00000400,     /*Hash or Perfect Filter enable           10         RW         0       */

  GmacSrcAddrFilter	   = 0x00000200,
  GmacSrcAddrFilterEnable  = 0x00000200,     /* (SAF)Source Address Filter enable       9         RW                 */
  GmacSrcAddrFilterDisable = 0x00000000,     /*                                                              0       */

  GmacSrcInvaAddrFilter    = 0x00000100,
  GmacSrcInvAddrFilterEn   = 0x00000100,     /* (SAIF)Inv Src Addr Filter enable        8         RW                 */
  GmacSrcInvAddrFilterDis  = 0x00000000,     /*                                                              0       */

  GmacPassControl	   = 0x000000C0,
  GmacPassControl3         = 0x000000C0,     /* (PCS)Forwards ctrl frms that pass AF    7:6       RW                 */
  GmacPassControl2         = 0x00000080,     /* Forwards all control frames                                          */
  GmacPassControl1         = 0x00000040,     /* Does not pass control frames                                         */
  GmacPassControl0         = 0x00000000,     /* Does not pass control frames                                 00      */

  GmacBroadcast		   = 0x00000020,
  GmacBroadcastDisable     = 0x00000020,     /* (DBF)Disable Rx of broadcast frames     5         RW                 */
  GmacBroadcastEnable      = 0x00000000,     /* Enable broadcast frames                                      0       */

  GmacMulticastFilter      = 0x00000010,
  GmacMulticastFilterOff   = 0x00000010,     /* (PM) Pass all multicast packets         4         RW                 */
  GmacMulticastFilterOn    = 0x00000000,     /* Pass filtered multicast packets                              0       */

  GmacDestAddrFilter       = 0x00000008,
  GmacDestAddrFilterInv    = 0x00000008,     /* (DAIF)Inverse filtering for DA          3         RW                 */
  GmacDestAddrFilterNor    = 0x00000000,     /* Normal filtering for DA                                      0       */

  GmacMcastHashFilter      = 0x00000004,
  GmacMcastHashFilterOn    = 0x00000004,     /* (HMC)perfom multicast hash filtering    2         RW                 */
  GmacMcastHashFilterOff   = 0x00000000,     /* perfect filtering only                                       0       */

  GmacUcastHashFilter      = 0x00000002,
  GmacUcastHashFilterOn    = 0x00000002,     /* (HUC)Unicast Hash filtering only        1         RW                 */
  GmacUcastHashFilterOff   = 0x00000000,     /* perfect filtering only                                       0       */

  GmacPromiscuousMode      = 0x00000001,
  GmacPromiscuousModeOn    = 0x00000001,     /* Receive all frames                      0         RW                 */
  GmacPromiscuousModeOff   = 0x00000000,     /* Receive filtered packets only                                0       */
};


/*GmacGmiiAddr             = 0x0010,    GMII address Register(ext. Phy) Layout          */
enum GmacGmiiAddrReg
{
  GmiiDevMask              = 0x0000F800,     /* (PA)GMII device address                 15:11     RW         0x00    */
  GmiiDevShift             = 11,

  GmiiRegMask              = 0x000007C0,     /* (GR)GMII register in selected Phy       10:6      RW         0x00    */
  GmiiRegShift             = 6,

  GmiiCsrClkMask	   = 0x0000001C,     /*CSR Clock bit Mask			 4:2			     */
  GmiiCsrClk5              = 0x00000014,     /* (CR)CSR Clock Range     250-300 MHz      4:2      RW         000     */
  GmiiCsrClk4              = 0x00000010,     /*                         150-250 MHz                                  */
  GmiiCsrClk3              = 0x0000000C,     /*                         35-60 MHz                                    */
  GmiiCsrClk2              = 0x00000008,     /*                         20-35 MHz                                    */
  GmiiCsrClk1              = 0x00000004,     /*                         100-150 MHz                                  */
  GmiiCsrClk0              = 0x00000000,     /*                         60-100 MHz                                   */

  GmiiWrite                = 0x00000002,     /* (GW)Write to register                      1      RW                 */
  GmiiRead                 = 0x00000000,     /* Read from register                                            0      */

  GmiiBusy                 = 0x00000001,     /* (GB)GMII interface is busy                 0      RW          0      */
};

/* GmacGmiiData            = 0x0014,    GMII data Register(ext. Phy) Layout             */
enum GmacGmiiDataReg
{
  GmiiDataMask             = 0x0000FFFF,     /* (GD)GMII Data                             15:0    RW         0x0000  */
};


/*GmacFlowControl    = 0x0018,    Flow control Register   Layout                  */
enum GmacFlowControlReg
{
  GmacPauseTimeMask        = 0xFFFF0000,     /* (PT) PAUSE TIME field in the control frame  31:16   RW       0x0000  */
  GmacPauseTimeShift       = 16,

  GmacPauseLowThresh	   = 0x00000030,
  GmacPauseLowThresh3      = 0x00000030,     /* (PLT)thresh for pause tmr 256 slot time      5:4    RW               */
  GmacPauseLowThresh2      = 0x00000020,     /*                           144 slot time                              */
  GmacPauseLowThresh1      = 0x00000010,     /*                            28 slot time                              */
  GmacPauseLowThresh0      = 0x00000000,     /*                             4 slot time                       000    */

  GmacUnicastPauseFrame    = 0x00000008,
  GmacUnicastPauseFrameOn  = 0x00000008,     /* (UP)Detect pause frame with unicast addr.     3    RW                */
  GmacUnicastPauseFrameOff = 0x00000000,     /* Detect only pause frame with multicast addr.                   0     */

  GmacRxFlowControl	   = 0x00000004,
  GmacRxFlowControlEnable  = 0x00000004,     /* (RFE)Enable Rx flow control                   2    RW                */
  GmacRxFlowControlDisable = 0x00000000,     /* Disable Rx flow control                                        0     */

  GmacTxFlowControl   	   = 0x00000002,
  GmacTxFlowControlEnable  = 0x00000002,     /* (TFE)Enable Tx flow control                   1    RW                */
  GmacTxFlowControlDisable = 0x00000000,     /* Disable flow control                                           0     */

  GmacFlowControlBackPressure= 0x00000001,
  GmacSendPauseFrame       = 0x00000001,     /* (FCB/PBA)send pause frm/Apply back pressure   0    RW          0     */
};

/*  GmacInterruptStatus	  = 0x0038,     Mac Interrupt ststus register	       */
enum GmacInterruptStatusBitDefinition
{
  GmacTSIntSts		   = 0x00000200,    /* set if int generated due to TS (Read Time Stamp Status Register to know details)*/
  GmacMmcRxChksumOffload   = 0x00000080,    /* set if int generated in MMC RX CHECKSUM OFFLOAD int register	                  */
  GmacMmcTxIntSts	   = 0x00000040,    /* set if int generated in MMC TX Int register			   */
  GmacMmcRxIntSts	   = 0x00000020,    /* set if int generated in MMC RX Int register 			   */
  GmacMmcIntSts		   = 0x00000010,    /* set if any of the above bit [7:5] is set			   */
  GmacPmtIntSts		   = 0x00000008,    /* set whenver magic pkt/wake-on-lan frame is received		   */
  GmacPcsAnComplete	   = 0x00000004,    /* set when AN is complete in TBI/RTBI/SGMIII phy interface        */
  GmacPcsLnkStsChange	   = 0x00000002,    /* set if any lnk status change in TBI/RTBI/SGMII interface        */
  GmacRgmiiIntSts	   = 0x00000001,    /* set if any change in lnk status of RGMII interface		   */

};

/*  GmacInterruptMask       = 0x003C,     Mac Interrupt Mask register	       */
enum GmacInterruptMaskBitDefinition
{
  GmacTSIntMask		   = 0x00000200,    /* when set disables the time stamp interrupt generation            */
  GmacPmtIntMask	   = 0x00000008,    /* when set Disables the assertion of PMT interrupt     	    	*/
  GmacPcsAnIntMask	   = 0x00000004,    /* When set disables the assertion of PCS AN complete interrupt	      	*/
  GmacPcsLnkStsIntMask	   = 0x00000002,    /* when set disables the assertion of PCS lnk status change interrupt	*/
  GmacRgmiiIntMask	   = 0x00000001,    /* when set disables the assertion of RGMII int 			*/
};

/**********************************************************
 * GMAC DMA registers
 * For Pci based system address is BARx + GmaDmaBase
 * For any other system translation is done accordingly
 **********************************************************/

enum DmaRegisters
{
  DmaBusMode        = 0x0000,    /* CSR0 - Bus Mode Register                          */
  DmaTxPollDemand   = 0x0004,    /* CSR1 - Transmit Poll Demand Register              */
  DmaRxPollDemand   = 0x0008,    /* CSR2 - Receive Poll Demand Register               */
  DmaRxBaseAddr     = 0x000C,    /* CSR3 - Receive Descriptor list base address       */
  DmaTxBaseAddr     = 0x0010,    /* CSR4 - Transmit Descriptor list base address      */
  DmaStatus         = 0x0014,    /* CSR5 - Dma status Register                        */
  DmaControl        = 0x0018,    /* CSR6 - Dma Operation Mode Register                */
  DmaInterrupt      = 0x001C,    /* CSR7 - Interrupt enable                           */
  DmaMissedFr       = 0x0020,    /* CSR8 - Missed Frame & Buffer overflow Counter     */
  DmaTxCurrDesc     = 0x0048,    /*      - Current host Tx Desc Register              */
  DmaRxCurrDesc     = 0x004C,    /*      - Current host Rx Desc Register              */
  DmaTxCurrAddr     = 0x0050,    /* CSR20 - Current host transmit buffer address      */
  DmaRxCurrAddr     = 0x0054,    /* CSR21 - Current host receive buffer address       */

  #ifdef AVB_SUPPORT
  HwFeature         = 0x0058,    /* Hardware Feature Register                         */

  DmaSlotFnCtrlSts  = 0x0030,    /* Slot function control and status register         */

  DmaChannelCtrl    = 0x0060,    /* Channel Control register only for Channel1 and Channel2 */
  DmaChannelAvSts   = 0x0064,    /* Channel Status register only for Channel1 and  Channel2 */
  IdleSlopeCredit   = 0x0068,    /* Idle slope credit register                              */
  SendSlopeCredit   = 0x006C,    /* Send slope credit register                              */
  HighCredit        = 0x0070,    /* High Credit register                                    */
  LoCredit          = 0x0074,    /* Lo Credit Register                                      */
  #endif

};

/**********************************************************
 * DMA Engine registers Layout
 **********************************************************/

/*DmaBusMode               = 0x0000,    CSR0 - Bus Mode */
enum DmaBusModeReg
{
                                          /* Bit description                                Bits     R/W   Reset value */
  #ifdef AVB_SUPPORT
  DmaChannelPrioWt       = 0x30000000,    /* Channel priority weight mask                     29:28    RW       0       */
  DmaChannelPrio1        = 0x00000000,    /* Channel priority weight 1                        29:28    RW       0       */
  DmaChannelPrio2        = 0x10000000,    /* Channel priority weight 2                        29:28    RW       0       */
  DmaChannelPrio3        = 0x20000000,    /* Channel priority weight 3                        29:28    RW       0       */
  DmaChannelPrio4        = 0x30000000,    /* Channel priority weight 4                        29:28    RW       0       */

  DmaTxRxPrio            = 0x08000000,    /* When set indicates Tx Dma has more priority       27      RW        0       */

  DmaPriorityRatio11     = 0x00000000,   /* (PR)TX:RX DMA priority ratio 1:1                15:14   RW        00      */
  DmaPriorityRatio21     = 0x00004000,   /* (PR)TX:RX DMA priority ratio 2:1                                          */
  DmaPriorityRatio31     = 0x00008000,   /* (PR)TX:RX DMA priority ratio 3:1                                          */
  DmaPriorityRatio41     = 0x0000C000,   /* (PR)TX:RX DMA priority ratio 4:1                                          */

  DmaArbitration         = 0x00000002,    /* Dma Arbitration decides whether strict prio or RR  1      RW       0       */
  DmaArbitrationStrict   = 0x00000002,    /* Dma Arbitration decides whether strict prio or RR  1      RW       0       */
  DmaArbitrationRR       = 0x00000000,    /* Dma Arbitration decides whether strict prio or RR  0      RW       0       */
  #endif


  DmaFixedBurstEnable     = 0x00010000,   /* (FB)Fixed Burst SINGLE, INCR4, INCR8 or INCR16   16     RW                */
  DmaFixedBurstDisable    = 0x00000000,   /*             SINGLE, INCR                                          0       */

  DmaTxPriorityRatio11    = 0x00000000,   /* (PR)TX:RX DMA priority ratio 1:1                15:14   RW        00      */
  DmaTxPriorityRatio21    = 0x00004000,   /* (PR)TX:RX DMA priority ratio 2:1                                          */
  DmaTxPriorityRatio31    = 0x00008000,   /* (PR)TX:RX DMA priority ratio 3:1                                          */
  DmaTxPriorityRatio41    = 0x0000C000,   /* (PR)TX:RX DMA priority ratio 4:1                                          */

  DmaBurstLengthx8        = 0x01000000,   /* When set mutiplies the PBL by 8                  24      RW        0      */

  DmaBurstLength256       = 0x01002000,   /*(DmaBurstLengthx8 | DmaBurstLength32) = 256      [24]:13:8                 */
  DmaBurstLength128       = 0x01001000,   /*(DmaBurstLengthx8 | DmaBurstLength16) = 128      [24]:13:8                 */
  DmaBurstLength64        = 0x01000800,   /*(DmaBurstLengthx8 | DmaBurstLength8) = 64        [24]:13:8                 */
  DmaBurstLength32        = 0x00002000,   /* (PBL) programmable Dma burst length = 32        13:8    RW                */
  DmaBurstLength16        = 0x00001000,   /* Dma burst length = 16                                                     */
  DmaBurstLength8         = 0x00000800,   /* Dma burst length = 8                                                      */
  DmaBurstLength4         = 0x00000400,   /* Dma burst length = 4                                                      */
  DmaBurstLength2         = 0x00000200,   /* Dma burst length = 2                                                      */
  DmaBurstLength1         = 0x00000100,   /* Dma burst length = 1                                                      */
  DmaBurstLength0         = 0x00000000,   /* Dma burst length = 0                                               0x00   */

  DmaDescriptor8Words     = 0x00000080,   /* Enh Descriptor works  1=> 8 word descriptor      7                  0    */
  DmaDescriptor4Words     = 0x00000000,   /* Enh Descriptor works  0=> 4 word descriptor      7                  0    */

  DmaDescriptorSkip16     = 0x00000040,   /* (DSL)Descriptor skip length (no.of dwords)       6:2     RW               */
  DmaDescriptorSkip8      = 0x00000020,   /* between two unchained descriptors                                         */
  DmaDescriptorSkip4      = 0x00000010,   /*                                                                           */
  DmaDescriptorSkip2      = 0x00000008,   /*                                                                           */
  DmaDescriptorSkip1      = 0x00000004,   /*                                                                           */
  DmaDescriptorSkip0      = 0x00000000,   /*                                                                    0x00   */

  DmaArbitRr              = 0x00000000,   /* (DA) DMA RR arbitration                            1     RW         0     */
  DmaArbitPr              = 0x00000002,   /* Rx has priority over Tx                                                   */

  DmaResetOn              = 0x00000001,   /* (SWR)Software Reset DMA engine                     0     RW               */
  DmaResetOff             = 0x00000000,   /*                                                                      0    */
};


/*DmaStatus         = 0x0014,    CSR5 - Dma status Register                        */
enum DmaStatusReg
{
  /*Bit 28 27 and 26 indicate whether the interrupt due to PMT GMACMMC or GMAC LINE Remaining bits are DMA interrupts*/

  #ifdef AVB_SUPPORT
  DmaSlotCounterIntr      = 0x40000000,   /* For Ch1 and Ch2 AVB slot interrupt status          31     RW       0       */
  #endif
  #ifdef LPI_SUPPORT
  GmacLPIIntr             = 0x40000000,   /* GMC LPI interrupt                                  31     RO       0       */
  #endif

  GmacPmtIntr             = 0x10000000,   /* (GPI)Gmac subsystem interrupt                      28     RO       0       */
  GmacMmcIntr             = 0x08000000,   /* (GMI)Gmac MMC subsystem interrupt                  27     RO       0       */
  GmacLineIntfIntr        = 0x04000000,   /* Line interface interrupt                           26     RO       0       */

  DmaErrorBit2            = 0x02000000,   /* (EB)Error bits 0-data buffer, 1-desc. access       25     RO       0       */
  DmaErrorBit1            = 0x01000000,   /* (EB)Error bits 0-write trnsf, 1-read transfr       24     RO       0       */
  DmaErrorBit0            = 0x00800000,   /* (EB)Error bits 0-Rx DMA, 1-Tx DMA                  23     RO       0       */

  DmaTxState              = 0x00700000,   /* (TS)Transmit process state                         22:20  RO               */
  DmaTxStopped            = 0x00000000,   /* Stopped - Reset or Stop Tx Command issued                         000      */
  DmaTxFetching           = 0x00100000,   /* Running - fetching the Tx descriptor                                       */
  DmaTxWaiting            = 0x00200000,   /* Running - waiting for status                                               */
  DmaTxReading            = 0x00300000,   /* Running - reading the data from host memory                                */
  DmaTxSuspended          = 0x00600000,   /* Suspended - Tx Descriptor unavailabe                                       */
  DmaTxClosing            = 0x00700000,   /* Running - closing Rx descriptor                                            */

  DmaRxState              = 0x000E0000,   /* (RS)Receive process state                         19:17  RO                */
  DmaRxStopped            = 0x00000000,   /* Stopped - Reset or Stop Rx Command issued                         000      */
  DmaRxFetching           = 0x00020000,   /* Running - fetching the Rx descriptor                                       */
  DmaRxWaiting            = 0x00060000,   /* Running - waiting for packet                                               */
  DmaRxSuspended          = 0x00080000,   /* Suspended - Rx Descriptor unavailable                                      */
  DmaRxClosing            = 0x000A0000,   /* Running - closing descriptor                                               */
  DmaRxQueuing            = 0x000E0000,   /* Running - queuing the recieve frame into host memory                       */

  DmaIntNormal            = 0x00010000,   /* (NIS)Normal interrupt summary                     16     RW        0       */
  DmaIntAbnormal          = 0x00008000,   /* (AIS)Abnormal interrupt summary                   15     RW        0       */

  DmaIntEarlyRx           = 0x00004000,   /* Early receive interrupt (Normal)       RW        0       */
  DmaIntBusError          = 0x00002000,   /* Fatal bus error (Abnormal)             RW        0       */
  DmaIntEarlyTx           = 0x00000400,   /* Early transmit interrupt (Abnormal)    RW        0       */
  DmaIntRxWdogTO          = 0x00000200,   /* Receive Watchdog Timeout (Abnormal)    RW        0       */
  DmaIntRxStopped         = 0x00000100,   /* Receive process stopped (Abnormal)     RW        0       */
  DmaIntRxNoBuffer        = 0x00000080,   /* Receive buffer unavailable (Abnormal)  RW        0       */
  DmaIntRxCompleted       = 0x00000040,   /* Completion of frame reception (Normal) RW        0       */
  DmaIntTxUnderflow       = 0x00000020,   /* Transmit underflow (Abnormal)          RW        0       */
  DmaIntRcvOverflow       = 0x00000010,   /* Receive Buffer overflow interrupt      RW        0       */
  DmaIntTxJabberTO        = 0x00000008,   /* Transmit Jabber Timeout (Abnormal)     RW        0       */
  DmaIntTxNoBuffer        = 0x00000004,   /* Transmit buffer unavailable (Normal)   RW        0       */
  DmaIntTxStopped         = 0x00000002,   /* Transmit process stopped (Abnormal)    RW        0       */
  DmaIntTxCompleted       = 0x00000001,   /* Transmit completed (Normal)            RW        0       */
};

/*DmaControl        = 0x0018,     CSR6 - Dma Operation Mode Register                */
enum DmaControlReg
{
  DmaDisableDropTcpCs	  = 0x04000000,   /* (DT) Dis. drop. of tcp/ip CS error frames        26      RW        0       */

  DmaStoreAndForward      = 0x00200000,   /* (SF)Store and forward                            21      RW        0       */
  DmaFlushTxFifo          = 0x00100000,   /* (FTF)Tx FIFO controller is reset to default      20      RW        0       */

  DmaTxThreshCtrl         = 0x0001C000,   /* (TTC)Controls thre Threh of MTL tx Fifo          16:14   RW                */
  DmaTxThreshCtrl16       = 0x0001C000,   /* (TTC)Controls thre Threh of MTL tx Fifo 16       16:14   RW                */
  DmaTxThreshCtrl24       = 0x00018000,   /* (TTC)Controls thre Threh of MTL tx Fifo 24       16:14   RW                */
  DmaTxThreshCtrl32       = 0x00014000,   /* (TTC)Controls thre Threh of MTL tx Fifo 32       16:14   RW                */
  DmaTxThreshCtrl40       = 0x00010000,   /* (TTC)Controls thre Threh of MTL tx Fifo 40       16:14   RW                */
  DmaTxThreshCtrl256      = 0x0000c000,   /* (TTC)Controls thre Threh of MTL tx Fifo 256      16:14   RW                */
  DmaTxThreshCtrl192      = 0x00008000,   /* (TTC)Controls thre Threh of MTL tx Fifo 192      16:14   RW                */
  DmaTxThreshCtrl128      = 0x00004000,   /* (TTC)Controls thre Threh of MTL tx Fifo 128      16:14   RW                */
  DmaTxThreshCtrl64       = 0x00000000,   /* (TTC)Controls thre Threh of MTL tx Fifo 64       16:14   RW        000     */

  DmaTxStart              = 0x00002000,   /* (ST)Start/Stop transmission                      13      RW        0       */

  DmaRxFlowCtrlDeact      = 0x00401800,   /* (RFD)Rx flow control deact. threhold             [22]:12:11   RW                 */
  DmaRxFlowCtrlDeact1K    = 0x00000000,   /* (RFD)Rx flow control deact. threhold (1kbytes)   [22]:12:11   RW        00       */
  DmaRxFlowCtrlDeact2K    = 0x00000800,   /* (RFD)Rx flow control deact. threhold (2kbytes)   [22]:12:11   RW                 */
  DmaRxFlowCtrlDeact3K    = 0x00001000,   /* (RFD)Rx flow control deact. threhold (3kbytes)   [22]:12:11   RW                 */
  DmaRxFlowCtrlDeact4K    = 0x00001800,   /* (RFD)Rx flow control deact. threhold (4kbytes)   [22]:12:11   RW                 */
  DmaRxFlowCtrlDeact5K    = 0x00400000,   /* (RFD)Rx flow control deact. threhold (4kbytes)   [22]:12:11   RW                 */
  DmaRxFlowCtrlDeact6K    = 0x00400800,   /* (RFD)Rx flow control deact. threhold (4kbytes)   [22]:12:11   RW                 */
  DmaRxFlowCtrlDeact7K    = 0x00401000,   /* (RFD)Rx flow control deact. threhold (4kbytes)   [22]:12:11   RW                 */

  DmaRxFlowCtrlAct        = 0x00800600,   /* (RFA)Rx flow control Act. threhold              [23]:10:09   RW                 */
  DmaRxFlowCtrlAct1K      = 0x00000000,   /* (RFA)Rx flow control Act. threhold (1kbytes)    [23]:10:09   RW        00       */
  DmaRxFlowCtrlAct2K      = 0x00000200,   /* (RFA)Rx flow control Act. threhold (2kbytes)    [23]:10:09   RW                 */
  DmaRxFlowCtrlAct3K      = 0x00000400,   /* (RFA)Rx flow control Act. threhold (3kbytes)    [23]:10:09   RW                 */
  DmaRxFlowCtrlAct4K      = 0x00000300,   /* (RFA)Rx flow control Act. threhold (4kbytes)    [23]:10:09   RW                 */
  DmaRxFlowCtrlAct5K      = 0x00800000,   /* (RFA)Rx flow control Act. threhold (5kbytes)    [23]:10:09   RW                 */
  DmaRxFlowCtrlAct6K      = 0x00800200,   /* (RFA)Rx flow control Act. threhold (6kbytes)    [23]:10:09   RW                 */
  DmaRxFlowCtrlAct7K      = 0x00800400,   /* (RFA)Rx flow control Act. threhold (7kbytes)    [23]:10:09   RW                 */

  DmaRxThreshCtrl         = 0x00000018,   /* (RTC)Controls thre Threh of MTL rx Fifo          4:3   RW                */
  DmaRxThreshCtrl64       = 0x00000000,   /* (RTC)Controls thre Threh of MTL tx Fifo 64       4:3   RW                */
  DmaRxThreshCtrl32       = 0x00000008,   /* (RTC)Controls thre Threh of MTL tx Fifo 32       4:3   RW                */
  DmaRxThreshCtrl96       = 0x00000010,   /* (RTC)Controls thre Threh of MTL tx Fifo 96       4:3   RW                */
  DmaRxThreshCtrl128      = 0x00000018,   /* (RTC)Controls thre Threh of MTL tx Fifo 128      4:3   RW                */

  DmaEnHwFlowCtrl         = 0x00000100,   /* (EFC)Enable HW flow control                      8       RW                 */
  DmaDisHwFlowCtrl        = 0x00000000,   /* Disable HW flow control                                            0        */

  DmaFwdErrorFrames       = 0x00000080,   /* (FEF)Forward error frames                        7       RW        0       */
  DmaFwdUnderSzFrames     = 0x00000040,   /* (FUF)Forward undersize frames                    6       RW        0       */
  DmaTxSecondFrame        = 0x00000004,   /* (OSF)Operate on second frame                     4       RW        0       */
  DmaRxStart              = 0x00000002,   /* (SR)Start/Stop reception                         1       RW        0       */
};


/*DmaInterrupt      = 0x001C,    CSR7 - Interrupt enable Register Layout     */
enum  DmaInterruptReg
{
  DmaIeNormal            = DmaIntNormal     ,   /* Normal interrupt enable                 RW        0       */
  DmaIeAbnormal          = DmaIntAbnormal   ,   /* Abnormal interrupt enable               RW        0       */

  DmaIeEarlyRx           = DmaIntEarlyRx    ,   /* Early receive interrupt enable          RW        0       */
  DmaIeBusError          = DmaIntBusError   ,   /* Fatal bus error enable                  RW        0       */
  DmaIeEarlyTx           = DmaIntEarlyTx    ,   /* Early transmit interrupt enable         RW        0       */
  DmaIeRxWdogTO          = DmaIntRxWdogTO   ,   /* Receive Watchdog Timeout enable         RW        0       */
  DmaIeRxStopped         = DmaIntRxStopped  ,   /* Receive process stopped enable          RW        0       */
  DmaIeRxNoBuffer        = DmaIntRxNoBuffer ,   /* Receive buffer unavailable enable       RW        0       */
  DmaIeRxCompleted       = DmaIntRxCompleted,   /* Completion of frame reception enable    RW        0       */
  DmaIeTxUnderflow       = DmaIntTxUnderflow,   /* Transmit underflow enable               RW        0       */

  DmaIeRxOverflow        = DmaIntRcvOverflow,   /* Receive Buffer overflow interrupt       RW        0       */
  DmaIeTxJabberTO        = DmaIntTxJabberTO ,   /* Transmit Jabber Timeout enable          RW        0       */
  DmaIeTxNoBuffer        = DmaIntTxNoBuffer ,   /* Transmit buffer unavailable enable      RW        0       */
  DmaIeTxStopped         = DmaIntTxStopped  ,   /* Transmit process stopped enable         RW        0       */
  DmaIeTxCompleted       = DmaIntTxCompleted,   /* Transmit completed enable               RW        0       */
};

/*
**********Enhanced Descritpor structure to support 8K buffer per buffer ****************************

DmaRxBaseAddr     = 0x000C,   CSR3 - Receive Descriptor list base address
DmaRxBaseAddr is the pointer to the first Rx Descriptors. the Descriptor format in Little endian with a
32 bit Data bus is as shown below

Similarly
DmaTxBaseAddr     = 0x0010,  CSR4 - Transmit Descriptor list base address
DmaTxBaseAddr is the pointer to the first Rx Descriptors. the Descriptor format in Little endian with a
32 bit Data bus is as shown below
                  --------------------------------------------------------------------------
    RDES0	  |OWN (31)| Status                                                        |
		  --------------------------------------------------------------------------
    RDES1	  | Ctrl | Res | Byte Count Buffer 2 | Ctrl | Res | Byte Count Buffer 1    |
		  --------------------------------------------------------------------------
    RDES2	  |  Buffer 1 Address                                                      |
		  --------------------------------------------------------------------------
    RDES3	  |  Buffer 2 Address / Next Descriptor Address                            |
		  --------------------------------------------------------------------------

                  --------------------------------------------------------------------------
    TDES0	  |OWN (31)| Ctrl | Res | Ctrl | Res | Status                              |
		  --------------------------------------------------------------------------
    TDES1	  | Res | Byte Count Buffer 2 | Res |         Byte Count Buffer 1          |
		  --------------------------------------------------------------------------
    TDES2	  |  Buffer 1 Address                                                      |
		  --------------------------------------------------------------------------
    TDES3         |  Buffer 2 Address / Next Descriptor Address                            |
		  --------------------------------------------------------------------------

*/

enum DmaDescriptorStatus    /* status word of DMA descriptor */
{

  DescOwnByDma          = 0x80000000,   /* (OWN)Descriptor is owned by DMA engine              31      RW                  */

  DescDAFilterFail      = 0x40000000,   /* (AFM)Rx - DA Filter Fail for the rx frame           30                          */

  DescFrameLengthMask   = 0x3FFF0000,   /* (FL)Receive descriptor frame length                 29:16                       */
  DescFrameLengthShift  = 16,

  DescError             = 0x00008000,   /* (ES)Error summary bit  - OR of the follo. bits:     15                          */
					/*  DE || OE || IPC || LC || RWT || RE || CE */
  DescRxTruncated       = 0x00004000,   /* (DE)Rx - no more descriptors for receive frame      14                          */
  DescSAFilterFail      = 0x00002000,   /* (SAF)Rx - SA Filter Fail for the received frame     13                          */
  DescRxLengthError	    = 0x00001000,   /* (LE)Rx - frm size not matching with len field       12                          */
  DescRxDamaged         = 0x00000800,   /* (OE)Rx - frm was damaged due to buffer overflow     11                          */
  DescRxVLANTag         = 0x00000400,   /* (VLAN)Rx - received frame is a VLAN frame           10                          */
  DescRxFirst           = 0x00000200,   /* (FS)Rx - first descriptor of the frame              9                          */
  DescRxLast            = 0x00000100,   /* (LS)Rx - last descriptor of the frame               8                          */
  DescRxLongFrame       = 0x00000080,   /* (Giant Frame)Rx - frame is longer than 1518/1522    7                          */
  DescRxCollision       = 0x00000040,   /* (LC)Rx - late collision occurred during reception   6                          */
  DescRxFrameEther      = 0x00000020,   /* (FT)Rx - Frame type - Ethernet, otherwise 802.3     5                          */
  DescRxWatchdog        = 0x00000010,   /* (RWT)Rx - watchdog timer expired during reception   4                          */
  DescRxMiiError        = 0x00000008,   /* (RE)Rx - error reported by MII interface            3                          */
  DescRxDribbling       = 0x00000004,   /* (DE)Rx - frame contains non int multiple of 8 bits  2                          */
  DescRxCrc             = 0x00000002,   /* (CE)Rx - CRC error                                  1                          */
//  DescRxMacMatch        = 0x00000001,   /* (RX MAC Address) Rx mac address reg(1 to 15)match   0                          */

  DescRxEXTsts          = 0x00000001,   /* Extended Status Available (RDES4)                           0                          */

  DescTxIntEnable       = 0x40000000,   /* (IC)Tx - interrupt on completion                    30                       */
  DescTxLast            = 0x20000000,   /* (LS)Tx - Last segment of the frame                  29                       */
  DescTxFirst           = 0x10000000,   /* (FS)Tx - First segment of the frame                 28                       */
  DescTxDisableCrc      = 0x08000000,   /* (DC)Tx - Add CRC disabled (first segment only)      27                       */
  DescTxDisablePadd   	= 0x04000000,   /* (DP)disable padding, added by - reyaz               26                       */

  DescTxCisMask     	= 0x00c00000,   /* Tx checksum offloading control mask		       23:22			*/
  DescTxCisBypass   	= 0x00000000,   /* Checksum bypass								*/
  DescTxCisIpv4HdrCs	= 0x00400000,	/* IPv4 header checksum								*/
  DescTxCisTcpOnlyCs    = 0x00800000,	/* TCP/UDP/ICMP checksum. Pseudo header checksum is assumed to be present	*/
  DescTxCisTcpPseudoCs  = 0x00c00000,	/* TCP/UDP/ICMP checksum fully in hardware including pseudo header		*/

  TxDescEndOfRing       = 0x00200000,   /* (TER)End of descriptors ring                        21                       */
  TxDescChain           = 0x00100000,   /* (TCH)Second buffer address is chain address         20                       */

  DescRxChkBit0		    = 0x00000001,   /*()  Rx - Rx Payload Checksum Error                   0                          */
  DescRxChkBit7	    	= 0x00000080,   /* (IPC CS ERROR)Rx - Ipv4 header checksum error       7                          */
  DescRxChkBit5 		= 0x00000020,   /* (FT)Rx - Frame type - Ethernet, otherwise 802.3     5                          */

  DescRxTSavail         = 0x00000080,   /* Time stamp available                                7                          */
  DescRxFrameType   	= 0x00000020,   /* (FT)Rx - Frame type - Ethernet, otherwise 802.3     5                          */

  DescTxIpv4ChkError    = 0x00010000,   /* (IHE) Tx Ip header error                            16                         */
  DescTxTimeout         = 0x00004000,   /* (JT)Tx - Transmit jabber timeout                    14                         */
  DescTxFrameFlushed    = 0x00002000,   /* (FF)Tx - DMA/MTL flushed the frame due to SW flush  13                         */
  DescTxPayChkError     = 0x00001000,   /* (PCE) Tx Payload checksum Error                     12                         */
  DescTxLostCarrier     = 0x00000800,   /* (LC)Tx - carrier lost during tramsmission           11                         */
  DescTxNoCarrier       = 0x00000400,   /* (NC)Tx - no carrier signal from the tranceiver      10                         */
  DescTxLateCollision   = 0x00000200,   /* (LC)Tx - transmission aborted due to collision      9                         */
  DescTxExcCollisions   = 0x00000100,   /* (EC)Tx - transmission aborted after 16 collisions   8                         */
  DescTxVLANFrame       = 0x00000080,   /* (VF)Tx - VLAN-type frame                            7                         */

  DescTxCollMask        = 0x00000078,   /* (CC)Tx - Collision count                            6:3                        */
  DescTxCollShift       = 3,

  DescTxExcDeferral     = 0x00000004,   /* (ED)Tx - excessive deferral                         2                        */
  DescTxUnderflow       = 0x00000002,   /* (UF)Tx - late data arrival from the memory          1                        */
  DescTxDeferred        = 0x00000001,   /* (DB)Tx - frame transmision deferred                 0                        */

	/*
	This explains the RDES1/TDES1 bits layout
			  --------------------------------------------------------------------
	    RDES1/TDES1  | Control Bits | Byte Count Buffer 2 | Byte Count Buffer 1          |
			  --------------------------------------------------------------------

	*/
// DmaDescriptorLength     length word of DMA descriptor


  RxDisIntCompl		= 0x80000000,	/* (Disable Rx int on completion) 			31			*/
  RxDescEndOfRing       = 0x00008000,   /* (TER)End of descriptors ring                         15                       */
  RxDescChain           = 0x00004000,   /* (TCH)Second buffer address is chain address          14                       */


  DescSize2Mask         = 0x1FFF0000,   /* (TBS2) Buffer 2 size                                28:16                    */
  DescSize2Shift        = 16,
  DescSize1Mask         = 0x00001FFF,   /* (TBS1) Buffer 1 size                                12:0                     */
  DescSize1Shift        = 0,


	/*
	This explains the RDES4 Extended Status bits layout
			   --------------------------------------------------------------------
	  RDES4   |                             Extended Status                        |
			   --------------------------------------------------------------------
	*/

  #ifdef AVB_SUPPORT
  DescRxVlanPrioVal     = 0x001C0000,    /* Gives the VLAN Priority Value                      20:18                     */
  DescRxVlanPrioShVal   = 18,            /* Gives the shift value to get priority value in LS bits                       */

  DescRxAvTagPktRx      = 0x00020000,    /* Indicates AV tagged Packet is received              17                       */
  DescRxAvPktRx         = 0x00010000,    /* Indicates AV Packet received                        16                       */

  #endif

  DescRxPtpAvail        = 0x00004000,    /* PTP snapshot available                              14                        */
  DescRxPtpVer          = 0x00002000,    /* When set indicates IEEE1584 Version 2 (else Ver1)   13                        */
  DescRxPtpFrameType    = 0x00001000,    /* PTP frame type Indicates PTP sent over ethernet     12                        */
  DescRxPtpMessageType  = 0x00000F00,    /* Message Type                                        11:8                      */
  DescRxPtpNo           = 0x00000000,    /* 0000 => No PTP message received                                               */
  DescRxPtpSync         = 0x00000100,    /* 0001 => Sync (all clock types) received                                       */
  DescRxPtpFollowUp     = 0x00000200,    /* 0010 => Follow_Up (all clock types) received                                  */
  DescRxPtpDelayReq     = 0x00000300,    /* 0011 => Delay_Req (all clock types) received                                  */
  DescRxPtpDelayResp    = 0x00000400,    /* 0100 => Delay_Resp (all clock types) received                                 */
  DescRxPtpPdelayReq    = 0x00000500,    /* 0101 => Pdelay_Req (in P to P tras clk)  or Announce in Ord and Bound clk     */
  DescRxPtpPdelayResp   = 0x00000600,    /* 0110 => Pdealy_Resp(in P to P trans clk) or Management in Ord and Bound clk   */
  DescRxPtpPdelayRespFP = 0x00000700,    /* 0111 => Pdealy_Resp_Follow_Up (in P to P trans clk) or Signaling in Ord and Bound clk   */
  DescRxPtpIPV6         = 0x00000080,    /* Received Packet is  in IPV6 Packet                  7                         */
  DescRxPtpIPV4         = 0x00000040,    /* Received Packet is  in IPV4 Packet                  6                         */

  DescRxChkSumBypass    = 0x00000020,    /* When set indicates checksum offload engine          5
                                            is bypassed                                                                   */
  DescRxIpPayloadError  = 0x00000010,    /* When set indicates 16bit IP payload CS is in error  4                         */
  DescRxIpHeaderError   = 0x00000008,    /* When set indicates 16bit IPV4 header CS is in       3
                                            error or IP datagram version is not consistent
                                            with Ethernet type value                                                      */
  DescRxIpPayloadType   = 0x00000007,     /* Indicate the type of payload encapsulated          2:0
                                             in IPdatagram processed by COE (Rx)                                          */
  DescRxIpPayloadUnknown= 0x00000000,     /* Unknown or didnot process IP payload                                         */
  DescRxIpPayloadUDP    = 0x00000001,     /* UDP                                                                          */
  DescRxIpPayloadTCP    = 0x00000002,     /* TCP                                                                          */
  DescRxIpPayloadICMP   = 0x00000003,     /* ICMP                                                                         */

};

/* From enum InitialRegisters */
enum DmaIntDisableVal
{
  DmaIntDisable           = 0,
};

/* Mac Management Counters (MMC): the interrupt masks only */
enum MMC_INTR_MASK
{
	GmacMmcIntrMaskRx		= 0x010C,	/* mask for interrupts generated from rx counters					*/
	GmacMmcIntrMaskTx		= 0x0110,	/* mask for interrupts generated from tx counters					*/
	GmacMmcRxIpcIntrMask		= 0x0200,	/*Maintains the mask for interrupt generated from rx IPC statistic counters 		*/
};

/* The functions drivers/net/jz_gmac.c uses */
s32 synopGMAC_set_mdc_clk_div(synopGMACdevice *gmacdev,u32 clk_div_val);
u32 synopGMAC_get_mdc_clk_div(synopGMACdevice *gmacdev);
s32 synopGMAC_read_phy_reg(u32 *RegBase,u32 PhyBase, u32 RegOffset, u16 * data);
s32 synopGMAC_write_phy_reg(u32 *RegBase, u32 PhyBase, u32 RegOffset, u16 data);
s32 synopGMAC_reset (synopGMACdevice * gmacdev );
s32 synopGMAC_dma_bus_mode_init(synopGMACdevice * gmacdev, u32 init_value );
s32 synopGMAC_dma_control_init(synopGMACdevice * gmacdev, u32 init_value);
void synopGMAC_wd_disable(synopGMACdevice * gmacdev);
void synopGMAC_jab_disable(synopGMACdevice * gmacdev);
void synopGMAC_jumbo_frame_enable(synopGMACdevice * gmacdev);
void synopGMAC_select_gmii(synopGMACdevice * gmacdev);
void synopGMAC_select_mii(synopGMACdevice * gmacdev);
void synopGMAC_rx_own_disable(synopGMACdevice * gmacdev);
void synopGMAC_loopback_off(synopGMACdevice * gmacdev);
void synopGMAC_set_full_duplex(synopGMACdevice * gmacdev);
void synopGMAC_set_half_duplex(synopGMACdevice * gmacdev);
void synopGMAC_retry_enable(synopGMACdevice * gmacdev);
void synopGMAC_pad_crc_strip_disable(synopGMACdevice * gmacdev);
void synopGMAC_back_off_limit(synopGMACdevice * gmacdev, u32 value);
void synopGMAC_deferral_check_disable(synopGMACdevice * gmacdev);
void synopGMAC_rx_enable(synopGMACdevice * gmacdev);
void synopGMAC_rx_disable(synopGMACdevice * gmacdev);
void synopGMAC_tx_enable(synopGMACdevice * gmacdev);
void synopGMAC_frame_filter_enable(synopGMACdevice * gmacdev);
void synopGMAC_src_addr_filter_enable(synopGMACdevice * gmacdev);
void synopGMAC_dst_addr_filter_normal(synopGMACdevice * gmacdev);
void synopGMAC_set_pass_control(synopGMACdevice * gmacdev,u32 passcontrol);
void synopGMAC_broadcast_enable(synopGMACdevice * gmacdev);
void synopGMAC_multicast_disable(synopGMACdevice * gmacdev);
void synopGMAC_multicast_hash_filter_disable(synopGMACdevice * gmacdev);
void synopGMAC_promisc_disable(synopGMACdevice * gmacdev);
void synopGMAC_unicast_hash_filter_disable(synopGMACdevice * gmacdev);
void synopGMAC_unicast_pause_frame_detect_enable(synopGMACdevice * gmacdev);
void synopGMAC_rx_flow_control_enable(synopGMACdevice * gmacdev);
void synopGMAC_tx_flow_control_enable(synopGMACdevice * gmacdev);
s32 synopGMAC_search_phy (synopGMACdevice * gmacdev);
s32 synopGMAC_check_phy_init(synopGMACdevice * gmacdev);
s32 synopGMAC_set_mac_addr(synopGMACdevice *gmacdev, u32 MacHigh, u32 MacLow, u8 *MacAddr);
void synopGMAC_rx_desc_init_ring(DmaDesc *desc, bool last_ring_desc);
void synopGMAC_tx_desc_init_ring(DmaDesc *desc, bool last_ring_desc);
bool synopGMAC_is_desc_owned_by_dma(volatile DmaDesc *desc);
u32 synopGMAC_get_rx_desc_frame_length(u32 status);
void synopGMAC_clear_interrupt(synopGMACdevice *gmacdev);
void synopGMAC_disable_interrupt_all(synopGMACdevice *gmacdev);
void synopGMAC_enable_dma_rx(synopGMACdevice * gmacdev);
void synopGMAC_enable_dma_tx(synopGMACdevice * gmacdev);
void synopGMAC_resume_dma_tx(synopGMACdevice * gmacdev);
void synopGMAC_resume_dma_rx(synopGMACdevice * gmacdev);
void synopGMAC_disable_dma_tx(synopGMACdevice * gmacdev);
void synopGMAC_disable_dma_rx(synopGMACdevice * gmacdev);
void synopGMAC_disable_mmc_tx_interrupt(synopGMACdevice *gmacdev, u32 mask);
void synopGMAC_disable_mmc_rx_interrupt(synopGMACdevice *gmacdev, u32 mask);
void synopGMAC_disable_mmc_ipc_rx_interrupt(synopGMACdevice *gmacdev, u32 mask);

#endif /* SYNOP_GMAC_H */
