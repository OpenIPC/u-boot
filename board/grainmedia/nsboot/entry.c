/*
 *      entry.c
 *
 *      Entry point for nsboot
 *
 *      Written by:     Justin Shih <wdshih@faraday-tech.com>
 *
 *              Copyright (c) 2014 GM, ALL Rights Reserve
 */
#include "board.h"
#include "nand.h"
#include "spi020.h"
#include "kprint.h"
#include "ahb_dma.h"

#include "package.h"

//#define DO_DEBUG

#define VERSION			    "Version 1.0.3"
#define LOAD_ADDR		    0x0
//#define UART_FROM_PLL1

#define SYS_CLK             30

FLASH_TYPE_T flash_type = FLASH_SPI_NAND;

extern void serial_init(void);

void hang(void)
{
    puts("### ERROR ### Please RESET the board ###\n");
    for (;;) ;
}

void __div0(void)
{
    hang();
}

/* libgcc's division by zero handler on Linux toolchains */
int raise(int sig)
{
    hang();
    return 0;
}

#ifdef DO_DEBUG

static void hexdump(unsigned char *data, int len)
{
    int i, j;

    for (i = 0; i < len; i += 16) {
        KPrint("\n%04xh:", i);
        for (j = 0; j < 16; j++)
            KPrint(" %02X", data[i + j]);
        KPrint("   ");
        for (j = 0; j < 16; j++) {
            int c = data[i + j];
            if (c < 0x20)
                KPrint(".");
            else
                KPrint("%c", c);
        }
    }
    KPrint("\n");
}

static void hdrdump(IMGHDR * hdr)
{
    KPrint("<Image Header>\n");
    KPrint("  magic : 0x%x\n", hdr->magic);
    KPrint("  chksum: 0x%x\n", hdr->chksum);
    KPrint("  size  : 0x%x\n", hdr->size);
    KPrint("  time  : 0x%x\n", hdr->time);
    KPrint("  name  : %s\n", hdr->name);
}

void syshdrdump(SYSHDR * hdr)
{
    KPrint("<System Header>\n");
    KPrint("  signature: %s\n", hdr->signature);
    KPrint("  bootm_addr: 0x%x\n", hdr->bootm_addr);
    KPrint("  stsize: %d\n", hdr->stsize);
    KPrint("  st_0: 0x%x\n", hdr->st.setting[0]);
    KPrint("  st_1: 0x%x\n", hdr->st.setting[1]);
    KPrint("  st_2: 0x%x\n", hdr->st.setting[2]);
}

#endif

void check_flash_type(void)
{
#if 0
    flash_type = FLASH_SPI_NAND;
#else
    /* bit7 => 1:NAND, 0:SPI
     */
    //A version
    if (read_reg(REG_PMU_BASE + 0x04) & (1 << 7))
        flash_type = FLASH_SPI_NAND;
    else
        flash_type = FLASH_NOR;

#endif
}

int u32PMU_ReadPLL1CLK(void)
{
    unsigned int value, n, m;

    value = read_reg(REG_PMU_BASE + 0x30);
    n = (value >> 4) & 0x7F;
    m = (value >> 11) & 0x1F;
    value = (SYS_CLK * n) / m;
 
    return value;
}

int u32PMU_ReadPLL2CLK(void)
{
    unsigned int value, n, m;

    value = read_reg(REG_PMU_BASE + 0x34);
    n = (value >> 4) & 0x7F;
    m = (value >> 11) & 0x1F;
    value = (SYS_CLK * n) / m;

    return value;
}

void check_SIP(void)
{
	unsigned int ID = 0;

    ID = (read_reg(REG_PMU_BASE) >> 2) & 0x1F;

	switch (ID) {
	  case 0x2:case 0x4:case 0x10:
        write_reg(REG_PMU_BASE + 0x64, 0x1 << 30);
	    break;
	  case 0x14:
        write_reg(REG_PMU_BASE + 0x64, 0x3 << 30);
	    break;
	  default:
	    break;
	}
}

