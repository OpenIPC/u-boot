/*
 *      main.c
 *
 *      Entry point for nsboot
 *
 *      Copyright (c) 2018 Novatek, ALL Rights Reserve
 */
#include "main.h"
#include "board.h"
#include "constant.h"
#include "compiler.h"
#include "mmc_api.h"
#include "spi020.h"
#include "fuart.h"
#include "timer.h"
#include "ahb_dma.h"
#include "bus.h"
#include "pll.h"
//#include "ddr.h"
#include "debug.h"
#include "bdata.h"
#include "nvt_rdata_sys.h"
#include "data_keeper.h"
#include "bl_func.h"
#include "cache.h"
#include "global.h"
#include "nvtpack.h"
#include "bin_info.h"
#include "Memory.h"

//#define MEM_TEST
//#define CONFIG_FIX_ADDR	1
//#define DO_DEBUG
//#define USE_SCP       /* use ice scp file set ddr and remap */

//int  __attribute__ ((section (".READ_CUSTOM_DATA"))) read_custom_data_if(unsigned int, struct buffer_info_t*, int, int);

FLASH_TYPE_T flash_type = FLASH_SD;
#define _THUMB2 __attribute__((target("thumb")))
#define VERSION			    "Version 1.0.00"

// Specific test
#define DRAM_TEST               (0)
#define HVYLOAD_TEST            (0)
#define WDT_TEST_ENABLE         (0)

#define WDT_CASE_INTRESET       (0)
#define WDT_CASE_EXTRESET       (1)
#define WDT_CASE_NORMAL         (2)
#define WDT_TEST_CASE           (WDT_CASE_EXTRESET)

#ifndef ALIGN_FLOOR
#define ALIGN_FLOOR(value, base)  ((value) & ~((base)-1))
#endif
#ifndef ALIGN_CEIL
#define ALIGN_CEIL(value, base)   ALIGN_FLOOR((value) + ((base)-1), base)
#endif

extern char _loader_exec_compres_start[];
extern char _loader_exec_compress_load_cpu_addr[];

extern char _loader_data_cpu_addr[];
extern char _loader_exec_end_base[];
extern char _loader_data_start_base[];
extern char _loader_data_size[];

extern int decompress(unsigned char *buf, int in_len, unsigned char *output);
extern void reset_cpu(unsigned int base, unsigned int count);
extern void wait_delay(unsigned int loop);
extern void jmpcore1(void);
extern void fLib_USB_Update_FW(void);

extern void serial_init(void);


extern unsigned int read_reg(unsigned int reg);
extern void write_reg(unsigned int reg, unsigned int value);

#define write_reg(m,n)	OUTW(m,n)
#define read_reg(m)		INW(m)

void hang(void);
void __div0(void);
void check_storage_type(void);
void u32PMU_WritePLL(unsigned int base, uint num, uint offset, uint value);
void set_bus(unsigned int base);
void set_ddr0(unsigned int base);
#if (defined(DDR1_1800_SZ_1Gb) || defined(DDR1_1800_SZ_2Gb) || defined(DDR1_1800_SZ_4Gb) || defined(DDR1_1600_SZ_4Gb))
/* RC DDRC1 */
void set_ddr1(unsigned int base);
#endif
void set_fire(unsigned int base);
void cpu_change_armpll(void);
void show_evb_message(void);
void reset_EP(void);
#if HVYLOAD_TEST
void heavy_load_test(void);
#endif
//void entry(void);
void entry(void) __attribute__ ((section (".part1")));

#define KPrint
void hang(void)
{
    debug_msg("### ERROR ### Please RESET the board ###\n");
    for (;;) ;
}

void __div0(void)
{
    hang();
}

#ifdef DO_DEBUG

_THUMB2 static void hexdump(unsigned char *data, int len)
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

_THUMB2 void syshdrdump(SYSHDR * hdr)
{
    KPrint("<System Header>\n");
    KPrint("  signature: %s\n", hdr->signature);
    KPrint("  bootm_addr: 0x%x\n", hdr->bootm_addr);
    KPrint("  stsize: %d\n", hdr->stsize);
    KPrint("  st_0: 0x%x\n", hdr->st.setting[0]);
    KPrint("  st_1: 0x%x\n", hdr->st.setting[1]);
    KPrint("  st_2: 0x%x\n", hdr->st.setting[2]);
}

#endif //DO_DEBUG

_THUMB2 void u32PMU_WritePLL(unsigned int base, uint num, uint offset, uint value)
{
    unsigned int data;

    if(num == 2) {
        /* CPU switch to 12M */
        write_reg(base + 0x1340, 0x02);
    }

    if(offset == 0x7C0) {/* CPU */
        data = (value * (1 << 17) / 80) / 12;
    } else if((offset == 0x4C0) || (offset == 0x500)) {/* DDR */
        data = (value * 68266) / 1000;/* 131072 / (12 * 160) *///0x01F111;
        //KPrint("data = 0x%x\n", data);
    } else {
        data = (value * (1 << 17)) / 12;
    }

    write_reg(base + 0x10000 + offset + 0x20, data & 0xFF);
    write_reg(base + 0x10000 + offset + 0x24, (data >> 8) & 0xFF);
    write_reg(base + 0x10000 + offset + 0x28, (data >> 16) & 0xFF);

    /* ddr0 & ddr1 set at the same time */
    if(num == 3)
        return;

    /* hit cache & wait clock stable */
    if((num < 2) || (num > 4)) {
        /* delay 30 us */
        wait_delay(3000);
    } else if(num == 2) {
        /* cpu delay 300 us */
        wait_delay(30000);
        write_reg(base + 0x1340, 0x03);
    } else if(num == 4) {
        /* ddr delay 30 us */
        wait_delay(3000);
        write_reg(base + 0x20, 0x0000A001);
        /* ddr delay 300 us */
        wait_delay(30000);
        write_reg(base + 0x20, 0x0000E001);
    }
}

