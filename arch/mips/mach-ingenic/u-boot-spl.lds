/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * The SPL in the cache the boot ROM locks as RAM, as the vendor
 * arch/mips/cpu/xburst/<soc>/u-boot-spl.lds has it.
 */

MEMORY { .sram : ORIGIN = CONFIG_SPL_TEXT_BASE,\
		LENGTH = CONFIG_SPL_MAX_SIZE }

OUTPUT_ARCH(mips)
ENTRY(_start)
SECTIONS
{
	.text      :
	{
		__start = .;
		*(.start_section*)
		*(.text*)
	} >.sram

	. = ALIGN(4);
	.rodata : {
		*(SORT_BY_ALIGNMENT(.rodata*))
		. = ALIGN(4);
		KEEP(*(SORT(__u_boot_list*)));
	} >.sram

	. = ALIGN(4);
	.data : { *(SORT_BY_ALIGNMENT(.data*)) *(.sdata*) } >.sram

	. = ALIGN(4);
	__image_copy_end = .;
	_image_binary_end = .;

	.bss (NOLOAD) : {
		. = ALIGN(4);
		__bss_start = .;
		*(.sbss*)
		*(.bss*)
		*(COMMON)
		. = ALIGN(4);
		__bss_end = .;
	} >.sram

	/DISCARD/ : {
		*(.dynbss)
		*(.dynstr)
		*(.dynamic)
		*(.interp)
		*(.hash)
		*(.gnu.*)
		*(.plt)
		*(.got.plt)
		*(.rel.plt)
		*(.rel.dyn)
		*(.MIPS.abiflags)
		*(.reginfo)
	}
}
