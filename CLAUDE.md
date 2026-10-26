# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this branch is

`openipc-grainmedia` is OpenIPC's U-Boot for Grain Media's IP camera SoCs.
The tree is **mainline U-Boot v2022.10**, the same release as
`openipc-hisilicon`, `openipc-xm`, `openipc-gk710x` and `openipc-fullhan`,
plus:

- the OpenIPC boot layer, carried over from `openipc-fullhan`
  (`common/openipc.c` is the same file);
- the Grain Media support, written from the GM8136 SDK's U-Boot 2013.01,
  nsboot and Linux 3.3, and from Longse GM8136S camera dumps;
- the SDK's **nsboot**, the first stage, as source in
  `board/grainmedia/nsboot`, built with the tree.

No vendor U-Boot driver comes along.

CI and user docs live on the `openipc` branch:

- `.github/workflows/ci.yml` only calls `build-grainmedia.yml@openipc`.
- `git show origin/openipc:CLAUDE.md` covers the workflow invariants.
- `git show origin/openipc-hisilicon:CLAUDE.md` covers the OpenIPC layer
  (default environment, `common/openipc.c`, env compatibility). It applies
  here unchanged.

Adding or dropping a SoC needs both branches:

- here: `build.sh` (`build_soc`, `ALL_SOCS`), `openipc/<soc>_config` and
  the table in `README.md`;
- there: `build-grainmedia.yml`'s matrices and the README.

## Targets

One die, a Faraday FA626TE (ARMv5TE; U-Boot builds it as ARM926EJ-S), in
three packages:

| Package | DDR | PMU ID bits 6:2 | Image (`<soc>`) |
| --- | --- | --- | --- |
| GM8136 | external (the SDK's table: 2 Gbit, 256 MiB) | 0x00 | `gm8136` |
| GM8136S | 128 MiB in the package | 0x14, 0x1C | `gm8136` |
| GM8135S | 64 MiB in the package | 0x04, 0x08, 0x10, 0x18 | `gm8135` |

The names are OpenIPC/firmware's, which calls GM8136 and GM8136S cameras
both `gm8136`. **Every image runs on every package**: nsboot and U-Boot
read the package from the PMU. The images differ only in `soc=`. There
are no `TARGET_*` symbols.

## Relationship to mainline

```sh
git diff --stat v2022.10 HEAD                  # the whole delta, small
git diff --diff-filter=M v2022.10 HEAD         # the mainline files we patched
```

**Added**

| Area | Location |
| --- | --- |
| SoC: clocks, timer, watchdog reset, DRAM size | `arch/arm/mach-grainmedia/` (`ARCH_GRAINMEDIA`; register map in `include/mach/grainmedia.h`) |
| Board: GMAC clocks, pins and PHY reset, SD0, `mem=`/`gmmem=`, `misc_init_r` → `openipc_helper()` | `board/grainmedia/grainmedia/` |
| nsboot, the SDK's first stage | `board/grainmedia/nsboot/` (not built by U-Boot's make; `build.sh` builds it) |
| Memory map, console, timer | `include/configs/grainmedia.h`, ending in `#include "openipc-common.h"` |
| Flash controller (FTSPI020), non-DM | `drivers/spi/ftspi020.c` (`FTSPI020`): takes spi-mem operations whole, so the mainline spi-nor layer drives the flash |
| Ethernet (FTGMAC100), non-DM | `drivers/net/ftgmac100_nodm.c` (`FTGMAC100_NODM`): mainline's driver from before its DM conversion |
| OpenIPC boot layer | `common/openipc.c`, `include/openipc.h`, `include/configs/openipc-common.h` (Grain Media values, same env commands) |
| Image layout, the build | `openipc/`, `build.sh` |

**Patched mainline files**, each small:

- `arch/arm/{Kconfig,Makefile}`: the `ARCH_GRAINMEDIA` hook.
- `common/{Kconfig,Makefile}`: `OPENIPC`, `PRODUCT_SOC`, `PRODUCT_SOCMODEL`.
- `drivers/spi/{Kconfig,Makefile}`: `FTSPI020`.
- `drivers/spi/spi-mem-nodm.c`: `spi_mem_exec_op()` is weak, so that
  `ftspi020.c` provides its own. The FTSPI020 runs whole commands (opcode,
  address width, dummy cycles, data count) and cannot take the generic
  version's byte stream, where address and dummy bytes look alike.
