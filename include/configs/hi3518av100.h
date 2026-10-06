#ifndef __HI3518AV100_H
#define __HI3518AV100_H

#include "hi3518ev100.h"

#undef CONFIG_HI3518EV100
#define CONFIG_HI3518AV100

#undef HISFV_RESET_GPIO_EN

/* These chips support up to 256MB of DDR */
#undef PHYS_SDRAM_1_SIZE
#define PHYS_SDRAM_1_SIZE           0x10000000

#endif /* __HI3518AV100_H */
