#include <StorageDef.h>
#include <string.h>
#include <debug.h>
#include <nvtpack.h>
#include <emb_partition_info.h>
#include <libfdt.h>
#include <Cache.h>
#include <timer.h>
#include <crypto.h>
#include <compiler.h>
#include "bl_func.h"
#include "bl_lowlevel_init.h"
#include "bl_linux.h"
#include "arch_timer.h"
#include "modelext_parser.h"
#include "bin_info.h"
#include "shm_info.h"



#if (BOOT_LINUX_SUPPORT)

#define CFG_INDEP_RAMDISK ENABLE
#if defined(__aarch64__)
#define CFG_LINUX_START_OFFSET 0x200000 /* MIN_KIMG_ALIGN SZ_2M */
#else
#define CFG_LINUX_START_OFFSET 0x8000
#endif
#define CFG_MULTI_MKIMAGE_LEN 0x8 //mkimage's multi image always comes 8 bytes for sub-image size
#define CFG_BOOTARG_MAX_LEN 0x1000
#define CFG_LINUX_MAX_CODE_SIZE 0x01000000 /* for 64MB dram, assume linux (ro+rw+bss) limited on this size */
#define CFG_LINUX_COMP_MAX_SIZE 0xA00000 /* max uImage size */
#define CFG_RAMFS_COMP_MAX_SIZE 0x500000 /* max ramdisk size */
#define CFG_GZ_WORK_SIZE 0x10000 /* 64KB are enough */
#define CFG_CORE2_ENTRY_REG 0xF07F8000
#ifndef CONFIG_SYS_FDT_PAD
#define CONFIG_SYS_FDT_PAD 0x3000  /* from uboot */
#endif

#define MEM_PATH_LINUX "/memory"
#define MEM_PATH_LINUXTMP "/nvt_memory_cfg/linuxtmp"
#define MEM_PATH_FDT "/nvt_memory_cfg/fdt"
#define MEM_PATH_ATF "/nvt_memory_cfg/atf"

#define IS_BIN_OVERLAP(addr1, size1, addr2, size2) (!(((addr1 + size1 - 1) < addr2) || (addr1 > (addr2 + size2 - 1))))

#define _THUMB2 __attribute__((target("thumb2")))

#define ALIGN_FLOOR(value, base)  ((value) & ~((base)-1))
#define ALIGN_CEIL(value, base)   ALIGN_FLOOR((value) + ((base)-1), base)
#define ALIGN_CEIL_64(a)        ALIGN_CEIL(a, 64)   ///< Round Up to 64

#define DBG_ERR printf

// sync from uboot
#if defined(__aarch64__)
#define CONFIG_BOOTARGS_COMMON "rootwait console=ttyS0,115200 debug_boot_weak_hash print-fatal-signals=1 "
#else
#define CONFIG_BOOTARGS_COMMON "rootwait nprofile_irq_duration=on "
#endif

#define CONFIG_BOOTARGS_RAMDISK CONFIG_BOOTARGS_COMMON "root=/dev/ram0 rootfstype=ramfs rdinit=/linuxrc "
#if defined(_STORAGEINT_NandSpi_)
#define CONFIG_BOOTARGS CONFIG_BOOTARGS_COMMON "root=ubi0:rootfs rootfstype=ubifs ubi.fm_autoconvert=1 init=/linuxrc "
#elif defined(_STORAGEINT_SpiFlash_)
#define CONFIG_BOOTARGS CONFIG_BOOTARGS_COMMON "rootfstype=squashfs ro "
#elif defined(_STORAGEINT_EMMC_)
#define CONFIG_BOOTARGS CONFIG_BOOTARGS_COMMON "rootfstype=ext4 rw "
#else //SD
#define CONFIG_BOOTARGS CONFIG_BOOTARGS_COMMON "root=/dev/mmcblk0p2 noinitrd rootfstype=ext3 init=/linuxrc "
#endif /* _NVT_ROOTFS_TYPE_ */

/*
 * Legacy format image header,
 * all data in network byte order (aka natural aka bigendian).
 */
#define IH_MAGIC 0x27051956  /* Image Magic Number       */
#define IH_NMLEN 32          /* Image Name Length        */
typedef struct image_header {
	unsigned int ih_magic;   /* Image Header Magic Number    */
	unsigned int ih_hcrc;    /* Image Header CRC Checksum    */
	unsigned int ih_time;    /* Image Creation Timestamp */
	unsigned int ih_size;    /* Image Data Size      */
	unsigned int ih_load;    /* Data  Load  Address      */
	unsigned int ih_ep;      /* Entry Point Address      */
	unsigned int ih_dcrc;    /* Image Data CRC Checksum  */
	unsigned char ih_os;     /* Operating System     */
	unsigned char ih_arch;   /* CPU architecture     */
	unsigned char ih_type;   /* Image Type           */
	unsigned char ih_comp;   /* Compression Type     */
	unsigned char ih_name[IH_NMLEN];  /* Image Name       */
} image_header_t;

typedef struct _LINUX_RESERVED_MEM {
	char name[32];
	uintptr_t addr;
	unsigned long size;
} LINUX_RESERVED_MEM;

/**
 *linuxtmp address must in range of linux memory
 */
