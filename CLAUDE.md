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
| AX630C (Sipeed MaixCAM2: eMMC, TF slot, 0.5-4 GiB) | `ax630c-emmc` | started by the board's own U-Boot from the card; qemu-bench `-M ax620e,chip=ax630c` to OpenIPC's userspace from the card; **no silicon** |

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
`drivers/usb/{host,cdns3}/{Kconfig,Makefile}`,
`drivers/net/dwc_eth_qos.{c,h}` (the `axera,ax620e-eqos` and
`axera,ax620-eqos` entries), `common/{Kconfig,Makefile}` (`OPENIPC`,
`PRODUCT_SOC`, `PRODUCT_SOCMODEL`, `OPENIPC_NOR_SPLIT`),
`drivers/gpio/dwapb_gpio.c` (drops `asm/arch/gpio.h`, as mainline
00b04933985), `.gitignore` (`/output/`). The AX620U uses mainline drivers plus
`drivers/net/dwc_eth_qos_ax620u.c` (glue on mainline's EQOS driver)
and `drivers/usb/cdns3/cdns3-axera.c` (`USB_CDNS3_AXERA`, glue on
mainline's cdns3);
the AX620E adds `drivers/gpio/axera_gpio.c`, `drivers/spi/axera_spi.c`,
`drivers/net/dwc_eth_qos_axera.c` and
`drivers/usb/host/dwc3-axera-glue.c` (`USB_XHCI_DWC3_AXERA`, before
mainline's xhci-dwc3), and sets the SD card clock and pads in
`board_init()`. Both Ethernet glues are
`DWC_ETH_QOS_AXERA`; the SoC picks the file.

## Build

```sh
./build.sh                 # every openipc/<target>_config
./build.sh ax620u-nand     # -> output/u-boot-ax620u-nand{,-stockspl}.bin
./build.sh ax630c-emmc     # -> output/u-boot-ax630c-emmc.bin (for the SD card)
```

`build.sh` runs `olddefconfig` on `openipc/<target>_config` (a complete
config fragment, no base file), builds, and lays out one image per target, written at flash 0. `JOBS`
sets make's `-j`. For `ax620u-*`: `u-boot.bin` behind its Axera header
(`openipc/axera-image.py`, checked against the boot partition in the
target's `CONFIG_MTDPARTS_DEFAULT`), the first stage built from
`openipc/ax620u-spl` (into `output/build-<target>/spl`), then
`openipc/ax620u-image.py` puts them together as the 768 KiB boot
partition: the first stage and its backup, then U-Boot at 0x40000 in at
most three blocks (a fourth is the spare for a bad one).
`u-boot-<target>-stockspl.bin` is the same with the cameras' own first
stage (`openipc/ax620u-spl/stock.bin`, changed to load U-Boot from
0x40000; the fallback; `SPL=stock` puts it in `u-boot-<target>.bin` and
builds no other).
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

- Boot ROM -> **first stage** (flash 0 and its backup at 0x20000: DDR
  training, PLLs) -> **U-Boot** (0x40000, up to the environment at
  0xc0000) -> Linux from UBI. Both are in `boot`, OpenIPC's NAND layout.
- The first stage is `openipc/ax620u-spl` (`README.md` there),
  reconstructed in C from the cameras' -O0 binary, SPI NAND boot only,
  its DDR tables generated from that binary. `ax620u-image.py` writes
  the boot ROM's header (twice, at 0 and 0x400; code at 0x800 and again
  at 0x20000), signed with the SDK test key; given the stock code it
  reproduces the cameras' header byte for byte (`--check <dump>`). The
  cameras keep the EIP-130 firmware at 0x40000 and 0x60000, which their
  first stage loads only on parts with secure boot fused; the image has
  none, and its header says so (firmware size, sum and places zero). The
  cameras' own first stage stays as `openipc/ax620u-spl/stock.bin`;
  `ax620u-image.py --stock` changes six instructions so it loads U-Boot
  from 0x40000 (`STOCK_PATCHES`) and writes its header again. Only QEMU
  has run the rebuilt one: its MMIO writes up to U-Boot equal the
  stock's.
- The SPL loads U-Boot from 0x40000, a 1 KiB Axera header and then the
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
  `fwglued` (`common/openipc.c`, `OPENIPC_NAND_SPL_SIZE`, the first
  stage's 0x40000) checks it (the first stage's two headers, U-Boot's at
  0x40000), U-Boot goes to 0x40000 (that area erased to the environment,
  bad blocks skipped as the first stage reads them, the good ones checked
  to hold it), then the first stage's used blocks to 0, which must all be
  good (the boot ROM wants the first stage and its copy at fixed places; a
  bad one there means a programmer). An image of U-Boot alone updates
  U-Boot only; one for the cameras' layout (U-Boot at 0x100000) is
  refused, as is anything else.
- No SPL here, `SKIP_LOWLEVEL_INIT`; `board_init_f()` runs on
  `SYS_INIT_RAM_ADDR` 0x40f00000.
- **Size.** U-Boot (with its header) must fit three 128 KiB blocks,
  0x60000: `ax620u-image.py` fails the build past that. It is built
  Thumb-2 with LTO (ARM-mode it is about 590 KiB), with `BOOTSTD`, long
  help, the block cache, `fdt`, `bdinfo`, `iminfo`, `imxtract`,
  `bootelf`, `mii`, regex and FIT printing off; about 5 KiB are left.
  FIT itself needs MD5 and SHA-1 (the firmware's `nand-fit.its` hashes
  with crc32 and sha1).

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
- USB: mainline `cdns3` (host only, xHCI at 0x4810000) on the Cadence
  USB3 dual-role controller at 0x4800000 (`otg`/`xhci`/`dev`), high
  speed, behind `cdns3-axera.c` (`axera,cdns-usb`, the stock U-Boot
  tree's node). The stock U-Boot has no USB host: the glue is the stock
  4.19 kernel's (`CONFIG_USB_CDNS3_AXERA`, "axera usb init" at
  0xc054f490): in ap_clk, reset bits 9-11 of +0x24 for 2 ms, clear +0x44
  bit 1, set +0x25c bits 20-22, then its clocks from the kernel's clock
  tables: pclk_usb (+0x14 bit 21), aclk_usb (+0x0c bit 3), usb_ref,
  usb2phy, stb_predft (32 kHz crystal, +0x00 bit 28 clear), otg_fast
  (24 MHz crystal, divider 1 at +0x18), lpm_predft (+0x08 bits 18, 17,
  16, 14, 13), through the set aliases. No VBUS GPIO in either stock
  tree. Mass storage for `srcusb` and the recovery (after the card).
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

OpenIPC's NAND layout, `768k(boot),256k(env),-(ubi)` on `spi4.0_nand`
(the AX620U kernel's name for the flash), environment at 0xc0000, 256
KiB, as on openipc-hisilicon and openipc-sigmastar: the environment
fills its partition, with no spare block. The cameras' own
layout (`1024k(spl),1536k(boot),512k(env),-(ubi)`, the first stage's
1 MiB) is not used. The UBI layout is openipc-hisilicon's: UBIFS
`rootfs` holding `/boot/fitImage`, `rootfs_data` on the rest,
`root=ubi0:rootfs ubi.mtd=2,2048`,
`mem=87M` (the rest is the vendor media drivers'). `SYS_BOOTMAPSZ`
(32 MiB) keeps the device tree inside `mem=`: placed at the top of the
256 MiB, the kernel never starts.

### Verification (qemu-bench `-M ax620u`)

```sh
dd if=output/u-boot-ax620u-nand.bin of=flash.bin conv=notrunc  # at 0 of a 128 MiB flash or dump
QEMU_TLBI_RELAXED=1 qemu-system-arm -M ax620u,flash-file=flash.bin -smp 4 -nographic
```

Networking: `-netdev user,id=n0,net=192.168.1.0/24,host=192.168.1.107,dhcpstart=192.168.1.10,tftp=<dir> -net nic,netdev=n0`
puts QEMU's TFTP server at the default `serverip`, so `fwupd` on an
empty flash (`u-boot-ax620u-nand.bin` at 0, the rest erased) fetches
`rootfs.ubifs.ax620u`, resets and boots it. SD: `-drive
if=sd,index=1,file=sd.img,format=raw` (a power-of-two size, FAT).
Checked: that bootfail path to the OpenIPC login, `ping`, `tftpboot`,
`run ub` (read back and compared), `run ur` over TFTP and from SD
(`srcmmc`), the SD recovery at boot, the retired-command migration. USB:
`-drive if=none,id=u0,file=usb.img,format=raw -device
usb-storage,drive=u0` (the model's xHCI at 0x4810000; its OTG block is a
plain register bank, so the cdns3 core's wait for "xHCI ready" times out
after 100 ms and says so, as the stock kernel's does, then carries on);
checked `usb start`, `run srcusb; run ur` and the USB recovery at boot,
each to the OpenIPC login. The
model's PHY is not an RTL8201F (ID 0044.1400), so `board_phy_config()`
is untested, as is everything clock- or pad-related.

The machine starts the code at flash 0x800 (no boot ROM model), so the
first stage must be there: `u-boot-ax620u-nand.bin` at 0 boots from an
otherwise erased 128 MiB flash (a camera's dump has its own layout: its
`env` and `ubi` are not OpenIPC's). Its header is not loaded either, so
the first stage reads RAM 0x400 as zeros and takes 64 KiB NAND blocks;
the U-Boot area is bounded by address (to 0xc0000), not by a count of
blocks, so both sizes work. The machine's
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
  0x40000 (the environment); about 15 KiB are left (SD and USB took
  40 KiB of the packed image, with `MMC_WRITE`, `BOOTSTD` and the block
  cache off).

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
- SD: mainline `sdhci-cadence` on the Cadence SD4HC at 0x104e0000, 4
  bit, 3.3 V, the SDK's and the IVG-A6 kernel's PHY delays, no card
  detect (`broken-cd`, as the vendor trees: the card is looked for, and
  "Card did not respond" is printed without one); `mmc0`.
  `board_init()` does what the SDK's `sdhci_ax620e.c` does around it:
  the card clock (FLASH_SYS_GLB: NPLL 400 MHz / 2, enable bit 9), the
  CMD/DATA pull-ups and the pads' IO voltage from the detector
  (PIN_MUX_G9 0x104f1000). The SDK's driver also divides the SDHCI clock
  itself; mainline's does the same from the capabilities.
- USB: mainline `xhci-dwc3` on the DWC3 at 0x8000000, host, high speed,
  behind `dwc3-axera-glue.c` (`axera,ax620e-dwc3`, the SDK's node and
  glue): FLASH_SYS_GLB clocks (EB0 bits 12/14, EB1 bit 5), PHY/VCC reset
  (bits 24/25 of +0x14) and host select (+0x40 bit 6 clear). The SDK
  also sets the flash bus clock to 100 MHz; not here (the Ethernet AXI
  clock is on it). No VBUS GPIO in the IVG-A6's tree.
- SD and USB give `srcmmc`, `srcusb` and the recovery at boot (`*.img`
  uImage wrappers whose load address is the flash offset, or
  `script.txt`, from the card, then a USB stick), besides TFTP and
  `loady`. No pads are set but the SD's.

### Verification (qemu-bench `-M ax620e`)

```sh
# a 16 MiB OpenIPC NOR image with this boot partition at 0, env erased
qemu-system-aarch64 -M ax620e,flash-file=flash.bin \
    -global axera-spi-nor.env-from-flash=on -nographic \
    -netdev user,id=n0,tftp=dir -net nic,netdev=n0
```

SD: `-drive if=sd,index=1,file=sd.img,format=raw`; USB: `-drive
if=none,id=u0,file=usb.img,format=raw -device usb-storage,drive=u0`
(the model's DWC3 xHCI, polled). Checked with SD and USB: `mmc info`,
`usb start`/`usb tree`, `fatls`, `run srcmmc` and `run srcusb` with `uk`
and `ur` (read back), the kernel booted from them, the recovery at boot
from each onto an erased flash to the OpenIPC login.

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

## AX630C (`ax630c-emmc`)

The AX620E port (`arch/arm/mach-axera/ax620e`, `board/axera/ax620e`)
with `AXERA_AX630C_EMMC`: `include/configs/ax630c.h`,
`arch/arm/dts/ax630c-openipc.dts`, the environment in the eMMC.

### Boot

- No OpenIPC first stage for the eMMC yet, so no boot partition image.
  The board's own chain (boot ROM, its first stage, ATF, OP-TEE, its
  U-Boot 2020.04) starts this U-Boot from the card:
  `output/u-boot-ax630c-emmc.bin` is `u-boot.bin` as linked, loaded and
  started at `SYS_TEXT_BASE` 0x50000000 (clear of the stock U-Boot at
  0x5C000400, its ATF at 0x40040000 and OP-TEE at 0x44200000; 4 KiB
  aligned). At the stock prompt (`dcache off` first: its MMU tables are
  where this U-Boot relocates to):

  ```
  fatload mmc 1:1 0x50000000 u-boot-ax630c-emmc.bin
  dcache off
  icache off
  go 0x50000000
  ```

- Started that way it runs in whatever EL the stock U-Boot ran in (EL2
  is expected under the ATF, not confirmed). In EL2, or with `psci=1` in
  the environment, `ft_board_setup()` gives the kernel's device tree a
  PSCI node, `enable-method = "psci"` on the CPUs and no-map
  reserved-memory for the ATF (0x40040000, 256 KiB) and OP-TEE
  (0x44200000, 32 MiB): the OpenIPC device tree has spin-table CPUs and
  knows of neither. In EL1 (the OpenIPC first stage to come, QEMU) it
  changes nothing; `psci=0` turns it off.
- The same two ranges (`ax630c_secure[]`, `arch/arm/mach-axera/ax620e/soc.c`)
  stay out of U-Boot's own MMU map when it is entered above EL1:
  `dram_init()` splits the DRAM's Normal region around them (the EL is
  known then, the environment is not). With `psci=1` in EL1,
  `misc_init_r()` makes them invalid in the live tables
  (`mmu_change_region_attr()`). Either way `board_lmb_reserve()` keeps
  `fatload`, `bootm`'s device tree and initrd out of them ("Reading
  file would overwrite reserved memory"); `psci=0` does not map them
  back. A cacheable mapping of TrustZone memory would let the core
  prefetch from it (an external abort).
- DRAM: the board id the first stage leaves in misc_info (0x740: chip
  type AX630C/AX631, ADC board id 2/5/10/14 = 0.5/1/2/4 GiB, the
  MaixCAM2 SOMs); without one the device tree's 512 MiB, the smallest
  SOM (less than a larger board has, never more). Not probed: the
  ATF/OP-TEE memory may be secure. U-Boot stays below 4 GiB
  (`board_get_usable_ram_top()`, 32-bit SDHCI DMA). `totalmem` is the
  DRAM size; `osmem`, when the environment has none, a quarter of it,
  128 MiB to 1 GiB (256M on 1 GiB, 1024M on 4 GiB): `mem=${osmem}`, the
  rest is the CMM pool `load_axera` makes from totalmem and mem=.
- `firmware_scan()` (`common/openipc.c`, `OPENIPC_MMC`) looks for
  `Image` on the card's FAT p1 (mmc 1), then for a uImage/FIT at the
  eMMC's kernel partition (mmc 0, 1 MiB), and sets `medium` (`sd`,
  `emmc`) and `rootargs` in RAM; `bootcmd` (`run bootflash`) runs
  `${medium}load` and `${medium}boot`: `booti` of `Image` and
  `ax630c-maixcam2.dtb` (`fdtfile`) with `root=/dev/mmcblk1p2`, or
  `bootm` of the eMMC's kernel with `root=/dev/mmcblk0p4
  blkdevparts=${mmcparts}`. Neither: `bootfail`, the prompt (nothing is
  fetched and written by itself while the eMMC holds the board's own
  firmware). Recovery: `script.txt` on the card only.

### Layout and environment

The eMMC user area, no partition table (the boot ROM reads the first
stage from 0): `768k(boot),256k(env),16384k(kernel),-(rootfs)`
(`mmcparts`, the kernel's `blkdevparts=`, which needs
`CONFIG_CMDLINE_PARTITION`). The environment at 0xc0000, 256 KiB,
`ENV_IS_IN_MMC` on mmc 0. `mmc_get_env_addr()` (`common/openipc.c`)
refuses an area that is neither blank (one byte value) nor an
environment: on a MaixCAM2 still in its own layout 0xc0000 is its
DDR/ATF partition, so U-Boot boots with the defaults and `saveenv`
fails there instead of overwriting it. The first boot saves the
defaults on a blank area. No `env erase` (`CMD_ERASEENV` off): the
MMC backend erases whole erase groups (512 KiB and more), which reach
into the boot partition before 0xc0000. `ukwrite`/`urwrite` write the kernel and
root partitions; `ubwrite` waits for the first stage. Linux reads the
environment with `/dev/mmcblk0 0xc0000 0x40000` in `fw_env.config`.

### Hardware

- eMMC: mainline `sdhci-cadence` on the SD4HC at 0x1b40000, 8 bits, the
  SDK's capability mask (1.8 V only) and PHY delays; `board_init()` sets
  its card clock (CPU_SYS_GLB: NPLL 400 MHz / 2) as the SDK's
  `sdhci_ax620e.c`. The TF slot as on the AX620E (0x104e0000, mmc 1
  here). No HS200 in U-Boot.
- UART0, Ethernet (the SoC's own PHY) as on the AX620E. No USB.

### Verification (qemu-bench `-M ax620e,chip=ax630c`)

```sh
qemu-system-aarch64 -M ax620e,chip=ax630c -m 1G -smp 1 -nographic \
    -device loader,file=u-boot-ax630c-emmc.bin,addr=0x50000000,cpu-num=0 \
    -drive if=sd,index=0,file=emmc.img,format=raw \
    -drive if=sd,index=1,file=sdcard.img,format=raw -nic user
# a MaixCAM2 board id: chip type at 0x784, board id at 0x79c
#   -device loader,addr=0x784,data=4,data-len=4 \
#   -device loader,addr=0x79c,data=14,data-len=4 -m 4G
```

The loader starts U-Boot in EL1, as the OpenIPC first stage would.
Checked: the firmware's SD image autoboots to the OpenIPC login with
`mem=` and the vendor modules loaded (CMM pool from mem= to the end of
the DRAM), 512 MiB without a board id and 4 GiB with id 14. QEMU's eMMC
has no 1.8 V: with the eMMC node's `sdhci-caps-mask` removed (in a test
copy of U-Boot's and the kernel's device trees) the first boot saves
the environment, `saveenv`/`reset` keeps it, `fw_printenv`/`fw_setenv`
in Linux read and write it, a FIT at 1 MiB boots from the eMMC; with it,
U-Boot has no environment device and boots with the defaults. The
model releases CPU1 by spin-table only under `flash-file=`: with the
loader it comes up only by PSCI (`psci=1`). Linux's `reboot` does not
reset the model (only its watchdog does; U-Boot's `reset` uses it).
With `flash-file=` (any file) the loader starts U-Boot in EL3: the
secure ranges are then out of the map (`md` there aborts, a
translation fault), as with `psci=1` in EL1.