_THUMB2 void check_storage_type(void)
{
    unsigned int value;

    value = read_reg(REG_PMU_BASE + 0x04) & (1 << 2);
    if (value)
        flash_type = FLASH_NOR;
    else
        flash_type = FLASH_SPI_NAND;
    //else
    //    flash_type = FLASH_SD;
}

#if HVYLOAD_TEST
__attribute__((target("thumb"))) void heavy_load_test(void)
{
    //volatile int tt = 0;
    unsigned int test_addr1 = 0x30001000, test_addr2 = 0x30101000, test_addr3 = 0x30202000;
    unsigned int test_size = 0x100000;
    unsigned int value;

    //value = read_reg(REG_PMU_BASE + 0x50);
    //value |= (1 << 25);
    //write_reg(REG_PMU_BASE + 0x50, value);

    // set heavy load pattern
    write_reg(REG_HEAVY_LOAD_BASE + 0x8090, 0xaaaaaaaa);
    write_reg(REG_HEAVY_LOAD_BASE + 0x8094, 0x55555555);
    write_reg(REG_HEAVY_LOAD_BASE + 0x8098, 0x00000000);
    write_reg(REG_HEAVY_LOAD_BASE + 0x809C, 0xffffffff);

    // set heavy load region
    write_reg(REG_HEAVY_LOAD_BASE + 0x8064, test_addr1);
    write_reg(REG_HEAVY_LOAD_BASE + 0x8068, test_size);
    write_reg(REG_HEAVY_LOAD_BASE + 0x8074, test_addr2);
    write_reg(REG_HEAVY_LOAD_BASE + 0x8078, test_size);
    write_reg(REG_HEAVY_LOAD_BASE + 0x8084, test_addr3);
    write_reg(REG_HEAVY_LOAD_BASE + 0x8088, test_size);

    // trigger heavy load

	write_reg(REG_HEAVY_LOAD_BASE + 0x8060, 0x00103fc1);
    write_reg(REG_HEAVY_LOAD_BASE + 0x8070, 0x00103fc1);
    write_reg(REG_HEAVY_LOAD_BASE + 0x8080, 0x00103fc1);
	write_reg(REG_HEAVY_LOAD_BASE + 0x805c, 0x7);

    value = read_reg(REG_HEAVY_LOAD_BASE + 0x80A0);
    value &= 0x03;

    while(value == 0) {
        value = read_reg(REG_HEAVY_LOAD_BASE + 0x80A0);
        //KPrint("data = 0x%x\n", value);
        value &= 0x03;
        if(value)
            break;
    }

    //KPrint("0x80A0 data = 0x%x\n", value);
    if(read_reg(REG_HEAVY_LOAD_BASE + 0x80A0) & 0x2){
        //KPrint("HEAVY LOAD fail, ERROR_DATA = 0x%x, CORRECT_DATA = 0x%x, ADDR = 0x%x\n",
          //  read_reg(REG_HEAVY_LOAD_BASE + 0x80A4), read_reg(REG_HEAVY_LOAD_BASE + 0x80A8), read_reg(REG_HEAVY_LOAD_BASE + 0x80AC));
          debug_msg("HEAVY LOAD NG\n");
          while(1);
    }
    else
        debug_msg("HEAVY LOAD pass\n");

    // write 1 clear
    write_reg(REG_HEAVY_LOAD_BASE + 0x80A0, 0x03);

	//tt++;
	//while(tt > 0);
}
#endif

void set_bus(unsigned int base)
{
    volatile unsigned int i;

    //KPrint("set_bus\n");

    for(i = 0; i < sizeof(bus_arg) / 8; i++) {
        write_reg(base + bus_arg[i][0], bus_arg[i][1]);
    }

    for(i = 0; i < 60000; i++);

}

/* RC DDRC0 */
void set_ddr0(unsigned int base)
{
#if 0
    volatile unsigned int i;

    //KPrint("set ddr0 arg\n");

    while(!(read_reg(base + 0x1000) & 0x10));

    for(i = 0; i < sizeof(DDR0_arg) / 8; i++) {
        write_reg(base + DDR0_arg[i][0], DDR0_arg[i][1]);
    }

    write_reg(base + 0x04, 0x00000001);
    while(!(read_reg(base + 0x04) & 0x100));

    //for(i = 0; i < 60000; i++);
#endif
}

#if (defined(DDR1_1800_SZ_1Gb) || defined(DDR1_1800_SZ_2Gb) || defined(DDR1_1800_SZ_4Gb) || defined(DDR1_1600_SZ_4Gb))
/* RC DDRC1 */
void set_ddr1(unsigned int base)
{
    volatile unsigned int i;

    //KPrint("set ddr1 arg\n");

    while(!(read_reg(base + 0x1000) & 0x10));

    for(i = 0; i < sizeof(DDR1_arg) / 8; i++) {
        write_reg(base + DDR1_arg[i][0], DDR1_arg[i][1]);
    }

    write_reg(base + 0x04, 0x00000001);
    while(!(read_reg(base + 0x04) & 0x100));

    //for(i = 0; i < 60000; i++);
}
#endif  //(defined(DDR1_1800_SZ_1Gb) || defined(DDR1_1800_SZ_2Gb) || defined(DDR1_1800_SZ_4Gb))

