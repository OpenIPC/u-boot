# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this branch is

`openipc-hisilicon` is OpenIPC's U-Boot for 38 HiSilicon and Goke IP-camera
SoCs. It is **mainline U-Boot v2022.10** with HiSilicon/XMedia vendor SDK code
grafted on, plus OpenIPC's own boot layer. CI and user-facing docs live on the
`openipc` branch:

- `.github/workflows/ci.yml` here only calls `build-hisilicon.yml@openipc`.
- The `openipc` branch has its own `CLAUDE.md` on the workflow invariants. Read
  it with `git show origin/openipc:CLAUDE.md`.
- Adding, removing or holding a SoC needs a change on **both** branches.
  - Here: `build.sh` (`build_soc`, `ALL_SOCS`) and `openipc/<soc>_*`.
  - There: both workflow matrices and the README.

## Relationship to mainline

```sh
git diff --stat v2022.10 HEAD                     # whole delta (~2500 files, ~530k lines)
git diff --diff-filter=M --stat v2022.10 HEAD     # the ~60 mainline files we patched
git log --oneline v2022.10..HEAD                  # our history
```

History on top of `v2022.10`:

- **`1ba97e9c9e4` "OpenIPC HiSilicon baseline"**: one squashed commit that
  imports all the vendor code.
- After it come ~40 OpenIPC commits:
  - SoC family ports (cv200, cv300, 18ev100, 16av100, 3536, 3520d, 3519v101,
    xmorca);
  - the recovery and env work;
  - flash IDs ported from u-boot-xmedia;
  - fixes.
- `origin/main` (equal to `master`) tracks mainline. Its merge-base with this
  branch is exactly `v2022.10`, so a mainline rebase means moving the
  baseline commit forward.

### Where the vendor code lives

Most of the delta is **added files**: whole vendor subsystems in new
directories. Mainline U-Boot has no counterpart for them.

| Area | Location |
| --- | --- |
| Per-SoC first stage: `start.S`, lowlevel init, DDR/PLL init, `hw_compressed/` (the decompressing loader that becomes `u-boot-z.bin`) | `arch/arm/cpu/armv7/{hi35xx,xm7xxx,xmorca}/`, and for the ARM926 parts `arch/arm/cpu/arm926ejs/{hi3516cv300,hi3518ev100,hi3518ev200}/` |
| Machine glue | `arch/arm/mach-hi35xx`, `arch/arm/mach-xmedia`, `arch/arm/mach-vendor` |
| Targets | `arch/arm/Kconfig`: `TARGET_HI*`/`TARGET_XM*` and the `*_FAMILY` symbols, `ARCH_BSP` |
| Boards | `board/hisilicon/<board>/`, `board/xmedia/<board>/`. Goke parts are XMedia dies, e.g. `xm72050200` = gk7205v200; `xmorca` = GK7206V1 |
| Per-SoC headers | `include/configs/<board>.h` |
| DDR training | `drivers/ddr/{hisilicon,xmedia,vendor}` (`vendor/` is the newer-SDK one, used by `ARCH_BSP`) |
| SPI flash (FMC, plus HISFC350 on old parts) | `drivers/mtd/fmc_*.c`, `drivers/mtd/spi/{fmc100,hisfc350}`, `drivers/mtd/spi/vendor_spi_nor.c`, `include/fmc_common.h`, `fmc_spi_ids.h` |
| NAND: SPI-NAND through FMC, and the HINFC301 parallel controller on V1 parts | `drivers/mtd/nand/raw/{fmc100,hinfc301}`, `nfc_common.c` |
| Ethernet MACs | `drivers/net/{hisfv300,sfv300,femac,higmacv300,gmacv300,ftgmac030}` |
| MMC/SDHCI | `drivers/mmc/{himci*,bsp_*,xm7*}.c` |
| USB | `drivers/phy/{hibvt,xmedia}`, `drivers/usb/gadget/udc3` |
| HW gunzip | `lib/hw_dec`, plus `cmd/hwzimage.c` and `cmd/cmd_ugzip.c` |
| ATAGs and I2C helpers | `drivers/lhs` |
| GPIO and pad-mux (`gpio status` shows the live pad function) | `drivers/gpio/hi_*` |
| Newer-SDK (`ARCH_BSP`: cv610/cv608, hi3519dv500) extras: OSD, audio, OTP, verified boot, security subsystem | `product/`, pulled in by `Makefile-otproduct`; `securec/` (Huawei bounds-checked libc) |
| Signed boot-image packers (GSL + DDR table + U-Boot), one per SDK generation | `image_tool/` (cv610/cv608, prebuilt `input/gsl.bin`), `image_tool_hi3519dv500/` |
| GSL first-stage sources. `gsl/` (cv6xx) is **not built**: those images use the prebuilt `image_tool/input/gsl.bin`. `gsl_hi3519dv500/` is built into `image_tool_hi3519dv500/input/gsl.bin` | `gsl/`, `gsl_hi3519dv500/` |
| Vendored host tools | `extras/gzip-1.11` (built by `build.sh`) |
| OpenIPC's own per-SoC inputs | `openipc/`: config fragments and reginfo; `reginfo/` holds the cv6xx and hi3519dv500 DDR tables |

