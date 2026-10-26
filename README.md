# OpenIPC U-Boot for Grain Media

Mainline U-Boot v2022.10 with the OpenIPC boot layer, for Grain Media's
GM8135S, GM8136 and GM8136S IP camera SoCs. The upstream U-Boot
documentation is in [`README`](README) and `doc/`; the details of this
port are in [`CLAUDE.md`](CLAUDE.md).

## Images

One NOR image per OpenIPC SoC name, `u-boot-<soc>-nor.bin`:

| `<soc>` | Packages | CPU | DDR |
| --- | --- | --- | --- |
| `gm8136` | GM8136, GM8136S | FA626TE | external (256 MiB table) / 128 MiB |
| `gm8135` | GM8135S | FA626TE | 64 MiB |

Each image holds the flash header the boot ROM reads, nsboot (the SDK's
first stage, built from source here, which sets up DDR for whichever
package it runs on) and U-Boot. Write it at flash offset 0 as it is; the
environment is at `0x40000`, the OpenIPC kernel at `0x50000`.

## Build

Needs an ARM cross compiler (`arm-linux-gnueabi-` by default;
`CROSS_COMPILE` overrides it, `NSBOOT_CROSS_COMPILE` nsboot's) and
Python 3.

```sh
bash build.sh                     # every SoC above
bash build.sh gm8136              # a single SoC
```

Images land in `output/` as `u-boot-<soc>-nor.bin`. The build prints how
much room is left before the environment, and fails if the image would
reach it.

## Install

From a running U-Boot with network (OpenIPC's environment):

```
setenv serverip <tftp server>
run ub                            # fetches u-boot-<soc>-nor.bin, writes it at 0
```

or with `sf write` of the file at offset 0, or with a programmer. SD card
and TFTP recovery work as on the other OpenIPC families.

## Status

These images have run only on qemu-hisilicon's Grain Media machines
(which run nsboot), not yet on a camera. The DDR tables and clock modes
are the SDK's defaults for each package.

CI: `.github/workflows/ci.yml` calls `build-grainmedia.yml` on the
`openipc` branch, which builds every SoC and boots each image twice
under QEMU.
