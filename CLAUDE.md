# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this branch is

`openipc` is the default branch of OpenIPC/u-boot, and it contains **no bootloader
sources**. It holds only `README.md` (user-facing docs for every SoC family) and
`.github/workflows/build-<family>.yml`. Each SoC family's U-Boot tree lives on its
own branch:

- `openipc-hisilicon`: U-Boot for HiSilicon/Goke camera SoCs. It has `build.sh`,
  `openipc/`, `product/` and the vendor tree.
- `openipc-xm`: U-Boot for Xiongmai XM510/XM530/XM550, on the same mainline
  release with the same OpenIPC layer. CI: `build-xm.yml`, called from that
  branch's `ci.yml`. Its own `CLAUDE.md` covers the run-in-place boot.
- `openipc-gk710x`: U-Boot for Goke GK7101/GK7102, on the same mainline
  release with the same OpenIPC layer. CI: `build-gk710x.yml`, called from
  that branch's `ci.yml`. Its own `CLAUDE.md` covers the boot header, the
  flash byte order and the kernel hand-off.
- `openipc-anyka`: U-Boot for Anyka AK3918EV200, AK391xEV300,
  AK391xEV330, AK3918EV300L and AK3918AV100 (N/P and X/XP), on the same
  mainline release with the same OpenIPC layer.
  CI: `build-anyka.yml`, called from that branch's `ci.yml`. Its own
  `CLAUDE.md` covers the boot headers, the EV330 first stage and the
  drivers.
- `openipc-novatek`: U-Boot for Novatek's camera dies (NA51089, NA51055,
  NA51084, NS02501) and DVR/NVR dies (NA51068, NA51103, NA51090), on
  the same mainline release with the same OpenIPC layer, plus the SDK's
  loader, built from source in the tree. CI: `build-novatek.yml`, called
  from that branch's `ci.yml`. Its own `CLAUDE.md` covers the loader, the
  image layout and the kernel hand-off.
- `openipc-allwinner`: U-Boot for Allwinner V3s, V831/V833 (sun8iw19)
  and V851s, on the same mainline release with the same OpenIPC layer, as
  mainline sunxi SoCs with their own SPL and DRAM drivers. CI: `build-allwinner.yml`,
  called from that branch's `ci.yml`. Its own `CLAUDE.md` covers the SPL,
  the DRAM port and the kernel hand-off.
- `openipc-fullhan`: U-Boot for Fullhan FH8632, FH885x V100,
  FH8626V100, FH885xV200 and FH865x, on the same mainline release with
  the same OpenIPC layer. CI: `build-fullhan.yml`, called from that
  branch's `ci.yml`. Its own `CLAUDE.md` covers the boot header and its
  DDR scripts, the vendor ramboots and the kernel hand-off.
- `openipc-grainmedia`: U-Boot for Grain Media GM8135S, GM8136 and
  GM8136S, on the same mainline release with the same OpenIPC layer,
  plus the SDK's nsboot, built from source in the tree. CI:
  `build-grainmedia.yml`, called from that branch's `ci.yml`. Its own
  `CLAUDE.md` covers nsboot, the image layout and the kernel hand-off.
- `openipc-sigmastar`: U-Boot for SigmaStar infinity3 (MSC313E, MSC316DC,
  MSC316DM), infinity2m (SSD201, SSD202D, SSR621Q), infinity6, 6b0, 6e, 6c
  (SSC3xx), Iford (SSC029A/B/C), Ifado (SSC375) and Ifliegen (SSC034A/B),
  NOR and SPI NAND, on the same mainline release with the same
  OpenIPC layer, behind the vendor's IPL and IPL_CUST, carried as
  binaries. CI: `build-sigmastar.yml`, called from that branch's
  `ci.yml`. Its own `CLAUDE.md` covers the boot chain, the image layout
  and the kernel hand-off.