typedef struct _FDT_INFO {
	unsigned long linux_addr;
	unsigned long linux_size;
	unsigned long linuxtmp_addr;
	unsigned long linuxtmp_size;
	unsigned long fdt_addr;
	unsigned long fdt_size;
	unsigned long reserved_cnt;
	LINUX_RESERVED_MEM reserved[8];
} FDT_INFO;

typedef struct _LINUXTMP_PARTITION {
	unsigned long tmp_begin;
	unsigned long tmp_curr;
	unsigned long tmp_end;
	unsigned long fdt_addr;
	unsigned long fdt_size;
	unsigned long ramfs_addr;
	unsigned long ramfs_size;
	unsigned long bootargs_addr;
	unsigned long bootargs_size;
	unsigned long lz_linux_addr;
	unsigned long lz_linux_size;
} LINUXTMP_PARTITION;

typedef struct _LINUXBOOT_INFO {
	FDT_INFO fdt_info;
	LINUXTMP_PARTITION linuxtmp;
} LINUXBOOT_INFO;

static void *m_fdt = NULL;
static unsigned int m_timer_interval = 0;
static LINUXBOOT_INFO info = {0};

_THUMB2 static void *fdt_get_base(void)
{
	return m_fdt;
}

_THUMB2 static int fdt_read_mem_node(const char *path, unsigned long *p_addr, unsigned long *p_size)
{
	unsigned char *p_fdt = (unsigned char *)fdt_get_base();

	int len;
	int nodeoffset;
	const void *nodep;  /* property node pointer */

	// get linux space
	nodeoffset = fdt_path_offset(p_fdt, path);
	if (nodeoffset < 0) {
		DBG_ERR("failed to offset for  %s = %d \n", path, nodeoffset);
		return -1;
	}

#if defined(__aarch64__)
	if (strncmp(path, "/nvt_memory_cfg/", strlen("/nvt_memory_cfg/")) == 0) {
		nodep = fdt_getprop(p_fdt, nodeoffset, "reg", &len);
		if (len == 0 || nodep == NULL) {
			DBG_ERR("failed to access reg.\n");
			return -1;
		} else {
			unsigned int *p_data = (unsigned int *)nodep;
			*p_addr = be32_to_cpu(p_data[0]);
			*p_size = be32_to_cpu(p_data[1]);
		}
	} else if (strncmp(path, "/memory", strlen("/memory")) == 0) {
		nodep = fdt_getprop(p_fdt, nodeoffset, "reg", &len);
		if (len == 0 || nodep == NULL) {
			DBG_ERR("failed to access reg.\n");
			return -1;
		} else {
			unsigned long *p_data = (unsigned long *)nodep;
			*p_addr = be64_to_cpu(p_data[0]);
			*p_size = be64_to_cpu(p_data[1]);
		}
	} else if (strncmp(path, "/hdal-memory/", strlen("/hdal-memory/")) == 0) {
		nodep = fdt_getprop(p_fdt, nodeoffset, "reg", &len);
		if (len == 0 || nodep == NULL) {
			DBG_ERR("failed to access reg.\n");
			return -1;
		} else {
			unsigned long *p_data = (unsigned long *)nodep;
			*p_addr = be64_to_cpu(p_data[2]);
			*p_size = be64_to_cpu(p_data[3]);
		}
	} else {
		DBG_ERR("failed to parse %s\n", path);
		return -1;
	}
#else
	if (strncmp(path, "/cpus/", strlen("/cpus/")) == 0) {
		nodep = fdt_getprop(p_fdt, nodeoffset, "cpu-release-addr", &len);
		if (len == 0 || nodep == NULL) {
			DBG_ERR("failed to access cpu-release-addr.\n");
			return -1;
		} else {
			unsigned long *p_data = (unsigned long *)nodep;
			*p_addr = be32_to_cpu(p_data[0]);
		}
	} else if (strncmp(path, "/hdal-memory/", strlen("/hdal-memory/")) == 0) {
		nodep = fdt_getprop(p_fdt, nodeoffset, "reg", &len);
		if (len == 0 || nodep == NULL) {
			DBG_ERR("failed to access reg.\n");
			return -1;
		} else {
			unsigned long *p_data = (unsigned long *)nodep;
			int addr_cells = fdt_address_cells(p_fdt, nodeoffset);
			if (addr_cells == 2) {
				//new, chip_id addr size
				*p_addr = be64_to_cpu(p_data[1]);
				*p_size = be64_to_cpu(p_data[2]);
			} else {
				//new, chip_id addr size
				*p_addr = be64_to_cpu(p_data[0]);
				*p_size = be64_to_cpu(p_data[1]);
			}
		}
	} else {
		nodep = fdt_getprop(p_fdt, nodeoffset, "reg", &len);
		if (len == 0 || nodep == NULL) {
			DBG_ERR("failed to access reg.\n");
			return -1;
		} else {
			unsigned int *p_data = (unsigned int *)nodep;
			*p_addr = be32_to_cpu(p_data[0]);
			*p_size = be32_to_cpu(p_data[1]);
		}
	}
#endif
	return 0;
}


