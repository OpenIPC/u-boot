#ifndef __PACKAGE_H__
#define __PACKAGE_H__

/*
 * What differs between the GM8136, GM8136S and GM8135S packages: the DDR
 * controller table and the CPU clock. package.c is built once per package,
 * with that package's MFLAGS (Makefile); entry.c picks the one matching
 * the chip's package ID at run time.
 */
struct gm_package {
    const char *name;
    const unsigned int (*ddr_arg)[2];
    int ddr_len;
    const unsigned int (*ddr_arg_ar)[2];    /* GM8135S, chip 0x8136108 */
    int ddr_ar_len;
    unsigned int fcs_value;                 /* set_fcs(); 0: no change */
    int fcs_mul;
    int fcs_div;
    const char *mode;
};

extern const struct gm_package gm8136_package;
extern const struct gm_package gm8136s_package;
extern const struct gm_package gm8135s_package;

#endif
