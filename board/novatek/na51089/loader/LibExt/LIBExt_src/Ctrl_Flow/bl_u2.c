#include "constant.h"
#include "fuart.h"
#include "fat.h"
#include "StorageDef.h"
#include "global.h"
#include "nvtpack.h"
#include "dram_partition_info.h"
#include "bl_func.h"
#include "string.h"
#include "lz.h"
#include "debug.h"
#include "loader.h"
#include "emb_partition_info.h"
#include "modelext_info.h"
#include "bin_info.h"
#include "shm_info.h"
#include "libfdt.h"
#include <stdlib.h>
//#include "mmc_api.h"
#include "Cache.h"
#include "Memory.h"
#include "timer.h"
#include "lz.h"
#include "bl_u2.h"
#include "nand.h"
#include "nor.h"

#ifndef ALIGN_FLOOR
#define ALIGN_FLOOR(value, base)  ((value) & ~((base)-1))
#endif
#ifndef ALIGN_CEIL
#define ALIGN_CEIL(value, base)   ALIGN_FLOOR((value) + ((base)-1), base)
#endif

#if (USB_WRITELOADER || UART_UPDATE)

//u2: means usb and uart
#include "scsi_op.h"

typedef enum _U2_FW_TYPE {
	U2_FW_TYPE_UNKNOWN,
	U2_FW_TYPE_NVTPACK,
	U2_FW_TYPE_UBOOT,
} U2_FW_TYPE;

#define MAKEUINT32(a, b, c, d) ((UINT32)((a)&0xFF)) | (((UINT32)((b)&0xFF)) << 8) | (((UINT32)((c)&0xFF)) << 16) | (((UINT32)((d)&0xFF)) << 24)

static U2_FW_TYPE m_u2_fw_type = U2_FW_TYPE_UNKNOWN;
static unsigned int m_u2_fw_size = 0;
static unsigned char *m_fdt = NULL;
UINT32 m_tmp_addr = SDRAM_Start_FW;
static int m_uboot_status = -2;
static int m_fdt_status = -1;
static SCSIOP_FLASH_OP m_flash_op = {0};

__attribute__((target("thumb"))) static int u2_check_firmware(unsigned int addr)
{
	NVTPACK_FW_HDR2 *p_hdr = (NVTPACK_FW_HDR2 *)addr;
	unsigned int size = p_hdr->TotalSize;
	NVTPACK_MEM mem_in ;
	mem_in.p_data = (void *)addr;
	mem_in.len = size;
	// all in one bin
	if (bl_chk_valid_all_in_one(&mem_in) == 0) {
		m_u2_fw_type = U2_FW_TYPE_NVTPACK;
		m_u2_fw_size = size;
		return 0;
	}
	// uboot
	NVTPACK_BFC_HDR *pBfc = (NVTPACK_BFC_HDR *)addr; // describe compressed u-boot
	if (pBfc->uiFourCC == MAKEFOURCC('B', 'C', 'L', '1')) {
		UINT32 size_comp_le = __builtin_bswap32(pBfc->uiSizeComp);
		UINT32 addr_uncomp = ALIGN_CEIL(addr + size_comp_le, 4);
		LZ_Uncompress((UINT8 *)addr + sizeof(NVTPACK_BFC_HDR), (unsigned char *)addr_uncomp, 1024);
		addr = addr_uncomp;
	}
	HEADINFO *p_headinfo = (HEADINFO *)(addr + BIN_INFO_OFFSET_UBOOT);
	if (p_headinfo->BinLength < 0x1000000 && bl_chk_uboot(addr, p_headinfo->BinLength) == 0) {
		m_u2_fw_type = U2_FW_TYPE_UBOOT;
		m_u2_fw_size = p_headinfo->BinLength;
		if (m_fdt == NULL) {
			//check fdt followed uboot
			unsigned char *fdt = (unsigned char *)(addr + m_u2_fw_size);
			if (fdt_check_header(fdt) != 0) {
				debug_err("no fdt behind uboot\r\n");
				return -1;
			}
		}
		return 0;
	}

	return -1;
}