_THUMB2 static int fdt_read_reserved_mem_node(FDT_INFO *p_info)
{
	unsigned char *p_fdt = (unsigned char *)fdt_get_base();

	int len;
	int nodeoffset;
	const char path[] = "/reserved-memory";
	nodeoffset = fdt_path_offset(p_fdt, path);
	if (nodeoffset < 0) {
		DBG_ERR("failed to offset for  %s = %d \n", path, nodeoffset);
		return -1;
	}

	int child;
	fdt_for_each_subnode(child, p_fdt, nodeoffset) {
		LINUX_RESERVED_MEM *p = (LINUX_RESERVED_MEM *)&p_info->reserved[p_info->reserved_cnt];
		strncpy(p->name, fdt_get_name(p_fdt, child, &len), sizeof(p->name) - 1);
		unsigned long *p_data = (unsigned long *)fdt_getprop(p_fdt, child, "reg", &len);
		if (p_data) {
#if defined(__aarch64__)
			p->addr = be64_to_cpu(p_data[0]);
			p->size = be64_to_cpu(p_data[1]);
#else
			p->addr = be32_to_cpu(p_data[0]);
			p->size = be32_to_cpu(p_data[1]);
#endif
			p_info->reserved_cnt++;
			if (p_info->reserved_cnt > sizeof(p_info->reserved) / sizeof(p_info->reserved[0])) {
				DBG_ERR("linux_reserved_mem_cnt too large: %ld\n", p_info->reserved_cnt);
				return -1;
			}
		}

	}
	return 0;
}


_THUMB2 static int fdt_get_info(FDT_INFO *p_info)
{
	int er;
	if ((er = fdt_read_mem_node(MEM_PATH_LINUX, &p_info->linux_addr, &p_info->linux_size)) != 0) {
		return er;
	}
	if ((er = fdt_read_mem_node(MEM_PATH_LINUXTMP, &p_info->linuxtmp_addr, &p_info->linuxtmp_size)) != 0) {
		return er;
	}
	if ((er = fdt_read_mem_node(MEM_PATH_FDT, &p_info->fdt_addr, &p_info->fdt_size)) != 0) {
		return er;
	}
#if defined(__aarch64__)
	if ((er = fdt_read_mem_node(MEM_PATH_ATF, &p_info->atf_addr, &p_info->atf_size)) != 0) {
		return er;
	}
#endif
	if ((er = fdt_read_reserved_mem_node(p_info)) != 0) {
		DBG_ERR("no reversed node\r\n");
	}
	return 0;
}

_THUMB2 static int create_new_fdt(LINUXTMP_PARTITION *p_linuxtmp)
{
	unsigned char *p_fdt = (unsigned char *)fdt_get_base();

	int new_size = ALIGN_CEIL(fdt_totalsize(p_fdt) + CONFIG_SYS_FDT_PAD, 0x1000);

	p_linuxtmp->fdt_addr = p_linuxtmp->tmp_curr;
	p_linuxtmp->fdt_size = new_size;
	p_linuxtmp->tmp_curr += new_size;

	if (p_linuxtmp->tmp_curr >= p_linuxtmp->tmp_end) {
		DBG_ERR("linuxtmp memory is too small, need more %d\n", p_linuxtmp->tmp_curr - p_linuxtmp->tmp_end);
		return -1;
	}

	int err = fdt_open_into(p_fdt, (void *)p_linuxtmp->fdt_addr, new_size);
	if (err != 0) {
		DBG_ERR("fdt move failed!");
		return -1;
	}

	return 0;
}


