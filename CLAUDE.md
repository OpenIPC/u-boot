# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this branch is

`openipc-fullhan` is OpenIPC's U-Boot for Fullhan's IP camera SoCs. The
tree is **mainline U-Boot v2022.10**, the same release as
`openipc-hisilicon`, `openipc-xm` and `openipc-gk710x`, plus:

- the OpenIPC boot layer, carried over from `openipc-xm` / `openipc-gk710x`
  (`common/openipc.c` is the same file);
- the Fullhan support, written from the vendor SDKs' U-Boots (2010.06) and
  kernels (3.0.8 and 4.9.129), and from camera flash dumps.

No vendor driver comes along. Two vendor first stages (ramboots) and the
DDR scripts of the boot header are carried as data (see "Boot").

CI and user docs live on the `openipc` branch:

- `.github/workflows/ci.yml` only calls `build-fullhan.yml@openipc`.
- `git show origin/openipc:CLAUDE.md` covers the workflow invariants.
- `git show origin/openipc-hisilicon:CLAUDE.md` covers the OpenIPC layer
  (default environment, `common/openipc.c`, env compatibility). It applies
  here unchanged.

Adding or dropping a SoC needs both branches:

- here: `build.sh` (`build_soc`, `ALL_SOCS`) and `openipc/<soc>_config`;
- there: `build-fullhan.yml`'s matrices and the README.

## Targets

| Image (`<soc>`) | Target | Generation | CPU | DDR |
| --- | --- | --- | --- | --- |
| `fh8852v200`, `fh8852v210` | `TARGET_FH8852V200` | FH885xV200 | ARM1176 | 64 MiB |
| `fh8856v200`, `fh8856v210` | `TARGET_FH8856V200` | FH885xV200 | ARM1176 | 128 MiB |
| `fh8858v200`, `fh8858v210` | `TARGET_FH8858V200` | FH885xV200 | ARM1176 | up to 256 MiB |
| `fh8652`, `fh8656`, `fh8658` | `TARGET_FH8652/6/8` | FH865x | Cortex-A7 | 64 / 128 / up to 256 MiB |
| `fh8852v100`, `fh8856v100` | `TARGET_FH8852V100/FH8856V100` | FH885x V100 | ARM1176 | 64 / 128 MiB |
| `fh8626v100` | `TARGET_FH8626V100` | FH8626V100 | ARM1176 | 64 MiB |
| `fh8833v100` | `TARGET_FH8632` | FH8632 (kernel name FH8833) | ARM1176 | 64 MiB |

The names are OpenIPC/firmware's where it has the SoC (`fh8833v100`,
`fh885{2,6}v100`, `fh885{2,6,8}v2{0,1}0`). The V210 parts are the V200
die with another package ID; their vendor U-Boot is the V200 one.

`dram_init()` sizes the DDR by probing up to `CONFIG_FH_DDR_MAX`.

## Relationship to mainline

```sh
git diff --stat v2022.10 HEAD                  # the whole delta, small
git diff --diff-filter=M v2022.10 HEAD         # the mainline files we patched
```

**Added**

| Area | Location |
| --- | --- |
| SoC: clocks, timer, watchdog, reset, DRAM size, GPIO, PHY-mode tag | `arch/arm/mach-fullhan/` (`ARCH_FULLHAN`; generation symbols `FH885XV200`, `FH865X`, `FH885XV100`, `FH8626V100`, `FH8632`; register map in `include/mach/fullhan.h`) |
| Board: pads, Ethernet PHY set-up, SD0, `misc_init_r` → `openipc_helper()` | `board/fullhan/fullhan/` |
| Memory map, console, timer | `include/configs/fullhan.h`, ending in `#include "openipc-common.h"` |
| Flash controller (SPI0, a DesignWare SSI with a GPIO chip select), non-DM | `drivers/spi/fh_spi.c` (`FH_SPI`), under the legacy `spi_xfer()` API: the mainline spi-nor layer drives the flash |
| OpenIPC boot layer | `common/openipc.c`, `include/openipc.h`, `include/configs/openipc-common.h` (Fullhan values, same env commands) |
| Boot header data, image tool, the build | `openipc/`, `build.sh` |

