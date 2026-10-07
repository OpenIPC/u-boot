// SPDX-License-Identifier: GPL-2.0+
/*
 * Pad-mux helpers for HiSilicon/Goke SoCs.
 *
 * HiSilicon and Goke use one register per pad whose low-nibble selector
 * picks the pad's function; the tables are indexed by that selector.
 */

#include <common.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <vsprintf.h>
#include <hi_padmux.h>

/* Selector field of a pad-mux register */
#define HI_MUX_SEL_MASK		0xf

/*
 * XM72050500/XM72050510/XM72050530 share one target; their tables differ
 * only in the function list of the reg86 row, so compile both and pick one
 * from the per-board CONFIG_PRODUCT_SOC string.
 */
#ifdef CONFIG_TARGET_XM720XXX
extern const char hi_muxctrl_names_v500[];
extern const struct hi_muxctrl hi_muxctrl_table_v500[];
extern const int hi_muxctrl_count_v500;
extern const char hi_muxctrl_names_v530[];
extern const struct hi_muxctrl hi_muxctrl_table_v530[];
extern const int hi_muxctrl_count_v530;

static const struct hi_muxctrl *hi_padmux_table;
static const char *hi_padmux_names;
static int hi_padmux_count;

static void hi_padmux_select(void)
{
	if (hi_padmux_table)
		return;

#ifdef CONFIG_PRODUCT_SOC
	if (!strcmp(CONFIG_PRODUCT_SOC, "gk7205v530")) {
		hi_padmux_table = hi_muxctrl_table_v530;
		hi_padmux_names = hi_muxctrl_names_v530;
		hi_padmux_count = hi_muxctrl_count_v530;
		return;
	}
#endif
	hi_padmux_table = hi_muxctrl_table_v500;
	hi_padmux_names = hi_muxctrl_names_v500;
	hi_padmux_count = hi_muxctrl_count_v500;
}
#else
extern const char hi_muxctrl_names[];
extern const struct hi_muxctrl hi_muxctrl_table[];
extern const int hi_muxctrl_count;

#define hi_padmux_table	hi_muxctrl_table
#define hi_padmux_names	hi_muxctrl_names
#define hi_padmux_count	hi_muxctrl_count

static void hi_padmux_select(void) {}
#endif

static const char *hi_padmux_name(const struct hi_muxctrl *row, int i)
{
	return row->funcs[i] ? hi_padmux_names + row->funcs[i] : NULL;
}

static const struct hi_muxctrl *hi_padmux_find(unsigned int gpio, int *sel)
{
	char name[16];
	int i, j;

	hi_padmux_select();

	snprintf(name, sizeof(name), "GPIO%d_%d", gpio / 8, gpio % 8);

	for (i = 0; i < hi_padmux_count; i++) {
		for (j = 0; j < HI_MUX_FUNCS_MAX; j++) {
			const char *func = hi_padmux_name(&hi_padmux_table[i], j);

			if (!func)
				break;
			if (!strcmp(func, name)) {
				if (sel)
					*sel = j;
				return &hi_padmux_table[i];
			}
		}
	}

	return NULL;
}

static int hi_padmux_write(const struct hi_muxctrl *row, int sel)
{
	u32 val;

	if (sel < 0 || sel > HI_MUX_SEL_MASK)
		return -EINVAL;

	val = readl(row->addr);
	val = (val & ~HI_MUX_SEL_MASK) | sel;
	writel(val, row->addr);

	return 0;
}

int hi_padmux_set_gpio(unsigned int gpio)
{
	const struct hi_muxctrl *row;
	int sel;

	row = hi_padmux_find(gpio, &sel);
	if (!row)
		return -ENOENT;

	return hi_padmux_write(row, sel);
}

const char *hi_padmux_get_func(unsigned int gpio)
{
	const struct hi_muxctrl *row;
	int sel;

	row = hi_padmux_find(gpio, NULL);
	if (!row)
		return NULL;

	sel = readl(row->addr) & HI_MUX_SEL_MASK;

	return hi_padmux_name(row, sel);
}

int hi_padmux_set_func(unsigned int gpio, const char *func)
{
	const struct hi_muxctrl *row;
	int sel = -1;
	int i;

	row = hi_padmux_find(gpio, NULL);
	if (!row)
		return -ENOENT;

	for (i = 0; i < HI_MUX_FUNCS_MAX; i++) {
		const char *name = hi_padmux_name(row, i);

		if (!name)
			break;
		if (!strcmp(name, func)) {
			sel = i;
			break;
		}
	}

	if (sel < 0 && func[0] >= '0' && func[0] <= '9')
		sel = simple_strtoul(func, NULL, 0);

	if (sel < 0)
		return -EINVAL;

	return hi_padmux_write(row, sel);
}

void hi_padmux_show(unsigned int gpio)
{
	const struct hi_muxctrl *row;
	int sel, i;

	row = hi_padmux_find(gpio, NULL);
	if (!row) {
		printf("padmux: no mux row for GPIO%d_%d\n",
		       gpio / 8, gpio % 8);
		return;
	}

	sel = readl(row->addr) & HI_MUX_SEL_MASK;

	const char *cur = hi_padmux_name(row, sel);

	printf("GPIO%d_%d mux @0x%08x = %d (%s)\n", gpio / 8, gpio % 8,
	       row->addr, sel, cur ? cur : "?");

	for (i = 0; i < HI_MUX_FUNCS_MAX; i++) {
		const char *name = hi_padmux_name(row, i);

		if (!name)
			break;
		printf("  %d: %s\n", i, name);
	}
}