### Mainline files we patched

The patched files are listed by `git diff --diff-filter=M v2022.10`. These
changes conflict on a rebase and change mainline behaviour:

- **Build system**:
  - The top-level `Makefile` adds the `u-boot-z.bin` / `u-boot-z.clean`
    targets. They run the per-SoC `hw_compressed/` Makefile on `u-boot.bin`
    and the `.reg` table. `u-boot-z.bin` is the flashable image; `u-boot.bin`
    alone does not boot.
  - It also adds `ddr_training_prepare`/`_clean`, chained into `prepare` and
    `clean`. These build the DDR training `cmd_bin` per family.
  - It adds `securec/` and `drivers/ddr/...` to `libs-y`.
  - `config.mk` exports `VENDER_CHIP`.
  - The various `Kconfig`/`Makefile` files only hook in the new directories.
- **Reverted-to-vendor copies.** The SDK carried older U-Boot copies of some
  mainline files, and the baseline commit took those copies over mainline's.
  The diff of these files shows mainline features *removed*. That is
  intentional and not a regression to "fix" piecemeal.
  - `drivers/mtd/spi/spi-nor-core.c`, `spi-nor-ids.c` and
    `include/linux/mtd/spi-nor.h`: SCCR/octal-DTR is gone and
    `stm_is_unlocked` became `stm_is_locked`.
  - `cmd/sf.c`: the `flash_is_unlocked` write/erase guard is dropped.
  - `drivers/usb/host/xhci*.c` and `include/usb/xhci.h`: the vendor xHCI,
    which includes the hibvt/xmedia PHY headers.
- **Small hooks**:
  - `serial_pl01x.c` adds `serial_enable_output()`, used for `silent=1`.
  - `nand_base.c`, `mtd.h`, `rawnand.h` and `spi_flash.h` gain the vendor
    fields and IDs.
  - `cmd/gpio.c` gets the legacy `gpio status`.
  - `cmd/net.c`, `mmc_boot.c`, `vsprintf.c`, `include/common.h`,
    `arch/arm/lib/reset.c` and `mach-types.h` carry small vendor hooks.
  - `env/Kconfig` lets `ENV_IS_IN_SPI_FLASH` depend on `FMC ||
    HISFC350_SPI_NOR`, not on DM SPI.
  - `scripts/check-config.sh` no longer fails on ad-hoc `CONFIG_`s, because
    the vendor headers define many.
- **Deleted**: mainline's CI configs (`.azure-pipelines.yml`,
  `.gitlab-ci.yml`), `.checkpatch.conf`, `.mailmap`, and a few binary test
  fixtures.

### The vendor driver model

Most SoCs bypass mainline's DM SPI/MTD stack:

- **SPI flash** goes through the vendor FMC driver straight into
  `spi_flash`. `fmc_common.c`/`fmc100` serve every FMC SoC. One
  `drivers/mtd/fmc_<soc>.c` per SoC supplies clocks and IO; register-layout
  differences are `#ifdef CONFIG_TARGET_<SOC>` blocks in `fmc_common.h`.
- **The Ethernet MACs** are legacy non-DM drivers.
- **Kernels are booted with ATAGs**, not a DTB (`CONFIG_SUPPORT_PASSING_ATAGS`
  in `openipc-common.h`).

When a new die differs from a supported one, extend the tree's driver with
`#ifdef CONFIG_TARGET_<SOC>`. Do not import the SDK's duplicate copy. The
GK7206 port (`784ae4105ee`) is the worked example for FMC, hw_dec and
timestamp.

## Build

```sh
bash build.sh               # every SoC in ALL_SOCS; continues past failures, exits non-zero if any failed
bash build.sh hi3516cv300   # one or more SoCs (CROSS_COMPILE defaults to arm-linux-gnueabi-)
```

Output:

- Images land in `output/`: `u-boot-<soc>-{nor,nand}.bin`, or
  `boot-<soc>[-<model>]-nor.bin` for the SoCs in `BOOTIMG_TABLE`
  (cv608, cv610, hi3519dv500). hi3519dv500 is arm64 and builds with
  `CROSS_COMPILE64` (default `aarch64-linux-gnu-`).
- Each SoC builds out-of-tree (`O=`) in `output/build-<soc>/`, and its
  `.config` stays there.

Build environment:

- `build.sh` first builds the patched gzip (`WSIZE=0x2000`) from
  `extras/gzip-1.11`. Set `GZIP_REBUILD=1` to force a rebuild.
- CI runs with `/bin/sh` symlinked to bash, because vendor recipes use
  bash-isms.