__attribute__((target("thumb"))) static int u2_boot_uboot(unsigned int addr, unsigned int size, U2_FW_TYPE fw_type)
{
	if (fw_type == U2_FW_TYPE_NVTPACK) {
		UINT32 comp_addr = 0;
		UINT32 comp_size = 0;
		DRAM_PARTITION *p_dram_partition = NULL;
		return bl_process_all_in_one(addr, size, &p_dram_partition, FUNC_UPDATE_FW, &comp_addr, &comp_size);
	} else {
		// update headinfo as real u-boot address
		HEADINFO *p_headinfo = (HEADINFO *)(addr + BIN_INFO_OFFSET_UBOOT);
		NVTPACK_BFC_HDR *pBfc = (NVTPACK_BFC_HDR *)addr; // describe compressed u-boot

		if (pBfc->uiFourCC == MAKEFOURCC('B', 'C', 'L', '1')) {
			UINT32 size_comp_le = __builtin_bswap32(pBfc->uiSizeComp);
			UINT32 addr_uncomp = ALIGN_CEIL(addr + size_comp_le, 4);
			LZ_Uncompress((UINT8 *)addr + sizeof(NVTPACK_BFC_HDR), (unsigned char *)addr_uncomp, 1024);
			p_headinfo = (HEADINFO *)(addr_uncomp + BIN_INFO_OFFSET_UBOOT);
		}
		// check tmp memory (file load) cannot overlap with uboot
		if ((addr < p_headinfo->CodeEntry && addr + size >  p_headinfo->CodeEntry) ||
			(addr > p_headinfo->CodeEntry && addr + size < p_headinfo->CodeEntry + size)) {
			debug_err_var("addr", addr);
			debug_err_var("size", size);
			debug_err_var("CodeEntry", p_headinfo->CodeEntry);
			debug_err("uboot & tmp_addr are overlapped\r\n");
			return -1;
		}

		if (pBfc->uiFourCC == MAKEFOURCC('B', 'C', 'L', '1')) {
			UINT32 size_comp_le = __builtin_bswap32(pBfc->uiSizeComp);
			LZ_Uncompress((UINT8 *)addr + sizeof(NVTPACK_BFC_HDR), (unsigned char *)p_headinfo->CodeEntry, size_comp_le);
		} else {
			utl_memcpy((void *)p_headinfo->CodeEntry, (void *)addr, size);
		}

		if (m_fdt != NULL) {
			bl_boot_uboot(m_fdt);
		} else {
			//check fdt followed uboot
			unsigned char *fdt = (unsigned char *)(addr + size);
			if (fdt_check_header(fdt) != 0) {
				debug_err("no fdt behind uboot\r\n");
				return -1;
			}
			utl_memcpy((void *)(p_headinfo->CodeEntry + size), fdt, fdt_totalsize(fdt));
			debug_msg_var("jump", p_headinfo->CodeEntry);
			//invalid Instruciton and data cache
			CPUCleanInvalidateDCacheAll();
			CPUInvalidateICacheAll();
			{
				typedef void (*BRANCH_CB)(void);
				BRANCH_CB p_func = (BRANCH_CB)p_headinfo->CodeEntry;
				p_func();
			}
		}
	}
	return 0;
}

__attribute__((target("thumb"))) static int u2_on_verify(unsigned int p_cmd, unsigned int *p_data, unsigned int *in_size)
{
	NVT_SCSI_CBW *p_cbw = (NVT_SCSI_CBW *)p_cmd;
	int cmd_id = p_cbw->CBWCB[1];

	switch (cmd_id) {
	case SCSIOP_OUT_MEM_WRITE:
		*p_data = MAKEUINT32(p_cbw->CBWCB[2], p_cbw->CBWCB[3], p_cbw->CBWCB[4], p_cbw->CBWCB[5]);
		*in_size = p_cbw->dCBWDataTransferLength;
		break;
	case SCSIOP_OUT_FLASH_ACCESS:
		if (p_cbw->dCBWDataTransferLength != sizeof(m_flash_op)) {
			debug_err_var("m_flash_op size not matched", *in_size);
			return -1;
		}
		*p_data = (UINT32)&m_flash_op;
		*in_size = p_cbw->dCBWDataTransferLength;
		break;
	case SCSIOP_IN_MEM_READ:
		*p_data = MAKEUINT32(p_cbw->CBWCB[2], p_cbw->CBWCB[3], p_cbw->CBWCB[4], p_cbw->CBWCB[5]);
		*in_size = p_cbw->dCBWDataTransferLength;
		//debug_msg_var("p_data:", MAKEUINT32(p_cbw->CBWCB[2], p_cbw->CBWCB[3], p_cbw->CBWCB[4], p_cbw->CBWCB[5]));
		break;
	case SCSIOP_IN_GET_TMP_ADDR:
		*p_data = (unsigned int)&m_tmp_addr;
		*in_size = sizeof(m_tmp_addr);
		break;
	case SCSIOP_IN_FDT:
		*p_data = (unsigned int)&m_fdt_status;
		*in_size = sizeof(m_fdt_status);
		if (bl_chk_fdt((unsigned int)SDRAM_Start_FW, fdt_totalsize(SDRAM_Start_FW)) == 0) {
			m_fdt_status = bl_copy_fdt_to_fdt_addr((unsigned char *)SDRAM_Start_FW, &m_fdt);
		} else {
			m_fdt_status = -1;
		}
		break;
	case SCSIOP_IN_UBOOT:
		*p_data = (unsigned int)&m_uboot_status;
		*in_size = sizeof(m_uboot_status);
		if (u2_check_firmware(SDRAM_Start_FW) == 0) {
			m_uboot_status = 0;
		} else {
			m_uboot_status = -1;
		}
		break;
	default:
		debug_err_var("unknown_cmd_id", cmd_id);
		break;
	}
	return 0;
}

