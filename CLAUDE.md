# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this branch is

`openipc-novatek` is OpenIPC's U-Boot for Novatek's NA51089 camera SoCs:
the NT98562 (64 MiB of DDR in the package) and the NT98566 (128 MiB), one
Cortex-A9 each. The tree is **mainline U-Boot v2022.10**, the same release
as `openipc-hisilicon`, `openipc-xm`, `openipc-gk710x` and
`openipc-anyka`, plus:

- the OpenIPC boot layer, as on `openipc-anyka`;
- the NA51089 support, written from the vendor sources: the NT9856x Linux
  SDK v1.01.007 (U-Boot 2019.04, kernel 4.19.91), and the device tree of a
  Xiongmai NT98562 camera;
- the SDK's loader, which runs before U-Boot, as Novatek ships it
  (`board/novatek/na51089/loader`), built from its sources.

CI and user docs live on the `openipc` branch:

- `.github/workflows/ci.yml` only calls `build-novatek.yml@openipc`.
- `git show origin/openipc:CLAUDE.md` covers the workflow invariants.
- `git show origin/openipc-hisilicon:CLAUDE.md` covers the OpenIPC layer
  (default environment, `common/openipc.c`, env compatibility). It applies
  here unchanged.

Adding or dropping a SoC needs both branches:

- here: `build.sh` (`build_soc`, `ALL_SOCS`), `openipc/<soc>_config`,
  `openipc/<soc>.dts`, and `TARGET_*` in `arch/arm/mach-novatek/Kconfig`;
- there: `build-novatek.yml`'s matrices and the README.

The NVR dies NA51103 (NT98332) and NA51090 (NT98633) are supported as
AArch64 targets behind the board's own loader and BL31; see "The NVR
dies" below. NS02501 (NT98532, NT98533) is NA51089's layout with its
own loader; see "NS02501" below. The other dies of the SDKs (NA51068,
NA51102, the other NS02xxx parts) are not supported.

## Relationship to mainline

```sh
git diff --stat v2022.10 HEAD                  # the whole delta
git diff --diff-filter=M v2022.10 HEAD         # the mainline files we patched
```

**Added**

