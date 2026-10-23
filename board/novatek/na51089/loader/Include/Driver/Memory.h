# ifndef _MEMORY_520_H
# define _MEMORY_520_H

#ifndef __ASSEMBLY__
    COMMENT  /* Memory Map */ /* # define BOOT_ROM_RAM_BASE_ADDR        0x20000000*/
#endif
# define DRAM_BASE_ADDR             (0x00000000)
# define DRAM_EXP_BASE_ADDR         (DRAM_BASE_ADDR + 0x180)
# define DRAM_INT_BASE_ADDR         (DRAM_BASE_ADDR + 0x200)

# define StackSize_KNL              0x1000       /* 4KB */
# define StackSize_INT              0x400        /* 1KB */

# define FAT_HEAP_BUFFER_SIZE       0xA0000//0x1E0000 //0x1E8000
# define EXFAT_BITMAP_BUFFER_SIZE   0x20000  //0x18000
# define LOADER_TMP_BUFFER_SIZE     0x80000

# define BOOT_ROM_SP_LIMIT_ADDR     0xF07F8FFC
# define BOOT_ROM_RAM_TEMP_ADDR     0xF0800200
# define BOOT_ROM_CFG_TEMP_ADDR     0xF0800248
# define BOOT_ROM_RAM_BASE_ADDR     0x00000000
# define DMA2_CTRL_REG_BASE_ADDR    0xF0100000
# define DMA2_PHY_REG_BASE_ADDR     0xF0101000
# define USB_OTG_REG_BASE_ADDR      0//0xF0600000
# define DDR_CTRL_REG_BASE_ADDR     0xF0000000      /* DDR phy base address     */
# define DDR_PHY_REG_BASE_ADDR      0xF0001000
# define DDR_ARB_REG_BASE_ADDR      0xF0008000
# define DMA2_ARB_REG_BASE_ADDR     0xF0108000

# define CLOCK_GEN_REG_BASE_ADDR    0xF0020000
# define PMU_REG_BASE_ADDR          0xF0024000
# define TOP_CTRL_REG_BASE_ADDR     0xF0010000
# define PAD_CTRL_REG_BASE_ADDR     0xF0030000
# define GPIO_CTRL_REG_BASE_ADDR    0xF0070000
# define PMC_REG_BASE               0xF00A0000

# define TRNG_REG_BASE_ADDR         0xF0680000

# define CPU_REG_BASE_ADDR          0xF0FF0000

# define UART0_REG_BASE_ADDR        0xF0290000
// Real board is UART3
# define UART2_REG_BASE_ADDR        0xF0310000
# define SM_HOST_REG_BASE_ADDR      0xF0400000
# define MS_HOST_REG_BASE_ADDR      0xF0410000
# define SD_HOST1_REG_BASE_ADDR     0xF0420000
# define SD_HOST2_REG_BASE_ADDR     0xF0500000
# define SD_HOST3_REG_BASE_ADDR     0xF0510000
# define SPI_CTRL_REG_BASE_ADDR     0xF0230000
# define ETH_CTRL_REG_BASE_ADDR     0xF02B0000
# define EFUSE_CTRL_REG_BASE_ADDR   0xF0660000
# define IDE_SRAM_BASE_ADDR         0xF07C0000
# define IDE_CTRL_REG_BASE_ADDR     0xF0800000
# define TIMER_REG_BASE_ADDR        0xF0040000
# define WDT_REG_BASE_ADDR          0xF0050000

# define BOOT_LOADER_TMP_BUFFER     0x00001000
# define CHIP_REMAP_REG_OFFSET      0x00000088
# define CPE_RTC_BASE               0xF0060000      /* RTC controller           */
# define CPE_GPIO_BASE              0xF0070000      /* GPIO controller          */
# define INTERRUPT_BASE_ADDR        0xF0080000      /* Interrupt controller     */
# define CC_BASE_ADDR               0xF0090000      /* CC controller     */
# define IOADDR_GIC_REG_BASE        0xFFD00000      /* GIC Interrupt     */
# define configARM_TIMER_BASEADDR   0xFFD00200
# define ARM_TIMER_LOAD_OFFSET      0x00       		/**< Timer Load Register */


