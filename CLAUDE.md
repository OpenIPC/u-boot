# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this branch is

`openipc-gk710x` is OpenIPC's U-Boot for Goke's own ARM1176 camera SoCs, the
GK7101 and GK7102. These are not the HiSilicon-derived GK7205/GK7605 that
`openipc-hisilicon` covers. The tree is **mainline U-Boot v2022.10**, the
same release as `openipc-hisilicon` and `openipc-xm`, plus:

- the OpenIPC boot layer, carried over from `openipc-xm`, which took it from
  `openipc-hisilicon`;
- the GK710x support, written from the vendor SDK (`GK710X_LinuxSDK_v2.0.0`,
  U-Boot 2012.10 and Linux 3.4.43).

No vendor driver comes along. The vendor's HAL blob is carried, for the
kernel only (see below).

CI and user docs live on the `openipc` branch:

- `.github/workflows/ci.yml` only calls `build-gk710x.yml@openipc`.
- `git show origin/openipc:CLAUDE.md` covers the workflow invariants.
- `git show origin/openipc-hisilicon:CLAUDE.md` covers the OpenIPC layer
  (default environment, `common/openipc.c`, env compatibility). It applies
  here unchanged.

Adding or dropping a SoC needs both branches:

- here: `build.sh` (`build_soc`, `ALL_SOCS`), `openipc/<soc>_config` and
  `openipc/<soc>.rvs`;
- there: `build-gk710x.yml`'s matrices and the README.

GK7101S/GK7102S (`gk7102s` in OpenIPC/firmware) are a different die with a
different HAL and register map. They are not supported here, and
qemu-hisilicon does not model them.

## Relationship to mainline

```sh
git diff --stat v2022.10 HEAD                  # the whole delta, small
git diff --diff-filter=M v2022.10 HEAD         # the mainline files we patched
```

**Added**

| Area | Location |
| --- | --- |
| SoC: clocks, timer, watchdog reset, DRAM size, pin functions | `arch/arm/mach-gk710x/` (`ARCH_GK710X`; `TARGET_GK7101`/`GK7102`; register map in `include/mach/gk710x.h`) |
| Kernel hand-off: HAL blob, U2K words | `arch/arm/mach-gk710x/kernel.c`, `hal_data.h` |
| Board: GPIO maps, internal Ethernet PHY, `misc_init_r` → `openipc_helper()` | `board/goke/gk710x/` |
| Memory map, console, timer | `include/configs/gk710x.h`, ending in `#include "openipc-common.h"` |
| SPI NOR controller, non-DM | `drivers/mtd/spi/gk_sflash.c` (`GK_SFLASH`). It provides `spi_flash_probe()` and fills `mtd->_read/_write/_erase`, as `xmsfc` does on `openipc-xm` |
| OpenIPC boot layer | `common/openipc.c`, `include/openipc.h` (as on `openipc-xm`), `include/configs/openipc-common.h` (GK rewrite, NOR only, same env commands) |
| Kconfig fragments, boot header, the build | `openipc/`, `build.sh` |

**Patched mainline files**, each small:

- `arch/arm/{Kconfig,Makefile}`: the `ARCH_GK710X` hook.
- `common/{Kconfig,Makefile}`: `OPENIPC`, `PRODUCT_SOC`, `PRODUCT_SOCMODEL`.
- `drivers/mtd/spi/{Kconfig,Makefile}`, `cmd/Kconfig` (`CMD_SF`) and
  `env/Kconfig` (`ENV_IS_IN_SPI_FLASH`): accept `GK_SFLASH`.
- `drivers/net/Kconfig`: `NETDEVICES` selects `DM_ETH` only when `DM` is on,
  as on `openipc-xm`.
- `drivers/net/designware.c`: the non-DM `designware_initialize()` uses an
  MDIO bus the board has already registered under the device's name. The
  GK710x PHY is not on the MAC's MDIO lines.

The console (NS16550), the timer (`lib/time.c` over a raw counter) and
Ethernet (DesignWare) are mainline drivers.

## The HAL, and why there is no vendor driver

The vendor U-Boot and kernel never touch a register directly. Every access
goes through a HAL blob, written against a fictitious map (AHB `0x90000000`,
APB `0xA0000000`). The HAL:

