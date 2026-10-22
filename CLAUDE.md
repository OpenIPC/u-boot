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

The other dies of the SDKs (NA51055, NA51068, NA51090, NA51102, NA51103,
the NS02xxx parts) are not supported.

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