# define BOOT_ROM_RAM_TEMP_ADDR2    0xF07E8000   	//Eth
# define PAD_REG_BASE				0xF0030000
# define L2_MEM_BASE				0xFFE00000
# define IOADDR_HVYLOAD_REG_BASE    0xF0008000

# define CC_CPU2_CPU1_CMDBUF_REG1 	0xF07F8000						//CC_CPU2_CPU1_CMDBUF_REG1 to store uboot starting address
# define CC_CPU2_CPU1_CMDBUF_REG2 	0xF07F8004 						//CC_CPU2_CPU1_CMDBUF_REG1 has used in core2_entry.s
# define NVT_CORE2_START 			CC_CPU2_CPU1_CMDBUF_REG2		//core2_entry.S 's entry point

# define ARM_TIMER_LOAD_OFFSET      		0x00       		/**< Timer Load Register */
# define ARM_TIMER_LOAD_OFFSET              0x00       /**< Timer Load Register */
# define ARM_TIMER_COUNTER_OFFSET           0x04       /**< Timer Counter Register */
# define ARM_TIMER_CONTROL_OFFSET           0x08       /**< Timer Control Register */
# define ARM_TIMER_ISR_OFFSET               0x0C       /**< Timer Interrupt Status Register */
# define ARM_TIMER_COUNTER_OFFSET_L         0x00       /**< Timer Counter Register */
# define ARM_TIMER_COUNTER_OFFSET_H         0x04       /**< Timer Counter Register */
# define ARM_TIMER_COUNTER_CMP_OFFSET_L     0x10       /**< Timer Counter Register */
# define ARM_TIMER_COUNTER_CMP_OFFSET_H     0x14       /**< Timer Counter Register */
# define CYGHWR_HAL_RTC_PRESCALER			1
# define ARM_TIMER_CONTROL_AUTO_RELOAD_MASK 0x00000002 /**< Auto-reload */
# define ARM_TIMER_CONTROL_PRESCALER_MASK   0x0000FF00 /**< Prescaler */
# define ARM_TIMER_CONTROL_PRESCALER_SHIFT  8
# define ARM_TIMER_CONTROL_IRQ_ENABLE_MASK  0x00000004 /**< Intr enable */
# define ARM_TIMER_CONTROL_AUTO_RELOAD_MASK 0x00000002 /**< Auto-reload */
# define ARM_TIMER_CONTROL_ENABLE_MASK      0x00000001 /**< Timer enable */

#define ARM_TIMER_ISR_EVENT_FLAG_MASK       0x00000001 /**< Event flag */
#define CRYPTO_REG_BASE_ADDR				0xF0620000
#define HASH_REG_BASE_ADDR         			0xF0670000
#define RSA_REG_BASE_ADDR          			0xF06A0000

#define CRYPTO_CONFIG_REG        			(CRYPTO_REG_BASE_ADDR + 0x00)
#define CRYPTO_CONTROL_REG       			(CRYPTO_REG_BASE_ADDR + 0x04)
#define CRYPTO_OUT_DATA_REG      			(CRYPTO_REG_BASE_ADDR + 0x08)
#define CRYPTO_STATUS_REG        			(CRYPTO_REG_BASE_ADDR + 0x0C)

#define CRYPTO_PIO_INPUT0_REG    			(CRYPTO_REG_BASE_ADDR + 0x30)
#define CRYPTO_PIO_INPUT1_REG    			(CRYPTO_REG_BASE_ADDR + 0x34)
#define CRYPTO_PIO_INPUT2_REG    			(CRYPTO_REG_BASE_ADDR + 0x38)
#define CRYPTO_PIO_INPUT3_REG    			(CRYPTO_REG_BASE_ADDR + 0x3C)
#define CRYPTO_PIO_OUTPUT0_REG   			(CRYPTO_REG_BASE_ADDR + 0x40)
#define CRYPTO_PIO_OUTPUT1_REG   			(CRYPTO_REG_BASE_ADDR + 0x44)
#define CRYPTO_PIO_OUTPUT2_REG   			(CRYPTO_REG_BASE_ADDR + 0x48)
#define CRYPTO_PIO_OUTPUT3_REG   			(CRYPTO_REG_BASE_ADDR + 0x4C)

// Hash
#define HASH_CONFIG_REG         			(HASH_REG_BASE_ADDR + 0x00)
#define HASH_STATUS_REG         			(HASH_REG_BASE_ADDR + 0x0C)
#define HASH_OUTPUT_DATA1_REG   			(HASH_REG_BASE_ADDR + 0x70)
#define HASH_INPUT_DATA_REG     			(HASH_REG_BASE_ADDR + 0x90)