__attribute__((target("thumb"))) static int u2_flash_open(unsigned int flash_type)
{
	static int is_opened[4] = {0};

	if (flash_type >= 4) {
		debug_err_var("flash_type", flash_type);
		return -1;
	}

	// open default and its related to
	if (is_opened[0] == 0) {
		if (bl_flash_open() != 0) {
			return -1;
		}
		is_opened[0] = 1;
#if !defined(_STORAGEINT_EMMC_)
		if (int_strg_obj == nand_get_storage_object()) {
			is_opened[FLASH_ACCESS_TYPE_NAND] = 1;
		}
		if (int_strg_obj == nor_get_storage_object()) {
			is_opened[FLASH_ACCESS_TYPE_NOR] = 1;
		}
#endif
	}

	if (is_opened[flash_type]) {
		return 0;
	}

#if !defined(_STORAGEINT_EMMC_)
	switch(flash_type) {
	case FLASH_ACCESS_TYPE_NAND:
#if 0
		//  User define SPI-NAND id table sample code
		nand_get_storage_object()->flash_setConfig(FLASH_CFG_ID_SPI_IDENTIFY_CB, (UINT32)nand_identify);
#endif
		nand_get_storage_object()->flash_open();
		is_opened[FLASH_ACCESS_TYPE_NAND] = 1;
		break;
	case FLASH_ACCESS_TYPE_NOR:
#if 0
		//  User define SPI-NOR id table sample code
		nor_get_storage_object()->flash_installIdentifyCB(nor_identify);
#endif
		nor_get_storage_object()->flash_open();
		is_opened[FLASH_ACCESS_TYPE_NOR] = 1;
		break;
	default:
		debug_err_var("unsupported flash_type", flash_type);
		return -1;
	}
	return 0;
#else
	debug_err("flash_type must be auto on emmc");
	return -1;
#endif
}

