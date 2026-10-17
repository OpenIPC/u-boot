# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this branch is

`openipc-xm` is OpenIPC's U-Boot for Xiongmai's own camera SoCs: XM510, XM530
and XM550. It is **mainline U-Boot v2022.10**, the same release as
`openipc-hisilicon`, with:

- the OpenIPC boot layer, carried over from `openipc-hisilicon`;
- the XM SoC support, ported from the vendor SDKs' U-Boot 2014.04 trees.

None of the HiSilicon/Goke vendor code comes along.

CI and user docs live on the `openipc` branch:

- `.github/workflows/ci.yml` only calls `build-xm.yml@openipc`.
- The `openipc` branch has its own `CLAUDE.md`, on the workflow invariants:
  `git show origin/openipc:CLAUDE.md`.
- `openipc-hisilicon` has a `CLAUDE.md` too:
  `git show origin/openipc-hisilicon:CLAUDE.md`. Its OpenIPC-layer notes
  (default environment, `common/openipc.c`, env compatibility) apply here
  unchanged.

Adding or dropping a SoC needs both branches:

- here: `build.sh` (`build_soc`, `ALL_SOCS`) and `openipc/<soc>_config`;
- there: `build-xm.yml`'s matrices and the README.

## Relationship to mainline

```sh
git diff --stat v2022.10 HEAD                  # the whole delta, small
git diff --diff-filter=M v2022.10 HEAD         # the mainline files we patched
```

**Added**

| Area | Location |
| --- | --- |
| SoC: PLL and DDR bring-up, timer, reset, DRAM size, SDHCI glue | `arch/arm/mach-xm/` (`ARCH_XM`; `TARGET_XM510`/`XM530`/`XM550`; register map in `include/mach/xm.h`) |
| Boards: `board_init`, `misc_init_r` → `openipc_helper()`, `board_eth_init` with the PHY reset / MAC glue | `board/xiongmai/{xm510,xm530}/`. XM550 uses the xm530 board with `XM_DDR3` |
| Memory map, console, timer | `include/configs/{xm510,xm530}.h`, each ending in `#include "openipc-common.h"` |
| SPI NAND on the v2 controller (XM530/550): raw NAND, full-ID table, on-die ECC status through `ecc.read_page` | `drivers/mtd/nand/raw/xmsfc_nand.c` (`NAND_XMSFC`) |
| SPI NOR controller, v1 (XM510) and v2 (XM530/550), non-DM | `drivers/mtd/spi/xmsfc.c`. It provides `spi_flash_probe()` and fills `mtd->_read/_write/_erase`, so mainline's non-DM `spi_flash_*()` wrappers, `sf` and env-in-SPI-flash work without the SPI and spi-nor layers |
| OpenIPC boot layer | `common/openipc.c` and `include/openipc.h` (copied from `openipc-hisilicon`, NAND gate rekeyed to `CONFIG_ENV_IS_IN_NAND`), `include/configs/openipc-common.h` (XM-only rewrite, same env commands) |
| Kconfig fragments and the build | `openipc/`, `build.sh` |

**Patched mainline files**, each small:

- `arch/arm/{Kconfig,Makefile}`: the `ARCH_XM` hook.
- `common/{Kconfig,Makefile}`: `OPENIPC`, `PRODUCT_SOC`, `PRODUCT_SOCMODEL`.
- `drivers/mtd/nand/raw/{Kconfig,Makefile}`: the `NAND_XMSFC` hook.
- `drivers/mtd/spi/{Kconfig,Makefile}`, `cmd/Kconfig` (`CMD_SF`) and
  `env/Kconfig` (`ENV_IS_IN_SPI_FLASH`): accept `XMSFC_SPI_NOR`.
- `drivers/net/Kconfig`: `NETDEVICES` selects `DM_ETH` only when `DM` is on.
  That keeps DesignWare on the legacy `designware_initialize()` path with DM
  off, while driver-model boards keep `DM_ETH`.
- `drivers/serial/serial_pl01x.c`: the non-DM driver's `base_regs` and
  `pl01x_type` were `.data` variables set at init. Before relocation those
  writes go nowhere (see below), so the console stayed silent. They are now
  compile-time constants, which is what they always were.

Unlike `openipc-hisilicon`, no mainline file is replaced by an older vendor
copy. Ethernet (DesignWare), SD (SDHCI, which is what the vendor's "arasan"
driver programs), the console (PL011) and the timer (SP804 via `lib/time.c`
and `CONFIG_SYS_TIMER_*`) are mainline drivers.

## Running in place: the constraint that shapes everything

The boot ROM jumps straight into SPI NOR, and U-Boot runs from the flash
controller's read window until it relocates:

- **XM510:** `SYS_TEXT_BASE=0x0`, the v1 window.
- **XM530/550:** `SYS_TEXT_BASE=0x08000000`, an alias of the v2 window at
  0x60000000.

The link address must equal the run address, because `relocate.S` does no
position-independent fixups. The only RAM before DDR is a 4 KiB SRAM at
0x04000000. Consequences:

- **`lowlevel_init`** (`arch/arm/mach-xm/lowlevel_init.S`) overrides the weak
  armv7 one. It runs `xm_early_init()` on an SRAM stack (0x04001000): timer,
  PLLs, then **DDR**. That code must use no global data and write no `.data`
  or `.bss`. `CONFIG_SYS_INIT_RAM_ADDR` is then in DDR, so `board_init_f()`
  and `SYS_MALLOC_F` work as on any board.
