#ifndef _BL_LINUX_H
#define _BL_LINUX_H

extern int linuxboot_setup(unsigned char *p_fdt);
extern void linuxboot_go(void);
extern int linuxboot_go_info(unsigned long *p_kernel_addr, unsigned long *p_fdt_addr);
extern int system_init_timer(void);

#endif