#if (CFG_INDEP_RAMDISK)
_THUMB2 static int load_ramdisk(LINUXTMP_PARTITION *p_linuxtmp)
{
	int er = 0;
	int rootfs_partition_id = -1;
	unsigned long bfc_data = p_linuxtmp->tmp_curr;

	int blk_size = (int)int_strg_obj->flash_getBlockSize();
	EMB_PARTITION *p_emb_partition_rootfs = bl_get_rootfs_partition(fdt_get_base(), &rootfs_partition_id);

	//load 1st block
	if ((er = int_strg_obj->flash_readSectors(p_emb_partition_rootfs->PartitionOffset / blk_size, blk_size, (UINT8 *)bfc_data, NAND_RW_FIRMWARE)) < 0) {
		DBG_ERR("NG ramdisk-1\r\n");
		return -1;
	}

	NVTPACK_BFC_HDR *p_hdr = (NVTPACK_BFC_HDR *)bfc_data;

	if (p_hdr->uiFourCC == MAKEFOURCC('B', 'C', 'L', '1')) {
#if (_GZIP_ENABLE_API_ == 1)
		//bfc compressed ramdisk
		unsigned int bfc_size = be32_to_cpu(p_hdr->uiSizeComp) + sizeof(NVTPACK_BFC_HDR);
		unsigned long comp_ramfs_addr = bfc_data + sizeof(NVTPACK_BFC_HDR);
		unsigned int comp_ramfs_size = be32_to_cpu(p_hdr->uiSizeComp);

		p_linuxtmp->tmp_curr += ALIGN_CEIL_64(bfc_size);
		p_linuxtmp->ramfs_addr = p_linuxtmp->tmp_curr;
		p_linuxtmp->ramfs_size = be32_to_cpu(p_hdr->uiSizeUnComp);
		p_linuxtmp->tmp_curr = p_linuxtmp->ramfs_addr + ALIGN_CEIL_64(p_linuxtmp->ramfs_size);
		if (p_linuxtmp->tmp_curr >= p_linuxtmp->tmp_end) {
			DBG_ERR("linuxtmp memory is too small, need more %d\n", p_linuxtmp->tmp_curr - p_linuxtmp->tmp_end);
			return -1;
		}

		unsigned int remain_blocks = ALIGN_CEIL(bfc_size - blk_size, blk_size) / blk_size;
		if ((er = int_strg_obj->flash_readSectors((p_emb_partition_rootfs->PartitionOffset / blk_size) + 1, remain_blocks*blk_size, (UINT8 *)(bfc_data + blk_size), NAND_RW_FIRMWARE)) < 0) {
			DBG_ERR("NG ramdisk-2\r\n");
			return -1;
		}
		//uncompress gzip
		UINT32 algorithm = be32_to_cpu(p_hdr->uiAlgorithm) & 0xFF;
		if (algorithm != 12) {
			DBG_ERR("must gzip\r\n");
			return -1;
		}
		GZ_Uncompress((unsigned char *)comp_ramfs_addr, (unsigned char *)p_linuxtmp->ramfs_addr, comp_ramfs_size);
#else
		DBG_ERR("ungzip is not supported in this build.\r\n");
		return -1;
#endif
	} else {
		p_linuxtmp->tmp_curr += ALIGN_CEIL_64(sizeof(image_header_t));
		p_linuxtmp->ramfs_addr = p_linuxtmp->tmp_curr;

		uintptr_t  load_addr = p_linuxtmp->ramfs_addr  - sizeof(image_header_t);

		//move 1st block
		memmove((void *)(load_addr), (void *)bfc_data, blk_size);

		image_header_t *p_hdr = (image_header_t *)load_addr;
		if (be32_to_cpu(p_hdr->ih_magic) != IH_MAGIC) {
			DBG_ERR("ramdisk IH_MAGIC is wrong\r\n");
			return -1;
		}

		//IH_TYPE_RAMDISK = 3
		if (p_hdr->ih_type != 3) {
			DBG_ERR("ih_type (0x%X) != IH_TYPE_RAMDISK\r\n", (int)p_hdr->ih_type);
			return -1;
		}
		// load rest of image data
		unsigned int image_size = be32_to_cpu(p_hdr->ih_size) + sizeof(image_header_t);
		p_linuxtmp->ramfs_size = be32_to_cpu(p_hdr->ih_size);
		p_linuxtmp->tmp_curr = p_linuxtmp->ramfs_addr + ALIGN_CEIL_64(p_linuxtmp->ramfs_size);

		unsigned int remain_blocks = ALIGN_CEIL(image_size - blk_size, blk_size) / blk_size;
		if ((er = int_strg_obj->flash_readSectors((p_emb_partition_rootfs->PartitionOffset / blk_size)+1, remain_blocks*blk_size, (UINT8 *)(load_addr + blk_size), NAND_RW_FIRMWARE)) < 0) {
			DBG_ERR("NG ramdisk-2\r\n");
			return -1;
		}
	}

	return 0;
}
#endif

