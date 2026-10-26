# OpenIPC U-Boot for Fullhan

Mainline U-Boot v2022.10 with the OpenIPC boot layer, for Fullhan IP camera
SoCs. The upstream U-Boot documentation is in [`README`](README) and
`doc/`; the details of this port are in [`CLAUDE.md`](CLAUDE.md).

## Images

One NOR image per SoC, `u-boot-<soc>-nor.bin`:

| `<soc>` | Generation | CPU | DDR |
| --- | --- | --- | --- |
| `fh8852v200`, `fh8852v210` | FH885xV200 | ARM1176 | 64 MiB |
| `fh8856v200`, `fh8856v210` | FH885xV200 | ARM1176 | 128 MiB |
| `fh8858v200`, `fh8858v210` | FH885xV200 | ARM1176 | up to 256 MiB |
| `fh8652`, `fh8656`, `fh8658` | FH865x | Cortex-A7 | 64 / 128 / up to 256 MiB |
| `fh8852v100`, `fh8856v100` | FH885x V100 | ARM1176 | 64 / 128 MiB |
| `fh8626v100` | FH8626V100 | ARM1176 | 64 MiB |
| `fh8833v100` | FH8632 (FH8833) | ARM1176 | 64 MiB |

Each image holds the boot ROM's header, its DDR scripts, the vendor ramboot
on FH865x and V100, and U-Boot. Write it at flash offset 0 as it is; the
environment is at `0x40000`, the OpenIPC kernel at `0x50000`.

## Build

Needs an ARM cross compiler (`arm-linux-gnueabi-` by default;
`CROSS_COMPILE` overrides it) and Python 3.

```sh
bash build.sh                     # every SoC above
bash build.sh fh8852v200          # a single SoC
bash build.sh fh8852v200 fh8652   # several
```

Images land in `output/` as `u-boot-<soc>-nor.bin`; each SoC builds in
`output/build-<soc>-nor/`. The build prints how much of the 256 KiB boot
partition is left, and fails if the image would reach the environment.

## Install

From a running U-Boot with network (OpenIPC's environment):

```
setenv serverip <tftp server>
run ub                            # fetches u-boot-<soc>-nor.bin, writes it at 0
```

or with `sf write` of the file at offset 0, or with a programmer. SD card
and TFTP recovery work as on the other OpenIPC families.

## Status

These images have run only on qemu-hisilicon's Fullhan machines, not yet on
a camera. Ethernet takes an external RMII PHY when one answers (V200 and
FH865x try it before the SoC's own PHY), and the vendor ramboots and DDR
scripts are carried as the cameras and SDKs have them.

CI: `.github/workflows/ci.yml` calls `build-fullhan.yml` on the `openipc`
branch, which builds every SoC and boots each image twice under QEMU.
