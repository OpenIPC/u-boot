/* SPDX-License-Identifier: GPL-2.0+ */
/* Runs where the boot ROM puts it: on-chip RAM 0x800, header below. */
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
	. = 0x800;
	.text : {
		KEEP(*(.text.start))
		*(.text*)
	}
	.rodata : { *(.rodata*) }
	. = ALIGN(4);
	.data : { *(.data*) }
	. = ALIGN(4);
	__image_end = .;
	.bss (NOLOAD) : {
		__bss_start = .;
		*(.bss*) *(COMMON)
		. = ALIGN(4);
		__bss_end = .;
	}
	/*
	 * The secondaries' check-in words, where the vendor first stage has
	 * them (the QEMU ax620u machine models them at this address).
	 */
	.pen 0x13298 (NOLOAD) : { KEEP(*(.pen)) }
	/DISCARD/ : { *(.ARM.exidx*) *(.comment) *(.note*) }
	ASSERT(__bss_end <= 0x13298, "first stage too big for the on-chip RAM")
}