_THUMB2 static int load_linux(LINUXTMP_PARTITION *p_linuxtmp, FDT_INFO *p_fdt_info)
{
	int er = 0;
	int kernel_partition_id = -1;
	uintptr_t bfc_data = p_linuxtmp->tmp_curr;
	uintptr_t load_addr = p_fdt_info->linux_addr + CFG_LINUX_START_OFFSET - sizeof(image_header_t);

	int blk_size = (int)int_strg_obj->flash_getBlockSize();
	EMB_PARTITION *p_emb_partition_kernel = bl_get_kernel_partition(fdt_get_base(), &kernel_partition_id);

	if (IS_SECURE() == 0 || SECURE_DECRYPT_RTOS == 0) {
		//load 1st block
		if ((er = int_strg_obj->flash_readSectors(p_emb_partition_kernel->PartitionOffset / blk_size, blk_size, (UINT8 *)bfc_data, NAND_RW_FIRMWARE)) < 0) {
			DBG_ERR("NG kernel-1\r\n");
			return -1;
		}

		NVTPACK_BFC_HDR *p_hdr = (NVTPACK_BFC_HDR *)bfc_data;

		if (p_hdr->uiFourCC == MAKEFOURCC('B', 'C', 'L', '1')) {
#if (_GZIP_ENABLE_API_ == 1)
			//bfc compressed kernel
			unsigned int bfc_size = be32_to_cpu(p_hdr->uiSizeComp) + sizeof(NVTPACK_BFC_HDR);
			unsigned long comp_kernel_addr = bfc_data + sizeof(NVTPACK_BFC_HDR);
			unsigned int comp_kernel_size = be32_to_cpu(p_hdr->uiSizeComp);

			p_linuxtmp->tmp_curr += ALIGN_CEIL_64(bfc_size);
			p_linuxtmp->lz_linux_addr = p_linuxtmp->tmp_curr;
			p_linuxtmp->lz_linux_size = be32_to_cpu(p_hdr->uiSizeUnComp);
			p_linuxtmp->tmp_curr = p_linuxtmp->lz_linux_addr + ALIGN_CEIL_64(p_linuxtmp->lz_linux_size);
			if (p_linuxtmp->tmp_curr >= p_linuxtmp->tmp_end) {
				DBG_ERR("linuxtmp memory is too small, need more %d\n", p_linuxtmp->tmp_curr - p_linuxtmp->tmp_end);
				return -1;
			}

			// check linux overlap with nvt_memory_cfg/fdt
			if (IS_BIN_OVERLAP(load_addr, p_linuxtmp->lz_linux_size, p_fdt_info->fdt_addr, p_fdt_info->fdt_size)) {
				DBG_ERR("OVERLAP: linux(0x%lX - 0x%lX) with fdt(0x%lX - 0x%lX)\n", load_addr, load_addr + p_linuxtmp->lz_linux_size, p_fdt_info->fdt_addr, p_fdt_info->fdt_addr + p_fdt_info->fdt_size);
				return -1;
			}

			unsigned int remain_blocks = ALIGN_CEIL(bfc_size - blk_size, blk_size) / blk_size;
			if ((er = int_strg_obj->flash_readSectors((p_emb_partition_kernel->PartitionOffset / blk_size)+1, remain_blocks*blk_size, (UINT8 *)(bfc_data + blk_size), NAND_RW_FIRMWARE)) < 0) {
				DBG_ERR("NG kernel-2\r\n");
				return -1;
			}

			//uncompress gzip
			UINT32 algorithm = be32_to_cpu(p_hdr->uiAlgorithm) & 0xFF;
			if (algorithm != 12) {
				DBG_ERR("must gzip\r\n");
				return -1;
			}
			// bfc has no image_header_t
			UINT32 decoded_size = GZ_Uncompress((unsigned char *)comp_kernel_addr, (unsigned char *)(load_addr + sizeof(image_header_t)), comp_kernel_size);
			return (decoded_size >= be32_to_cpu(p_hdr->uiSizeUnComp) ) ? 0 : -1;
#else
			DBG_ERR("ungzip is not supported in this build.\r\n");
			return -1;
#endif
		} else {
			//move 1st block
			memmove((void *)(load_addr), (void *)bfc_data, blk_size);

			image_header_t *p_hdr = (image_header_t *)load_addr;
			if (be32_to_cpu(p_hdr->ih_magic) != IH_MAGIC) {
				DBG_ERR("kernel IH_MAGIC is wrong\r\n");
				return -1;
			}

#if defined(__aarch64__)
			//IH_TYPE_KERNEL = 2
			if (p_hdr->ih_type != 2) {
				DBG_ERR("ih_type (0x%X) != IH_TYPE_KERNEL\r\n", (int)p_hdr->ih_type);
				return -1;
			}
#else
			//IH_TYPE_MULTI = 4
			if (p_hdr->ih_type != 4 && p_hdr->ih_type != 2) {
				DBG_ERR("ih_type (0x%X) != IH_TYPE_MULTI or IH_TYPE_KERNEL\r\n", (int)p_hdr->ih_type);
				return -1;
			}
#endif
			if (p_hdr->ih_type == 4) {
				memmove((void *)(load_addr - CFG_MULTI_MKIMAGE_LEN), (void *)load_addr, blk_size);
				load_addr -= CFG_MULTI_MKIMAGE_LEN;
				p_hdr = (image_header_t *)load_addr;
			}
			// load rest of image data
			unsigned int image_size = be32_to_cpu(p_hdr->ih_size) + sizeof(image_header_t);

			// check linux overlap with nvt_memory_cfg/fdt
			if (IS_BIN_OVERLAP(load_addr, image_size, p_fdt_info->fdt_addr, p_fdt_info->fdt_size)) {
				DBG_ERR("OVERLAP: linux(0x%lX - 0x%lX) with fdt(0x%lX - 0x%lX)\n", load_addr, load_addr + image_size, p_fdt_info->fdt_addr, p_fdt_info->fdt_addr + p_fdt_info->fdt_size);
				return -1;
			}

			unsigned int remain_blocks = ALIGN_CEIL(image_size - blk_size, blk_size) / blk_size;
			if ((er = int_strg_obj->flash_readSectors((p_emb_partition_kernel->PartitionOffset / blk_size)+1, remain_blocks*blk_size, (UINT8 *)(load_addr + blk_size), NAND_RW_FIRMWARE)) < 0) {
				DBG_ERR("NG kernel-2\r\n");
				return -1;
			}
			return 0;
		}
	} else {
#if (SECURE_DECRYPT_KERNEL)
		//load 1st block
		if ((er = int_strg_obj->flash_readSectors(p_emb_partition_kernel->PartitionOffset / blk_size, blk_size, (UINT8 *)bfc_data, NAND_RW_FIRMWARE)) < 0) {
			DBG_ERR("NG kernel-1\r\n");
			return -1;
		}

		// load rest of image data
		int bin_size = ((HEADINFO *)(bfc_data))->BinLength;

		unsigned int remain_blocks = ALIGN_CEIL(bin_size - blk_size, blk_size) / blk_size;
		if ((er = int_strg_obj->flash_readSectors((p_emb_partition_kernel->PartitionOffset / blk_size)+1, remain_blocks*blk_size, (UINT8 *)(bfc_data + blk_size), NAND_RW_FIRMWARE)) < 0) {
			DBG_ERR("NG kernel-2\r\n");
			return -1;
		}

		//decrypt
		unsigned int plaintext_size = 0;
		if ((er = do_decrypt_aes(bfc_data, bfc_data, &plaintext_size, bin_size)) != 0) {
			debug_err_var("aes fail, er=", er);
			return er;
		}

		// uncompress
		NVTPACK_BFC_HDR *p_hdr = (NVTPACK_BFC_HDR *)bfc_data;
		if (p_hdr->uiFourCC != MAKEFOURCC('B', 'C', 'L', '1')) {
			DBG_ERR("BCL1 header is wrong\r\n");
			return -1;
		}

		//uncompress gzip
		UINT32 algorithm = be32_to_cpu(p_hdr->uiAlgorithm) & 0xFF;
		if (algorithm != 12) {
			DBG_ERR("must gzip\r\n");
			return -1;
		}
		// bfc has no image_header_t
		unsigned long comp_kernel_addr = bfc_data + sizeof(NVTPACK_BFC_HDR);
		unsigned int comp_kernel_size = be32_to_cpu(p_hdr->uiSizeComp);
		UINT32 decoded_size = GZ_Uncompress((unsigned char *)comp_kernel_addr, (unsigned char *)(load_addr + sizeof(image_header_t)), comp_kernel_size);
		return (decoded_size >= be32_to_cpu(p_hdr->uiSizeUnComp) ) ? 0 : -1;
#else
		debug_err("please ENABLE SECURE_DECRYPT_KERNEL config\n");
		return -1;
#endif
	}
	return 0;
}