//RSA
#define RSA_CONFIG_REG              	 	(RSA_REG_BASE_ADDR + 0x00)
#define RSA_CONTROL_REG              		(RSA_REG_BASE_ADDR + 0x04)
#define RSA_STATUS_REG               		(RSA_REG_BASE_ADDR + 0x0C)
#define RSA_KEY_N_REG                		(RSA_REG_BASE_ADDR + 0x10)
#define RSA_KEY_N_ADDR_REG           		(RSA_REG_BASE_ADDR + 0x14)
#define RSA_KEY_ED_REG               		(RSA_REG_BASE_ADDR + 0x18)
#define RSA_KEY_ED_ADDR_REG          		(RSA_REG_BASE_ADDR + 0x1C)
#define RSA_DATA_REG                 		(RSA_REG_BASE_ADDR + 0x20)
#define RSA_DATA_ADDR_REG            		(RSA_REG_BASE_ADDR + 0x24)
#define RSA_CRC32_DEFAULT_REG_OFS    		(RSA_REG_BASE_ADDR + 0x30)
#define RSA_CRC32_POLY_REG_OFS       		(RSA_REG_BASE_ADDR + 0x34)
#define RSA_CRC32_OUTPUT_REG_OFS     		(RSA_REG_BASE_ADDR + 0x38)

// Crypto
#define CRYPTO_CONFREG_SWRST          	(1<<0)
#define CRYPTO_CONFREG_CRYPTO_EN      	(1<<1)
#define CRYPTO_CONFREG_AES128         	(2<<4)
#define CRYPTO_CONFREG_DECRYPT        	(1<<8)

#define CRYPTO_PIO_DONE               	(1<<0)

// Hash
#define HASH_CONFREG_INITSTATE        	(1<<20)
#define HASH_CONFREG_SHA256           	(1<<4)
#define HASH_CONFREG_ENABLE           	(1<<1)

//RSA
#define RSA_ENABLE                    	(1<<0)

#define RSA_KEYWIDTH_256              	(0x0<<1)
#define RSA_KEYWIDTH_512              	(0x1<<1)
#define RSA_KEYWIDTH_1024             	(0x2<<1)
#define RSA_KEYWIDTH_2048             	(0x3<<1)
#define RSA_KEYWIDTH_4096             	(0x4<<1)

#define RSA_NORMAL_MODE               	(0x0<<4)
#define RSA_MODE_CRC_KEY_N           	(0x1<<4)
#define RSA_MODE_CRC_KEY_ED           	(0x2<<4)


#define RSA_TRANSFER_END              	(0x1<<0)
#define RSA_BUSY                      	(0x1<<1)

#define HASH_TRANSFER_END             	(0x1<<0)



#define TOP_CTRL_SRAM_RESET_REG0    	(TOP_CTRL_REG_BASE_ADDR + 0x1000)

//Shut down SRAM SDIO1 mask
#define TOP_SRAM_SD_SDIO1           	0x20000000
//Shut down SRAM RSA mask
#define TOP_SRAM_SD_RSA             	0x02000000


#define reg0_cache_type_ofs			0x004			//reg0_cache_type
#define CACHE_CTRL_REG_OFS			0x100
#define CACHE_AUX_CTRL_REG_OFS		0x104

#define CACHE_TAG_RAM_CTRL_OFS		0x108
#define CACHE_DATA_RAM_CTRL_OFS		0x10C


#define reg2_int_clear_ofs			0x220			//reg2_int_clear
#define reg7_cache_sync				0x730			//reg7_cache_sync
#define reg7_inv_way				0x77C			//reg7_inv_way

#define CACHE_PREFETCH_REG_OFS		0xF60

#define L2_REG0_BASE	            (L2_MEM_BASE + 0x000)	/* Cache ID and Cache Type */
#define L2_REG1_BASE	            (L2_MEM_BASE + 0x100)	/* Control */
#define L2_REG2_BASE	            (L2_MEM_BASE + 0X200)	/* Interrupt and Counter Control Registers */
#define L2_REG7_BASE	            (L2_MEM_BASE + 0x700)	/* Cache Maintenance Operations */
#define L2_REG9_BASE	            (L2_MEM_BASE + 0x900)	/* Cache Lockdown */
#define L2_REG12_BASE	            (L2_MEM_BASE + 0xC00)	/* Address Filtering */
#define L2_REG15_BASE	            (L2_MEM_BASE + 0xF00)	/* Debug, Prefetch and Power */