- `drivers/net/{Kconfig,Makefile}`, `include/netdev.h`: `FTGMAC100_NODM`.
- `drivers/net/Kconfig`: `NETDEVICES` selects `DM_ETH` only when `DM` is on.

The console (NS16550, 32-bit registers, 25 MHz), the timer (`lib/time.c`
over FTTMR010 timer 0 counting APB clocks down) and SD (SDHCI, non-DM
`add_sdhci()`, PIO) are mainline drivers.

## Boot

```
0x00000  NOR system header: "GM8136", image 0 = U-Boot, 0x55AA at 0xFE
0x01000  nsboot (24 KiB at most)
0x08000  U-Boot (to 0x40000: 224 KiB)
0x40000  environment, 64 KiB
0x50000  kernel (OpenIPC layout)
```

1. The mask ROM checks the header and loads nsboot from the second 4 KiB
   sector into the MCP100's SRAM at 0x92000000, and runs it.
2. nsboot (`entry.c`) remaps DDR to 0, programs the DDR controller from
   its package's table, changes the CPU clock through the PMU's frequency
   change (interrupt 8, WFI), reads the header with the FTSPI020 and loads
   image 0 -- U-Boot -- to DDR 0 and jumps there.
3. U-Boot is linked at 0 (`SYS_TEXT_BASE`) and relocates to the top of
   DRAM as usual.

