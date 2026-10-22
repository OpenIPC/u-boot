/*
    Main control function

    This file is implement by user mode

    @file       bl_func.c

    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.
*/

#include "fuart.h"
#include "fat.h"
#include "rtc.h"
#include "timer.h"
#include "StorageDef.h"
#include "global.h"
#include "Clock.h"
#include "bl_func.h"
#include "Cache.h"
#include "string.h"
#include "lz.h"
#include "debug.h"
#include "CC.h"
#include "loader.h"
#include "nvtpack.h"
#include "dram_partition_info.h"
#include "emb_partition_info.h"
#include "modelext_parser.h"
#include "bin_info.h"
#include "shm_info.h"
#include "gic.h"
#include "libfdt.h"
#include <stdlib.h>
#include "bl_u2.h"
#include "crypto.h"
#include "nand.h"
#include "nor.h"

//#if (LOADER_TYPE == STAND_ALONE_LOADER_528) || (LOADER_TYPE == COMBINATION_528)
//static uint32_t	global_timer_freq				= 3000000;	//3MHz = 3000000 Hz
//#endif
PSTORAGE_OBJ int_strg_obj = NULL;

#define FW_PART1_SIZE_OFFSET (CODE_SECTION_OFFSET + 0x04) //ref to CodeInfo.S on rtos (addr of _section_01_size)
//#define TEE_HEADER_SIZE 0x1C // refer to: optee_header_t

#define _THUMB2 __attribute__((target("thumb2")))

#define bl_memcpy   utl_memcpy
#define bl_memset   utl_memset
#define ALIGN_FLOOR(value, base)  ((value) & ~((base)-1))
#define ALIGN_CEIL(value, base)   ALIGN_FLOOR((value) + ((base)-1), base)
#define SIZE_PRELOAD 0x400
#define SIZE_PRESERVE_USB    0x2000

#define HEADINFO_UBOOT(p_parti) ((HEADINFO *)(p_parti->uboot_addr + BIN_INFO_OFFSET_UBOOT))
#define HEADINFO_TEEOS(p_parti) ((HEADINFO *)(p_parti->teeos_addr + BIN_INFO_OFFSET_TEEOS))
#define IS_BIN_OVERLAP(addr1, size1, addr2, size2) (!(((addr1 + size1 - 1) < addr2) || (addr1 > (addr2 + size2 - 1))))

#if (STORAGE_EXT_TYPE != STORAGE_EXT_USB)
extern void set_usb_suspend(void);
#endif

#if (_ROM_PUBLIC_API_ == 1)
#define ROM_LZMA_POSITION       	0x7fd0
UINT8 lzma_temp_buffer[65536];
int (*rom_lzma_inflate)(UINT8 *in_ptr, UINT32 in_size, UINT8 *out_ptr, UINT32  out_size, UINT8 *p_tmp, UINT32 tmp_size);
#endif

extern char _loader_exec_compres_start[];
//extern char _load_nand_table_start_base[];
extern char _load_LOADER_CONFIGRAM_FREQ_PARAM_end_base[];
extern void  core1_reset(void);

// Function from Timer.c
//extern UINT32 timer_getLdrElapse(void);

// Define in MakeConfig.txt
static UINT8 UPDATE_FW_NAME[]     = {"FW98520A.BIN"};
static UINT8 UPDATE_LOADER_NAME[] = {"LD98520A.BIN"};
static UINT8 RUN_FW_NAME[]        = {"FW98520T.BIN"};
static UINT8 *LOADER_START_STR   = {(UINT8 *)"\r\nLoader Start ..."};
//#if !USB_WRITELOADER
#if (STORAGE_EXT_TYPE != STORAGE_EXT_USB)
//static UINT8 *RUN_WRKEY_NAME     = {(UINT8 *)"WRKEY.BIN"};
#if UPDATE_SIM_CODE
static UINT8 *RUN_WRKEY_NAME     = {(UINT8 *)"SIM.BIN"};
#endif
#endif
static UINT8 RECOVERY_FW_NAME[]     = {"FW98520R.BIN"};
static UINT32 g_uiPartitionID = 2;

static UINT32 g_uiVersion = 0;

// Error message
static char FWErrorMsg[]     = "\r\nFW check fail\r\n";
static char RWErrorMsg[]     = "\r\nR/W error\r\n";
static char LoaderErrorMsg[] = "\r\nLoader check fail\r\n";

UINT32 TopOfStack;
UINT32 BaseOfStack;

extern char _loader_heap_base[];

static BOOL g_is_flash_open = FALSE; // indicate flash is open for updating non-fully all-in-one bin.

static LDR_SPECIAL_KEY_CB gSpecialKeyCallBack = NULL;
static LDR_CARD_DETECT_CB gCardDetectCallBack = NULL;
static STORAGEINT gStorageIntType = STORAGEINT_UNOKNOWN;

static LDR_RECOVERY_TRIGGER_CB gRecoveryTriggerCallBack = NULL;
//#if (!USB_WRITELOADER)
//#if (STORAGE_EXT_TYPE != STORAGE_EXT_USB)
//static BOOL g_is_recovery_triggered = FALSE; // indicate recovery flow is triggered.
//#endif
_THUMB2 static int bl_is_smp(unsigned char *p_fdt);
static UINT32 g_uiStartBlkUpdateFW = StartNandBlkUpdateFW;
static UINT32 g_uiNandBlkSize = 0;
static UINT32 g_rtos_load_addr = 0;
static UINT32 g_rtos_target_addr = 0;
static UINT32 g_rtos_size = 0;

#if (FDT_SUPPORT)
#define PATH_MEM_DRAM "/nvt_memory_cfg/dram"
#define PATH_MEM_LOADER "/nvt_memory_cfg/loader"
#define PATH_MEM_UBOOT "/nvt_memory_cfg/uboot"
#define PATH_MEM_FDT "/nvt_memory_cfg/fdt"
#define PATH_MEM_SHMEM "/nvt_memory_cfg/shmem"
#define PATH_MEM_RTOS "/nvt_memory_cfg/rtos"
#define PATH_MEM_NUTTX "/nvt_memory_cfg/nuttx"
#define PATH_MEM_TEEOS "/nvt_memory_cfg/teeos"
#define PATH_MEM_HDAL "/hdal-memory/media"
#define PATH_MEM_CORE2_ENTRY1 "/nvt_memory_cfg/core2entry1"
#define PATH_MEM_CORE2_ENTRY2 "/nvt_memory_cfg/core2entry2"
#define PATH_NVT_INFO "/nvt_info"
#define PATH_NVTPACK_INDEX "nvtpack/index"
#define PATH_PARTITION_LOADER "partition_loader"
#define PATH_PARTITION_FDT "partition_fdt"
#define PATH_PARTITION_UBOOT "partition_uboot"
#define PATH_PARTITION_NUTTX "partition_nuttx"
#define PATH_PARTITION_TEEOS "partition_teeos"
#define PATH_PARTITION_RTOS "partition_rtos"
#define PROPERTY_REG "reg"
#define PROPERTY_LABEL "label"
#define PROPERTY_PARTITION_NAME "partition_name"
#define PROPERTY_NVT_LINUX_SMP "NVT_LINUX_SMP"
static DRAM_PARTITION g_dram_partition = {0};
static EMB_PARTITION g_emb_uboot = {0};
static EMB_PARTITION g_emb_teeos = {0};
static EMB_PARTITION g_emb_rtos = {0};
#else // NO FDT_SUPPORT
// refer to met-tbl.dtsi /nvt_memory_cfg
static DRAM_PARTITION g_dram_partition = {
	.dram_addr =   0x00000000,
	.dram_size =   0x20000000,
	.rev_addr =    0x00007E00, ///< shmem
	.rev_size =    0x00000200, ///< shmem
	.loader_addr = 0x01000000,
	.loader_size = 0x00800000,
	.fdt_addr    = 0x01800000,
	.fdt_size    = 0x00040000,
	.uboot_addr =  0x1E000000,
	.uboot_size =  0x01FC0000,
	.rtos_addr =   0x01840000, ///< optional, only for fast-boot,
	.rtos_size =   0x00780000, ///< optional, only for fast-boot
	.core2_entry1_addr = 0x00000000, ///< optional, only for dual core IC (fixed here, and must be)
	.core2_entry1_size = 0x00004000, ///< optional, only for dual core IC
	.core2_entry2_addr = 0x1FFC0000, ///< optional, only for dual core IC (address better in the bottom of dram to avoid memory space overlap)
	.core2_entry2_size = 0x00040000, ///< optional, only for dual core IC
};
// refer to storage-partition.dtsi
static EMB_PARTITION g_emb_uboot = {
	.PartitionOffset = 0xC0000,
	.PartitionSize   = 0xA0000,
};
static EMB_PARTITION g_emb_rtos = {  ///< optional, only for fast-boot,
	.PartitionOffset = 0x2B40000,
	.PartitionSize   = 0xA00000,
};
static EMB_PARTITION g_emb_teeos = { ///< optional, only for optee,
	.PartitionOffset = 0,
	.PartitionSize   = 0,
};
//following just for processing all-in-one bin (nvtpack)
//refer to nvtpack.dtsi
#define NVTPACK_IDX_UBOOT 3
#define NVTPACK_IDX_TEEOS -1
#define NVTPACK_IDX_RTOS  10
#endif



#if (UART_UPDATE_ == ENABLE)
static char g_strLength[80];            // buffer to store length string
#endif

//---------------------------------------------------------------------------
// Static function
//---------------------------------------------------------------------------
int bl_boot_uboot(unsigned char *p_fdt);

/**
    Display error message.

    Display error message.
    CPU loop forever and LED is red.

    @param Msg: Message to display
    @return void
*/
_THUMB2 static void bl_displayErrMsg(char *Msg)
{
	// Display error message to UART
	debug_msg(Msg);
	// Loop forever
	while (1) {
		;
	}
}


/**
    Invert endianess of input word


    @param[in] a    input word

    @return translated word
*/
_THUMB2 static UINT32 invertEndian(UINT32 a)
{
	return __builtin_bswap32(a);
}


/**
    Check FW code.

    Check FW code and file length.
    The FW binary file must be post-proecessd by encrypt_bin.exe.
    This function must sync to encrypt_bin.exe.
    If FW checking is fail, the CPU will loop forever and LED is red.

    @param[in] uiAddress    FW code in DRAM starting address
    @param[in] uiFileLen    FW code length
    @return void
*/
_THUMB2 static void bl_checkFW(UINT32 uiAddress, UINT32 uiFileLen)
{
	NVTPACK_MEM mem = {0};
	mem.p_data = (void *)uiAddress;
	mem.len = uiFileLen;
	if (nvtpack_calc_nvt_sum(&mem) != 0) {
		bl_displayErrMsg(FWErrorMsg);
	}
}

/**
    Check boot loader code.

    Check boot loader code.

    @return void
*/
_THUMB2 static void bl_checkLoader(UINT32 uiAddr, UINT32 uiSize)
{
	UINT16  *puiValue, uiSum;
	UINT32  i;

	puiValue    = (UINT16 *)uiAddr;
	uiSum       = 0;

	for (i = 0; i < (uiSize >> 1); i++) {
		uiSum += (*puiValue + i);
		puiValue++;
	}

	if ((*(UINT16 *)(uiAddr + LOADER_TAG_OFFSET) != LOADER_TAG_VALUE) ||
		(uiSum != 0)) {
		bl_displayErrMsg(LoaderErrorMsg);
	}
}
#if (SECURE_DECRYPT_UBOOT || SECURE_DECRYPT_OPTEE_OS)


_THUMB2 static void get_bininfo_size_offset(unsigned char* bininfo,unsigned int* size,unsigned int *offset)
{
	/****this headinfo is for encrypt header
	BinInfo  will set partition offset and size
	BinInfo_1[0~3]  public key offset
	BinInfo_1[4~7]  public key length
	BinInfo_2[0~3]  signature offset
	BinInfo_2[4~7]  signature length
	BinInfo_3[0~3]  encrypted offset
	BinInfo_3[4~7]  encrypted size
	BinLength       total bin size
	Checksum     total bin checksum
	others parameters  set 0
	*/
	*size = bininfo[4] | bininfo[5]<<8 | bininfo[6] << 16 | bininfo[7] << 24;
	*offset = bininfo[0] | bininfo[1] << 8 | bininfo[2] << 16 | bininfo[3] <<24;
}




_THUMB2 static int decrypt_aes_cbc(unsigned int input, unsigned int output, unsigned int len)
{

	CRYPT_OP  crypt_op_param;
	if(len &0x0f)
	{
		debug_msg_var("enc size", len);
		debug_err("not align 16\r\n");
		return -21;
	}
	if((input & 0x03) || (output & 0x03) || (len & 0x03) ){ //check word alignment
		//debug_msg_var("input", input);
		//debug_msg_var("output", output);
		//debug_msg_var("len", len);
		//debug_err("not word align\r\n");
		return -22;
	}
	crypt_op_param.op_mode = CRYPTO_CBC;
	crypt_op_param.en_de_crypt = CRYPTO_DECRYPT;
	crypt_op_param.src_addr = input;
	crypt_op_param.dst_addr = output;
	crypt_op_param.length = len;    //Need align to 16bytes align
	//debug_msg_var("src",(unsigned int)crypt_op_param.src_addr);
	//debug_msg_var("dst",(unsigned int)crypt_op_param.dst_addr);

	#if 0
	debug_msg_var("input",input);
	debug_msg_var("output",output);
	debug_msg_var("len",len);
	unsigned char *tmp = (unsigned char *) crypt_op_param.src_addr;
	int i=0;
	debug_msg_var("encryped buf addr",(unsigned int)tmp);
	for(i=0;i<16;i++){
		debug_msg_var("encryped buf",(unsigned int)tmp[i]);
	}
	#endif
	if(crypto_data_operation(EFUSE_OTP_1ST_KEY_SET_FIELD, crypt_op_param) != 0)
	{
		//debug_err("aes dec fail\r\n");
		return -23;
	}
	#if 0
	unsigned char *tmp = (unsigned char *) crypt_op_param.src_addr;
	int i=0;
	tmp = (unsigned char *) crypt_op_param.dst_addr;
	debug_msg_var("decryped addr",(unsigned int)&tmp[0]);
	for(i=0;i<16;i++){
		debug_msg_var("decryped buf",(unsigned int)tmp[i]);
	}

	#endif
	return 0;

}
#if (SECURE_SIGNATURE_BY_AES == 0)
_THUMB2 static void data_reverse(unsigned char* input_data, unsigned int size)
{
	unsigned int i=0;
	unsigned char tmp=0;
	for(i=0;i< (size/2);i++)
	{
		tmp = input_data[size - 1 - i];
		input_data[size - 1 - i] = input_data[i];
		input_data[i] = tmp;
	}

}
#endif
_THUMB2 static int do_decrypt_aes(unsigned int input_data, unsigned int output_data, unsigned int* output_size,unsigned int partition_size)
{

	unsigned int encrypt_offset =0;
	unsigned int encrypt_size =0;
	unsigned int data_size = 0;
	HEADINFO *p_headinfo = (HEADINFO *)input_data;
	int er=0;
	get_bininfo_size_offset((unsigned char *)p_headinfo->BinInfo_3, &encrypt_size, &encrypt_offset);
	data_size = p_headinfo->BinLength - encrypt_offset;
#if 1 //for signature check
	unsigned int signature_offset =0;
	unsigned int signature_size = 0;
	UINT32 hash_buf[8];
	unsigned int sha256_align_size =0;
	UINT32 * signature_buff = NULL;
	get_bininfo_size_offset((unsigned char *)p_headinfo->BinInfo_2, &signature_size, &signature_offset);
	#if (SECURE_SIGNATURE_BY_AES == 0)
		unsigned int key_offset =0;
		unsigned int key_size =0;
		unsigned int n_size=0;
		unsigned int e_size=0;
		UINT32 sign_rsa_output[64];
		UINT32 *key_n = NULL;
		UINT32 *key_e = NULL;
		UINT32 *signature_addr = NULL;
		get_bininfo_size_offset((unsigned char *)p_headinfo->BinInfo_1, &key_size, &key_offset);
		n_size = signature_size;// if rsa 2048 , signature_size should be 256 bytes
		e_size = key_size - n_size;
		key_n = (UINT32 *)(input_data + key_offset);
		// now only support rsa 2048
		key_e = (UINT32 *)(input_data + key_offset + signature_size); // signature_size should be 256 bytes, n key should be 256
		data_reverse((unsigned char *)(key_n),n_size);
		data_reverse((unsigned char *)(key_e),e_size);

		er = rsa_keycheck(key_n, 1);// check rsa n key in efuse 1 and 2 field, 0 field for aes key
		if(er == 0)
		{
			debug_err("key fail\r\n");
			return -20;
		}
		signature_addr = (UINT32 *)(input_data + signature_offset);
		data_reverse((unsigned char *)signature_addr, signature_size);

		//decrypt signature
		rsa_decrypt(signature_addr, signature_size, key_n, n_size, key_e, e_size,sign_rsa_output);

		data_reverse((unsigned char *)sign_rsa_output, signature_size);

		//remove pending data 0, the last 32 bytes will be signature data
		signature_buff = (UINT32 *)&sign_rsa_output[(sizeof(sign_rsa_output)/4 ) - (32/4 )];// offset need devided by 4, because of UINT32 type
	#else
		//UINT32 sign_aes_output[8];

		er = decrypt_aes_cbc((UINT32 )(input_data + signature_offset), output_data,signature_size);
		if(er != 0)
		{
			debug_err("decrypt_aes_cbc fail\r\n");
			return -20;
		}
		signature_buff =(UINT32 *)output_data;

	#endif
	//hash encrypt data

	if((data_size & 0x3f) != 0)
	{
		// hardware sha256 need align 64, need set pending 0
		sha256_align_size = ((data_size/0x40) + 1)*0x40;
		bl_memset((unsigned char *)(input_data + encrypt_offset + data_size),0, sha256_align_size - data_size);
	}
	else
	{
		sha256_align_size = data_size;
	}

	shahw((unsigned char *)(input_data + encrypt_offset), sha256_align_size, hash_buf);


	//compare signature and current hash
	if(memcmp((void *)hash_buf, (void *)signature_buff, sizeof(hash_buf)) != 0)
	{
		debug_err("sig fail\r\n");
		return -20;
	}
#endif

	//decrypt data
	//dma addr should 4 alignment, input_data address is 4 alignment
	if((input_data + encrypt_offset) & 0x03)
	{
		utl_memcpy((void *)input_data,(unsigned char*)(input_data + encrypt_offset), data_size);
		er = decrypt_aes_cbc(input_data, output_data, encrypt_size);
	}
	else
	{
		er = decrypt_aes_cbc(input_data + encrypt_offset, output_data, encrypt_size);
	}
	if(er != 0)
	{
		debug_err("aes fail\r\n");
		return er;
	}

	*output_size = data_size;

	//add plaintext data to output buf-->  aes should 16 alignment, if data not alignment, the last bytes we will not encrypt data
	int plaintext_size = data_size - encrypt_size;
	unsigned char *p_output = (unsigned char *)output_data;
	if( plaintext_size > 0)
	{
		utl_memcpy(&p_output[encrypt_size], (unsigned char*)(input_data + encrypt_offset+ encrypt_size),plaintext_size);
	}
	return 0;

}
#endif