/**
 * Cache ID and Cache Type
 */
#define L2_REG0_CACHE_ID				(*((volatile unsigned long *)(L2_REG0_BASE + 0x00)))
#define L2_REG0_CACHE_TYPE				(*((volatile unsigned long *)(L2_REG0_BASE + 0x04)))

#define S_L2_REG0_CACHE_TYPE_DB			(31)
#define M_L2_REG0_CACHE_TYPE_DB			(0xf << S_L2_REG0_CACHE_TYPE_DB)
#define S_L2_REG0_CACHE_TYPE_CTYPE		(25)
#define M_L2_REG0_CACHE_TYPE_CTYPE		(0xf << S_L2_REG0_CACHE_TYPE_CTYPE)
#define S_L2_REG0_CACHE_TYPE_H			(24)
#define M_L2_REG0_CACHE_TYPE_H			(0x1 << S_L2_REG0_CACHE_TYPE_H)
#define S_L2_REG0_CACHE_TYPE_DWS		(20)
#define M_L2_REG0_CACHE_TYPE_DWS		(0x7 << S_L2_REG0_CACHE_TYPE_DWS)
#define S_L2_REG0_CACHE_TYPE_DA			(18)
#define M_L2_REG0_CACHE_TYPE_DA			(0x1 << S_L2_REG0_CACHE_TYPE_DA)
#define S_L2_REG0_CACHE_TYPE_DLS		(12)
#define M_L2_REG0_CACHE_TYPE_DLS		(0x3 << S_L2_REG0_CACHE_TYPE_DLS)
#define S_L2_REG0_CACHE_TYPE_IWS		(8)
#define M_L2_REG0_CACHE_TYPE_IWS		(0x7 << S_L2_REG0_CACHE_TYPE_IWS)
#define S_L2_REG0_CACHE_TYPE_IA			(6)
#define M_L2_REG0_CACHE_TYPE_IA			(0x1 << S_L2_REG0_CACHE_TYPE_IA)
#define S_L2_REG0_CACHE_TYPE_ILS		(0)
#define M_L2_REG0_CACHE_TYPE_ILS		(0x3 << S_L2_REG0_CACHE_TYPE_ILS)

#define K_L2_REG0_CACHE_TYPE_DA_16WAY	(1)
#define K_L2_REG0_CACHE_TYPE_DA_8WAY	(0)

/**
 * Control
 */
#define L2_REG1_CONTROL					(*((volatile unsigned long *)(L2_REG1_BASE + 0x00)))
#define L2_REG1_AUX_CTRL				(*((volatile unsigned long *)(L2_REG1_BASE + 0x04)))
#define L2_REG1_TAG_RAM_CTRL			(*((volatile unsigned long *)(L2_REG1_BASE + 0x08)))
#define L2_REG1_DATA_RAM_CTRL			(*((volatile unsigned long *)(L2_REG1_BASE + 0x0C)))

#define S_L2_REG1_CONTROL_EN	(0)
#define M_L2_REG1_CONTROL_EN	(0x1 << S_L2_REG1_CONTROL_EN)

#define K_L2_REG1_CONTROL_EN_ON		1
#define K_L2_REG1_CONTROL_EN_OFF	0

