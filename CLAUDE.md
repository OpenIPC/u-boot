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
| AX620E family (AX620Q/AX630C, 2x Cortex-A53, AArch64) | `ax620e-nor` | qemu-bench `-M ax620e` to OpenIPC's userspace and back; **no silicon** |

`git show origin/openipc-hisilicon:CLAUDE.md` covers the OpenIPC layer
(default environment, `common/openipc.c`, env compatibility); it applies
here unchanged. CI: `.github/workflows/ci.yml` calls `build-axera.yml`
on `openipc` (both targets built, the AX620E image smoke-tested on
qemu-bench `-M ax620e`, release by hand to the `axera` tag; no publish to
OpenIPC/firmware); `git show origin/openipc:CLAUDE.md` describes it.

## Relationship to mainline

```sh
git diff --stat v2022.10 HEAD
git diff --diff-filter=M v2022.10 HEAD      # the mainline files patched
```

Patched mainline files: `arch/arm/{Kconfig,Makefile}` (`ARCH_AXERA`),
`arch/arm/dts/Makefile`, `drivers/{gpio,spi,net}/{Kconfig,Makefile}`,
`drivers/net/dwc_eth_qos.{c,h}` (the `axera,ax620e-eqos` entry), `common/{Kconfig,Makefile}` (`OPENIPC`,
`PRODUCT_SOC`, `PRODUCT_SOCMODEL`, `OPENIPC_NOR_SPLIT`),
`drivers/gpio/dwapb_gpio.c` (drops `asm/arch/gpio.h`, as mainline
00b04933985), `.gitignore` (`/output/`). The AX620U uses mainline drivers
only; the AX620E adds `drivers/gpio/axera_gpio.c`,
`drivers/spi/axera_spi.c` and `drivers/net/dwc_eth_qos_axera.c` (glue
on mainline's EQOS driver).

## Build

```sh
./build.sh                 # every openipc/<target>_config
./build.sh ax620u-nand     # -> output/u-boot-ax620u-nand{,-stockspl}.bin
```

`build.sh` runs `olddefconfig` on `openipc/<target>_config` (a complete
config fragment, no base file), builds, and lays out one image per target, written at flash 0. `JOBS`
sets make's `-j`. For `ax620u-*`: `u-boot.bin` behind its Axera header
(`openipc/axera-image.py`, checked against the boot partition in the
target's `CONFIG_MTDPARTS_DEFAULT`), the first stage built from
`openipc/ax620u-spl` (into `output/build-<target>/spl`), then
`openipc/ax620u-image.py` puts them together: the `spl` partition (first
stage, its backup, the EIP-130 firmware) and U-Boot at 0x100000, at most
up to the environment. `u-boot-<target>-stockspl.bin` is the same with
the cameras' own first stage (`openipc/ax620u-spl/stock.bin`, the
fallback; `SPL=stock` puts it in `u-boot-<target>.bin` and builds no
other).
Toolchains: `arm-linux-gnueabi-` (`CROSS_COMPILE`), and for `ax620e-*`
`aarch64-linux-gnu-` (`CROSS_COMPILE_64`); those targets are laid out by
`openipc/ax620e-image.py` instead, after `build.sh` has built the first
stage (`make -C openipc/ax620e-spl`, into `output/build-<target>/spl`),
twice: `u-boot-<target>.bin` with that first stage and
`u-boot-<target>-stockspl.bin` with the cameras' own
(`openipc/ax620e-spl/stock.bin`, the fallback; `SPL=stock` puts it in
`u-boot-<target>.bin` and builds no other). The tool needs the SDK's `ax_gzip` (x86-64 binary; `build.sh` fetches it
at a fixed commit and checks its SHA-256 into `output/`, or
`AX_GZIP=<path>`) and `curl`.

## AX620U

### Boot

- Boot ROM -> **first stage** (`spl`, flash 0, 1 MiB: DDR training,
  PLLs) -> **U-Boot** (`boot`, 0x100000, 1.5 MiB) -> Linux from UBI.
