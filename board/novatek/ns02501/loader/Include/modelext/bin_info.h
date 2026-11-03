/**
    Bin File Information

    Bin file layout

    -----------------------------------------------------
     HEADINFO  0x00 ~ 0x80 = 128 bytes = 32 WORDS
    -----------------------------------------------------

     EXCEPTION_TABLE: 0x180 ~ 0x200 = 128 bytes = 32 WORDS (see exception_MIPS.s)

    -----------------------------------------------------

     VECTOR_TABLE: 0x200 ~ 0x2f0 = 240 bytes = 60 WORDS    (see isr_MIPS.s)

    -----------------------------------------------------

     Code Info: 0x2f0 ~ 0x400 = 272 bytes = 68 WORDS       (see CodeInfo_MIPS.s)

    -----------------------------------------------------

     Code Entry: 0x400                                     (see Loader_MIPS.s)

        :

       PART-1
       PART-2
       PART-3
         :
       PART-N

    -----------------------------------------------------

    @file       bin_info.h
    @ingroup    mMODELEXT
    @note       THESE STRUCTS ARE VERY VERY IMPORTANT FORMAT DEFINITION OF SYSTEM,
                DO NOT MODIFY ANY ITEM OR INSERT/REMOVE ANY ITEM OF THESE STRUCTS!!!

    Copyright   Novatek Microelectronics Corp. 2011.  All rights reserved.
*/
#ifndef _BINI_NFO_H
#define _BINI_NFO_H

#define BIN_INFO_VER           0x19062115 ///< YYYY/MM/DD HH

#define BIN_INFO_OFFSET_RTOS   0x00000200 ///< rtos HEADINFO.o attached to this offset
#define BIN_INFO_OFFSET_UBOOT  0x00000300 ///< uboot HEADINFO.o attached to this
#define BIN_INFO_OFFSET_NUTTX  0x00000100 ///< nuttx
#define BIN_INFO_OFFSET_TEEOS  0x00000100 ///< teeos
#define BIN_INFO_OFFSET_BL31   0x00000300 ///< bl31
#define BIN_INFO_OFFSET_8051   0x00000300 ///< 8051

//Ep: Encryption Program (CheckSum version)
//Epcrc: Encryption Program (CRC version)
//Bfc: Bin File Compress
//Ld: Loader
//Fw: Firmware

//Head control flag for HEADINFO.BinCtrl
#define HDCF_LZCOMPRESS_EN  0x0000001 ///< BIT 0.compressed enable (0=no,1=yes)

//Head control flag for HEADINFO.BinCtrl (BL31)
#define BINCTRL_BL31_OS_AMP 0x00000001UL ///< indicate os multi-core heterogeneous

/**
     @name Ld control flag for LDINFO.LdCtrl
*/
//@{
#define BOOT_FLAG_PARTLOAD_EN    0x0000001 ///< BIT 0.PARTLOAD_EN (0=no,1=yes)
//@}

/**
 * @brief for encrypt_sign_tool bitwise to HEADINFO::BinCtrl
 */
#define SIGN_BINCTRL_LIMITED_SIZE 0x1      //indicate if sha_limited_size is enabled
#define SIGN_BINCTRL_NO_ENCRYPTIONDATA 0x2 //indicate if signature only without encrypted context

//HEADINFO::Resv field definition
typedef enum _HEADINFO_RESV_IDX_{
	HEADINFO_RESV_IDX_FDT_ADDR = 0,
	HEADINFO_RESV_IDX_SHM_ADDR = 1,
	HEADINFO_RESV_IDX_BOOT_FLAG = 2,
	HEADINFO_RESV_IDX_DRAM_SIZE = 3,
	HEADINFO_RESV_IDX_COUNTS = 19,
} HEADINFO_RESV_IDX;

