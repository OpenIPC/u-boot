// SPDX-License-Identifier: GPL-2.0+
/*
 * Starting Linux on the Hi3519DV500: through the secure loader and BL31.
 *
 * U-Boot does not own EL3 on this SoC. The boot ROM leaves the GSL
 * (gsl_hi3519dv500/) waiting on the IPC mailbox: told where BL31 is, it
 * starts it, and BL31 then waits to be told where the kernel and its DTB are.
 * Jumped into directly, the kernel has nothing behind its PSCI calls: on a
 * Taobao Hi3519DV500 kit the board reset at "psci: probing for conduit method
 * from DT", and with PSCI taken out of the DTB the drivers hung one after
 * another from the MDIO probe on.
 *
 * bootm loads and prepares everything as on any other board -- kernel, ramdisk,
 * the DTB with its bootargs -- and arch/arm/lib/bootm.c hands the entry point
 * and the DTB here instead of jumping. So the saved environments of cameras
 * already out there, whose boot commands end in `bootm`, boot without change.
 *
 * The vendor SDK's U-Boot does the same with its `ipcboot` command
 * (U-Boot 2022.07, cmd/vendor/ipcboot.c). The BL31 (bl31.bin, bl31.README)
 * travels with this U-Boot:
 *
 * - NAND: in the boot partition, at 0x60000, right after the part the secure
 *   loader reads. build.sh appends it to the NAND boot image, so whatever
 *   writes that image -- `run ubwrite`, defib, sysupgrade -- writes it too.
 *   The secure loader reads U-Boot through the flash controller's boot-mode
 *   window, which ends at 0x60000 on a W25N04KV kit: an image reaching past
 *   it stops after "System startup". BL31 inside U-Boot would push it past.
 * - NOR: linked into U-Boot (bl31_blob.S).
 */

#include <common.h>
#include <bootm.h>
#include <cpu_func.h>
#include <image.h>
#include <ipc.h>
#include <malloc.h>
#include <asm/cache.h>
#include <linux/kernel.h>
#ifdef CONFIG_FMC_SPI_NAND
#include <nand.h>
#include <u-boot/crc.h>
#endif

#define BL31_ADDR	0x40010000UL	/* the GSL's BL31_BASE */
#define BL31_SIZE	0x20000UL	/* and its BL31_SIZE */

/* How long the secure loader and BL31 get to answer each message. */
#define IPC_BOOT_TIMEOUT_MS	5000

static bool bl31_ready;

#ifdef CONFIG_FMC_SPI_NAND
/* Where build.sh puts it: a header, then bl31.bin. */
#define BL31_NAND_OFFSET	0x60000	/* in the image, i.e. counting good blocks */
#define BL31_NAND_END		0xc0000	/* the end of the 768k boot partition */
#define BL31_NAND_MAGIC		0x31334c42	/* "BL31" */

struct bl31_nand_hdr {
	__le32 magic;
	__le32 len;
	__le32 crc;	/* crc32 of the len bytes after the header */
	__le32 reserved;
};

/*
 * Where the image's 0x60000 is on the chip: `nand write`, which put the image
 * there, skips bad blocks, so each one before it moves BL31 a block further.
 */
static loff_t bl31_nand_start(struct mtd_info *mtd)
{
	loff_t off = 0, good = 0;

	while (good < BL31_NAND_OFFSET) {
		if (off >= BL31_NAND_END)
			return -1;
		if (!nand_block_isbad(mtd, off))
			good += mtd->erasesize;
		off += mtd->erasesize;
	}
	return off;
}