- `openipc-ingenic`: U-Boot for Ingenic T10/T20/T21/T23/T30/T31 (XBurst) and
  T40/T41 (XBurst2), MIPS, on the same mainline release with the same OpenIPC layer, plus
  the vendor SPL built from source in the tree. CI: `build-ingenic.yml`,
  called from that branch's `ci.yml`. Its own `CLAUDE.md` covers the
  SPL, the DDR tables and the vendor quirks.
- `openipc-axera`: U-Boot for Axera AX620U (Cortex-A7, SPI NAND) and the
  AX620E family (Cortex-A53, AArch64, NOR), on the same mainline release
  with the same OpenIPC layer, behind a DDR-init first stage built from
  source and carried in the image (AX620E: the SDK's BL1 SPL,
  `openipc/ax620e-spl`; AX620U: a C rebuild of the cameras' own,
  `openipc/ax620u-spl`). CI:
  `build-axera.yml`, called from that branch's `ci.yml`. Its own
  `CLAUDE.md` covers the boot chain, the Axera header and the drivers.
- `main`: a fork of upstream mainline U-Boot, kept for rebasing.

To change bootloader code, check out `openipc-hisilicon`, or read it with
`git show origin/openipc-hisilicon:<path>`. Changes on this branch are almost
always to CI or documentation.

`openipc-hisilicon` has its own `CLAUDE.md`. It covers how that tree differs
from mainline v2022.10 (the vendor code, the mainline files it patched, the
files it reverted to vendor copies), plus the build, config assembly and the
OpenIPC boot layer. Read it before any
change that touches both branches: `git show origin/openipc-hisilicon:CLAUDE.md`.
Adding, removing or holding a SoC needs edits on both branches. On
`openipc-hisilicon`, change `build.sh` and `openipc/<soc>_*`. Here, change
both workflow matrices and the README.

## Cross-branch CI wiring

The build logic lives here. Other branches call it:

- `openipc-hisilicon/.github/workflows/ci.yml` is a thin caller:
  `uses: OpenIPC/u-boot/.github/workflows/build-hisilicon.yml@openipc` with
  `ref: ${{ github.sha }}`. A change to `build-hisilicon.yml` therefore affects
  that branch's push/PR CI as soon as it lands on `openipc`.
- A PR against `openipc` runs the workflow from the PR's tree against the
  current `openipc-hisilicon` (no `ref` input, so it falls back to
  `'openipc-hisilicon'`).
- The `resolve` job pins the source revision once. Every build leg checks out
  that SHA, and release provenance and the openipc.org report must name it.
  `github.sha` names the `openipc` commit, so it is never the built revision.
- Job graph: `resolve` → `build` (matrix, one SoC per leg) + `qemu` (builds
  `qemu-system-arm` from widgetii/qemu-hisilicon once) → `qemu_smoke` (hard
  gate) → `release` / `publish`. These last two run only on `workflow_dispatch`.

## `build-xm.yml`

- It has the same job graph as `build-hisilicon.yml`, with the same
  `resolve`/`ref` rules and the same partition limits.
- `pull_request` runs only when the workflow file itself changes.
- **No `hold`:** every SoC has a qemu-hisilicon machine, so every image is
  smoke-tested and released.
- **NOR only:** every leg builds, uploads and smoke-tests
  `u-boot-<soc>-nor.bin` alone. openipc-xm has no SPI NAND images.