//#NT#2013/04/25#Steven Wang -begin
//#NT#Show Duty calibration log
#if (_LOADER_DUTY_CALIBRATION_ == ENABLE && _LOADER_DUTY_CALIBRATION_LOG_ == ENABLE)
void BL_showROMLog(UINT32 buffer)
{
	UINT32  value;
	UINT32  index;
	UINT32  P_duty;
	UINT32  N_duty;
	UINT32  diff;

	index = 0;
	diff = 0;
	P_duty = 0;
	N_duty = 0;
	uart_putSystemUARTStr("\r\n");
	while (1) {
		value = INREG32(buffer + index);

		if ((value & 0xF0000000) == 0xF0000000) {
			value &= ~(0x40000000);
			uart_putSystemUARTStr(Dec2HexStr(value));
			uart_putSystemUARTStr(" = ");
		}

		else if ((value & 0xF0000000) == 0x50000000) {
			uart_putSystemUARTStr(Dec2HexStr(value & 0xFFFFFFF));
			uart_putSystemUARTStr("\r\n");
		}

		else if ((value & 0xF0000000) == 0x20000000) {
//            uart_putSystemUARTStr("P_duty = ");
			P_duty = value & 0xFFFFFFF;
			//uart_putSystemUARTStr(Dec2HexStr(value & 0xFFFFFFF));
			uart_putSystemUARTStr(Dec2HexStr(P_duty));
			uart_putSystemUARTStr("\r\n");
		}

		else if ((value & 0xF0000000) == 0x30000000) {
//            uart_putSystemUARTStr("N_duty = ");
			N_duty = value & 0xFFFFFFF;
			//uart_putSystemUARTStr(Dec2HexStr(value & 0xFFFFFFF));
			uart_putSystemUARTStr(Dec2HexStr(N_duty));
			uart_putSystemUARTStr("\r\n");
			if (P_duty > N_duty) {
				if (diff == 2) {
					uart_putSystemUARTStr("============\r\n");
					P_duty = 0;
					N_duty = 0;
					diff = 0;
//                    diff = 3;
				} else {
					diff = 1;
				}
			} else {
				if (diff == 1) {
					uart_putSystemUARTStr("============\r\n");
					P_duty = 0;
					N_duty = 0;
					diff = 0;
//                    diff = 3;
				} else {
					diff = 2;
				}
			}
		} else if ((value & 0xF0000000) == 0x60000000) {
			uart_putSystemUARTStr("N+P = ");
			uart_putSystemUARTStr(Dec2HexStr(value & 0xFFFFFFF));
			uart_putSystemUARTStr("\r\n");
		} else if ((value & 0xF0000000) == 0x70000000) {
			if ((value & 0xFFFFFFF) == 0x0654321) {
				break;
			}
		} else {
			uart_putSystemUARTStr("Unknow \r\n");
		}
		index += 4;
	}
	uart_putSystemUARTStr("\r\n[0xC0001000] = 0x");
	uart_putSystemUARTStr(Dec2HexStr(INREG32(0xC0001000)));
	uart_putSystemUARTStr("  [0xC000101C] = 0x");
	uart_putSystemUARTStr(Dec2HexStr(INREG32(0xC000101C)));
	uart_putSystemUARTStr("\r\n");
}
#endif
//#NT#2013/04/25#Steven Wang -end


#if (DRAM_RANGE_SCAN_EN == ENABLE)
/**
    Check Sram code.

    Check is dram scan code or not

    @param[in] uiAddress    Scan FW code in DRAM starting address
    @return void
*/
static BOOL bl_checkDramScanFW(UINT32 uiAddress)
{
	if ((*(UINT32 *)uiAddress & SRAM_TAG) == SRAM_TAG) {
		return TRUE;
	} else {
		return FALSE;
	}
}
#endif

#if 0
static BOOL bl_spiIdentify(UINT32 uiMfgID, UINT32 uiTypeID, UINT32 uiCapacityID, PSPI_IDENTIFY pIdentify)
{
	// Sample to support SST25VF032
	if ((uiMfgID == 0xBF) &&
		(uiCapacityID == 0x4A)) {
		pIdentify->bDualRead = FALSE;
		pIdentify->bSupportAAI = TRUE;
		pIdentify->bSupportEWSR = TRUE;
		pIdentify->uiFlashSize = 4 * 1024 * 1024;
		return TRUE;
	}

	return FALSE;
}
#endif

/*
    Update multi-binary image


    @param[in] uiFwBuf      buffer store fw read from card

    @return
        -@ b TRUE: success
        -@ b FALSE: fail
*/

#if !REMOVED_FLASH
#if 0
static void bl_interrupt_init(void)
{
	debug_msg("arm_gic_distif_setup\r\n");
	arm_gic_distif_setup();
	debug_msg("arm_gic_cpuif_setup\r\n");
	arm_gic_cpuif_setup();

	//
	//debug_dump_addr(0xF1500000+0x1000,0x500);
	//debug_dump_addr(0xF1500000+0x2000,0x100);
}
#endif

#if 0//(LOADER_TYPE == STAND_ALONE_LOADER_528) || (LOADER_TYPE == COMBINATION_528)
#define OUTW(addr,value)    (*(UINT32 volatile *)(addr) = (UINT32)(value))
#define INW(addr)           (*(UINT32 volatile *)(addr))


static void bl_cpu_timer_init(UINT32 value)
{
	UINT32 dwVal;
	OUTW(configARM_TIMER_BASEADDR + ARM_TIMER_LOAD_OFFSET, value);

	dwVal = INW(configARM_TIMER_BASEADDR + ARM_TIMER_CONTROL_OFFSET);
	/* Enable Auto reload mode.  */
	dwVal |= ARM_TIMER_CONTROL_AUTO_RELOAD_MASK;
	/* Clear prescaler control bits */
	dwVal &= ~ARM_TIMER_CONTROL_PRESCALER_MASK;
	/* Set prescaler value */
	dwVal |= ((CYGHWR_HAL_RTC_PRESCALER - 1) << ARM_TIMER_CONTROL_PRESCALER_SHIFT);
	/* Enable the decrementer */
	//dwVal |= ARM_TIMER_CONTROL_ENABLE_MASK;
	/* Enable the interrupt */
	dwVal |= ARM_TIMER_CONTROL_IRQ_ENABLE_MASK;

	OUTW(configARM_TIMER_BASEADDR + ARM_TIMER_CONTROL_OFFSET, dwVal);

	OUTW(configARM_TIMER_BASEADDR + ARM_TIMER_ISR_OFFSET,
                     ARM_TIMER_ISR_EVENT_FLAG_MASK);

}
#endif
#endif
_THUMB2 int bl_flash_open(void)
{
	int er;
	//UINT32  uiStorageVersion = (UINT32)&_load_nand_table_start_base;

	if (g_is_flash_open) {
		return 0;
	}
	//==========================================================================
	//=  User define SPI-NAND id table sample code							   =
	//=  You can remove // to use it										   =
	//==========================================================================
	//int_strg_obj->flash_setConfig(FLASH_CFG_ID_SPI_IDENTIFY_CB, (UINT32)nand_identify);

	//==========================================================================
	//=  User define SPI-NOR id table sample code							   =
	//=  You can remove // to use it										   =
	//==========================================================================
	//int_strg_obj->flash_installIdentifyCB(nor_identify);

	er = int_strg_obj->flash_open();
	if (er < 0) {
		debug_err("flash open fail\r\n");
		return -1;
	}

	//OUTREG32(NAND_TABLE_VERSION_ADDR, INREG32(uiStorageVersion));

	//if (er == E_OK) {
	//	OUTREG32(NAND_TABLE_FLAG_ADDR, 0x46495053); //'S''P''I''F'
	//} else { // E_OK_TABLE_FOUND(1) or E_OK_TABLE_NOT_FOUND(2)
	//	OUTREG32(NAND_TABLE_FLAG_ADDR, er);
	//}
	g_uiNandBlkSize = int_strg_obj->flash_getBlockSize();
	if (g_uiNandBlkSize == 0x10000) {
		debug_msg("SPI NOR\r\n");
		g_uiStartBlkUpdateFW = 1;
	} else if (g_uiNandBlkSize == EMMC_BLOCK_SIZE) {
		debug_msg("EMMC\r\n");
		g_uiStartBlkUpdateFW = FDT_OFFSET / EMMC_BLOCK_SIZE;
	} else {
		g_uiStartBlkUpdateFW = StartNandBlkUpdateFW;
	}

	g_is_flash_open = TRUE;
	return 0;
}

#if 0 //[-Werror=unused-function]
static int bl_flash_close(void)
{
	if (!g_is_flash_open) {
		return 0;
	}

	flash_close();
	g_is_flash_open = FALSE;
	return 0;
}
#endif

#if (FDT_SUPPORT)
_THUMB2 static const void *bl_get_fdt_property(const void *p_dtb, const char *p_path, const char *p_property, int *len)
{
	int nodeoffset; /* next node offset from libfdt */
	const void *nodep;  /* property node pointer */

	nodeoffset = fdt_path_offset(p_dtb, p_path);
	if (nodeoffset < 0) {
		return NULL;
	}
	nodep = fdt_getprop(p_dtb, nodeoffset, p_property, len);
	if (len == 0) {
		return NULL;
	}
	return nodep;
}
#endif


_THUMB2 static unsigned char *bl_get_fdt_cfg(const void *p_dtb, MODELEXT_TYPE type)
{
#if (FDT_SUPPORT)
	int  len;
	const int *nodep;

	UINT8 *p_rt = NULL;

	switch (type) {
	case MODELEXT_TYPE_DRAM_PARTITION:
		nodep = (const int *)bl_get_fdt_property(p_dtb, PATH_MEM_DRAM, PROPERTY_REG, &len);
		if (nodep == NULL) {
			debug_err("dtb get dram fail\r\n");
			return NULL;
		}
		g_dram_partition.dram_addr = be32_to_cpu(nodep[0]);
		g_dram_partition.dram_size = be32_to_cpu(nodep[1]);

		nodep = (const int *)bl_get_fdt_property(p_dtb, PATH_MEM_LOADER, PROPERTY_REG, &len);
		if (nodep == NULL) {
			debug_err("dtb get loader fail\r\n");
			return NULL;
		}
		g_dram_partition.loader_addr = be32_to_cpu(nodep[0]);
		g_dram_partition.loader_size = be32_to_cpu(nodep[1]);

		nodep = (const int *)bl_get_fdt_property(p_dtb, PATH_MEM_UBOOT, PROPERTY_REG, &len);
		if (nodep == NULL) {
			debug_err("dtb get uboot fail\r\n");
			return NULL;
		}
		g_dram_partition.uboot_addr = be32_to_cpu(nodep[0]);
		g_dram_partition.uboot_size = be32_to_cpu(nodep[1]);

		nodep = (const int *)bl_get_fdt_property(p_dtb, PATH_MEM_FDT, PROPERTY_REG, &len);
		if (nodep == NULL) {
			debug_err("dtb get fdt fail\r\n");
			return NULL;
		}
		g_dram_partition.fdt_addr = be32_to_cpu(nodep[0]);
		g_dram_partition.fdt_size = be32_to_cpu(nodep[1]);

		nodep = (const int *)bl_get_fdt_property(p_dtb, PATH_MEM_RTOS, PROPERTY_REG, &len);
		if (nodep != NULL) {
			g_dram_partition.rtos_addr = be32_to_cpu(nodep[0]);
			g_dram_partition.rtos_size = be32_to_cpu(nodep[1]);
		}

		nodep = (const int *)bl_get_fdt_property(p_dtb, PATH_MEM_NUTTX, PROPERTY_REG, &len);
		if (nodep != NULL) {
			g_dram_partition.nuttx_addr = be32_to_cpu(nodep[0]);
			g_dram_partition.nuttx_size = be32_to_cpu(nodep[1]);
		}

		nodep = (const int *)bl_get_fdt_property(p_dtb, PATH_MEM_TEEOS, PROPERTY_REG, &len);
		if (nodep != NULL) {
			g_dram_partition.teeos_addr = be32_to_cpu(nodep[0]);
			g_dram_partition.teeos_size = be32_to_cpu(nodep[1]);
		}

		nodep = (const int *)bl_get_fdt_property(p_dtb, PATH_MEM_HDAL, PROPERTY_REG, &len);
		if (nodep == NULL) {
			debug_err("dtb get hdal fail\r\n");
			return NULL;
		}
		g_dram_partition.hdal1_addr = be32_to_cpu(nodep[0]);
		g_dram_partition.hdal1_size = be32_to_cpu(nodep[1]);
		if (len >= 16) {
			g_dram_partition.hdal2_addr = be32_to_cpu(nodep[2]);
			g_dram_partition.hdal2_size = be32_to_cpu(nodep[3]);
		}

#if 0//(LOADER_TYPE == STAND_ALONE_LOADER_528) || (LOADER_TYPE == COMBINATION_528)
		nodep = (const int *)bl_get_fdt_property(p_dtb, PATH_MEM_CORE2_ENTRY1, PROPERTY_REG, &len);
		if (nodep != NULL) {
			g_dram_partition.core2_entry1_addr = be32_to_cpu(nodep[0]);
			g_dram_partition.core2_entry1_size = be32_to_cpu(nodep[1]);
		}

		nodep = (const int *)bl_get_fdt_property(p_dtb, PATH_MEM_CORE2_ENTRY2, PROPERTY_REG, &len);
		if (nodep != NULL) {
			g_dram_partition.core2_entry2_addr = be32_to_cpu(nodep[0]);
			g_dram_partition.core2_entry2_size = be32_to_cpu(nodep[1]);
		}
#endif
		p_rt = (UINT8 *)&g_dram_partition;
		break;

	case MODELEXT_TYPE_BIN_INFO:
		nodep = (const int *)bl_get_fdt_property(p_dtb, PATH_MEM_SHMEM, PROPERTY_REG, &len);
		if (nodep == NULL) {
			debug_err("dtb get shmem fail\r\n");
			return NULL;
		}

		if (sizeof(SHMINFO) > be32_to_cpu(nodep[1])) {
			debug_err("shmem size mismatch\r\n");
			return NULL;
		}

		p_rt = (UINT8 *)be32_to_cpu(nodep[0]);
		break;

	default:
		debug_err_var("not handle type\r\n", type);
		return NULL;
	}
	return p_rt;
#else // NO FDT_SUPPORT
	switch (type) {
	case MODELEXT_TYPE_DRAM_PARTITION:
		return (UINT8 *)&g_dram_partition;
	case MODELEXT_TYPE_BIN_INFO:
		return (UINT8 *)g_dram_partition.rev_addr;
	default:
		return NULL;
	}
#endif
}

_THUMB2 int bl_chk_valid_all_in_one(NVTPACK_MEM *p_mem_all_in_one)
{
	NVTPACK_ER er;
	NVTPACK_VER ver;

	er = nvtpack_getver(p_mem_all_in_one, &ver);

	if (er != NVTPACK_ER_SUCCESS) {
		return -1;
	}

	if (ver == NVTPACK_VER_16072017) {
		return 0;
	}

	return -1;
}


#if !REMOVED_FLASH
_THUMB2 int bl_chk_fdt(unsigned int addr, unsigned int size)
{
	DRAM_PARTITION *p_dram_partition = NULL;

	if (fdt_check_full((void *)addr, size) != 0) {
		debug_err("invalid dtb\r\n");
		return -1;
	}

	p_dram_partition = (DRAM_PARTITION *)bl_get_fdt_cfg((const void *)addr, MODELEXT_TYPE_DRAM_PARTITION);

	if (p_dram_partition == NULL) {
		debug_err("invalid dram_partition\r\n");
		return -1;
	}

	// check tmp memory (file load) cannot overlap with uboot and teeos
	// we allow to overlap with linux, root-fs because uboot will reload bin file later
	debug_msg_var("tmp_addr", SDRAM_Start_FW);

//	if ((addr < p_dram_partition->uboot_addr && addr + size > p_dram_partition->uboot_addr) ||
//		(addr > p_dram_partition->uboot_addr && addr + size < p_dram_partition->uboot_addr + p_dram_partition->uboot_size)) {
    if (IS_BIN_OVERLAP(addr, size, p_dram_partition->uboot_addr, p_dram_partition->uboot_size)) {
		debug_err_var("a", addr);
		debug_err_var("s", size);
		debug_err("uboot olp\r\n");
		return -1;
	}
#if 1
	//if ((addr < p_dram_partition->teeos_addr && addr + size > p_dram_partition->teeos_addr) ||
	//	(addr > p_dram_partition->teeos_addr && addr + size < p_dram_partition->teeos_addr + p_dram_partition->teeos_size)) {
    if (p_dram_partition->teeos_size) {
		if (IS_BIN_OVERLAP(addr, size, p_dram_partition->teeos_addr, p_dram_partition->teeos_size)) {
			debug_err_var("a", addr);
			debug_err_var("s", size);
			debug_err("teeos olp\r\n");
			return -1;
		}
	}
#endif
	// check if memory size matched real size
	UINT32 ld_dram1_size = dma_get_dram_capacity(DMA_ID_1);
	if (p_dram_partition->dram_size > ld_dram1_size) {
		debug_err_var("fw_d1_s", p_dram_partition->dram_size);
		debug_err_var("ld_d1_s", ld_dram1_size);
		debug_err("d1 s not matched.\r\n");
		return -1;
	}
	UINT32 ld_dram2_size = dma_get_dram_capacity(DMA_ID_2);
	if (p_dram_partition->hdal2_size > ld_dram2_size) {
		debug_err_var("fw_d2_s", p_dram_partition->hdal2_size);
		debug_err_var("ld_d2_s", ld_dram2_size);
		debug_err("d2 s not matched.\r\n");
		return -1;
	}
	return 0;
}
#endif

