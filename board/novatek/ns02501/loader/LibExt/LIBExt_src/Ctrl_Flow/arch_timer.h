#ifndef _ARCH_TIMER_H
#define _ARCH_TIMER_H

#if defined(__aarch64__)
#define configTIMER_NUMBER 26
#else
#define configTIMER_NUMBER 29
#endif

/**
 * arch timer, refre to RTOS
 */
#define configCPU_CLOCK_HZ          12000000 /* for arm32 reference */
#define configTICK_RATE_HZ	    1000
#define SYSCNT_READ_BASE            (0xFFE50000)
#define SYSCNT_CTRL_BASE            (0xFFE60000)

#define CNTCR_OFF                   0x000
#define CNTSR_OFF                   0x004
#define CNTCVL_OFF                  0x008
#define CNTCVU_OFF                  0x00c
#define CNTFID_OFF                  0x020

#define CNTCR_EN                    (1 << 0)    /* Enables the counter      */
#define CNTCR_HDBG                  (1 << 1)    /* Halt-on-debug            */
#define CNTCR_FCREQ(x)              ((x) << 8)  /* Frequency change request */

#define CNTP_CTL_ENABLE_SHIFT       0
#define CNTP_CTL_IMASK_SHIFT        1
#define CNTP_CTL_ISTATUS_SHIFT      2
#define CNTP_CTL_ENABLE_MASK        1
#define CNTP_CTL_IMASK_MASK         1
#define CNTP_CTL_ISTATUS_MASK       1

#define CNTK_CTL_ACCESS_FROM_EL0    (1<<0)

#if (configTIMER_NUMBER == 27) /* Virtual timer */
#define  READ_CNTxCT(m)         read_cntvct()
#define WRITE_CNTx_CVAL(m)      write_cntv_cval(m)
#define  READ_CNTx_CTL()        read_cntv_ctl()
#define WRITE_CNTx_CTL(m)       write_cntv_ctl(m)
#elif(configTIMER_NUMBER == 29 || configTIMER_NUMBER == 30) /* Non sec timer */
#define  READ_CNTxCT(m)         read_cntpct()
#define WRITE_CNTx_CVAL(m)      write_cntp_cval(m)
#define  READ_CNTx_CTL()        read_cntp_ctl()
#define WRITE_CNTx_CTL(m)       write_cntp_ctl(m)
#elif(configTIMER_NUMBER == 26) /* Non-secure EL2 physical timer */
#define  READ_CNTxCT(m)         read_cntpct()
#define WRITE_CNTx_CVAL(m)      write_cnthp_cval(m)
#define  READ_CNTx_CTL()        read_cnthp_ctl()
#define WRITE_CNTx_CTL(m)       write_cnthp_ctl(m)
#endif

unsigned int read_cntfrq(void);
void write_cntfrq(unsigned int freq);
unsigned int read_cntkctl(void);
void write_cntkctl(unsigned int cntkctl);

//arm virtual timer
unsigned long long read_cntvct(void);
void write_cntv_cval(unsigned long long cval);
unsigned int read_cntv_ctl(void);
void write_cntv_ctl(unsigned int ctl);

//arm physical timer
unsigned long long read_cntpct(void);
void write_cntp_cval(unsigned long long cval);
unsigned int read_cntp_ctl(void);
void write_cntp_ctl(unsigned int ctl);

//arm el2 physical timer
unsigned int read_cnthctl(void);
void write_cnthctl(unsigned int cnthctl);
void write_cnthp_cval(unsigned long long cval);
unsigned int read_cnthp_ctl(void);
void write_cnthp_ctl(unsigned int ctl);
#endif