| Area | Location |
| --- | --- |
| SoC: the loader's HEADINFO, reset, DRAM size | `arch/arm/mach-novatek/` (`ARCH_NOVATEK`; `TARGET_NT98562/NT98566`, `NVT_NA51089`; register map in `include/mach/na51089.h`) |
| Board: the loader's device tree, the memory split, Ethernet pins, clocks and PHY, `misc_init_r` → `openipc_helper()` | `board/novatek/na51089/na51089.c` |
| HEADINFO at 0x300 | `board/novatek/na51089/u-boot.lds` (mainline's ARM one plus two lines) |
| The loader | `board/novatek/na51089/loader/` (the SDK's, see "Boot") |
| SPI NOR on the flash controller, non-DM | `drivers/mtd/spi/nvt_sflash.c` (`NVT_SFLASH`). It provides `spi_flash_probe()` and fills `mtd->_read/_write/_erase`, as `ak_sflash` does on `openipc-anyka` |
| Ethernet, non-DM | `drivers/net/nvt_eqos.c` (`NVT_EQOS`) |
| SD on SDIO1, non-DM | `drivers/mmc/nvt_mmc.c` (`MMC_NVT`) |
| Console, timer | mainline NS16550 and `lib/time.c`, from `include/configs/novatek.h`, which ends in `#include "openipc-common.h"` |
| OpenIPC boot layer | `common/openipc.c`, `include/openipc.h` (as on `openipc-anyka`), `include/configs/openipc-common.h` (Novatek parameters, NOR only, same env commands) |
| Kconfig fragments, device trees, the image, the build | `openipc/`, `build.sh` |

**Patched mainline files**, each small:

- `arch/arm/{Kconfig,Makefile}`: the `ARCH_NOVATEK` hook.
- `common/{Kconfig,Makefile}`: `OPENIPC`, `PRODUCT_SOC`, `PRODUCT_SOCMODEL`.
- `boot/image-fdt.c`: with `ARCH_NOVATEK`, a plain uImage boots with the
  device tree at `${fdtaddr}` (see "Hand-off to the kernel").
- `drivers/mtd/spi/{Kconfig,Makefile}`, `cmd/Kconfig` (`CMD_SF`) and
  `env/Kconfig` (`ENV_IS_IN_SPI_FLASH`): accept `NVT_SFLASH`.
- `drivers/net/{Kconfig,Makefile}`, `drivers/mmc/{Kconfig,Makefile}`: the
  two drivers.
- `drivers/net/Kconfig`: `NETDEVICES` selects `DM_ETH` only when `DM` is on,
  as on `openipc-xm`.

## Boot

The mask ROM loads the loader (32 KiB) from flash offset 0 into SRAM and
starts it. The loader, closed code in the vendor's libraries, trains the
DDR. Then, from `LibExt/LIBExt_src/Ctrl_Flow/bl_func.c`:

1. it reads the device tree from flash block 1 (`0x10000` on NOR) and
   copies it to `/nvt_memory_cfg/fdt` (`0x100000`);
2. it finds `/nor/partition_uboot`, by its label, through
   `/nor/nvtpack/index` (without the index entry it finds nothing), and
   loads it to `/nvt_memory_cfg/uboot` (`0x03900000`): a BCL1 container is
   unpacked (algorithm 11, LZMA, by a mask ROM routine), anything else is
   copied for its HEADINFO's length;
3. it fills SHMINFO at `/nvt_memory_cfg/shmem` (`0x200000`): `"LD_NVT"`
   at +0, the device tree's address at +0x2C;
4. it checks the HEADINFO at 0x300 into U-Boot: CodeEntry must be
   `/nvt_memory_cfg/uboot`, and the 16-bit words plus their indices must
   sum to 0 over BinLength; then it calls CodeEntry.

`openipc/nvt-image.py` builds the 256 KiB image the programmer writes at
offset 0:

| Offset | Contents |
| --- | --- |
| `0x00000` | the loader |
| `0x10000` | the device tree (`openipc/<soc>.dts`) |
| `0x20000` | U-Boot, its HEADINFO filled in, in a BCL1 LZMA container (the slot is 128 KiB) |
| `0x40000` | the environment (not in the image) |

- The BCL1 container is what the SDK's `bfc c lzma` writes: `"BCL1"`, a
  checksum (LE16), the algorithm (BE16, 11), the unpacked and packed sizes
  (BE32), then an `.lzma` stream with its real size. The loader reads the
  packed size, rounded up to whole flash blocks, from the start of the
  container: `nvt-image.py` pads the stream when the 16-byte header would
  push its end past that.
- `CONFIG_SYS_TEXT_BASE` is `/nvt_memory_cfg/uboot`; the HEADINFO's
  CodeEntry is the same constant (`arch/arm/mach-novatek/soc.c`).
- **The device tree** is a Xiongmai NT98562 camera's
  (IPC_NT98562_80N40_S38 V5.00.R02, which its loader carries inside), with
  the OpenIPC boot chain in its `/nor` node; `nt98566.dts` changes what
  Xiongmai's NT98566 tree changes (DDR size and split, a card power GPIO,
  media paths). The vendor kernels take everything from it: pinmux, clocks,
  the media pool.
- **The loader** (`board/novatek/na51089/loader`) is the SDK's
  `nt9856x_loader_release_v1.01.007`, imported as it ships, then changed
  only to build with a distribution's Arm toolchain (`-fshort-enums`, as
  its libraries were built; a `raise()` for a Linux toolchain's libgcc) and
  to default to the NT98562 on NOR. Its make system runs Novatek's Linux
  host tools from `Tools/Bin`: `nvt-ld-op` (the linker script), `bfc`
  (packs part of the loader), `Configram560` (writes the DDR parameters of
  the package, `-package=562` or `566`), `encrypt_boot` and
  `encrypt_boot_dump` (sums). `build.sh` builds it in a copy
  (`output/build-<soc>-loader`) with `CHIP` and `DRAM1_CLK` of Xiongmai's
  loaders: 562 at 666 MHz, 566 at 933 MHz. `.gitattributes` there keeps
  the vendor bytes (CRLF sources, the host binaries) as they are.
- The image is in CPU byte order, as `sf write` stores it and an external
  programmer writes it.

## Hand-off to the kernel

- OpenIPC uImages carry no device tree. `misc_init_r()` copies the
  loader's (from SHMINFO) into malloc'd memory, out of the way of the
  kernel unpacking at 0x8000, and sets `${fdtaddr}`; `bootm ${baseaddr}`
  boots with it (the `boot/image-fdt.c` change).
- `ft_board_setup()` does what the vendor U-Boot's `nvt_boot` does with
  `osmem`: `/memory` is `<0 osmem>`, `/hdal-memory/media` the rest of the
  DDR, the vendor media modules' pool. `mem=${osmem}` stays the first
  kernel argument. `board_lmb_reserve()` keeps the device tree below
  `osmem`, where Linux can reach it; lmb refuses a reservation that
  overlaps one it has, so only the gaps are reserved.
- The console is `ttyS0`, the MTD name `spi_nor.0` (`nvt-devname`).

## Peripherals

- **Console:** UART0, a 16550 with 32-bit registers at 24 MHz; the loader
  sets its pins.
- **Timer:** timer 0 of the timer block, free running at 1 MHz from the
  loader on; `CONFIG_SYS_TIMER_COUNTER`.
- **Reset:** the watchdog, then its manual trigger, as the vendor U-Boot.
- **SPI NOR:** the flash controller at `0xF0400000` runs one operation at a
  time (`drivers/mtd/spi/nvt_sflash.c` describes them). READ 0x03 at
  48 MHz, single lane; whole cache lines by DMA, the rest through the FIFO;
  256-byte pages by DMA, shorter ones by PIO; 64 KiB erases polled by the
  controller; 3-byte addresses: a larger part is used for its first
  16 MiB; block protection left by a vendor loader is cleared, and if it
  stays, writes and erases fail (-EROFS) rather than being dropped.
- **Ethernet:** the Synopsys Ethernet QoS MAC at `0xF02B0000` and the
  embedded PHY (`0xF02B3800`). `board_eth_init()` reads the wiring from the
  loader's device tree (`/top@f0010000/eth` pinmux; Xiongmai's 0x22 is the
  embedded PHY, LEDs on the second pads) and brings up the clocks, the PHY
  PLL and the PHY's power as the vendor does. The MAC's DMA reset only
  completes once the PHY gives it an RX clock, so the PHY comes first. An
  RMII PHY is wired the vendor's way too, untested.