static int bl31_load(void)
{
	struct mtd_info *mtd = get_nand_dev_by_index(0);
	const size_t hlen = sizeof(struct bl31_nand_hdr);
	size_t n = hlen + BL31_SIZE;
	struct bl31_nand_hdr *hdr;
	loff_t start;
	u32 len;
	u8 *buf;
	int ret = -1;

	if (!mtd) {
		puts("BL31: no NAND\n");
		return -1;
	}
	start = bl31_nand_start(mtd);
	if (start < 0) {
		puts("BL31: too many bad blocks in the boot partition\n");
		return -1;
	}
	buf = malloc(n);
	if (!buf)
		return -1;
	if (nand_read_skip_bad(mtd, start, &n, NULL, BL31_NAND_END - start,
			       buf)) {
		puts("BL31: cannot read it from NAND\n");
		goto out;
	}
	hdr = (struct bl31_nand_hdr *)buf;
	len = le32_to_cpu(hdr->len);
	if (le32_to_cpu(hdr->magic) != BL31_NAND_MAGIC || !len || len > BL31_SIZE) {
		printf("BL31: none at NAND 0x%llx; write a boot image that carries it\n",
		       (unsigned long long)start);
		goto out;
	}
	if (crc32(0, buf + hlen, len) != le32_to_cpu(hdr->crc)) {
		puts("BL31: CRC mismatch in NAND\n");
		goto out;
	}
	memcpy((void *)BL31_ADDR, buf + hlen, len);
	ret = 0;
out:
	free(buf);
	return ret;
}
#else
/* bl31_blob.S */
extern const char hi3519dv500_bl31_start[], hi3519dv500_bl31_end[];

static int bl31_load(void)
{
	const ulong len = hi3519dv500_bl31_end - hi3519dv500_bl31_start;

	if (len > BL31_SIZE) {
		puts("BL31: the built-in image is too large\n");
		return -1;
	}
	memcpy((void *)BL31_ADDR, hi3519dv500_bl31_start, len);
	return 0;
}
#endif

/*
 * bootm's last step before the caches go off and the jump: put BL31 where the
 * secure loader starts it from. Flash is still usable here.
 */
void board_prep_linux(struct bootm_headers *images)
{
	bl31_ready = !bl31_load();
	if (bl31_ready)
		flush_dcache_range(BL31_ADDR, BL31_ADDR + BL31_SIZE);
}

/*
 * The secure loader's SMC that parks this core (the vendor's gsl_core_pwrdwn()).
 * One call, so not through CONFIG_ARM_SMCCC, which brings U-Boot's PSCI
 * firmware driver with it. x0-x17 are the SMC calling convention's to clobber.
 */
static void gsl_smc(unsigned long fid)
{
	register unsigned long x0 asm("x0") = fid;

	asm volatile("smc #0"
		     : "+r"(x0)
		     :
		     : "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9",
		       "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17",
		       "memory");
}

static int ipc_boot_send(uint32_t cmd, ulong a, ulong b, int args)
{
	struct ipc_share_msg msg = { .cmd = cmd };

	msg.buf[msg.len++] = upper_32_bits(a);
	msg.buf[msg.len++] = lower_32_bits(a);
	if (args > 1) {
		msg.buf[msg.len++] = upper_32_bits(b);
		msg.buf[msg.len++] = lower_32_bits(b);
	}
	invalidate_dcache_all();
	if (ipc_send_msg(IPC_NODE_CORE0, &msg))
		return -1;

	for (;;) {
		struct ipc_share_msg ack;

		if (ipc_recv_msg(IPC_NODE_CORE1, &ack, IPC_BOOT_TIMEOUT_MS) < 0) {
			printf("BL31: no answer to message 0x%x\n", cmd);
			return -1;
		}
		switch (ack.cmd) {
		case IPC_CMD_ACK:
			return 0;
		case IPC_CMD_NEED_BL31:
			puts("BL31: the secure loader wants BL31 first\n");
			return -1;
		case IPC_CMD_NEED_TEEIMG:
			puts("BL31: TEE is enabled on this SoC; it wants a TEE image\n");
			return -1;
		case IPC_CMD_NEED_KERNEL:
			puts("BL31: it wants a kernel\n");
			return -1;
		default:
			printf("BL31: unexpected reply 0x%x\n", ack.cmd);
		}
	}
}

/*
 * Called by boot_jump_linux() with caches already cleaned and off, after
 * board_prep_linux() loaded BL31. Returns only when the hand-off failed.
 */
void hi3519dv500_boot_linux(ulong entry, ulong fdt)
{
	if (!bl31_ready) {
		puts("BL31: not loaded, the kernel cannot start\n");
		return;
	}
	if (ipc_boot_send(IPC_CMD_START_BL31, BL31_ADDR, 0, 1))
		return;
	if (ipc_boot_send(IPC_CMD_START_KERNEL, entry, fdt, 2))
		return;
	/* Hand this core back to the secure loader; BL31 starts the kernel. */
	gsl_smc(TEGRA_SMC_GSL_FUNCID);
	puts("BL31: the secure loader did not take the core back\n");
}