_THUMB2 static int make_bootargs(LINUXTMP_PARTITION *p_linuxtmp, FDT_INFO *p_fdt_info, unsigned int bootts_begin, int ramdisk)
{
	// read extend bootarg in fdt
	unsigned char *p_fdt = (unsigned char *)fdt_get_base();
	if (p_fdt == NULL) {
		DBG_ERR("p_fdt is NULL.\n");
		return -1;
	}

	int len;
	int nodeoffset = fdt_path_offset(p_fdt, "/chosen");
	const char *bootargs = (const char *)fdt_getprop(p_fdt, nodeoffset, "bootargs", &len);

	p_linuxtmp->bootargs_addr = p_linuxtmp->tmp_curr;
	p_linuxtmp->bootargs_size = CFG_BOOTARG_MAX_LEN;
	p_linuxtmp->tmp_curr += p_linuxtmp->bootargs_size;

	if (p_linuxtmp->tmp_curr >= p_linuxtmp->tmp_end) {
		DBG_ERR("linuxtmp memory is too small, need more %d\n", p_linuxtmp->tmp_curr - p_linuxtmp->tmp_end);
		return -1;
	}

	int rootfs_partition_id = -1;
	bl_get_rootfs_partition(fdt_get_base(), &rootfs_partition_id);

	utl_memset((char *)p_linuxtmp->bootargs_addr, 0, p_linuxtmp->bootargs_size);

	if (ramdisk) {
		snprintf((char *)p_linuxtmp->bootargs_addr, CFG_BOOTARG_MAX_LEN - 1, CONFIG_BOOTARGS_RAMDISK "bootts=%u,%u user_debug=0xff %s", bootts_begin, (unsigned int)timer_getLdrElapse(), bootargs);
	} else {
#if defined(_STORAGEINT_SpiFlash_)
		snprintf((char *)p_linuxtmp->bootargs_addr, CFG_BOOTARG_MAX_LEN - 1, CONFIG_BOOTARGS "bootts=%u,%u user_debug=0xff %s root=/dev/mtdblock%d", bootts_begin, (unsigned int)timer_getLdrElapse(), bootargs, rootfs_partition_id);
#elif defined(_STORAGEINT_NandSpi_)
		snprintf((char *)p_linuxtmp->bootargs_addr, CFG_BOOTARG_MAX_LEN - 1, CONFIG_BOOTARGS "bootts=%u,%u user_debug=0xff %s ubi.mtd=%d", bootts_begin, (unsigned int)timer_getLdrElapse(), bootargs, rootfs_partition_id);
#elif defined(_STORAGEINT_EMMC_)
		snprintf((char *)p_linuxtmp->bootargs_addr, CFG_BOOTARG_MAX_LEN - 1, CONFIG_BOOTARGS "bootts=%u,%u user_debug=0xff %s", bootts_begin, (unsigned int)timer_getLdrElapse(), bootargs);
#endif
	}

#if (_NOR_DTR_PARAM_ == ENABLE)
	strncat((char *)p_linuxtmp->bootargs_addr, " flash_dtr=on", CFG_BOOTARG_MAX_LEN - 1);
#endif

	return 0;
}

_THUMB2 static int fdt_find_or_add_subnode(void *fdt, int parentoffset, const char *name)
{
	int offset;

	offset = fdt_subnode_offset(fdt, parentoffset, name);

	if (offset == -FDT_ERR_NOTFOUND) {
		offset = fdt_add_subnode(fdt, parentoffset, name);
	}

	if (offset < 0) {
		printf("%s: %s: %s\n", __func__, name, fdt_strerror(offset));
	}

	return offset;
}

_THUMB2 static int chk_mem_overlap(FDT_INFO *p_fdt_info, const char *name, uintptr_t addr, unsigned long size)
{
	int i;
	for (i = 0; i < (int)p_fdt_info->reserved_cnt; i++) {
		LINUX_RESERVED_MEM *p = (LINUX_RESERVED_MEM *)&p_fdt_info->reserved[i];
		if (IS_BIN_OVERLAP(addr, size,  p->addr, p->size)) {

			DBG_ERR("OVERLAP: %s(%lX - %lX) with %s(%lX - %lX)\n",
				name, addr, addr + size, p->name, p->addr, p->addr + p->size);
			DBG_ERR("Please move reserved memory in nvt-mem-tbl.dtsi\n");
			return -1;
		}
	}
	return 0;
}