#if !REMOVED_FLASH
_THUMB2 int bl_chk_uboot(unsigned int addr, unsigned int size)
{
#if (_FPGA_EMULATION_ == 0)
	NVTPACK_MEM mem = {0};
	mem.p_data = (void *)addr;
	mem.len = size;
	if (nvtpack_calc_nvt_sum(&mem) != 0) {
		debug_err("uboot check sum fail");
		return -1;
	}
#endif
	return 0;
}

#if 0//(LOADER_TYPE == STAND_ALONE_LOADER_528) || (LOADER_TYPE == COMBINATION_528)
static void bl_core2_reset(void)
{
	UINT32 core_reg;

	debug_msg("core2_reset\r\n");

	core_reg = *(volatile UINT32 *)0xF0E400F0;
	core_reg &= ~(1<<3);
	core_reg &= ~(1<<11);
	*(volatile UINT32 *)0xF0E400F0 = core_reg;
	core_reg |= (1<<2)|(1<<10) | (1<<21);
	core_reg |= (1<<17);
	*(volatile UINT32 *)0xF0E400F0 = core_reg;
}

static void bl_core2_prepare(DRAM_PARTITION *p_dram_partition, unsigned int last_addr)
{
	extern char _load_core2_jump_program_start_base[];
	extern char _load_core2_jump_program_end_base[];
	extern char _load_core2_entry_program_start_base[];
	extern char _load_core2_entry_program_end_base[];
	UINT32 code2JumpCodelen, code2EntryCodelen;

	//check core2_entry1 must at addr 0
	if (p_dram_partition->core2_entry1_addr != CORE2_JUMP_ADDR) {
		bl_displayErrMsg("core2entry1 != 0");
	}

	// copy core2 jump code to dram 0x0
	code2JumpCodelen = _load_core2_jump_program_end_base - _load_core2_jump_program_start_base;
	utl_memcpy((void *)p_dram_partition->core2_entry1_addr, _load_core2_jump_program_start_base, code2JumpCodelen);
//	CPUflushWriteCache(p_dram_partition->core2_entry1_addr, code2JumpCodelen);

	debug_msg_var("core2_jump_program", (int)_load_core2_jump_program_start_base);
	debug_msg_var("code2JumpCodelen", (int)code2JumpCodelen);
	debug_msg_var("core2_entry2_addr", (int)p_dram_partition->core2_entry2_addr);

	// copy core2 entry code to dram specified by fdt
	*(volatile UINT32 *)(NVT_CORE2_START) = p_dram_partition->core2_entry2_addr;
	code2EntryCodelen = _load_core2_entry_program_end_base - _load_core2_entry_program_start_base;
	utl_memcpy((void *)p_dram_partition->core2_entry2_addr, _load_core2_entry_program_start_base, code2EntryCodelen);
//	CPUflushWriteCache(p_dram_partition->core2_entry2_addr, code2EntryCodelen);

	debug_msg_var("core2_entry_program", (int)_load_core2_entry_program_start_base);
	debug_msg_var("code2EntryCodelen", (int)code2EntryCodelen);

	*(volatile UINT32 *)0xF07F8000 = last_addr;

	debug_msg_var("0xF07F8000=", (int)*(volatile UINT32 *)0xF07F8000);
}

static int bl_smp_start(unsigned char *p_modelext, UINT32 uboot_entry)
{
	SHMINFO *p_bininfo = (SHMINFO *)bl_get_fdt_cfg((const void *)p_modelext, MODELEXT_TYPE_BIN_INFO);
	if (p_bininfo == NULL) {
		debug_err("null p_bininfo\r\n");
		return -1;
	}

	DRAM_PARTITION *p_dram_partition = (DRAM_PARTITION *)bl_get_fdt_cfg((const void *)p_modelext, MODELEXT_TYPE_DRAM_PARTITION);
	if (p_bininfo == NULL) {
		debug_err("null p_dram_partition\r\n");
		return -1;
	}

	debug_msg_var("fdt", (UINT32)p_modelext);
	debug_msg_var("shm", (UINT32)p_bininfo);

	p_bininfo->boot.fdt_addr = (UINT32)p_modelext;
	debug_msg_var("p_bininfo->boot.fdt_addr", p_bininfo->boot.fdt_addr);
	utl_memset(p_bininfo->boot.LdInfo_1, 0, sizeof(p_bininfo->boot.LdInfo_1));
	utl_memcpy(p_bininfo->boot.LdInfo_1, "LD_NVT", 6);
	// clear cc_core1_addr, cc_core2_addr
#if USB_WRITELOADER
	if(utl_get_bootsrc() == BOOT_SOURCE_UART) {
		p_bininfo->comm.Resv[0] = BOOT_REASON_FROM_UART;
	} else if(USB_WRITELOADER || utl_get_bootsrc() == BOOT_SOURCE_USB) {
		p_bininfo->comm.Resv[0] = BOOT_REASON_FROM_USB;
	} else {
		p_bininfo->comm.Resv[0] = BOOT_REASON_NORMAL;
	}
#else
//	if (card_get_type() == EXT_STORAGE_TYPE_ETH)
//        p_bininfo->comm.Resv[0] = BOOT_REASON_FROM_ETH;
//    else if (g_is_recovery_triggered)
//		p_bininfo->comm.Resv[0] = BOOT_REASON_RECOVERY_SYS;
//    else
#if (STORAGE_EXT_TYPE == STORAGE_EXT_ETH)
	p_bininfo->comm.Resv[0] = BOOT_REASON_FROM_ETH;
#else
	p_bininfo->comm.Resv[0] = BOOT_REASON_NORMAL;
#endif
#endif
	p_bininfo->comm.Resv[1] = 0; // COMM_CORE1_START
	p_bininfo->comm.Resv[2] = 0; // COMM_CORE2_START
	p_bininfo->comm.Resv[3] = 0; // COMM_UITRON_COMP_ADDR
	p_bininfo->comm.Resv[4] = 0; // COMM_UITRON_COMP_LEN

	debug_msg("smp(no tee)\r\n");

	bl_core2_prepare(p_dram_partition, 0);

//#if USB_WRITELOADER
#if 0
	USBStateMachine();
	uart_putSystemUARTStr("USB update Done\n\r");
	timer_delay(1000000);
	USBOTGReset();
#endif



	//invalid Instruciton and data cache
	CPUCleanInvalidateDCacheAll();
	CPUInvalidateICacheAll();

	//reset core2
	bl_core2_reset();

    {
		typedef void (*BRANCH_CB)(void);
		BRANCH_CB p_func = (BRANCH_CB)uboot_entry;
		debug_msg_var("jump", uboot_entry);
		p_func();
	}

	return 0;
}
#endif
_THUMB2 static int bl_entry_boot(unsigned char *p_fdt, UINT32 uboot_entry)
{
	SHMINFO *p_bininfo = (SHMINFO *)bl_get_fdt_cfg((const void *)p_fdt, MODELEXT_TYPE_BIN_INFO);
	if (p_bininfo == NULL) {
		debug_err("null p_bininfo\r\n");
		return -1;
	}

	debug_msg_var("fdt", (UINT32)p_fdt);
	debug_msg_var("shm", (UINT32)p_bininfo);

	p_bininfo->boot.fdt_addr = (UINT32)p_fdt;
	utl_memset(p_bininfo->boot.LdInfo_1, 0, sizeof(p_bininfo->boot.LdInfo_1));
	utl_memcpy(p_bininfo->boot.LdInfo_1, "LD_NVT", 6);
	// clear cc_core1_addr, cc_core2_addr
#if USB_WRITELOADER
    CPUflushWriteCache((UINT32)p_bininfo, sizeof(BININFO));
	if(utl_get_bootsrc() == BOOT_SOURCE_USB) {
		p_bininfo->comm.Resv[0] = BOOT_REASON_FROM_USB;
	} else {
		p_bininfo->comm.Resv[0] = BOOT_REASON_FROM_UART;
	}
#else
	if (card_get_type() == EXT_STORAGE_TYPE_ETH)
        p_bininfo->comm.Resv[0] = BOOT_REASON_FROM_ETH;
   // else if (g_is_recovery_triggered)
	//	p_bininfo->comm.Resv[0] = BOOT_REASON_RECOVERY_SYS;
    else
		p_bininfo->comm.Resv[0] = BOOT_REASON_NORMAL;
#endif
	p_bininfo->comm.Resv[1] = 0; // COMM_CORE1_START
	p_bininfo->comm.Resv[2] = 0; // COMM_CORE2_START
	p_bininfo->comm.Resv[3] = 0; // COMM_UITRON_COMP_ADDR
	p_bininfo->comm.Resv[4] = 0; // COMM_UITRON_COMP_LEN

	//invalid Instruciton and data cache
	CPUCleanInvalidateDCacheAll();
	CPUInvalidateICacheAll();

    {
		typedef void (*BRANCH_CB)(void);
		BRANCH_CB p_func = (BRANCH_CB)uboot_entry;
		debug_msg_var("jump", uboot_entry);
		p_func();
	}

	return 0;
}

_THUMB2 int bl_copy_fdt_to_fdt_addr(unsigned char *p_fdt /*IN*/, unsigned char **pp_fdt /*OUT*/)
{
	DRAM_PARTITION *p_dram_partition;

	p_dram_partition = (DRAM_PARTITION *)bl_get_fdt_cfg((const void *)p_fdt, MODELEXT_TYPE_DRAM_PARTITION);

	if (p_dram_partition == NULL) {
		debug_err("bl_copy_fdt_to_fdt_addr failed.\r\n");
		return -1;
	}

	// load fdt to fdt buffer
	utl_memcpy((void *)p_dram_partition->fdt_addr, p_fdt, fdt_totalsize(p_fdt));
	*pp_fdt = (unsigned char *)p_dram_partition->fdt_addr;

	//reset shminfo
	SHMINFO *p_bininfo = (SHMINFO *)bl_get_fdt_cfg((const void *)p_fdt, MODELEXT_TYPE_BIN_INFO);
	bl_memset(p_bininfo, 0, sizeof(SHMINFO));
	return 0;
}

_THUMB2 static int bl_load_fdt_from_all_in_one(unsigned char *p_all_in_one, unsigned int all_in_one_size, unsigned char **pp_fdt /*OUT*/)
{
	NVTPACK_ER er;
	NVTPACK_MEM emb_fdt;
	NVTPACK_VERIFY_OUTPUT np_verify = {0};
	NVTPACK_GET_PARTITION_INPUT np_get_input;
	NVTPACK_MEM mem_in = {(void *)p_all_in_one, (unsigned int)all_in_one_size};

	if (nvtpack_verify(&mem_in, &np_verify) != NVTPACK_ER_SUCCESS) {
		debug_err("packbin verify failed.\r\n");
		return -1;
	}

	np_get_input.id = 1; // fdt must always put in partition[1]
	np_get_input.mem = mem_in;
	er = nvtpack_get_partition(&np_get_input, &emb_fdt);

	if (er == NVTPACK_ER_NOT_FOUND) {
		return -2;
	} else if (er == NVTPACK_ER_SUCCESS) {
		debug_msg_var("fdt addr", (int)emb_fdt.p_data);
		debug_msg_var("fdt size", (int)emb_fdt.len);
		if (bl_chk_fdt((unsigned int)emb_fdt.p_data, emb_fdt.len) == 0) {
			return bl_copy_fdt_to_fdt_addr(emb_fdt.p_data, pp_fdt);
		}
	}
	return -1;
}

_THUMB2 static unsigned int bl_load_rtos_from_all_in_one(unsigned char *p_all_in_one, unsigned int all_in_one_size, unsigned char **pp_fdt /*OUT*/)
{
	UINT32 uItron_fw_addr= 0;
	NVTPACK_ER er;
	NVTPACK_MEM emb_fdt;
	NVTPACK_VERIFY_OUTPUT np_verify = {0};
	NVTPACK_GET_PARTITION_INPUT np_get_input;
	NVTPACK_MEM mem_in = {(void *)p_all_in_one, (unsigned int)all_in_one_size};
	HEADINFO  *p_headinfo;

	if (nvtpack_verify(&mem_in, &np_verify) != NVTPACK_ER_SUCCESS) {
		debug_err("packbin verify failed.\r\n");
		return 0;
	}

	np_get_input.id = 5; // rtos must always put in partition[5] for now
	np_get_input.mem = mem_in;
	er = nvtpack_get_partition(&np_get_input, &emb_fdt);//&np_get_input, &emb_fdt);

	if (er == NVTPACK_ER_NOT_FOUND) {
		return 0;
	} else if (er == NVTPACK_ER_SUCCESS) {
		debug_msg("get rtos partition ok\r\n");

		p_headinfo = (HEADINFO *)(emb_fdt.p_data + BIN_INFO_OFFSET_RTOS);
		uItron_fw_addr = p_headinfo->CodeEntry - CODE_SECTION_OFFSET;

		utl_memcpy((void *)uItron_fw_addr, emb_fdt.p_data, emb_fdt.len);
		CPUflushWriteCache(uItron_fw_addr, emb_fdt.len);
	}
	debug_msg_var("uItron_fw_addr=", uItron_fw_addr);
	return uItron_fw_addr;
}

_THUMB2 static int bl_load_fdt_from_flash(unsigned char *p_tmp, unsigned int tmp_size, unsigned char **pp_fdt /*OUT*/)
{

	// read first block to get dtb size
	int blk_size = (int)int_strg_obj->flash_getBlockSize();
	int er = int_strg_obj->flash_readSectors(g_uiStartBlkUpdateFW, blk_size, p_tmp, NAND_RW_FIRMWARE);
	if (er < 0) {
		debug_err("bl_load_fdt_from_flash");
		return -1;
	}

	int total_size = ALIGN_CEIL(fdt_totalsize(p_tmp), blk_size);
	if ((int)tmp_size < total_size) {
		debug_err_var("tmp_size too small, require:", fdt_totalsize(p_tmp));
		return -1;
	}

	total_size -= blk_size;
	if (total_size > 0) {
		// read remain to get uboot starting addr on dram and offset in flash
		er = int_strg_obj->flash_readSectors(g_uiStartBlkUpdateFW+1, total_size, p_tmp+blk_size, NAND_RW_FIRMWARE);
		if (er < 0) {
			debug_err("bl_load_fdt_from_flash");
			return -1;
		}
	}

	if (bl_chk_fdt((unsigned int)p_tmp, fdt_totalsize(p_tmp)) == 0) {
		return bl_copy_fdt_to_fdt_addr(p_tmp, pp_fdt);
	}

	return -1;
}

#if (FDT_SUPPORT)
_THUMB2 static char *bl_get_partition_name(unsigned char *p_fdt)
{
	const static char *p_names[3] = {
		"/nand",
		"/nor",
		"/mmc@f0510000"
	};

	const char *name = NULL;
	switch(gStorageIntType) {
		case STORAGEINT_SPI_NAND:
			name = p_names[0];
			break;
		case STORAGEINT_SPI_NOR:
			name = p_names[1];
			break;
		case STORAGEINT_EMMC:
			name = p_names[2];
			break;
		default:
			debug_err("loader_setStorageIntType needs.\r\n");
			return NULL;
	}

	if (fdt_path_offset(p_fdt, name) > 0) {
		return (char *)name;
	}

	debug_err("E:bl_get_partition_name\r\n");
	return NULL;
}
#endif

#if (FDT_SUPPORT)
_THUMB2 static char *bl_make_fdt_path(char *root, char *name)
{
	//for fdt to combine the full path of node
	static char g_tmp_path[64] = {0};
	g_tmp_path[0] = 0;
	strcat(g_tmp_path, "/");
	strcat(g_tmp_path, root);
	strcat(g_tmp_path, "/");
	strcat(g_tmp_path, name);
	return g_tmp_path;
}
#endif

#if (FDT_SUPPORT)
_THUMB2 static int bl_get_partition(unsigned char *p_fdt /*IN*/, char *p_emb_name /*IN*/, EMB_PARTITION *p_emb/*OUT*/, int *p_id)
{
	int len;
	const unsigned long long *nodep;
	char *p_name = bl_get_partition_name(p_fdt);
	char *p_path = NULL;

	if (p_name == NULL) {
		return -1;
	}

	p_path = bl_make_fdt_path(p_name, p_emb_name);

	// get partition offset and size
	nodep = (const unsigned long long *)bl_get_fdt_property(p_fdt, p_path, PROPERTY_REG, &len);
	if (nodep == NULL) {
		debug_err("bl_get_partition-1\r\n");
		return -1;
	}

	p_emb->PartitionOffset = be64_to_cpu(nodep[0]);
	p_emb->PartitionSize = be64_to_cpu(nodep[1]);

	// get partition label
	char *p_label_name = (char *)bl_get_fdt_property(p_fdt, p_path, PROPERTY_LABEL, &len);
	if (p_label_name == NULL) {
		debug_err("bl_get_partition-2\r\n");
		return -1;
	}

	// get id
	p_path = bl_make_fdt_path(p_name, PATH_NVTPACK_INDEX);
	int nodeoffset = fdt_path_offset(p_fdt, p_path);

	for (nodeoffset = fdt_first_subnode(p_fdt, nodeoffset);
		(nodeoffset >= 0);
		(nodeoffset = fdt_next_subnode(p_fdt, nodeoffset))) {
		const struct fdt_property *prop;

		if (!(prop = fdt_get_property(p_fdt, nodeoffset, PROPERTY_PARTITION_NAME, &len))) {
			break;
		}
		const char *p_id_name = fdt_get_name(p_fdt, nodeoffset, &len);

		if (strcmp(p_label_name, (char *)prop->data) == 0) {
			*p_id = atoi(p_id_name + strlen("id"));
			return 0;
		}
	}

	return -1;
}
#endif

