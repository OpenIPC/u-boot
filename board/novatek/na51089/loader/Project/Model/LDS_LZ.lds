/* Linker script for malta
 *
 * Version: Sourcery G++ Lite 4.3-221
 * Support: https://support.codesourcery.com/GNUToolchain/
 *
 * Copyright (c) 2007, 2008, 2009 CodeSourcery, Inc.
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions.  No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 * */

/* Rule:
 * 1. to use .o in project layer just type the .o name with file extension. (UiFlow.o)
 * 2. to use .o in library layer have to add prefix module name. (DscSystem/SysTask.o)
 * */

OUTPUT_ARCH(arm)
ENTRY(__start)
SEARCH_DIR(.)

SECTIONS
{
    .BOOT_EXEC 0xF07C0000|0x00000000:
    AT (0)
    {
    	_load_BOOT_EXEC_start_base = .;
    	KEEP($LD_RESET_LIB/reset.o(.data*));
    	$LD_RESET_LIB/reset.o(.text*);
    	_load_BOOT_EXEC_end_base = .;
    }
    .LOADER_CONFIGRAM (0xF07C0000|0x00000000) + 0x30:
    AT (0x30)
    {
        _load_LOADER_CONFIGRAM_start_base = .;
        KEEP($LD_REMAP_LIB/$LD_CONFIGRAM.o(.data*));
        _load_LOADER_CONFIGRAM_end_base = .;
        _load_LOADER_CONFIGRAM_FREQ_PARAM_start_base = .;
        KEEP($LD_RESET_LIB/configramFreqParam.o(.data*));
        _load_LOADER_CONFIGRAM_FREQ_PARAM_end_base = .;
    }
    .LOADER_CONFIGRAM2 (0xF07C0000|0x00000000) + 0x300:
    AT (0x300)
    {
        KEEP($LD_RESET_LIB/config_dram1_tbl.o(.rodata*));
	config_dram1_tbl_2_start_base = .;
	KEEP($LD_RESET_LIB/config_dram1_2_tbl.o(.rodata*));
    }
    .LOADER_CORE2_ENTRY_PROGRAM (0xF07C0000|0x00000000) + 0x500:
    AT (0x500)
    {
        _load_core2_entry_program_start_base = .;
        KEEP($LD_RESET_LIB/core2_entry.o(.text));
       _load_core2_entry_program_end_base = .;
    }
    .LOADER_REMAP (0xF07C0000|0x00000000) + 0x600:
    AT (0x600)
    {
        _load_LOADER_REMAP_start_base = .;
        $LD_REMAP_LIB/$LD_DOREMAP.o(.text*);
        _load_LOADER_REMAP_end_base = .;
        _load_vector_exp_table_start_base = .;
        KEEP($LD_RESET_LIB/exp.o(.text*));
        _load_vector_exp_table_end_base = .;
        KEEP($LD_RESET_LIB/configDDR.o(.text*));
        KEEP($LD_RESET_LIB/configDDR.o(.rodata*));
        KEEP(Driver/fuart.o(.text* .data* .rodata*));
        KEEP(Debug/debug.o (.text* .data* .bss*));
        KEEP($LD_COMMON_LIB/global.o (.text* .data* .bss* .rodata*));
        . = ALIGN(4);       /* This is MUST. Ensure section size is word aligned. */
    }

    . = ALIGN(4);
    .LOADER_EXEC ADDR(.LOADER_REMAP) + SIZEOF(.LOADER_REMAP) :
    AT (LOADADDR(.LOADER_REMAP) + SIZEOF(.LOADER_REMAP))
    {
        _loader_exec_start_base = .;
        KEEP(Ctrl_Flow/main.o(.part1*));
        KEEP(Driver/rtc.o(.part1*));
        * (.part1)
        _loader_exec_end_base = .;
    }
    _loader_exec_cpu_addr = LOADADDR(.LOADER_EXEC) + ADDR(.BOOT_EXEC);
    _loader_exec_size = SIZEOF(.LOADER_EXEC);

	  . = ALIGN(4);
    .text $LD_BASE_ADDR:
    AT (LOADADDR(.LOADER_EXEC) + SIZEOF(.LOADER_EXEC))
    {
        _loader_exec_compres_start = .;
        _internal_strg_param_start = .;
        *(.text*)
        _loader_exec_compres_end = .;
    }
    _loader_exec_compress_load_base = LOADADDR(.text);
    _loader_exec_compress_load_cpu_addr = LOADADDR(.text) + ADDR(.BOOT_EXEC);
    .ARM.exidx :
	{
	    *(.ARM.exidx*)
	    *(.gnu.linkonce.armexidx.*)
	}
    . = ALIGN(4);
    .rodata :
    {
      *(.rodata*)
      . = ALIGN(8);       /* This is MUST. Ensure ro size is 2 word aligned. */
    }
    .dram_text :
    AT (LOADADDR(.rodata) + SIZEOF(.rodata))
    {
      _loader_dram_text_start_base = .;
      * (.dram_text)
    }
    _loader_dram_text_cpu_addr = LOADADDR(.dram_text) + ADDR(.BOOT_EXEC);
    _loader_dram_text_size = SIZEOF(.dram_text);
    .data :
    AT (LOADADDR(.dram_text) + SIZEOF(.dram_text))
    {
      _loader_data_start_base = .;
      _load_general_var_base = .;
      _image_general_var_base = .;
      *(.data*)
      _image_general_var_limit = .;
    }
    _loader_data_cpu_addr = LOADADDR(.data) + ADDR(.BOOT_EXEC);
    _loader_data_size = SIZEOF(.data);
    .dummy : { LONG (0x00000000) }
    . = ALIGN(4);
    .got : { *(.got) }
    . = ALIGN(4);
    .bss : {     /* locate bss at DRAM end */
        _image_general_zi_zi_base = .;
        *(.bss)
        . = ALIGN(0x4000);
        _ttb = .;
        . = . + 0x4000;
	. = ALIGN(0x400);
	_ttb_lv2 = .;
        . = . + 0x400;
    }
    .common :
    {
        __common_base = ABSOLUTE(.);
        /* collect "small" un-initialized global data (mark "C" in sym file) */
        *(.scommon*)
        /* collect "large" un-initialized global data (mark "B" in sym file) */
        *(COMMON)
        __common_limit = ABSOLUTE(.);
    }
    . = ALIGN(64);
    _image_general_zi_zi_limit = ABSOLUTE(.);
    _load_load_reset_base = .;
    _image_load_reset_base = .;
    _image_load_reset_length = .;
    _image_load_reset_zi_base = .;
    _image_load_reset_zi_limit = .;
	. = ALIGN(64);
	.loader_heap :
	{
		_loader_heap_base = .;
		. += 0xA0000 + 0x20000 + 0x80000;
	}
	. = ALIGN(64);
    _stack_start = .;

}
