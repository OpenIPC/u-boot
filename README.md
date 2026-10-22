# OpenIPC U-Boot

[![hisilicon build](https://github.com/OpenIPC/u-boot/actions/workflows/build-hisilicon.yml/badge.svg)](https://github.com/OpenIPC/u-boot/actions/workflows/build-hisilicon.yml)
[![xiongmai build](https://github.com/OpenIPC/u-boot/actions/workflows/build-xm.yml/badge.svg)](https://github.com/OpenIPC/u-boot/actions/workflows/build-xm.yml)
[![gk710x build](https://github.com/OpenIPC/u-boot/actions/workflows/build-gk710x.yml/badge.svg)](https://github.com/OpenIPC/u-boot/actions/workflows/build-gk710x.yml)
[![anyka build](https://github.com/OpenIPC/u-boot/actions/workflows/build-anyka.yml/badge.svg)](https://github.com/OpenIPC/u-boot/actions/workflows/build-anyka.yml)

Unified repository for the U-Boot bootloaders maintained by
[OpenIPC](https://openipc.org) for supported SoCs.

Each SoC family lives on its own branch. This **`openipc`** branch is the
entry point: it holds the documentation and the CI workflows for every
family — not bootloader sources.

| Branch | Contents |
| --- | --- |
| `openipc` (this branch) | Unified documentation and CI workflows |
| `openipc-hisilicon` | U-Boot for HiSilicon/Goke IP-camera SoCs |
| `openipc-xm` | U-Boot for Xiongmai XM510/XM530/XM550 |
| `openipc-gk710x` | U-Boot for Goke GK7101/GK7102 |
| `openipc-anyka` | U-Boot for Anyka AK3918EV200, AK391xEV300, AK391xEV330 |
| `main` | Fork of upstream mainline U-Boot, kept for rebasing |

New families are added here as they are ported, each with its own
`.github/workflows/build-<family>.yml`.

## hisilicon

The current port: U-Boot for 38 HiSilicon and Goke camera SoCs.

**NOR + NAND:**

`hi3516cv500`, `hi3516dv300`, `hi3516av300`, `hi3516cv300`, `hi3516dv200`,
`hi3516ev200`, `hi3516ev300`, `hi3518ev300`, `gk7201v200`, `gk7201v300`,
`gk7202v300`, `gk7205v200`, `gk7205v300`, `gk7205v500`, `gk7205v510`,
`gk7205v530`, `gk7605v100`, `gk7206v11a`\*, `gk7206v12a`\*

\* Goke GK7206V1 (XMedia Lotus "xmorca"), from the vendor SDK's own
first stage and DDR-init code. Not yet booted on hardware: CI builds the
images but keeps them out of the release and publish steps.

**NOR only:**

`hi3516cv100`, `hi3516cv200`, `hi3516cv608`, `hi3516cv610`,
`hi3516av100`, `hi3516av200`, `hi3516dv100`, `hi3516ev100`,
`hi3518av100`, `hi3518cv100`, `hi3518ev100`, `hi3518ev200`,
`hi3519v101`, `hi3515av100`, `hi3520av100`, `hi3520dv100`,
`hi3520dv200`, `hi3536cv100`, `hi3536dv100`, `hi3519dv500`\*\*

\*\* arm64, built with `aarch64-linux-gnu-` (`CROSS_COMPILE64`). Its signed
boot images, `boot-hi3519dv500-{dmeb,dmebpro}-nor.bin`, go in a 512 KiB
boot partition: env at `0x80000`, firmware at `0xc0000`.

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

## xiongmai

U-Boot for Xiongmai's own camera SoCs, on the same mainline release
(v2022.10) and with the same OpenIPC boot layer as `openipc-hisilicon`.

**NOR:** `xm510`, `xm530`, `xm550`

The boot ROM runs U-Boot in place from SPI NOR, and U-Boot brings up DDR
itself, so the image is `u-boot.bin` as linked: `u-boot-<soc>-nor.bin`,
written at flash offset 0. The flash driver clears the per-block write
protection that Xiongmai bootloaders leave on (SR3.WPS), under which the
vendor U-Boot's `sf erase`/`sf write` report success and change nothing.

```sh
git clone --branch openipc-xm https://github.com/OpenIPC/u-boot.git
cd u-boot

bash build.sh               # xm510 xm530 xm550
bash build.sh xm530         # a single SoC
```

Images land in `output/` as `u-boot-<soc>-nor.bin`.

The toolchain is `arm-linux-gnueabi-`, as for hisilicon.

## gk710x

U-Boot for Goke's own ARM1176 camera SoCs, the GK7101 and GK7102 -- not
the HiSilicon-derived GK7205/GK7605 above -- on the same mainline release
(v2022.10) and with the same OpenIPC boot layer as `openipc-hisilicon`.

**NOR:** `gk7101`, `gk7102`, and the later GK710xS die: `gk7101s`,
`gk7102s`. The `gk7102s` image is also the one for GK7102C cameras, the
same family: it boots the GK7102C's boot header and hands its kernel that
board's GPIO map by default (`gpiomap=v2` or `v1` for SDK-style GK7102S
boards).

The boot ROM runs the DDR set-up listed in the image's header and copies
U-Boot into DDR, so `u-boot-<soc>-nor.bin` is the header and `u-boot.bin`,
written at flash offset 0. Like OpenIPC's `uImage` and `rootfs` for these
SoCs it is in the normal (CPU) byte order: load it by tftp, from an SD
card or a USB stick, and write it with `sf write` (or `run ub`).

**Programmer images.** The flash controller swaps the bytes of every
32-bit word on the way to the chip and back, so the chip itself holds
every word reversed: U-Boot, environment, kernel and rootfs alike. A
programmer writes the chip directly, so it needs the whole flash image in
that order. Do not write the files with a programmer as they are, and do
not just concatenate them: lay them out at their offsets, then reverse
every 32-bit word of the whole image.

```sh
# U-Boot alone
objcopy -I binary -O binary --reverse-bytes=4 \
    u-boot-gk7102-nor.bin u-boot-gk7102-programmer.bin

# a full 8 MiB flash: U-Boot at 0, kernel at 0x50000, rootfs at 0x250000
tr '\0' '\377' < /dev/zero | head -c 8M > full.bin
dd if=u-boot-gk7102-nor.bin  of=full.bin conv=notrunc
dd if=uImage.gk7102          of=full.bin conv=notrunc bs=64k seek=5
dd if=rootfs.squashfs.gk7102 of=full.bin conv=notrunc bs=64k seek=37
objcopy -I binary -O binary --reverse-bytes=4 full.bin full-programmer.bin

# a programmer's dump back into the normal order (what sf read shows)
objcopy -I binary -O binary --reverse-bytes=4 dump.bin dump-normal.bin
```

Without binutils, `perl -0777 -pe 's/(.)(.)(.)(.)/$4$3$2$1/gs' in > out`
does the same. qemu-hisilicon's `flash-file=` takes the programmer image.
A programmer image written through U-Boot anyway (`sf write` at 0, SD
recovery) is recognised and swapped back.

The vendor kernel needs what the vendor U-Boot leaves it besides ATAGs:
its HAL blob, memory layout words and the board's GPIO map, from which it
configures every pin. This U-Boot provides all three. The GPIO map is the
vendor reference boards' default layout (rb-sc1035 v2.1, sc1045, imx225,
ov9750, jxh42 v2.0, gc1024; GK7101 rb-ov2710 v2.2); `setenv gpiomap v1`
selects the other one (rb-ov9712, ar0130, jxh42 v1.0, GK7101 imx222).

U-Boot has started on GK7102C cameras; the rest has run only on
qemu-hisilicon's machines. Without Ethernet (GK7102C cameras), recovery is
from an SD card: `*.img` files, each a uImage wrapper whose load address is
the flash offset its payload goes to.

```sh
git clone --branch openipc-gk710x https://github.com/OpenIPC/u-boot.git
cd u-boot

bash build.sh               # gk7101 gk7102 gk7101s gk7102s
bash build.sh gk7102        # a single SoC
```

Images land in `output/` as `u-boot-<soc>-nor.bin`.

## anyka

U-Boot for Anyka's ARM926 camera SoCs, on the same mainline release
(v2022.10) and with the same OpenIPC boot layer as `openipc-hisilicon`.

**NOR:** `ak3918ev200`; `ak3916ev300`, `ak3918ev300`, `ak3919ev300`
(the AK3916EV301 runs the `ak3916ev300` image); `ak3918ev330`,
`ak3919ev330`. Within a generation the images differ only in the DDR size
(the AK3919s have 128 MiB, the others 64 MiB) and in `soc`.

The mask ROM runs the PLL and DDR script in the image's header and copies
U-Boot into DDR, so `u-boot-<soc>-nor.bin` is written at flash offset 0
as it is. On the EV330 the ROM loads a 4 KiB first stage into on-chip
memory, which loads U-Boot from flash offset 0x1200; that first stage and
its header are the vendor's, from a camera's flash, and lead the image.

Vendor uImages are built with the entry point 0x40 past the load address,
for the vendor U-Boot's way of running them where they were read to; this
U-Boot boots OpenIPC images (entry = load) and does not do that.

These images have so far run only on qemu-hisilicon's Anyka machines, not
on a camera. SD card recovery is not enabled.

```sh
git clone --branch openipc-anyka https://github.com/OpenIPC/u-boot.git
cd u-boot

bash build.sh               # all six
bash build.sh ak3918ev300   # a single SoC
```

Images land in `output/` as `u-boot-<soc>-nor.bin`.

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
  the 31 SoCs with a machine model are a hard gate, the eight without one
  get no smoke leg;
- uploads one artifact per SoC.

Two dispatch inputs control uploads; both skip the held GK7206 images.

- **`release: true`** uploads the built images to this repository's
  releases: the HiSilicon `hi*` images to the
  [`Hisilicon` release](../../releases/tag/hisilicon), the Goke `gk*`
  images to the [`Goke` release](../../releases/tag/goke).
- **`publish: true`** pushes them to the `OpenIPC/firmware` `latest`
  release and reports the result to openipc.org. This requires a
  `FIRMWARE_RELEASE_TOKEN` secret with release write access to
  `OpenIPC/firmware`. `latest` already carries images under the same
  names from OpenIPC/u-boot-xmedia (`u-boot-gk7205v500/v510/v530-*`,
  `u-boot-hi3516ev200/ev300/dv200-*`, `u-boot-hi3518ev300-*`) and
  u-boot-hi3516cv6xx (`boot-hi3516cv60x/cv610-*`) and u-boot-hi3519dv500
  (`boot-hi3519dv500-*`); publishing replaces them with this tree's builds.

```sh
gh workflow run build-hisilicon.yml                                   # build + smoke test
gh workflow run build-hisilicon.yml -f release=true                   # ... and this repo's release
gh workflow run build-hisilicon.yml -f release=true -f publish=true   # ... and OpenIPC/firmware + openipc.org
```

### xiongmai

The [build-xm](.github/workflows/build-xm.yml) workflow has the same
shape. It runs on pull requests to this branch that change it, on pushes
and pull requests to `openipc-xm` (through that branch's `ci.yml`), and
manually. Every SoC has a qemu-hisilicon machine model, so every image is
smoke-tested:

- booted in place from an 8 MiB flash image whose part powers up with the
  Xiongmai per-block protection on;
- booted a second time, which must load the environment the first boot
  saved.

Its `release` input fills this repository's
[`Xiongmai` release](../../releases/tag/xiongmai); `publish` uploads to
`OpenIPC/firmware` `latest`, where no other repository publishes
`u-boot-xm*` names, and reports to openipc.org.

```sh
gh workflow run build-xm.yml -f release=true -f publish=true
```

### gk710x

The [build-gk710x](.github/workflows/build-gk710x.yml) workflow has the
same shape. It runs on pull requests to this branch that change it, on
pushes and pull requests to `openipc-gk710x` (through that branch's
`ci.yml`), and manually. Every SoC has a qemu-hisilicon machine model, so
every image is smoke-tested: booted from an 8 MiB flash image holding it as
a programmer would write it (word-swapped), then a second time, which must load the
environment the first boot saved.

Its `release` input fills this repository's
[`Goke` release](../../releases/tag/goke); `publish` uploads to
`OpenIPC/firmware` `latest`, where no other repository publishes
`u-boot-gk710*` names, and reports to openipc.org.

```sh
gh workflow run build-gk710x.yml -f release=true -f publish=true
```

### anyka

The [build-anyka](.github/workflows/build-anyka.yml) workflow has the same
shape. It runs on pull requests to this branch that change it, on pushes
and pull requests to `openipc-anyka` (through that branch's `ci.yml`), and
manually. Every SoC has a qemu-hisilicon machine model, so every image is
smoke-tested: booted from an 8 MiB flash image, then a second time, which
must load the environment the first boot saved.

Its `release` input fills this repository's
[`Anyka` release](../../releases/tag/anyka); `publish` uploads to
`OpenIPC/firmware` `latest`, where no other repository publishes
`u-boot-ak39*` names, and reports to openipc.org.

```sh
gh workflow run build-anyka.yml -f release=true -f publish=true
```

## Adding a SoC family

1. Push the port on a new branch, based on a mainline U-Boot release or
   on an existing family's baseline.
2. Add a `build-<family>.yml` workflow on this branch.
3. Document the family and its branches here.

## License

U-Boot is licensed under the GPL-2.0+; see the `Licenses/` directory and
the per-file SPDX tags on the corresponding branch.