static void platform_setup(unsigned int pmu_base, unsigned int h2x_base, unsigned int dmac030_base, unsigned int tmr010_base)
{
#if 0
    unsigned int value;

    /* Enable gating clock */
    /* release tmr & wdt */
    value = read_reg(pmu_base + 0x78);
    value = value & ~((0x1 << 7) | (0x1 << 15));
    write_reg(pmu_base + 0x78, value);

    /* disable DSP */
    write_reg(pmu_base + 0x104, 0x00060000);

    /* Set H2X R/W prottype */
    write_reg(h2x_base + 0x20000, 0x5 << 2);
    //write_reg(h2x_base + 0x40000, 0x5 << 2);
    //write_reg(h2x_base + 0x60000, 0x14);
    write_reg(h2x_base + 0x80000, 0x14);
    //write_reg(h2x_base + 0xA0000, 0x5 << 2);
    write_reg(h2x_base + 0xC0000, 0x5 << 2);

	/* Set VGA & CVBS */
	value = read_reg(pmu_base + 0xFC) & 0xFE1FFE1F;
	value = value | (0x7 << 21) | (0x7 << 5);
	write_reg(pmu_base + 0xFC, value);

    /* WDT exit reset */
    value = read_reg(pmu_base + 0x5C);
    value |= (0x1 << 28);
    write_reg(pmu_base + 0x5C, value);

    /* DMAC030 exit reset */
    value = read_reg(pmu_base + 0x50);
    value |= (0x1 << 16);
    write_reg(pmu_base + 0x50, value);

    /* Turn on u_FTDMAC030 PClk */
    value = read_reg(pmu_base + 0x70);
    value &= ~(0x1 << 23);
    write_reg(pmu_base + 0x70, value);

    /* Disable debug uart */
    value = read_reg(0xfe4e0010);
    value &= ~(0x1 << 7);
    write_reg(0xfe4e0010, value);
    /* The following IP is used by ROM CODE, we need to clear its interrupt
     */
    /* clear DMAC030 pending interrupts */
    write_reg(dmac030_base + 0x8, 0xff);
    /* clear DMAC030 Error/Abort interrupts */
    write_reg(dmac030_base + 0x10, 0xff);
    /* synchronize the corresponding dma_req */
    write_reg(dmac030_base + 0x20, 0xffff);

    write_reg(tmr010_base + 0x38, 0xff);

	/* UART pin mux */
	/* UART clock source */
    /* UART from PLL4, calc clock div to uart */
    //reg = read_reg(REG_PMU_BASE + 0x194) & 0xFFFFFFC0;
    //value = u32PMU_ReadPLL4CLK() / 6;
    //value = (value / (2*16*115200)) - 1;
    value = read_reg(pmu_base + 0x194);
    value &= ~0xFF;
    write_reg(pmu_base + 0x194, value | 0x1);

	if(pmu_base == REG_PMU_BASE)
    	serial_init();
#endif
}

void set_fire(unsigned int base)
{
#if 0
#if (defined(DDR0_1200_SZ_4Gbx2) || defined(DDR0_1200_SZ_2Gbx2))
    unsigned int i, PLL[6][2] = {};
    KPrint("clock no change...\n");
	return;
#else
    #if (defined(DDR0_1600_SZ_4Gbx2) || defined(DDR0_1600_SZ_2Gbx2))
        /* codec, axi0, CPU, ddr0, ddr1 7.5~11.5, cnn */
        unsigned int i, PLL[6][2] = {{0x740, 380}, {0x400, 467}, {0x7C0, 1100}, {0x4C0, 1600}, {0x500, 1600}, {0x480, 600}};
    #else
        #if (defined(DDR0_1800_SZ_4Gb) || defined(DDR0_1800_SZ_4Gbx2) || defined(DDR0_1800_SZ_2Gbx2) || defined(DDR0_1800_SZ_1Gbx2))
            unsigned int i, PLL[6][2] = {{0x740, 380}, {0x400, 467}, {0x7C0, 1100}, {0x4C0, 1866}, {0x500, 1866}, {0x480, 600}};
        #else
            KPrint("not define this type ddr setting...\n");
        	return;
        #endif
    #endif
#endif
    debug_msg("clock change...");
    for(i = 0; i < 6; i++) {
        //KPrint("offset = 0x%08x, data = 0x%08x\n", PLL[i][0], PLL[i][1]);
        u32PMU_WritePLL(base, i, PLL[i][0], PLL[i][1]);
    }

    debug_msg("done\n");
#endif
}

#ifdef MEM_TEST
int memtest(void);

_THUMB2 int memtest(void)
{
    unsigned int *bg;
    int i = 0, j = 0, fail = 0;
    unsigned int pattern[2] = {0x5a5a5a5a, 0xa5a5a5a5};
    int TEST_LENGTH = 0x100000; /* 1MB */
    unsigned int addr = 0x1000000;

    //KPrint("Test addr: 0x%x, len = 0x%x\n", addr, TEST_LENGTH);
	debug_msg("Test addr: 0x%x, len =\n");
    bg = (unsigned int *)addr;

    for (j = 0; j < (TEST_LENGTH / 4); j += 2) {
        *bg++ = pattern[0];
        *bg++ = pattern[1];
    }

    bg = (unsigned int *)addr;
    for (j = 0; j < (TEST_LENGTH / 4); j += 2) {
        if (*bg != pattern[i] || (*(bg + 1) != pattern[i + 1])){
            fail++;
#if 1
            if (*bg != pattern[i])
				debug_msg("addr: 0x%x correct data: 0x%x, err = 0x%x\n");
            if (*(bg + 1) != pattern[i + 1])
				debug_msg("addr: 0x%x correct data: 0x%x, err = 0x%x\n");
#endif
        }
        bg += 2;
    }

    if (fail)
        debug_msg("error number %d: ");
    else
        debug_msg("Test pass, pattern 0x%x, 0x%x\n");

    if(fail > 0)
    	while(1);

    return 0;
}
#endif