- **Smoke test:** two boots of an 8 MiB flash image under
  `-global xm-sfc.wps=on`.
  - It passes on `Hit Ctrl-C`.
  - It fails if the second boot reports a bad environment CRC. That means the
    driver did not clear the Xiongmai per-block protection, and the first
    boot's `saveenv` was dropped.
  - It needs a qemu-hisilicon with the `wps` property (PR #150 or later).
- **Release and publish:**
  - `release` uploads to the `xiongmai` tag.
  - `publish` shares the `publish-firmware-latest` concurrency group with
    hisilicon and reports `--source uboot` under a `uboot-xm-…` build id.
  - openipc.org keeps the newest build per file name, so the two workflows'
    pushes do not displace each other.

## `build-gk710x.yml`

- It is `build-xm.yml` for `openipc-gk710x`: the same job graph and
  `resolve`/`ref` rules, NOR only (256 KiB boot partition, header
  included).
- **No `hold`:** every SoC has a qemu-hisilicon machine (`gk7101`,
  `gk7102`; and `gk7101s`, `gk7102s`, which need qemu-hisilicon's GK710xS
  machines).
- **Smoke test:** two boots of an 8 MiB flash image holding only U-Boot.
  - The release image is **in chip byte order** (the flash controller packs
    bytes MSB first, so the chip holds every word byte-swapped), and goes
    into the flash file as it is. An image of an earlier build, in CPU
    order ("17KG"), is swapped into chip order first; anything else is
    refused. `openipc-gk710x`'s `CLAUDE.md` explains how `sf write` takes the
    same file.
  - It passes on `Hit Ctrl-C`, and fails if the second boot reports a bad
    environment CRC.
- **Release and publish:**
  - `release` uploads to the `gk710x` tag.
  - `publish` shares the `publish-firmware-latest` concurrency group and
    reports a `uboot-gk710x-…` build id.
  - openipc.org accepts the report only from workflows it allowlists
    (OpenIPC/website `service/internal/builds/verify.go`). `build-gk710x.yml`
    has to be added there before the first `publish`.

## `build-anyka.yml`

- It is `build-gk710x.yml` for `openipc-anyka`: the same job graph and
  `resolve`/`ref` rules, NOR only (256 KiB boot partition, header
  included; on the EV330 the 0x1200-byte header and first stage).
- **No `hold`:** nine of the ten SoCs have machines of the same names in
  qemu-hisilicon (now OpenIPC/qemu-bench, the repository the `qemu` job
  clones): #161 or later; the EV300L #305 or later; the two AV100s #310 or
  later, which adds `ak3918av100x`. The AV100 ROM copies from flash 0x200
  like the others, through the secure-boot space (qemu-bench #376; the
  header's load address is 0x80DFFC00, u-boot #155): an `ANYKA322` image
  built before #155 no longer boots there, as it does not on the chip.
- **`ak3918ev331` has no machine of its own.** It is the AK3919EV330's die
  and 128 MiB, so the smoke step boots it on `ak3919ev330` (the `case` in
  "Smoke-test in QEMU"). A new target without a same-named machine needs an
  arm there too.
- **Smoke test:** two boots of an 8 MiB flash image holding only U-Boot,
  in CPU byte order. It passes on `Hit Ctrl-C`,
  and fails if the second boot reports a bad environment CRC.
- **Release and publish:**
  - `release` uploads to the `anyka` tag.
  - `publish` shares the `publish-firmware-latest` concurrency group and
    reports a `uboot-anyka-…` build id.
  - openipc.org accepts the report only from workflows it allowlists
    (OpenIPC/website `service/internal/builds/verify.go`). `build-anyka.yml`
    has to be added there before the first `publish`.

## `build-novatek.yml`

- It is `build-anyka.yml` for `openipc-novatek`: the same job graph and
  `resolve`/`ref` rules.
- **Images.** The camera dies (NA51089, NA51055, NA51084, NS02501) and
  NA51068 are NOR only:
  the 256 KiB boot partition holds the loader, the device tree at
  0x10000 and U-Boot at 0x20000. The NVRs (`nt98332`, `nt98633`) release
  U-Boot's slot alone (128 KiB of NOR, 896 KiB of NAND), and build.sh
  also writes `u-boot-<soc>-qemu.bin`, a boot area for the emulator's
  stand-in loader. That one is uploaded as `qemu-image-<soc>`, outside
  the release's `u-boot-*` pattern, and never released.
- The build legs also install `gcc-arm-none-eabi` (the loaders; the
  NT98528/529 one needs a bare-metal toolchain, build.sh refuses a Linux
  one) and `gcc-aarch64-linux-gnu` (the NVRs).
- **Partition fit:** each leg checks its released image against
  `limit_kb`, its slot.
- **No `hold`:** every SoC has a qemu-hisilicon machine, but the NT98520
  and NT98522, which run on the NT98525's and NT98523's (the same dies).
  The camera machines run the image's own loader through their mask ROM
  model; the NVR legs run the stand-in image (`-smp 2`, `-smp 4`). The
  machines beyond `nt98562`/`nt98566` need OpenIPC/qemu-hisilicon#222 on
  its master, which the `qemu` job builds (qemu-system-arm and
  qemu-system-aarch64).
- **Smoke test:** two boots of a flash image holding only the image (8
  MiB NOR; 16 MiB for `nt98332`, 128 MiB of NAND for `nt98633`). It
  passes on `Hit Ctrl-C`, and fails if the second boot reports a bad
  environment CRC.
- **Release and publish:**
  - `release` uploads to the `novatek` tag.
  - `publish` shares the `publish-firmware-latest` concurrency group and
    reports a `uboot-novatek-…` build id.
  - openipc.org accepts the report only from workflows it allowlists
    (OpenIPC/website `service/internal/builds/verify.go`).
    `build-novatek.yml` has to be added there before the first `publish`.

## `build-allwinner.yml`

- It is `build-anyka.yml` for `openipc-allwinner`: the same job graph and
  `resolve`/`ref` rules, NOR only (256 KiB boot partition: the SPL, then
  U-Boot LZMA-packed at 0x8000).
- The build legs also install `xz-utils`, for the LZMA image.
- **No `hold`:** every SoC has a qemu-hisilicon machine (`v3s`, `v831`,
  `v833`, `v851s`). The `-mmc` legs need its boot ROM stand-in to boot
  from the SD card (qemu-bench #204); `v3s` and `v851s` are
  `allwinner-cam.c` there.
- **Two kinds per SoC:** `u-boot-<soc>-nor.bin` and `-mmc.bin` (SD card
  boot: the SPL reads U-Boot from the card); one artifact holds both, the
  smoke matrix has a leg per kind.
- **Smoke test:** two boots, `-nor` from an 8 MiB flash holding only the
  image, `-mmc` from a 64 MiB card with it at 8 KiB and the flash erased.
  It passes on `Hit Ctrl-C`, and fails if the second boot reports a bad
  environment CRC (the environment is in the flash for both).
- **Release and publish:**
  - `release` uploads to the `allwinner` tag.
  - `publish` shares the `publish-firmware-latest` concurrency group and
    reports a `uboot-allwinner-…` build id. It skips `u-boot-v851s-*`,
    in the upload and in the report: OpenIPC/u-boot-allwinner publishes
    the camera-tested `u-boot-v851s-nor.bin` to the same release, and
    `--clobber` would replace it. Lift the filter only once these V851s
    images have run on a camera.
  - openipc.org accepts the report only from workflows it allowlists
    (OpenIPC/website `service/internal/builds/verify.go`).
    `build-allwinner.yml` has to be added there before the first
    `publish`.

## `build-fullhan.yml`

- It is `build-novatek.yml` for `openipc-fullhan`: the same job graph and
  `resolve`/`ref` rules, NOR only (256 KiB boot partition: the ROM header
  and DDR scripts, the ramboot on FH865x/V100, U-Boot).
- **No `hold`:** every SoC has a qemu-hisilicon machine (qemu-bench #207).
  The smoke matrix maps images to machines: `fh885{2,6,8}v210` boot on
  `fh885{2,6,8}v200`, `fh8833v100` on `fh8632`. Keep the build matrix,
  the smoke matrix and the README's list in step.
- **Smoke test:** two boots of an 8 MiB flash image holding only the
  image. It passes on `Hit Ctrl-C`, and fails if the second boot reports
  a bad environment CRC.
- **Release and publish:**
  - `release` uploads to the `fullhan` tag.
  - `publish` shares the `publish-firmware-latest` concurrency group and
    reports a `uboot-fullhan-…` build id.
  - openipc.org accepts the report only from workflows it allowlists
    (OpenIPC/website `service/internal/builds/verify.go`).
    `build-fullhan.yml` has to be added there before the first `publish`.

## `build-grainmedia.yml`

- It is `build-fullhan.yml` for `openipc-grainmedia`: the same job graph
  and `resolve`/`ref` rules, NOR only (256 KiB boot partition: the flash
  header, nsboot, U-Boot).
- **Images are not machines:** one image serves several packages, so the
  smoke matrix boots `gm8136` on both `gm8136` and `gm8136s`, `gm8135` on
  `gm8135s`. Keep the build matrix, the smoke matrix and the README's list
  in step.
- **Smoke test:** two boots of an 8 MiB flash image holding only the
  image; qemu-hisilicon runs nsboot from it. It passes on `Hit Ctrl-C`,
  and fails if the second boot reports a bad environment CRC.
- **Release and publish:**
  - `release` uploads to the `grainmedia` tag.
  - `publish` shares the `publish-firmware-latest` concurrency group and
    reports a `uboot-grainmedia-…` build id.
  - openipc.org accepts the report only from workflows it allowlists
    (OpenIPC/website `service/internal/builds/verify.go`).
    `build-grainmedia.yml` has to be added there before the first
    `publish`.

## `build-sigmastar.yml`

- It is `build-anyka.yml` for `openipc-sigmastar`: the same job graph and
  `resolve`/`ref` rules, NOR and SPI NAND. A NOR image is the 256 KiB
  boot partition (the vendor's IPL, IPL_CUST and on Ifliegen TF-A, the
  die's flash table, U-Boot). A SPI NAND one is the 768 KiB boot partition
  of openipc-hisilicon's `768k(boot),256k(env),-(ubi)`, the fit check's
  limit on every die.
- **Every image a machine:** each leg, NOR and NAND, boots on its own
  machine (`ssc029b` as `ssc029,bond=0x10`, `ssc029c` as `ssc029`; the
  infinity6e NAND legs at `bond=0x10`). The `qemu` job builds a pinned
  commit, OpenIPC/qemu-bench#242's, for ssc029a/b, ssc327de and ssc034a/b,
  until it merges. Keep the build matrix, the smoke matrix and the README's
  list in step.
- **Smoke test:** two boots of the image in an erased flash: 8 MiB NOR
  (infinity3) or 16 MiB, or 128 MiB of SPI NAND pages with their spares.
  The machine's mask ROM loads the IPL. It passes on `Hit Ctrl-C`, and
  fails if the second boot reports U-Boot's "Warning - bad CRC" (the
  infinity6c IPL's own "env bad CRC" is not U-Boot's).
- **Release and publish:**
  - `release` uploads to the `sigmastar` tag.
  - `publish` is commented out for now, with its input: SigmaStar images
    go only to the `sigmastar` release. When it returns, it shares the
    `publish-firmware-latest` concurrency group and reports a
    `uboot-sigmastar-…` build id. No other repository
    publishes `u-boot-msc31*` names to `latest`, and `PUBLISH_GLOB` keeps
    `publish` to those: `u-boot-ssc*` in `latest` are
    OpenIPC/u-boot-sigmastar's vendor U-Boot, which cameras flash, and the
    ones built here have not run on silicon. `release` carries them all.
  - openipc.org accepts the report only from workflows it allowlists
    (OpenIPC/website `service/internal/builds/verify.go`).
    `build-sigmastar.yml` has to be added there before the first
    `publish`.

## `build-ingenic.yml`

- `resolve` -> `build` (matrix, one target per leg) -> `release` +
  `publish`; no `qemu`/`qemu_smoke`, since qemu-hisilicon has no Ingenic
  (MIPS) machine.
- NOR on every leg: 256 KiB boot partition (the SPL in its 26 KiB slot,
  then the lzop-packed U-Boot), env at 0x40000. The T40 legs (`kinds:
  "nor nand"`) also build `-nand.bin`: 768 KiB boot partition (the SPL in
  its 27 KiB slot, U-Boot at 0x6c00), env at 0xc0000, openipc-hisilicon's
  UBI layout. The build legs install `gcc-mipsel-linux-gnu` and `lzop`.
- **`publish` uploads only `publish_targets`**, and only from a dispatch
  on `openipc` (openipc.org accepts the report from no other branch; it
  lists this workflow in its builds pushers, OpenIPC/website#591). The
  default is the seven T41 targets, which have no other source in
  OpenIPC/firmware `latest`. The `u-boot-t10..t31-nor.bin` names there are
  OpenIPC/u-boot-ingenic's, which cameras run: add one of those targets to
  a publish, or to the default, only after its image here has booted on a
  camera.
- Keep the build matrix and the README's list in step with the branch's
  `build.sh` `ALL_SOCS`.

## `build-axera.yml`

- It is `build-sigmastar.yml` for `openipc-axera`: the same job graph and
  `resolve`/`ref` rules. The matrix is build.sh's targets, `<soc>-<flash>`:
  `ax620u-nand` (arm-linux-gnueabi-) and `ax620e-nor`
  (gcc-aarch64-linux-gnu, build.sh's `CROSS_COMPILE_64` default); the build
  legs also need `curl`, as build.sh fetches the SDK's `ax_gzip` (checked
  against its SHA-256).
- **Partition fit:** every image is written at 0 with its first stage,
  up to the environment: the limit is the built revision's own
  `CONFIG_ENV_OFFSET` (from `output/build-<soc>/.config`), so a layout
  change needs no workflow change. `ax620u-nand` is the 768 KiB boot
  partition of OpenIPC's NAND layout (`0xc0000`; revisions before it
  built the cameras' 1 MiB `spl` and 1536 KiB `boot`, `0x280000`),
  `ax620e-nor` the 256 KiB boot partition. `u-boot-<soc>-stockspl.bin`,
  the same with the cameras' own first stage (the fallback), is checked
  against the same limit.
- **Artifacts:** `u-boot-<soc>` holds `u-boot-<soc>.bin` only, what the
  release takes; `stockspl-<soc>` holds the `-stockspl` image, for the
  smoke test, and is not released.
- **Smoke test:** the `qemu` job builds qemu-system-aarch64
  from OpenIPC/qemu-bench at a pinned commit; the source-built AX620E first
  stage needs one with `axera-spi-nor` ending a command at the flash's
  software reset (qemu-bench#351), or its first read returns zeros
  (`verify_img_header: magic 0x55543322 != 0x0`), and gets the AX620Q DDR
  path from #346's default eFuse chip type; the pin, `e84387f`, has both. One QEMU run of `-M ax620e`
  on a 16 MiB erased NOR holding the image, with
  `-global axera-spi-nor.env-from-flash=on`: the first boot must reach
  `Hit Ctrl-C` (saving the default environment), then `reset` (the
  watchdog, back through the first stage) must reach it again, with
  U-Boot's "Warning - bad CRC" printed once only. The model keeps flash
  writes in memory, not in the file, so two QEMU runs would not test the
  saved environment. The same for AX620U on `-M ax620u` (128 MiB erased
  SPI NAND, the image at 0, U-Boot's header at 0x40000 or, in the
  cameras' layout of earlier revisions, 0x100000; the machine starts the
  first stage at flash 0x800, no boot ROM model, so the first stage's
  header at RAM 0x400 is zeros and its NAND geometry 64 KiB blocks; the
  AX620U legs need no qemu-bench change),
  and for each SoC's `-stockspl` image: four legs. A leg whose image the
  revision does not build (no `-stockspl`, or an AX620U image of U-Boot
  alone) is skipped with a warning, except AX620E's source one.
- **Release and publish:**
  - `release` uploads to the `axera` tag.
  - `publish` is commented out, with its input: neither image has run on
    silicon. When it returns, it shares the `publish-firmware-latest`
    concurrency group, runs only from `openipc`, and reports a
    `uboot-axera-…` build id; openipc.org accepts the report only from
    workflows it allowlists (OpenIPC/website
    `service/internal/builds/verify.go`), so add `build-axera.yml` there
    first.

## Invariants to keep when editing `build-hisilicon.yml`

- **Three SoC lists must agree**: the `build` matrix, the `qemu_smoke` matrix,
  and the README's SoC list. The comment above `qemu_smoke` names the SoCs that
  have no qemu machine model and therefore no smoke leg. Keep that comment and
  the README count ("31 SoCs with a machine model, eight without") in sync.
- **Partition limits**: NOR images must be ≤ 256 KiB (env at 0x40000), or a
  leg's `nor_kb` (hi3519dv500: 512 KiB, env at 0x80000); NAND images ≤ 768 KiB
  (0xc0000). An oversized image bricks the device on upgrade.
- **hi3519dv500 is arm64**: its leg needs `gcc-aarch64-linux-gnu`, and its
  smoke leg runs `qemu-aarch64`, built by the `qemu` job beside `qemu-arm`.
- **Smoke pass criterion** is the autoboot prompt `Hit Ctrl-C`, not the banner.
  The banner prints before the board init that can hang (see #4).
- **Held SoCs** (`hold: true`, currently none): their artifact is
  named `hold-u-boot-<soc>`, so the `u-boot-*` download pattern in
  release/publish skips them. Holding a SoC means setting `hold: true`. Do not
  delete the SoC from the matrix.
- **`release` vs `publish` are deliberately separate inputs.**
  - `release` uploads to this repo's `hisilicon` pre-release.
  - `publish` uploads to the OpenIPC/firmware `latest` release, clobbering
    same-named images from u-boot-xmedia and u-boot-hi3516cv6xx, and then
    reports to openipc.org.
  - `publish` runs under the `publish-firmware-latest` concurrency group, so
    the openipc.org report always describes exactly the bytes uploaded.
  - It checks out `OpenIPC/firmware` at a pinned commit, because that job holds
    an `id-token`.
- Artifact names differ by SoC family: most SoCs produce
  `u-boot-<soc>-{nor,nand}.bin`, while hi3516cv608/cv610 and hi3519dv500
  produce `boot-<soc>*-nor.bin`. The `files:` globs and the smoke step's `ls` both
  depend on these names.

## Commands

```sh
yamllint .github/workflows/build-hisilicon.yml       # only local workflow lint available

# Build locally (on the hisilicon branch, toolchain arm-linux-gnueabi-):
git worktree add ../u-boot-hisi openipc-hisilicon
cd ../u-boot-hisi && bash build.sh hi3516cv300       # one SoC; no args = all
# -> output/u-boot-<soc>-nor.bin / -nand.bin

# Reproduce one smoke leg locally (needs qemu-hisilicon's qemu-system-arm, which
# has the -M <soc> machine models; the stock system qemu does not):
dd if=/dev/zero of=flash.bin bs=1M count=8
dd if=output/u-boot-<soc>-nor.bin of=flash.bin conv=notrunc
qemu-system-arm -M <soc>,flash-file=flash.bin -nographic   # expect "Hit Ctrl-C"

gh workflow run build-hisilicon.yml                                   # build + smoke
gh workflow run build-hisilicon.yml -f release=true                   # + own release
gh workflow run build-hisilicon.yml -f release=true -f publish=true   # + OpenIPC/firmware + openipc.org
```

`build-hisilicon.yml`'s build legs symlink `/bin/sh` to bash, because the
`openipc-hisilicon` vendor recipes depend on bash-isms. They also need
`pycryptodome`, `u-boot-tools`, `device-tree-compiler` and `lzop`. The other
families' builds use none of those recipes: their workflows install what their
`build.sh` runs (for `openipc-novatek`, also `gcc-arm-none-eabi`; its loader's
make system builds under dash).

## Conventions

- Commit subjects use a `ci:` prefix. Bodies explain the failure mode a change
  prevents and how it was checked, often against qemu-hisilicon locally.
- To add a SoC family: push the port on a new branch, add
  `build-<family>.yml` here, and document the family in `README.md`.
