# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this branch is

`openipc-novatek` is OpenIPC's U-Boot for Novatek's camera SoCs:

- NA51089, one Cortex-A9: the NT98560 (external DDR), NT98561 and NT98562
  (64 MiB of DDR in the package) and NT98566 (128 MiB);
- the NT9852x SDK's two dies, two Cortex-A9s each, NA51089's blocks at
  NA51089's addresses: NA51055 (NT98520, NT98522, NT98523, NT98525) and
  NA51084 (NT98528, NT98529); see "The NT9852x dies" below;

and for two NVR dies, NA51103 and NA51090 (see "The NVR dies"). The tree
is **mainline U-Boot v2022.10**, the same release as `openipc-hisilicon`,
`openipc-xm`, `openipc-gk710x` and `openipc-anyka`, plus:

- the OpenIPC boot layer, as on `openipc-anyka`;
- the NA51089 support, written from the vendor sources: the NT9856x Linux
  SDK v1.01.007 (U-Boot 2019.04, kernel 4.19.91), and the device tree of a
  Xiongmai NT98562 camera; the NT9852x support from that SDK's U-Boot
  2019.04 and board trees;
- the SDKs' loaders, which run before U-Boot, as Novatek ships them
  (`board/novatek/na51089/loader`, `board/novatek/na51055/loader`), built
  from their sources.

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
AArch64 targets behind the board's own loader and BL31 (see "The NVR
dies"), the NA51068 DVR/NVR die (NT98321, NT98323) on SPI NOR (see
"NA51068"), and NS02501 (NT98532, NT98533) with NA51089's layout and its
own loader (see "NS02501"). The other dies of the SDKs (NA51102, the
other NS02xxx parts) are not supported.

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

