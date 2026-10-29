// SPDX-License-Identifier: MIT
/*
 * Synopsys DesignWare GMAC core functions, as the Ingenic T-series has it.
 *
 * From drivers/net/SynopGMAC_Dev.c of OpenIPC/u-boot-ingenic (Synopsys,
 * 2007-2009, with the Ingenic and OpenIPC changes), trimmed to the
 * functions drivers/net/jz_gmac.c uses; their bodies are unchanged.
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
 */

#include <common.h>
#include <linux/delay.h>
#include <asm/arch/base.h>
#include "synop_gmac.h"

s32 synopGMAC_set_mdc_clk_div(synopGMACdevice *gmacdev,u32 clk_div_val)
{
	u32 orig_data;
	orig_data = synopGMACReadReg((u32 *)gmacdev->MacBase,GmacGmiiAddr); //set the mdc clock to the user defined value
	orig_data &= (~ GmiiCsrClkMask);
	orig_data |= clk_div_val;
	synopGMACWriteReg((u32 *)gmacdev->MacBase, GmacGmiiAddr ,orig_data);
	return 0;
}

u32 synopGMAC_get_mdc_clk_div(synopGMACdevice *gmacdev)
{
	u32 data;
	data = synopGMACReadReg((u32 *)gmacdev->MacBase,GmacGmiiAddr);
	data &= GmiiCsrClkMask;
	return data;
}

s32 synopGMAC_read_phy_reg(u32 *RegBase,u32 PhyBase, u32 RegOffset, u16 * data)
{
	u32 addr;
	u32 loop_variable;

	//printf("read phybase = %x, regoffset = %x\n", PhyBase, RegOffset);
	while(synopGMACReadReg(RegBase,GmacGmiiAddr) & GmiiBusy) {
		printf("===>PHY busy!\n");
	}

	addr = ((PhyBase << GmiiDevShift) & GmiiDevMask) | ((RegOffset << GmiiRegShift) & GmiiRegMask);
	addr = addr | GmiiBusy ; //Gmii busy bit

	// printf("====>PHY: addr = 0x%08x\n", addr);

	synopGMACWriteReg(RegBase,GmacGmiiAddr,addr); //write the address from where the data to be read in GmiiGmiiAddr register of synopGMAC ip

	for(loop_variable = 0; loop_variable < DEFAULT_LOOP_VARIABLE; loop_variable++){ //Wait till the busy bit gets cleared with in a certain amount of time
		if (!(synopGMACReadReg(RegBase,GmacGmiiAddr) & GmiiBusy)){
			break;
		}
		plat_delay(DEFAULT_DELAY_VARIABLE);
	}
	if(loop_variable < DEFAULT_LOOP_VARIABLE) {
		*data = (u16)(synopGMACReadReg(RegBase,GmacGmiiData) & 0xFFFF);
		//printf("read phybase = %x, regoffset = %x, data = %x\n", PhyBase, RegOffset, *data);
		//		 printf("======>PHY: data = 0x%04x\n", *data);
	} else{
		//		 printf("====>Error::: PHY not responding Busy bit didnot get cleared !!!!!!\n");
		return -ESYNOPGMACPHYERR;
	}
	return -ESYNOPGMACNOERR;
}

s32 synopGMAC_write_phy_reg(u32 *RegBase, u32 PhyBase, u32 RegOffset, u16 data)
{
	u32 addr;
	u32 loop_variable;

	//printf("write phybase = %x, regoffset = %x, data = %x\n", PhyBase, RegOffset, data);
	synopGMACWriteReg(RegBase,GmacGmiiData,data); // write the data in to GmacGmiiData register of synopGMAC ip

	addr = ((PhyBase << GmiiDevShift) & GmiiDevMask) | ((RegOffset << GmiiRegShift) & GmiiRegMask) | GmiiWrite;

	addr = addr | GmiiBusy ; //set Gmii clk to 20-35 Mhz and Gmii busy bit

	synopGMACWriteReg(RegBase,GmacGmiiAddr,addr);
	for(loop_variable = 0; loop_variable < DEFAULT_LOOP_VARIABLE; loop_variable++){
		if (!(synopGMACReadReg(RegBase,GmacGmiiAddr) & GmiiBusy)){
			break;
		}
		plat_delay(DEFAULT_DELAY_VARIABLE);
	}

	if(loop_variable < DEFAULT_LOOP_VARIABLE){
		return -ESYNOPGMACNOERR;
	}
	else{
		TR("Error::: PHY not responding Busy bit didnot get cleared !!!!!!\n");
		return -ESYNOPGMACPHYERR;
	}
}

