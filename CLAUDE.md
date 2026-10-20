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
- `openipc-anyka`: U-Boot for Anyka AK3918EV200, AK391xEV300 and
  AK391xEV330, on the same mainline release with the same OpenIPC layer.
  CI: `build-anyka.yml`, called from that branch's `ci.yml`. Its own
  `CLAUDE.md` covers the boot headers, the EV330 first stage and the
  drivers.
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
- **NAND legs:** xm530/xm550 build `kinds: "nor nand"`, and `qemu_smoke`
  has a NAND leg for each.
  - A NAND leg boots a raw 128 MiB image (2048+64-byte pages) through
    `-global xm-sfc.nand-file=`, which needs qemu-hisilicon's NAND model.
  - The NAND images rest on an unverified guess at the XM530 NAND boot
    path; `openipc-xm`'s `CLAUDE.md` explains it.
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
- **No `hold`:** all six SoCs have qemu-hisilicon machines of the same
  names (qemu-hisilicon #161 or later).
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
- **Held SoCs** (`hold: true`, currently gk7206v11a/v12a): their artifact is
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

CI's build legs symlink `/bin/sh` to bash, because the vendor recipes depend on
bash-isms. They also need `pycryptodome`, `u-boot-tools`,
`device-tree-compiler` and `lzop`.

## Conventions

- Commit subjects use a `ci:` prefix. Bodies explain the failure mode a change
  prevents and how it was checked, often against qemu-hisilicon locally.
- To add a SoC family: push the port on a new branch, add
  `build-<family>.yml` here, and document the family in `README.md`.