#define S_L2_REG1_AUX_CTRL_BRESP						(30)
#define M_L2_REG1_AUX_CTRL_BRESP						(0x1 << S_L2_REG1_AUX_CTRL_BRESP)
#define S_L2_REG1_AUX_CTRL_INSTR_PREF					(29)
#define M_L2_REG1_AUX_CTRL_INSTR_PREF					(0x1 << S_L2_REG1_AUX_CTRL_INSTR_PREF)
#define S_L2_REG1_AUX_CTRL_DATA_PERF					(28)
#define M_L2_REG1_AUX_CTRL_DATA_PERF					(0x1 << S_L2_REG1_AUX_CTRL_DATA_PERF)
#define S_L2_REG1_AUX_CTRL_NS_INT_CTRL					(27)
#define M_L2_REG1_AUX_CTRL_NS_INT_CTRL					(0x1 << S_L2_REG1_AUX_CTRL_NS_INT_CTRL)
#define S_L2_REG1_AUX_CTRL_NS_LOCK_EN					(26)
#define M_L2_REG1_AUX_CTRL_NS_LOCK_EN					(0x1 << S_L2_REG1_AUX_CTRL_NS_LOCK_EN)
#define S_L2_REG1_AUX_CTRL_CACHE_POLICY					(25)
#define M_L2_REG1_AUX_CTRL_CACHE_POLICY					(0x1 << S_L2_REG1_AUX_CTRL_CACHE_POLICY)
#define S_L2_REG1_AUX_CTRL_FORCE_WA						(23)
#define M_L2_REG1_AUX_CTRL_FORCE_WA						(0x3 << S_L2_REG1_AUX_CTRL_FORCE_WA)
#define S_L2_REG1_AUX_CTRL_SHARED_OVERRIDE_EN			(22)
#define M_L2_REG1_AUX_CTRL_SHARED_OVERRIDE_EN			(0x1 << S_L2_REG1_AUX_CTRL_SHARED_OVERRIDE_EN)
#define S_L2_REG1_AUX_CTRL_PARITY_EN					(21)
#define M_L2_REG1_AUX_CTRL_PARITY_EN					(0x1 << S_L2_REG1_AUX_CTRL_PARITY_EN)
#define S_L2_REG1_AUX_CTRL_EVENT_MON_BUD_EN				(20)
#define M_L2_REG1_AUX_CTRL_EVENT_MON_BUD_EN				(0x1 << S_L2_REG1_AUX_CTRL_EVENT_MON_BUD_EN)
#define S_L2_REG1_AUX_CTRL_WAT_SIZE						(17)
#define M_L2_REG1_AUX_CTRL_WAT_SIZE						(0x7 << S_L2_REG1_AUX_CTRL_WAT_SIZE)
#define S_L2_REG1_AUX_CTRL_ASSOCIATIVITY				(16)
#define M_L2_REG1_AUX_CTRL_ASSOCIATIVITY				(0x1 << S_L2_REG1_AUX_CTRL_ASSOCIATIVITY)
#define S_L2_REG1_AUX_CTRL_SHARED_INV_EN				(13)
#define M_L2_REG1_AUX_CTRL_SHARED_INV_EN				(0x1 << S_L2_REG1_AUX_CTRL_SHARED_INV_EN)
#define S_L2_REG1_AUX_CTRL_EXCLUSIVE_CACHE_CONF			(12)
#define M_L2_REG1_AUX_CTRL_EXCLUSIVE_CACHE_CONF			(0x1 << S_L2_REG1_AUX_CTRL_EXCLUSIVE_CACHE_CONF)
#define S_L2_REG1_AUX_CTRL_STORE_BUD_DEV_LIMIT_EN		(11)
#define M_L2_REG1_AUX_CTRL_STORE_BUD_DEV_LIMIT_EN		(0x1 << S_L2_REG1_AUX_CTRL_STORE_BUD_DEV_LIMIT_EN)
#define S_L2_REG1_AUX_CTRL_HIGH_PRIO_SO_DEV_READS_EN	(10)
#define M_L2_REG1_AUX_CTRL_HIGH_PRIO_SO_DEV_READS_EN	(0x1 << S_L2_REG1_AUX_CTRL_HIGH_PRIO_SO_DEV_READS_EN)
#define S_L2_REG1_AUX_CTRL_FULL_LINE_Z_EN				(0)
#define M_L2_REG1_AUX_CTRL_FULL_LINE_Z_EN				(0x1 << S_L2_REG1_AUX_CTRL_FULL_LINE_Z_EN)

/**
 * Interrupt and Counter Control Registers
 */