s32 synopGMAC_reset (synopGMACdevice * gmacdev )
{
	u32 data = 0;
	int cnt = 0;
	synopGMACWriteReg((u32 *)gmacdev->DmaBase, DmaBusMode ,DmaResetOn);
	plat_delay(DEFAULT_LOOP_VARIABLE);
	data = synopGMACReadReg((u32 *)gmacdev->DmaBase, DmaBusMode);
	while (1) {
		data = synopGMACReadReg((u32 *)gmacdev->DmaBase, DmaBusMode);
		TR("DATA after Reset = %08x\n",data);
		if (data & DmaResetOn) {
			if(cnt > 10)
				printf("Bus Mode Reg after reset: 0x%08x\n", data);
			udelay(1);
			cnt ++;
		} else{
			printf("Bus Mode Reg after write: 0x%08x\n", data);
			break;
		}
	}

	return 0;
}

s32 synopGMAC_dma_bus_mode_init(synopGMACdevice * gmacdev, u32 init_value )
{
	synopGMACWriteReg((u32 *)gmacdev->DmaBase, DmaBusMode ,init_value);
	TR("bus_mode after init: 0x%08x\n", synopGMACReadReg((u32 *)gmacdev->DmaBase, DmaBusMode));
	return 0;

}

s32 synopGMAC_dma_control_init(synopGMACdevice * gmacdev, u32 init_value)
{
	synopGMACWriteReg((u32 *)gmacdev->DmaBase, DmaControl, init_value);
	return 0;
}

void synopGMAC_wd_disable(synopGMACdevice * gmacdev)
{
	synopGMACSetBits((u32 *)gmacdev->MacBase, GmacConfig, GmacWatchdog);
	return;
}

void synopGMAC_jab_disable(synopGMACdevice * gmacdev)
{
	// synopGMACClearBits((u32 *)gmacdev->MacBase, GmacConfig, GmacJabber);
	synopGMACSetBits((u32 *)gmacdev->MacBase, GmacConfig, GmacJabber);
	return;
}

void synopGMAC_jumbo_frame_enable(synopGMACdevice * gmacdev)
{
	synopGMACSetBits((u32 *)gmacdev->MacBase, GmacConfig, GmacJumboFrame);
	return;
}

void synopGMAC_select_gmii(synopGMACdevice * gmacdev)
{
	synopGMACClearBits((u32 *)gmacdev->MacBase, GmacConfig, GmacMiiGmii);
	return;
}

void synopGMAC_select_mii(synopGMACdevice * gmacdev)
{
	synopGMACSetBits((u32 *)gmacdev->MacBase, GmacConfig, GmacMiiGmii);
	return;
}

void synopGMAC_rx_own_disable(synopGMACdevice * gmacdev)
{
	synopGMACSetBits((u32 *)gmacdev->MacBase, GmacConfig, GmacRxOwn);
	return;
}

void synopGMAC_loopback_off(synopGMACdevice * gmacdev)
{
	synopGMACClearBits((u32 *)gmacdev->MacBase, GmacConfig, GmacLoopback);
	return;
}

void synopGMAC_set_full_duplex(synopGMACdevice * gmacdev)
{
	synopGMACSetBits((u32 *)gmacdev->MacBase, GmacConfig, GmacDuplex);
	return;
}

void synopGMAC_set_half_duplex(synopGMACdevice * gmacdev)
{
	synopGMACClearBits((u32 *)gmacdev->MacBase, GmacConfig, GmacDuplex);
	return;
}

void synopGMAC_retry_enable(synopGMACdevice * gmacdev)
{
	synopGMACClearBits((u32 *)gmacdev->MacBase, GmacConfig, GmacRetry);
	return;
}

void synopGMAC_pad_crc_strip_disable(synopGMACdevice * gmacdev)
{
	synopGMACClearBits((u32 *)gmacdev->MacBase, GmacConfig, GmacPadCrcStrip);
	return;
}

void synopGMAC_back_off_limit(synopGMACdevice * gmacdev, u32 value)
{
	u32 data;
	data = synopGMACReadReg((u32 *)gmacdev->MacBase, GmacConfig);
	data &= (~GmacBackoffLimit);//edit by liuyang the privious is no number
	data |= value;
	synopGMACWriteReg((u32 *)gmacdev->MacBase, GmacConfig,data);
	return;
}

void synopGMAC_deferral_check_disable(synopGMACdevice * gmacdev)
{
	synopGMACClearBits((u32 *)gmacdev->MacBase, GmacConfig, GmacDeferralCheck);
	return;
}

void synopGMAC_rx_enable(synopGMACdevice * gmacdev)
{
	synopGMACSetBits((u32 *)gmacdev->MacBase, GmacConfig, GmacRx);
	return;
}

