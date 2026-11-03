#ifndef _BL_LOWLEVEL_INIT_H
#define _BL_LOWLEVEL_INIT_H

extern void bl_lowlevel_init(void);
extern void bl_lowlevel_core2_jump(void);
extern void bl_lowlevel_core2_setaddr(unsigned long core2_addr);
extern void bl_lowlevel_cpu_timer_init(void);
extern int bl_lowlevel_is_dtr_tunned(void);
extern unsigned long bl_lowlevel_get_fdt(void);
extern unsigned long bl_lowlevel_get_fdtfast(void);
extern int bl_lowlevel_is_fast_sensor(void);
extern int bl_lowlevel_is_flash_opened(void);
extern int bl_sensor_chgmode(unsigned char *p_fdt_hdr, int fdt_from_dram);
#endif