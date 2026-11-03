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
    _loader_fixed_bin_size = $LOADER_FIXED_BIN_SIZE;
    .BOOT_EXEC 0xFE080000|0x00000000:
    AT (0)
    {
	_stext = .;
    	_load_BOOT_EXEC_start_base = .;
    	KEEP(Reset/reset.o(.data*));
    	Reset/reset.o(.text*);
    	_load_BOOT_EXEC_end_base = .;
    }
    .LOADER_CONFIGRAM (0xFE080000|0x00000000) + 0x30:
    AT (0x30)
    {
        _load_LOADER_CONFIGRAM_start_base = .;
        KEEP($LD_REMAP_LIB/$LD_CONFIGRAM.o(.data*));
        _load_LOADER_CONFIGRAM_end_base = .;
        _load_LOADER_CONFIGRAM_FREQ_PARAM_start_base = .;
        KEEP(Reset/configramFreqParam.o(.data*));
        _load_LOADER_CONFIGRAM_FREQ_PARAM_end_base = .;
    }
    .LOADER_CONFIGRAM2 (0xFE080000|0x00000000) + 0x300:
    AT (0x300)
    {
	config_dram_tbl_start_base = .;
	KEEP(Reset/config_dram1_tbl.o(.rodata*));
	config_dram_tbl_end_base = .;
    }
    .LOADER_CONFIGRAM3 (0xFE080000|0x00000000) + 0x400:
    AT (0x400)
    {
	config_dram_tbl2_start_base = .;
	KEEP(Reset/config_dram1_2_tbl.o(.rodata*));
	config_dram_tbl2_end_base = .;
    }
	LOADER_CORE2_ENTRY_PROGRAM (0xFE080000|0x00000000) + 0x500:
    AT (0x500)
    {
       _load_core2_jump_program_start_base = .;
        KEEP(Reset/core2_jump.o(.text));
       _load_core2_jump_program_end_base = .;
        _load_core2_entry_program_start_base = .;
        KEEP(Reset/core2_entry.o(.text));
       _load_core2_entry_program_end_base = .;
    }

    .LOADER_REMAP (0xFE080000|0x00000000) + 0x700:
    AT (0x700)
    {
	    _load_LOADER_REMAP_start_base = .;
        KEEP($LD_REMAP_LIB/$LD_DOREMAP.o(.text* .data*));
        _load_LOADER_REMAP_end_base = .;
        KEEP(Reset/configDDR.o (.text* .rodata* .data* .bss*));
        KEEP(Reset/ddr_basic.o (.text* .rodata* .data*));
        KEEP(Reset/ddr_control.o (.text* .rodata*));
        KEEP(Reset/ddr_globals.o (.data*));
        KEEP(Reset/ddr_io.o (.text* .rodata*));
        KEEP(Reset/clk_tree_k.o (.text* .rodata*));
        KEEP(Reset/zqk.o (.text* .rodata*));
        KEEP(Reset/dutyk.o (.text* .rodata* .data* .bss*));
        KEEP(Reset/issue_cmd.o (.text* .rodata*));
        KEEP(Reset/phy_bist.o (.text* .rodata*));
        KEEP(Reset/psdk.o (.text* .rodata*));
	KEEP(Reset/qsen.o (.text* .rodata* .data* .bss*));
        KEEP(Reset/read_eye.o (.text* .rodata* .data*));
        KEEP(Reset/read_vref.o (.text* .rodata*));
        KEEP(Reset/write_eye.o (.text* .rodata* .data*));
        KEEP(Reset/write_vref.o (.text* .rodata* .data* .bss*));
        KEEP(Reset/write_leveling.o (.text* .rodata*));
        KEEP(Reset/register_673c.o(.text* .rodata* .data* .bss*));
        KEEP(Driver/serial.o(.text* .data* .rodata*));
        KEEP(Driver/timer.o(.text* .rodata*));
        KEEP(Debug/debug.o (.text* .data* .bss*));
        KEEP(Common/global.o (.text* .data* .bss*));
        KEEP(Reset/ddr_basic.o(.bss*));
        KEEP(Reset/ddr_io.o(.bss*));
        KEEP(Reset/write_eye.o(.bss*));
        *libgcc.a:*(.text*);
        . = ALIGN(4);       /* This is MUST. Ensure section size is word aligned. */
    }
    . = ALIGN(4);
    .LOADER_EXEC ADDR(.LOADER_REMAP) + SIZEOF(.LOADER_REMAP) :
    AT (LOADADDR(.LOADER_REMAP) + SIZEOF(.LOADER_REMAP))
    {
        _loader_exec_start_base = .;
        KEEP(Ctrl_Flow/main.o(.part1*));
        * (.part1)
	KEEP(CA9/cache.o(.text));
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
	    _etext = .;
	    _loader_exec_compres_end = .;
	}

	_loader_exec_compress_load_base = LOADADDR(.text);
	_loader_exec_compress_load_cpu_addr = LOADADDR(.text) + ADDR(.BOOT_EXEC);
      _loader_data_start_base = .;
    _load_general_var_base = .;
    _image_general_var_base = .;
	_sdata = .;
    .data : { *(.data*) }
    _image_general_var_limit = .;
	_edata = .;
    _loader_data_cpu_addr = LOADADDR(.data) + ADDR(.BOOT_EXEC);
    _loader_data_size = SIZEOF(.data);
    .dummy : { LONG (0x00000000) }
    . = ALIGN(4);
    .got : { *(.got*) }
    . = ALIGN(4);
    .rodata :
    {
    	*(.rodata*)
	. = ALIGN(8);       /* This is MUST. Ensure ro size is 2 word aligned. */
    }

    .bss : {
        _image_general_zi_zi_base = .;
		_sbss = . ;
		*(.bss*)
		. = ALIGN(0x4000);
        _ttb = .;
        . = . + 0x4000;
        _image_general_zi_zi_limit = .;
    	_load_load_reset_base = .;
    	_image_load_reset_base = .;
    	_image_load_reset_length = .;
    	_image_load_reset_zi_base = .;
    	_image_load_reset_zi_limit = .;
		_ebss = . ;
    }
	. = ALIGN(64);
	.loader_heap :
	{
		_loader_heap_base = .;
		. += 0xA0000 + 0x20000 + 0x80000;
	}
	. = ALIGN(64);
	.lzma :
	{
		_lzma_tmp_start = .;
		. += 0x10000;
		_lzma_tmp_end = .;
	}
	. = ALIGN(64);
    _stack_start = .;

}

EXTERN($LD_LDS_EXTERN)