void synopGMAC_rx_disable(synopGMACdevice * gmacdev)
{
	synopGMACClearBits((u32 *)gmacdev->MacBase, GmacConfig, GmacRx);
	return;
}

void synopGMAC_tx_enable(synopGMACdevice * gmacdev)
{
	synopGMACSetBits((u32 *)gmacdev->MacBase, GmacConfig, GmacTx);
	return;
}

void synopGMAC_frame_filter_enable(synopGMACdevice * gmacdev)
{
	synopGMACClearBits((u32 *)gmacdev->MacBase, GmacFrameFilter, GmacFilter);
	return;
}

void synopGMAC_src_addr_filter_enable(synopGMACdevice * gmacdev)
{
	synopGMACSetBits((u32 *)gmacdev->MacBase, GmacFrameFilter, GmacSrcAddrFilter);
	return;
}

void synopGMAC_dst_addr_filter_normal(synopGMACdevice * gmacdev)
{
	synopGMACClearBits((u32 *)gmacdev->MacBase, GmacFrameFilter, GmacDestAddrFilterNor);
	return;
}

void synopGMAC_set_pass_control(synopGMACdevice * gmacdev,u32 passcontrol)
{
	u32 data;
	data = synopGMACReadReg((u32 *)gmacdev->MacBase, GmacFrameFilter);
	data &= (~GmacPassControl);
	data |= passcontrol;
	synopGMACWriteReg((u32 *)gmacdev->MacBase,GmacFrameFilter,data);
	return;
}

void synopGMAC_broadcast_enable(synopGMACdevice * gmacdev)
{
	synopGMACClearBits((u32 *)gmacdev->MacBase, GmacFrameFilter, GmacBroadcast);
	return;
}

void synopGMAC_multicast_disable(synopGMACdevice * gmacdev)
{
	synopGMACClearBits((u32 *)gmacdev->MacBase, GmacFrameFilter, GmacMulticastFilter);
	return;
}

void synopGMAC_multicast_hash_filter_disable(synopGMACdevice * gmacdev)
{
	synopGMACClearBits((u32 *)gmacdev->MacBase, GmacFrameFilter, GmacMcastHashFilter);
	return;
}

void synopGMAC_promisc_disable(synopGMACdevice * gmacdev)
{
	synopGMACClearBits((u32 *)gmacdev->MacBase, GmacFrameFilter, GmacPromiscuousMode);
	return;
}

void synopGMAC_unicast_hash_filter_disable(synopGMACdevice * gmacdev)
{
	synopGMACClearBits((u32 *)gmacdev->MacBase, GmacFrameFilter, GmacUcastHashFilter);
	return;
}

void synopGMAC_unicast_pause_frame_detect_enable(synopGMACdevice * gmacdev)
{
	synopGMACSetBits((u32 *)gmacdev->MacBase, GmacFlowControl, GmacUnicastPauseFrame);
	return;
}

void synopGMAC_rx_flow_control_enable(synopGMACdevice * gmacdev)
{
	synopGMACSetBits((u32 *)gmacdev->MacBase, GmacFlowControl, GmacRxFlowControl);
	return;
}

void synopGMAC_tx_flow_control_enable(synopGMACdevice * gmacdev)
{
	synopGMACSetBits((u32 *)gmacdev->MacBase, GmacFlowControl, GmacTxFlowControl);
	return;
}

