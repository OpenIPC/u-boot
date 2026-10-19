/*
 * (C) Copyright 2013
 * David Feng <fenghua@phytium.com.cn>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

OUTPUT_FORMAT("elf64-littleaarch64", "elf64-littleaarch64", "elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
ENTRY(_start)
SECTIONS
{
	. = 0x48700000;
	__image_copy_start =.;
	. = ALIGN(8);
	.text :
	{
		start.o (.text*)
		uart.o (.text*)
		div0.o (.text*)
		image_data.o (.text*)
		startup.o(.text*)
		reset.o(.text*)
		*(.text*)
	}

	. = ALIGN(8);
	.image : { *(.image) }

	. = ALIGN(8);
	.rodata : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) }

	. = ALIGN(8);
	.data : {
		*(.data*)
	}

	. = ALIGN(8);

	.got : { *(.got) }

	. = ALIGN(8);
	__image_copy_end =.;
	__bss_start = .;
	.bss :
	{
		*(.bss)
	}
	__bss_end = .;


	_end = .;
}
