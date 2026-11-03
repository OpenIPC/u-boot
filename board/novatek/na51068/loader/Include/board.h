#ifndef __BOARD_H__
#define __BOARD_H__

//#define NULL    0
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

#define IMAGE_ADDR          		0x100000
#if (defined(CONFIG_MT) || defined(CONFIG_RA))
#define LOAD_ADDR           		0x0F000040
#define LOAD_EP_ADDR        		0x1F000000
#else
#define LOAD_ADDR          			0xB00000//0xF9001000
#endif
#define LOAD_FDT_ADDR       		0x01F00000
#define SYS_CLK             		12000000

#define SRAM0_BASE              	0xf9001640
#define SRAM0_SIZE              	0x6000  /* 24K */
#define SRAM1_BASE              	SRAM0_BASE+SRAM0_SIZE//(0xf9001200 + 0x6000)
#define SRAM1_SIZE              	0x2000

//#define AXIC_REMAP              0x130

#define REG_HEAVY_LOAD_BASE     	0xfd600000
#define REG_H2X_BASE          		0xfe600000
#define REG_AXIC_BASE          		0xfe700000

#define REG_UART0_BASE          	0xfe200000
#define REG_UART1_BASE          	0xfe220000
#define REG_UART2_BASE          	0xfe240000
#define REG_TOP_BASE            	0xfe030000
#define REG_PAD_BASE				0xfe040000
#define REG_PMU_BASE            	0xfe000000
#define REG_DMA_BASE            	0xfca00000	/* dmac030 */

#define REG_DDR0_BASE           	0xfc400000
#define REG_DDR1_BASE           	0xfc500000
//#define REG_DDR2_BASE           	0xfc600000
//#define REG_DDR3_BASE           	0xfc700000

//#define REG_GPIO0_BASE          	0xfe400000
//#define REG_GPIO1_BASE          	0xfe420000
#define REG_GPIO2_BASE          	0xfe440000

#define REG_WDT_BASE            	0xfe8a0000
#define REG_GIC_BASE         		0xf9c00000
//#define REG_SPI020_BASE         	0xfa700000
#define BASE_SPI        			0xFA900000
//#define REG_DMAC020_BASE        	0xFA500000
#define REG_DMAC030_BASE        	REG_DMA_BASE
#define REG_TMR010_BASE         	0xFE8C0000
#define REG_SDHCI_BASE         	 	0xFA600000
#define AXI_PCIE_IC10				0xFE840000  /* S1 for pcie0, S2 for pcie1 */
#define REG_CRYPTO_BASE         	0xFEC60000
#define REG_HASH_BASE           	0xFEC80000
#define REG_RSA_BASE            	0xFECA0000
#define INTC_OSPREY_BASE        	0xF8401000
#define MP0_BASE                	0xFDC00000
#define REG_PWM_BASE            	0xFE6C0000
#define configARM_TIMER_BASEADDR	0xF9C00200
#define ARM_TIMER_LOAD_OFFSET                   0x00       /**< Timer Load Register */
#define ARM_TIMER_COUNTER_OFFSET                0x04       /**< Timer Counter Register */
#define ARM_TIMER_CONTROL_OFFSET                0x08       /**< Timer Control Register */
#define ARM_TIMER_ISR_OFFSET                    0x0C       /**< Timer Interrupt Status Register */
#define ARM_TIMER_COUNTER_OFFSET_L              0x00       /**< Timer Counter Register */
#define ARM_TIMER_COUNTER_OFFSET_H              0x04       /**< Timer Counter Register */
#define ARM_TIMER_COUNTER_CMP_OFFSET_L          0x10       /**< Timer Counter Register */
#define ARM_TIMER_COUNTER_CMP_OFFSET_H          0x14       /**< Timer Counter Register */

#define CYGHWR_HAL_RTC_PRESCALER				1
#define ARM_TIMER_CONTROL_PRESCALER_MASK        0x0000FF00 /**< Prescaler */
#define ARM_TIMER_CONTROL_PRESCALER_SHIFT       8
#define ARM_TIMER_CONTROL_IRQ_ENABLE_MASK       0x00000004 /**< Intr enable */
#define ARM_TIMER_CONTROL_AUTO_RELOAD_MASK      0x00000002 /**< Auto-reload */
#define ARM_TIMER_CONTROL_ENABLE_MASK           0x00000001 /**< Timer enable */

#define ARM_TIMER_ISR_EVENT_FLAG_MASK           0x00000001 /**< Event flag */


#define PAD_PUPD_REG0					    	(REG_PAD_BASE + 0x00)
#define PAD_PUPD_REG1				    		(REG_PAD_BASE + 0x04)
#define PAD_PUPD_REG8				    		(REG_PAD_BASE + 0x40)


#ifndef __ASSEMBLY__

#include "constant.h"

typedef enum
{
    FLASH_NOR = 0,
    FLASH_SPI_NAND,
    FLASH_SD
} FLASH_TYPE_T;

unsigned int read_reg(unsigned int reg);
void write_reg(unsigned int reg, unsigned int value);
void sal_memset(void *pbuf, unsigned char val, int len);
int sal_memcmp(void *sbuf, void *dbuf, int len);
//int sal_memcpy(void *sbuf, void *dbuf, int len);
void *memcpy(void *dst, const void *src, unsigned int n);


#ifndef OUTW
#define OUTW(addr,value)    (*(UINT32 volatile *)(addr) = (UINT32)(value))
#endif

#ifndef INW
#define INW(addr)           (*(UINT32 volatile *)(addr))
#endif
#define _ALIGNED(x) __attribute__((aligned(x)))

#endif /* __ASSEMBLY__ */


#endif