//HEADINFO::Resv field definition for teeos
typedef enum HEADINFO_TEEOS_RESV_IDX{
	HEADINFO_TEEOS_RESV_IDX_LOAD_ADDR = 0,
	HEADINFO_TEEOS_RESV_IDX_UBOOT_ADDR = 1, //< write by loader
	HEADINFO_TEEOS_RESV_IDX_KERNEL_ENTRY_ADDR = 2, //< write by loader
	HEADINFO_TEEOS_RESV_IDX_KERNEL_FDT_ADDR = 3, //< write by loader
	HEADINFO_TEEOS_RESV_IDX_COUNTS = 19,
} HEADINFO_TEEOS_RESV_IDX;

//HEADINFO::Resv field definition for bl31
typedef enum HEADINFO_BL31_RESV_IDX{
  HEADINFO_BL31_RESV_IDX_LOAD_ADDR = 0,
  HEADINFO_BL31_RESV_IDX_UBOOT_ADDR = 1, //< write by loader
  HEADINFO_BL31_RESV_IDX_TEEOS_ADDR = 2, //< write by loader
  HEADINFO_BL31_RESV_IDX_RTOS_ADDR = 3, //< write by loader
  HEADINFO_BL31_RESV_IDX_COUNTS = 19,
} HEADINFO_BL31_RESV_IDX;

/**
 * @brief HEADINFO::Resv1 for encrypt_sign_tool
 */
typedef enum HEADINFO_SIGN_RESV_IDX{
	HEADINFO_SIGN_RESV_IDX_LIMITED_SIZE = 0, //value to sha_limited_size
	HEADINFO_SIGN_RESV_IDX_COUNTS = 19,
} HEADINFO_SIGN_RESV_IDX;

/**
     Header information

     0x00 ~ 0x80 = 128 bytes = 32 WORDS
*/
typedef struct HEADINFO {
	unsigned int CodeEntry;   ///< [0x00] fw CODE entry (4) ----- r by Ld
	unsigned int Resv1[HEADINFO_RESV_IDX_COUNTS];
	                          ///< [0x04~0x50] reserved (4*19) -- reserved, its mem value will filled by Ld
	                          ///< Resv1[HEADINFO_RESV_IDX_FDT_ADDR]: store fdt addr for rtos
	                          ///< Resv1[HEADINFO_RESV_IDX_SHM_ADDR]: store shm addr for rtos
	                          ///< Resv1[HEADINFO_RESV_IDX_BOOT_FLAG]: boot flag for rtos
	                          ///< Resv1[HEADINFO_RESV_IDX_DRAM_SIZE]: Total DRAM size for rtos
	char BinInfo_1[8];        ///< [0x50~0x58] CHIP-NAME (8) ---- r by Ep
	char BinInfo_2[8];        ///< [0x58~0x60] SDK version (8)
	char BinInfo_3[8];        ///< [0x60~0x68] SDK releasedate (8)
	unsigned int BinLength;   ///< [0x68] Bin File Length (4) --- w by Ep/bfc
	unsigned int Checksum;    ///< [0x6c] Check Sum or CRC (4) ----- w by Ep/Epcrc
	unsigned int CRCLenCheck; ///< [0x70~0x74] Length check for CRC (4) ----- w by Epcrc (total len ^ 0xAA55)
	unsigned int ModelextAddr;///< [0x74~0x78] where modelext data is. w by Ld / u-boot
	unsigned int BinCtrl;     ///< [0x78~0x7C] Bin flag (4) --- w by bfc
	                          ///<             BIT 0.compressed enable (w by bfc)
	                          ///<             BIT 1.Linux SMP enable (1: SMP/VOS, 0: Dual OS)
	unsigned int CRCBinaryTag;///< [0x7C~0x80] Binary Tag for CRC (4) ----- w by Epcrc (0xAA55 + "NT")
}
HEADINFO;