s32 synopGMAC_search_phy (synopGMACdevice * gmacdev) {
	int phy_id = 0;
	u16 data;
	s32 status = 0;
	u16 id1 = 0;
	u16 id2 = 0;

	for (phy_id = 0; phy_id < 32; phy_id++) {
        status += synopGMAC_read_phy_reg((u32 *)gmacdev->MacBase,phy_id, PHY_ID_HI_REG, &id1);
        status += synopGMAC_read_phy_reg((u32 *)gmacdev->MacBase,phy_id, PHY_ID_LOW_REG, &id2);
        if ((!status)&&(id2 != 0xffff)) {
            if ((0x0 == id1) && (0x0 == id2)) {
                return -ESYNOPGMACPHYERR;
            }
            printf("====>phy %d: 0x%x-0x%x found\n", phy_id, id1, id2);
            if ((0x0 == id1) && ((0x128 == id2) || (0x118 == id2))) {
                status = synopGMAC_write_phy_reg((u32 *)gmacdev->MacBase, 0, 0x1e, 0x50);
                status = synopGMAC_read_phy_reg((u32 *)gmacdev->MacBase, 0, 0x1f, &data);
                /* bit6: clk devide to 25M */
                data |= 0x40;
                status = synopGMAC_write_phy_reg((u32 *)gmacdev->MacBase, 0, 0x1f, data);

                status = synopGMAC_write_phy_reg((u32 *)gmacdev->MacBase, 0, 0x1e, 0x4000);
                status = synopGMAC_read_phy_reg((u32 *)gmacdev->MacBase, 0, 0x1f, &data);
                //data = 0x13;
                data |= 0x10;
                status = synopGMAC_write_phy_reg((u32 *)gmacdev->MacBase, 0, 0x1f, data);

#if 1
                /* restart Auto Negotiation */
                status = synopGMAC_write_phy_reg((u32 *)gmacdev->MacBase, 0, 0x4, 0x1e1);
                status = synopGMAC_write_phy_reg((u32 *)gmacdev->MacBase, 0, 0x0, 0x3100);
                status = synopGMAC_write_phy_reg((u32 *)gmacdev->MacBase, 0, 0x0, 0x3300);
#endif
                /* config leds */
                status = synopGMAC_write_phy_reg((u32 *)gmacdev->MacBase, gmacdev->PhyBase, 0x1e, 0x40c0);
                status += synopGMAC_read_phy_reg((u32 *)gmacdev->MacBase, gmacdev->PhyBase, 0x1f, &data);
                data &= 0xffe0;
                data |= 0x1000;
                status += synopGMAC_write_phy_reg((u32 *)gmacdev->MacBase, gmacdev->PhyBase, 0x1f, data);

                status += synopGMAC_write_phy_reg((u32 *)gmacdev->MacBase, gmacdev->PhyBase, 0x1e, 0x40c3);
                status += synopGMAC_read_phy_reg((u32 *)gmacdev->MacBase, gmacdev->PhyBase, 0x1f, &data);
                data &= 0xfcff;
                data |= 0x10;
                status += synopGMAC_write_phy_reg((u32 *)gmacdev->MacBase, gmacdev->PhyBase, 0x1f, data);
                if(status)
                    printf("ethphy: config leds failed! %d\n", data);
                /* read status */

                status = synopGMAC_read_phy_reg((u32 *)gmacdev->MacBase,gmacdev->PhyBase, 0x0, &data);
                if(status)
                    return status;
                if((data&0x2000)==(0x2000)) {
                    gmacdev->Speed      =   SPEED100;
                } else if((data&0x2000)==(0x0000)) {
                    gmacdev->Speed      =   SPEED10;
                }

                if((data&(0x100))==0x100) {
                    gmacdev->DuplexMode = FULLDUPLEX;
                } else {
                    gmacdev->DuplexMode = HALFDUPLEX;
                }
            }
			printf("SPEED:%d, DUPLEX:%d\n", gmacdev->Speed, gmacdev->DuplexMode);
            break;
        }
	}
    if (phy_id >= 32) return -ESYNOPGMACPHYERR;

    status = synopGMAC_read_phy_reg((u32 *)gmacdev->MacBase, phy_id, PHY_STATUS_REG, &data);
    if ( (!status) && (data != 0xffff)) {
        if((data & Mii_AutoNegCmplt) != 0){
            printf("====>phy %d Autonegotiation Complete\n", phy_id);
        }
    }

	return phy_id;
}

#ifdef CONFIG_T41
/*
 * The T41 SDK's: the PHY found by its OUI in phy_lists[], whose
 * check_init reads link, speed and duplex its own way. Without a match
 * phy_list is left at the last entry, and that one runs, as in the SDK.
 */
