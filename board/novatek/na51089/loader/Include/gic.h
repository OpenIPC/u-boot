
#ifndef _ASM_ARMV8_GIC_H_
#define _ASM_ARMV8_GIC_H_
//#include "FLibARM.h"
#define UART_INTID     43



#define GIC_UART_INTID (UART_INTID + 32)
#if 0
/*
 * Distributor layout
 */
#define GICD_CTLR      0x0000
#define GICD_TYPER     0x0004
#define GICD_IIDR      0x0008
#define GICD_IGROUP    0x0080
#define GICD_ISENABLE  0x0100
#define GICD_ICENABLE  0x0180
#define GICD_ISPEND    0x0200
#define GICD_ICPEND    0x0280
#define GICD_ISACTIVE  0x0300
#define GICD_ICACTIVE  0x0380
#define GICD_IPRIORITY 0x0400
#define GICD_ITARGETS  0x0800
#define GICD_ICFG      0x0c00
#define GICD_PPISR     0x0d00
#define GICD_SPISR     0x0d04
#define GICD_SGIR      0x0f00
#define GICD_CPENDSGI  0x0f10
#define GICD_SPENDSGI  0x0f20
#define GICD_PIDR4     0x0fd0
#define GICD_PIDR5     0x0fd4
#define GICD_PIDR6     0x0fd8
#define GICD_PIDR7     0x0fdc
#define GICD_PIDR0     0x0fe0
#define GICD_PIDR1     0x0fe4
#define GICD_PIDR2     0x0fe8
#define GICD_PIDR3     0x0fec
#define GICD_CIDR0     0x0ff0
#define GICD_CIDR1     0x0ff4
#define GICD_CIDR2     0x0ff8
#define GICD_CIDR3     0x0ffc

/*
 * CPU Interface layout
 */
#define GICC_CTLR      0x0000
#define GICC_PMR       0x0004
#define GICC_BPR       0x0008
#define GICC_IAR       0x000c
#define GICC_EOIR      0x0010
#define GICC_RPR       0x0014
#define GICC_HPPIR     0x0018
#define GICC_ABPR      0x001c
#define GICC_AIAR      0x0020
#define GICC_AEOIR     0x0024
#define GICC_AHPPIR    0x0028
#define GICC_APR0      0x00d0
#define GICC_NSAPR0    0x00e0
#define GICC_IIDR      0x00fc
#define GICC_DIR       0x1000
#endif

#define MAX_SPIS            480

#define MAX_PPIS            14
#define MAX_SGIS            16

#define MIN_SGI_ID          0
#define MIN_PPI_ID          16
#define MIN_SPI_ID          32

#define GRP0                0
#define GRP1                1

#define _GICD_              0x1000
#define _GICC_              0x2000
#define _GICH_              0x4000
#define _GICV_              0x6000

#define GIC_PRI_MASK                0xff
#define GIC_HIGHEST_SEC_PRIORITY    0
#define GIC_LOWEST_SEC_PRIORITY     127
#define GIC_HIGHEST_NS_PRIORITY     128
#define GIC_LOWEST_NS_PRIORITY      254 /* 255 would disable an interrupt */
#define GIC_SPURIOUS_INTERRUPT      1023
#define GIC_TARGET_CPU_MASK 0xff

/* Distributor interface definitions */
#define GICD_CTLR           _GICD_ + 0x0
#define GICD_TYPER          _GICD_ + 0x4
#define GICD_IGROUPR        _GICD_ + 0x80
#define GICD_ISENABLER      _GICD_ + 0x100
#define GICD_ICENABLER      _GICD_ + 0x180
#define GICD_ISPENDR        _GICD_ + 0x200
#define GICD_ICPENDR        _GICD_ + 0x280
#define GICD_ISACTIVER      _GICD_ + 0x300
#define GICD_ICACTIVER      _GICD_ + 0x380
#define GICD_IPRIORITYR     _GICD_ + 0x400
#define GICD_ITARGETSR      _GICD_ + 0x800
#define GICD_ICFGR          _GICD_ + 0xC00
#define GICD_SGIR           _GICD_ + 0xF00
#define GICD_CPENDSGIR      _GICD_ + 0xF10
#define GICD_SPENDSGIR      _GICD_ + 0xF20