- **Writes before relocation are silently lost:** the window ignores them, and
  qemu logs them only with `-d guest_errors`. Any driver or code that stores to
  a global before relocation breaks; the PL011 fix above is the example.
- **No compressed image.** The flashable image is `u-boot.bin` as linked; there
  is no `u-boot-z` as on HiSilicon. The NOR image must stay ≤ 256 KiB (env at
  0x40000), so check sizes after enabling anything. Thumb-2 and
  `CC_OPTIMIZE_FOR_SIZE` are on.
- **xmsfc register operations take the window out of bus-read mode.** The
  driver restores it (`xmsfc_bus_mode()`) after every v2 operation, and only
  ever runs after relocation, from DDR.

## SPI NAND (XM530/XM550): built on a guess

Xiongmai never shipped a NAND U-Boot from this SDK. There is a
`CONFIG_SYS_USE_SPI_NAND` block in the vendor `xm530.h`, but no board target,
and nothing documents how the boot ROM starts from SPI NAND.

**What the NAND image assumes:**

- the controller's window serves the NAND page data linearly, as in NOR
  bus-read mode;
- U-Boot is linked at 0x60001000 (the vendor's NAND `TEXT_BASE`), behind a
  4 KiB header;
- the header's first word is `ldr pc, [pc, #-4]` followed by that address, so
  it reaches U-Boot whether the ROM starts at the window or the 0x08000000
  alias (`build.sh`).

**What it carries:**

- layout `768k(boot),256k(env),-(ubi)`;
- the environment at 0xc0000;
- the kernel at `/boot/uImage` inside the UBIFS `rootfs` volume;
- the same recovery as on `openipc-hisilicon` NAND.

Only qemu-hisilicon's NAND model, which implements the same guess, has run
it. The OpenIPC XM530 kernel has no SPI NAND driver either, so a NAND image
loads and starts the kernel, which then cannot mount its root. Treat NAND as
unverified on silicon, and say so wherever it is offered.

## Build

```sh
bash build.sh               # xm510 xm530 xm550; continues past failures, exits non-zero if any failed
bash build.sh xm530         # one or more SoCs (CROSS_COMPILE defaults to arm-linux-gnueabi-)
```

- Output: `output/u-boot-<soc>-nor.bin`, plus `-nand.bin` for xm530/xm550,
  built out of tree in `output/build-<soc>-<nor|nand>/`.
- `.config` is `openipc/openipc_config`, then `openipc/<soc>_config`, then
  `openipc/nand_config` for NAND images, then `olddefconfig`. A later line
  wins. The "override" warnings are that layering.
- There are no `configs/*_defconfig` for these targets.
- The host toolchain must match CI (Ubuntu `gcc-arm-linux-gnueabi`). A Docker
  image with it works:
  `docker run --rm -u $(id -u):$(id -g) -v $PWD:/src -w /src <image> bash build.sh`.

## Verification

The emulator is widgetii/qemu-hisilicon, with machines `xm510`, `xm530` and
`xm550`. They boot the image in place from `-M <soc>,flash-file=<8 MiB image>`,
with the reset vector at the window or alias, exactly as silicon does.

- **CI smoke:** pass on `Hit Ctrl-C`.
- **Checks to run by hand:**
  - `sf erase/write/read` + `cmp.b` with an unaligned length;
  - `saveenv` + `reset`;
  - a full 8 MiB image (U-Boot at 0, `uImage` at 0x50000, rootfs at 0x250000,
    from OpenIPC/firmware `latest` `openipc.<xm510|xm530>-nor-lite.tgz`;
    XM550 uses xm530) reaching `login:` with `mem=` first on the kernel
    command line;
  - TFTP recovery from an empty flash
    (`-nic user,net=192.168.1.0/24,host=192.168.1.107,tftp=<dir>`);
  - the boot limit: two `reset`s, then `altbootcmd` with `failsafe`.
- **SR3.WPS:** `-global xm-sfc.wps=on` powers the part up as Xiongmai
  bootloaders leave it, with per-block locks on. `xmsfc_unprotect()` must clear
  it before writes land.

- **NAND:** `-global xm-sfc.nand-file=<raw image>`, with no `flash-file`.
  - The raw image is pages of 2048 + 64 bytes, U-Boot at page 0, and
    `ubinize`'s output at 1 MiB.
  - It must reach the prompt, save the env, mount the rootfs and start the
    kernel.
  - From an erased NAND, TFTP recovery must create the UBI volumes and boot.

**What qemu does not check:** DDR training values, the v2 bus-read restore and
PHY/MDIO timing are acked instantly. A change to those needs a real board.

## Things to keep

- `mem=` must stay the **first** kernel argument. `load_xiongmai` places the
  vendor MMZ from `$1` of `/proc/cmdline`.
- The MTD name is `xm_sfc`, the vendor kernel driver's (`SFC` in
  `openipc-common.h`).
- The boot counter is a DRAM word:
  - 0x81f20000 on XM530/550;
  - 0x81b00000 on the 32 MiB XM510, where 0x81f20000 is inside U-Boot's
    relocated heap.

  The XM kernels have no device tree to reserve it, and the XM firmware does
  not ship `openipc-failsafe`, so nothing clears it. The boot-limit path
  therefore boots the same kernel with a `failsafe` argument that this
  firmware ignores.
- The vendor's `ATAG_XMINFO` is not passed. The OpenIPC XM kernels only show it
  in `/proc/xm/xminfo`.

## Conventions

- One topic per commit, `<area>: <what changed>` (`xm:`, `xmsfc:`, `openipc:`,
  `serial:`, `ci:`).
- Bodies say what was verified, and where (qemu machine, or board).