**It calls CodeEntry with its MMU and caches on**: SCTLR 0x00c5187d, its
translation table at 0x1018000 (in its DRAM area, `/nvt_memory_cfg/loader`)
mapping the DDR write-back write-allocate and the registers as device
memory. This U-Boot runs with the data cache off and no cache maintenance
(`SYS_DCACHE_OFF`), so `cpu_init_cp15` has to run: `openipc_config` has
`SKIP_LOWLEVEL_INIT_ONLY` (no `lowlevel_init`, the loader has the clocks
and DDR up), not `SKIP_LOWLEVEL_INIT`, which skipped it too. Without it the
cache stayed live under the MAC's and the flash controller's DMA, and
anything loaded to 0x1000000 (`${baseaddr}`) overwrote the table. At the
prompt SCTLR reads 0x00c5187a.

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
- **SPI NAND** (`<soc>-nand`: NT98560/562/566, NT98525). The SDKs' NAND
  boards' layout, 128 KiB blocks: loader `0x0`, tree `0x40000`, its copy
  (`fdt.restore`) `0x80000`, U-Boot `0xC0000` (a 2 MiB partition),
  environment `0x2C0000` (two blocks for one bad), UBI from `0x300000`:
  `spi_nand.0:2816k(boot),256k(env),-(ubi)`. The loader is built with
  `STORAGEINT=NandSpi` (the libraries' `STRGINT_NandSpi.a`) and reads the
  tree from block 2 and its partitions from `/nand`:
  `openipc/<soc>-nand.dts` is the board's tree plus `openipc/nvt-nand.dtsi`
  (the partitions and nvtpack index in `/nand`, `/nor` disabled and
  emptied, `nvt_info` EMBMEM). `nvt-image.py --nand` writes both tree
  copies and ends the image at U-Boot's last block (1 MiB); `${ubwrite}`
  erases the 2816 KiB boot area and writes it at 0. The NT98525's NAND
  loader is 34.5 KiB with the SD card update: `STORAGEEXT=NONE`, as the
  528 model. U-Boot: `openipc/nand_config` after the SoC's fragment
  (`nvt_spinand.c`, UBI, UBIFS, an 8 MiB malloc), the NAND env in
  `include/configs/openipc-common.h` and the offsets in `novatek.h`.
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

## Recovery

For a board whose flash has no valid loader. Novatek's mask ROM falls
through flash, SD, Ethernet (DHCP and TFTP), USB, UART, or the straps
pick one (BS[3:0]; NT9856x: 4 Ethernet, 0xC UART; NT9832x BS[2:0]: 3, 4).
From any of them it takes a loader into SRAM, nothing more: see the SDKs'
"Boot ROM code User Guide" and "PCTOOL User Guide".

`build.sh` writes `output/recovery-<soc>-<flash>/` for every target with
a loader in the tree (not the NVRs: no loader source):

| File | What it is |
| --- | --- |
| `loader.bin` | the loader built `STORAGEEXT=Eth`, under the name an `Eth` build gets (the library's own string): the name the ROM fetches is inferred from that, no document says so |
| `wrloader.bin` | the loader built `STORAGEEXT=Uart`. Not on NA51055 / NA51084: no UART library, no boot-source API, no ROM guide in that SDK |
| `FW98<part>T.BIN` | the pack: an all-in-one image (`nvt-image.py --pack`) of the plain tree and plain U-Boot, the loader's "run" file (`bl_process_all_in_one()`, `FUNC_RUN_CARD`) |

Nothing here writes the flash:

- **Ethernet:** the library (`STRGEXT_Eth.a`) registers TFTP as the
  loader's filesystem. OpenIPC's change: `bl_open_update_file()` keeps an
  `Eth` loader from asking for `LD98xxxA.BIN` and `FW98xxxA.BIN`, the
  files that would be flashed, and the library hangs on TFTP's "file not
  found" anyway. So serve `FW98xxxT.BIN` or it never returns. Host:
  `dnsmasq --port=0 --enable-tftp --tftp-root=<set> --dhcp-range=...`.
- **UART:** `openipc/nvt-uart-boot.py PORT FW98xxxT.BIN` speaks to a
  running `wrloader.bin` (`uart_upgrade.c`, `scsi_op.h`): 16-byte commands,
  the pack by `0x04` to the loader's temp address, `0x87`, `0x82`. A UART
  build serves UART whatever the straps say (the stock one only did with
  BS = 0xC, and took USB under the ROM's auto-boot). 4 KiB a command: a
  reply within the timeout is the only progress there is.
- **Not done: the ROM's half of UART.** Baud rate, handshake and load
  address of `wrloader.bin` are in no SDK document; the tool cannot send
  it yet. Nor is `Eth`'s file name, DHCP options or BS[9:8] checked on
  silicon. NA51068's embedded PHY is MAC1.
- **Size:** the `Eth` loaders of NA51055 and NA51068 have no LZMA decoder,
  to fit (they run the plain pack; a flash boot from them stops at "lzma").

Under qemu-hisilicon the ROM's fetch is a host file:
`-M <soc>,flash-file=<blank>,boot-file=<set>/loader.bin,boot-strap=4
-nic user,tftp=<set>` (NA51068: strap 3, and `-nic user` twice, MAC1 is
the second), the real loader and library on from there; with
`boot-file=<set>/wrloader.bin` and `-serial tcp:...` for the UART tool.

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
bash build.sh               # every SoC; continues past failures, exits non-zero if any failed
bash build.sh nt98566       # one or more SoCs (CROSS_COMPILE defaults to arm-linux-gnueabi-)
bash build.sh nt98566-nand  # the SPI NAND image (NAND_SOCS: nt98560 nt98562 nt98566 nt98525)
```

- The output is `output/u-boot-<soc>-nor.bin` (`-nand.bin`), built out of
  tree in `output/build-<soc>-nor/` and `output/build-<soc>-nor-loader/`
  (`-nand`, `-nand-loader`).
  `output/recovery-<soc>-<flash>/` is the recovery set (see Recovery).
- `.config` is `openipc/openipc_config`, then `openipc/<soc>_config`, then
  `olddefconfig`. There are no `configs/*_defconfig` for these targets.
- The loader uses `LOADER_CROSS_COMPILE`: `arm-none-eabi-` if there is one,
  else `CROSS_COMPILE` (refused for the NT98528/529).
- U-Boot is ~233 KiB, ~94 KiB packed (NA51068: 242 KiB, 98 KiB), in the 128 KiB slot;
  `nvt-image.py` fails the build if a part does not fit. Check the packed
  size after enabling anything.
- nt98332 and nt98633 are AArch64: `CROSS_COMPILE_A64`, default
  `aarch64-linux-gnu-` (Debian/Ubuntu `gcc-aarch64-linux-gnu`; CI must
  install it). Their outputs are `output/u-boot-nt98332-nor.bin` /
  `u-boot-nt98633-nand.bin` (U-Boot's slot alone) and
  `u-boot-<soc>-qemu.bin` (the stand-in's boot area, for CI).

## Verification

The emulator is OpenIPC/qemu-hisilicon (`docs/novatek.md` there), a
machine per SoC but the NT98520 and NT98522, which run on the NT98525's
and NT98523's (the same dies). Since OpenIPC/qemu-hisilicon#222,
`-M <soc>,flash-file=<image>` runs the image's own loader through a mask
ROM model: the DDR training, the device tree at 0x10000, U-Boot's BCL1
container, SHMINFO, the jump. (Before, a stand-in did the loader's job
without running it.)

- **CI smoke:** two boots of an 8 MiB image holding only the image. The
  second must load the environment the first saved. The `-nand` images
  boot from a 128 MiB one (the machine's W25N01; its mask ROM model reads
  the loader from NAND block 0).
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
  - SPI NAND (each `-nand` image): `nand write/read` of 3 MB with a
    matching `cmp`; `ubi part ubi`, `ubi create/write/read` of 2 MB;
    `run ub` (the image from TFTP, `${ubwrite}`), then `reset` boots the
    rewritten image through its loader and the volume is still there.
  - SD (`-drive if=sd,index=0,format=raw,file=<image>`): `mmc info`,
    `fatls`, `fatload` and `mmc read/write` against the host's CRCs;
    recovery of a changed kernel, and nothing rewritten with the same one.
- **What qemu does not check:** the DDR parameters against real DDR (the
  model's training succeeds), caches, the pin, clock, reset and pad set-up
  of every block, the flash controller's timing, the PHY's power-on order
  and MDIO clock, the SD host's command and FIFO timing.
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

## The NT9852x dies: NA51055, NA51084

The NT9852x Linux SDK's dies: NA51055 (chip ID 0x4821; NT98520, NT98522,
NT98523, NT98525) and NA51084 (0x5021; NT98528, NT98529). Two Cortex-A9s
each, NA51089's blocks at NA51089's addresses; one U-Boot code base with
NA51089 (`NVT_NA51055`, board `na51089`), told apart at run time by the
chip ID (`nvt_chip_id()`), as the vendor's U-Boot 2019.04 does.

- **The loader** (`board/novatek/na51055/loader`) is
  `nt9852x_loader_release_v2.05.013`, the SDK's `na51055_ldr`: the
  NA51089 loader's toolchain changes, plus
  - an LZMA decoder (`bl_lzma.c`, U-Boot's `lib/lzma` LzmaDec): its mask
    ROM has none and its own LZ leaves U-Boot ~176 KiB, past its 128 KiB
    slot. The image is then the same as NA51089's.
  - `LOADER_TYPE` follows `CHIP` (the 528/529 flow), through
    `LibExt/LIBExt_src/Ctrl_Flow/Makefile`, which builds `bl_func.c`.
  - The image is limited to 32 KiB (64 KiB for the 528 model): the 52x
    loaders end at ~32.73 KiB; the 528 one is built without its own SD
    card update (`STORAGEEXT=NONE`).
  - The 528 model needs a bare-metal toolchain: built with
    arm-linux-gnueabi it stops after the DDR training (why is open);
    arm-none-eabi and the SDK's Linaro arm-eabi work. `build.sh` refuses
    a Linux toolchain for it.
- **DDR** (`build.sh`): the NT98522/523's in the package (512 Mbit,
  1 Gbit) with no DDR2; the others' on the board, the SDK boards' 4 Gbit
  DDR3 dies (NT98520: one; NT98525/528/529: two, the second at
  0x40000000, the media pool's alone, which `ft_board_setup()` keeps).
  A board with other DDR needs its own loader build.
- **Device trees:** `openipc/na51055.dtsi` and `na51084.dtsi` are the
  SDK's NT98525 and NT98528 NOR boards, flattened, with the OpenIPC chain
  in `/nor`, U-Boot at 0x03900000 and the loader's scratch below it; the
  parts' trees change what their SDK boards change. The SDK's NA51055
  boards describe one CPU, its NA51084 ones two (`cpu-release-addr`
  0xF07F8000). `osmem`: 64M on the NT98523 (its CMA is at 48 MiB), 256M
  on the 512 MiB parts.
- **What differs from NA51089** (vendor `nvt_na51055_a32/clock.c`,
  `eth_na51055.c`, `nvt_ivot_mmc.c`):
  - PLL8's ratio at CG+0x1188 (NA51055) / CG+0x47E0 (NA51084); NA51084
    runs 1000 MHz unless the tree asks for 1200/1350, which only the
    NT98529 gets.
  - Ethernet: own pin and LED bits, the external PHY selected by bit 8 of
    0x3004, NA51084's RMII_2 and RGMII, no PHY PLL reset; the DMA's
    descriptor skip counts in 4-byte units on NA51055, so the NT9852x
    targets pack the descriptors (16 bytes; the data cache is off, a
    build-time check says so); AALE.
  - SD: NA51055 samples on the internal clock (SRC_CLK_SEL).

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
## NA51068

The NT9832x SDK v1.02.024's die: NT98321 (DDR3) and NT98323 (DDR4), two
Cortex-A9s (MPCore at `0xF9C00000`), Faraday-heritage peripherals,
external DDR (512 MiB or more on every SDK board). `TARGET_NT98321`,
`TARGET_NT98323`, `NVT_NA51068`; `board/novatek/na51068`,
`arch/arm/mach-novatek/na51068.c`, `include/mach/na51068.h`,
`openipc/na51068.dtsi` (the SDK's `cfg_98323_DDR4_4GX1_NOR_EVB` tree with
the OpenIPC `/nor` chain; `nt98321.dts` changes what the 321 EVB does).

- **Boot:** as NA51089, same image (`nvt-image.py`): loader 0 (64 KiB),
  tree 0x10000, BCL1 LZMA U-Boot 0x20000. The loader
  (`board/novatek/na51068/loader`, `na51068_loader_v1.08.014`) runs from
  SRAM `0xFE080000` and then from DDR `0x100000`; its mask ROM has no
  routines, its LZMA decoder is `lzma_inflate()` of its own `Compress.a`
  (OpenIPC hooks algorithm 11 to it). Models `323_DDR4_4GX1_NOR_EVB` and
  `321_DDR3_4GX1_NOR_EVB` (SpiFlash, `STORAGEEXT NONE`: with the SD
  update code the image is over 64 KiB). The tree goes to `0x80000`,
  SHMINFO `0xA00000`, U-Boot `0x1E000000` (the SDK's), the second core's
  pen `0x1FF80000` (HEADINFO BinCtrl SMP).
- **The loader jumps with its MMU on**, page tables in its DDR region:
  `SKIP_LOWLEVEL_INIT_ONLY` (not `SKIP_LOWLEVEL_INIT`) so that
  `cpu_init_cp15` turns it off; else writes to `0x100000+` hang.
- **Timer:** the A9 global timer at PLL16's rate (CG `0x107C0`+0x20..28,
  12 MHz x ratio / 2^17; an eighth of the CPU). The CPU clock is the
  loader's: the vendor U-Boot's raise to 1400 MHz needs the core voltage
  set by PWM from eFuse trim, not ported.
- **Console** UART0 `0xFE200000`, 48 MHz. **Reset** the watchdog
  `0xFE8A0000`, as the vendor kernel.
- **SPI NOR:** FTSPI020 `0xFA900000`, `drivers/mtd/spi/nvt_spi020.c`
  (`NVT_SPI020`), PIO, single lane, 4-byte opcodes past 16 MiB.
- **Ethernet:** two `nvt_eqos` MACs (`0xFCC00000` eth0, `0xFCD00000`
  eth1), each RGMII, RMII or the embedded PHY per `/top@fe030000/eth`
  pinmux (`0x1201`: MAC0 RGMII, embedded PHY on MAC1). Both registered;
  `eth_init` moves on to the next without link.
- **SD:** a standard SDHCI at `0xFA600000`, mainline `sdhci.c` non-DM
  (`add_sdhci`).
- **Kernel:** `osmem` 192M (323) / 160M (321) as the SDK boards;
  `CONFIG_SYS_BOOTMAPSZ` 128 MiB: the SDK's kernel 4.9 hangs silently
  with the tree and initrd just below 192 MiB.
- **NAND:** the qemu machine picks NAND for a 128/256 MiB image; NT98321
  boards usually boot from SPI NAND, which needs a SPI020 NAND driver and
  a UBI layout: not done.
- **Verified under qemu** (`-M nt98323` / `nt98321`, real loader): two-boot
  smoke; `sf` read/erase/write at odd offsets (16 and 32 MiB); `saveenv` +
  `reset`; dhcp, ping, 3 MB tftp on both MACs; SD `mmc read/write`,
  `fatls`, recovery of `uImage.<soc>.img`; the SDK's kernel 4.9.118 +
  NT9852x ramdisk to a shell with `${fdtaddr}`. **On silicon:** nothing.

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
