// SPDX-License-Identifier: GPL-2.0+
/*
 * Novatek NA51089 camera board: the device tree the loader hands over,
 * and the memory split the vendor kernels expect in it.
 */

#include <common.h>
#include <env.h>
#include <fdt_support.h>
#include <init.h>
#include <lmb.h>
#include <malloc.h>
#include <openipc.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/na51089.h>
#include <linux/libfdt.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	return 0;
}

/*
 * The loader leaves its device tree, which describes the board for the
 * vendor kernels, in DDR and its address in SHMINFO. OpenIPC uImages carry
 * no device tree, so bootm passes this one (${fdtaddr}). It lies where the
 * kernel is unpacked, so it is copied away first.
 */
static void nvt_fdt_init(void)
{
	const void *shm = (const void *)NVT_SHMINFO;
	const void *fdt;
	void *copy;

	if (memcmp(shm + NVT_SHM_LD_NVT, "LD_NVT", 6)) {
		printf("FDT:   no device tree from the loader\n");
		return;
	}
	fdt = (const void *)readl(shm + NVT_SHM_FDT_ADDR);
	if (fdt_check_header(fdt)) {
		printf("FDT:   bad device tree from the loader at %p\n", fdt);
		return;
	}
	copy = malloc(fdt_totalsize(fdt));
	if (!copy)
		return;
	memcpy(copy, fdt, fdt_totalsize(fdt));
	env_set_hex("fdtaddr", (ulong)copy);
}

int misc_init_r(void)
{
	int ret = openipc_helper();

	/* After the helper's first saveenv: the address is this boot's */
	nvt_fdt_init();
	return ret;
}

/* What Linux gets of the DDR (mem=); the vendor media modules get the rest */
static ulong nvt_osmem(void)
{
	char *end;
	ulong osmem = ustrtoul(env_get("osmem") ?: "", &end, 0);

	if (!osmem || osmem > gd->ram_size)
		osmem = gd->ram_size;
	return osmem;
}

/*
 * The device tree and initrd go where Linux can reach them: everything
 * past ${osmem} is reserved. lmb refuses a reservation that overlaps one
 * it has (U-Boot's own, up to the end of DDR), so only the gaps are.
 */
void board_lmb_reserve(struct lmb *lmb)
{
	struct lmb_property rsv[8];
	phys_addr_t lo = NVT_DDR_BASE + nvt_osmem();
	phys_addr_t hi = NVT_DDR_BASE + gd->ram_size;
	unsigned long i, n = min_t(unsigned long, lmb->reserved.cnt,
				   ARRAY_SIZE(rsv));

	/* A copy: the reservations below insert into the sorted list */
	memcpy(rsv, &lmb->reserved.region[0], n * sizeof(rsv[0]));
	for (i = 0; i < n && lo < hi; i++) {
		phys_addr_t base = rsv[i].base;
		phys_addr_t end = base + rsv[i].size;

		if (end <= lo || base >= hi)
			continue;
		if (base > lo)
			lmb_reserve(lmb, lo, base - lo);
		lo = max(lo, end);
	}
	if (lo < hi)
		lmb_reserve(lmb, lo, hi - lo);
}

/*
 * As the vendor U-Boot's nvt_boot: Linux gets the first ${osmem} of the
 * DDR, and the media pool of the vendor modules (/hdal-memory/media) the
 * rest.
 */
int ft_board_setup(void *blob, struct bd_info *bd)
{
	ulong osmem = nvt_osmem();
	fdt32_t media[2];
	int node, ret;

	ret = fdt_fixup_memory(blob, NVT_DDR_BASE, osmem);
	if (ret)
		return ret;

	node = fdt_path_offset(blob, "/hdal-memory/media");
	if (node < 0)
		return 0;
	media[0] = cpu_to_fdt32(NVT_DDR_BASE + osmem);
	media[1] = cpu_to_fdt32(gd->ram_size - osmem);
	return fdt_setprop(blob, node, "reg", media, sizeof(media));
}