- **SD:** SDIO1, Novatek's own host, 4-bit, PIO through the FIFO, up to
  24 MHz, no card detect (with no card the first command times out, about
  10 ms). `common/openipc.c` recovers from it at every boot as on the other
  targets (`uImage.<soc>.img`, `rootfs.squashfs.<soc>.img`).

## Build

```sh
bash build.sh               # both; continues past failures, exits non-zero if any failed
bash build.sh nt98566       # one or more SoCs (CROSS_COMPILE defaults to arm-linux-gnueabi-)
```

- The output is `output/u-boot-<soc>-nor.bin`, built out of tree in
  `output/build-<soc>-nor/` and `output/build-<soc>-loader/`.
- `.config` is `openipc/openipc_config`, then `openipc/<soc>_config`, then
  `olddefconfig`. There are no `configs/*_defconfig` for these targets.
- The loader uses `LOADER_CROSS_COMPILE`: `arm-none-eabi-` if there is one,
  else `CROSS_COMPILE`.
- U-Boot is ~233 KiB, ~94 KiB packed, in the 128 KiB slot;
  `nvt-image.py` fails the build if a part does not fit. Check the packed
  size after enabling anything.
- nt98332 and nt98633 are AArch64: `CROSS_COMPILE_A64`, default
  `aarch64-linux-gnu-` (Debian/Ubuntu `gcc-aarch64-linux-gnu`; CI must
  install it). Their outputs are `output/u-boot-nt98332-nor.bin` /
  `u-boot-nt98633-nand.bin` (U-Boot's slot alone) and
  `u-boot-<soc>-qemu.bin` (the stand-in's boot area, for CI).

## Verification

The emulator is OpenIPC/qemu-hisilicon, machines `nt98562`, `nt98566`
(`docs/novatek.md` there). `-M <soc>,flash-file=<image>` does the loader's
job (not the DDR training, so it does not run the loader): it reads the
device tree at 0x10000, unpacks U-Boot from its BCL1 container (LZMA
support is newer than the first NT9856x machines), fills SHMINFO and starts
it.

- **CI smoke:** two boots of an 8 MiB image holding only the image. The
  second must load the environment the first saved.
- **Checks run by hand** (all passed under qemu-hisilicon):
  - An 8 MiB image with `uImage` at 0x50000 and rootfs at 0x250000 from
    the OpenIPC/firmware `nt98562_lite` and `nt98566_lite` releases boots
    to login on both machines; the kernel's device tree has `/memory` and
    `/hdal-memory/media` split at `osmem`; `/proc/mtd` matches `mtdparts`;
    `fw_printenv` works.
  - `sf erase/write/read` + `crc32` with an unaligned offset, length and
    buffer (the DMA and the PIO paths).
  - `saveenv` + `reset` (the watchdog).
  - TFTP recovery from a flash holding only the image
    (`-nic user,net=192.168.1.0/24,host=192.168.1.107,tftp=<dir>`), to
    login.
  - The boot limit: three `reset`s, then `altbootcmd` boots with
    `failsafe`.
  - `dhcp`, `ping`, a 3 MB `tftpboot` with matching `crc32`; Linux's
    driver gets its own lease after U-Boot used the MAC.
  - SD (`-drive if=sd,index=0,format=raw,file=<image>`): `mmc info`,
    `fatls`, `fatload` and `mmc read/write` against the host's CRCs;
    recovery of a changed kernel, and nothing rewritten with the same one.
- **What qemu does not check:** the loader at all (the DDR parameters, its
  flash and BCL1 handling: the build is the vendor's, the toolchain is
  not), the pin, clock, reset and pad set-up of every block, the flash
  controller's timing, the PHY's power-on order and MDIO clock, the SD
  host's command and FIFO timing.
- **On silicon:** nothing yet.

## NS02501: NT98532, NT98533

Two Cortex-A32s run in AArch32 (a GIC-400, the generic timer), chip ID
`0x5521`, NA51089's blocks at NA51089's addresses. `NVT_NS02501`
(`TARGET_NT98532`, `TARGET_NT98533`) builds the NA51089 board code with
the die's differences compiled in; the vendor sources are the NS02501
SDK's U-Boot 2021.10 (`nvt_ns02501_a32`, `nvt-ns02501`,
`eth_ns02501.c`, `nvt_ivot_mmc_a64.c` + `_ns02501.h`, `nvt_flash_spi`).

- **Loader** (`board/novatek/ns02501/loader`, `ns02501_loader_v3.07.101`):
  45056 bytes, its main part deflated (the SoC's gzip engine unpacks
  it). `CHIP` is the package, `532BD` / `533QD` here (the SDK boards':
  2 Gbit, 1066 MHz; `Configram533` has every package's DDR parameters,
  there is no ConfigRam directory). Built with `ROM_PUBLIC_API` off, it
  has no ROM LZMA: it carries LzmaDec (`bl_lzma.c`), as NA51055's. Any
  of arm-none-eabi, arm-linux-gnueabi 13 or 15 builds a working one.
- **The tree is packed.** The SDK trees are ~95 KiB (the vendor kernels
  take their clocks from `nvt-clk` in them): `nvt-image.py --fdt-lzma`
  puts the tree at 0x10000 in an LZMA BCL1 container (~8.5 KiB), which
  the loader unpacks (`bl_load_fdt_from_flash()`). Same layout otherwise.
- **Memory:** 256 MiB; SHMINFO at the SDK's `0xA00000` (`NVT_SHMINFO`);
  `osmem` 36M (the SDK boards'); `/hdal-memory` has two address cells
  (`[DDR id][address]`), which `ft_board_setup()` follows.
- **Console** UART0 at 48 MHz (the vendor's `ns02501_a32.h`); timer 0 as
  on NA51089; reset by the watchdog with CG+0x80 bit 8, CG+0x74 bit 19,
  CTRL `0x102` (`ns02501_hw_init.c`).
- **Flash:** TOP+0x04 [13:12] = 1, pads off GPIO 0x140F, a 240 MHz
  source, the tree's 12 MHz, fixed pad drive, output delay 3 from 48 MHz.
- **Ethernet:** its own pinmux bits (RMII_1 0x1, RMII_2 0x2, EXTPHYCLK
  0x100; the SDK boards' 0 is the embedded PHY), node `/eth0@f02b0000`,
  PHY MDIO address 1, the vendor's default trims (its eFuse reader is
  closed; Linux keeps what U-Boot set), AALE.
- **SD:** CGPIO13-18, clock source code 1 for 192 MHz (0 is 480 MHz),
  4-bit pad drive fields from PAD+0x50 (2 as the tree), the AXI channel
  enabled.
- **Not done:** the CPU clock (left as the loader sets it; the vendor
  caps it at 800 MHz without an eFuse code), the OTP 800 ms power-down
  wait, an external PHY's EVB reset GPIO, pad power from the tree.
- **Other packages** (532QB..533BE): the same U-Boot with a loader
  built for its `CHIP` and `NVT_DDR_SIZE` to match; no target each.

Verified under qemu-hisilicon (`nt98532`, `nt98533`, images from CI's
toolchains, the real loader): two boots with the env kept; dhcp, ping,
3 MB tftp + crc32, unaligned `sf`, `mmc info`/`fatls`/write+read;
`reset` makes the watchdog reset the machine (`-no-reboot` exits), but
the model does not run its loader again after any reset, its own
`system_reset` too (its core reset register at `0xFFE40000` keeps its
value, so core 1, which trains the DDR, is not started again), so the
reboot itself is not seen; SCTLR `0x00c5187d` at U-Boot's entry, `0x00c5187a` at the prompt;
the SDK's 5.10.168 kernel boots to its shell with `/memory`
`<0 0x2400000>` and media `<0 0x2400000 0xdc00000>`. **On silicon:**
nothing yet.

## The NVR dies: NT98332 (NA51103), NT98633 (NA51090)

Two (NA51103) or four (NA51090) Cortex-A53s, 1 / 2 GiB, the blocks at
`0x2_F000_0000` (`arch/arm/mach-novatek/include/mach/na511xx.h`), board
code in `board/novatek/na511xx/`, `NVT_NA511XX` selects `ARM64`
(`ARCH_NOVATEK` selects `CPU_V7A` only without it).

**Boot chain, and why only U-Boot is replaced.** Mask ROM (AArch32) ->
the board's loader (Xiongmai's FW98332A, the NVR's FW98633A; no source
for these dies in any SDK) -> DDR training -> RMR warm reset of the A53s
into AArch64 -> the board's BL31 (TF-A v2.2, TZASC over its own
megabyte at `0x1F00000`) -> U-Boot -> Linux, which brings the other cores
up through BL31's PSCI. BL31 enters BL33 at the address in its own
header (written from `/nvt_memory_cfg/uboot`: `0x1E000000` /
`0x3E000000`) with a fixed SPSR `0x3C9`, AArch64 EL2h: U-Boot is AArch64
and runs at EL2. The vendor's `_a32` configs are template copies and
cannot be entered from it. So the OpenIPC image is U-Boot's slot alone;
the loader, its tree and BL31 stay as the board has them.

| | NT98332 (Xiongmai NBD80N36RA, 16 MiB NOR) | NT98633 (N2032-8HDD, 128 MiB SPI NAND) |
| --- | --- | --- |
| loader / tree / BL31 | `0x0` / `0x10000` (`XMFDT32G` + LZMA) / in the loader | `0x0` / `0x40000` / `0x80000` |
| U-Boot slot | `0x20000`, 128 KiB, bare LZMA (`5D`, 16 MiB dict, end marker) | `0xC0000`, 896 KiB, BCL1 alg 11 (size field = length twice) |
| environment | `0x40000` (OpenIPC NOR layout from here: kernel `0x50000`, rootfs `0x250000`) | `0x1A0000`, the board's uenv, 2 blocks |
| then | | UBI `0x1E0000`..end (rootfs UBIFS with `/boot/uImage`) |
| mtdparts | `spi_nor.0:` OpenIPC NOR list | `spi_nand.0:1664k(boot),256k(env),-(ubi)` |

`openipc/nvt-nvr-image.py` builds the slot (HEADINFO length and
checksum as for NA51089, `BinFlag` 2); given a dump of the board's
flash it also writes a copy with the slot in. `${ubwrite}` writes only
the slot. Installing from the vendor U-Boot: tftp the slot, then
`sf erase 0x20000 0x20000; sf write <addr> 0x20000 0x20000` (NOR) or
`nand erase 0xc0000 0xe0000; nand write <addr> 0xc0000 0xe0000` (NAND).

**Peripherals.**
- Console UART0 `0x2_F028_0000`, 16550, 48 MHz; the architected timer;
  reset by the watchdog (NA51103 `0x60000`, NA51090 `0x110000`).
- MMU on (`mem_map` in `soc.c`): with it off every access is a device one
  and U-Boot's unaligned accesses fault. Caches on, so the drivers' cache
  maintenance matters.
- Flash: the NA51089 controller at `0x2_F018_0000`. NOR through
  `nvt_sflash.c` (per-die clock/pins/PHY; 128-byte FIFO); SPI NAND
  through `drivers/mtd/nand/raw/nvt_spinand.c`, the controller run as a
  plain SPI master (not its NAND mode), the chip's on-die ECC on.
- Ethernet: `nvt_eqos.c` (MDC divider fixed for the 150 MHz APB). NA51103
  uses the MAC at `0x440000`, NA51090 the one at `0x210000` (the tree's
  and Linux's eth0). Pins, clocks and the external PHY's reset GPIO come
  from the loader's tree (`/top/eth` pinmux), as eth_na51103.c /
  eth_na51090.c. phylib binds the PHY as MII, as the vendor does (its
  strapped RGMII delays stay).
- Memory: U-Boot stays in its 32 MiB region; staging at `0x2000000`
  (past BL31); the boot counter at `0x1E20000` (`0x1F20000` is BL31's);
  `mem=` and `/hdal-memory/media` (4-cell addresses) split at `${osmem}`
  (the boards' firmware: `0x15e00000`, `0x57600000`). The tree's
  `/reserved-memory` keeps BL31 out of the kernel's and bootm's way.
- Not supported: SD (no slot on the NVRs), USB/SATA/HDMI in U-Boot, the
  second MAC, the CPU clock raise the vendor U-Boot does from the tree.

**Verified under qemu-hisilicon** (`nt98332`, `nt98633`, the boards'
dumps with the slot and env replaced, the real loader and BL31): prompt
twice with the env kept; `sf`/`nand` erase/write/read + `crc32`
(unaligned on NOR); `saveenv` + `reset`; `ping`, `dhcp`, `tftpboot` 3 MB
with the host's crc32; UBI create/write/read across a reset; TFTP
recovery onto an empty flash then boot from it (NOR: squashfs layout;
NAND: UBIFS `/boot/uImage`); the boot limit's failsafe; the dumps' own
4.19 kernels boot (their uImages say "uncompressed" over LZMA data:
repack them with `-C lzma`). CI smoke: `u-boot-<soc>-qemu.bin` on the
stand-in loader (`-smp 2` / `-smp 4`). Not checked: the boards' DDR
contents across a watchdog reset (the boot counter), the controller's
timing and the NAND's ECC error paths, the PHYs. **On silicon:**
nothing yet.

## Things to keep

- `mem=` stays the first kernel argument, as on the other OpenIPC targets.
- The boot counter is the DRAM word at `0x01F20000`, the other families'
  DDR + 0x1f20000. A healthy boot does not clear it; past `bootlimit` the
  same kernel boots with `failsafe`, as on the other families.
- `${fdtaddr}` is set after `openipc_helper()`, so its first `saveenv`
  does not store it; it is this boot's malloc address.
- `ustrtoul()` dereferences its end pointer: never pass NULL.

## Conventions

- One topic per commit, `<area>: <what changed>` (`novatek:`,
  `nvt_sflash:`, `net:`, `mmc:`, `openipc:`, `ci:`).
- Commit bodies say what was verified, and where: the qemu machine, or a
  board.
