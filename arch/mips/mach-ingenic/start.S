/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * SPL start for the Ingenic T-series, from the vendor
 * arch/mips/cpu/xburst/<soc>/start.S (OpenIPC/u-boot-ingenic) and
 * arch/mips/cpu/xburst2/t40/start.S (OpenIPC/u-boot-t40) and t41/start.S
 * (Ingenic T41 SDK 1.2.0), the SFC NOR boot.
 *
 * The first 2 KiB are the boot ROM's: the SFC boot header (the ROM checks
 * a CRC7 of the SPL from 2 KiB on, which openipc/ingenic-image.py writes
 * into byte 9, and the SPL length at byte 12) and space reserved for the
 * secure boot. The ROM runs the code after it from the cache, on its own
 * stack.
 *
 * Copyright (c) 2019 Ingenic Semiconductor Co.,Ltd
 */

#include <config.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>

#define RESERVED_FOR_SC(x) .space 1536, x

	.set noreorder
	.globl _start
	.section .start_section, "ax"

_start:
	.word 0x03040506
	.word 0x55aa5502
	.word 0x000000aa
	.space 500, 0
	RESERVED_FOR_SC(0)

#ifdef CONFIG_T23
	/* GPIO A and B: drive strength, as the T23 vendor SPL sets it */
	la	t8, 0xffffffff
	la	t9, 0xb0010138
	sw	t8, 0(t9)

	la	t8, 0xffffffff
	la	t9, 0xb0010148
	sw	t8, 0(t9)

	la	t8, 0xaaaaaaaa
	la	t9, 0xb0010134
	sw	t8, 0(t9)

	la	t8, 0xaaaaaaaa
	la	t9, 0xb0010144
	sw	t8, 0(t9)

	la	t8, 0xffffffff
	la	t9, 0xb0011138
	sw	t8, 0(t9)

	la	t8, 0xffffffff
	la	t9, 0xb0011148
	sw	t8, 0(t9)

	la	t8, 0xaaaaaaaa
	la	t9, 0xb0011134
	sw	t8, 0(t9)

	la	t8, 0xaaaaaaaa
	la	t9, 0xb0011144
	sw	t8, 0(t9)
#endif

#ifndef CONFIG_CPU_XBURST2
	/* Invalidate BTB */
	mfc0	v0, CP0_CONFIG, 7
	nop
	ori	v0, v0, 2
	mtc0	v0, CP0_CONFIG, 7
	nop

	/* Clear bit 31 of CP0 $12 select 2 */
	mfc0	v0, $12, 2
	nop
	li	v1, 0x7fffffff
	and	v0, v0, v1
	mtc0	v0, $12, 2
	nop
#endif

#ifdef CONFIG_T41
	/* force the RISC-V core into soft reset, as the T41 vendor SPL */
	la	t8, 0xb2200fe0
	lw	t0, 0(t8)
	lui	t1, 0x8000
	or	t0, t0, t1
	sw	t0, 0(t8)
#endif

	/*
	 * CU0=UM=EXL=IE=0, BEV=ERL=1, IP2~7=1
	 */
	li	t0, 0x0040FC04
	mtc0	t0, CP0_STATUS

	/* CAUSE register */
	/* IV=1, use the specical interrupt vector (0x200) */
#ifdef CONFIG_CPU_XBURST2
	/* the T40 vendor SPL sets IV and keeps the rest */
	mfc0	t0, CP0_CAUSE
	li	t1, 0x00800000
	or	t0, t0, t1
	mtc0	t0, CP0_CAUSE
#else
	li	t1, 0x00800000
	mtc0	t1, CP0_CAUSE
#endif

#ifdef CONFIG_T41
	/* GPIO A, B and C pin set-up, as the T41 vendor SPL writes it */
	/* PA06~PA19 */
	la	t8, 0xf7fc0
	la	t9, 0xb0010134
	sw	t8, 0(t9)

	la	t8, 0xf7fc0
	la	t9, 0xb0010144
	sw	t8, 0(t9)

	/* PA15 */
	la	t8, 0x8000
	la	t9, 0xb0010138
	sw	t8, 0(t9)

	la	t8, 0x8000
	la	t9, 0xb00101a4
	sw	t8, 0(t9)

	/* PB00~PB31 */
	la	t8, 0xffffffff
	la	t9, 0xb0011134
	sw	t8, 0(t9)

	la	t8, 0xffffffff
	la	t9, 0xb0011148
	sw	t8, 0(t9)

	/* PC00~PC31 */
	la	t8, 0xffffffff
	la	t9, 0xb0012134
	sw	t8, 0(t9)

	la	t8, 0xffffffff
	la	t9, 0xb0012148
	sw	t8, 0(t9)
#endif

	/* The stack is the boot ROM's */
	j	board_init_f
	 nop