static int check_phy_init_ds008(synopGMACdevice *gmacdev)
{
    u16 data;
    s32 status = -ESYNOPGMACNOERR;

    printf("########### check_phy_init_ds008 #############\n");

    status = synopGMAC_read_phy_reg((u32 *)gmacdev->MacBase,gmacdev->PhyBase,PHY_STATUS_REG, &data);
    if(status)
        return status;
    if((data & Mii_Link) == 0){
        printf("No Link\n");
        gmacdev->LinkState = LINKDOWN;
        return -ESYNOPGMACPHYERR;
    }
    else{
        gmacdev->LinkState = LINKUP;
        printf("Link UP\n");
    }

    status = synopGMAC_read_phy_reg((u32 *)gmacdev->MacBase,gmacdev->PhyBase,PHY_CONTROL_REG, &data);
	if(status)
		return status;

	if(data & 0x2000)
		gmacdev->Speed      =   SPEED100;
	else
		gmacdev->Speed      =   SPEED10;
	if(data & 0x0100)
		gmacdev->DuplexMode = FULLDUPLEX;
	else
		gmacdev->DuplexMode = HALFDUPLEX;

	return status;
}
static int check_phy_init_ip101g(synopGMACdevice *gmacdev)
{
    u16 data;
    s32 status = -ESYNOPGMACNOERR;
    int speed_bit;

    printf("########### check_phy_init_ip101g #############\n");

    status = synopGMAC_read_phy_reg((u32 *)gmacdev->MacBase,gmacdev->PhyBase,PHY_STATUS_REG, &data);
    if(status)
        return status;
    if((data & Mii_Link) == 0){
        printf("No Link\n");
        gmacdev->LinkState = LINKDOWN;
        return -ESYNOPGMACPHYERR;
    }
    else{
        gmacdev->LinkState = LINKUP;
        printf("Link UP\n");
    }

    status = synopGMAC_read_phy_reg((u32 *)gmacdev->MacBase,gmacdev->PhyBase,PHY_SPECIFIC_STATUS_REG, &data);
    if(status)
        return status;
    speed_bit = (data & (0xf<<5)) >> 5;

	switch(speed_bit) {
        case 0x8:
            gmacdev->Speed = SPEED100;
            gmacdev->DuplexMode = FULLDUPLEX;
            break;
        case 0x4:
            gmacdev->Speed = SPEED100;
            gmacdev->DuplexMode = HALFDUPLEX;
            break;
        case 0x2:
            gmacdev->Speed = SPEED10;
            gmacdev->DuplexMode = FULLDUPLEX;
            break;
        case 0x1:
            gmacdev->Speed = SPEED10;
            gmacdev->DuplexMode = HALFDUPLEX;
            break;
    }
    return status;
}

static int check_phy_init_8720a(synopGMACdevice *gmacdev)
{
    u16 data;
    s32 status = -ESYNOPGMACNOERR;

    printf("######### check_phy_init_8720a #############\n");

    status = synopGMAC_read_phy_reg((u32 *)gmacdev->MacBase,gmacdev->PhyBase,PHY_STATUS_REG, &data);
    if(status)
        return status;
    if((data & Mii_Link) == 0){
        TR("No Link\n");
        gmacdev->LinkState = LINKDOWN;
        return -ESYNOPGMACPHYERR;
    }
    else{
        gmacdev->LinkState = LINKUP;
        TR("Link UP\n");
    }
    //status = synopGMAC_read_phy_reg((u32 *)gmacdev->MacBase,gmacdev->PhyBase,PHY_SPECIFIC_STATUS_REG, &data);
    status = synopGMAC_read_phy_reg((u32 *)gmacdev->MacBase,gmacdev->PhyBase, 31, &data);
    if(status)
        return status;
    switch((data >> 2) & 0x7) {
        case 0x1:
            gmacdev->DuplexMode = HALFDUPLEX;
            gmacdev->Speed      =   SPEED10;
            break;
        case 0x5:
            gmacdev->DuplexMode = FULLDUPLEX;
            gmacdev->Speed      =   SPEED10;
            break;
        case 0x2:
            gmacdev->DuplexMode = HALFDUPLEX;
            gmacdev->Speed      =   SPEED100;
            break;
        case 0x6:
            gmacdev->DuplexMode = FULLDUPLEX;
            gmacdev->Speed = SPEED100;
            break;
        default:
            printf("unsupported speed Mode!, trying default speed Mode, data: %x\n", data);
            gmacdev->DuplexMode = FULLDUPLEX;
            gmacdev->Speed      =   SPEED100;
            break;

    }

    return status;
}


struct phy_list {
    unsigned int oui_id;
    int (*check_init)(synopGMACdevice *gmacdev);
};

struct phy_list phy_lists[] = {
/* add supported phy here, and rewrite check_init in need */
    [0] = {
        .oui_id = 0x300007,
        .check_init = check_phy_init_8720a,
    },
    [1] = {
        .oui_id = 0x30243,
        .check_init = check_phy_init_ip101g,
    },
	[2] = {
        .oui_id = 0x10937c,
        .check_init = check_phy_init_ds008,
    },

};


