/*
 * DDR training custom implementation for Hi3516CV200 / Hi3518EV200.
 */

#include "ddr_interface.h"
#include "ddr_training_impl.h"

#define DDRT_TEST_CLK   (CRG_REG_BASE + 0xd8)
#define DDRT_ENABLE     (MISC_REG_BASE + 0x4)

void ddr_cmd_prepare_copy(void) { return; }

void ddr_ddrt_prepare_custom(void)
{
	ddr_write(ddr_read(DDRT_TEST_CLK) | 0x8, DDRT_TEST_CLK);
	ddr_write(ddr_read(DDRT_ENABLE) | 0x4, DDRT_ENABLE);
}

void ddr_cmd_site_save(void) { return; }

void ddr_cmd_site_restore(void) { return; }