#define IGROUPR_SHIFT       5
#define ISENABLER_SHIFT     5
#define ICENABLER_SHIFT     ISENABLER_SHIFT
#define ISPENDR_SHIFT       5
#define ICPENDR_SHIFT       ISPENDR_SHIFT
#define ISACTIVER_SHIFT     5
#define ICACTIVER_SHIFT     ISACTIVER_SHIFT
#define IPRIORITYR_SHIFT    2
#define ITARGETSR_SHIFT     2
#define ICFGR_SHIFT         4
#define CPENDSGIR_SHIFT     2
#define SPENDSGIR_SHIFT     CPENDSGIR_SHIFT

/* GICD_TYPER bit definitions */
#define IT_LINES_NO_MASK    0x1f

/* GICD_ICFGR bit definitions */
#define LEVEL_SENSITIVE     0x0
#define TRIGGER_SENSITIVE   0x1

/* Physical CPU Interface registers */
#define GICC_CTLR           _GICC_ + 0x0
#define GICC_PMR            _GICC_ + 0x4
#define GICC_BPR            _GICC_ + 0x8
#define GICC_IAR            _GICC_ + 0xC
#define GICC_EOIR           _GICC_ + 0x10
#define GICC_RPR            _GICC_ + 0x14
#define GICC_HPPIR          _GICC_ + 0x18
#define GICC_AHPPIR         _GICC_ + 0x28
#define GICC_IIDR           _GICC_ + 0xFC
#define GICC_DIR            _GICC_ + 0x1000
#define GICC_PRIODROP       _GICC_ + GICC_EOIR

/* GICC_CTLR bit definitions */
#define EOI_MODE_NS         (1 << 10)
#define EOI_MODE_S          (1 << 9)
#define IRQ_BYP_DIS_GRP1    (1 << 8)
#define FIQ_BYP_DIS_GRP1    (1 << 7)
#define IRQ_BYP_DIS_GRP0    (1 << 6)
#define FIQ_BYP_DIS_GRP0    (1 << 5)
#define CBPR                (1 << 4)
#define FIQ_EN              (1 << 3)
#define ACK_CTL             (1 << 2)
#define ENABLE_GRP1         (1 << 1)
#define ENABLE_GRP0         (1 << 0)

/* GICC_IIDR bit masks and shifts */
#define GICC_IIDR_PID_SHIFT     20
#define GICC_IIDR_ARCH_SHIFT    16
#define GICC_IIDR_REV_SHIFT     12
#define GICC_IIDR_IMP_SHIFT     0

#define GICC_IIDR_PID_MASK  0xfff
#define GICC_IIDR_ARCH_MASK 0xf
#define GICC_IIDR_REV_MASK  0xf
#define GICC_IIDR_IMP_MASK  0xfff

/* HYP view virtual CPU Interface registers */
#define GICH_CTL            0x0
#define GICH_VTR            0x4
#define GICH_ELRSR0         0x30
#define GICH_ELRSR1         0x34
#define GICH_APR0           0xF0
#define GICH_LR_BASE        0x100

/* Virtual CPU Interface registers */
#define GICV_CTL            0x0
#define GICV_PRIMASK        0x4
#define GICV_BP             0x8
#define GICV_INTACK         0xC
#define GICV_EOI            0x10
#define GICV_RUNNINGPRI     0x14
#define GICV_HIGHESTPEND    0x18
#define GICV_DEACTIVATE     0x1000

extern void     nvt_enable_irq(int number);
extern void     nvt_disable_irq(int number);
extern void     arm_gic_cpuif_setup(void);
extern void     arm_gic_distif_setup(void);
extern void     gicd_write_igroupr(UINT32 id, UINT32 val);
extern void     gicd_write_ipriorityr(UINT32 id, UINT32 val);

extern UINT32   gicc_get_IAR(void);
extern void     gicc_set_EOIR(UINT32 val);




#endif /* _ASM_ARMV8_GIC_H_ */
