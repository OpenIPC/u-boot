/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * SigmaStar (MStar) infinity3: MSC313E, MSC316DC, MSC316DM.
 *
 * Every peripheral sits on the RIU, a bus of 16-bit registers: bank B
 * (256 bytes of register space, 128 registers) at 0x1F000000 + B * 0x200,
 * register R at + R * 4, its upper 16 bits reading 0. The vendor sources
 * name a register by its 8-bit address, 0xBBBBRR / 2 for the register R
 * of bank B.
 */

#ifndef __SIGMASTAR_H
#define __SIGMASTAR_H

#define SS_RIU_BASE		0x1F000000
#define SS_BANK(b)		(SS_RIU_BASE + (b) * 0x200)
#define SS_REG(b, r)		(SS_BANK(b) + (r) * 4)

#define SS_DDR_BASE		0x20000000
/* The SPI NOR, read through the XIU window */
#define SS_XIU_FLASH_BASE	0x14000000

/* Banks */
#define SS_BANK_PMSLEEP		0x000E
#define SS_BANK_FSP		0x0016
#define SS_BANK_PMTOP		0x001E
#define SS_BANK_TIMER0		0x0030	/* the PIU timers, timer 0 at reg 0x10 */
#define SS_BANK_ALBANY0		0x0031	/* the internal Ethernet PHY */
#define SS_BANK_ALBANY1		0x0032
#define SS_BANK_ALBANY2		0x0033
#define SS_BANK_BDMA		0x1002	/* channel 0 */
#define SS_BANK_CHIPTOP		0x101E
#define SS_BANK_L3BRIDGE	0x1022
#define SS_BANK_CLKGEN0		0x1038
#define SS_BANK_UART0		0x1108
#define SS_BANK_SCGPCTRL	0x1133
#define SS_BANK_EMAC0		0x1510
#define SS_BANK_EMAC1		0x1511

/* PMTOP reg 0: the die's ID, 0xC2 on infinity3 */
#define SS_CHIP_ID		SS_REG(SS_BANK_PMTOP, 0x00)

/* PM sleep reg 0x2E: 0x79 resets the chip */
#define SS_PM_RESET		SS_REG(SS_BANK_PMSLEEP, 0x2E)
#define SS_PM_RESET_KEY		0x79

/* PIU timer 0, 32 bits at the 12 MHz crystal rate */
#define SS_XTAL_HZ		12000000
#define SS_TIMER0_CTRL		SS_REG(SS_BANK_TIMER0, 0x10)
#define SS_TIMER0_MAX_L		SS_REG(SS_BANK_TIMER0, 0x12)
#define SS_TIMER0_MAX_H		SS_REG(SS_BANK_TIMER0, 0x13)
#define SS_TIMER0_CNT_L		SS_REG(SS_BANK_TIMER0, 0x14)
#define SS_TIMER0_CNT_H		SS_REG(SS_BANK_TIMER0, 0x15)
#define SS_TIMER_EN		BIT(0)

/* UART0: an 8250 with its registers 8 bytes apart, at 172.8 MHz */
#define SS_UART0_BASE		SS_BANK(SS_BANK_UART0)
#define SS_UART_HZ		172800000

/* L3 bridge: flushes the MIU write pipe, so DMA sees what the CPU wrote */
#define SS_L3_FLUSH		SS_REG(SS_BANK_L3BRIDGE, 0x05)
#define SS_L3_FLUSH_STATUS	SS_REG(SS_BANK_L3BRIDGE, 0x10)
#define SS_L3_FLUSH_DONE	BIT(12)

/* DMA engines take MIU addresses: DDR from 0 */
#define SS_MIU_ADDR(p)		((unsigned long)(p) - SS_DDR_BASE)

#ifndef __ASSEMBLY__
void ss_miu_flush(void);
#endif

#endif