#define L2_REG2_EV_CNT_CTRL				(*((volatile unsigned long *)(L2_REG2_BASE + 0x00)))
#define L2_REG2_EV_CNT1_CFG				(*((volatile unsigned long *)(L2_REG2_BASE + 0x04)))
#define L2_REG2_EV_CNT0_CFG				(*((volatile unsigned long *)(L2_REG2_BASE + 0x08)))
#define L2_REG2_EV_CNT1					(*((volatile unsigned long *)(L2_REG2_BASE + 0x0C)))
#define L2_REG2_EV_CNT0					(*((volatile unsigned long *)(L2_REG2_BASE + 0x10)))
#define L2_REG2_INT_MASK				(*((volatile unsigned long *)(L2_REG2_BASE + 0x14)))
#define L2_REG2_INT_MASK_STATUS			(*((volatile unsigned long *)(L2_REG2_BASE + 0x18)))
#define L2_REG2_INT_RAW_STATUS			(*((volatile unsigned long *)(L2_REG2_BASE + 0x1C)))
#define L2_REG2_INT_CLEAR				(*((volatile unsigned long *)(L2_REG2_BASE + 0x20)))

#define S_L2_REG2_INT_DECERR	(8)
#define M_L2_REG2_INT_DECERR	(0x1 << S_L2_REG2_INT_DECERR)	/* Decode error */
#define S_L2_REG2_INT_SLVERR	(7)
#define M_L2_REG2_INT_SLVERR	(0x1 << S_L2_REG2_INT_SLVERR)	/* Slave error */
#define S_L2_REG2_INT_ERRRD		(6)
#define M_L2_REG2_INT_ERRRD		(0x1 << S_L2_REG2_INT_ERRRD)	/* Data RAM read error */
#define S_L2_REG2_INT_ERRRT		(5)
#define M_L2_REG2_INT_ERRRT		(0x1 << S_L2_REG2_INT_ERRRT)	/* Tag RAM read error */
#define S_L2_REG2_INT_ERRWD		(4)
#define M_L2_REG2_INT_ERRWD		(0x1 << S_L2_REG2_INT_ERRWD)	/* Data RAM write error */
#define S_L2_REG2_INT_ERRWT		(3)
#define M_L2_REG2_INT_ERRWT		(0x1 << S_L2_REG2_INT_ERRWT)	/* Tag RAM write error */
#define S_L2_REG2_INT_PARRD		(2)
#define M_L2_REG2_INT_PARRD		(0x1 << S_L2_REG2_INT_PARRD)	/* Parity error on data RAM read */
#define S_L2_REG2_INT_PARRT		(1)
#define M_L2_REG2_INT_PARRT		(0x1 << S_L2_REG2_INT_PARRT)	/* Pariry error on tag RAM read */
#define S_L2_REG2_INT_ECNTR		(0)
#define M_L2_REG2_INT_ECNTR		(0x1 << S_L2_REG2_INT_ECNTR)	/* Event counter1/0 overflow increment */

/**
 * Cache Maintenance Operations
 */
#define L2_REG7_CACHE_SYNC				(*((volatile unsigned long *)(L2_REG7_BASE + 0x30)))
#define L2_REG7_INV_PA					(*((volatile unsigned long *)(L2_REG7_BASE + 0x70)))
#define L2_REG7_INV_WAY					(*((volatile unsigned long *)(L2_REG7_BASE + 0x7C)))
#define L2_REG7_CLEAN_PA				(*((volatile unsigned long *)(L2_REG7_BASE + 0xB0)))
#define L2_REG7_CLEAN_INDEX				(*((volatile unsigned long *)(L2_REG7_BASE + 0xB8)))
#define L2_REG7_CLEAN_WAY				(*((volatile unsigned long *)(L2_REG7_BASE + 0xBC)))
#define L2_REG7_CLEAN_INV_PA			(*((volatile unsigned long *)(L2_REG7_BASE + 0xF0)))
#define L2_REG7_CLEAN_INV_INDEX			(*((volatile unsigned long *)(L2_REG7_BASE + 0xF8)))
#define L2_REG7_CLEAN_INV_WAY			(*((volatile unsigned long *)(L2_REG7_BASE + 0xFC)))


#define K_L2_REG7_CACHE_SYNC_C			(0x1)

#define K_L2_REG7_INV_WAY_8WAY			(0x00ff)
#define K_L2_REG7_INV_WAY_16WAY			(0xffff)
#define K_L2_REG7_CLEAN_WAY_8WAY        (0x00ff)
#define K_L2_REG7_CLEAN_WAY_16WAY       (0xffff)


/**
 * Cache Lockdown
 */
