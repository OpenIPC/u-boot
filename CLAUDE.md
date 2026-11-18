# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this branch is

OpenIPC's U-Boot for Axera's camera SoCs. The tree is **mainline U-Boot
v2022.10**, the same release as the other `openipc-*` branches, plus:

- the OpenIPC boot layer: `common/openipc.c` and `include/openipc.h` as
  `openipc-sigmastar` has them (the copy with the SPI NAND path), with
  `OPENIPC_SPI_NAND` also set by `CONFIG_MTD_SPI_NAND`;
- `include/configs/openipc-common.h`, the OpenIPC default environment,
  with the per-SoC values taken from `include/configs/<soc>.h`;
- the SoC support under `arch/arm/mach-axera/<soc>/` (`ARCH_AXERA`; each
  SoC selects its own CPU), `board/axera/<soc>/`, `arch/arm/dts/<soc>-*.dts`.

| SoC | Targets | Status |
| --- | --- | --- |
| AX620U (`axera,ax620x`, 4x Cortex-A7, AArch32) | `ax620u-nand` | qemu-bench `-M ax620u` to OpenIPC's userspace; **no silicon** |

`git show origin/openipc-hisilicon:CLAUDE.md` covers the OpenIPC layer
(default environment, `common/openipc.c`, env compatibility); it applies
here unchanged. There is no `build-axera.yml` on `openipc` yet, so no CI.

## Relationship to mainline

```sh
git diff --stat v2022.10 HEAD
git diff --diff-filter=M v2022.10 HEAD      # the mainline files patched
```

Patched mainline files: `arch/arm/{Kconfig,Makefile}` (`ARCH_AXERA`),
`arch/arm/dts/Makefile`, `common/{Kconfig,Makefile}` (`OPENIPC`,
`PRODUCT_SOC`, `PRODUCT_SOCMODEL`, `OPENIPC_NOR_SPLIT`),
`drivers/gpio/dwapb_gpio.c` (drops `asm/arch/gpio.h`, as mainline
00b04933985), `.gitignore` (`/output/`). Every driver is mainline's.

## Build

```sh
./build.sh                 # every openipc/<target>_config
./build.sh ax620u-nand     # -> output/u-boot-ax620u-nand.bin
```

`build.sh` runs `olddefconfig` on `openipc/<target>_config` (a complete
config fragment, no base file), builds, and wraps `u-boot.bin` with
`openipc/axera-image.py`, checking it against the boot partition in the
target's `CONFIG_MTDPARTS_DEFAULT`. `JOBS` sets make's `-j`.
Toolchain: `arm-linux-gnueabi-`.

## AX620U

### Boot

- Boot ROM -> **vendor SPL** (`spl`, flash 0, 1 MiB: DDR training, PLLs,
  signed, no source; kept as shipped) -> **U-Boot** (`boot`, 0x100000,
  1.5 MiB) -> Linux from UBI.
- The SPL loads the boot partition, a 1 KiB Axera header and then the
  image, to 0x40000000 and jumps to 0x40000400 (`SYS_TEXT_BASE`). It
  checks two 32-bit LE word sums only: the image's (+0x14) and the
  header's over +0x08..+0x2bc (+0x00). `axera-image.py` writes the stock
  header's capability and flash words, the size and the sums; the RSA
  key and signature stay zero (no secure boot on these cameras; a part
  with secure boot fused would refuse the image).
- No SPL here, `SKIP_LOWLEVEL_INIT`; `board_init_f()` runs on
  `SYS_INIT_RAM_ADDR` 0x40f00000.

### Hardware

- UART0 0x4990000, `snps,dw-apb-uart` (ns16550), clocked by the stock
  U-Boot's `ap_clk`, 200 MHz (not confirmed on silicon).
- SPI NAND (MX35LF1GE4AB on the known camera) on the DesignWare SSI at
  0x08000000, DWC_ssi register layout: mainline `designware_spi`
  (`snps,dwc-ssi-1.01a`), polled, one data line, chip select on GPIO2 5
  (0x10010000, mainline `dwapb_gpio`). The DT says bus width 1: with 4,
  spi-mem would pick quad opcodes this driver cannot send. 25 MHz
  (BAUDR 8 of 200 MHz; the SPL runs BAUDR 4).
- `board_nand_init()` takes the spi-nand MTD out of the table and
  registers it as nand0 (`ENV_IS_IN_NAND`, `nand`, `openipc.c`).
- Reset: 0x81 to 0x02220044 (the stock U-Boot's), then the DesignWare
  watchdog at 0x49aa000 as the stock kernel's `dw_wdt_restart()` (what
  QEMU resets on). Timer: the generic timer.
- No network, so a firmware the scan finds broken sends `fwrecovery`
  to a `tftpboot` that does not exist and resets, over and over: Ctrl-C
  at the prompt stops it.
- No Ethernet driver (EQOS at 0x4970000), no SD, so `NET` is off and
  updates come from Linux (`sysupgrade`) or a programmer.

### Layout and environment

`1024k(spl),1536k(boot),512k(env),-(ubi)` on `spi4.0_nand` (the AX620U
kernel's name for the flash), environment at 0x280000, 256 KiB in a
512 KiB range (`fw_env` finds it by scanning). The UBI layout is
openipc-hisilicon's: UBIFS `rootfs` holding `/boot/fitImage`,
`rootfs_data` on the rest, `root=ubi0:rootfs ubi.mtd=3,2048`,
`mem=87M` (the rest is the vendor media drivers'). `SYS_BOOTMAPSZ`
(32 MiB) keeps the device tree inside `mem=`: placed at the top of the
256 MiB, the kernel never starts.

### Verification (qemu-bench `-M ax620u`)

```sh
dd if=output/u-boot-ax620u-nand.bin of=flash.bin bs=1M seek=1 conv=notrunc  # into a 128 MiB dump
QEMU_TLBI_RELAXED=1 qemu-system-arm -M ax620u,flash-file=flash.bin -smp 4 -nographic
```

The flash must hold the vendor SPL at 0 (from a camera). The machine's
`axera-spi-nand` model needs READID answered after the opcode alone,
the 0x0b/0x3b cache reads, and a PROGRAM LOAD kept open across the SER
write that mainline's driver makes after queueing the data (SER 0 ends
it).