static int check_phy_negotiation_status(synopGMACdevice *gmacdev)
{
    int i;
    struct phy_list * phy_list;
    int status = 0;
    unsigned int phy_id_hi = 0, phy_id_low = 0, phy_id = 0;
    u16 data = 0;;

    status = synopGMAC_read_phy_reg((u32 *)gmacdev->MacBase,gmacdev->PhyBase,PHY_ID_HI_REG, &data);
    phy_id_low = data; /* OUI 3 to 18 bit */
    status = synopGMAC_read_phy_reg((u32 *)gmacdev->MacBase,gmacdev->PhyBase,PHY_ID_LOW_REG, &data);
    phy_id_hi = ((data >> 10) & 0x3f); /* OUI 19 to 24 bit */
    phy_id = phy_id_hi << 16 | phy_id_low;

    printf("mac phy_id is: %x\n", phy_id);

    for(i = 0; i < ARRAY_SIZE(phy_lists); i++) {
        phy_list = &phy_lists[i];
        if(phy_list->oui_id == phy_id) {
            break;
        }
    }

    if((phy_list != NULL) && (phy_list->check_init != NULL)) {
        status = phy_list->check_init(gmacdev);
    } else {
        printf("#### ERROR ###, need phy check_init.        \n  \
            please implement phy check_init function .  \n  \
            or check wheter your phy_type is in phy_lists[]\n");
    }

    return status;

}


