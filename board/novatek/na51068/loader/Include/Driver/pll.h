#ifndef __PLL_H__
#define __PLL_H__

    /* 
    CPU  1200=0x027fff      0x01aaaa
    DSP   800=0x855555      0x640000
    codec 400=0x42aaaa      0x3a5555
    ddr0  466=0x4daaaa      0x42aaaa
    ddr1  466=0x42aaaa      0x42aaaa
    */
#ifdef MODE0//CPU 800, DSP 600, codec 350, ddr0 1600, ddr1 1600
	unsigned int PLL[5] = {0x01aaaa, 0x640000, 0x3a5555, 0x42aaaa, 0x42aaaa};
#endif
#ifdef MODE1
	unsigned int PLL[5] = {0x01aaaa, 0x640000, 0x3a5555, 0x42aaaa, 0x42aaaa};
#endif
#ifdef MODE2
	unsigned int PLL[5] = {0x01aaaa, 0x640000, 0x3a5555, 0x42aaaa, 0x42aaaa};
#endif
#ifdef MODE3
	unsigned int PLL[5] = {0x01aaaa, 0x640000, 0x3a5555, 0x42aaaa, 0x42aaaa};
#endif

#endif