_THUMB2 void cpu_change_armpll(void)
{
	//spare pll power down
	write_reg(REG_PMU_BASE + 0x10000 + (0x210), 0x01);

	#if 0
    debug_msg("CPU1400 ");
	//CPU 1400 MHz -> CPU-MPLL-175Mhz = 0x1D2AAA
	write_reg(REG_PMU_BASE + 0x10000 + (0x7C0+0x20), 0xAA); // B0
	write_reg(REG_PMU_BASE + 0x10000 + (0x7C0+0x24), 0x2A); // B1
	write_reg(REG_PMU_BASE + 0x10000 + (0x7C0+0x28), 0x1D); // B2
	#elif 1
    debug_msg("CPU1200 ");
	//CPU 1200 MHz -> CPU-MPLL-150Mhz = 0x190000
	write_reg(REG_PMU_BASE + 0x10000 + (0x7C0+0x20), 0x00); // B0
	write_reg(REG_PMU_BASE + 0x10000 + (0x7C0+0x24), 0x00); // B1
	write_reg(REG_PMU_BASE + 0x10000 + (0x7C0+0x28), 0x19); // B2
	#elif 0
    debug_msg("CPU1000 ");
	//CPU 1000 MHz -> CPU-MPLL-125Mhz = 0x14D555
	write_reg(REG_PMU_BASE + 0x10000 + (0x7C0+0x20), 0x55); // B0
	write_reg(REG_PMU_BASE + 0x10000 + (0x7C0+0x24), 0xD5); // B1
	write_reg(REG_PMU_BASE + 0x10000 + (0x7C0+0x28), 0x14); // B2
	#else
    debug_msg("CPU800");
	//CPU 800 MHz -> CPU-MPLL-100Mhz = 0x10AAAA
	write_reg(REG_PMU_BASE + 0x10000 + (0x7C0+0x20), 0xAA); // B0
	write_reg(REG_PMU_BASE + 0x10000 + (0x7C0+0x24), 0xAA); // B1
	write_reg(REG_PMU_BASE + 0x10000 + (0x7C0+0x28), 0x10); // B2
	#endif

	// wait 300us for synthesizer stable
	udelay(300);

	// de-assert resetb_armpll to 1: Write 0xD1 = 0x10
	write_reg(REG_PMU_BASE + 0x10000 + (0xD1<<2), 0x10);

	// wait 300us for ARMPLL stable
	udelay(500);

	// Set ARMPLL output. Write 0xD0 = 0x03
	write_reg(REG_PMU_BASE + 0x10000 + (0xD0<<2), 0x03);

	// Change CPU to APLL
	write_reg(REG_PMU_BASE + 0x100, read_reg(REG_PMU_BASE + 0x100)|(0x1<<31));

    debug_msg("DONE\r\n");
}


_THUMB2 void show_evb_message(void)
{

    debug_msg("NA51068 ");
	debug_msg(Dec2HexStr2Bytes((LoaderInternalInfo[1] >> 28) & 0xF));
	debug_msg(".");
	debug_msg(Dec2HexStr2Bytes((LoaderInternalInfo[1] >> 24) & 0xF));
	debug_msg(".");
	debug_msg(Dec2HexStr2Bytes((LoaderInternalInfo[1] >> 16) & 0xFF));
	debug_msg("\r\n");

#ifdef DDR0_1200_SZ_4Gbx2
	KPrint("DDR0 1200 4Gbx2\r\n");
#endif

#ifdef DDR0_1600_SZ_2Gbx1
	KPrint("DDR0 1600 2Gbx1\r\n");
#endif

#ifdef DDR0_1600_SZ_4Gbx1
	KPrint("DDR0 1600 4Gbx1\r\n");
#endif

#ifdef DDR0_1600_SZ_2Gbx2
	KPrint("DDR0 1600 2Gbx2\r\n");
#endif

#ifdef DDR0_1600_SZ_4Gbx2
	KPrint("DDR0 1600 4Gbx2\r\n");
#endif

#ifdef DDR0_1800_SZ_4Gb
	KPrint("DDR0 1800 4Gb\r\n");
#endif

#ifdef DDR0_1800_SZ_1Gbx2
	KPrint("DDR0 1800 1Gbx2\r\n");
#endif

#ifdef DDR0_1800_SZ_4Gbx2
	KPrint("DDR0 1800 4Gbx2\r\n");
#endif

#ifdef DDR0_FPGA
	debug_msg("FPGA EMULATION Loader\r\n");
#endif

#ifdef MODE0
	KPrint("CPU800, DSP 600, codec 350, ddr 1600\n");
#endif


}