_THUMB2 static int fdt_chosen(LINUXTMP_PARTITION *p_linuxtmp)
{
	int   nodeoffset;
	int   err;
	char  *str;     /* used to set string properties */

	void *fdt = (void *)p_linuxtmp->fdt_addr;

	err = fdt_check_header(fdt);
	if (err < 0) {
		DBG_ERR("fdt_chosen: %s\n", fdt_strerror(err));
		return err;
	}

	/* find or create "/chosen" node. */
	nodeoffset = fdt_find_or_add_subnode(fdt, 0, "chosen");
	if (nodeoffset < 0) {
		return nodeoffset;
	}

	str = (char *)p_linuxtmp->bootargs_addr;
	if (str) {
		err = fdt_setprop(fdt, nodeoffset, "bootargs", str,
				strlen(str) + 1);
		if (err < 0) {
			DBG_ERR("WARNING: could not set bootargs %s.\n",
				fdt_strerror(err));
			return err;
		}
	}

	if (p_linuxtmp->ramfs_addr) {
#if defined(__aarch64__)
		fdt64_t tmp = cpu_to_fdt64(p_linuxtmp->ramfs_addr);
#else
		fdt32_t tmp = cpu_to_fdt32(p_linuxtmp->ramfs_addr);
#endif
		err = fdt_setprop(fdt, nodeoffset, "linux,initrd-start", &tmp, sizeof(tmp));
		if (err < 0) {
			DBG_ERR("WARNING: could not set initrd-start %s.\n",
				fdt_strerror(err));
			return err;
		}

#if defined(__aarch64__)
		tmp = cpu_to_fdt64(p_linuxtmp->ramfs_addr + p_linuxtmp->ramfs_size);
#else
		tmp = cpu_to_fdt32(p_linuxtmp->ramfs_addr + p_linuxtmp->ramfs_size);
#endif
		err = fdt_setprop(fdt, nodeoffset, "linux,initrd-end", &tmp, sizeof(tmp));
		if (err < 0) {
			DBG_ERR("WARNING: could not set initrd-end %s.\n",
				fdt_strerror(err));
			return err;
		}
	}

	return 0;
}

_THUMB2 static void system_timer_stop(void)
{
	WRITE_CNTx_CTL(0);
}

_THUMB2 static  void system_timer_start(void)
{
	unsigned long long cval;
	unsigned int ctl = 0;

	cval = READ_CNTxCT() + m_timer_interval;
	//  Counter-timer Physical Timer CompareValue register 4438
	WRITE_CNTx_CVAL(cval);

	ctl  = READ_CNTx_CTL();
	ctl |= (1 << CNTP_CTL_ENABLE_SHIFT);
	ctl |= (1 << CNTP_CTL_IMASK_SHIFT); // disable timer irq
	WRITE_CNTx_CTL(ctl);
}

_THUMB2 int system_init_timer(void)
{
	unsigned int CNTKCTL;

	/* Disable clock for timer */
	OUTREG32((SYSCNT_CTRL_BASE + CNTCR_OFF), 0);

	system_timer_stop();

	// CNTKCTL, Counter-timer Kernel Control register @ arm-arm 4433
	CNTKCTL = read_cntkctl();
#if (configTIMER_NUMBER == 27) //Virtual timer
	CNTKCTL |= 0x02;
#elif(configTIMER_NUMBER == 29 || configTIMER_NUMBER == 26 || configTIMER_NUMBER == 30) //Non sec timer
	CNTKCTL |= 0x01;
#endif
	write_cntkctl(CNTKCTL);

#if defined(__aarch64__)
#if (EXPECT_EL == EXCEPTION_LEVEL_2)
	/* CNTHCTL_EL2.EL1PCEN: Set to 0 to enable traps to
	 *  Hyp mode of Non-secure EL0 and EL1 accesses to the
	 *  physical timer registers.
	 *
	 * CNTHCTL_EL2.EL1PCTEN: Set to 0 to enable traps to
	 *  Hyp mode of  Non-secure EL0 and EL1 accesses to the
	 *  physical counter registers.
	 */
	unsigned int cnthctl = read_cnthctl();
	cnthctl &= ~(0x3); //
	write_cnthctl(cnthctl);
#endif
#else
	write_cntfrq(configCPU_CLOCK_HZ);
#endif

	m_timer_interval = read_cntfrq() / configTICK_RATE_HZ;

	system_timer_start();

	/* Enable clock for timer, this is a global setting. */
	OUTREG32((SYSCNT_CTRL_BASE + CNTCR_OFF), (CNTCR_FCREQ(0) | CNTCR_HDBG | CNTCR_EN));
	return 0;
}

