
/**
 * @brief This File is unable to use printf,
 * because rom api not ready at this time.
 */
#include "constant.h"
#include "fuart.h"
#include "fat.h"
#include "StorageDef.h"
#include "global.h"
#include "bl_func.h"
#include "string.h"
#include "debug.h"
#include "loader.h"
#include <stdlib.h>
#include "nand.h"
#include "nor.h"
#include "Cache.h"
#include "libfdt.h"
#include "bl_lowlevel_init.h"
#include <cfe.h>

#define CFG_FDTFAST_BUF        IPE_SRAM_BASE_ADDR
#define CFG_FDTFAST_BUF_SIZE   IPE_SRAM_BASE_SIZE

#define ALIGN_FLOOR(value, base)  ((value) & ~((base)-1))
#define ALIGN_CEIL(value, base)   ALIGN_FLOOR((value) + ((base)-1), base)

#if defined(_STORAGEINT_NandSpi_)
#define EMB_GETSTRGOBJ() nand_get_storage_object()
#elif defined(_STORAGEINT_NandEccBCH_)
#define EMB_GETSTRGOBJ() nand_bch_get_storage_object()
#elif defined(_STORAGEINT_SpiFlash_)
#define EMB_GETSTRGOBJ() nor_get_storage_object()
#elif defined(_STORAGEINT_EMMC_)
#define EMB_GETSTRGOBJ() emmc_get_storage_object()
#elif defined(_STORAGEINT_EMMC_NAND_COMBO_)
#elif defined(_STORAGEINT_NONE_)
#else
#error "unknown internal storage type"
#endif

#define _THUMB2 __attribute__((target("thumb")))

typedef enum _CFE_TYPE {
	CFE_TYPE_WRITE = 0,
	CFE_TYPE_MASK_CLEAR,
	CFE_TYPE_MASK_SET,
	CFE_TYPE_DELAY,
} CFE_TYPE;

typedef struct _CFE_CMD {
	unsigned int type;
	unsigned int addr;
	unsigned int value;
} CFE_CMD;

static volatile unsigned int *mp_core2_addr = (volatile unsigned int *)CC_CPU2_CPU1_CMDBUF_REG1;
static UINT32 g_flash_dtr = 0;
static uintptr_t g_fdtfast_addr = 0;
static uintptr_t g_fdt_addr = 0;
static UINT32 g_fast_sensor = 0;
static UINT32 g_is_flash_opened = 0;

extern void config_dma_wait_done(void);

#if (FDTFAST_SENSOR==ENABLE)
#define FDTFAST_SENSOR_CHK_STS DISABLE
#if (FDTFAST_SENSOR_CHK_STS==ENABLE)
_THUMB2 static void bl_DumpReg(UINT32 base, UINT32 size)
{
	UINT32 end = base + size;
	printf("========== DumpReg 0x%08X ==========\r\n", base);
	while (base < end) {
		printf("0x%08X : 0x%08X 0x%08X 0x%08X 0x%08X\r\n", base,
			*(volatile UINT32 *)(base + 0x0), *(volatile UINT32 *)(base + 0x4),
			*(volatile UINT32 *)(base + 0x8), *(volatile UINT32 *)(base + 0xc));
		base += 0x10;
	}
}
#define DelayT(x)           do {for(volatile int t=0; t<(x); t++);} while(0)
_THUMB2 static void bl_CheckStatus(void)
{
	OUTREG32(0xf0280018, 0xffffffff);
	OUTREG32(0xf028001c, 0xffffffff);
	bl_DumpReg(0xF0280000, 0x100);
	DelayT(100000);
	bl_DumpReg(0xF0280000, 0x100);
	bl_DumpReg(0xF0020000, 0x100);
	bl_DumpReg(0xF00D0000, 0x100);
}
#endif
_THUMB2 static int bl_parse_preset_sub_dtsi(UINT8 *pfdt_addr, INT32 node_ofst, UINT32 *param_ptr, char *prop_name)
{
	const void *pfdt_node;
	INT32 param_size;
	UINT32 *pdata = NULL;

	pfdt_node = fdt_getprop(pfdt_addr, node_ofst, prop_name, (int *)&param_size);
	if ((pfdt_node != NULL) && (param_size != 0)) {
		pdata = (UINT32 *)pfdt_node;
		*param_ptr = be32_to_cpu(*pdata);
		return 0;
	} else {
		return -1;
	}
}

