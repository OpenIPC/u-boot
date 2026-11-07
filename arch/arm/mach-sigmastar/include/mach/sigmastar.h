/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * SigmaStar (MStar) camera SoCs: infinity3 (MSC313E, MSC316DC, MSC316DM),
 * infinity2m (SSD201, SSD202D), infinity6 (SSC325), infinity6b0 (SSC333,
 * SSC335, SSC337), infinity6e (SSC30K, SSC338Q) and infinity6c (SSC377,
 * SSC378). They share the memory map and most of the register map; what
 * differs is under the die's name below.
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
#define SS_BANK_PMGPIO		0x000F
#define SS_BANK_FSP_PM		0x0016
#define SS_BANK_FSP_NONPM	0x100D	/* infinity6e, past QFN88 */
#define SS_BANK_PMTOP		0x001E
#define SS_BANK_TIMER0		0x0030	/* the PIU timers, timer 0 at reg 0x10 */
/* The internal Ethernet PHY */
#if defined(CONFIG_SS_I6C_FAMILY)
#define SS_BANK_ALBANY0		0x1518
#define SS_BANK_ALBANY1		0x1519
#define SS_BANK_ALBANY2		0x151A
#elif defined(CONFIG_SS_INFINITY6E)
#define SS_BANK_ALBANY0		0x1514
#define SS_BANK_ALBANY1		0x1515
#define SS_BANK_ALBANY2		0x1516
#else
#define SS_BANK_ALBANY0		0x0031
#define SS_BANK_ALBANY1		0x0032
#define SS_BANK_ALBANY2		0x0033
#endif
#define SS_BANK_BDMA		0x1002	/* channel 0 */
#define SS_BANK_CHIPTOP		0x101E
#define SS_BANK_MIU		0x1012
#define SS_BANK_L3BRIDGE	0x1022
#define SS_BANK_CLKGEN0		0x1038
#define SS_BANK_PADTOP		0x103C
#define SS_BANK_UART0		0x1108
#define SS_BANK_SCGPCTRL	0x1133
#define SS_BANK_SCGPCTRL_SD	0x1135	/* Iford: the SD clock's own */
#define SS_BANK_PADGPIO		0x103E	/* infinity6c */
/* The SD host (FCIE5) */
#if defined(CONFIG_SS_INFINITY3) || defined(CONFIG_SS_INFINITY2M)
#define SS_BANK_SDIO0		0x1410
#else
#define SS_BANK_SDIO0		0x1413
#endif
#define SS_BANK_EMAC0		0x1510
#define SS_BANK_EMAC1		0x1511

/* PMTOP reg 0: the die's ID, 0xC2 on infinity3 */
#define SS_CHIP_ID		SS_REG(SS_BANK_PMTOP, 0x00)

/* PM sleep reg 0x2E: 0x79 resets the chip */
#define SS_PM_RESET		SS_REG(SS_BANK_PMSLEEP, 0x2E)
#define SS_PM_RESET_KEY		0x79

/* infinity6e: the bond, which says which FSP the flash is on */
#define SS_CHIP_BOND		SS_REG(SS_BANK_CHIPTOP, 0x48)
#define SS_CHIP_BOND_MASK	0x30
#define SS_CHIP_BOND_QFN88	0x00

/*
 * The DDR's size as the IPL left it, log2 of MiB. infinity6c's IPL writes a
 * fixed 0x6000 to MIU reg 0x69 and the size to bank 0x1659 reg 0x0D.
 */
#ifdef CONFIG_SS_I6C_FAMILY
#define SS_DDR_LOG2_MB		readw(SS_REG(0x1659, 0x0d))
#else
#define SS_DDR_LOG2_MB		(readw(SS_REG(SS_BANK_MIU, 0x69)) >> 12)
#endif

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

/*
 * Ifliegen: the IPL leaves UART0 in the URDMA's DMA mode, its console
 * bytes going through the TX FIFO at reg 0x17; out of DMA mode UART0 is the
 * 8250 again. PM sleep reg 9 bit 11 holds its RX off (the SDK's
 * hal_uart_force_rx_disable() for digmux 0).
 */
#define SS_URDMA_CTRL		SS_REG(0x1107, 0x00)
#define SS_URDMA_DMA_MODE	BIT(1)
#define SS_URDMA_TX_DMA_EN	BIT(2)
#define SS_URDMA_RX_DMA_EN	BIT(3)
#define SS_URDMA_TX_BUSY	BIT(12)
#define SS_URDMA_RX_BUSY	BIT(13)
#define SS_URDMA_TXFIFO_STS	SS_REG(0x1107, 0x1C)
#define SS_URDMA_TXFIFO_EMPTY	BIT(0)
#define SS_UART0_RX_OFF		SS_REG(SS_BANK_PMSLEEP, 0x09)
#define SS_UART0_RX_OFF_BIT	BIT(11)

/* L3 bridge: flushes the MIU write pipe, so DMA sees what the CPU wrote */
#define SS_L3_FLUSH		SS_REG(SS_BANK_L3BRIDGE, 0x05)
#define SS_L3_FLUSH_STATUS	SS_REG(SS_BANK_L3BRIDGE, 0x10)
#define SS_L3_FLUSH_DONE	BIT(12)

/* DMA engines take MIU addresses: DDR from 0 */
#define SS_MIU_ADDR(p)		((unsigned long)(p) - SS_DDR_BASE)

/* ATAGs, where each vendor U-Boot leaves them */
#ifdef CONFIG_SS_INFINITY3
#define SS_BOOT_PARAMS		(SS_DDR_BASE + 0x100)
#else
#define SS_BOOT_PARAMS		SS_DDR_BASE
#endif

#ifndef __ASSEMBLY__
void ss_miu_flush(void);
unsigned int ss_fsp_bank(void);
void ss_set_mem_env(void);
int ms_sdmmc_initialize(void);
#endif

#endif