_THUMB2 int linuxboot_setup(unsigned char *p_fdt)
{
	int er;
	int ramdisk = 0;
	unsigned int bootts_begin = timer_getLdrElapse();

	// initialize for fdt_get_base()
	m_fdt = p_fdt;

	LINUXBOOT_INFO *p_info = &info;
	LINUXTMP_PARTITION *p_linuxtmp = &p_info->linuxtmp;
	FDT_INFO *p_fdt_info = &p_info->fdt_info;

	if ((er = fdt_get_info(p_fdt_info)) != 0) {
		return er;
	}

	p_linuxtmp->tmp_begin = p_fdt_info->linuxtmp_addr;
	p_linuxtmp->tmp_curr = p_linuxtmp->tmp_begin;
	p_linuxtmp->tmp_end = p_linuxtmp->tmp_begin + p_fdt_info->linuxtmp_size;

	// create new fdt for nodes of bootargs and ramdisk
	if ((er = create_new_fdt(p_linuxtmp)) != 0) {
		return er;
	}

	// use fdt_get_base() use new fdt address, because old one will be destroyed on load_linux()
	m_fdt = (void *)p_linuxtmp->fdt_addr;

#if (CFG_INDEP_RAMDISK)
	// load ramdisk into memory and reserved memory
	if ((er = load_ramdisk(p_linuxtmp)) == 0) {
		ramdisk = 1;
	} else {
		DBG_ERR("no ramdisk\r\n");
	}
#endif

	if ((er = load_linux(p_linuxtmp, p_fdt_info)) != 0) {
		return er;
	}

	// make bootargs
	if ((er = make_bootargs(p_linuxtmp, p_fdt_info, bootts_begin, ramdisk)) != 0) {
		return er;
	}

	// insert bootargs and ramdisk into fdt
	if ((er = fdt_chosen(p_linuxtmp)) != 0) {
		return er;
	}

	if ((er = chk_mem_overlap(p_fdt_info, "fdt", p_linuxtmp->fdt_addr, p_linuxtmp->fdt_size)) != 0) {
		return er;
	}
#if (CFG_INDEP_RAMDISK)
	if ((er = chk_mem_overlap(p_fdt_info, "ramfs", p_linuxtmp->ramfs_addr, p_linuxtmp->ramfs_size)) != 0) {
		return er;
	}
#endif
	if ((er = chk_mem_overlap(p_fdt_info, "bootargs", p_linuxtmp->bootargs_addr, p_linuxtmp->bootargs_size)) != 0) {
		return er;
	}

	return 0;
}

_THUMB2 void linuxboot_go(void)
{
	LINUXBOOT_INFO *p_info = &info;
	LINUXTMP_PARTITION *p_linuxtmp = &p_info->linuxtmp;
	FDT_INFO *p_fdt_info = &p_info->fdt_info;

#if 0
	printf("tmp_begin=0x%lX\n", p_linuxtmp->tmp_begin);
	printf("tmp_curr=0x%lX\n", p_linuxtmp->tmp_curr);
	printf("tmp_end=0x%lX\n", p_linuxtmp->tmp_end);
	printf("fdt_addr=0x%lX\n", p_linuxtmp->fdt_addr);
	printf("fdt_size=0x%lX\n", p_linuxtmp->fdt_size);
	printf("ramfs_addr=0x%lX\n", p_linuxtmp->ramfs_addr);
	printf("ramfs_size=0x%lX\n", p_linuxtmp->ramfs_size);
	printf("lz_linux_addr=0x%lX\n", p_linuxtmp->lz_linux_addr);
	printf("lz_linux_size=0x%lX\n", p_linuxtmp->lz_linux_size);
	printf("bootargs_addr=0x%lX\n", p_linuxtmp->bootargs_addr);
	printf("bootargs_size=0x%lX\n", p_linuxtmp->bootargs_size);
#endif

#if defined(__aarch64__)
	unsigned long x0 = p_linuxtmp->fdt_addr;
#else
	unsigned int r2 = p_linuxtmp->fdt_addr;
#endif
	unsigned long kernel_addr = p_fdt_info->linux_addr + CFG_LINUX_START_OFFSET;

	system_init_timer();

#if defined(__aarch64__)
	{
		void (*kernel_entry)(unsigned long fdt, unsigned long x1, unsigned long x2, unsigned long x3);
		kernel_entry = (void (*)(unsigned long, unsigned long, unsigned long, unsigned long))((kernel_addr));
		CPUCleanInvalidateDCacheAll();
		CPUInvalidateICacheAll();
		DISABLE_DCACHE();
		DISABLE_ICACHE();
		DISABLE_MMU();
		CLEAR_MMU();
		kernel_entry(x0, 0, 0, 0); // never returned
	}
#else
	// disable both irq and fiq
	__asm volatile ( "CPSID if" ::: "memory" );
	__asm volatile ( "DSB" );
	__asm volatile ( "ISB" );

	CPUCleanInvalidateDCacheAll();
	CPUInvalidateICacheAll();
	DISABLE_DCACHE();
	DISABLE_ICACHE();
	DISABLE_MMU();
	CLEAR_MMU();

	//alway enter #Mode_SVC before start linux
	__asm__("cps #0x13");
	{
		void (*kernel_entry)(int zero, int arch, unsigned int params);
		kernel_entry = (void (*)(int, int,  unsigned int))((kernel_addr));
		kernel_entry(0, 0, r2); // never returned
	}
#endif
}

_THUMB2 int linuxboot_go_info(unsigned long *p_kernel_addr, unsigned long *p_fdt_addr)
{
	LINUXBOOT_INFO *p_info = &info;
	FDT_INFO *p_fdt_info = &p_info->fdt_info;

	*p_kernel_addr = p_fdt_info->linux_addr + CFG_LINUX_START_OFFSET;
	*p_fdt_addr = p_info->linuxtmp.fdt_addr;

	return 0;
}

#endif // (BOOT_LINUX_SUPPORT)