#define L2_REG9_D_LOCKDOWN0				(*((volatile unsigned long *)(L2_REG9_BASE + 0x00)))
#define L2_REG9_I_LOCKDOWN0				(*((volatile unsigned long *)(L2_REG9_BASE + 0x04)))
#define L2_REG9_D_LOCKDOWN1				(*((volatile unsigned long *)(L2_REG9_BASE + 0x08)))
#define L2_REG9_I_LOCKDOWN1				(*((volatile unsigned long *)(L2_REG9_BASE + 0x0C)))
#define L2_REG9_D_LOCKDOWN2				(*((volatile unsigned long *)(L2_REG9_BASE + 0x10)))
#define L2_REG9_I_LOCKDOWN2				(*((volatile unsigned long *)(L2_REG9_BASE + 0x14)))
#define L2_REG9_D_LOCKDOWN3				(*((volatile unsigned long *)(L2_REG9_BASE + 0x18)))
#define L2_REG9_I_LOCKDOWN3				(*((volatile unsigned long *)(L2_REG9_BASE + 0x1C)))
#define L2_REG9_D_LOCKDOWN4				(*((volatile unsigned long *)(L2_REG9_BASE + 0x20)))
#define L2_REG9_I_LOCKDOWN4				(*((volatile unsigned long *)(L2_REG9_BASE + 0x24)))
#define L2_REG9_D_LOCKDOWN5				(*((volatile unsigned long *)(L2_REG9_BASE + 0x28)))
#define L2_REG9_I_LOCKDOWN5				(*((volatile unsigned long *)(L2_REG9_BASE + 0x2C)))
#define L2_REG9_D_LOCKDOWN6				(*((volatile unsigned long *)(L2_REG9_BASE + 0x30)))
#define L2_REG9_I_LOCKDOWN6				(*((volatile unsigned long *)(L2_REG9_BASE + 0x34)))
#define L2_REG9_D_LOCKDOWN7				(*((volatile unsigned long *)(L2_REG9_BASE + 0x38)))
#define L2_REG9_I_LOCKDOWN7				(*((volatile unsigned long *)(L2_REG9_BASE + 0x3C)))
#define L2_REG9_LOCK_LINE_EN			(*((volatile unsigned long *)(L2_REG9_BASE + 0x50)))
#define L2_REG9_UNLOCK_WAY				(*((volatile unsigned long *)(L2_REG9_BASE + 0x54)))

/**
 * Address Filtering
 */
#define L2_REG12_ADDR_FILTERING_START	(*((volatile unsigned long *)(L2_REG12_BASE + 0x00)))
#define L2_REG12_ADDR_FILTERING_END		(*((volatile unsigned long *)(L2_REG12_BASE + 0x04)))

/**
 * Debug, Prefetch and Power
 */
#define L2_REG15_DEBUG_CTRL				(*((volatile unsigned long *)(L2_REG15_BASE + 0x40)))
#define L2_REG15_PREF_CTRL				(*((volatile unsigned long *)(L2_REG15_BASE + 0x60)))
#define L2_REG15_POWER_CTRL				(*((volatile unsigned long *)(L2_REG15_BASE + 0x80)))

#define S_L2_REG15_DEBUG_CTRL_SPNIDEN	(2)
#define M_L2_REG15_DEBUG_CTRL_SPNIDEN	(0x1 << S_L2_REG15_DEBUG_CTRL_SPNIDEN)
#define S_L2_REG15_DEBUG_CTRL_DWB		(1)
#define M_L2_REG15_DEBUG_CTRL_DWB		(0x1 << S_L2_REG15_DEBUG_CTRL_DWB)
#define S_L2_REG15_DEBUG_CTRL_DCL		(0)
#define M_L2_REG15_DEBUG_CTRL_DCL		(0x1 << S_L2_REG15_DEBUG_CTRL_DCL)