s32 synopGMAC_check_phy_init(synopGMACdevice * gmacdev) {

    u16 data;
    s32 status = -ESYNOPGMACNOERR;
    s32 loop_count;

    status = synopGMAC_read_phy_reg((u32 *)gmacdev->MacBase,gmacdev->PhyBase,PHY_CONTROL_REG, &data);
    if(status)
        return status;
    if (data & 0x1000) {//Autonegotiation
        //loop_count = DEFAULT_LOOP_VARIABLE;
        /* delay 2 second */
        loop_count = 20000;
        while(loop_count-- > 0){
            udelay(100);
            status = synopGMAC_read_phy_reg((u32 *)gmacdev->MacBase,gmacdev->PhyBase,PHY_STATUS_REG, &data);
            if(status)
                return status;

            if((data & Mii_AutoNegCmplt) != 0){
                TR("Autonegotiation Complete\n");
                break;
            }
        }

        status = check_phy_negotiation_status(gmacdev);
        if(status) {
            printf("Falied to check negotiation status!\n");
        }
        printf("====>PHY Autonegotiation Complete!\n");

    } else {
        status = synopGMAC_read_phy_reg((u32 *)gmacdev->MacBase,gmacdev->PhyBase,PHY_STATUS_REG, &data);
        if(status)
            return status;
        if((data & 1) == 0){
            TR("No Link\n");
            gmacdev->LinkState = LINKDOWN;
            return -ESYNOPGMACPHYERR;
        }
        else{
            gmacdev->LinkState = LINKUP;
            TR("Link UP\n");
        }
        status = synopGMAC_read_phy_reg((u32 *)gmacdev->MacBase,gmacdev->PhyBase,PHY_CONTROL_REG, &data);
        if(status)
            return status;
		if(data & 0x2000)
            gmacdev->Speed      =   SPEED100;
        else
            gmacdev->Speed      =   SPEED10;
        if(data & 0x0100)
            gmacdev->DuplexMode = FULLDUPLEX;
        else
            gmacdev->DuplexMode = HALFDUPLEX;
    }

    printf("Link is up in %s mode\n",(gmacdev->DuplexMode == FULLDUPLEX) ? "FULL DUPLEX": "HALF DUPLEX");
    if(gmacdev->Speed == SPEED1000)
        printf("Link is with 1000M Speed \n");
    if(gmacdev->Speed == SPEED100)
        printf("Link is with 100M Speed \n");
    if(gmacdev->Speed == SPEED10)
        printf("Link is with 10M Speed \n");

    return -ESYNOPGMACNOERR;

}
#else
s32 synopGMAC_check_phy_init(synopGMACdevice * gmacdev) {

    u16 data;
    s32 status = -ESYNOPGMACNOERR;
    s32 loop_count;
    u16 id1 = 0;
    u16 id2 = 0;

    status = synopGMAC_read_phy_reg((u32 *)gmacdev->MacBase,gmacdev->PhyBase,PHY_CONTROL_REG, &data);
    if(status)
        return status;
    if (data & 0x1000) {//Autonegotiation
        //loop_count = DEFAULT_LOOP_VARIABLE;
        /* delay 2 second */
        loop_count = 20000;
        while(loop_count-- > 0){
            udelay(100);
            status = synopGMAC_read_phy_reg((u32 *)gmacdev->MacBase,gmacdev->PhyBase,PHY_STATUS_REG, &data);
            if(status)
                return status;

            if((data & Mii_AutoNegCmplt) != 0){
                TR("Autonegotiation Complete\n");
                break;
            }
        }

        printf("====>PHY Autonegotiation Complete!\n");
        status = synopGMAC_read_phy_reg((u32 *)gmacdev->MacBase,gmacdev->PhyBase,PHY_STATUS_REG, &data);

        status += synopGMAC_read_phy_reg((u32 *)gmacdev->MacBase,gmacdev->PhyBase, 2, &id1);
        status += synopGMAC_read_phy_reg((u32 *)gmacdev->MacBase,gmacdev->PhyBase, 3, &id2);

        if(status)
            return status;
#if (CONFIG_NET_PHY_TYPE == PHY_TYPE_DM9161) || (CONFIG_NET_PHY_TYPE == PHY_TYPE_8710A) || (CONFIG_NET_PHY_TYPE == PHY_TYPE_IP101G)
		/* RTL8201F */
		if((data & 1) == 0){
			TR("No Link\n");
			gmacdev->LinkState = LINKDOWN;
			return -ESYNOPGMACPHYERR;
		}
		else{
			gmacdev->LinkState = LINKUP;
			TR("Link UP\n");
		}
		status = synopGMAC_read_phy_reg((u32 *)gmacdev->MacBase,gmacdev->PhyBase,0x0, &data);
		if(status)
			return status;
		if(data&0x0100) {
			gmacdev->DuplexMode = FULLDUPLEX;
		}
		else {
			gmacdev->DuplexMode = HALFDUPLEX;
		}
		if(data&0x2000) {
			gmacdev->Speed      =   SPEED100;
		}
		else {
			gmacdev->Speed = SPEED10;
		}

        if((data & 1) == 0){
            TR("No Link\n");
            gmacdev->LinkState = LINKDOWN;
            return -ESYNOPGMACPHYERR;
        }else{
            gmacdev->LinkState = LINKUP;
            TR("Link UP\n");
        }
#elif (CONFIG_NET_PHY_TYPE == PHY_TYPE_88E1111)
        if((data & 0x4) == 0){
            TR("No Link\n");
            gmacdev->LinkState = LINKDOWN;
            return -ESYNOPGMACPHYERR;
        }
        else{
            gmacdev->LinkState = LINKUP;
            TR("Link UP\n");
        }
        status = synopGMAC_read_phy_reg((u32 *)gmacdev->MacBase,gmacdev->PhyBase,PHY_SPECIFIC_STATUS_REG, &data);
        int speed_bit;
        speed_bit = data & (0x3<<14);
        switch(speed_bit) {
        case 0x8000:
            gmacdev->Speed = SPEED1000;
            break;
        case 0x4000:
            gmacdev->Speed = SPEED100;
            break;
        case 0x0000:
            gmacdev->Speed = SPEED10;
            break;
        }
        if(data & (0x1<<13)) {
            gmacdev->DuplexMode = FULLDUPLEX;
        } else {
            gmacdev->DuplexMode = HALFDUPLEX;
        }
#endif //CONFIG_NET_PHY_TYPE
    } else {
        status = synopGMAC_read_phy_reg((u32 *)gmacdev->MacBase,gmacdev->PhyBase,PHY_STATUS_REG, &data);
        if(status)
            return status;
        if((data & 1) == 0){
            TR("No Link\n");
            gmacdev->LinkState = LINKDOWN;
            return -ESYNOPGMACPHYERR;
        }
        else{
            gmacdev->LinkState = LINKUP;
            TR("Link UP\n");
        }
        status = synopGMAC_read_phy_reg((u32 *)gmacdev->MacBase,gmacdev->PhyBase,PHY_CONTROL_REG, &data);
        if(status)
            return status;
        if(data & 0x2000)
            gmacdev->Speed      =   SPEED100;
        else
            gmacdev->Speed      =   SPEED10;
        if(data & 0x0100)
            gmacdev->DuplexMode = FULLDUPLEX;
        else
            gmacdev->DuplexMode = HALFDUPLEX;
    }

    TR("Link is up in %s mode\n",(gmacdev->DuplexMode == FULLDUPLEX) ? "FULL DUPLEX": "HALF DUPLEX");
    if(gmacdev->Speed == SPEED1000)
        TR("Link is with 1000M Speed \n");
    if(gmacdev->Speed == SPEED100)
        TR("Link is with 100M Speed \n");
    if(gmacdev->Speed == SPEED10)
        TR("Link is with 10M Speed \n");

    return -ESYNOPGMACNOERR;

}
#endif

s32 synopGMAC_set_mac_addr(synopGMACdevice *gmacdev, u32 MacHigh, u32 MacLow, u8 *MacAddr)
{
	u32 data;

	data = (MacAddr[5] << 8) | MacAddr[4];
	synopGMACWriteReg((u32 *)gmacdev->MacBase,MacHigh,data);
	data = (MacAddr[3] << 24) | (MacAddr[2] << 16) | (MacAddr[1] << 8) | MacAddr[0] ;
	synopGMACWriteReg((u32 *)gmacdev->MacBase,MacLow,data);
	return 0;
}