static void platform_setup(void)
{
    unsigned int reg, value;

#ifdef PLL3_594//default clock 540, Nx30/M
    /* PLL3 set 594MHz */
    reg = read_reg(REG_PMU_BASE + 0x34) & 0x0003FFFF;

    reg = reg | (5 << 27) | (99 << 20);     //PLL3 ms[31:27], PLL3 ns[26:20]
    //reg = reg | (3 << 11) | (80 << 4);      //PLL2 ms[15:11], PLL2 ns[10:4]
    write_reg(REG_PMU_BASE + 0x34, reg);
#endif
    /* UART pin mux */
    reg = read_reg(REG_PMU_BASE + 0x58) & 0x0FFFFFFF;
    write_reg(REG_PMU_BASE + 0x58, reg | (0x5 << 28));

#ifdef CONFIG_FPGA
    /* Open AXI/AHB/APB clock */
    write_reg(REG_PMU_BASE + 0xB0, 0x0);
    write_reg(REG_PMU_BASE + 0xB4, 0x0);
    write_reg(REG_PMU_BASE + 0xB8, 0x0);
    write_reg(REG_PMU_BASE + 0xBC, 0x0);
#else
    /* axic2 PWM enable */
    write_reg(REG_PMU_BASE + 0x38, 0x1);
    
    /* UART clock div */
    reg = read_reg(REG_PMU_BASE + 0x70) & 0xFFC0FFFF;
#ifdef UART_FROM_PLL1
    write_reg(REG_PMU_BASE + 0x70, reg | (0x9 << 16));

    reg = read_reg(REG_PMU_BASE + 0x30) & 0xFFFFF80F;
    reg = reg | (40 << 4);
    write_reg(REG_PMU_BASE + 0x30, reg);
    
    /* UART clock source */
    reg = read_reg(REG_PMU_BASE + 0x28) | (0x1 << 3);
    write_reg(REG_PMU_BASE + 0x28, reg);      
#else
    /* UART from PLL2, calc 25MHz to uart */
    value = u32PMU_ReadPLL2CLK() / 2;
    value = (value / 25) - 1;
    write_reg(REG_PMU_BASE + 0x70, reg | (value << 16));
#endif        
#endif
    serial_init();
    check_SIP();
}

void set_fcs(unsigned int cpu_value, int mul, int div)
{
    unsigned int setting;

    /* set PLL3 CC */
    //setting = read_reg(REG_PMU_BASE + 0x80) & 0xFFFFFCFF;
    //write_reg(REG_PMU_BASE + 0x80, (setting | (0x2 << 8)));
    
    setting = read_reg(REG_PMU_BASE + 0x30) & 0xF;
    /* Set CPU to deal with FCS irq */
    write_reg(0x96000000 + 0x45C, 0x100);
    write_reg(0x96000000 + 0x4, 0x100);

    /* set PLL1 */
    setting = setting | (cpu_value << 16) | (div << 11) | (mul << 4) | (0x3 << 2);
    write_reg(REG_PMU_BASE + 0x30, setting);

    KPrint("go...");

    /* set PLL2 & PLL3 frang */
#ifdef MODE_CPU    
    setting = read_reg(REG_PMU_BASE + 0x34) & 0xFFF3F803;
    write_reg(REG_PMU_BASE + 0x34, setting | (0x3 << 18) | (40 << 4) | (0x3 << 2));
#else
    setting = read_reg(REG_PMU_BASE + 0x34) & 0xFFF3FFF3;
    write_reg(REG_PMU_BASE + 0x34, setting | (0x3 << 18) | (0x3 << 2));
#endif
        
    setting = read_reg(REG_PMU_BASE + 0x0C);
    write_reg(REG_PMU_BASE + 0x0C, setting | (1 << 21));        //select CPU off
    write_reg(REG_PMU_BASE + 0x0C, setting | (1 << 21) | (1 << 2));

    setting = 0;

    __asm__ __volatile__(" mcr p15, 0, %0, c7, c0, 4 ": "=r"(setting):: "memory");
    __asm__ __volatile__(" nop ":::"memory");
    __asm__ __volatile__(" nop ":::"memory");
    __asm__ __volatile__(" nop ":::"memory");   //return here from IDLE mode interrupt 
}

void InvalidateCache(void)
{
    unsigned int setting = 0;

    __asm__ __volatile__(" mcr p15, 0, %0, c7, c5, 0 ": "=r"(setting):: "memory");

    __asm__ __volatile__(" mcr p15, 0, %0, c7, c6, 0 ": "=r"(setting):: "memory");
}

#ifdef MEM_TEST
int memtest(void)
{
    unsigned int *bg;
    int i = 0, j = 0, fail = 0;
    unsigned int pattern[2] = {0x5a5a5a5a, 0xa5a5a5a5};
    int TEST_LENGTH = 0x60000;
    unsigned int addr = 0;

    KPrint("Test addr: 0x%x, len = 0x%x\n", addr, TEST_LENGTH);    
    bg = (unsigned int *)addr;
    for (j = 0; j < (TEST_LENGTH / 4); j += 2) {
        *bg++ = pattern[i];
        *bg++ = pattern[i + 1];
    }
    
    bg = (unsigned int *)addr;
    for (j = 0; j < (TEST_LENGTH / 4); j += 2) {
        if (*bg != pattern[i] || (*(bg + 1) != pattern[i + 1]))
            fail++;
    
        bg += 2;
    }
    if (fail) {
        KPrint("error number %d: ", fail);
    } else {
        KPrint("Test pass, pattern 0x%x, 0x%x\n", pattern[i], pattern[i + 1]);
    }
    if(fail > 0)
    	while(1);

    return 0;
}
#endif