_THUMB2 static void *bl_get_uboot_partition(unsigned char *p_fdt /*IN*/, int *p_id /*OUT*/)
{
#if (FDT_SUPPORT)
	if (bl_get_partition(p_fdt, PATH_PARTITION_UBOOT, &g_emb_uboot, p_id) != 0) {
		return NULL;
	}
#else
	*p_id = NVTPACK_IDX_UBOOT;
#endif
	g_emb_uboot.EmbType = EMBTYPE_UBOOT;
	return &g_emb_uboot;
}

_THUMB2 static void *bl_get_rtos_partition(unsigned char *p_fdt /*IN*/, int *p_id /*OUT*/)
{
#if (FDT_SUPPORT)
	if (bl_get_partition(p_fdt, PATH_PARTITION_RTOS, &g_emb_rtos, p_id) != 0) {
		return NULL;
	}
#else
	*p_id = NVTPACK_IDX_RTOS;
#endif
	g_emb_rtos.EmbType = EMBTYPE_RTOS;
	return &g_emb_rtos;
}

#if (NUTTX_SUPPORT)
_THUMB2 static void *bl_get_nuttx_partition(unsigned char *p_fdt /*IN*/, int *p_id /*OUT*/)
{
	if (bl_get_partition(p_fdt, PATH_PARTITION_NUTTX, &g_emb_nuttx, p_id) != 0) {
		return NULL;
	}
	g_emb_nuttx.EmbType = EMBTYPE_NUTTX;

	return &g_emb_nuttx;
}
#endif

_THUMB2 static void *bl_get_teeos_partition(unsigned char *p_fdt /*IN*/, int *p_id /*OUT*/)
{
#if (FDT_SUPPORT)
	if (bl_get_partition(p_fdt, PATH_PARTITION_TEEOS, &g_emb_teeos, p_id) != 0) {
		return NULL;
	}
#else
	*p_id = NVTPACK_IDX_TEEOS;
#endif
	g_emb_teeos.EmbType = EMBTYPE_TEEOS;

	return &g_emb_teeos;
}
#endif

// return decompress_fw_size
_THUMB2 static unsigned int bl_decompress_rtos(UINT32 compress_addr, UINT32 compress_size, UINT32 fw_base_addr)
{
	UINT32 decoded_size;
	NVTPACK_BFC_HDR *pBfc = (NVTPACK_BFC_HDR *)compress_addr;
	UINT32 size_comp_le = invertEndian(pBfc->uiSizeComp);
	UINT32 size_uncomp_le = invertEndian(pBfc->uiSizeUnComp);

	size_comp_le = (compress_size < size_comp_le) ? compress_size : size_comp_le;
	decoded_size = LZ_Un_compress((UINT8 *)compress_addr + sizeof(NVTPACK_BFC_HDR), (UINT8 *)fw_base_addr, size_comp_le);
	//because some padding bytes are decoded, we must return real rtos size
	return (decoded_size < size_uncomp_le) ? decoded_size : size_uncomp_le;
}