- CI also needs `pycryptodome`, `u-boot-tools`, `device-tree-compiler`,
  `lzop`, bison/flex and libssl-dev.

There are **no `configs/*_defconfig` for most SoCs**. The ones in `configs/`
are vendor leftovers, apart from the cv608/cv610 `_openipc_` ones. For each
image, `build.sh` builds `.config` by concatenating these, later lines
winning, and then runs `olddefconfig`:

1. `openipc/openipc_config`: the base for every SoC.
2. `openipc/<soc>_config`: the gk7205v5xx SoCs share `xm720xxx_config`, and
   the GK7206 dies share `gk7206v100_config`.
3. `openipc/nand_config`, for NAND images only.
4. `CONFIG_PRODUCT_SOC="<soc>"`.

`openipc/<soc>_reginfo.bin` becomes `$O/.reg`, the DDR/pinmux table spliced
into the first stage. `build.sh` has one function per family (`build_v500`,
`build_nor_ddr`, `build_nor_splice`, `build_xmedia`, `build_xmorca`), because
each SDK generation packages its image differently. The SoCs whose boot ROM
starts a signed GSL (cv610/cv608, hi3519dv500) are rows of `BOOTIMG_TABLE`
instead -- SoC, DDR binning, DDR table, arch, packer, GSL source -- built by
the one `build_bootimg`.
Several SoCs share one board and binary and differ only in fragment and
reginfo. For example, `hi3516cv100`/`hi3518*v100` all build
`u-boot-hi3518ev100.bin`.

**Hard size limits**: a NOR image may be at most 256 KiB, because env starts
at 0x40000; hi3519dv500, whose boot partition is 512 KiB (env at 0x80000,
firmware at 0xc0000, see `OPENIPC_KERNADDR`), at most 512 KiB. A NAND image may be at most 768 KiB. CI rejects an oversized
image, because flashing one overwrites env and bricks the camera.

**Verification** (there are no unit tests):

- **QEMU smoke**: CI boots each NOR image in widgetii/qemu-hisilicon. It
  passes only on `Hit Ctrl-C to stop autoboot`, not on the banner.
- **Hardware**: load the image into RAM with `defib` before writing flash.
- Commit bodies state which check was done, or say "Build-tested only".

## The OpenIPC boot layer

This is the code with no SDK or mainline counterpart, and most changes touch
it.

- `include/configs/openipc-common.h` is the **only** place that defines the
  default env: bootcmd, bootargs, the `ub`/`uk`/`ur`/`fwupd` update commands,
  `altbootcmd`, NOR/NAND mtdparts and `baseaddr`/`osmem` per family. Per-SoC
  headers include it last and may override only `OPENIPC_*` macros. They must
  never define `CONFIG_EXTRA_ENV_SETTINGS`, `CONFIG_BOOTARGS` or
  `CONFIG_BOOTCOMMAND`.
- `common/openipc.c` holds the OpenIPC boot logic, entered from each board's
  `misc_init_r()` via `openipc_helper()`:
  - **NOR**: `firmware_scan()` locates the kernel and squashfs, then sets
    `kernaddr`/`kernsize` and rebuilds `mtdparts`.
  - **NAND**: it attaches UBI and requires a UBIFS `rootfs` volume holding
    `/boot/fitImage` or `/boot/uImage`. There is no kernel volume.
  - **Broken firmware**: the scan sets `bootfail`, and `firmware_recovery()`
    tries SD/USB.
  - **Boot loops**: past `bootlimit` (the DRAM bootcount word at
    SDRAM + 0x1f20000, cleared by openipc-failsafe), `altbootcmd` boots the
    same kernel with `failsafe`. It does not reflash.
- OpenIPC also changed these Kconfig defaults:
  - autoboot stops on Ctrl-C only;
  - `silent=1` suppresses console output;
  - `CMD_ECHO` is enabled;
  - the prompt is `OpenIPC # `.

**Saved environments must keep working.** Cameras upgraded from the old
per-SoC OpenIPC bootloaders (u-boot-gk7205v200, u-boot-hi3516ev200,
u-boot-xmedia and others) keep their env. That env may lack `mtdids` or
`loadkernel`, or carry retired commands, which `altbootcmd_migrate()`
rewrites. So:

- Fall back to the controller name (`sfc`, or `hi_sfc` on HiSilicon).
- Inline commands rather than `run` a variable that may not exist.

Firmware, sysupgrade and openipc.org depend on two layouts. Do not change
either:

- the NAND layout `768k(boot),256k(env),-(ubi)`, with the kernel inside the
  rootfs;
- the NOR offsets in `openipc-common.h`.

## Conventions

- `.gitignore` keeps mainline's `.*` rule, so new files under `.github/`
  need `git add -f`.
- One topic per commit, in the form `<area>: <what changed>` (`nand:`,
  `openipc:`, `hinfc301:`, `fmc100:`, `xmorca:`, `ci:`). The body explains
  the failure on real cameras, the root cause, and what was verified on
  which board.