1. moves each block to its real place;
2. reorders registers inside some blocks;
3. permutes the bits of some registers (RCT PLLs, timer and watchdog
   control, pad control).

This tree programs the silicon directly. Every address and value in
`include/mach/gk710x.h`, `soc.c` and `board/goke/gk710x/gk710x.c` is the
store the HAL itself makes. It was found by executing the HAL's `hw_writel`
under Unicorn (qemu-hisilicon `qemu-boot/gk710x-halmap.py`). To port another
vendor register write, run it through the HAL the same way. Do not guess
from the vendor `*_reg.h`.

The kernel still needs the blob (see "Hand-off to the kernel").

## Boot

- The mask ROM reads the header at flash offset 0. It runs the register
  writes and polls listed there (eFuse, DDR PHY, DDR controller), copies
  U-Boot to `0xC0800000` (`SYS_TEXT_BASE`) and starts it.
  - DDR is up before the first instruction, so there is no SPL, and
    `lowlevel_init` is empty.
- `openipc/gk710x-image.py` builds that header:
  - the words around the table are as in every SDK prebuilt and camera
    flash seen;
  - the table is `openipc/<soc>.rvs`, taken from the SDK's prebuilt
    U-Boots, which matches what is on cameras in the field;
  - GK7101 and GK7102 differ only in the DDR controller word at
    `0x60004004` (128 vs 64 MiB). `dram_init()` reads the same word back.
- **Byte order.** The flash controller packs bytes MSB first. U-Boot, the
  ROM and the vendor kernel all move whole little-endian words through it, so:
  - the chip holds every 32-bit word byte-swapped;
  - a camera dump reads as big-endian ("GK7101" in a hex editor);
  - `output/u-boot-<soc>-nor.bin` is in CPU order, which is what `sf write`
    and the kernel's MTD driver expect;
  - a raw image for a programmer, or for qemu's `flash-file=`, needs every
    word swapped.
  - `gk_sflash.c` keeps that mapping for unaligned offsets as well: whole
    words only, writes padded with `0xff`.

## Hand-off to the kernel

The vendor kernel, OpenIPC's `gk7102` included, needs more than ATAGs.
`board_prep_linux()` in `mach-gk710x/kernel.c` provides:

- **The HAL** at `0xC0012000`, with its address in U2K word `0xC0000008`.
  The kernel calls `hal_init()` first thing and refuses a HAL older than
  20160324. The carried one is 20160913.
- **U2K words** at `0xC0000000`:
  - total DDR in MiB;
  - `mem=` in bytes, from `osmem`;
  - the bitstream buffer, from `bsbsize`;
  - `phytype`;
  - the `0x7101` valid marker.

  The kernel places BSB, DSP and USR memory from these. `mem=` on the
  command line still sets the kernel's own size.
- **The GPIO map** (vendor `gpio_cfg_t`) in the top 64 KiB of DDR.
  - The kernel configures every pin from it ("gpio map get from uboot"):
    sensor I2C and reset, IR cut, SD, Ethernet LED and PHY clock.
  - Without it a camera boots with no sensor bus.
  - `board_get_usable_ram_top()` keeps U-Boot below that 64 KiB.

The SDK's 20-odd board GPIO headers come down to two layouts
(`board/goke/gk710x/gpio_map.c`):

- `v2`, the default: rb-sc1035 v2.1, sc1045, imx225, ov9750, jxh42 v2.0,
  gc1024, GK7101 rb-ov2710 v2.2;
- `v1`: rb-ov9712, ar0130, jxh42 v1.0, GK7101 imx222.

`gpiomap=v1` in the environment selects the second. U-Boot also applies the
selected map at `misc_init_r()`, as the vendor U-Boot did, so its own
Ethernet gets its PHY clock pin. Other vendor boards (EVBs, rb-ov2710 v1/v2.0/v2.1,
ps1210, bg0703, ar0230) have other maps and are not covered.

## Peripherals

- **Console:** UART0 at `0x70005000`, a 16550 with 32-bit registers, 24 MHz.
  `arch_cpu_init()` sets the vendor's CPU PLL and UART divider.
