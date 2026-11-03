![OpenIPC logo][logo]

# OpenIPC U-Boot

- [Supported vendors](#supported-vendors)
  - [HiSilicon and Goke](#hisilicon-and-goke)
  - [Xiongmai](#xiongmai)
  - [Goke GK710x](#goke-gk710x)
  - [Anyka](#anyka)
  - [Novatek](#novatek)
  - [Allwinner](#allwinner)
  - [Fullhan](#fullhan)
  - [Grain Media](#grain-media)
  - [SigmaStar](#sigmastar)
  - [Ingenic](#ingenic)
- [Images](#images)
- [Flash layout](#flash-layout)
- [What happens at boot](#what-happens-at-boot)
- [Commands](#commands)
- [Recovery](#recovery)
- [Building](#building)
- [Branches and CI](#branches-and-ci)

## Supported vendors

| Vendor | Source branch | Releases | CI |
| --- | --- | --- | --- |
| [HiSilicon and Goke](#hisilicon-and-goke) | `openipc-hisilicon` | [hisilicon](../../releases/tag/hisilicon), [goke](../../releases/tag/goke) | [![hisilicon](https://github.com/OpenIPC/u-boot/actions/workflows/build-hisilicon.yml/badge.svg)](https://github.com/OpenIPC/u-boot/actions/workflows/build-hisilicon.yml) |
| [Xiongmai](#xiongmai) | `openipc-xm` | [xiongmai](../../releases/tag/xiongmai) | [![xm](https://github.com/OpenIPC/u-boot/actions/workflows/build-xm.yml/badge.svg)](https://github.com/OpenIPC/u-boot/actions/workflows/build-xm.yml) |
| [Goke GK710x](#goke-gk710x) | `openipc-gk710x` | [goke](../../releases/tag/goke) | [![gk710x](https://github.com/OpenIPC/u-boot/actions/workflows/build-gk710x.yml/badge.svg)](https://github.com/OpenIPC/u-boot/actions/workflows/build-gk710x.yml) |
| [Anyka](#anyka) | `openipc-anyka` | [anyka](../../releases/tag/anyka) | [![anyka](https://github.com/OpenIPC/u-boot/actions/workflows/build-anyka.yml/badge.svg)](https://github.com/OpenIPC/u-boot/actions/workflows/build-anyka.yml) |
| [Novatek](#novatek) | `openipc-novatek` | [novatek](../../releases/tag/novatek) | [![novatek](https://github.com/OpenIPC/u-boot/actions/workflows/build-novatek.yml/badge.svg)](https://github.com/OpenIPC/u-boot/actions/workflows/build-novatek.yml) |
| [Allwinner](#allwinner) | `openipc-allwinner` | [allwinner](../../releases/tag/allwinner) | [![allwinner](https://github.com/OpenIPC/u-boot/actions/workflows/build-allwinner.yml/badge.svg)](https://github.com/OpenIPC/u-boot/actions/workflows/build-allwinner.yml) |
| [Fullhan](#fullhan) | `openipc-fullhan` | [fullhan](../../releases/tag/fullhan) | [![fullhan](https://github.com/OpenIPC/u-boot/actions/workflows/build-fullhan.yml/badge.svg)](https://github.com/OpenIPC/u-boot/actions/workflows/build-fullhan.yml) |
| [Grain Media](#grain-media) | `openipc-grainmedia` | [grainmedia](../../releases/tag/grainmedia) | [![grainmedia](https://github.com/OpenIPC/u-boot/actions/workflows/build-grainmedia.yml/badge.svg)](https://github.com/OpenIPC/u-boot/actions/workflows/build-grainmedia.yml) |
| [SigmaStar](#sigmastar) | `openipc-sigmastar` | [sigmastar](../../releases/tag/sigmastar) | [![sigmastar](https://github.com/OpenIPC/u-boot/actions/workflows/build-sigmastar.yml/badge.svg)](https://github.com/OpenIPC/u-boot/actions/workflows/build-sigmastar.yml) |
| [Ingenic](#ingenic) | `openipc-ingenic` | [ingenic](../../releases/tag/ingenic) | [![ingenic](https://github.com/OpenIPC/u-boot/actions/workflows/build-ingenic.yml/badge.svg)](https://github.com/OpenIPC/u-boot/actions/workflows/build-ingenic.yml) |

### HiSilicon and Goke

| SoC | CPU | RAM | Flash | Notes |
| --- | --- | --- | --- | --- |
| `hi3516cv500` `hi3516dv300` `hi3516av300` | Cortex-A7 ×2 | external | NOR, NAND | |
| `hi3516ev200` `hi3518ev300` | Cortex-A7 | 64 MiB | NOR, NAND | |
| `hi3516ev300` | Cortex-A7 | 128 MiB | NOR, NAND | |
| `hi3516dv200` | Cortex-A7 | — | NOR, NAND | |
| `gk7205v200` `gk7202v300` | Cortex-A7 | 64 MiB | NOR, NAND | Goke twins of the hi3516ev200 family |
| `gk7205v300` | Cortex-A7 | 128 MiB | NOR, NAND | Goke twin of the hi3516ev300 |
| `gk7201v200` `gk7201v300` `gk7605v100` | Cortex-A7 | — | NOR, NAND | Goke, hi3516ev200 family |
| `gk7205v500` `gk7205v510` `gk7205v530` | Cortex-A7 | — | NOR, NAND | |
| `gk7206v11a` | Cortex-A7 ×2 | — | NOR, NAND | **held**: built, not released until booted on hardware |
| `gk7206v12a` | Cortex-A7 ×2 | 256 MiB | NOR, NAND | **held**: built, not released until booted on hardware |
| `hi3516cv610` `hi3516cv608` | Cortex-A7 ×2 | 64 MiB | NOR | signed boot image, `boot-*.bin` |
| `hi3519dv500` | Cortex-A55 (arm64) | external | NOR | signed boot image, 512 KiB boot partition |
| `hi3516cv300` | ARM926 | external | NOR, NAND | |
| `hi3516ev100` | ARM926 | — | NOR | |
| `hi3518ev200` | ARM926 | 64 MiB | NOR | |
| `hi3516cv200` | ARM926 | external | NOR | |
| `hi3518ev100` | ARM926 | — | NOR | |
| `hi3516cv100` `hi3518av100` `hi3518cv100` | ARM926 | external | NOR | |
| `hi3516av100` `hi3516dv100` | Cortex-A7 | external | NOR | |
| `hi3516av200` `hi3519v101` | Cortex-A17 + A7 | external | NOR | |
| `hi3515av100` `hi3520av100` `hi3520dv100` `hi3520dv200` | ARMv7 | external | NOR | DVR/NVR |
| `hi3536cv100` `hi3536dv100` | ARMv7 | external | NOR | NVR |

- hi3519dv500: the boot partition is 512 KiB, so env is at `0x80000` and
  firmware at `0xc0000`. Its images are
  `boot-hi3519dv500-{dmeb,dmebpro}-nor.bin`. Building it also needs
  `aarch64-linux-gnu-` (`CROSS_COMPILE64`).
- Some SoCs can also recover from a USB stick.
- Publishing replaces the same-named images that OpenIPC/u-boot-xmedia,
  u-boot-hi3516cv6xx and u-boot-hi3519dv500 used to put in
  OpenIPC/firmware.

### Xiongmai

| SoC | CPU | RAM | Flash | Notes |
| --- | --- | --- | --- | --- |
| `xm510` | Cortex-A5 | 32/64 MiB DDR2 | NOR | |
| `xm530` | Cortex-A5 ×2 | 64 MiB DDR2 | NOR | |
| `xm550` | Cortex-A5 ×2 | 128 MiB DDR3 | NOR | XM530 die |
| `xm530v200` | Cortex-A5 ×2 | 128 MiB DDR3 | NOR | |
| `xm533` | Cortex-A5 | 64 MiB DDR2 | NOR | |
| `xm535d1` `xm535w1` | Cortex-A5 | 64 MiB DDR2 | NOR | |
| `xm535d2` | Cortex-A5 | 128 MiB DDR3 | NOR | |

- The boot ROM runs U-Boot in place from SPI NOR, and U-Boot brings up
  DDR itself, so the image is `u-boot.bin` as linked.
- Xiongmai bootloaders leave per-block write protection on, under which
  their `sf erase`/`sf write` report success and change nothing. This
  U-Boot clears it.

### Goke GK710x

| SoC | CPU | RAM | Flash | Notes |
| --- | --- | --- | --- | --- |
| `gk7101` | ARM1176 | 128 MiB | NOR | |
| `gk7102` | ARM1176 | 64 MiB | NOR | |
| `gk7101s` | ARM1176 | 128 MiB | NOR | |
| `gk7102s` | ARM1176 | 64 MiB | NOR | also the image for **GK7102C** cameras |

- These are Goke's own chips, not the HiSilicon-derived GK7205 above.
- GK7102C cameras have no Ethernet: recover them from an SD card.
- **Programmer images.** The flash controller swaps the bytes of every
  32-bit word, so the chip stores everything (U-Boot, env, kernel,
  rootfs) word-reversed. Files written through U-Boot (`sf write`,
  `run ub`, SD recovery) are fine as they are. For an external
  programmer, lay the files out at their offsets first, then reverse
  every word of the whole image:

```sh
tr '\0' '\377' < /dev/zero | head -c 8M > full.bin
dd if=u-boot-gk7102-nor.bin  of=full.bin conv=notrunc
dd if=uImage.gk7102          of=full.bin conv=notrunc bs=64k seek=5
dd if=rootfs.squashfs.gk7102 of=full.bin conv=notrunc bs=64k seek=37
objcopy -I binary -O binary --reverse-bytes=4 full.bin full-programmer.bin
```

The same `objcopy` turns a programmer dump back into normal order. A
programmer image written through U-Boot by mistake is recognised and
swapped back.

- The vendor kernel's GPIO map defaults to the reference boards' layout;
  `setenv gpiomap v1` picks the other one (rb-ov9712, ar0130, jxh42 v1.0,
  GK7101 imx222).

### Anyka

| SoC | CPU | RAM | Flash | Notes |
| --- | --- | --- | --- | --- |
| `ak3918ev200` | ARM926 | 64 MiB | NOR | |
| `ak3916ev300` | ARM926 | 64 MiB | NOR | also runs the AK3916EV301 |
| `ak3918ev300` | ARM926 | 64 MiB | NOR | |
| `ak3919ev300` | ARM926 | 128 MiB | NOR | |
| `ak3918ev330` | ARM926 | 64 MiB | NOR | vendor first stage leads the image |
| `ak3919ev330` | ARM926 | 128 MiB | NOR | vendor first stage leads the image |

- The images for one generation differ only in RAM size and `soc`.
- Vendor uImages have entry = load + 0x40, which this U-Boot does not
  run. Repack them with `mkimage -e <load address>`.
- SD recovery takes only these file names: `uImage.<soc>.img` (or
  `fitImage.<soc>.img`) and `rootfs.squashfs.<soc>.img`.

### Novatek

| SoC | CPU | RAM | Flash | Notes |
| --- | --- | --- | --- | --- |
| `nt98562` | Cortex-A9 | 64 MiB | NOR | NA51089 |
| `nt98566` | Cortex-A9 | 128 MiB | NOR | NA51089 |

- The image contains Novatek's loader, built from SDK source. It sits at
  offset 0, the device tree at `0x10000` and U-Boot (LZMA) at `0x20000`.
- OpenIPC kernels carry no device tree: U-Boot passes them the loader's.
- Building needs `arm-none-eabi-` for the loader (`LOADER_CROSS_COMPILE`).
- SD recovery takes only these file names: `uImage.<soc>.img` (or
  `fitImage.<soc>.img`) and `rootfs.squashfs.<soc>.img`.

### Allwinner

| SoC | CPU | RAM | Flash | Notes |
| --- | --- | --- | --- | --- |
| `v3s` | Cortex-A7 | 64 MiB DDR2 | NOR, SD | |
| `v831` | Cortex-A7 | 64 MiB DDR2 | NOR, SD | |
| `v833` | Cortex-A7 | 128 MiB DDR3 | NOR, SD | |
| `v851s` | Cortex-A7 | 64 MiB DDR2 | NOR, SD | OpenIPC/firmware still ships u-boot-allwinner's camera-tested image |

- Mainline sunxi SPL, with no vendor boot0, OP-TEE or boot package.
- **Booting from an SD card.** The boot ROM tries the card before the
  flash, so a camera with a broken or empty flash boots from a card that
  has `u-boot-<soc>-mmc.bin` at 8 KiB:

```sh
dd if=u-boot-v831-mmc.bin of=/dev/sdX bs=1k seek=8 conv=fsync
```

When it boots that way, `source` is the card's FAT partition: `run ub`
writes `u-boot-<soc>-nor.bin` to the flash, and `run fwupd` writes the
firmware. Remove the card and the camera boots from flash.

- V831/V833 OpenIPC kernels carry no device tree. U-Boot passes them the
  vendor's, which is built in. V3s/V851s kernels carry their own.
- Building needs `xz`.
- SD recovery takes only these file names: `uImage.<soc>.img` (or
  `fitImage.<soc>.img`) and `rootfs.squashfs.<soc>.img`.

### Fullhan

| SoC | CPU | RAM | Flash | Notes |
| --- | --- | --- | --- | --- |
| `fh8852v200` `fh8852v210` | ARM1176 | 64 MiB | NOR | V210 is a name of the same die |
| `fh8856v200` `fh8856v210` | ARM1176 | 128 MiB | NOR | |
| `fh8858v200` `fh8858v210` | ARM1176 | up to 256 MiB | NOR | |
| `fh8652` | Cortex-A7 | 64 MiB | NOR | vendor ramboot in the image |
| `fh8656` | Cortex-A7 | 128 MiB | NOR | vendor ramboot in the image |
| `fh8658` | Cortex-A7 | up to 256 MiB | NOR | vendor ramboot in the image |
| `fh8852v100` | ARM1176 | 64 MiB | NOR | vendor ramboot in the image |
| `fh8856v100` | ARM1176 | 128 MiB | NOR | vendor ramboot in the image |
| `fh8626v100` | ARM1176 | 64 MiB | NOR | vendor ramboot in the image |
| `fh8833v100` | ARM1176 | 64 MiB | NOR | the FH8632 |

- The image holds the flash header, the vendor's DDR scripts, the ramboot
  where there is one, and U-Boot.

### Grain Media

| SoC | CPU | RAM | Flash | Notes |
| --- | --- | --- | --- | --- |
| `gm8136` | FA626TE (ARMv5) | 128 MiB | NOR | GM8136S; the same image runs GM8136 with external DDR |
| `gm8135` | FA626TE (ARMv5) | 64 MiB | NOR | GM8135S |

- The image holds the SDK's first stage, nsboot, built from source. One
  build serves every package because nsboot picks the DDR table by the
  chip's package ID.

### SigmaStar

| SoC | CPU | RAM | Flash | Notes |
| --- | --- | --- | --- | --- |
| `msc313e` | Cortex-A7 | 64 MiB | NOR | infinity3 |
| `msc316dc` `msc316dm` | Cortex-A7 | 128 MiB | NOR | infinity3 |

- The vendor boot chain is kept. The 256 KiB image holds the MXP
  partition table at `0x3000`, the vendor IPL at `0x4000`, IPL_CUST at
  `0x10000`, and U-Boot (xz uImage) at `0x20000`.
- Vendor kernels are xz-compressed uImages that only the vendor U-Boot
  unpacks.
- SD recovery takes only these file names: `uImage.<soc>.img` (or
  `fitImage.<soc>.img`) and `rootfs.squashfs.<soc>.img`.

### Ingenic

| SoC | CPU | RAM | Flash | Notes |
| --- | --- | --- | --- | --- |
| `t10` `t10l` | XBurst | 64 MiB DDR2 | NOR | |
| `t20` `t20l` | XBurst | 64 MiB DDR2 | NOR | |
| `t20x` | XBurst | 128 MiB DDR2 | NOR | |
| `t21n` | XBurst | 64 MiB DDR2 | NOR | |
| `t23n` `t23zn` | XBurst | 64 MiB DDR2 | NOR | |
| `t23x` | XBurst | 128 MiB DDR2 | NOR | |
| `t30n` `t30l` | XBurst | 64 MiB DDR2 | NOR | |
| `t30x` | XBurst | 128 MiB DDR2 | NOR | |
| `t30a` `t30a1` | XBurst | 128 MiB DDR3 | NOR | |
| `t31n` `t31l` `t31lc` | XBurst | 64 MiB DDR2 | NOR | |
| `t31x` | XBurst | 128 MiB DDR2 | NOR | |
| `t31a` `t31al` | XBurst | 128 MiB DDR3 | NOR | |
| `t40n` | XBurst2 ×2 | 128 MiB DDR2 | NOR, NAND | |
| `t40xp` | XBurst2 ×2 | 256 MiB DDR3 | NOR, NAND | |
| `t40a` | XBurst2 ×2 | 1 GiB DDR3 | NOR, NAND | |
| `t41l` `t41lq` | XBurst2 ×2 | 64 MiB DDR2 | NOR | |
| `t41n` `t41nq` `t41a` | XBurst2 ×2 | 128 MiB DDR3 | NOR | |
| `t41zx` `t41xq` | XBurst2 ×2 | 256 MiB DDR3 | NOR | |

- The images are not published to OpenIPC/firmware, where cameras keep
  using [OpenIPC/u-boot-ingenic](https://github.com/OpenIPC/u-boot-ingenic)'s.
  Its target names are kept here; `l` parts run at lower clocks.
- The image is the vendor SPL, built from source, followed by U-Boot
  (lzop). On NOR, U-Boot starts at `0x6800`; on NAND, at `0x6c00`.
- Building needs `mipsel-linux-gnu-` and `lzop`. For `.img` recovery
  files, use `mkimage -A mips`.

## Images

| File | What it is |
| --- | --- |
| `u-boot-<soc>-nor.bin` | the whole boot partition for SPI NOR |
| `u-boot-<soc>-nand.bin` | the same for SPI NAND |
| `u-boot-<soc>-mmc.bin` | Allwinner only: boots from an SD card |
| `boot-<soc>-*.bin` | hi3516cv6xx, hi3519dv500: signed boot image |

Each image already holds whatever the boot ROM needs in front of U-Boot
(DDR scripts, a first stage, a vendor loader). Write it at offset 0 as it
is. From a running U-Boot, either this one or a vendor one, with a TFTP
server:

```sh
setenv serverip 192.168.1.107
tftpboot 0x42000000 u-boot-hi3516ev300-nor.bin   # a free RAM address on your SoC
sf probe 0; sf erase 0 0x40000; sf write 0x42000000 0 ${filesize}
```

Once this U-Boot runs, `run ub` does the same.

## Flash layout

### SPI NOR

| Offset | Size | Partition |
| --- | --- | --- |
| `0x000000` | 256 KiB | `boot` (U-Boot) |
| `0x040000` | 64 KiB | `env` |
| `0x050000` | 2 MiB (lite) or 3 MiB (ultimate) | `kernel` |
| `0x250000` / `0x350000` | 5 MiB (lite) or 10 MiB (ultimate) | `rootfs` (squashfs) |
| after rootfs | rest of the chip | `rootfs_data` |

The rootfs you flash picks the layout: anything over 5 MiB means
ultimate. On some boards the kernel and rootfs sit back to back in one
`firmware` partition instead (the *split* layout, `firmware.bin.<soc>`),
if the kernel supports it. U-Boot detects which layout is on the flash.

### SPI NAND

| Offset | Size | Partition |
| --- | --- | --- |
| `0x000000` | 768 KiB | `boot` |
| `0x0c0000` | 256 KiB | `env` |
| `0x100000` | rest | `ubi`: volumes `rootfs` (UBIFS, kernel at `/boot/uImage` or `/boot/fitImage`) and `rootfs_data` |

OpenIPC firmware, sysupgrade and openipc.org rely on these layouts, and
they do not change.

## What happens at boot

1. **Recovery check.** If there is an SD card or USB stick, U-Boot looks
   for [recovery files](#sd-card-or-usb-stick) on it and flashes them. It
   skips files that are already on the flash, so a card left in the slot
   does nothing.
2. **First start.** If the env partition is blank, U-Boot saves the
   default environment. It never overwrites an env area that holds
   something it can't read.
3. **Firmware scan.** U-Boot finds the kernel and rootfs on the flash and
   sets `kernaddr`, `kernsize` and `mtdparts` to match. This lets it boot
   either layout, and firmware flashed by older OpenIPC bootloaders. If it
   finds nothing bootable, it sets `bootfail`.
4. **Autoboot** starts after 1 second unless you press **Ctrl-C**.
   `bootcmd` is `run bootflash`. If `bootfail` is set, it runs `fwupd`
   from the current `source` (TFTP by default) and resets. Otherwise it
   loads the kernel and boots it.
5. **Boot loop guard.** A counter in RAM counts boots that never reached
   a healthy userspace. After `bootlimit` (2) such boots, `altbootcmd`
   boots the same kernel with `failsafe` on the command line. OpenIPC's
   failsafe mode brings up only network and SSH, so the camera stays
   reachable.

Cameras upgraded from an older OpenIPC bootloader keep their saved
environment, and it still works.

## Commands

Each update command downloads a file from `source` into RAM and writes
it to the right place:

| Command | File | Writes |
| --- | --- | --- |
| `run ub` | `u-boot-<soc>-nor.bin` / `-nand.bin` | the boot partition |
| `run uk` | `uImage.<soc>` (or `fitImage.<soc>`) | the kernel slot (NOR) |
| `run ur` | `rootfs.squashfs.<soc>` / `rootfs.ubifs.<soc>` | the rootfs slot, picking lite or ultimate; on NAND, the `rootfs` UBI volume |
| `run fwupd` | `firmware.bin.<soc>` if present, else the rootfs then the kernel | the whole firmware |
| `run bootnfs` | `uImage.<soc>` | nothing: boots the kernel with its root on NFS (`${serverip}:${nfsroot}`) |

These commands pick where the files come from. The choice lasts until
reset, or for good after `saveenv`:

| Command | Source |
| --- | --- |
| `run srcnet` | TFTP from `serverip` (default) |
| `run srcmmc` | FAT on the SD card |
| `run srcusb` | FAT on a USB stick |

| Variable | Default | Meaning |
| --- | --- | --- |
| `ipaddr` / `serverip` | `192.168.1.10` / `192.168.1.107` | camera and TFTP server addresses |
| `soc` | the image's SoC | used in every file name above |
| `osmem` | depends on the SoC | RAM given to Linux (`mem=`); the rest goes to the media drivers |
| `extras` | empty | extra kernel arguments, added to `bootargs` |
| `bootlimit` | `2` | boot-loop threshold for failsafe |
| `totalmem` | detected | DDR size U-Boot found (read only) |

Other defaults: autoboot stops on Ctrl-C only, `setenv silent 1`
silences the console, and `echo` is available.

## Recovery

### Over the network (TFTP)

Put the firmware files for your SoC on a TFTP server, press Ctrl-C to stop
autoboot, then:

```sh
setenv ipaddr 192.168.1.10
setenv serverip 192.168.1.107
run fwupd          # kernel + rootfs (or firmware.bin.<soc>)
reset
```

A camera with no bootable firmware on its flash does this by itself on
every boot (see step 4 above). So a server at `192.168.1.107` with the
right files unbricks it without a console.

### SD card or USB stick

On every boot, U-Boot reads the first FAT partition of the card, or the
whole card if it has no partition table. It acts on what it finds in the
root directory:

1. **`script.txt`**: if present, U-Boot runs it and does nothing else.
   It can be plain text with one U-Boot command per line, or a script
   wrapped with `mkimage -T script`. For example, `run srcmmc; run fwupd`
   flashes the firmware files on the card with the normal update
   commands.
2. **`*.img`**: each one is a uImage wrapper whose **load address is the
   flash offset** for its payload. U-Boot checks the header and data
   CRCs, then writes the payload (without the wrapper header) at that
   offset. This works for anything: U-Boot itself, a kernel, a rootfs, a
   whole flash dump.

Make an `.img` with `mkimage` (from `u-boot-tools`):

```sh
# kernel into the kernel slot
mkimage -A arm -T firmware -C none -a 0x50000  -d uImage.hi3516ev300          kernel.img
# rootfs into the lite rootfs slot
mkimage -A arm -T firmware -C none -a 0x250000 -d rootfs.squashfs.hi3516ev300  rootfs.img
# U-Boot itself
mkimage -A arm -T firmware -C none -a 0x0      -d u-boot-hi3516ev300-nor.bin   uboot.img
```

Copy the files to the card, insert it and power on. Remove the card when
the console shows `Recovery: wrote ...`.

On SPI NAND, recovery takes a single raw file with no wrapper,
`rootfs.ubifs.<soc>.img`, and writes it into the `rootfs` UBI volume.
`rootfs_data` is kept if the new rootfs fits. Some vendors accept only
fixed file names, as their sections say.

## Building

```sh
git clone --branch openipc-<vendor> https://github.com/OpenIPC/u-boot.git
cd u-boot
bash build.sh          # every SoC on the branch
bash build.sh <soc>    # one SoC, e.g. bash build.sh hi3516ev300
```

Images land in `output/`. The toolchain is `arm-linux-gnueabi-`
(Debian/Ubuntu `gcc-arm-linux-gnueabi`). Any extra tools a vendor needs
are listed in its section. The build fails if an image would overflow its
boot partition, because flashing it would overwrite the environment.

## Branches and CI

This `openipc` branch holds only this README and the CI workflows. The
bootloader sources are on the `openipc-<vendor>` branches listed
[above](#supported-vendors). `main` mirrors upstream U-Boot.

Each vendor has a `build-<vendor>.yml` workflow here, which its branch
calls on every push and pull request. It builds every SoC and uploads
the images. Run by hand, it can also publish:

```sh
gh workflow run build-<vendor>.yml                                   # build
gh workflow run build-<vendor>.yml -f release=true                   # + this repo's release
gh workflow run build-<vendor>.yml -f release=true -f publish=true   # + OpenIPC/firmware latest and openipc.org
```

`publish` needs a `FIRMWARE_RELEASE_TOKEN` secret. Ingenic has no
`publish`, and held SoCs are never released.

**Adding a vendor:** push the port on a new `openipc-<vendor>` branch,
add `build-<vendor>.yml` here, and add a row and a section to this
README.

## License

U-Boot is GPL-2.0+; see `Licenses/` and the SPDX tags on each vendor
branch.

## Support

- Questions and bug reports: [GitHub Issues](https://github.com/OpenIPC/u-boot/issues)
- Documentation: [openipc.org](https://openipc.org)

[![Telegram](https://openipc.org/images/telegram_button.svg)](https://openipc.org/our-channels)

[logo]: https://raw.githubusercontent.com/OpenIPC/.github/refs/heads/main/assets/openipc-logo-black.svg
