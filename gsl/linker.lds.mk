/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
MEMORY
{
	sram (rx)  : ORIGIN = TEXT_BASE, LENGTH = TEXT_SIZE
	ddr (rx)  : ORIGIN = DDR_BASE, LENGTH = DDR_SIZE
}
SECTIONS
{
	. = TEXT_BASE;
	. = ALIGN(8);
	.text	:
	{
	  boot/start.o  (.text)
	  *(.text)
	} > sram
	.text.ddr : ALIGN(8) {
		*(.text.ddr);
	} > ddr AT > sram

	.data.ddr : ALIGN(8) {
		*(.data.ddr);
	} > ddr AT > sram
	. = LOADADDR(.text.ddr) +  SIZEOF(.text.ddr);
	. = ALIGN(8);
	.rodata :
	{
	_ddr_load_start = LOADADDR(.text.ddr);
	_ddr_text_start = ADDR(.text.ddr);
	_ddr_text_end = ADDR(.text.ddr) + SIZEOF(.text.ddr) + SIZEOF(.data.ddr);
	  *(.rodata)
	  *(.version)
	} > sram
	. = ALIGN(8);
	.data : { *(.data) }
	. = ALIGN(1024);
	_end = .;
	. = RAM_BASE+0x1000+0x500;
	__bss_start = .;
	.bss : { *(.bss) }
	. = ALIGN(8);
	__bss_end = .;
}