- **Timer:** timer 1 at `0x7000B000`, counting down at the 69 MHz APB
  clock. **Reset:** the watchdog.
- **Ethernet:** a DesignWare GMAC at `0x6000E000` with the SoC's own
  10/100 PHY.
  - The PHY's MII registers are memory mapped at `0x60022000 + 4n`. The
    board registers them as the MDIO bus, PHY address 0.
  - `gk_ephy_init()` does the vendor's PHY clock (audio PLL, out on a pin),
    analog trim and MAC/PHY mux.
  - External-PHY boards (`phytype=1/2`) are not handled.
- **SD:** not enabled. The vendor U-Boot had none, the controller's base
  clock is unknown, and the SD pins depend on the board map. SD recovery in
  `common/openipc.c` is compiled out with `CONFIG_MMC`.
- **SPI NAND:** not supported. OpenIPC's GK7102 firmware is NOR only.

## Build

```sh
bash build.sh               # gk7101 gk7102; continues past failures, exits non-zero if any failed
bash build.sh gk7102        # one or more SoCs (CROSS_COMPILE defaults to arm-linux-gnueabi-)
```

- The output is `output/u-boot-<soc>-nor.bin`, built out of tree in
  `output/build-<soc>-nor/`.
- `.config` is `openipc/openipc_config`, then `openipc/<soc>_config`, then
  `olddefconfig`.
- There are no `configs/*_defconfig` for these targets.
- There is no Thumb-2 on ARM1176, so the image is ~237 KiB.
  `BOARD_SIZE_LIMIT` keeps header + image under the 256 KiB boot
  partition. Check the size after enabling anything.

## Verification

The emulator is widgetii/qemu-hisilicon, machines `gk7101` and `gk7102`
(`docs/goke-gk710x.md` there). `-M gk7102,flash-file=<image>` does the boot
ROM's job from the header, so it boots exactly the flashed bytes. The image
must be byte-swapped as described above.

- **CI smoke:** two boots of an 8 MiB image holding only U-Boot. The second
  must load the environment the first saved.
- **Checks to run by hand:**
  - A full 8 MiB image reaches `login:`. It has U-Boot at 0, `uImage` at
    0x50000 and rootfs at 0x250000, from OpenIPC/firmware `latest`
    `openipc.gk7102-nor-lite.tgz` (gk7101 boots the same).
  - The kernel log shows `hal version = 20160913`, BSB/DSP/USR inside DDR,
    and `[BOOT VERSION] GK7102 openipc-v2`.
  - `sf erase/write/read` + `cmp.b` with an unaligned offset and length.
  - `saveenv` + `reset`; `fw_printenv` from Linux.
  - TFTP recovery from a flash holding only U-Boot
    (`-nic user,net=192.168.1.0/24,host=192.168.1.107,tftp=<dir>`).
  - The boot limit: two `reset`s, then `altbootcmd` boots with `failsafe`.
- **What qemu does not check:**
  - DDR timing (the header is the vendor's, so this is low risk);
  - PLL and clock values;
  - pad control;
  - the PHY's analog trim and real link.

  None of this has run on silicon yet.

## Things to keep

- `mem=` stays the first kernel argument, as on the other OpenIPC targets.
  `osmem`/`bsbsize` must leave room for the DSP:
  - GK7102: 36M + 2M of 64 MiB;
  - GK7101: 50M + 4M of 128 MiB.
- The MTD name is `gk_flash`, the vendor kernel driver's (`SFC` in
  `openipc-common.h`). The console is `ttySGK0`.
- The boot counter is the DRAM word at `0xC1F20000`, like the other
  families' DDR + 0x1f20000. A healthy boot does not clear it, and the
  GK7102 firmware ships no openipc-failsafe. After two reboots the boot
  limit path therefore boots the same kernel with `failsafe`, which that
  firmware ignores, as on `openipc-xm`.

## Conventions

- One topic per commit, `<area>: <what changed>` (`gk710x:`, `gk_sflash:`,
  `openipc:`, `net:`, `ci:`).
- Commit bodies say what was verified, and where: the qemu machine, or a
  board.
