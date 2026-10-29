# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this branch is

`openipc-ingenic` is OpenIPC's U-Boot for Ingenic's XBurst camera SoCs: the
T10, T20, T21, T23, T30 and T31 and their package variants, and the XBurst2
T40. The tree is
**mainline U-Boot v2022.10**, the same release as the other `openipc-*`
branches, plus:

- the OpenIPC boot layer, as `openipc-gk710x` carries it (`common/openipc.c`
  unchanged, the split NOR scan off);
- the T-series support, ported from the vendor U-Boot 2013.07 that
  [OpenIPC/u-boot-ingenic](https://github.com/OpenIPC/u-boot-ingenic)
  carries (Ingenic's ISVP SDK with OpenIPC's changes on top). T23X and
  T23ZN are added from the vendor T23 code and the Ingenic T23 SDK 1.1.0;
- the T40 from [OpenIPC/u-boot-t40](https://github.com/OpenIPC/u-boot-t40)
  (the vendor XBurst2 U-Boot 2013.07, with OpenIPC's "Sync" commit).

The whole boot chain is built here from source, the vendor SPL included;
there is no binary blob (u-boot-ingenic shipped one for the T31LC: it is
the standard T31 SPL with the T31LC clocks, which this tree builds).

CI and user docs live on the `openipc` branch:

- `.github/workflows/ci.yml` only calls `build-ingenic.yml@openipc`.
- `git show origin/openipc:CLAUDE.md` covers the workflow invariants.
- `git show origin/openipc-hisilicon:CLAUDE.md` covers the OpenIPC layer.

Adding or dropping a target needs both branches: here `build.sh`
(`ALL_SOCS`) and `openipc/<soc>_config`; there `build-ingenic.yml`'s matrix
and the README.

## Targets

| Target | Kconfig | CPU / DDR clock | DDR |
| --- | --- | --- | --- |
| t10, t10l | T10 (+LITE_VERSION) | 860 / 400, 712 / 400 MHz | DDR2 64 MiB, DWC PHY |
| t20, t20l, t20x | T20 (+LITE_VERSION, +DDR2_128M) | 860 or 712 / 500 MHz | DDR2 64/128 MiB, DWC PHY |
| t21n | T21 | 864 / 450 MHz | DDR2 64 MiB, internal Ethernet PHY |
| t23n, t23zn, t23x | T23 + T23N / T23ZN / T23X | 1188 / 600 MHz | DDR2 64/128 MiB |
| t30n, t30l, t30x, t30a, t30a1 | T30 (+LITE, +DDR2_128M, +T30A, +DDR3_128M) | 900 / 500, 750 / 500, T30A 1200 / 600 MHz | DDR2/DDR3 64/128 MiB |
| t31n, t31l, t31x, t31a, t31al, t31lc | T31 (+LITE, +DDR2_128M, +DDR3_128M+T31A, +T31AL, +T31LC) | 1404 / 500, 1008 / 500, 1392 / 600, T31A 1392 / 750, T31LC 1104 / 500 MHz | DDR2/DDR3 64/128 MiB |
| t40n, t40xp, t40a | T40 + T40N / T40XP / T40A (XBurst2) | 912 / 550, 1008 / 700, 1008 / 700 MHz | DDR2 128 MiB, DDR3 256 MiB, DDR3 1 GiB |

The names are u-boot-ingenic's (`u-boot-<target>-nor.bin` in
OpenIPC/firmware `latest`), and each target selects what its boards.cfg
line selected there. u-boot-t40 published its T40 images as
`u-boot-<target>-universal.bin`; here they are `-nor.bin` like the rest. The clocks and the DDR chip follow from the Kconfig
options in `arch/mips/mach-ingenic/include/mach/<soc>-hw.h`, the vendor
header's hardware section.

## Relationship to mainline

```sh
git diff --stat v2022.10 HEAD
git diff --diff-filter=M v2022.10 HEAD         # the mainline files we patched
```

**Added**

| Area | Location |
| --- | --- |
| SoC: SPL, PLL, clocks, DDR, timer, reset, caches, GPIO, UART | `arch/mips/mach-ingenic/` (`ARCH_INGENIC`; `<soc>/` per SoC; `xburst2/` the T40's CPU, G-OST timer, GPIO, SFC and DDR code), headers `arch/mips/include/asm/arch-<soc>/` (vendor), `include/mach/` |
| DDR register values, computed on the build host | `arch/mips/mach-ingenic/ddr_params_creator.c`, `ddr2_params.c`, `ddr3_params.c` (vendor `tools/ingenic-tools`; XBurst2's in `xburst2/`), chip timings `xburst{,2}/include/ddr/` (the vendor changed them between the generations) |
| Board: pins, clock parents, quirks of OpenIPC cameras | `board/ingenic/isvp/isvp.c` |
| SPI NOR on the SFC, non-DM | `drivers/mtd/spi/jz_sfc_nor.c` (`JZ_SFC_NOR`), providing `spi_flash_probe()` |
| SD card on MSC0, non-DM | `drivers/mmc/jz_mmc_legacy.c` (`JZ_MMC`); the T40's SDHCI: `drivers/mmc/jz_sdhci.c` (`JZ_SDHCI`, on mainline's SDHCI core) |
| Ethernet (Synopsys GMAC), non-DM | `drivers/net/jz_gmac.c`, `synop_gmac.c` (`JZ_GMAC`) |
| Memory map | `include/configs/ingenic-isvp.h`, ending in `#include "openipc-common.h"` |
| OpenIPC boot layer | `common/openipc.c`, `include/openipc.h`, `include/configs/openipc-common.h` (Ingenic parameters) |
| Kconfig fragments, the image packer, the build | `openipc/`, `build.sh` |

**Patched mainline files**: `arch/mips/{Kconfig,Makefile}` (the hook, cache
sizes), `arch/mips/include/asm/global_data.h` (`gd->arch.gi`, the SPL's
clock record), `arch/mips/include/asm/gpio.h` (the vendor GPIO API),
`common/Makefile` (the layer stays out of the SPL), the `JZ_*` driver
hooks in `drivers/*/{Kconfig,Makefile}`, `cmd/Kconfig` (`CMD_SF`),
`env/Kconfig` (`ENV_IS_IN_SPI_FLASH`).

The vendor's hardware constants keep their vendor names (`CONFIG_SYS_APLL_FREQ`,
`CONFIG_DDR_CS0`, ...) in the mach headers, outside `include/config.h`, so
the ported code reads as the vendor's; what selects them is Kconfig.

## Boot

1. The boot ROM reads the SFC boot header at flash offset 0 (three words,
   then 2 KiB the ROM keeps; it checks a CRC7 of the SPL from 2 KiB on at
   byte 9 and the SPL length at byte 12), copies the SPL into the cache,
   locked as RAM at 0x80001000, and runs it on its own stack.
2. The SPL (`arch/mips/mach-ingenic/start.S`, `spl.c`) runs the vendor
   sequence: pin functions, UART1 clock and console, OST timer, clocks
   stopped, PLLs, clock gates and parents, DDR (`ddr_innophy.c` on the
   T21/T23/T30/T31, `ddr_dwc.c` on the T10/T20, the DQS training where the
   vendor builds it). Its limit is the vendor's 26 KiB slot.
3. It reads the uImage at 0x6800, lzop-packed U-Boot, into DDR at
   0x80200000, unpacks it to 0x80100000 and jumps there. U-Boot then
   relocates itself to the top of the DDR.

`openipc/ingenic-image.py` writes the CRC7 and the length, pads the SPL to
0x6800 and appends the uImage; the image ends before the environment at
0x40000. One file, written at offset 0 by a programmer or by `run ub`.

### T40 (XBurst2)

The same image layout and SPL flow, with what u-boot-t40's SPL adds:

- `start.S`: Cause.IV set by read-modify-write; no BTB or $12.2 writes.
- First thing in `board_init_f()`: TCU timer 3 started (the vendor's
  boot-time stamps); then the CCU IFU loop closed (0xb2200fe0 |= 0x18).
- DDR: `xburst2/ddr_innophy.c` with the hardware training the vendor
  header selects (no DQS software training).
- Before the jump: L1 and L2 written back (`flush_cache_all()`), then the
  L2 size set (0xb2200060 = 0x400, 128 KiB) between sync + uncached loads.
- The SPL is padded with 0xff to 64 bytes (`spl_pad_to_block`) and, on
  t40n/t40xp (SPL_PARAMS_FIXER in their boards.cfg lines, not t40a), gets
  the boot ROM's parameter block at 256 (`spl_params_fixer`: "INGE", the
  length rounded to 512, CPM descriptors for APLL/MPLL/CPCCR/SFC); both in
  `openipc/ingenic-image.py`, equal to the vendor's but for the length.
- Caches: `xburst2/cpu.c`, the vendor's `asm/jz_cache.h` loops; range
  operations on L1 only, as the vendor did.
- U-Boot keeps to the first 256 MiB (KSEG0), as the vendor's
  CONFIG_SYS_SDRAM_MAX_TOP did: the T40A has 1 GiB.
- The vendor's clock id `SFC` is `SFC_CLK` here: `SFC` is
  openipc-common.h's MTD id, which common/openipc.c uses.

## Hand-off to the kernel

Mainline MIPS `bootm` (argc/argv/envp in a0-a2), as the vendor U-Boot did.
The kernel's command line from the environment: `mem=${osmem}
rmem=${rmem} console=ttyS1,115200 ... mtdparts=jz_sfc:...`, the MTD name
of the vendor kernels. `osmem`/`rmem` default to 40M and 24M@0x2800000
(64 MiB packages) or 64M and 64M@0x4000000 (128 MiB), as u-boot-ingenic;
on the T40 as u-boot-t40 set them: T40N 48M, 64M@0x3000000 and
`nmem=16M@0x7000000`, T40XP 100M, 128M@0x6400000 and `nmem=28M@0xE400000`,
T40A the 64 MiB defaults (u-boot-t40 left it there). `nmem` is in the
environment only, not in bootargs, as there.

## Board quirks kept from u-boot-ingenic

- T31 (not T31A/T31AL): PA25/PA26, the flash's IO2/IO3, inputs without
  pulls in `board_early_init_f()`.
- T23: the vendor SPL's GPIO A/B drive strengths (`start.S`), UART1 pull-ups,
  PB06 pull-down, MSC0 pins driven low (`board_early_init_f()`).
- T23/T31: PB24 (UART1 RX) pull-up; T31: GPIO62 low (HiChip SD power).
- No Ethernet PHY found (`jz_net_initialize()` < 0): T31 GPIO61 (PPS) and
  GPIO48 (Wyze V3) low, T20 the Wyze V2 USB/SD power and card-detect
  pulse; `extras=nogmac` for the kernel unless set. The SD recovery
  therefore runs from `last_stage_init()`, after the network.
- T40: GPIO port A PA00-PA22 at 1.8 V (`board_early_init_f()`); the
  GMAC as u-boot-t40 drives it (L2 written back around DMA buffers, MDC
  divider /2, CPM PHY control set in `jz_init()` with bit 23 cleared for
  an external PHY, a shorter IP101G reset on GPIO PC7), keeping OpenIPC's
  PHY id return. No nogmac quirk: u-boot-t40 had none.
- The first boot with a blank environment saves the default one
  (`openipc_helper()`), as u-boot-ingenic's env_common.c did.

What u-boot-ingenic did with its own commands (`sdupdate` with
`autoupdate-*.img`, `boot.scr` on every boot, rootfs size detection in
`sf probe`) is the OpenIPC layer's SD recovery and firmware scan here.

## Build

```sh
bash build.sh            # every target
bash build.sh t31n       # one
```

Toolchain `mipsel-linux-gnu-` (Debian/Ubuntu `gcc-mipsel-linux-gnu`), plus
`lzop` and `python3`. Images land in `output/` as `u-boot-<target>-nor.bin`.

## How it was verified

There is no qemu-hisilicon machine for these SoCs, so CI builds only.
Against the vendor tree (u-boot-ingenic, T20/T21 from 90873ad, before its
T23 commit broke them; u-boot-t40 for the T40) built for every target:

- the generated `ddr_reg_values.h` is byte-identical;
- 76 hardware macros the ported code uses evaluate to the same values
  (preprocessor dump of the vendor `include/config.h` against Kconfig +
  `mach/ingenic.h`); this caught the GPIO port stride, 0x100 on the
  T10/T20 and 0x1000 on the others;
- the SPL header CRC7/length match the vendor `spi_checksum`; on the T40
  94 macros (L1/L2 cache, PLLs, CPCCR, DDR, GMAC, SFC; PHY_TYPE_IP101G
  undefined on both sides, as in isvp_t40.h) and the first 512 bytes of
  the SPL (header, params block) match, but for the length fields;
- adding the T40 left the XBurst objects byte-identical (code and data,
  debug info stripped; only the SPL's build time differs).

None of it has run on a camera yet.