`openipc/gm-image.py` writes the header (the SDK's `NOR_SYSHDR`, filled
as the Longse cameras' flash has it) and lays the three out; it refuses an
nsboot over 24 KiB and a U-Boot that reaches the environment. The SDK puts
U-Boot at 0x10000; here it is at 0x8000, which the 0x55AA header allows,
because 192 KiB is too little. The image goes to flash as it is, at 0.

### nsboot

`board/grainmedia/nsboot` is the SDK's `gm8136_nsboot`, imported as it
ships (first commit; its `.gitattributes` keeps the CRLF files byte for
byte), then:

- **build fixes** for current compilers: `<stdarg.h>` instead of its own
  `va_list`, `-fno-stack-protector -fno-pie`, `ld --gc-sections`, a
  `raise()` for Linux libgcc's division by zero;
- **one image for every package.** The SDK builds nsboot for one package
  (`MFLAGS`: chip, DDR clock and part, CPU clock mode); the wrong one
  leaves a camera without memory, and OpenIPC names GM8136 and GM8136S
  cameras alike. `package.c`, built once per package with that package's
  flags -- the SDK's "default setting" of each -- holds its DDR table and
  `set_fcs()` arguments; `entry.c` picks one by the PMU ID as the kernel's
  `pmu_get_chip()` reads it:

  | Package | Flags | CPU / DDR |
  | --- | --- | --- |
  | GM8136 | `SYSTEM_EVB DDR_1180 MODE_3 DDR_SZ_8136_2Gbx1` | 885 / 1180 MHz |
  | GM8136S | `DDR_1180 MODE_4 DDR_SZ_8136_1Gbx1` | 885 / 1180 MHz |
  | GM8135S | `DDR_950 MODE_4 DDR_SZ_8135_512Mbx1` | 712 / 950 MHz |

  The CPU clocks are among those the vendor `adda308` audio driver
  accepts. The GM8135S "AR" table (chip 0x8136108) is kept.

The Longse cameras' own nsboot is an older GM8136S build: same table but
DDRC 0x1C (0x2044 / 0x2046 against the SDK's 0x2022 / 0x2023). The SDK's
values are used.

## Peripherals

- **Console:** UART0 at `0x90700000`, a 16550 with 32-bit registers, at
  25 MHz: nsboot sets the divider from PLL2 and the pin mux.
- **Timer:** FTTMR010 timer 0 at `0x90D00000`, counting APB clocks (AHB /
  2, 100 MHz with the SDK's clocks) down over 32 bits. Its interrupt mask
  is left alone (see "Hand-off").
- **Reset:** the FTWDT010 at `0x90E00000`, load 1 and reset, as the SDK
  U-Boot does. The PMU's automatic watchdog is turned off at start
  (`0x7C` bit 4).
- **Flash:** FTSPI020 at `0x92300000`, single wire, PIO: a FIFO (32 words)
  at a time when it reports ready, then completion. The divider and mode
  stay as the boot ROM left them.
- **DRAM size:** the size code nsboot's table leaves in DDRC `0x10` (2 =
  64 MiB ... 5 = 512 MiB), checked for a part that aliases or ends below
  that, at 16 MiB + n, clear of U-Boot at 0.
- **Ethernet:** FTGMAC100 at `0x92500000`, RMII, the SDK's set-up: MAC and
  PHY clocks from PLL2 (50 MHz), pins (`0x5C`), PHY reset on GPIO 50
  (GPIO1 bit 18). The driver finds the PHY on MDIO and sets the MAC's
  speed and duplex from it.
- **SD:** SD0, an FTSDC021 (SDHCI) at `0x92800000`, base clock PLL2 / 3 /
  2, the kernel's pins (`0x58`) and drive strength.

## Hand-off to the kernel

The vendor kernel (3.3, OpenIPC's too) is machine **758** (`GM`), no
device tree, ATAGs at `0x100`, uImage loaded at `0x8000`.

- `mem=` is the **whole DRAM** and `gmmem=` the media pool (frammap) the
  kernel carves out of it; the kernel hangs before its console without
  `mem=`. `misc_init_r()` sets `osmem` and `gmmem` as the SDK U-Boot picks
  them (64 / 34, 128 / 90, 256 / 190, 512 / 432 MiB) unless the
  environment has them; the bootargs carry both.
- MTD id: `nor-flash`. The kernel tries `mtdparts=` first and falls back
  to the header's images.
- The kernel programs its timers itself but never clears FTTMR010's
  interrupt mask: U-Boot must not set it, or the kernel hangs at
  "Calibrating delay loop".

## Build

```sh
bash build.sh                 # every SoC; continues past failures, exits non-zero if any failed
bash build.sh gm8136          # one or more SoCs (CROSS_COMPILE defaults to arm-linux-gnueabi-)
```

- The output is `output/u-boot-<soc>-nor.bin`; U-Boot builds out of tree
  in `output/build-<soc>-nor/`, nsboot in `output/build-nsboot/` (a copy
  of its sources; `NSBOOT_CROSS_COMPILE` overrides its compiler).
- `.config` is `openipc/openipc_config`, then `openipc/<soc>_config`, then
  `olddefconfig`. There are no `configs/*_defconfig` for these targets.
- The build prints how much room is left before the environment (about
  17 KiB, with LTO). Check it after enabling anything.

## Verification

The emulator is widgetii/qemu-hisilicon (qemu-bench), machines `gm8135s`,
`gm8136`, `gm8136s` (`docs/grainmedia.md` there). `-M <machine>,flash-file=
<image>` does the boot ROM's job: it loads nsboot from the image into SRAM
and runs it, so **nsboot runs under QEMU** (its DDR table goes to a model
that only reports it).

- **CI smoke:** two boots of an 8 MiB image holding only the image, on
  gm8136 and gm8136s (`gm8136`) and gm8135s (`gm8135`). The second must
  load the environment the first saved.
- **Checked by hand under QEMU, on all three machines:**
  - OpenIPC `gm8136` / `gm8135` lite (uImage at 0x50000, rootfs at
    0x250000) reach `login:` with a DHCP lease;
  - `sf erase/write/read` + `cmp.b` at an unaligned offset and length,
    ping, TFTP, `fatload` from an SD card, `saveenv` across a reset;
  - TFTP recovery from a flash holding only U-Boot, to `login:`;
  - `run ub` from TFTP, then a boot of the written image;
  - `fw_printenv` / `fw_setenv` from Linux (`/dev/mtd1 0x0 0x10000
    0x10000`);
  - the boot limit: two `reset`s, then `altbootcmd` boots with `failsafe`.
- **Not checked by QEMU, and not on silicon yet:** DDR training (the
  tables are the SDK's), the clocks after the frequency change, the PHY's
  clocks and reset, SD timing, the FTSPI020's FIFO handshake on a real
  controller.

## Things to keep

- `mem=` stays the first kernel argument, as on the other OpenIPC targets.
- The boot counter is the DRAM word at `0x01F20000`, DDR + 0x1f20000 as on
  the other families. The kernel has no device tree to name it, so
  OpenIPC's `bootcount-clear` does not clear it: after two boots the boot
  limit boots with `failsafe`.
- The environment is at 0x40000, 64 KiB, as OpenIPC/firmware's
  `fw_env.config` has it.
- nsboot's first commit stays the SDK's sources as they ship: changes go
  in their own commits on top.

## Conventions

- One topic per commit, `<area>: <what changed>` (`grainmedia:`,
  `grainmedia: nsboot:`, `spi:`, `net:`, `openipc:`, `ci:`).
- Commit bodies say what was verified, and where: the QEMU machine, or a
  board.
