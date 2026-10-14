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

The current port: U-Boot for 36 HiSilicon and Goke camera SoCs.

**NOR + NAND:**

`hi3516cv500`, `hi3516dv300`, `hi3516av300`, `hi3516dv200`,
`hi3516ev200`, `hi3516ev300`, `hi3518ev300`, `gk7201v200`, `gk7201v300`,
`gk7202v300`, `gk7205v200`, `gk7205v300`, `gk7205v500`, `gk7205v510`,
`gk7205v530`, `gk7605v100`

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

The [build-hisilicon](.github/workflows/build-hisilicon.yml) workflow is
started manually from the Actions tab:

- builds every SoC in the list above, one matrix leg per SoC, and fails
  if an image overflows its boot partition;
- boots each NOR image in the matching machine model from
  [widgetii/qemu-hisilicon](https://github.com/widgetii/qemu-hisilicon);
  the 30 SoCs with a machine model are a hard gate, the six without one
  get no smoke leg;
- uploads one artifact per SoC.

With **`release: true`** the run additionally uploads every built image
to this repository's [`Hisilicon` release](../../releases/tag/hisilicon),
pushes them to the `OpenIPC/firmware` `latest` release and reports the
result to openipc.org. This requires a `FIRMWARE_RELEASE_TOKEN` secret
with release write access to `OpenIPC/firmware`.

```sh
gh workflow run build-hisilicon.yml                    # build + smoke test
gh workflow run build-hisilicon.yml -f release=true    # ... and publish
```

## Adding a SoC family

1. Push the port on a new branch, based on a mainline U-Boot release or
   on an existing family's baseline.
2. Add a `build-<family>.yml` workflow on this branch.
3. Document the family and its branches here.

## License

U-Boot is licensed under the GPL-2.0+; see the `Licenses/` directory and
the per-file SPDX tags on the corresponding branch.