/* set ep 12MHz */
_THUMB2 void reset_EP(void)
{
    unsigned int reg;
    /* set ext clk */
    reg = read_reg(REG_PMU_BASE + 0x1a4);
    reg &= ~0x3;
    reg |= 0x1;
    write_reg(REG_PMU_BASE + 0x1a4, reg);

    /* set clk 12MHz */
    reg = read_reg(REG_PMU_BASE + 0x170);
    reg &= ~0xFF;
    reg |= 0x6;
    write_reg(REG_PMU_BASE + 0x170, reg);

    /* enable gpio2 */
    reg = read_reg(REG_PMU_BASE + 0x58);
    reg |= (0x1 << 29);
    write_reg(REG_PMU_BASE + 0x58, reg);

    reg = read_reg(REG_PMU_BASE + 0x74);
    reg &= ~(0x1 << 5);
    write_reg(REG_PMU_BASE + 0x74, reg);

    /* set gpio2 pulse */
    reg = read_reg(REG_GPIO2_BASE + 0x08);
    reg |= (0x1 << 27);
    write_reg(REG_GPIO2_BASE + 0x08, reg);

    reg = read_reg(REG_GPIO2_BASE + 0x14);
    reg |= (0x1 << 27);
    write_reg(REG_GPIO2_BASE + 0x14, reg);

    wait_delay(0x10000);

    reg = read_reg(REG_GPIO2_BASE + 0x10);
    reg |= (0x1 << 27);
    write_reg(REG_GPIO2_BASE + 0x10, reg);
}

#if (_FPGA_EMULATION_ == 1)
static inline unsigned long read_periphbase(void)
{
        unsigned long val;
        __asm__ __volatile__(
                                "mrc p15, 4, %0, c15, c0, 0\n\t"
                                : "=r"(val)
                                );
        return val;
}
#endif


#if (_LOADER_COMPRESSED_ == ENABLE) && defined(_UBOOT_BIND_ON_)
_THUMB2 static void bl_core2_reset(void)
{
	UINT32 core_reg;
	core_reg = *(volatile UINT32 *)0xF84000F0;
	core_reg &= ~(1<<3);
	core_reg &= ~(1<<11);
	*(volatile UINT32 *)0xF84000F0 = core_reg;
	core_reg |= (1<<2)|(1<<10) | (1<<21);
	core_reg |= (1<<17);
	*(volatile UINT32 *)0xF84000F0 = core_reg;

}

_THUMB2 static UINT32 bl_boot_bind_uboot(void)
{
	extern char _lzma_tmp_start[];
	extern char _lzma_tmp_end[];
	_ALIGNED(4) static char uboot_pre_deocde[BIN_INFO_OFFSET_UBOOT+sizeof(HEADINFO)];
	int loader_size, padding_bytes;
	NVTPACK_BFC_HDR *p_loader, *p_uboot;

	p_loader = (NVTPACK_BFC_HDR *)(_loader_exec_compress_load_cpu_addr);
	padding_bytes = 4 - (__builtin_bswap32(p_loader->uiSizeComp)&0x3);
	loader_size = __builtin_bswap32(p_loader->uiSizeComp) + LDC_HEADER_SIZE + padding_bytes;
	p_uboot = (NVTPACK_BFC_HDR *)(_loader_exec_compress_load_cpu_addr + loader_size);

	lzma_inflate(
		(UINT8 *)&p_uboot[1],
		__builtin_bswap32(p_uboot->uiSizeComp),
		(UINT8 *)uboot_pre_deocde,
		sizeof(uboot_pre_deocde),
		(UINT8 *)_lzma_tmp_start,
		(UINT32)(_lzma_tmp_end - _lzma_tmp_start)
	);

	HEADINFO *p_head = (HEADINFO *)&uboot_pre_deocde[BIN_INFO_OFFSET_UBOOT];

	lzma_inflate(
		(UINT8 *)&p_uboot[1],
		__builtin_bswap32(p_uboot->uiSizeComp),
		(UINT8 *)p_head->CodeEntry,
		__builtin_bswap32(p_uboot->uiSizeUnComp),
		(UINT8 *)_lzma_tmp_start,
		(UINT32)(_lzma_tmp_end - _lzma_tmp_start)
	);

	UINT32 No_CPU;
	No_CPU = *(UINT32 *)0xF9C00004;
	No_CPU &= 0x3;

	if (No_CPU) {
		extern char _load_core2_jump_program_start_base[];
		extern char _load_core2_jump_program_end_base[];
		extern char _load_core2_entry_program_start_base[];
		extern char _load_core2_entry_program_end_base[];
		UINT32 code2JumpCodelen = _load_core2_jump_program_end_base - _load_core2_jump_program_start_base;
		UINT32 code2EntryCodelen = _load_core2_entry_program_end_base - _load_core2_entry_program_start_base;
		UINT32 core2_entry1_addr = 0x00000000;
		//UINT32 core2_entry2_addr = 0x1FFC0000;
		UINT32 core2_entry2_addr = (_SDRAM_SIZE_ / 8) * 1024 * 1024 - ALIGN_CEIL(code2EntryCodelen, 0x10000);
		// copy core2 jump code to dram 0x0
		utl_memcpy((void *)core2_entry1_addr, _load_core2_jump_program_start_base, code2JumpCodelen);
		CPUflushWriteCache(core2_entry1_addr, code2JumpCodelen);;
		//debug_msg_var("core2_jump_program", (int)_load_core2_jump_program_start_base);
		//debug_msg_var("code2JumpCodelen", (int)code2JumpCodelen);
		debug_msg_var("core2_entry2_addr", (int)core2_entry2_addr);

		// copy core2 entry code into the bottom of memory
		*(volatile UINT32 *)(NVT_CORE2_START) = core2_entry2_addr;
		utl_memcpy((void *)core2_entry2_addr, _load_core2_entry_program_start_base, code2EntryCodelen);
		CPUflushWriteCache(core2_entry2_addr, code2EntryCodelen);
		//debug_msg_var("core2_entry_program", (int)_load_core2_entry_program_start_base);
		//debug_msg_var("code2EntryCodelen", (int)code2EntryCodelen);
		*(volatile UINT32 *)0xFE0B9000 = 0;
		//debug_msg_var("0xFE0B9000=", (int)*(volatile UINT32 *)0xFE0B9000);
		//reset core2
		//debug_msg("reset core2\r\n");
		bl_core2_reset();
	}

	debug_msg_var("j", p_head->CodeEntry);

	CPUCleanInvalidateDCacheAll();
	CPUInvalidateICacheAll();

    {
		typedef void (*BRANCH_CB)(void);
		BRANCH_CB p_func = (BRANCH_CB)p_head->CodeEntry;
		p_func();
	}

	return p_head->CodeEntry;
}
#endif
//---------------------------------------------------------------------------
// Static function
//---------------------------------------------------------------------------
/*
    BL_RunPartOne

    Expend loader if use compress

    @return void
*/
_THUMB2 static void BL_RunPartOne(void)
{
#if (_LOADER_COMPRESSED_ == ENABLE)
    UINT8 *in_addr = (UINT8 *)(_loader_exec_compress_load_cpu_addr);
    UINT8 *out_addr = (UINT8 *)(_loader_exec_compres_start);
    UINT32 uiCompressedOutSize;
	unsigned int out_size;

    // Get decompress size within specific header
    uiCompressedOutSize = (((UINT32)*(in_addr + 12)) << 24) +
                          (((UINT32)*(in_addr + 13)) << 16) +
                          (((UINT32)*(in_addr + 14)) <<  8) +
                          ((UINT32)*(in_addr + 15));

    // Decompress loader image
    out_size = LZ_Uncompress((unsigned char *)(in_addr + LDC_HEADER_SIZE), (unsigned char *)(out_addr), uiCompressedOutSize);

    // Need to clean data cache
    CPUInvalidateICacheAll();
    CPUCleanInvalidateDCacheAll();

    // Copy data to its execution address
	in_addr = (UINT8 *)((UINT32)_loader_exec_compres_start+(UINT32)_loader_data_cpu_addr - (UINT32)_loader_exec_end_base);
    out_addr = (UINT8 *)(_loader_data_start_base);
	out_size =(UINT32) _loader_data_size;

	//PrintDec2Hex((UINT32)in_addr);
	//PrintDec2Hex((UINT32)out_addr);
	//PrintDec2Hex((UINT32)out_size);
	utl_memcpy((void *)out_addr, in_addr, out_size);

    debug_msg("\r\nUNZOK!");
#else
    debug_msg("\r\nOK!");
#endif
}