- The first stage is `openipc/ax620u-spl` (`README.md` there),
  reconstructed in C from the cameras' -O0 binary, SPI NAND boot only,
  its DDR tables generated from that binary. `ax620u-image.py` writes
  the boot ROM's header (twice, at 0 and 0x400; code at 0x800 and again
  at 0x20000), signed with the SDK test key; given the stock code it
  reproduces the cameras' header byte for byte (`--check <dump>`).
  0x40000 and 0x60000 hold the EIP-130 firmware (the header's firmware
  size and sum match; the SDK's public `eip.bin`, not per camera), which
  the image carries as the cameras have it
  (`openipc/ax620u-spl/eip130.bin`). The cameras' own first stage stays
  as `openipc/ax620u-spl/stock.bin`. Only QEMU has run
  the rebuilt one: its MMIO writes up to U-Boot equal the stock's.
- The SPL loads the boot partition, a 1 KiB Axera header and then the
  image, to 0x40000000 and jumps to 0x40000400 (`SYS_TEXT_BASE`). It
  checks the header only: magic and its 32-bit LE word sum (+0x00, over
  +0x08..+0x3f8). The image's sum (+0x14) is not checked, by the stock
  first stage (its decompile) nor by `openipc/ax620u-spl`.
  `axera-image.py` writes the stock header's capability and flash words,
  the size and both sums; in U-Boot's header the RSA key and signature
  stay zero (no secure boot on these cameras; a part with secure boot
  fused would refuse the image). The first stage's own header, for the
  boot ROM, is another format and does carry the SDK test key and a
  signature, as the cameras' does (`ax620u-image.py`).
- `ubwrite` (`run ub`, recovery) writes the image as it lies from 0:
  `fwglued` (`common/openipc.c`, `OPENIPC_NAND_SPL_SIZE`) checks it (the
  first stage's two headers, U-Boot's at 0x100000), U-Boot goes to `boot`
  (bad blocks skipped, as the first stage reads it), then the used part of
  `spl` to `spl`, whose blocks must all be good (the boot ROM wants the
  first stage and its copies at fixed places; a bad one there means a
  programmer). An image of U-Boot alone (the AX620U images before the
  first stage was in them) updates `boot` only. Anything else writes
  nothing.
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
- No network, MMC or USB, so nothing can fetch a firmware: when the scan
  finds it broken (`bootfail`), `bootflash` says so and stays at the
  prompt instead of resetting (`openipc-common.h`, `OPENIPC_RECOVERY`).
  There, `loady ${baseaddr}` over the console and `run urwrite` (a
  `rootfs.ubifs`) or `run ubwrite` (a U-Boot image) restore it.
- `ubwrite` and `urwrite` ask `fwfit` (`common/openipc.c`) first whether
  the image fits -- the boot partition's good blocks, or the UBI device
  with the rootfs and rootfs_data volumes it may replace -- so an image
  too big erases or removes nothing.
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
dd if=output/u-boot-ax620u-nand.bin of=flash.bin conv=notrunc  # at 0 of a 128 MiB flash or dump
QEMU_TLBI_RELAXED=1 qemu-system-arm -M ax620u,flash-file=flash.bin -smp 4 -nographic
```

The machine starts the code at flash 0x800 (no boot ROM model), so the
first stage must be there: `u-boot-ax620u-nand.bin` at 0 boots from an
otherwise erased 128 MiB flash, or from a camera's dump. The machine's
`axera-spi-nand` model needs READID answered after the opcode alone,
the 0x0b/0x3b cache reads, and a PROGRAM LOAD kept open across the SER
write that mainline's driver makes after queueing the data (SER 0 ends
it).

## AX620E

### Boot

- Boot ROM -> **first stage** (flash 0, 64 KiB: DDR) -> **U-Boot** (flash
  0x10000) -> Linux, OpenIPC's NOR layout.
- The first stage is the SDK's BL1 SPL built from source,
  `openipc/ax620e-spl` (`README.md` there: Sipeed's SDK commit, the files
  imported, the changes). The boot ROM loads it to 0x03000400 behind a
  1 KiB header that `ax620e-image.py` writes as the SDK's
  `spl_AX620E_sign.py -small_size_nor` (`spl_header()`: checksums,
  capability 0x54FAFE, the test key and a signature); for the Xiongmai
  cameras' own first stage it gives their header byte for byte. The SPL
  picks the DRAM by the eFuse chip type (AX620Q: LPDDR4 x16 256 MiB),
  reads the flash with 0x03 on one line and enters U-Boot in EL1, as the
  cameras' first stage does. `AX620E_XM` (README there) makes it set up
  the DDR as that first stage does, an older SDK build: in QEMU the DDR
  controller, PHY and DDR PLL get the same 1308 writes in the same order
  (the 2666 tables at 2400 MT/s, its PHY values and training limits),
  and the CPU PLL the same values. Still different: the board init
  around it (the AXI timeout and watchdog/thermal abort enables, the
  ADC reference for the board id), the flash clock registers and when
  the flash is first set up. Only a camera confirms any of it.
- `openipc/ax620e-spl/stock.bin` is the cameras' own first stage (header
  and code, 0xa5bc bytes from flash 0 of the IVG-A6 dump; README there),
  the fallback: `u-boot-ax620e-nor-stockspl.bin` carries it, and
  `ax620e-image.py --signed` checks its header and copies it as it is.
- The first stage reads the image at 0x10000: a 1 KiB Axera header, then
  the payload in Axera's own LZ77/Huffman format ("20", `ax_gzip`; the
  GZIPD engine unpacks it, no other writer exists). It unpacks to
  0x5C000400 and enters there in **EL1**. `ax620e-image.py` writes the
  header as the SDK's `sec_boot_AX620E_sign.py` (checksums, capability
  0x54FAFE, RSA-2048 key, PKCS#1 v1.5 SHA-256 signature with
  `openipc/axera-sdk-test-key.pem`, the SDK's published test key): byte
  for byte the SDK script's output.
- `SYS_TEXT_BASE` is 0x5C001000: the payload starts with a branch there.
  At 0x5C000400 U-Boot dies after relocation, as the offset is then not
  whole pages and `adrp` addresses pages. (Not `POSITION_INDEPENDENT`
  either: it parks in `wfi` when not 4 KiB aligned.)
- **Three tiles.** The first stage streams the packed image in 64 KiB
  tiles; below three (16 + 2 x 64 KiB + 8 KiB) it queues the first tile
  before reading all of it and the engine unpacks 16 stale bytes. The
  tool pads the payload past U-Boot's end with incompressible filler up
  to three; the stock images are all larger. Keep the image at most
  0x40000 (the environment); about 55 KiB are left.

### Hardware

- UART0 0x4880000, ns16550 (`snps,dw-apb-uart`), 208 MHz (the SDK's).
- SPI NOR on the boot DesignWare SSI at 0x1a00000, `axera_spi`: transmit
  only or receive only transfers, chip select GPIO2 29 (`axera_gpio`,
  one register per pin, not a DW GPIO), receives kept to the FIFO depth
  (no DMA), clock and sample delay as the first stage left them,
  opcode 0x03 single width (ops with dummy cycles refused). Mainline
  `designware_spi` uses EEPROM-read/TR transfers the vendor driver does
  not, so it is not used. Receive phases clear x19 for the QEMU model
  (see below).
- Ethernet: mainline `dwc_eth_qos` + `dwc_eth_qos_axera.c` (clocks,
  resets, RMII select, the SoC's 10/100 PHY with the eFuse bandgap trim
  from misc_info at 0x740; 312 MHz CSR clock). Descriptors are 64-byte
  padded with an AXI width of 8 for the skip length (not confirmed on
  silicon).
- DDR 256 MiB at 0x40000000; `mem=120M`, the rest is the vendor's CMM;
  `SYS_BOOTMAPSZ` 64 MiB keeps the device tree in the kernel's memory.
- Watchdog 0 is stopped at boot. Reset: the software chip reset
  (`chip_rst_sw()`), then watchdog 0 with a zero timeout (the kernel's
  restart handler, what QEMU resets on).
- No SD driver (Cadence SD4HC with an Axera PHY), so no SD recovery;
  updates over TFTP (`ub`, `uk`, `ur`) or `loady`. No pads are set.

### Verification (qemu-bench `-M ax620e`)

```sh
# a 16 MiB OpenIPC NOR image with this boot partition at 0, env erased
qemu-system-aarch64 -M ax620e,flash-file=flash.bin \
    -global axera-spi-nor.env-from-flash=on -nographic \
    -netdev user,id=n0,tftp=dir -net nic,netdev=n0
```

Checked (with the stock first stage, before it was built from source):
first boot saves the default environment, the scan finds the
kernel and rootfs, DHCP/ping/TFTP (CRC checked), `sf` reads, the kernel
(5.15, multi-file uImage) to login with DHCP, `fw_setenv` from Linux
read back by U-Boot on the next boot (reboot via the watchdog), `run ub`
writing this image over TFTP and booting it after `reset`. The model's
`axera-spi-nor` copies receive data to `[x19 + 120]` for the stock
U-Boot's driver; `axera_spi` zeroes x19 so it does not scribble.

The source-built first stage needs a qemu-bench whose `axera-spi-nor`
ends a command at the SDK's software reset (0x66, 0x99), which it sends
before its first read; with an older one that read returns zeros and it
stops at `verify_img_header: magic 0x55543322 != 0x0`. With it, it boots
this U-Boot, the environment save, `reset`, and the OpenIPC kernel to
login, both on the AX620Q DDR path (`-M ax620e,chip=ax620q`, the eFuse
chip type qemu-bench's `chip=` reports) and on the AX630C one a qemu-bench
without `chip=` gives (an eFuse of zeros). QEMU's DDR models accept any
settings: only a camera tests the DDR. The `-stockspl` image boots the
same way (env save, `reset`, env loaded, kernel to login).