void synopGMAC_rx_desc_init_ring(DmaDesc *desc, bool last_ring_desc)
{
	desc->status = 0;
	desc->length = last_ring_desc ? RxDescEndOfRing : 0;
	desc->buffer1 = 0;
	desc->buffer2 = 0;
	desc->data1 = 0;
	desc->data2 = 0;
	return;
}

void synopGMAC_tx_desc_init_ring(DmaDesc *desc, bool last_ring_desc)
{
#ifdef ENH_DESC
	desc->status |= (last_ring_desc? TxDescEndOfRing : 0);
	desc->length = 0;
#else
	desc->length = last_ring_desc? TxDescEndOfRing : 0;
#endif
	desc->buffer1 = 0;
	desc->buffer2 = 0;
	desc->data1 = 0;
	desc->data2 = 0;
	return;
}

bool synopGMAC_is_desc_owned_by_dma(volatile DmaDesc *desc)
{
	return ((desc->status & DescOwnByDma) == DescOwnByDma );
}

u32 synopGMAC_get_rx_desc_frame_length(u32 status)
{
	return ((status & DescFrameLengthMask) >> DescFrameLengthShift);
}

void synopGMAC_clear_interrupt(synopGMACdevice *gmacdev)
{
	u32 data;
	data = synopGMACReadReg((u32 *)gmacdev->DmaBase, DmaStatus);
	synopGMACWriteReg((u32 *)gmacdev->DmaBase, DmaStatus ,data);
}

void synopGMAC_disable_interrupt_all(synopGMACdevice *gmacdev)
{
	synopGMACWriteReg((u32 *)gmacdev->DmaBase, DmaInterrupt, DmaIntDisable);
	return;
}

void synopGMAC_enable_dma_rx(synopGMACdevice * gmacdev)
{
	//	synopGMACSetBits((u32 *)gmacdev->DmaBase, DmaControl, DmaRxStart);
	u32 data;
	data = synopGMACReadReg((u32 *)gmacdev->DmaBase, DmaControl);
	data |= DmaRxStart;
	synopGMACWriteReg((u32 *)gmacdev->DmaBase, DmaControl ,data);

}

void synopGMAC_enable_dma_tx(synopGMACdevice * gmacdev)
{
	//	synopGMACSetBits((u32 *)gmacdev->DmaBase, DmaControl, DmaTxStart);
	u32 data;
	data = synopGMACReadReg((u32 *)gmacdev->DmaBase, DmaControl);
	data |= DmaTxStart;
	synopGMACWriteReg((u32 *)gmacdev->DmaBase, DmaControl ,data);

}

void synopGMAC_resume_dma_tx(synopGMACdevice * gmacdev)
{
	synopGMACWriteReg((u32 *)gmacdev->DmaBase, DmaTxPollDemand, 0);

}

void synopGMAC_resume_dma_rx(synopGMACdevice * gmacdev)
{
	synopGMACWriteReg((u32 *)gmacdev->DmaBase, DmaRxPollDemand, 0);

}

void synopGMAC_disable_dma_tx(synopGMACdevice * gmacdev)
{
	//	synopGMACClearBits((u32 *)gmacdev->DmaBase, DmaControl, DmaTxStart);
	u32 data;
	data = synopGMACReadReg((u32 *)gmacdev->DmaBase, DmaControl);
	data &= (~DmaTxStart);
	synopGMACWriteReg((u32 *)gmacdev->DmaBase, DmaControl ,data);
}

void synopGMAC_disable_dma_rx(synopGMACdevice * gmacdev)
{
	//	synopGMACClearBits((u32 *)gmacdev->DmaBase, DmaControl, DmaRxStart);
	u32 data;
	data = synopGMACReadReg((u32 *)gmacdev->DmaBase, DmaControl);
	data &= (~DmaRxStart);
	synopGMACWriteReg((u32 *)gmacdev->DmaBase, DmaControl ,data);
}

void synopGMAC_disable_mmc_tx_interrupt(synopGMACdevice *gmacdev, u32 mask)
{
	synopGMACSetBits((u32 *)gmacdev->MacBase,GmacMmcIntrMaskTx,mask);
	return;
}

void synopGMAC_disable_mmc_rx_interrupt(synopGMACdevice *gmacdev, u32 mask)
{
	synopGMACSetBits((u32 *)gmacdev->MacBase,GmacMmcIntrMaskRx,mask);
	return;
}

void synopGMAC_disable_mmc_ipc_rx_interrupt(synopGMACdevice *gmacdev, u32 mask)
{
	synopGMACSetBits((u32 *)gmacdev->MacBase,GmacMmcRxIpcIntrMask,mask);
	return;
}
