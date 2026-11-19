# AX620E first stage (SDK BL1 SPL)

The code the AX620E boot ROM loads from SPI NOR offset 0 and runs at
0x03000400 in EL3: clocks, LPDDR4/DDR4/DDR3 init and training, then
U-Boot from flash 0x10000 (GZIPD-unpacked to 0x5C000400, entered in EL1).
`build.sh` builds it with `make -C openipc/ax620e-spl O=<dir>` and
`openipc/ax620e-image.py` puts the boot ROM's header in front of it.

## Provenance

`bl1/` is `boot/bl1` of the AX620E SDK as Sipeed publishes it,
<https://github.com/sipeed/maix_ax620e_sdk> at
`8aac11dd693deafa47f56f46cd0d628a4c0e3ef1` (that `boot/bl1` tree,
`391de07ba1f66b3f4cbea56bcd7fe450f643fff3`, is unchanged at the
repository's `d781b0bc`). Only the files the NOR SPL compiles or
includes are here, plus `board/arch/arm64/bl1.lds`; the import commit
has them unchanged (two CRLF files stored with LF). `git log -- bl1`
shows the import and every change since.

Everything is built from source. Nothing prebuilt is linked: the
`.o` files a built SDK tree has under `core/fatfs`, `core/fdl` and `fdl/`
are build output for the USB/UART download and SD boot images, which
this SPL does not use, and the EIP-130 firmware (`eip130_fw.h`,
`build/tools/imgsign/eip_ax620e.bin`) is neither compiled in nor put in
the image (the header keeps its size and sum, as the cameras' have).

## Configuration

The SDK's `spl/Makefile` with `build/projects/AX620Q_nor_arm32_k419`'s
values, `ARCH=arm64` (the AX620E cameras run an AArch64 first stage and
an AArch64 U-Boot): `FLASH_TYPE=nor`, `SUPPPORT_GZIPD`, no ATF, no
DDRINIT partition, U-Boot's header at flash 0x10000 (no backup copy) and
RAM 0x5C000000. The part is not a build option: the SPL reads the chip
type from the eFuse and picks the DRAM from it (AX620Q: LPDDR4, one rank,
x16, 2 Gbit = 256 MiB, 2400 MT/s, the SDK's `LPDDR4_CONFIG_2400`).

## Changes from the SDK

- `start.S`, `spl_main.c`: the SDK's AArch64 path assumes ATF. Without it
  `main()` now returns U-Boot's entry and `start.S` leaves EL3 for EL1 and
  jumps there (the code already in `start.S`, which expects the entry in
  x23), as the AX620E Xiongmai cameras' first stage does. The SDK instead
  branches to U-Boot in EL3, and its `wakeup_slave()` waits for a
  `pen_release` only the AArch32 `start.S` writes; here the second core is
  released into `_start` and parks in its spin table, unwaited, and
  `is_boot_uboot()` (an EL3 jump) is skipped, as `main()` loads U-Boot
  anyway. The watchdog is not armed before the jump (the Xiongmai first
  stage does not either; U-Boot stops it).
- `boot.c`: `SPINOR_SINGLE_IO` reads the flash with 0x03 on one data
  line, as the Xiongmai first stage does, instead of setting the flash's
  QE bit and reading with 0x6b (quad output).
- `spinand.h`: drops an unused `#include "errno.h"`, so the SPL builds
  with no C library headers (`-nostdinc`; `-std=gnu11`, as the code
  predates C23).
