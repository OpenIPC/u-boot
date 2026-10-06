#ifndef __HI3516CV100_H
#define __HI3516CV100_H

#include "hi3518ev100.h"

#undef CONFIG_HI3518EV100
#define CONFIG_HI3516CV100

#undef HISFV_RESET_GPIO_EN

/* These chips support up to 256MB of DDR */
#undef PHYS_SDRAM_1_SIZE
#define PHYS_SDRAM_1_SIZE           0x10000000

#endif /* __HI3516CV100_H */