#define S_L2_REG15_PREF_CTRL_DL_FILL_EN					(30)
#define M_L2_REG15_PREF_CTRL_DL_FILL_EN					(0x1 << S_L2_REG15_PREF_CTRL_DL_FILL_EN)
#define S_L2_REG15_PREF_CTRL_INST_PREF_EN				(29)
#define M_L2_REG15_PREF_CTRL_INST_PREF_EN				(0x1 << S_L2_REG15_PREF_CTRL_INST_PREF_EN)
#define S_L2_REG15_PREF_CTRL_DATA_PREF_EN				(28)
#define M_L2_REG15_PREF_CTRL_DATA_PREF_EN				(0x1 << S_L2_REG15_PREF_CTRL_DATA_PREF_EN)
#define S_L2_REG15_PREF_CTRL_DL_WRAP_READ_DIS			(27)
#define M_L2_REG15_PREF_CTRL_DL_WRAP_READ_DIS			(0x1 << S_L2_REG15_PREF_CTRL_DL_WRAP_READ_DIS)
#define S_L2_REG15_PREF_CTRL_PREF_DROP_EN				(24)
#define M_L2_REG15_PREF_CTRL_PREF_DROP_EN				(0x1 << S_L2_REG15_PREF_CTRL_PREF_DROP_EN)
#define S_L2_REG15_PREF_CTRL_INCR_DL_FILL_EN			(23)
#define M_L2_REG15_PREF_CTRL_INCR_DL_FILL_EN			(0x1 << S_L2_REG15_PREF_CTRL_INCR_DL_FILL_EN)
#define S_L2_REG15_PREF_CTRL_NOT_SAME_ID_EXCLU_SEQ_EN	(21)
#define M_L2_REG15_PREF_CTRL_NOT_SAME_ID_EXCLU_SEQ_EN	(0x1 << S_L2_REG15_PREF_CTRL_NOT_SAME_ID_EXCLU_SEQ_EN)
#define S_L2_REG15_PREF_CTRL_PREF_OFF					(0)
#define M_L2_REG15_PREF_CTRL_PREF_OFF					(0xf << S_L2_REG15_PREF_CTRL_PREF_OFF)

// DDR phy duty configuration register bit
#define DDR_PHY_INCREASE_DUTY       0x40            // P < N
#define DDR_PHY_DECREASE_DUTY       0x00            // P > N
#define DDR_PHY_DUTY_TYPE_MSK       0x40            // bit mask for increase/decrease duty
#define DDR_PHY_CLK_ADJ_EN          0x80

// DDR phy duty calibration control register bit
#define DDR_PHY_CLK_OP_ONGOING      0x10
#define DDR_PHY_CLK_OP_DONE         0x00
#define DDR_PHY_DQS_OP_ONGOING      0x20
#define DDR_PHY_DQS_OP_DONE         0x00


# define DDR_PHY_CAL_CLK_ADJ_OFS    0x000           /* 0x00                     */
# define DDR_PHY_CAL_CMD_ADJ_OFS    0x01C           /* 0x1C                     */
# define DDR_PHY_CAL_CTRL_OFS       0x150           /* 0x54                     */
# define DDR_PHY_CAL_CLK_CFG_OFS    0x154           /* 0x55                     */
# define DDR_PHY_CAL_DQS_CFG_OFS    0x158           /* 0x56                     */
# define DDR_PHY_CAL_DTY_CNT_LB_OFS 0x15C           /* 0x57 duty cnt low byte   */
# define DDR_PHY_CAL_DTY_CNT_HB_OFS 0x160           /* 0x58 duty cnt high byte  */
# define DDR_PHY_CAL_DQS0_ADJ_OFS   0x1D0           /* 0x1D0                    */
# define DDR_PHY_CAL_DQ_ADJ_OFS     0x1D4           /* 0x1D0                    */


#define BOOT_SOURCE_SPI                0x00
#define BOOT_SOURCE_CARD               0x01
#define BOOT_SOURCE_SPI_NAND_2K        0x02
#define BOOT_SOURCE_SPI_NAND_RS_2K     0x03
#define BOOT_SOURCE_ETHERNET           0x04
#define BOOT_SOURCE_USB                0x05
#define BOOT_SOURCE_SPI_NAND_4K        0x06
#define BOOT_SOURCE_BMC                0x07
#define BOOT_SOURCE_EMMC_4BIT          0x08
#define BOOT_SOURCE_EMMC_8BIT          0x09
#define BOOT_SOURCE_SPI_NAND_RS_4K     0x0A
#define BOOT_SOURCE_USB_FULL           0x0B

#define BOOT_SOURCE_MSK                0x0F


# define IDENTIFY_ERR       2
# define MBR_ERR            3
# define PBR_ERR            4
# define READ512_ERR        5
# define READ_ERR           7
# define HPA_ERR            8
# define CHECKSUM_ERR       9
#ifndef __ASSEMBLY__
    END
#endif
# endif