/*
 * The package, as the kernel's pmu_get_chip() reads it from PMU ID bits
 * 6:2: 0 is the GM8136 (external DDR), 0x14 and 0x1C the GM8136S, the
 * rest GM8135S. This used to be fixed at build time (MFLAGS).
 */
const struct gm_package *get_package(void)
{
    switch ((read_reg(REG_PMU_BASE) >> 2) & 0x1F) {
    case 0x0:
        return &gm8136_package;
    case 0x14: case 0x1C:
        return &gm8136s_package;
    default:
        return &gm8135s_package;
    }
}

#ifndef USE_SCP
void set_ddr(const struct gm_package *pkg)
{
    volatile int i, reg;
    const unsigned int (*arg)[2] = pkg->ddr_arg;
    int len = pkg->ddr_len;

    //KPrint("set ddr arg\n");
#ifdef MODE_SPECIAL
	reg = 0;
#else
    reg = read_reg(REG_PMU_BASE);
#endif

	if (pkg->ddr_arg_ar && reg == 0x8136108) {
	    arg = pkg->ddr_arg_ar;
	    len = pkg->ddr_ar_len;
	}
	for(i = 0; i < len; i++)
	    write_reg(REG_DDR0_BASE + arg[i][0], arg[i][1]);
    
    write_reg(REG_DDR0_BASE + 0x04, 0x00000001);
    while(!(read_reg(REG_DDR0_BASE + 0x04) & 0x100));
    
    for(i = 0; i < 60000; i++);
}
#endif

void FCS_irq_data(void)
{
    //enable CPU IRQ
    __asm__ __volatile__("\n"
			"	MRS r0, CPSR\n"
			"	BIC r0, r0, #0x80\n"
			"	MSR CPSR_c, r0\n");

    //ldr pc, 0x40 assembly code
    write_reg(0x18, 0xE59FF020);
    //0x40 data is 0x92000040 assembly code
    write_reg(0x40, 0x92000040);
}

//volatile int tt=0;
void entry(void)
{
    unsigned int image_offset = (unsigned int)-1;
    struct spi_flash *spi_chip = NULL;
    unsigned int reg;
    void (*image_entry) (void) = (void *)LOAD_ADDR;
    const struct gm_package *pkg;

    platform_setup();
    pkg = get_package();

#ifndef USE_SCP
    set_ddr(pkg);
#endif    
    FCS_irq_data();

#ifndef CONFIG_FPGA
#ifdef MEM_TEST
    KPrint("memtest before FCS\n");
    memtest();
    write_reg(0x18, 0xE59FF020);
    write_reg(0x40, 0x92000040);
#endif
    KPrint("%s ", pkg->name);
    if (pkg->fcs_mul)
        set_fcs(pkg->fcs_value, pkg->fcs_mul, pkg->fcs_div);
    KPrint("%s\n", pkg->mode);
#endif  //CONFIG_FPGA

#ifdef MEM_TEST
    KPrint("memtest after FCS\n");
    memtest();    
#endif
	KPrint(VERSION "\n");

    InvalidateCache();
    check_flash_type();

    /* SPI clock on */
    reg = read_reg(REG_PMU_BASE + 0xB4);
    reg &= ~(0x1 << 15);
    write_reg(REG_PMU_BASE + 0xB4, reg);

#ifdef CONFIG_SPI_DMA    
    /* DMA init */
    Dma_init((1 << REQ_SPI020RX) | (1 << REQ_SPI020TX));
#endif
    /* SPI init */
    spi_chip = FTSPI020_Init();
    if (!spi_chip) {
        KPrint("SPI init fail! \n");
        return;
    }

    if (flash_type == FLASH_SPI_NAND) {
        //KPrint("MP SPI-NAND Bootstrap " VERSION "\n");

        image_offset = fLib_SPI_NAND_Copy_Image((unsigned int)LOAD_ADDR);
    }
    if (flash_type == FLASH_NOR) {
        //KPrint("MP SPI-NOR Bootstrap " VERSION "\n");

        image_offset = fLib_NOR_Copy_Image((unsigned int)LOAD_ADDR);
    }

    /* The copy functions return (unsigned) -1 when the load fails */
    if (image_offset == (unsigned int)-1) {
        KPrint("image_offset < 0\n");
        hang();
    }
//tt++;
//while(tt > 0);            
    //KPrint("Boot image offset: 0x%x. Booting Image .....\n", image_offset);
    image_entry();

    return;
}
