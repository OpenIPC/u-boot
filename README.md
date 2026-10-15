# OpenIPC U-Boot

[![hisilicon build](https://github.com/OpenIPC/u-boot/actions/workflows/build-hisilicon.yml/badge.svg)](https://github.com/OpenIPC/u-boot/actions/workflows/build-hisilicon.yml)

Unified repository for the U-Boot bootloaders maintained by
[OpenIPC](https://openipc.org) for supported SoCs.

Each SoC family lives on its own branch. This **`openipc`** branch is the
entry point: it holds the documentation and the CI workflows for every
family — not bootloader sources.

| Branch | Contents |
| --- | --- |
| `openipc` (this branch) | Unified documentation and CI workflows |
| `openipc-hisilicon` | U-Boot for HiSilicon/Goke IP-camera SoCs |
| `main` | Fork of upstream mainline U-Boot, kept for rebasing |

New families are added here as they are ported, each with its own
`.github/workflows/build-<family>.yml`.

## hisilicon

The current port: U-Boot for 38 HiSilicon and Goke camera SoCs.

**NOR + NAND:**

`hi3516cv500`, `hi3516dv300`, `hi3516av300`, `hi3516dv200`,
`hi3516ev200`, `hi3516ev300`, `hi3518ev300`, `gk7201v200`, `gk7201v300`,
`gk7202v300`, `gk7205v200`, `gk7205v300`, `gk7205v500`, `gk7205v510`,
`gk7205v530`, `gk7605v100`, `gk7206v11a`\*, `gk7206v12a`\*

\* Goke GK7206V1 (XMedia Lotus "xmorca"), from the vendor SDK's own
first stage and DDR-init code. Not yet booted on hardware: CI builds the
images but keeps them out of the release and publish steps.

**NOR only:**

`hi3516cv100`, `hi3516cv200`, `hi3516cv300`, `hi3516cv608`,
`hi3516cv610`, `hi3516av100`, `hi3516av200`, `hi3516dv100`,
`hi3516ev100`, `hi3518av100`, `hi3518cv100`, `hi3518ev100`,
`hi3518ev200`, `hi3519v101`, `hi3515av100`, `hi3520av100`,
`hi3520dv100`, `hi3520dv200`, `hi3536cv100`, `hi3536dv100`

### Build

```sh
git clone --branch openipc-hisilicon https://github.com/OpenIPC/u-boot.git
cd u-boot

bash build.sh               # every supported SoC
bash build.sh hi3516cv300   # a single SoC
```

The toolchain is `arm-linux-gnueabi-` (Debian/Ubuntu package
`gcc-arm-linux-gnueabi`). Images land in `output/` as
`u-boot-<soc>-nor.bin` / `u-boot-<soc>-nand.bin`.

## CI

The [build-hisilicon](.github/workflows/build-hisilicon.yml) workflow runs
on pull requests to this branch, on pushes and pull requests to
`openipc-hisilicon` (through that branch's `ci.yml`, which builds the
pushed commit or the pull request merged into its base), and manually
from the Actions tab. Each run:

- builds every SoC in the list above, one matrix leg per SoC, and fails
  if an image overflows its boot partition;
- boots each NOR image in the matching machine model from
  [widgetii/qemu-hisilicon](https://github.com/widgetii/qemu-hisilicon);
  the 30 SoCs with a machine model are a hard gate, the eight without one
  get no smoke leg;
- uploads one artifact per SoC.

Two dispatch inputs control uploads; both skip the held GK7206 images.

- **`release: true`** uploads every built image to this repository's
  [`Hisilicon` release](../../releases/tag/hisilicon).
- **`publish: true`** pushes them to the `OpenIPC/firmware` `latest`
  release and reports the result to openipc.org. This requires a
  `FIRMWARE_RELEASE_TOKEN` secret with release write access to
  `OpenIPC/firmware`. `latest` already carries images under the same
  names from OpenIPC/u-boot-xmedia (`u-boot-gk7205v500/v510/v530-*`,
  `u-boot-hi3516ev200/ev300/dv200-*`, `u-boot-hi3518ev300-*`) and
  u-boot-hi3516cv6xx (`boot-hi3516cv60x/cv610-*`); publishing replaces
  them with this tree's builds.

```sh
gh workflow run build-hisilicon.yml                                   # build + smoke test
gh workflow run build-hisilicon.yml -f release=true                   # ... and this repo's release
gh workflow run build-hisilicon.yml -f release=true -f publish=true   # ... and OpenIPC/firmware + openipc.org
```

## Adding a SoC family

1. Push the port on a new branch, based on a mainline U-Boot release or
   on an existing family's baseline.
2. Add a `build-<family>.yml` workflow on this branch.
3. Document the family and its branches here.

## License

U-Boot is licensed under the GPL-2.0+; see the `Licenses/` directory and
the per-file SPDX tags on the corresponding branch.
