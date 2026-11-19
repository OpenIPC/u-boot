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
`drivers/net/dwc_eth_qos.{c,h}` (the `axera,ax620e-eqos` and
`axera,ax620-eqos` entries), `common/{Kconfig,Makefile}` (`OPENIPC`,
`PRODUCT_SOC`, `PRODUCT_SOCMODEL`, `OPENIPC_NOR_SPLIT`),
`drivers/gpio/dwapb_gpio.c` (drops `asm/arch/gpio.h`, as mainline
00b04933985), `.gitignore` (`/output/`). The AX620U uses mainline drivers plus
`drivers/net/dwc_eth_qos_ax620u.c` (glue on mainline's EQOS driver);
the AX620E adds `drivers/gpio/axera_gpio.c`, `drivers/spi/axera_spi.c`
and `drivers/net/dwc_eth_qos_axera.c`. Both glues are
`DWC_ETH_QOS_AXERA`; the SoC picks the file.

## Build

```sh
./build.sh                 # every openipc/<target>_config
./build.sh ax620u-nand     # -> output/u-boot-ax620u-nand.bin
```

`build.sh` runs `olddefconfig` on `openipc/<target>_config` (a complete
config fragment, no base file), builds, and wraps `u-boot.bin` with
`openipc/axera-image.py`, checking it against the boot partition in the
target's `CONFIG_MTDPARTS_DEFAULT`. `JOBS` sets make's `-j`.
Toolchains: `arm-linux-gnueabi-` (`CROSS_COMPILE`), and for `ax620e-*`
`aarch64-linux-gnu-` (`CROSS_COMPILE_64`); those targets are laid out by
`openipc/ax620e-image.py` instead, which needs the SDK's `ax_gzip`
(x86-64 binary; `build.sh` fetches it at a fixed commit and checks its
SHA-256 into `output/`, or `AX_GZIP=<path>`) and `curl`.

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
- Ethernet: mainline `dwc_eth_qos` + `dwc_eth_qos_ax620u.c`,
  `axera,ax620-eqos` (the stock U-Boot's compatible), RMII to an
  external RTL8201F at MDIO address 1, written from the stock U-Boot
  2020.04 binary (no AX620U source is published) and its devicetree:
  in the AP clock block 0x49ba000, the bus clock select (+0x00 [1:0], 3
  for the 312 MHz `emac-bus-clock`, which is also the CSR clock, MDIO
  CR 5), the MAC reset (+0x20 bit 4, 50 ms), RMII/RGMII (+0x34 bit 10),
  the RMII RX divider per speed (+0x18); 0x49bc500 bit 3 only for
  `rmii-phy-dir` "input". PHY reset GPIO1 (0x4960000) 28, low 100 ms
  then 200 ms. `board_phy_config()` puts the RTL8201F in RMII mode with
  its TXC an output, the PHY driving the 50 MHz reference clock (page 7
  reg 16: bit 3 set, bit 12 clear), as the stock driver does. No clock gates or pads are touched
  (the stock U-Boot does not either). Descriptors 64-byte padded with
  an AXI width of 8 for the skip length, as on the AX620E (not
  confirmed on silicon). Random MAC (`NET_RANDOM_ETHADDR`).
- SD: mainline `sdhci-cadence` on the Cadence SD4HC at 0x4950000, 4 bit,
  3.3 V, the stock U-Boot's PHY delays, card detect on GPIO1 2
  (`cd-inverted`); `mmc0`. FAT for `srcmmc` and the recovery
  (`rootfs.ubifs.ax620u.img` or `script.txt` on the card, at boot).
- With a medium to fetch from, a firmware the scan finds broken
  (`bootfail`) runs `fwupd` (`run ur`: TFTP `rootfs.ubifs.ax620u`) and
  resets, as on the other OpenIPC platforms. `loady ${baseaddr}` and
  `run urwrite` / `run ubwrite` still work without a network. An
  environment saved by the earlier build without network keeps its
  stay-at-the-prompt `fwrecovery`/`bootflash` until `openipc_helper()`
  replaces them (both variants are in `OPENIPC_RETIRED_COMMON`).
- `uk` has no `ukwrite` on NAND (as on openipc-hisilicon): the kernel
  is `/boot/fitImage` in the rootfs, so `ur` updates both.
- `ubwrite` and `urwrite` ask `fwfit` (`common/openipc.c`) first whether
  the image fits -- the boot partition's good blocks, or the UBI device
  with the rootfs and rootfs_data volumes it may replace -- so an image
  too big erases or removes nothing.

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

Networking: `-netdev user,id=n0,net=192.168.1.0/24,host=192.168.1.107,dhcpstart=192.168.1.10,tftp=<dir> -net nic,netdev=n0`
puts QEMU's TFTP server at the default `serverip`, so `fwupd` on an
empty flash (the SPL at 0, U-Boot at 0x100000, the rest erased) fetches
`rootfs.ubifs.ax620u`, resets and boots it. SD: `-drive
if=sd,index=1,file=sd.img,format=raw` (a power-of-two size, FAT).
Checked: that bootfail path to the OpenIPC login, `ping`, `tftpboot`,
`run ub` (read back and compared), `run ur` over TFTP and from SD
(`srcmmc`), the SD recovery at boot, the retired-command migration. The
model's PHY is not an RTL8201F (ID 0044.1400), so `board_phy_config()`
is untested, as is everything clock- or pad-related.

The flash must hold the vendor SPL at 0 (from a camera). The machine's
`axera-spi-nand` model needs READID answered after the opcode alone,
the 0x0b/0x3b cache reads, and a PROGRAM LOAD kept open across the SER
write that mainline's driver makes after queueing the data (SER 0 ends
it).

## AX620E

### Boot

- Boot ROM -> **vendor first stage** (flash 0, 64 KiB: DDR; signed with
  the SDK's test key, no source; `openipc/ax620e.spl`, the 0xa5bc bytes
  of an AX620E Xiongmai camera's flash) -> **U-Boot** (flash 0x10000) ->
  Linux, OpenIPC's NOR layout.
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

Checked: first boot saves the default environment, the scan finds the
kernel and rootfs, DHCP/ping/TFTP (CRC checked), `sf` reads, the kernel
(5.15, multi-file uImage) to login with DHCP, `fw_setenv` from Linux
read back by U-Boot on the next boot (reboot via the watchdog), `run ub`
writing this image over TFTP and booting it after `reset`. The model's
`axera-spi-nor` copies receive data to `[x19 + 120]` for the stock
U-Boot's driver; `axera_spi` zeroes x19 so it does not scribble.
