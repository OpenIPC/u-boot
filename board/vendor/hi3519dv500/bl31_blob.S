/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * The BL31 bl31_handoff.c hands the Hi3519DV500 secure loader.
 * bl31.bin and how it was built: bl31.README.
 */

	.section .rodata.hi3519dv500_bl31, "a"
	.balign 64
	.global hi3519dv500_bl31_start
hi3519dv500_bl31_start:
	.incbin "bl31.bin"
	.global hi3519dv500_bl31_end
hi3519dv500_bl31_end:
