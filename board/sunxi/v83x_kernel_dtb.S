/* SPDX-License-Identifier: GPL-2.0+ */
/* The kernel's device tree (v83x-kernel.dts), gzipped, see v83x.c */

	.section .rodata.v83x_kernel_dtb, "a"
	.balign 4
	.global v83x_kernel_dtb_gz
v83x_kernel_dtb_gz:
	.incbin V83X_KERNEL_DTB_GZ
	.global v83x_kernel_dtb_gz_end
v83x_kernel_dtb_gz_end:
