/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Pad-mux helpers for HiSilicon/Goke SoCs.
 *
 * The tables themselves live in hi_padmux_tables.c and are ported from
 * OpenIPC ipctool (MIT, Copyright (c) 2023 OpenIPC).
 */

#ifndef __HI_PADMUX_H
#define __HI_PADMUX_H

#include <common.h>

/* Maximum number of functions a mux register selects between */
#define HI_MUX_FUNCS_MAX	9

/* One pad-mux register: its address plus offsets into the name blob for
 * the functions its selector chooses between, indexed by the selector
 * value. Offset 0 is the empty string and terminates the list. */
struct hi_muxctrl {
	u32 addr;
	u16 funcs[HI_MUX_FUNCS_MAX];
};

/* Flat GPIO numbering: gpio = bank * 8 + pin */
int hi_padmux_set_gpio(unsigned int gpio);
int hi_padmux_set_func(unsigned int gpio, const char *func);
const char *hi_padmux_get_func(unsigned int gpio);
void hi_padmux_show(unsigned int gpio);

#endif /* __HI_PADMUX_H */
