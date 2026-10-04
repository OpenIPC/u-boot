/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
 */
#ifndef __ASM_ARM_IO_H
#define __ASM_ARM_IO_H

#include <types.h>
#include <barriers.h>

#define __arch_getl(a)      (*(volatile unsigned int *)(a))
#define __arch_putl(v, a)   (*(volatile unsigned int *)(unsigned long)(a) = (v))

#define mb()        dsb()
#define __iormb()   dmb()
#define __iowmb()   dmb()

#ifndef writel
#define writel(v, c) ({ u32 __v = v; __iowmb(); __arch_putl(__v, c); __v; })
#endif
#ifndef readl
#define readl(c)    ({ u32 __v = __arch_getl(c); __iormb(); __v; })
#endif

#endif  /* __ASM_ARM_IO_H */