_THUMB2 void entry(void)
{
    UINT32 reg;
#if HVYLOAD_TEST
//	UINT32 src;
//	UINT32 i;
//	UINT32 reg;
#endif
	void (*image_entry)(void) = (void *)LOAD_ADDR;

#ifndef HEAVY_LOAD
	//struct spi_flash   *spi_chip = NULL;
	//unsigned int reg;//, i;

#ifdef COMPRESS
    unsigned char *in_ptr = (unsigned char *)IMAGE_ADDR;
    unsigned char *out_ptr = (unsigned char *)LOAD_ADDR;
    unsigned int img_addr = IMAGE_ADDR;
#else
    unsigned int img_addr = LOAD_ADDR;
#endif
#endif  //HEAVY_LOAD


    // Run part one loader
    // If using compressed loader, just decompress the loader image at first!
    BL_RunPartOne();

	platform_setup(REG_PMU_BASE, REG_H2X_BASE, REG_DMAC030_BASE, REG_TMR010_BASE);
#if (_FPGA_EMULATION_ == 1 )
	debug_msg_var("\r\nperiphbase=", read_periphbase());
#endif
#ifndef USE_SCP
/*
#ifdef CONFIG_MT
    debug_msg("MT test\r\n");
#else
#ifdef CONFIG_RA
    reset_cpu(REG_WDT_BASE, 0xffffffff);
#else
    reset_cpu(REG_WDT_BASE, SYS_CLK * 160);
#endif //CONFIG_RA
#endif //CONFIG_MT
*/

    show_evb_message();

	#if (_FPGA_EMULATION_ == 0)
	cpu_change_armpll();
	#endif

	#if 0
	fLib_USB_Update_FW();
	#endif


#if (_FPGA_EMULATION_ == 0)
	//set_fire(REG_PMU_BASE);
#endif


#if (_FPGA_EMULATION_ == 0)
    //write_reg(REG_PMU_BASE + 0x20, 0x0000A001); /* DDR0 + DDR1 phy reset */
    //write_reg(REG_PMU_BASE + 0x20, 0x0000E001);
	//set_ddr0(REG_DDR0_BASE);
#endif

    //debug_msg("0xFE030000 = 0x");
    //debug_msg(Dec2HexStr(*(UINT32 *)0xFE030000));
	//debug_msg("\r\n");
	reg = *(UINT32 *)0xFE030000;
	reg = reg&0xFFFFFF3F;
	*(UINT32 *)0xFE030000 = reg;
    //debug_msg("Force bit6/7 to 0\r\n");
    //debug_msg("0xFE030000 = 0x");
    //debug_msg(Dec2HexStr(*(UINT32 *)0xFE030000));
	//debug_msg("\r\n");


#ifdef DDR1_1200_SZ_4Gb
#ifndef CONFIG_FPGA
    debug_msg("DDR1 1200 4Gb\r\n");
    set_ddr1(REG_DDR1_BASE);
#endif
#endif  //DDR1_1200_SZ_4Gb

#ifdef DDR1_1600_SZ_4Gb
#ifndef CONFIG_FPGA
    debug_msg("DDR1 1600 4Gb\r\n");
    set_ddr1(REG_DDR1_BASE);
#endif
#endif  //DDR1_1600_SZ_4Gb

#ifdef DDR1_1800_SZ_1Gb
    debug_msg("DDR1 1800 1Gb\r\n");
    set_ddr1(REG_DDR1_BASE);
#endif  //DDR1_1800_SZ_1Gb

#ifdef DDR1_1800_SZ_2Gb
    debug_msg("DDR1 1800 2Gb\r\n");
    set_ddr1(REG_DDR1_BASE);
#endif  //DDR1_1800_SZ_2Gb

#ifdef DDR1_1800_SZ_4Gb
#if (_FPGA_EMULATION_ == 0)
    debug_msg("DDR1 1800 4Gb\n");
    set_ddr1(REG_DDR1_BASE);
#endif
#endif  //DDR1_1800_SZ_4Gb


    //write_reg(REG_PMU_BASE + 0x198, 0x10); //remap

	/* modify DDR base address if user update DDR size */
	//set_bus(REG_AXIC_BASE);
#else
    heavy_load_test();
#endif  //USE_SCP

#if HVYLOAD_TEST
heavy_load_test();
#endif

#if DRAM_TEST
printf("mem r/w test ~~ \r\n");

for(i=0;i<0x100000;i=i+4)
{
    write_reg( i, i);
}

for(i=0;i<0x100000;i=i+4)
{
    src = read_reg(i);
	if(src !=i)
	{
    	printf("mem r/w failed ~~ \r\n");
        while(1);
	}
}
printf("mem r/w pass \r\n");
#endif

#if WDT_TEST_ENABLE
	//UINT32 reg;
	//if ((INREG32(0xF0020010) & 0x3) != 1)
		//debug_msg(("CPU CLKSEL != PLL8 !!\r\n"));

#if (WDT_TEST_CASE  == WDT_CASE_NORMAL)
	//gpio_setDir(Y_GPIO_Y, GPIO_DIR_OUTPUT);
	*(UINT32 *)(0xF0070024) = 1;
	//gpio_setPin(Y_GPIO_Y);
	*(UINT32 *)(0xF0070004) = 1;
	debug_msg(("Set PGPIO0 ~ \r\n"));

#elif (WDT_TEST_CASE == WDT_CASE_INTRESET)
    reg = (*(UINT32 *)(WDT_REG_BASE_ADDR+0x80))&0xFF;

	if(reg)
		debug_msg("count >0\r\n");
	else
		debug_msg("count = 0\r\n");

	enableWdtClock();
	reg = WDT_RESET_NUM0_EN |WDT_MODE_RESET | WDT_EN_MSK|WDT_KEYCTRL(WDT_KEY_VALUE);
	write_reg(WDT_CTRL_REG, reg);
	debug_msg("Wait WDT int reset\r\n");

#elif (WDT_TEST_CASE == WDT_CASE_EXTRESET)
    reg = (*(UINT32 *)(0xfe8a0000+0x80))&0xFF;
	if(reg)
		debug_msg("count >0\r\n");
	else
		debug_msg("count = 0\r\n");

	//enableWdtPCLK
   	write_reg (0xFE00005C ,0xFFFFFFFF);

	reg =*(UINT32 *)(0xfe000078);
   	write_reg (0xFE000078 ,reg&0xFFFF7FFF);

	//reg = 0x33|WDT_KEYCTRL(0x5A96);
	reg = 0x5A960033;
	write_reg(0xFE8A0000, reg);
	debug_msg("Wait WDT ext reset\r\n");
	while(1);
#endif
	while(1);
	return;
#endif



#ifdef MEM_TEST
    debug_msg("memtest after FCS\r\n");
    memtest();
	#ifdef FCS_TEST
    {
    	unsigned int i = 0;
    	while(1) {
            //KPrint("Test loop %d, ", i++);
            memtest();
    	}
    }
	#endif
#endif  //MEM_TEST



#ifdef HEAVY_LOAD
    heavy_load_test();
#endif

    //check_storage_type();

    /* Release SPI */
    //reg = read_reg(REG_PMU_BASE + 0x54);
    //reg |= (0x1 << 11);
    //write_reg(REG_PMU_BASE + 0x54, reg);

    /* Turn on SPI gating clock */
    //reg = read_reg(REG_PMU_BASE + 0x1A0);
	//#if (defined(CONFIG_MT) || defined(CONFIG_RA))
    //reg = (reg & 0xCC003FFF) | 0x22AA8000;
	//#else
    //reg = (reg & 0xCFC03FFF) | 0x202A8000;
	//#endif
    //write_reg(REG_PMU_BASE + 0x1A0, reg);

	//#ifdef CONFIG_SPI_DMA
	//Dma_init((1 << REQ_SPI020RX) | (1 << REQ_SPI020TX));
	//#endif

#if 1

    //if (flash_type == FLASH_SD) {
		debug_msg("mmc bootstrap\r\n");

		#if 0 // load rtos bin directly
	    /* sdhci init */
	    //extern int mmc_loader(unsigned int load_addr);
	    debug_msg_var("loader_addr=",img_addr);
	    unsigned int image_size = prj_main(img_addr);
	    if(image_size > 0) {
	        goto img_boot;
	    }
	    else {
	        debug_msg("no mmc\n");
	    }
    	//} else {
		#else // load all-in-one firmware
#if defined(_UBOOT_BIND_ON_)
		img_addr = bl_boot_bind_uboot();
#else
		debug_msg("load all-in-one fw\r\n");
		img_addr = bl_mainFlow();
#endif
		debug_msg_var("loader_addr=",img_addr);
		if(img_addr > 0) {
	        goto img_boot;
	    }
	    else {
	        debug_msg("no mmc\n");
	    }
		#endif
		#endif

		/* SPI init */
		//FTSPI020_Init(); //------------->TODO
        //spi_chip = 0;
        //if (!spi_chip) {
        //    debug_msg("SPI init fail! \n");
        //    goto exit_nsboot;
        //}
    //}

    //if (flash_type == FLASH_NOR) {
	//	debug_msg("FLASH_NOR TODO\n");
		/*
	    image_size = fLib_NOR_Copy_Image(img_addr, LOAD_FDT_ADDR);
	    if (image_size < 0) {
	        debug_msg("image_size < 0\n");
	        goto exit_nsboot;
	    }
	    */
    //}

    //if (flash_type == FLASH_SPI_NAND) {
	//	debug_msg("FLASH_SPI_NAND TODO\n");
		/*
	    image_size = fLib_SPI_NAND_Copy_Image(img_addr, LOAD_FDT_ADDR);
	    if (image_size < 0) {
	        debug_msg("image_size < 0\n");
	        goto exit_nsboot;
	    }
	    */
    //}

img_boot:

#if 0
#if (defined(CONFIG_MT) || defined(CONFIG_RA))
    //reset_EP();

	#if 0
	#ifdef CONFIG_MT
    /* Release UART1 */
    reg = read_reg(REG_PMU_BASE + 0x5C);
    reg |= (0x1 << 23);
    write_reg(REG_PMU_BASE + 0x5C, reg);

    reg = read_reg(REG_PMU_BASE + 0x78);
    reg &= ~(0x1 << 10);
    write_reg(REG_PMU_BASE + 0x78, reg);

    write_reg(REG_UART1_BASE + 0x8, 0x00000035);
    write_reg(REG_UART1_BASE + 0xC, 0x00000083);
    write_reg(REG_UART1_BASE + 0x0, 0x00000045);
    write_reg(REG_UART1_BASE + 0x4, 0x00000000);
    write_reg(REG_UART1_BASE + 0xC, 0x00000003);
    write_reg(REG_UART1_BASE + 0x0, 0x00000030);
    write_reg(REG_UART1_BASE + 0x0, 0x00000024);
	#endif
	#endif

	#if 0
	#ifdef CONFIG_RA
    /* Release UART2 */
    reg = read_reg(REG_PMU_BASE + 0x5C);
    reg |= (0x1 << 24);
    write_reg(REG_PMU_BASE + 0x5C, reg);

    reg = read_reg(REG_PMU_BASE + 0x78);
    reg &= ~(0x1 << 11);
    write_reg(REG_PMU_BASE + 0x78, reg);

    write_reg(REG_UART2_BASE + 0x8, 0x00000035);
    write_reg(REG_UART2_BASE + 0xC, 0x00000083);
    write_reg(REG_UART2_BASE + 0x0, 0x00000045);
    write_reg(REG_UART2_BASE + 0x4, 0x00000000);
    write_reg(REG_UART2_BASE + 0xC, 0x00000003);
    write_reg(REG_UART2_BASE + 0x0, 0x00000030);
    write_reg(REG_UART2_BASE + 0x0, 0x00000024);
	#endif
	#endif

    /* jump */
    write_reg(0x0, 0xe59ff014);
    write_reg(0x1C, LOAD_EP_ADDR);
    //write_reg(LOAD_EP_ADDR, 0xEAFFFFFE); /* loop */

    /* Reset secondary CPU */
    reg = read_reg(INTC_OSPREY_BASE + 0x4);
    reg |= (1 << 10) | (1 << 2);
    write_reg(INTC_OSPREY_BASE + 0x4, reg);
    //KPrint("addr 0x%x, data 0x%x\n", INTC_OSPREY_BASE + 0x4, reg);

    //KPrint("Jump to 0x%x\n", LOAD_ADDR);

    jmpcore1();/* LOAD_ADDR 0x0f000040 */
    //image_entry();
#endif
#endif

    //reset_EP();

    /*KPrint("write pattern\n");
    for(i = 0; i < 0x10000000; i += 4){
        write_reg(i, i);
    }
    KPrint("read compare\n");
    for(i = 0; i < 0x10000000; i += 4){
        if(i != read_reg(i)) {
            KPrint("compare fail\n");
            while(1);
        }
    }*/
    //KPrint("Boot image offset: 0x%x. Booting Image .....\n", image_size);
    //write_reg(0xF9000FF8, 0x7e9f05a3);
    //write_reg(0xF9000FFC, 0x1fe38ae7);
    CPUCleanInvalidateDCacheAll();
	CPUInvalidateICacheAll();
//    __asm__ __volatile__("mcr p15, 0, r0, c7, c1, 0");	//invalidate I-cache inner shareable
//    __asm__ __volatile__("mcr p15, 0, r0, c7, c5, 0");	//I+BTB cache invalidate

#ifdef COMPRESS
    KPrint("Uncompress from 0x%x to 0x%x, size %d\n", in_ptr, out_ptr, image_size);
    if(decompress(in_ptr, image_size, out_ptr) < 0)
        KPrint("Uncompress fail\n");
#endif

    image_entry();

//exit_nsboot:
//    return;
}