_THUMB2 int bl_sensor_chgmode(unsigned char *p_fdt_hdr, int fdt_from_dram)
{
	int i, n, len;
	int i2c_id = 0;
	int nodeoffset;
	const void *nodep;
	UINT32 desc_addr;
	UINT32 desc_size;

	nodeoffset = fdt_path_offset(p_fdt_hdr, "sensor0");
	if (nodeoffset< 0) {
		debug_msg("sn: n/a\r\n");
		return 0;
	}

	nodep = fdt_getprop(p_fdt_hdr, nodeoffset, "cmd", &len);
	if (len == 0 || nodep == NULL) {
		debug_err("sn: no cmd\r\n");
		return -1;
	}
	desc_addr = (UINT32)nodep;
	desc_size = (UINT32)len;

	nodep = fdt_getprop(p_fdt_hdr, nodeoffset, "cfe", &len);
	if (len == 0 || nodep == NULL) {
		debug_err("sn: no cfe\r\n");
		return -1;
	}

	bl_parse_preset_sub_dtsi((UINT8 *)p_fdt_hdr, (INT32)nodeoffset, (UINT32 *)&i2c_id, "i2c_id");

	CFE_CMD *p_cfe = (CFE_CMD *)nodep;
	n = len / sizeof(CFE_CMD);
	cfe_open(0);
	for (i = 0; i < n; i++) {
		int type = be32_to_cpu(p_cfe->type);
		unsigned int addr = be32_to_cpu(p_cfe->addr);
		unsigned int value = be32_to_cpu(p_cfe->value);
		switch (type)
		{
		case CFE_TYPE_WRITE:
			cfe_write_reg(addr, value);
			break;
		case CFE_TYPE_MASK_CLEAR:
			cfe_mask_clear(addr, value);
			break;
		case CFE_TYPE_MASK_SET:
			cfe_mask_set(addr, value);
			break;
		case CFE_TYPE_DELAY:
			cfe_delay(value);
			break;
		default:
			debug_err_var("sn: un cmd: ", type);
			return -1;
		}
		p_cfe++;
	}

	if (fdt_from_dram) {
		utl_memcpy((void *)CFG_FDTFAST_BUF, (void *)desc_addr, desc_size);
		desc_addr = (UINT32)CFG_FDTFAST_BUF;
	}
	CPUflushWriteCache(desc_addr, desc_size);
	cfe_auto_desc(desc_addr, I2C+i2c_id);
	cfe_trig();

#if (FDTFAST_SENSOR_CHK_STS==ENABLE)
	//---- check status
	bl_CheckStatus();
#endif
	g_fast_sensor = 1;
	return 0;
}
#endif