__attribute__((target("thumb"))) static int u2_on_vendor(unsigned int p_cmd)
{
	int er;
	PSTORAGE_OBJ p_strg = NULL;
	NVT_SCSI_CBW *p_cbw = (NVT_SCSI_CBW *)p_cmd;
	int cmd_id = p_cbw->CBWCB[1];

	switch (cmd_id) {
	case SCSIOP_OUT_MEM_WRITE:
		break;
	case SCSIOP_OUT_FLASH_ACCESS:
		if (u2_flash_open(m_flash_op.type) != 0) {
			return -1;
		}
		switch(m_flash_op.type) {
		case FLASH_ACCESS_TYPE_AUTO:
			p_strg = int_strg_obj;
			break;
#if !defined(_STORAGEINT_EMMC_)
		case FLASH_ACCESS_TYPE_NAND:
			p_strg = nand_get_storage_object();
			break;
		case FLASH_ACCESS_TYPE_NOR:
			p_strg = nor_get_storage_object();
			break;
#endif
		default:
			debug_err_var("unsupport flash_type", m_flash_op.type);
			return -1;
		}

		if (m_flash_op.is_write) {
			//write
			unsigned int block_size = p_strg->flash_getBlockSize();
			unsigned int startblk = (unsigned int)(m_flash_op.offset / block_size);
			unsigned int write_size = ALIGN_CEIL(m_flash_op.size, block_size);
			if (startblk == 0) {
				// write block 0 means update loader area
				if ((er = p_strg->flash_writePartition(startblk, write_size, (unsigned int)(m_flash_op.partition_size), (UINT8 *)SDRAM_Start_FW, NAND_RW_LOADER)) != 0) {
					debug_err_var("wr failed", er);
					return -1;
				}
			} else {
				if ((er = p_strg->flash_writePartition(startblk, write_size, (unsigned int)(m_flash_op.partition_size), (UINT8 *)SDRAM_Start_FW, NAND_RW_FIRMWARE)) != 0) {
					debug_err_var("wr failed", er);
					return -1;
				}
			}
			//Once write loader something wrong happen..
			//Can enable code bellow make sure data were program into SPI-NOR/NAND
#if 0
			else {	//Read back and make checksum
				unsigned int reload_addr;
				UINT32	status = 0;
				reload_addr = SDRAM_Start_FW + write_size;

				int_strg_obj->flash_getConfig(FLASH_CFG_ID_SPI_GET_STATUS_1,(UINT32)&status);
				debug_msg_var(
"status1", status);

				if (int_strg_obj->flash_readSectors(startblk, write_size, (UINT8 *)reload_addr, NAND_RW_LOADER) < 0) {
				     debug_err("rd fail\r\n");
				} else {
				     debug_err("rd success\r\n");
				}
				// Verify
				if (memcmp((void *)SDRAM_Start_FW, (void *)reload_addr, write_size) != 0) {
				     debug_err("verify fail\r\n");
				} else {
				    debug_msg_var("read back 0x0",*(UINT32 *)(reload_addr+0x0));
				    debug_msg_var("read back 0x4",*(UINT32 *)(reload_addr+0x4));
				    debug_msg_var("read back 0x8",*(UINT32 *)(reload_addr+0x8));
				    debug_msg_var("read back 0xc",*(UINT32 *)(reload_addr+0xc));
				}

			}
#endif
		} else {
			//read
			unsigned int block_size = p_strg->flash_getBlockSize();
			unsigned int startblk = (unsigned int)(m_flash_op.offset / block_size);
			unsigned int read_size = ALIGN_CEIL(m_flash_op.size, block_size);
			if (startblk == 0) {
				// read block 0 means update loader area
				if ((er = p_strg->flash_readSectors(startblk, read_size, (UINT8 *)SDRAM_Start_FW, NAND_RW_LOADER)) != 0) {
					debug_err_var("rd failed", er);
					return -1;
				}
			} else {
				if ((er = p_strg->flash_readSectors(startblk, read_size, (UINT8 *)SDRAM_Start_FW, NAND_RW_FIRMWARE)) != 0) {
					debug_err_var("rd failed", er);
					return -1;
				}
			}
		}
		break;
	case SCSIOP_IN_MEM_READ:
	case SCSIOP_IN_GET_TMP_ADDR:
		break;
	case SCSIOP_IN_FDT:
		break;
	case SCSIOP_IN_UBOOT:
		break;
	default:
		debug_err_var("unknown_cmd_id", cmd_id);
		break;
	}
	return 0;
}

__attribute__((target("thumb"))) int bl_usb(void)
{

	typedef int (*MSDC_Verify_CB)(unsigned int pCmdBuf, unsigned int *pDataBuf, unsigned int *IN_size);
	typedef int (*MSDC_VenDone_CB)(unsigned int pCmdBuf);
	extern void fLib_USB_Update_FW(void);
	extern MSDC_Verify_CB  guiU2MsdcCheck_cb;
	extern MSDC_VenDone_CB guiU2MsdcVendorDone_cb;
	guiU2MsdcCheck_cb = u2_on_verify;
	guiU2MsdcVendorDone_cb = u2_on_vendor;
	fLib_USB_Update_FW();
	u2_boot_uboot(SDRAM_Start_FW, m_u2_fw_size, m_u2_fw_type);
	return 0;
}

__attribute__((target("thumb"))) int bl_uart(void)
{

	typedef int  (*UART_Verify_CB)(unsigned int pCmdBuf, unsigned int *pDataBuf, unsigned int *IN_size);
	typedef int (*UART_Vendor_CB)(unsigned int pCmdBuf);
	extern void uart_upgrade_procedure(void);
	extern UART_Verify_CB guiUARTCheck_cb;
	extern UART_Vendor_CB guiUARTVendor_cb;
	guiUARTCheck_cb = u2_on_verify;
	guiUARTVendor_cb = u2_on_vendor;
	uart_upgrade_procedure();
	u2_boot_uboot(SDRAM_Start_FW, m_u2_fw_size, m_u2_fw_type);
	return 0;
}

#endif //USB_WRITELOADER