typedef struct HEADINFO64 {
	unsigned long long CodeEntry;   ///< [0x00] fw CODE entry (4) ----- r by Ld
	unsigned long long Resv1[HEADINFO_RESV_IDX_COUNTS];
	                          ///< [0x08~0xA0] reserved (4*19) -- reserved, its mem value will filled by Ld
	                          ///< Resv1[HEADINFO_RESV_IDX_FDT_ADDR]: store fdt addr for rtos
	                          ///< Resv1[HEADINFO_RESV_IDX_SHM_ADDR]: store shm addr for rtos
	                          ///< Resv1[HEADINFO_RESV_IDX_BOOT_FLAG]: boot flag for rtos
	                          ///< Resv1[HEADINFO_RESV_IDX_DRAM_SIZE]: Total DRAM size for rtos
	char BinInfo_1[8];        ///< [0xA0~0xA8] CHIP-NAME (8) ---- r by Ep
	char BinInfo_2[8];        ///< [0xA8~0xB0] SDK version (8)
	char BinInfo_3[8];        ///< [0xB0~0xB8] SDK releasedate (8)
	unsigned int BinLength;   ///< [0xB8] Bin File Length (4) --- w by Ep/bfc
	unsigned int Checksum;    ///< [0xBc] Check Sum or CRC (4) ----- w by Ep/Epcrc
	unsigned int CRCLenCheck; ///< [0xC0~0xC4] Length check for CRC (4) ----- w by Epcrc (total len ^ 0xAA55)
	unsigned int ModelextAddr;///< [0xC4~0xC8] where modelext data is. w by Ld / u-boot
	unsigned int BinCtrl;     ///< [0xC8~0xCC] Bin flag (4) --- w by bfc
	                          ///<             BIT 0.compressed enable (w by bfc)
	                          ///<             BIT 1.Linux SMP enable (1: SMP/VOS, 0: Dual OS)
	unsigned int CRCBinaryTag;///< [0xCC~0xD0] Binary Tag for CRC (4) ----- w by Epcrc (0xAA55 + "NT")
}
HEADINFO64;

STATIC_ASSERT(sizeof(HEADINFO) == 128);


/**
     Binary file information

     0x00 ~ 0x180 = 384 bytes = 96 WORDS
*/
typedef struct _BININFO {
	HEADINFO head;      ///< 0x00 ~ 0x80 = 128 bytes = 32 WORDS, header information
	unsigned int Resv[64];  ///< 0x80 ~ 0x180 = 384 bytes = 64 WORDS, reserved information
	                        ///< Resv[0]: signature size, for sha is made for part of content
}
BININFO;

STATIC_ASSERT(sizeof(BININFO) == 384);

///////////////////////////////////////////////////////////////////////////////
//Code Info: 0x2f0 ~ 0x400
#if defined(__aarch64__)
#define ZI_SECTION_OFFSET   (0x3f0+0x10) // ZI area information of starting offset
#define CODE_SECTION_OFFSET (0x3f0+0x20) ///< code section information of starting offset
#else
#define ZI_SECTION_OFFSET   (0x3f0+0x10) ///< ZI area information of starting offset
#define CODE_SECTION_OFFSET (0x3f0+0x18) ///< code section information of starting offset
#endif
#define CODE_SECTION_OFFSET64 (0x3f0+0x20) ///< on 32 bit loader to load rtos-64
/*
bfc syntex

bfc.exe p1 p2 p3 p4 p5 p6 p7 p8 p9

p1: compress/decompress
p2: compress method
p3: input file name
p4: output file name
p5: partial load flag ('1' means partial load)
p6: partial load file start [locate offset of bin file]
p7: output binary file length [locate offset of bin file]
p8: partial compress flag [locate offset of bin file]
p9: NAND block size (option)

NOTE: p6~p9 must be hex format (0x****)

@$(BFC) c lz $(BIN_R) tmp 1 0x310 0x68 0x78 $(EMBMEM_BLK_SIZE)
*/

#endif