#if (FDTFAST_FROM_DRAM != ENABLE)
_THUMB2 static int bl_lowlevel_load_fdtfast(void)
{
#if (FDTFAST_SENSOR==ENABLE)
#if (MULTI_CODE_FLOW) && !defined(_STORAGEINT_NONE_)
	PSTORAGE_OBJ strg_obj = EMB_GETSTRGOBJ();
#if defined(_STORAGEINT_NandSpi_)
	strg_obj->flash_setConfig(FLASH_CFG_ID_SPI_SUPPORT_4BITS, TRUE);
#elif defined(_STORAGEINT_EMMC_)
	//strg_obj->flash_setConfig(FLASH_CFG_ID_EMMC_SUPPORT_8BITS, TRUE);
	UINT32 high_speed = TRUE;
	strg_obj->flash_setConfig(FLASH_CFG_ID_HIGH_SPEED, high_speed);
	if (ROM_GetCurrBS() == BOOT_SOURCE_EMMC_8BIT) {
		strg_obj->flash_setConfig(FLASH_CFG_ID_EMMC_SUPPORT_8BITS, TRUE);
	} else if (ROM_GetCurrBS() == BOOT_SOURCE_EMMC_4BIT) {
		strg_obj->flash_setConfig(FLASH_CFG_ID_EMMC_SUPPORT_8BITS, FALSE);
	} else {
#if (EMMC_PINMUX_SETTING == EMMC_PINMUX_8BITS)
		strg_obj->flash_setConfig(FLASH_CFG_ID_EMMC_SUPPORT_8BITS, TRUE);
#else
		strg_obj->flash_setConfig(FLASH_CFG_ID_EMMC_SUPPORT_8BITS, FALSE);
#endif
	}
#endif
#if (_NOR_DTR_PARAM_ == ENABLE) //DTR force enable
	strg_obj->flash_installIdentifyCB(nor_identify);
#endif
#if defined(_FLASH_FREQ_MHZ_)
	strg_obj->flash_setFrequency(_FLASH_FREQ_MHZ_);
#endif

#if (_NOR_DTR_PARAM_ == ENABLE)
	strg_obj->flash_setConfig(FLASH_CFG_ID_SPI_SUPPORT_DTR, TRUE);
	g_flash_dtr = 1;
#endif

        INT32 er = strg_obj->flash_open();
	if (er < 0) {
		debug_msg("ll:flash o ng\r\n");
	} else {
		int blk_size = strg_obj->flash_getBlockSize();
		int fdtfast_iblk = FDTFAST_OFFSET / blk_size;
		unsigned char *p_tmp = (unsigned char *)CFG_FDTFAST_BUF;
#if 0// (_NOR_DTR_PARAM_ == ENABLE) auto tunning not ready yet
		if(strg_obj->flash_auto_tune_read_block) {
			if ((er = strg_obj->flash_auto_tune_read_block(fdtfast_iblk, 0x1000, p_tmp, NAND_RW_FIRMWARE)) < 0) {
				debug_msg("tunning NG\r\n");
				//bl_displayErrMsg(RWErrorMsg); // read bl31 failed
			} else {
				g_flash_dtr = 1;
				debug_msg("tunning OK\r\n");
			}
		}
#endif
		er = strg_obj->flash_readSectors(fdtfast_iblk, blk_size, p_tmp, NAND_RW_FIRMWARE);
		if (er < 0) {
			debug_err("r fdtfast NG-1\r\n");
			return -1;
		}

		g_is_flash_opened = 1;

		unsigned int fdt_tunning_offset = 0;
		unsigned int forcc = *(unsigned int *)p_tmp;
		if (forcc == DTR_TUNNING_PATTERN_DATA1 || forcc == DTR_TUNNING_PATTERN_DATA2) {
			fdt_tunning_offset += DTR_TUNNING_PATTERN_SIZE;
		}

		unsigned char *p_fdt_hdr = p_tmp + fdt_tunning_offset;

		if (fdt_check_header(p_fdt_hdr) != 0) {
			debug_msg("no fdtfast\r\n");
			return -1;
		}

		int fdtfast_size = fdt_totalsize(p_fdt_hdr);
		int total_size = ALIGN_CEIL(fdtfast_size + fdt_tunning_offset, blk_size);
		if ((int)CFG_FDTFAST_BUF_SIZE < total_size) {
			debug_err("CFG_FDTFAST_BUF_SIZE too small\r\n");
			return -1;
		}

		total_size -= blk_size;
		if (total_size > 0) {
			// read remain to get uboot starting addr on dram and offset in flash
			er = strg_obj->flash_readSectors(fdtfast_iblk + 1, total_size, p_tmp + blk_size, NAND_RW_FIRMWARE);
			if (er < 0) {
				debug_err("r fdtfast NG-2\r\n");
				return -1;
			}
		}

		if (fdt_check_full(p_fdt_hdr, fdt_totalsize(p_fdt_hdr)) != 0) {
			debug_err("chk fdtfast NG\r\n");
			return -1;
		}

		g_fdtfast_addr = (uintptr_t)p_fdt_hdr;

		#if (FDTFAST_SENSOR==ENABLE)
		if (bl_sensor_chgmode(p_fdt_hdr, 0) != 0) {
			debug_err("sen chgmode NG\r\n");
			return -1;
		}
		#endif

		return 0;
	}
#endif
#endif
	return -1;
}
#endif

_THUMB2 void bl_lowlevel_init(void)
{
	*mp_core2_addr = 0; //clean core2 jump address

#if (FDTFAST_FROM_DRAM != ENABLE)
	bl_lowlevel_load_fdtfast();
#endif
	config_dma_wait_done();
	debug_msg("bl_lowlevel_init done\r\n");
}

_THUMB2 int bl_lowlevel_is_dtr_tunned(void)
{
	return g_flash_dtr;
}


_THUMB2 int bl_lowlevel_is_fast_sensor(void)
{
	return g_fast_sensor;
}

_THUMB2 uintptr_t bl_lowlevel_get_fdtfast(void)
{
	return g_fdtfast_addr;
}

_THUMB2 uintptr_t bl_lowlevel_get_fdt(void)
{
	return g_fdt_addr;
}

_THUMB2 void bl_lowlevel_core2_jump(void)
{
	typedef void (*BRANCH_CB)(void);

	//must comes before while, because loader has been replaced then.
	bl_lowlevel_cpu_timer_init();
	while(*mp_core2_addr == 0) {
	}

	perf_mark("c2jp");

#if (DUMP_FLOW_TIMESTAMP)
	perf_dump();
#endif

//	printf("go to %08X\n", *mp_core2_addr);
	DISABLE_MMU();
	CLEAR_MMU();

	BRANCH_CB p_func = (BRANCH_CB)(*mp_core2_addr);
	p_func();
}

_THUMB2 void bl_lowlevel_core2_setaddr(unsigned long core2_addr)
{
	*mp_core2_addr = core2_addr;
}

_THUMB2 void bl_lowlevel_cpu_timer_init(void)
{
	UINT32 value = CPU_TIMER_FREQ;
	asm volatile("mcr p15, 0, %0, c14, c0, 0" : : "r"(value));
}

_THUMB2 int bl_lowlevel_is_flash_opened(void)
{
	return g_is_flash_opened;
}