**Patched mainline files**, each small:

- `arch/arm/{Kconfig,Makefile}`: the `ARCH_FULLHAN` hook.
- `common/{Kconfig,Makefile}`: `OPENIPC`, `PRODUCT_SOC`, `PRODUCT_SOCMODEL`.
- `drivers/spi/{Kconfig,Makefile}`: `FH_SPI`.
- `drivers/mtd/spi/{Kconfig,spi-nor-ids.c}`: `SPI_FLASH_BOYA`, the Boya
  BY25Qxx parts. Fullhan cameras ship BY25Q64AS, which mainline does not
  know (the vendor U-Boots take any JEDEC ID).
- `drivers/net/Kconfig`: `NETDEVICES` selects `DM_ETH` only when `DM` is on.
- `drivers/net/designware.c`, `include/netdev.h`: no network rather than a
  crash without a PHY (as on `openipc-gk710x`), and a weak
  `designware_board_adjust_link()` called with the negotiated link: the
  Fullhan MAC's 10/100 clock is PMU `SYS_CTRL` bit 24, outside the MAC.

The console (NS16550, 32-bit registers), the timer (`lib/time.c` over timer
0's down-counter at 1 MHz), Ethernet (DesignWare, normal descriptors) and
SD (DesignWare MMC with its IDMAC, `add_dwmci()`) are mainline drivers.

## Boot

The mask ROM reads the header at flash 0: `2BL*OABD`, then 64-byte records
from 0x100, DDR-script records first, then images. Each record has a name,
four words (+0x10 package ID, +0x14 compression, +0x18 unpadded size),
size, flash offset, load, entry, type and `~crc32` of the data.

1. The ROM runs the DDR script (16-byte `{addr, value, arg, op}` entries
   for the PMU PLLs and the DDR controller) whose package ID matches the
   chip's.
2. It loads and starts the type-0x10 image:
   - **U-Boot itself**, at `0xA0800000` (`SYS_TEXT_BASE`), on FH885xV200,
     FH8626V100 and FH8632. FH8632's SDK image has an SPL there, which
     loaded U-Boot from 0x50000 -- past the OpenIPC environment -- so this
     tree has the ROM load U-Boot as FH885x V100/FH8626V100 cameras do;
   - **a vendor ramboot** in SRAM, on FH865x and FH885x V100. It loads the
     **type-0x11** U-Boot from wherever the header says, with its own SPI
     driver. Run under Unicorn on these images, both reach U-Boot with the
     right bytes at `0xA0800000`:
     - the FH865x one is needed: the ROM has run the package's type-0x43
       script, and the ramboot writes the DDR size (PMU 0x334), copies the
       eFuse trims (the internal PHY's to PMU 0x340), and for external DDR3
       (FH8658) probes the size and runs the matching 0x1x53 script. It
       checks U-Boot's CRC and starts it with r1 = 9999, r2 = 0xA0000100;
     - the V100 one runs the type-0x43 script of the package itself (an
       "adaptive DDR" header, one image for the FH8852 and FH8856), sets
       the flash's quad-enable bit on Winbond and GigaDevice parts, and
       does not check U-Boot's CRC. The FH8852 cameras seen boot without
       it, from a type-0x23 script the ROM runs; it is kept so that one
       image serves both packages;
3. DDR is up before U-Boot runs: no SPL, an empty `lowlevel_init`.

`openipc/<generation>.boot` holds, as text, the header of a camera's flash
or SDK image (`fullhan-image.py extract` wrote them; the comment at the top
of each says which): the root header, every DDR script entry by entry, the
image records' fields. `openipc/fh865x.ramboot` (from an FH8652 camera) and
`openipc/fh885xv100.ramboot` (the SDK's `RamBoot_FH8856_FH8852.bin`) are
the vendor ramboots, byte for byte.

`fullhan-image.py` lays the boot partition out again: header at 0, then
each script, the ramboot and U-Boot at 4 KiB boundaries from 0x1000, with
sizes and CRCs recomputed. Every DDR script and ramboot record rebuilds to
its vendor CRC. The image must end before the environment at 0x40000; the
tool refuses one that does not, and says how much room is left:

| Generation | U-Boot at | Room for u-boot.bin |
| --- | --- | --- |
| FH885xV200 (7 DDR scripts) | 0xA000 | 216 KiB |
| FH865x (6 scripts, ramboot) | 0x13000 | 180 KiB (Thumb-2) |
| FH885x V100 (2 scripts, ramboot) | 0x6000 | 232 KiB |
| FH8626V100, FH8632 (1 script) | 0x2000 | 248 KiB |

**V200 is the tight one** (about 8 KiB to spare on ARM1176 with LTO).
Check the build's "bytes to spare" after enabling anything.

The images are in CPU order: they go to flash as they are, with a
programmer, `sf write` or SD/TFTP recovery (`ubwrite`).

## Peripherals

- **Console:** UART0 at `0xF0700000`, DesignWare 16550 with 32-bit
  registers; 16.67 MHz (30 MHz on FH8632) from the PMU divider
  `arch_cpu_init()` sets as the vendor `clock_init()` does.
- **Timer:** channel 0 of the DW APB timers at `0xF0C00000`, free running,
  counting down at 1 MHz. **Reset:** PMU `SWRST_MAIN` bit 31 (`0xF0000040`,
  `0xF000004C` on FH8626V100).
- **Watchdog:** off at boot. If something started it, it is given its
  longest period, as the vendor U-Boots do; nothing feeds it.
- **Flash:** SPI0 (`0xF0500000`; `0xF0E00000` on FH8626V100), mode 3, one
  cycle of RX sample delay, full-duplex PIO in FIFO-sized chunks. The chip
  select is a GPIO -- 6 on V200/FH865x, 54 before -- with SER set too (the
  SSI does not clock without it); the pad is muxed to GPIO. The SPIC
  registers are set as the vendor's `spic_wire_init()` leaves them.
- **Ethernet:** the GMAC at `0xE0600000` is a DesignWare MAC with normal
  descriptors, DMA at +0x1000, on RMII. `fh_phy_setup()` does what the
  vendor drivers do before their PHY code, over raw MDIO:
  - V200 / FH865x: an RMII PHY on the MAC pads, or the SoC's own 10/100
    PHY (`0x00441400`). As the vendor's `auto_find_phy()` does, the
    external one is tried first: pads 20-29 in their RMII functions (REF_CLK
    drives the 50 MHz out), the internal PHY off, the PHY reset (GPIO 29,
    pad 64) pulsed with RXDV (pad 27) strapped low through GPIO 54. If
    nothing answers on MDIO, the pads go back as they were and the internal
    PHY is selected, clocked, reset, given its eFuse trim (PMU 0x340) and
    the vendor's register writes. Any PHY that answers is taken -- the
    vendor U-Boots take only IP101G, RTL8201 and the internal one, so an
    FH8852V200 camera with a JL1101 has no network in its own U-Boot. QEMU
    has an IP101G, so only the external path runs there;
  - V100 / FH8626V100 / FH8632: an IP101G or RTL8201 on the pads. RXDV is a
    strap held low through GPIO 41 while the PHY's reset GPIO (11; 2 on
    FH8632) is pulsed; then `SYS_CTRL |= 0x07000000` and the MAC reset; the
    PHY's RMII set-up registers. FH8632 feeds the PHY its 50 MHz from
    AC_MCLK (pad 59).
  The pads are written field by field, as each vendor pinctrl does: the
  V100 / FH8626V100 one never sets output enable, so it is kept.
- **SD:** SD0 (`0xE2000000`) for recovery, 50 MHz source, the vendor's
  clock phase; power is GPIO 5, low on, before V200. FH8632's package has
  only D0/D1 bonded: 1-bit.

## Hand-off to the kernel

All the vendor kernels are ATAG board-file kernels for machine **9999**
(`CONFIG_MACH_TYPE`), loaded at `0xA0008000`, ATAGs at `0xA0000100`.

- FH885x V100, FH8626V100, FH8632 (3.0.8) also get the vendor's
  `ATAG_PHYMODE` (`0x41000601`, 4 = RMII; `setup_board_tags()` in
  `soc.c`). Without it the V100 kernels' fh_gmac runs MII.
- FH8632's kernel takes its clocksource from the PTS counter in the PAE
  block and never enables that clock: `arch_cpu_init()` leaves every
  FH8632 clock ungated, as the vendor's PHY reset did.
- The kernels redo their own pins and clocks.
- `mem=`: 32M on V200 / FH865x and 29M before, where OpenIPC's
  `load_fullhan` and the SDKs put the media pool.
- MTD id: `spi0.0` (4.9 spi-nor) on V200 / FH865x, `spi_flash` (3.0.8
  m25p80) before. Console `ttyS0`.

## Build

```sh
bash build.sh                 # every SoC; continues past failures, exits non-zero if any failed
bash build.sh fh8852v200      # one or more SoCs (CROSS_COMPILE defaults to arm-linux-gnueabi-)
```

- The output is `output/u-boot-<soc>-nor.bin`, built out of tree in
  `output/build-<soc>-nor/`.
- `.config` is `openipc/openipc_config`, then `openipc/<soc>_config`, then
  `olddefconfig`. There are no `configs/*_defconfig` for these targets.
- FH865x builds Thumb-2 (`SYS_THUMB_BUILD`); the ARM1176 parts cannot.

## Verification

The emulator is widgetii/qemu-hisilicon (qemu-bench), machines
`fh8852v200`, `fh8856v200`, `fh8858v200`, `fh8652`, `fh8656`, `fh8658`,
`fh8852v100`, `fh8856v100`, `fh8626v100`, `fh8632` (`docs/fullhan.md`
there). `-M <machine>,flash-file=<image>` does the boot ROM's job from the
header; where a ramboot is the type-0x10 image it loads the type-0x11
U-Boot itself (QEMU's DDR needs no training), so **the ramboots do not run
under QEMU**.

- **CI smoke:** two boots of an 8 MiB image holding only U-Boot, on every
  machine. The second must load the environment the first saved.
- **Checked by hand under QEMU:**
  - OpenIPC `fh8852v200` and `fh8852v100` lite (uImage at 0x50000, rootfs
    at 0x250000) reach `login:` with a DHCP lease;
  - the FH8626V100 and FH8652 cameras' own kernels and rootfs, and the
    FH8632 SDK's uImage with its initrd, boot to their shells;
  - TFTP, `sf erase/write/read` + `cmp.b` at an unaligned offset and
    length, `fatload` from an SD card (1-bit on FH8632);
  - TFTP recovery from a flash holding only U-Boot, to `login:`;
  - `fw_printenv` / `fw_setenv` from Linux (`/dev/mtd1 0x0 0x10000
    0x10000`);
  - the boot limit: two `reset`s, then `altbootcmd` boots with `failsafe`.
- **Not checked by QEMU, and not on silicon yet:**
  - the ramboots (FH865x, V100), and the FH8632 ROM loading U-Boot
    directly (its SDK always went through the SPL);
  - DDR scripts (vendor data, low risk), PLLs and dividers, pad values;
  - the internal PHY's training and the external PHYs' set-up;
  - SD clock phase and timing.

## Things to keep

- `mem=` stays the first kernel argument, as on the other OpenIPC targets.
- The boot counter is the DRAM word at `0xA1F20000`, DDR + 0x1f20000 as on
  the other families, inside the kernel's 32M on V200 / FH865x. With
  `mem=29M` (V100, FH8626V100, FH8632) the media pool starts at
  `0xA1D00000`, over that word, so there it is `0xA1C20000`. The kernels
  have no device tree to name it, so OpenIPC's `bootcount-clear` does not
  clear it: after two boots the boot limit boots with `failsafe`.
- The environment is at 0x40000, 64 KiB, as in the vendor layout and
  OpenIPC/firmware's `fw_env.config`.

## Conventions

- One topic per commit, `<area>: <what changed>` (`fullhan:`, `fh_spi:`,
  `openipc:`, `net:`, `ci:`).
- Commit bodies say what was verified, and where: the QEMU machine, or a
  board.