#if !REMOVED_FLASH
_THUMB2 static int bl_load_uboot_from_all_in_one(unsigned char *p_fdt, unsigned char *p_all_in_one, unsigned int all_in_one_size)
{
	NVTPACK_ER er;
	NVTPACK_MEM mem_uboot;
	int uboot_partition_id;
	NVTPACK_GET_PARTITION_INPUT   input;
	DRAM_PARTITION *p_dram_partition = NULL;
	EMB_PARTITION *p_emb_partition_uboot = bl_get_uboot_partition(p_fdt, &uboot_partition_id);

	if (p_emb_partition_uboot == NULL) {
		debug_err("null p_emb_partition_uboot\r\n");
		return -1;
	}

	p_dram_partition = (DRAM_PARTITION *)bl_get_fdt_cfg((const void *)p_fdt, MODELEXT_TYPE_DRAM_PARTITION);

	if (p_dram_partition == NULL) {
		debug_err("null p_dram_partition\r\n");
		return -1;
	}

	// get partition data address and size
	input.id = (unsigned int)uboot_partition_id;
	input.mem.p_data = (void *)p_all_in_one;
	input.mem.len    = all_in_one_size;
	er = nvtpack_get_partition(&input, &mem_uboot);
	if (er == NVTPACK_ER_NOT_FOUND) {
		return -2;
	}

	if (er != NVTPACK_ER_SUCCESS) {
		return -1;
	}

	debug_msg_var("uboot_addr", p_dram_partition->uboot_addr);
//	debug_msg_var("uboot_size", p_dram_partition->uboot_size);
# if 0
	if(0) {
#else
#if (_FPGA_EMULATION_ == 0)
	if(is_secure_enable() == 0) {
#else
	if (1) {
#endif
#endif
		if (bl_chk_uboot((unsigned int)mem_uboot.p_data, mem_uboot.len) != 0) {
			return -1;
		}

		// flow to handle compressed u-boot and uncompressed one.
		HEADINFO *p_headinfo = (HEADINFO *)(mem_uboot.p_data + BIN_INFO_OFFSET_UBOOT); // describe uncompressed u-boot
		NVTPACK_BFC_HDR *pBfc = (NVTPACK_BFC_HDR *)mem_uboot.p_data; // describe compressed u-boot

		// load uboot bin to dram partiton location
		//debug_msg_var("uboot_addr", p_dram_partition->uboot_addr);
		//debug_msg_var("uboot_size", p_dram_partition->uboot_size);

		if (pBfc->uiFourCC == MAKEFOURCC('B', 'C', 'L', '1')) {
			if((cpu_to_be32(pBfc->uiAlgorithm) & 0xFF ) == 11) {
#if (_ROM_PUBLIC_API_ == 1)
				UINT32 lzma_addr = *(UINT32 *)ROM_LZMA_POSITION;
				UINT32 size_comp_le = invertEndian(pBfc->uiSizeComp);
				UINT32 size_uncomp_le = invertEndian(pBfc->uiSizeUnComp);
			rom_lzma_inflate = (int (*)(UINT8 *, UINT32 , UINT8 * , UINT32 , UINT8 *, UINT32 ))((lzma_addr));
	    rom_lzma_inflate(mem_uboot.p_data + sizeof(NVTPACK_BFC_HDR), size_comp_le, (unsigned char *) p_dram_partition->uboot_addr, size_uncomp_le, (UINT8 *)lzma_temp_buffer, 65536);
				debug_msg("lzma ");

#else
				debug_msg("Can not support LZMA\r\n");
#endif
			} else {
				UINT32 size_comp_le = invertEndian(pBfc->uiSizeComp);
				LZ_Un_compress(mem_uboot.p_data + sizeof(NVTPACK_BFC_HDR), (unsigned char *) p_dram_partition->uboot_addr, size_comp_le);
				debug_msg("lz");
			}
		} else {
			debug_msg("nml");
			utl_memcpy((void *)p_dram_partition->uboot_addr, mem_uboot.p_data, p_headinfo->BinLength);
		}
	} else {
#if (SECURE_DECRYPT_UBOOT)
		utl_memcpy((void *)p_dram_partition->uboot_addr, mem_uboot.p_data, mem_uboot.len);
		unsigned int plaintext_size =0;

		if((er = do_decrypt_aes(p_dram_partition->uboot_addr, p_dram_partition->uboot_addr, &plaintext_size, p_dram_partition->uboot_size))!= 0 )
		{
			//debug_err("aes fail\r\n");
			return er;
		}
		if (bl_chk_uboot((unsigned int)p_dram_partition->uboot_addr, plaintext_size) != 0) {
			return -1;
		}
#else
		debug_msg("please enable SECURE_DECRYPT_UBOOT\r\n");
		return -1;
#endif
	}

	return 0;
}

#if (NUTTX_SUPPORT)
_THUMB2 static int bl_load_nuttx_from_all_in_one(unsigned char *p_fdt, unsigned char *p_all_in_one, unsigned int all_in_one_size)
{
	NVTPACK_ER er;
	NVTPACK_MEM mem_nuttx;
	int nuttx_partition_id;
	NVTPACK_GET_PARTITION_INPUT   input;
	DRAM_PARTITION *p_dram_partition = NULL;
	EMB_PARTITION *p_emb_partition_nuttx = bl_get_nuttx_partition(p_fdt, &nuttx_partition_id);

	if (p_emb_partition_nuttx == NULL) {
		debug_err("null p_emb_partition_nuttx\r\n");
		return -1;
	}

	p_dram_partition = (DRAM_PARTITION *)bl_get_fdt_cfg((const void *)p_fdt, MODELEXT_TYPE_DRAM_PARTITION);

	if (p_dram_partition == NULL) {
		debug_err("null p_dram_partition\r\n");
		return -1;
	}

	// get partition data address and size
	input.id = (unsigned int)nuttx_partition_id;
	input.mem.p_data = (void *)p_all_in_one;
	input.mem.len    = all_in_one_size;
	er = nvtpack_get_partition(&input, &mem_nuttx);
	if (er == NVTPACK_ER_NOT_FOUND) {
		return -2;
	}

	if (er != NVTPACK_ER_SUCCESS) {
		return -1;
	}

	// using check uboot's check sum is ok
	if (bl_chk_uboot((unsigned int)mem_nuttx.p_data, mem_nuttx.len) != 0) {
		return -1;
	}

	// flow to handle compressed u-boot and uncompressed one.
	HEADINFO *p_headinfo = (HEADINFO *)(mem_nuttx.p_data + BIN_INFO_OFFSET_NUTTX); // describe uncompressed u-boot
	NVTPACK_BFC_HDR *pBfc = (NVTPACK_BFC_HDR *)mem_nuttx.p_data; // describe compressed u-boot

	// load nuttx bin to dram partiton location
	debug_msg_var("nuttx_addr", p_dram_partition->nuttx_addr);
	debug_msg_var("nuttx_size", p_dram_partition->nuttx_size);

	if (pBfc->uiFourCC == MAKEFOURCC('B', 'C', 'L', '1')) {
		UINT32 size_comp_le = invertEndian(pBfc->uiSizeComp);
		LZ_Un_compress(mem_nuttx.p_data + sizeof(NVTPACK_BFC_HDR), (unsigned char *) p_dram_partition->nuttx_addr, size_comp_le);
	} else {
		utl_memcpy((void *)p_dram_partition->nuttx_addr, mem_nuttx.p_data, p_headinfo->BinLength);
	}

	return 0;
}
#endif

_THUMB2 static int bl_load_teeos_from_all_in_one(unsigned char *p_fdt, unsigned char *p_all_in_one, unsigned int all_in_one_size)
{
	NVTPACK_ER er;
	NVTPACK_MEM mem_teeos;
	int teeos_partition_id;
	NVTPACK_GET_PARTITION_INPUT   input;
	DRAM_PARTITION *p_dram_partition = NULL;
	EMB_PARTITION *p_emb_partition_teeos = bl_get_teeos_partition(p_fdt, &teeos_partition_id);
	HEADINFO *p_headinfo = NULL;
	if (p_emb_partition_teeos == NULL) {
		debug_err("null p_emb_partition_teeos\r\n");
		return -1;
	}

	p_dram_partition = (DRAM_PARTITION *)bl_get_fdt_cfg((const void *)p_fdt, MODELEXT_TYPE_DRAM_PARTITION);

	if (p_dram_partition == NULL) {
		debug_err("null p_dram_partition\r\n");
		return -1;
	}

	// get partition data address and size
	input.id = (unsigned int)teeos_partition_id;
	input.mem.p_data = (void *)p_all_in_one;
	input.mem.len    = all_in_one_size;
	er = nvtpack_get_partition(&input, &mem_teeos);
	if (er == NVTPACK_ER_NOT_FOUND) {
		return -2;
	}

	if (er != NVTPACK_ER_SUCCESS) {
		return -1;
	}

#if  0
	if(1){
#else
	if(is_secure_enable()) {
#endif
#if (SECURE_DECRYPT_OPTEE_OS)
		unsigned int plaintext_size =0;
		utl_memcpy((void *)p_dram_partition->teeos_addr, mem_teeos.p_data, mem_teeos.len);
		if((er = do_decrypt_aes(p_dram_partition->teeos_addr, p_dram_partition->teeos_addr,&plaintext_size, p_dram_partition->teeos_size))!= 0)
		{
			//debug_err("aes fail\r\n");
			return er;
		}

		//this is the second headinfo information , not encrpyted headinfo
		p_headinfo = (HEADINFO *)(p_dram_partition->teeos_addr + BIN_INFO_OFFSET_TEEOS);
#else
		debug_err("please enable SECURE_DECRYPT_OPTEE_OS");
		return -1;
#endif
	}
	else{
		p_headinfo = (HEADINFO *)(mem_teeos.p_data + BIN_INFO_OFFSET_TEEOS); // describe uncompressed u-boot
		utl_memcpy((void *)p_dram_partition->teeos_addr, (unsigned char*)mem_teeos.p_data, p_headinfo->BinLength);
	}
	UINT32 teeos_addr = p_headinfo->Resv1[HEADINFO_TEEOS_RESV_IDX_LOAD_ADDR];

	//check addr matched or not
	debug_msg_var("teeos_addr", p_dram_partition->teeos_addr);
	if (teeos_addr != p_dram_partition->teeos_addr) {
		debug_msg_var("teeos_addr(bin)", teeos_addr);
		debug_err("teeos addr not matched.\r\n");
		return -1;
	}

	// using check uboot's check sum is ok
	if (bl_chk_uboot((unsigned int)teeos_addr, p_headinfo->BinLength) != 0) {
		return -1;
	}

	return 0;
}

_THUMB2 static int bl_load_uboot_from_flash(unsigned char *p_fdt, unsigned char *p_tmp)
{
	int er;
	int uboot_partition_id;
	DRAM_PARTITION *p_dram_partition = NULL;
	unsigned int blk_size = int_strg_obj->flash_getBlockSize();
	EMB_PARTITION *p_emb_partition_uboot = bl_get_uboot_partition(p_fdt, &uboot_partition_id);

	if (p_emb_partition_uboot == NULL) {
		debug_err("null p_emb_partition_uboot\r\n");
		return -1;
	}


	p_dram_partition = (DRAM_PARTITION *)bl_get_fdt_cfg(p_fdt, MODELEXT_TYPE_DRAM_PARTITION);
	if (p_dram_partition == NULL) {
		debug_err("null p_dram_partition\r\n");
		return -1;
	}


#if (SECURE_DECRYPT_UBOOT)
	if ((er = int_strg_obj->flash_readSectors(p_emb_partition_uboot->PartitionOffset / blk_size,
			 ALIGN_CEIL(BIN_INFO_OFFSET_UBOOT + sizeof(HEADINFO), blk_size), p_tmp, NAND_RW_FIRMWARE)) < 0)
	{
		debug_err_var("bl_load_uboot_from_flash,er=", er);
		bl_displayErrMsg(RWErrorMsg); // read uboot failed
		return -1;
	}
#else

	if ((er = int_strg_obj->flash_readSectors(p_emb_partition_uboot->PartitionOffset / blk_size,
		 ALIGN_CEIL(BIN_INFO_OFFSET_UBOOT + sizeof(HEADINFO), blk_size), p_tmp, NAND_RW_FIRMWARE)) < 0)
	{
		debug_err_var("bl_load_uboot_from_flash,er=", er);
		bl_displayErrMsg(RWErrorMsg); // read uboot failed
		return -1;
	}

#endif

	debug_msg_var("uboot_addr", p_dram_partition->uboot_addr);
	debug_msg_var("uboot_size", p_dram_partition->uboot_size);
#if 0
	if(0){
#else
	if(is_secure_enable() == 0) {
#endif
		HEADINFO *p_headinfo = (HEADINFO *)(p_tmp + BIN_INFO_OFFSET_UBOOT); // describe uncompressed u-boot
		NVTPACK_BFC_HDR *pBfc = (NVTPACK_BFC_HDR *)p_tmp; // describe compressed u-boot

		if (pBfc->uiFourCC == MAKEFOURCC('B', 'C', 'L', '1'))
		{
			UINT32 size_comp_le = invertEndian(pBfc->uiSizeComp);
			if ((er = int_strg_obj->flash_readSectors(p_emb_partition_uboot->PartitionOffset / blk_size,
					 ALIGN_CEIL(size_comp_le, blk_size),p_tmp, NAND_RW_FIRMWARE)) < 0)
			{
				debug_err_var("bl_load_uboot_from_flash_comp,er=", er); // read bfc-uboot failed
				bl_displayErrMsg(RWErrorMsg);
			}
			if((cpu_to_be32(pBfc->uiAlgorithm) & 0xFF ) == 11) {
#if (_ROM_PUBLIC_API_ == 1)
				UINT32 lzma_addr = *(UINT32 *)ROM_LZMA_POSITION;
				UINT32 size_comp_le = invertEndian(pBfc->uiSizeComp);
				UINT32 size_uncomp_le = invertEndian(pBfc->uiSizeUnComp);
				rom_lzma_inflate = (int (*)(UINT8 *, UINT32 , UINT8 * , UINT32 , UINT8 *, UINT32 ))((lzma_addr));
				rom_lzma_inflate(p_tmp + sizeof(NVTPACK_BFC_HDR), size_comp_le, (unsigned char *) p_dram_partition->uboot_addr, size_uncomp_le, (UINT8 *)lzma_temp_buffer, 65536);
				debug_msg("lzma ");
#else
				debug_msg("Can not support LZMA\r\n");
#endif
			} else {
				LZ_Un_compress(p_tmp + sizeof(NVTPACK_BFC_HDR), (unsigned char *)p_dram_partition->uboot_addr, size_comp_le);
			}
		}
		else
		{
			if ((er = int_strg_obj->flash_readSectors(p_emb_partition_uboot->PartitionOffset / blk_size,
				 ALIGN_CEIL(p_headinfo->BinLength, blk_size),
				 (UINT8 *)p_dram_partition->uboot_addr, NAND_RW_FIRMWARE)) < 0)
			{
				debug_err_var("bl_load_uboot_from_flash,er=", er); // read bfc-uboot failed
				bl_displayErrMsg(RWErrorMsg);
			}
		}
	}
	else
	{
#if (SECURE_DECRYPT_UBOOT)
		HEADINFO *p_headinfo = (HEADINFO *)(p_tmp); // describe uncompressed u-boot
		if ((er = int_strg_obj->flash_readSectors(p_emb_partition_uboot->PartitionOffset / blk_size,
			 ALIGN_CEIL(p_headinfo->BinLength, blk_size),
			 (UINT8 *)p_dram_partition->uboot_addr, NAND_RW_FIRMWARE)) < 0)
		{
			debug_err_var("bl_load_uboot_from_flash,er=", er); // read bfc-uboot failed
			bl_displayErrMsg(RWErrorMsg);
		}
		unsigned int plaintext_size=0;
		if((er =do_decrypt_aes(p_dram_partition->uboot_addr, p_dram_partition->uboot_addr, &plaintext_size, p_dram_partition->uboot_size))!= 0)
		{
			//debug_err("aes fail\r\n");
			return er;
		}
#else
		debug_msg("plase enable SECURE_DECRYPT_UBOOT config\r\n");
		return -1;
#endif
	}
	return 0;
}

_THUMB2 static int bl_load_rtos_from_flash(unsigned char *p_fdt, unsigned char *p_tmp)
{
	int er;
	int rtos_partition_id;
	DRAM_PARTITION *p_dram_partition = NULL;
	unsigned int blk_size = int_strg_obj->flash_getBlockSize();
	EMB_PARTITION *p_emb_partition_rtos = bl_get_rtos_partition(p_fdt, &rtos_partition_id);
	SHMINFO *p_shminfo = (SHMINFO *)bl_get_fdt_cfg((const void *)p_fdt, MODELEXT_TYPE_BIN_INFO);

	if (p_emb_partition_rtos == NULL) {
		debug_err("null p_emb_partition_rtos\r\n");
		return -1;
	}


	p_dram_partition = (DRAM_PARTITION *)bl_get_fdt_cfg(p_fdt, MODELEXT_TYPE_DRAM_PARTITION);
	if (p_dram_partition == NULL) {
		debug_err("null p_dram_partition\r\n");
		return -1;
	}

#if 0//(LOADER_TYPE == STAND_ALONE_LOADER_528) || (LOADER_TYPE == COMBINATION_528)
	if(bl_is_smp(p_fdt)) {
		bl_core2_prepare(p_dram_partition, 0);
		//invalid Instruciton and data cache
		CPUCleanInvalidateDCacheAll();
		bl_core2_reset();
	}
#endif

	// load 1st block of rtos to tmp dram (to detect compressed u-boot)
	unsigned int bininfo_preload_size = ALIGN_CEIL(BIN_INFO_OFFSET_RTOS + sizeof(HEADINFO), blk_size);
	if ((er = int_strg_obj->flash_readSectors(p_emb_partition_rtos->PartitionOffset / blk_size, bininfo_preload_size, p_tmp, NAND_RW_FIRMWARE)) < 0) {
		debug_err_var("bl_load_rtos_from_flash,er=", er);
		bl_displayErrMsg(RWErrorMsg); // read rtos failed
	}

	// flow to handle compressed u-boot and uncompressed one.
	NVTPACK_BFC_HDR *pBfc = (NVTPACK_BFC_HDR *)p_tmp; // describe compressed u-boot

	// load rtos bin to dram partiton location
	//debug_msg_var("rtos_addr", p_dram_partition->rtos_addr);
	//debug_msg_var("rtos_size", p_dram_partition->rtos_size);

	if (pBfc->uiFourCC == MAKEFOURCC('B', 'C', 'L', '1')) {

		if((cpu_to_be32(pBfc->uiAlgorithm) & 0xFF ) == 11)
		{
			/* lzma compressed image*/
			debug_msg("lzma, use uboot\r\n");
			er = bl_load_uboot_from_flash((unsigned char *)p_dram_partition->fdt_addr, p_tmp);
			if (er != 0) {
				bl_displayErrMsg("load uboot failed\r\n");
			}

			// boot uboot
			er = bl_boot_uboot((unsigned char *)p_dram_partition->fdt_addr);
			if (er != 0) {
				bl_displayErrMsg("boot uboot failed\r\n");
			}
		} else {
			UINT32 size_comp_le = invertEndian(pBfc->uiSizeComp);
			if ((er = int_strg_obj->flash_readSectors(p_emb_partition_rtos->PartitionOffset / blk_size, ALIGN_CEIL(size_comp_le, blk_size), p_tmp, NAND_RW_FIRMWARE)) < 0) {
				debug_err_var("bl_load_rtos_from_flash_comp,er=", er); // read bfc-rtos failed
				bl_displayErrMsg(RWErrorMsg);
			}
			LZ_Un_compress(p_tmp + sizeof(NVTPACK_BFC_HDR), (unsigned char *)p_dram_partition->rtos_addr, size_comp_le);
		}
	} else {
		BININFO *p_bininfo = (BININFO *)(p_tmp + BIN_INFO_OFFSET_RTOS); // describe uncompressed u-boot
		if (p_bininfo->head.Resv1[HEADINFO_RESV_IDX_BOOT_FLAG] & BOOT_FLAG_PARTLOAD_EN) {
			// copy bininfo_preload to rtos_addr
			bl_memcpy((UINT8 *)p_dram_partition->rtos_addr, p_tmp, bininfo_preload_size);
			// preload some to get part-1 size for partial load and partial compressed load
			UINT32 preload_size = ALIGN_CEIL(FW_PART1_SIZE_OFFSET, blk_size) - bininfo_preload_size;

			if (preload_size!= 0) {
				er = int_strg_obj->flash_readSectors(
					(p_emb_partition_rtos->PartitionOffset + bininfo_preload_size) / blk_size,
					preload_size,
					(UINT8 *)(p_dram_partition->rtos_addr + bininfo_preload_size), NAND_RW_FIRMWARE
				);

				if (er < 0) {
					debug_err_var("bl_load_rtos_from_flash_pl1,er=", er); // read bfc-uboot failed
					bl_displayErrMsg(RWErrorMsg);
				}
			}
			UINT32 part1_size = *(UINT32 *)(p_dram_partition->rtos_addr + FW_PART1_SIZE_OFFSET);
			debug_msg_var("part1_size",part1_size);
			part1_size = ALIGN_CEIL(part1_size, blk_size);
			//debug_msg_var("part1_size_aligned",part1_size);
			if ((er = int_strg_obj->flash_readSectors(p_emb_partition_rtos->PartitionOffset / blk_size, part1_size, (UINT8 *)p_dram_partition->rtos_addr, NAND_RW_FIRMWARE)) < 0) {
				debug_err_var("bl_load_rtos_from_flash_pl1,er=", er);
				bl_displayErrMsg(RWErrorMsg);
			}
			// update loaded size
			p_shminfo->boot.LdLoadSize = part1_size;
		} else {
			// full load
			debug_msg_var("rtos_size", p_bininfo->head.BinLength);
			if ((er = int_strg_obj->flash_readSectors(p_emb_partition_rtos->PartitionOffset  /blk_size, ALIGN_CEIL(p_bininfo->head.BinLength, blk_size), (UINT8 *)p_dram_partition->rtos_addr, NAND_RW_FIRMWARE)) < 0) {
				debug_err_var("bl_load_rtos_from_flash,er=", er); // read bfc-uboot failed
				bl_displayErrMsg(RWErrorMsg);
			}
			// update loaded size
			p_shminfo->boot.LdLoadSize = p_bininfo->head.BinLength;
		}
	}
	return 0;
}

_THUMB2 static int bl_load_teeos_from_flash(unsigned char *p_fdt, unsigned char *p_tmp)
{
	int er;
	int teeos_partition_id;
	DRAM_PARTITION *p_dram_partition = NULL;
	unsigned int blk_size = int_strg_obj->flash_getBlockSize();
	EMB_PARTITION *p_emb_partition_teeos = bl_get_teeos_partition(p_fdt, &teeos_partition_id);

	if (p_emb_partition_teeos == NULL) {
		debug_err("null p_emb_partition_teeos\r\n");
		return -1;
	}


	p_dram_partition = (DRAM_PARTITION *)bl_get_fdt_cfg(p_fdt, MODELEXT_TYPE_DRAM_PARTITION);
	if (p_dram_partition == NULL) {
		debug_err("null p_dram_partition\r\n");
		return -1;
	}

	// load 1st block of u-boot to tmp dram (to detect compressed u-boot)
	if ((er = int_strg_obj->flash_readSectors(p_emb_partition_teeos->PartitionOffset / blk_size, ALIGN_CEIL(BIN_INFO_OFFSET_TEEOS + sizeof(HEADINFO), blk_size), p_tmp, NAND_RW_FIRMWARE)) < 0) {
		debug_err_var("bl_load_teeos_from_flash,er=", er);
		bl_displayErrMsg(RWErrorMsg); // read teeos failed
	}

	// flow to handle compressed u-boot and uncompressed one.
#if (SECURE_DECRYPT_OPTEE_OS)
	HEADINFO *p_headinfo = (HEADINFO *)(p_tmp); // describe uncompressed u-boot
#else
	HEADINFO *p_headinfo = (HEADINFO *)(p_tmp + BIN_INFO_OFFSET_TEEOS); // describe uncompressed u-boot

#endif

	if ((er = int_strg_obj->flash_readSectors(p_emb_partition_teeos->PartitionOffset / blk_size, ALIGN_CEIL(p_headinfo->BinLength, blk_size), (UINT8 *)p_dram_partition->teeos_addr, NAND_RW_FIRMWARE)) < 0) {
			debug_err_var("bl_load_teeos_from_flash,er=", er); // read bfc-teeos failed
			bl_displayErrMsg(RWErrorMsg);
	}
#if 0
	if(1){
#else
	if(is_secure_enable()) {
#endif
#if (SECURE_DECRYPT_OPTEE_OS)

		unsigned int plaintext_size=0;
		if((er = do_decrypt_aes(p_dram_partition->teeos_addr, p_dram_partition->teeos_addr, &plaintext_size, p_dram_partition->teeos_size)) != 0)
		{
			//debug_err("aes fail\r\n");
			return er;
		}

		p_headinfo = (HEADINFO *)(p_dram_partition->teeos_addr + BIN_INFO_OFFSET_TEEOS); // describe uncompressed u-boot
#else
		debug_err("please ENABLE SECURE_DECRYPT_OPTEE_OS config\n");
		return -1;
#endif
	}

	// load teeos bin to dram partiton location
	UINT32 teeos_addr = p_headinfo->Resv1[HEADINFO_TEEOS_RESV_IDX_LOAD_ADDR];
	debug_msg_var("teeos_addr", p_dram_partition->teeos_addr);
	if (teeos_addr != p_dram_partition->teeos_addr) {
		debug_msg_var("teeos_addr(bin)", teeos_addr);
		debug_err("teeos addr not matched.\r\n");
		return -1;
	}
	//check check sum
	if (bl_chk_uboot((unsigned int)teeos_addr, p_headinfo->BinLength) != 0) {
		return -1;
	}
	return 0;
}
#endif

#if !(USB_WRITELOADER || UART_UPDATE)
_THUMB2 static int bl_load_rtos_from_non_nvtpack(unsigned int src_addr, unsigned int src_size, unsigned int *p_dst_addr, unsigned int *p_dst_size)
{
	NVTPACK_BFC_HDR *pBFC = (NVTPACK_BFC_HDR *)src_addr;
	HEADINFO *p_headinfo = (HEADINFO *)(src_addr + BIN_INFO_OFFSET_RTOS);
	if (pBFC->uiFourCC == MAKEFOURCC('B', 'C', 'L', '1')) {
		// to avoid rtos_addr is overlapped by src_addr, we need extract some bits to get real rtos addr.
		// compressed firmware
		debug_msg("compressed t.bin\r\n");
		// decode some bytes to get uncompressed address
		bl_decompress_rtos(src_addr, SIZE_PRELOAD, src_addr + SIZE_PRELOAD);
		p_headinfo = (HEADINFO *)(src_addr + SIZE_PRELOAD + BIN_INFO_OFFSET_RTOS);
		// check if valid after decode
		if ((strncmp(p_headinfo->BinInfo_1, "NT", 2) == 0) ||
			(strncmp(p_headinfo->BinInfo_1, "NC", 2) == 0)) {
			UINT32 uncompress_addr = p_headinfo->CodeEntry - CODE_SECTION_OFFSET;
			UINT32 uncompress_size = p_headinfo->BinLength;
			UINT32 compress_size = invertEndian(pBFC->uiSizeComp) + sizeof(NVTPACK_BFC_HDR);
			//adjust src_addr that lay compressed f.w followed by uncomppressed f.w
			src_addr = uncompress_addr + uncompress_size;
			src_addr = ALIGN_CEIL(src_addr, 4);
			src_size = fat_read_rootfile((UINT8 *)src_addr, compress_size);
		} else {
			return -1;
		}
	} else if ((strncmp(p_headinfo->BinInfo_1, "NT", 2) == 0) ||
			   (strncmp(p_headinfo->BinInfo_1, "NC", 2) == 0)) {
		// uncompressed firmware
		UINT32 uncompress_addr = p_headinfo->CodeEntry - CODE_SECTION_OFFSET;
		UINT32 uncompress_size = p_headinfo->BinLength;

		src_size = fat_read_rootfile((UINT8 *)src_addr, uncompress_size);
		src_addr = uncompress_addr;
		src_size = uncompress_size;
		debug_msg("uncompressed t.bin\r\n");
#if (DRAM_RANGE_SCAN_EN == ENABLE)
	} else if (utl_is_sram_fw(src_addr) == TRUE) {
		debug_msg("detected as SRAM fw\r\n");
		src_size = fat_read_rootfile((UINT8 *)src_addr, FAT_READ_TOTAL_FILE_LENGTH);
#endif
	} else {
		return -1;
	}

	*p_dst_addr = src_addr;
	*p_dst_size = src_size;
	return 0;
}
#endif

#if 0//(STORAGE_EXT_TYPE == STORAGE_EXT_USB)
_THUMB2 static int bl_load_rtos_from_usb_raw(unsigned int src_addr, unsigned int src_size, unsigned int *p_dst_addr, unsigned int *p_dst_size)
{
	NVTPACK_BFC_HDR *pBFC = (NVTPACK_BFC_HDR *)src_addr;
	HEADINFO *p_headinfo = (HEADINFO *)(src_addr + BIN_INFO_OFFSET_RTOS);
	if (pBFC->uiFourCC == MAKEFOURCC('B', 'C', 'L', '1')) {
		// to avoid rtos_addr is overlapped by src_addr, we need extract some bits to get real rtos addr.
		// compressed firmware
		debug_msg("compressed t.bin\r\n");
		// decode some bytes to get uncompressed address
		// assume usb write loader receive full fw at 0x0200_0000, decompress it to smaller address
		bl_decompress_rtos(src_addr, SIZE_PRELOAD, src_addr - SIZE_PRESERVE_USB);
		p_headinfo = (HEADINFO *)(src_addr - SIZE_PRESERVE_USB + BIN_INFO_OFFSET_RTOS);
		// check if valid after decode
		if ((strncmp(p_headinfo->BinInfo_1, "NT", 2) == 0) ||
			(strncmp(p_headinfo->BinInfo_1, "NC", 2) == 0)) {
			UINT32 uncompress_addr = p_headinfo->CodeEntry - CODE_SECTION_OFFSET;
			UINT32 uncompress_size = p_headinfo->BinLength;
			UINT32 compress_size = invertEndian(pBFC->uiSizeComp) + sizeof(NVTPACK_BFC_HDR);
			//adjust src_addr that lay compressed f.w followed by uncomppressed f.w
			utl_memcpy((void *)(uncompress_addr + uncompress_size), (void *)src_addr, compress_size);
			src_addr = uncompress_addr + uncompress_size;
			src_addr = ALIGN_CEIL(src_addr, 4);
			src_size = compress_size;
		} else {
			return -1;
		}
	} else if ((strncmp(p_headinfo->BinInfo_1, "NT", 2) == 0) ||
			   (strncmp(p_headinfo->BinInfo_1, "NC", 2) == 0)) {
		// uncompressed firmware
		UINT32 uncompress_addr = p_headinfo->CodeEntry - CODE_SECTION_OFFSET;
		UINT32 uncompress_size = p_headinfo->BinLength;
		debug_msg("uncompressed t.bin\r\n");
		utl_memcpy((void *)uncompress_addr, (void *)src_addr, uncompress_size);
		src_size = uncompress_size;
#if (DRAM_RANGE_SCAN_EN == ENABLE)
	} else if (utl_is_sram_fw(src_addr) == TRUE) {
		debug_msg("detected as SRAM fw. NOt Support from USB\r\n");
#endif
	} else {
		return -1;
	}

	*p_dst_addr = src_addr;
	*p_dst_size = src_size;
	return 0;
}
#endif

//#if !USB_WRITELOADER
#if !((STORAGE_EXT_TYPE == STORAGE_EXT_USB) || (STORAGE_EXT_TYPE == STORAGE_EXT_UART))
_THUMB2 static int bl_load_rtos_from_uart(unsigned int src_addr, unsigned int src_size, unsigned int *p_dst_addr, unsigned int *p_dst_size)
{
#if UART_UPDATE_
	UINT32 tick_kms = 10;  // timeout 1000 ms
	char key;

	debug_msg("UART press Enter to\r\n");

	while (--tick_kms) {
		uart_chkChar(&key);

		if (key != 0x00) {
			break;
		}
		timer_delay(100000); //100000
		debug_msg(".");
	}

	if (tick_kms) {
		UINT32 uiLength;
		debug_msg("\r\nEnter uboot length (Decimal):\r\n"); //must \r\n at the end because of auto_test tool
		uart_getStr_polling(g_strLength);
		uiLength = DecStr2Int(g_strLength);
		debug_msg("\r\nPlz pass uboot bin > ");
		uart_getBinary((char *)src_addr, uiLength);   // Temp Receive to DRAM start
		debug_msg("\r\nGot it\r\n");
#if (DRAM_RANGE_SCAN_EN == ENABLE)
		// First word is code entry point address, once if entry address is 0xC000XXXX
		// represent code is running on sram.
		if (bl_checkDramScanFW(src_addr) == TRUE) {
			UINT32 i;

			// Enable sram usage
			SETREG32(0xF0900128, 0x00000002);
			SETREG32(0xF0800128, 0x00000006);
			SETREG32(0xF0020060, 0x00030002);

			uiLength = ((uiLength + 3) & 0xFFFFFFFC);

			for (i = 0; i < uiLength; i += 4) {
				*(UINT32 *)(src_addr + i) = *(UINT32 *)(src_addr + i);
			}
			*(UINT32 *)(src_addr) = *(UINT32 *)(src_addr);
			return 1;
		}
#endif
		if (*(UINT32 *)src_addr == MAKEFOURCC('B', 'C', 'L', '1')) {
			HEADINFO *p_headinfo = NULL;
			NVTPACK_BFC_HDR *pBFC = (NVTPACK_BFC_HDR *)src_addr;
			bl_checkFW(src_addr, uiLength);
			debug_msg_var("uiLength", uiLength);
			debug_msg("compressed t.bin\r\n");
			// decode some bytes to get uncompressed address
			bl_decompress_rtos(src_addr, SIZE_PRELOAD, src_addr + uiLength);
			p_headinfo = (HEADINFO *)(src_addr + uiLength + BIN_INFO_OFFSET_RTOS);
			// check if valid after decode
			if ((strncmp(p_headinfo->BinInfo_1, "NT", 2) == 0) ||
				(strncmp(p_headinfo->BinInfo_1, "NC", 2) == 0)) {
				UINT32 uncompress_addr = p_headinfo->CodeEntry - CODE_SECTION_OFFSET;
				UINT32 uncompress_size = p_headinfo->BinLength;
				UINT32 compress_size = invertEndian(pBFC->uiSizeComp) + sizeof(NVTPACK_BFC_HDR);
				unsigned int ori_src_addr = src_addr;
				//adjust src_addr that lay compressed f.w followed by uncomppressed f.w
				src_addr = uncompress_addr + uncompress_size;
				src_addr = ALIGN_CEIL(src_addr, 4);
				src_size = uncompress_size;
				utl_memcpy((void *)src_addr, (void *)ori_src_addr, uiLength);
				*p_dst_addr = src_addr;
				*p_dst_size = compress_size;
				return 0;
			} else {
				return -1;
			}
			return 0;
		}
	}
#endif
	return -1;
}
#endif

_THUMB2 static void bl_update_uItron_headInfo(UINT32 fw_base_addr, DRAM_PARTITION  *p_dram_partition)
{
	HEADINFO *pHeadInfo;
#if (UITRON_FW == ENABLE)
	BININFO	 *pBinInfo;
#endif
	pHeadInfo = (HEADINFO *)(fw_base_addr + BIN_INFO_OFFSET_RTOS);
	if (p_dram_partition) {
		pHeadInfo->ModelextAddr = p_dram_partition->fdt_addr;
	} else {
		pHeadInfo->ModelextAddr = 0;
#if (UITRON_FW == ENABLE)
	pBinInfo = (BININFO *)(fw_base_addr + BIN_INFO_OFFSET_RTOS);
	pBinInfo->ld.Resv[0] = LoaderInternalInfo[1];
	pBinInfo->ld.LdPackage = (LoaderInternalInfo[3] & 0xFFFF);
#endif
	}
}

_THUMB2 static int bl_update_loader_bininfo(unsigned char *p_fdt, unsigned int ld_flag, unsigned int rtos_loaded_size)
{
	BOOTINFO  *p_ld;
	SHMINFO *p_shminfo = (SHMINFO *)bl_get_fdt_cfg((const void *)p_fdt, MODELEXT_TYPE_BIN_INFO);
	DRAM_PARTITION *p_dram_partition = (DRAM_PARTITION *)bl_get_fdt_cfg((const void *)p_fdt, MODELEXT_TYPE_DRAM_PARTITION);
	BININFO *p_bininfo = (BININFO *)(p_dram_partition->rtos_addr + BIN_INFO_OFFSET_RTOS);

	p_ld = &p_shminfo->boot;
	if (!(p_bininfo->head.Resv1[HEADINFO_RESV_IDX_BOOT_FLAG] & BOOT_FLAG_PARTLOAD_EN)) {
		// LdLoadSize updated on uboot or bl_load_rtos_from_flash(), if BOOT_FLAG_PARTLOAD_EN
		p_ld->LdLoadSize = rtos_loaded_size;
	}
	p_ld->LdLoadTime = timer_getLdrElapse();
	p_ld->LdResvSize = 0; //unused
	p_ld->FWResvSize = 0; //unused
	return 0;
}

_THUMB2 static int bl_is_smp(unsigned char *p_fdt)
{
#if (FDT_SUPPORT)
	int len;
	const char *nodep;
	nodep = (const char *)bl_get_fdt_property(p_fdt, PATH_NVT_INFO, PROPERTY_NVT_LINUX_SMP, &len);
	debug_msg_var("nodep", (UINT32)nodep);
	if (nodep == NULL) {
		return 0;
	}
	debug_msg((char *)nodep);
	if (strcmp(nodep, "NVT_LINUX_SMP_ON") == 0) {
		return 1;
	}
	return 0;
#else
	return 0;
#endif
}

#if !REMOVED_FLASH
_THUMB2 int bl_boot_uboot(unsigned char *p_fdt)
{
#if 0//(LOADER_TYPE == STAND_ALONE_LOADER_528) || (LOADER_TYPE == COMBINATION_528)
	UINT32			No_CPU;
#endif
	DRAM_PARTITION *p_dram_partition = (DRAM_PARTITION *)bl_get_fdt_cfg((const void *)p_fdt, MODELEXT_TYPE_DRAM_PARTITION);

	if (p_dram_partition == NULL) {
		debug_err("null p_dram_partition\r\n");
		return -1;
	}

	// update shminfo
	SHMINFO *p_shminfo = (SHMINFO *)bl_get_fdt_cfg((const void *)p_fdt, MODELEXT_TYPE_BIN_INFO);
	unsigned int *p_param = (unsigned int *)(&_load_LOADER_CONFIGRAM_FREQ_PARAM_start_base[0]);
	p_shminfo->boot.LdPackage = p_param[3] & 0xFFFF;
	p_shminfo->boot.LdStorage = (p_param[3] >> 16) & 0xFF;

	// update headinfo as real u-boot address
	HEADINFO *p_headinfo = (HEADINFO *)(p_dram_partition->uboot_addr + BIN_INFO_OFFSET_UBOOT);

	if (bl_chk_uboot(p_dram_partition->uboot_addr, p_headinfo->BinLength) != 0) {
		return -1;
	}
	// start cpu2
	if (p_dram_partition->uboot_addr != p_headinfo->CodeEntry) {
		debug_err_var("drampat-uboot_addr ", (int)p_dram_partition->uboot_addr);
		debug_err_var("bin-uboot_addr ", (int)p_headinfo->CodeEntry);
		return -1;
	}
	// for uboot to indicate this boot is all-in-one fw or non-all-in-one boot from uart or usb
	// if boot from uart or usb with non-all-in-one fw, the uboot's ModelextAddr will be zero
	p_headinfo->ModelextAddr = p_dram_partition->fdt_addr;

	CPUflushWriteCache(p_headinfo->CodeEntry, p_headinfo->BinLength);
	UINT32 isSMP = bl_is_smp(p_fdt);

#if 0//(LOADER_TYPE == STAND_ALONE_LOADER_528) || (LOADER_TYPE == COMBINATION_528)
	No_CPU = *(UINT32 *)0xFFD00004;
	No_CPU &= 0x3;
	debug_msg_var("core No.=", No_CPU+1);
#endif
	// init cpu timer
//	bl_cpu_timer_init(CPU_TIMER_SETTING);
#if 0//(LOADER_TYPE == STAND_ALONE_LOADER_528) || (LOADER_TYPE == COMBINATION_528)
   	if (!isSMP || !No_CPU)
#else
	if (!isSMP)
#endif
    {
#if 0//(LOADER_TYPE == STAND_ALONE_LOADER_528) || (LOADER_TYPE == COMBINATION_528)
        debug_msg_var("Bin core=", isSMP+1);
#endif
		bl_entry_boot(p_fdt, p_headinfo->CodeEntry);
	} else {
#if 0//(LOADER_TYPE == STAND_ALONE_LOADER_528) || (LOADER_TYPE == COMBINATION_528)
        // init cpu timer
		bl_cpu_timer_init(global_timer_freq);

		bl_smp_start(p_fdt, p_headinfo->CodeEntry);
		// boot u-boot and never return back to loader
#else
		//bl_smp_start(p_fdt, p_headinfo->CodeEntry);
		debug_err("not support smp\r\n");
#endif
	}
	return 0;
}

#if (NUTTX_SUPPORT)
_THUMB2 static int bl_boot_nuttx(unsigned char *p_fdt)
{
	DRAM_PARTITION *p_dram_partition = (DRAM_PARTITION *)bl_get_fdt_cfg((const void *)p_fdt, MODELEXT_TYPE_DRAM_PARTITION);

	if (p_dram_partition == NULL) {
		debug_err("null p_dram_partition\r\n");
		return -1;
	}

	// update headinfo as real u-boot address
	HEADINFO *p_headinfo = (HEADINFO *)(p_dram_partition->nuttx_addr + BIN_INFO_OFFSET_NUTTX);

	if (bl_chk_uboot(p_dram_partition->nuttx_addr, p_headinfo->BinLength) != 0) {
		return -1;
	}

	// start cpu2
	if (p_dram_partition->nuttx_addr != p_headinfo->CodeEntry) {
		debug_err_var("drampat-nuttx_addr ", (int)p_dram_partition->nuttx_addr);
		debug_err_var("bin-nuttx_addr ", (int)p_headinfo->CodeEntry);
		return -1;
	}

	CPUflushWriteCache(p_headinfo->CodeEntry, p_headinfo->BinLength);

	// init cpu timer
	bl_cpu_timer_init(CPU_TIMER_SETTING);

	bl_entry_boot(p_fdt, p_headinfo->CodeEntry);
	// boot nuttx and never return back to loader
	return 0;
}
#endif

_THUMB2 static int bl_boot_teeos(unsigned char *p_fdt)
{
	DRAM_PARTITION *p_dram_partition = (DRAM_PARTITION *)bl_get_fdt_cfg((const void *)p_fdt, MODELEXT_TYPE_DRAM_PARTITION);

	if (p_dram_partition == NULL) {
		debug_err("null p_dram_partition\r\n");
		return -1;
	}

	// update headinfo as real u-boot address
	HEADINFO *p_headinfo = (HEADINFO *)(p_dram_partition->teeos_addr + BIN_INFO_OFFSET_TEEOS);

	// update uboot addr for teeos
	p_headinfo->Resv1[HEADINFO_TEEOS_RESV_IDX_UBOOT_ADDR] = p_dram_partition->uboot_addr;


#if 0 //do not checksum, because teeos header has removed cause checksum failed
	if (bl_chk_uboot(p_dram_partition->teeos_addr, p_headinfo->BinLength) != 0) {
		return -1;
	}
#endif

	//flush uboot
	CPUflushWriteCache(p_dram_partition->uboot_addr, p_dram_partition->uboot_size);
	//flush teeos
	CPUflushWriteCache(p_headinfo->CodeEntry, p_headinfo->BinLength);

	// init cpu timer

#if 0//(LOADER_TYPE == STAND_ALONE_LOADER_528) || (LOADER_TYPE == COMBINATION_528)
   	bl_cpu_timer_init(CPU_TIMER_SETTING);
	// boot core2 (after teeos is loaded)
	if (HEADINFO_UBOOT(p_dram_partition)->BinCtrl & 0x00000002) {
		//if SMP, trigger core2
		debug_msg("smp(tee)\r\n");
		bl_core2_prepare(p_dram_partition, p_headinfo->CodeEntry);
		//invalid Instruciton and data cache
		CPUCleanInvalidateDCacheAll();
		bl_core2_reset();
	} else {
		debug_msg_var("not smp\r\n", p_headinfo->BinCtrl);
	}
#endif
	bl_entry_boot(p_fdt, p_headinfo->CodeEntry);
	// boot teeos and never return back to loader
	return 0;
}

_THUMB2 static int bl_update_loader_flag(unsigned char *p_fdt, UINT32 uiLoaderFunc)
{
	SHMINFO *p_bininfo = (SHMINFO *)bl_get_fdt_cfg((const void *)p_fdt, MODELEXT_TYPE_BIN_INFO);

	utl_memcpy(p_bininfo->boot.LdInfo_1, "LD_NVT", 6);

	p_bininfo->boot.LdCtrl2 = 0;
	if (uiLoaderFunc & FUNC_UPDATE_FW) {
		p_bininfo->boot.LdCtrl2 |= LDCF_UPDATE_FW;
	}
	if (uiLoaderFunc & FUNC_UPDATE_LOADER) {
		p_bininfo->boot.LdCtrl2 |= LDCF_UPDATE_LD;
	}
	if (uiLoaderFunc & FUNC_RUN_CARD) {
		p_bininfo->boot.LdCtrl2 |= LDCF_BOOT_CARD;
	}
	if (uiLoaderFunc & FUNC_RUN_FLASH) {
		p_bininfo->boot.LdCtrl2 |= LDCF_BOOT_FLASH;
	}
	//CPUflushWriteCache((UINT32)p_bininfo, sizeof(BININFO));
	debug_msg_var("LdCtrl2", p_bininfo->boot.LdCtrl2);

	unsigned int ver;
	SHMINFO *p_shminfo = (SHMINFO *)bl_get_fdt_cfg((const void *)p_fdt, MODELEXT_TYPE_BIN_INFO);
	BOOTINFO  *p_ld = &p_shminfo->boot;

	utl_memset(p_ld->LdInfo_1, 0, sizeof(p_ld->LdInfo_1));
	utl_memcpy(p_ld->LdInfo_1, "LD_NVT", 6);

	ver = (((LoaderInternalInfo[1] >> 28) & 0xF) << 24) |
		  (((LoaderInternalInfo[1] >> 24) & 0xF) << 16) |
		  ((LoaderInternalInfo[1] >> 16) & 0xFF);

	if (g_uiVersion == 0) {
		g_uiVersion = ver;
	}

	utl_memcpy(&p_ld->LdInfo_1[8], &g_uiVersion, sizeof(g_uiVersion));
	unsigned int *p_param = (unsigned int *)(&_load_LOADER_CONFIGRAM_FREQ_PARAM_start_base[0]);
	p_ld->LdPackage = p_param[3] & 0xFFFF;
	p_ld->LdStorage = (p_param[3] >> 16) & 0xFF;

	return 0;
}

// return uItron_fw_addr
_THUMB2 unsigned int bl_process_all_in_one(UINT32 uiFwBuf, UINT32 uiFwBufSize, DRAM_PARTITION **pOut_dram_partition, UINT32 uiLoaderFunc, UINT32 *p_comp_addr, UINT32 *p_comp_size)
{
	int er;
	unsigned char *p_fdt = NULL;
	DRAM_PARTITION *p_dram_partition = NULL;
	//p_tmp for the case of all-in-one without fdt or uboot
	unsigned char *p_tmp = (unsigned char *)(uiFwBuf + ALIGN_CEIL(uiFwBufSize, 4));

	// load fdt
	er = bl_load_fdt_from_all_in_one((unsigned char *)uiFwBuf, uiFwBufSize, &p_fdt);
	if (er == -2) { // try to load from nand
		debug_msg("fdt from flash.\r\n");
		// open flash
		if (bl_flash_open() != 0) { // dont move flash open outside section, consider that T without flash device.
			bl_displayErrMsg("flash open failed\r\n");
		}

		//p_tmp for the case of all-in-one without fdt or uboot
		unsigned char *p_tmp = (unsigned char *)(uiFwBuf + ALIGN_CEIL(uiFwBufSize, 4));
		er = bl_load_fdt_from_flash(p_tmp, 0x2000000, &p_fdt); // dtb size less than 32MB to be safer.
		if (er != 0) {
			bl_displayErrMsg("load fdt failed\r\n");
		}
	}

	// update loader flag
	bl_update_loader_flag(p_fdt, uiLoaderFunc);
	if (uiLoaderFunc & FUNC_UPDATE_FW) {
		SHMINFO *p_bininfo = (SHMINFO *)bl_get_fdt_cfg(p_fdt, MODELEXT_TYPE_BIN_INFO);
		if (p_bininfo == NULL) {
			debug_err("null p_bininfo\r\n");
			return -1;
		}
		p_bininfo->comm.Resv[5] = uiFwBuf; // COMM_FW_UPD_ADDR
		p_bininfo->comm.Resv[6] = uiFwBufSize; // COMM_FW_UPD_LEN
		debug_msg_var("upd_src_addr=", uiFwBuf);
		debug_msg_var("upd_src_size=", uiFwBufSize);
	}

	p_dram_partition = (DRAM_PARTITION *)bl_get_fdt_cfg(p_fdt, MODELEXT_TYPE_DRAM_PARTITION);

	//check loader addr is matched with loader self.
	extern char _loader_exec_compres_start[];
	if (p_dram_partition->loader_addr != (UINT32)_loader_exec_compres_start) {
		debug_msg_var("p_dram_partition->loader_addr", p_dram_partition->loader_addr);
		debug_msg_var("_loader_exec_compres_start", (UINT32)_loader_exec_compres_start);
		bl_displayErrMsg("loader addr is not match.");
	}
	//when rtos boot from flash, the boot from loader directly,
	//but when rtos need to burn image, the uboot is still required.
	//so, any one need to burn image, uboot is always needed to boot.
	//if ((uiLoaderFunc & FUNC_UPDATE_FW) || p_dram_partition->rtos_addr == 0) {
	if (1) {
		// always use uboot to handle all-in-one bin
		if (p_dram_partition->nuttx_size == 0 && p_dram_partition->teeos_size == 0) {
			// load uboot
			er = bl_load_uboot_from_all_in_one((unsigned char *)p_dram_partition->fdt_addr, (unsigned char *)uiFwBuf, uiFwBufSize);
			if (er == -2) { // try to load from nand
				debug_msg("uboot from flash.\r\n");
				// open flash
				if (bl_flash_open() != 0) { // dont move flash open outside section, consider that T without flash device.
					bl_displayErrMsg("flash open failed\r\n");
				}
				er = bl_load_uboot_from_flash((unsigned char *)p_dram_partition->fdt_addr, p_tmp);
				if (er != 0) {
					bl_displayErrMsg("load uboot failed\r\n");
				}
			}

			er = bl_boot_uboot((unsigned char *)p_dram_partition->fdt_addr);
			if (er != 0) {
				bl_displayErrMsg("boot uboot failed\r\n");
			}
		} else if (p_dram_partition->nuttx_size) {
#if (NUTTX_SUPPORT)
			// load nuttx
			er = bl_load_nuttx_from_all_in_one((unsigned char *)p_dram_partition->fdt_addr, (unsigned char *)uiFwBuf, uiFwBufSize);
			if (er != 0) {
				bl_displayErrMsg("load nuttx failed\r\n");
			}
			er = bl_boot_nuttx((unsigned char *)p_dram_partition->fdt_addr);
			if (er != 0) {
				bl_displayErrMsg("boot nuttx failed\r\n");
			}
#else
			bl_displayErrMsg("NUTTX_SUPPORT disabled\r\n");
#endif
		} else if (p_dram_partition->teeos_size) {
			// load teeos
			er = bl_load_teeos_from_all_in_one((unsigned char *)p_dram_partition->fdt_addr, (unsigned char *)uiFwBuf, uiFwBufSize);
			if (er != 0) {
				bl_displayErrMsg("load teeos failed\r\n");
			}
			// load uboot
			er = bl_load_uboot_from_all_in_one((unsigned char *)p_dram_partition->fdt_addr, (unsigned char *)uiFwBuf, uiFwBufSize);
			if (er != 0) {
				bl_displayErrMsg("load uboot failed\r\n");
			}
			// boot teeos
			er = bl_boot_teeos((unsigned char *)p_dram_partition->fdt_addr);
			if (er != 0) {
				bl_displayErrMsg("boot teeos failed\r\n");
			}
		}
		return er;
	} else {
		// update rtos information
		// the following is for uncompressed-rtos only, others needing uboot
		SHMINFO *p_bininfo = (SHMINFO *)bl_get_fdt_cfg((const void *)p_dram_partition->fdt_addr, MODELEXT_TYPE_BIN_INFO);

		if (p_bininfo == NULL) {
			debug_err("null p_bininfo\r\n");
			return -1;
		}
		*p_comp_addr = p_bininfo->comm.Resv[3]; // COMM_UITRON_COMP_ADDR
		*p_comp_size = p_bininfo->comm.Resv[4]; // COMM_UITRON_COMP_LEN

		*pOut_dram_partition = p_dram_partition;
		return bl_load_rtos_from_all_in_one((unsigned char *)uiFwBuf, uiFwBufSize, &p_fdt);
	}
}
#endif

// return uItron_fw_addr, no need to fully decode
_THUMB2 static unsigned int bl_process_rtos_only(UINT32 uiFwBuf, UINT32 uiFwBufSize, UINT32 uiLoaderFunc, UINT32 *p_comp_addr, UINT32 *p_comp_size)
{
	UINT32              uItron_fw_addr;
	NVTPACK_BFC_HDR     *pBFC;
	HEADINFO            *p_headinfo;

	debug_msg("\r\nbl_process_rtos_only\r\n");
	pBFC = (NVTPACK_BFC_HDR *)uiFwBuf;
	if (pBFC->uiFourCC == MAKEFOURCC('B', 'C', 'L', '1')) {
		UINT32 compressSize;
		unsigned char *p_tmp = (unsigned char *)(uiFwBuf + ALIGN_CEIL(uiFwBufSize, 4));
		LZ_Un_compress((UINT8 *)uiFwBuf + LDC_HEADER_SIZE, p_tmp, SIZE_PRELOAD);
		p_headinfo = (HEADINFO *)(p_tmp + BIN_INFO_OFFSET_RTOS);
		uItron_fw_addr = p_headinfo->CodeEntry - CODE_SECTION_OFFSET;
#if UITRON_FW
		uItron_fw_addr = p_headinfo->CodeEntry - CODE_ENTRY_OFFSET; // cliff
#endif

		if ((uItron_fw_addr & 0x0000FFFF) != 0) { //cc engine's limitation
			debug_err_var("rtos addr must match (&0x0000FFFF)==0", uItron_fw_addr); // but 660 allow
			uItron_fw_addr = uItron_fw_addr & 0xFFFF0000;
		}
		compressSize = invertEndian(pBFC->uiSizeComp) + sizeof(NVTPACK_BFC_HDR);
		//debug_dump_addr(tmpBuf,0x200);
		debug_msg_var("F compress uItron_fw_addr", uItron_fw_addr);
		// uiFwBuf has adjusted on bl_load_rtos_from_non_nvtpack()
		*p_comp_addr = uiFwBuf;
		*p_comp_size = compressSize;
	} else {
		p_headinfo = (HEADINFO *)(uiFwBuf + BIN_INFO_OFFSET_RTOS);
		uItron_fw_addr = p_headinfo->CodeEntry - CODE_SECTION_OFFSET;
#if UITRON_FW
		uItron_fw_addr = p_headinfo->CodeEntry - CODE_ENTRY_OFFSET; // cliff
#endif

		if ((uItron_fw_addr & 0x0000FFFF) != 0) { //cc engine's limitation
			debug_err_var("rtos addr must match (&0x0000FFFF)==0", uItron_fw_addr); // but 660 allow
			uItron_fw_addr = uItron_fw_addr & 0xFFFF0000;
		}

		debug_msg_var("Normal uItron_fw_addr", uItron_fw_addr);
		*p_comp_addr = 0;
		*p_comp_size = 0;
	}
	return uItron_fw_addr;
}
#if !UPDATE_EMU_CODE && !REMOVED_FLASH
_THUMB2 static unsigned int bl_process_flash_boot(unsigned char *p_tmp, DRAM_PARTITION **pOut_dram_partition, UINT32 uiLoaderFunc, UINT32 *p_comp_addr, UINT32 *p_comp_size)
{
	int er;
	unsigned char *p_fdt = NULL;
	DRAM_PARTITION *p_dram_partition = NULL;

	// open flash
	if (bl_flash_open() != 0) {
		bl_displayErrMsg("flash open failed\r\n");
	}

#if (FDT_SUPPORT)
	// load fdt
	er = bl_load_fdt_from_flash(p_tmp, SDRAM_Start_FW, &p_fdt); // dtb size less than 32MB to be safer.
	if (er != 0) {
		bl_displayErrMsg("load fdt failed\r\n");
	}
#endif
	// update loader flag
	bl_update_loader_flag(p_fdt, uiLoaderFunc);

	p_dram_partition = (DRAM_PARTITION *)bl_get_fdt_cfg(p_fdt, MODELEXT_TYPE_DRAM_PARTITION);

	if (p_dram_partition->teeos_size) {
			// load teeos
			er = bl_load_teeos_from_flash((unsigned char *)p_dram_partition->fdt_addr, p_tmp);
			if (er != 0) {
				bl_displayErrMsg("load teeos failed\r\n");
			}
			// load uboot
			er = bl_load_uboot_from_flash((unsigned char *)p_dram_partition->fdt_addr, p_tmp);
			if (er != 0) {
				bl_displayErrMsg("load uboot failed\r\n");
			}
			// boot teeos
			er = bl_boot_teeos((unsigned char *)p_dram_partition->fdt_addr);
			if (er != 0) {
				bl_displayErrMsg("boot teeos failed\r\n");
			}
	} else if (p_dram_partition->rtos_addr == 0 || DUAL_RTOS_SUPPORT) {
		// load uboot
		er = bl_load_uboot_from_flash((unsigned char *)p_dram_partition->fdt_addr, p_tmp);
		if (er != 0) {
			bl_displayErrMsg("load uboot failed\r\n");
		}

		// boot uboot
		er = bl_boot_uboot((unsigned char *)p_dram_partition->fdt_addr);
		if (er != 0) {
			bl_displayErrMsg("boot uboot failed\r\n");
		}
	 } else {
		// load rtos
		er = bl_load_rtos_from_flash((unsigned char *)p_dram_partition->fdt_addr, p_tmp);
		if (er != 0) {
			bl_displayErrMsg("load rtos failed\r\n");
		}
	}

	// update compressed rtos information
	SHMINFO *p_bininfo = (SHMINFO *)bl_get_fdt_cfg((unsigned char *)p_dram_partition->fdt_addr, MODELEXT_TYPE_BIN_INFO);
	if (p_bininfo == NULL) {
		debug_err("null p_bininfo\r\n");
		return -1;
	}
	*p_comp_addr = p_bininfo->comm.Resv[3]; // COMM_UITRON_COMP_ADDR
	*p_comp_size = p_bininfo->comm.Resv[4]; // COMM_UITRON_COMP_LEN

	*pOut_dram_partition = p_dram_partition;
	return p_dram_partition->rtos_addr;
}
#endif

/**
    bl_process_update_loader.

    Write loader will update loader binary file

    @param[in] loader_addr   loader in DRAM starting address
    @param[in] loader_size   loader code length (from loader header offset 0x24)
    @return void
*/
_THUMB2 static int bl_process_update_loader(unsigned int loader_addr, unsigned int loader_size)
{
	unsigned int reload_addr;

	// Check boot loader read from SD card
	if(is_data_area_encrypted() == 0) {

#if ((STORAGE_EXT_TYPE == STORAGE_EXT_ETH)|(STORAGE_EXT_TYPE == STORAGE_EXT_USB))
	//UINT32 uiOffset;
	//uiOffset = *((UINT32 *)(loader_addr + 0x80));
	//if(uiOffset) {//Combo loader
	//	if(*(UINT32*)0xF00100F0 == 0x50210000) {
	//		debug_msg("combo loader 528\r\n");
	//		bl_checkLoader(loader_addr+uiOffset, COMBINATION_LOADER_SIZE - uiOffset); //check 528
	//	} else{
	//		debug_msg("combo loader 52X\r\n");
	//		bl_checkLoader(loader_addr, loader_size);  //check 52x
	//	}
	//}else
		bl_checkLoader(loader_addr, loader_size);
#else
#if 0//(LOADER_TYPE == COMBINATION_528)
        debug_msg("CB8\r\n");
		bl_checkLoader(loader_addr + loader_size, (COMBINATION_LOADER_SIZE - loader_size));
#else	//STAND_ALONE_LOADER or combination 52x loader
        //debug_msg("STD\r\n");
		bl_checkLoader(loader_addr, loader_size);
#endif
#endif
	}
	debug_msg("update loader\r\n");

	// open flash
	if (bl_flash_open() != 0) {
		bl_displayErrMsg("flash open failed\r\n");
	}

	// Program loader
#if 0
#if ((STORAGE_EXT_TYPE == STORAGE_EXT_ETH)|(STORAGE_EXT_TYPE == STORAGE_EXT_USB))
	UINT32 uiOffset;
	uiOffset = *((UINT32 *)(loader_addr + 0x80));
	if(uiOffset) //Combo loader
		loader_size = COMBINATION_LOADER_SIZE;
	//If single loader , it already got
#else
#if (LOADER_TYPE == STAND_ALONE_LOADER_560) || (LOADER_TYPE == STAND_ALONE_LOADER_528)
#else	//Combination loader
	loader_size = COMBINATION_LOADER_SIZE;
#endif
#endif
#endif
	if (int_strg_obj->flash_writeSectors(StartNandBlkUpdateLoader, loader_size, (UINT8 *)loader_addr, NAND_RW_LOADER) < 0) {
		bl_displayErrMsg(RWErrorMsg);
	}
	// Read back
	reload_addr = loader_addr + loader_size;
	if (int_strg_obj->flash_readSectors(StartNandBlkUpdateLoader, loader_size, (UINT8 *)reload_addr, NAND_RW_LOADER) < 0) {
		bl_displayErrMsg("rd fail\r\n");
	}
	// Verify
	if (memcmp((void *)loader_addr, (void *)reload_addr, loader_size) != 0) {
		bl_displayErrMsg("verify fail\r\n");
	}
	return 0;
}
#if UPDATE_EMU_CODE
_THUMB2 static int bl_process_update_emu_firmware(unsigned int emu_addr, unsigned int emu_size)
{
	unsigned int reload_addr;

	// Check boot loader read from SD card
//	bl_checkFW(emu_addr, emu_size);
	debug_msg("update emu firmware size");
	uart_putSystemUARTStr(Dec2HexStr(emu_size));
	uart_putSystemUARTStr("\r\n");

	// open flash
	if (bl_flash_open() != 0) {
		bl_displayErrMsg("flash open failed\r\n");
	}

	// Program firmware
	if (int_strg_obj->flash_writeSectors(g_uiStartBlkUpdateFW, emu_size, (UINT8 *)emu_addr, NAND_RW_FIRMWARE) < 0) {
		bl_displayErrMsg(RWErrorMsg);
	}
	// Read back
	reload_addr = emu_addr + emu_size;
	if (int_strg_obj->flash_readSectors(g_uiStartBlkUpdateFW, emu_size, (UINT8 *)reload_addr, NAND_RW_FIRMWARE) < 0) {
		bl_displayErrMsg("rd fail\r\n");
	}
	// Verify
	if (memcmp((void *)emu_addr, (void *)reload_addr, emu_size) != 0) {
		bl_displayErrMsg("verify fail\r\n");
	}
	return 0;
}
#endif

_THUMB2 void bl_read_rtos_addr(UINT32 *pLoadAddr, UINT32 *pTargetAddr, UINT32 *pSize)
{
	*pLoadAddr = g_rtos_load_addr;
	*pTargetAddr = g_rtos_target_addr;
	*pSize = g_rtos_size;
}

/**
    main flow code

    If there is Calibration Firmware code store in NAND, running flow as follow
    O's work flow

    @return fw base addr
*/

_THUMB2 UINT32 bl_mainFlow(void)
{
	UINT32  uiUpdateFileLen = 0;
	UINT32  uiLoaderFunc = 0;
	UINT32  uiLoaderSize = 32 * 1024; // pre-assume 32KB, actual size is parsed from loader
	//Show Duty calibration log
#if (_LOADER_DUTY_CALIBRATION_ == ENABLE && _LOADER_DUTY_CALIBRATION_LOG_ == ENABLE)
	UINT32  uiLoaderAddress;
	UINT32  uiLogSramAddress;
#endif

#if !(USB_WRITELOADER || UART_UPDATE)
	unsigned int adjusted_addr = 0;
	unsigned int adjusted_size = 0;
#endif
	UINT32 uiFwBaseAddr = SDRAM_Start_FW;       // FW base address
	DRAM_PARTITION *p_dram_partition = NULL;

	// BaseOfStack is initialized at doRemapLZ.s
	UINT32 uiheapBufferAddr           = (UINT32)_loader_heap_base;
//	UINT32 uiheapBufferAddr           = BaseOfStack + 0x40000;        // reserve 16KB for tmp buffer usage
	UINT32 uiTmpBufferAddr            = uiheapBufferAddr + FAT_HEAP_BUFFER_SIZE;
	UINT32 uiUpdateBootloaderBufAddr  = uiTmpBufferAddr + 0x4000;
	UINT32 uiUpdateMainBinBufAddr     = SDRAM_Start_FW;

	// UART initial sequence
	//uart_openSystemUART();
	// rtc reset shutdown timer
	// rtc_resetShutDownTimer();




#if 0
	// adjust PAD driving (if required)
	bl_adjustDriving();
	/*
	    - @b RTC_PWR_SW_STS: Power on source is PWR_SW
	    - @b RTC_PWR_VBAT_STS: Power on source is PWR_VBAT
	    - @b RTC_PWR_VBUS_STS: Power on source is PWR_VBUS
	*/
	uiPowerOnSrc = rtc_getPWRONSource();

	if (uiPowerOnSrc & RTC_PWR_SW_STS) {
		uart_putSystemUARTStr("\r\nSW PON\r\n");
	} else if (uiPowerOnSrc & RTC_PWR_VBAT_STS) {
		uart_putSystemUARTStr("\r\nVBAT PON\r\n");
	} else if (uiPowerOnSrc & RTC_PWR_VBUS_STS) {
		uart_putSystemUARTStr("\r\nVBUS PON\r\n");
	} else if (rtc_getIsAlarmPowerOn()) {
		uart_putSystemUARTStr("\r\nPwrAlarm PON\r\n");
	} else {
		uart_putSystemUARTStr("\r\nPOR PON\r\n");
	}
#endif
	// Display Loader Version
	debug_msg((char *)LOADER_START_STR);
	UTL_setDrvTmpBufferAddress(uiTmpBufferAddr);


#if 0 // for now, reduce code size
	if (rtc_chkS3boot()) {
		UINT32 resume_addr;

		uart_putSystemUARTStr("main selfing..\r\n");
		resume_addr = bl_resume_cpu1((unsigned char *)_BOARD_IPC_ADDR_);
		if (resume_addr == 0) {
			// in codition for MODELEXT_BUILT_IN_ON
			resume_addr = RESUME_ADDR;
		}
		return resume_addr;
	} else {

		uart_putSystemUARTStr("main not selfing..\r\n");
	}
#endif

#if 0
	// Sample to hook spi flash extending function
	flash_installIdentifyCB(bl_spiIdentify);
#endif
	prj_main();

#if !(USB_WRITELOADER)
	set_usb_suspend();
#endif

	if(utl_get_bootsrc() == BOOT_SOURCE_UART)
	{
#if UART_UPDATE
		debug_msg("Boot from UART ...\r\n");
		bl_uart();//never returned.
#else
		bl_displayErrMsg("UART_UPDATE must enable on loader\r\n");
#endif
	}

	if(USB_WRITELOADER || utl_get_bootsrc() == BOOT_SOURCE_USB)
	{
#if USB_WRITELOADER
		debug_msg("Boot from USB ...\r\n");
		bl_usb(); //never returned.
#else
		bl_displayErrMsg("USB_WRITELOADER must enable on loader\r\n");
#endif
	}

#if !(USB_WRITELOADER || UART_UPDATE)
	if (bl_load_rtos_from_uart(uiUpdateMainBinBufAddr, FAT_READ_TOTAL_FILE_LENGTH, &adjusted_addr, &adjusted_size) == 0) {
		//specail case, load small rtos from uart
		uiUpdateMainBinBufAddr = adjusted_addr;
		uiUpdateFileLen = adjusted_size;
		uiLoaderFunc |= FUNC_RUN_CARD;
	} else if ((int_strg_obj->flash_getBlockSize() == EMMC_BLOCK_SIZE) && gRecoveryTriggerCallBack && gRecoveryTriggerCallBack()) {
		debug_msg("Recovery triggered not support currently.\r\n");
#if 0
		// open flash
		if (bl_flash_open() != 0) {
			bl_displayErrMsg("flash open failed\r\n");
		} else {
			flash_mount_fs(0, BaseOfStack + 0x4000, FAT_HEAP_BUFFER_SIZE);
			if (flash_mount_partition(g_uiPartitionID) == E_OK) {
				if (flash_open_file(RECOVERY_FW_NAME) == TRUE) {
					NVTPACK_MEM mem_in ;
					flash_read_file((UINT8 *)uiUpdateMainBinBufAddr, SIZE_PRELOAD);
					mem_in.p_data = (void *)uiUpdateMainBinBufAddr;
					mem_in.len = uiUpdateFileLen;
					// all in one bin
					if (bl_chk_valid_all_in_one(&mem_in) == 0) {
						// Read all
						uiUpdateFileLen = flash_read_file((UINT8 *)uiUpdateMainBinBufAddr, FAT_READ_TOTAL_FILE_LENGTH);
						uiLoaderFunc |= FUNC_UPDATE_FW;
						g_is_recovery_triggered = TRUE;
					} else {
						debug_msg("Recovery should be all in one bin!\r\n");
					}
				} else {
					debug_msg("no recovery bin!\r\n");
				}
			} else {
				debug_msg("Partition error!\r\n");
			}
		}
#endif
	} else if (((gSpecialKeyCallBack == NULL) || gSpecialKeyCallBack()) &&
			   ((gCardDetectCallBack == NULL) || gCardDetectCallBack())) {
		if (card_open() == TRUE && fat_initFAT(uiheapBufferAddr, FAT_HEAP_BUFFER_SIZE) == TRUE) {
#if UPDATE_SIM_CODE
			BOOL bWDTInit = UTL_canUpdateSecKey();
			if (bWDTInit && fat_open_rootfile(RUN_WRKEY_NAME) == TRUE) {
				debug_msg("sim.bin exist\r\n"); // the others A or T are skipped.
				// Read byte count specified in file directory entry
				uiUpdateFileLen = fat_read_rootfile((UINT8 *)uiUpdateMainBinBufAddr, FAT_READ_TOTAL_FILE_LENGTH);
				fat_close_rootfile();
				debug_msg("\r\n"); // for line end RRRRRRR....
				uiLoaderFunc |= FUNC_RUN_WRBIN;
			} else { // exclude others update if FUNC_RUN_WRBIN is existing.
#endif
				// Update loader or not, loader is fixed to 16 KB
				if (fat_open_rootfile(UPDATE_LOADER_NAME) == TRUE) {
					// Read byte count specified in file directory entry
					fat_read_rootfile((UINT8 *)uiUpdateBootloaderBufAddr, FAT_READ_TOTAL_FILE_LENGTH);
					fat_close_rootfile();
					debug_msg("\r\n"); // for line end RRRRRRR....
					uiLoaderFunc |= FUNC_UPDATE_LOADER;
					uiLoaderSize = *((UINT32 *)(uiUpdateBootloaderBufAddr + 0x24));
				}
				// "Update FW" or "Run FW" function
				//  Update FW has higher priority
				if (fat_open_rootfile(UPDATE_FW_NAME) == TRUE) {
					NVTPACK_MEM mem_in ;
					fat_read_rootfile((UINT8 *)uiUpdateMainBinBufAddr, SIZE_PRELOAD);
					mem_in.p_data = (void *)uiUpdateMainBinBufAddr;
					mem_in.len = uiUpdateFileLen;
					// all in one bin
					if (bl_chk_valid_all_in_one(&mem_in) == 0) {
						// Read all
						uiUpdateFileLen = fat_read_rootfile((UINT8 *)uiUpdateMainBinBufAddr, FAT_READ_TOTAL_FILE_LENGTH);
						uiLoaderFunc |= FUNC_UPDATE_FW;
					} else {
						// only rtos
						debug_msg("not all-in-one, force behavior as T bin.\r\n");
						adjusted_addr = 0;
						adjusted_size = 0;
						if (bl_load_rtos_from_non_nvtpack(uiUpdateMainBinBufAddr, uiUpdateFileLen, &adjusted_addr, &adjusted_size) == 0) {
							uiFwBaseAddr = adjusted_addr; //fix compressed fit bl_checkDramScanFW() after copy its to temp area, see commit log
							uiUpdateFileLen = adjusted_size;
						} else {
							bl_displayErrMsg("invalid firmware");
						}
#if UPDATE_EMU_CODE
						uiLoaderFunc |= FUNC_UPDATE_FW;
#else
						uiLoaderFunc |= FUNC_RUN_CARD;
#endif

					}
					fat_close_rootfile();
					debug_msg("\r\n"); // for line end RRRRRRR....
				}
				// Run FW has lower priority
				else if (fat_open_rootfile(RUN_FW_NAME) == TRUE) {
					NVTPACK_MEM mem_in ;
					uiUpdateFileLen = fat_read_rootfile((UINT8 *)uiUpdateMainBinBufAddr, SIZE_PRELOAD);
					mem_in.p_data = (void *)uiUpdateMainBinBufAddr;
					mem_in.len = uiUpdateFileLen;
					if (bl_chk_valid_all_in_one(&mem_in) == 0) {
						// Read all
						uiUpdateFileLen = fat_read_rootfile((UINT8 *)uiUpdateMainBinBufAddr, FAT_READ_TOTAL_FILE_LENGTH);
					} else {
						// Read rtos
						unsigned int adjusted_addr = 0;
						unsigned int adjusted_size = 0;
						if (bl_load_rtos_from_non_nvtpack(uiUpdateMainBinBufAddr, uiUpdateFileLen, &adjusted_addr, &adjusted_size) != 0) {
							bl_displayErrMsg("invalid firmware");
						} else {
							uiFwBaseAddr = adjusted_addr; //fix compressed fit bl_checkDramScanFW() after copy its to temp area, see commit log
							uiUpdateFileLen = adjusted_size;
						}
						uiLoaderFunc |= FUNC_RUN_CARD;
					}
					fat_close_rootfile();
					debug_msg("\r\n"); // for line end RRRRRRR....
					uiLoaderFunc |= FUNC_RUN_CARD;
				}
#if UPDATE_SIM_CODE
			}
#endif
			card_close();
		} else {
			debug_msg("card open fail\r\n");
//			while (1);
		}
	} else if (((gSpecialKeyCallBack == NULL) || gSpecialKeyCallBack()) &&
			   ((gCardDetectCallBack != NULL) || (gCardDetectCallBack() == FALSE))) {
		debug_msg("No card inserted\r\n");
	}
#endif

	if (uiLoaderFunc & FUNC_UPDATE_LOADER) {
		bl_process_update_loader(uiUpdateBootloaderBufAddr, uiLoaderSize);
	}

	// Run FW
	UINT32 comp_addr = 0;
	UINT32 comp_size = 0;
	if (uiLoaderFunc & (FUNC_RUN_CARD | FUNC_UPDATE_FW)) {
		debug_msg("RC\r\n");
		NVTPACK_MEM mem_in ;
		int chk_valid_all_in_one;
#if (DRAM_RANGE_SCAN_EN == ENABLE)
		// First word is code entry point address, once if entry address is 0xC000XXXX
		// represent code is running on sram.
		debug_msg("Check SRAM fw\r\n");
		if (bl_checkDramScanFW(uiUpdateMainBinBufAddr) == TRUE) {

			debug_msg("This fw is on SRAM\r\n");
			// Enable sram usage
			SETREG32(0xF0900128, 0x00000002);
			SETREG32(0xF0800128, 0x00000006);
			SETREG32(0xF0020060, 0x00030002);

			debug_msg(Dec2HexStr(*((UINT32 *)SRAM_Start_FW)));
			debug_msg("before jump\r\n");
//          while (b_debug_go == FALSE);
			load_dram_scan(uiUpdateMainBinBufAddr, uiUpdateFileLen);
			return *((UINT32 *)SRAM_Start_FW);
		}
#endif
		mem_in.p_data = (void *)uiUpdateMainBinBufAddr;
		mem_in.len = uiUpdateFileLen;
		chk_valid_all_in_one = bl_chk_valid_all_in_one(&mem_in);
		if (chk_valid_all_in_one == 0) {
			// all-in-one flow
			// File len got from fat_read_rootfile() may exceed actual file size.
			// In such condition, we should use info in NVTPACK_FW_HDR2
			if (((NVTPACK_FW_HDR2 *)uiUpdateMainBinBufAddr)->TotalSize < uiUpdateFileLen) {
				uiUpdateFileLen = ((NVTPACK_FW_HDR2 *)uiUpdateMainBinBufAddr)->TotalSize;
				mem_in.len = uiUpdateFileLen;
			}
#if (REMOVED_FLASH == ENABLE)
            bl_displayErrMsg("cannot process all-in-one fw");
#else
			uiFwBaseAddr = bl_process_all_in_one(uiUpdateMainBinBufAddr, uiUpdateFileLen, &p_dram_partition, uiLoaderFunc, &comp_addr, &comp_size);
#endif
		} else {
			// non-all-in-one flow
			// check valid
			if (uiLoaderFunc & FUNC_UPDATE_FW) {
#if UPDATE_EMU_CODE
				bl_process_update_emu_firmware(uiUpdateMainBinBufAddr, uiUpdateFileLen);
#else
				bl_displayErrMsg("cannot write non-all-in-one fw");
#endif
			}
			uiFwBaseAddr = bl_process_rtos_only(uiUpdateMainBinBufAddr, mem_in.len, uiLoaderFunc, &comp_addr, &comp_size);
		}

		// here uiFwBaseAddr is ready to use.
		if (comp_addr == 0) {
			HEADINFO *p_headinfo = (HEADINFO *)(uiFwBaseAddr + BIN_INFO_OFFSET_RTOS);
			debug_msg("Nrml\r\n");
			if (chk_valid_all_in_one == 0) {
				uiUpdateFileLen = p_headinfo->BinLength;
				uiUpdateMainBinBufAddr = uiFwBaseAddr;
			} else {
				// uiUpdateFileLen is already set when file is loaded
			}
		} else {
			debug_msg("Fcompress\r\n");
			//for speed up, u-boot only copy compressed rtos and loader needs to decode it
			uiUpdateFileLen = bl_decompress_rtos(comp_addr, comp_size, uiFwBaseAddr);
			uiUpdateMainBinBufAddr = uiFwBaseAddr;
		}
		debug_msg_var("uiUpdateFileLen", uiUpdateFileLen);
		//boot from flash or update fw is no need to check sanity because of ecc, turn on it just for debug
		if (uiLoaderFunc & FUNC_RUN_CARD) {
			bl_checkFW(uiUpdateMainBinBufAddr, uiUpdateFileLen);
		}
		if (chk_valid_all_in_one == 0) {
			bl_update_uItron_headInfo(uiFwBaseAddr, p_dram_partition);
		} else {
			debug_msg_var("fw load addr", uiUpdateMainBinBufAddr);
			bl_update_uItron_headInfo(uiUpdateMainBinBufAddr, p_dram_partition);
		}
	} else { //boot from flash
#if (REMOVED_FLASH == DISABLE)
#if UPDATE_EMU_CODE
//		UINT32  uiNandBlkSize;
		// open flash
		if (bl_flash_open() != 0) {
			bl_displayErrMsg("flash open failed\r\n");
		}
		debug_msg("RFlash\r\n");
		if (int_strg_obj->flash_readSectors(g_uiStartBlkUpdateFW, g_uiNandBlkSize, (UINT8 *)SDRAM_Start_FW, NAND_RW_FIRMWARE) < 0) {
			bl_displayErrMsg(RWErrorMsg);
		}

		uiUpdateFileLen = *(volatile UINT32 *)(SDRAM_Start_FW + 0x168);

		if (int_strg_obj->flash_readSectors(g_uiStartBlkUpdateFW + 1, uiUpdateFileLen - g_uiNandBlkSize, (UINT8 *)(SDRAM_Start_FW + g_uiNandBlkSize), NAND_RW_FIRMWARE) < 0) {
			bl_displayErrMsg(RWErrorMsg);
		}

		uiFwBaseAddr = bl_process_rtos_only(SDRAM_Start_FW, uiUpdateFileLen, uiLoaderFunc, &comp_addr, &comp_size);
		bl_checkFW(SDRAM_Start_FW, uiUpdateFileLen);
		uiUpdateMainBinBufAddr = SDRAM_Start_FW;
		if (comp_addr == 0) {
			debug_msg("Nrml\r\n");
		} else {
			debug_msg("Fcompress not support\r\n");
		}

#else
		uiFwBaseAddr = bl_process_flash_boot((UINT8 *)SDRAM_Start_FW, &p_dram_partition, uiLoaderFunc, &comp_addr, &comp_size);
		if (comp_addr == 0) {
			uiUpdateMainBinBufAddr = uiFwBaseAddr;
		} else {
			//for speed up, u-boot only copy compressed rtos and loader needs to decode it
			uiUpdateFileLen = bl_decompress_rtos(comp_addr, comp_size, uiFwBaseAddr);
			uiUpdateMainBinBufAddr = uiFwBaseAddr;
		}
		// no need to bl_checkFW, because check sanity in flash_read
		bl_update_uItron_headInfo(uiFwBaseAddr, p_dram_partition);
#endif
#endif
	}

	if (p_dram_partition) {
		bl_update_loader_bininfo((unsigned char *)p_dram_partition->fdt_addr, uiLoaderFunc, uiUpdateFileLen);
	}

#if UPDATE_EMU_CODE
	debug_msg_var("emu fw len", uiUpdateFileLen);
#endif
	g_rtos_load_addr = uiUpdateMainBinBufAddr;
	g_rtos_target_addr = uiFwBaseAddr;
	g_rtos_size = uiUpdateFileLen;
	//invalid Instruciton and data cache
	CPUCleanInvalidateDCacheAll();
	CPUInvalidateICacheAll();
	debug_msg_var("rtos start", uiFwBaseAddr);
	return uiFwBaseAddr;

}

_THUMB2 void loader_setUpdateFwName(char *fileName)
{
	strncpy((char *)UPDATE_FW_NAME, fileName, sizeof(UPDATE_FW_NAME));
}

_THUMB2 void loader_setUpdateLdrName(char *fileName)
{
	strncpy((char *)UPDATE_LOADER_NAME, fileName, sizeof(UPDATE_LOADER_NAME));
}

_THUMB2 void loader_setRunFwName(char *fileName)
{
	strncpy((char *)RUN_FW_NAME, fileName, sizeof(RUN_FW_NAME));
}

_THUMB2 void loader_setRecoveryFwName(char *fileName)
{
	strncpy((char *)RECOVERY_FW_NAME, fileName, sizeof(RECOVERY_FW_NAME));
}

_THUMB2 void loader_setRecoveryPartitionID(UINT32 partition_id)
{
	g_uiPartitionID = partition_id;
}

_THUMB2 void loader_setVersion(UINT32 version)
{
	g_uiVersion = version;
}

_THUMB2 void loader_installSpecialKeyCB(LDR_SPECIAL_KEY_CB callback)
{
	gSpecialKeyCallBack = callback;
}

_THUMB2 void loader_installCardDetectCB(LDR_CARD_DETECT_CB callback)
{
	gCardDetectCallBack = callback;
}

_THUMB2 void loader_installRecoveryTriggerCB(LDR_RECOVERY_TRIGGER_CB callback)
{
	gRecoveryTriggerCallBack = callback;
}

_THUMB2 void loader_setStorageIntType(STORAGEINT type, PSTORAGE_OBJ strg_obj)
{
	gStorageIntType = type;
	int_strg_obj = strg_obj;
}
