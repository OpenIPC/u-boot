# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this branch is

`openipc-allwinner` is OpenIPC's U-Boot for the Allwinner V831 and V833
(sun8iw19p1): one Cortex-A7 with the H6's peripherals, clocks and memory
map, and the DDR in the package (V831: 64 MiB DDR2, V833: 128 MiB DDR3).
The tree is **mainline U-Boot v2022.10**, the same release as the other
`openipc-*` branches, plus:

- the OpenIPC boot layer, carried over from `openipc-novatek`
  (`common/openipc.c`, `include/openipc.h`, `include/configs/openipc-common.h`);
- the V831/V833 support, as a new mainline `ARCH_SUNXI` SoC,
  `MACH_SUN8I_V831`, with its own SPL: no vendor boot0, OP-TEE or
  boot package is needed or carried.

The DRAM initialisation is the vendor boot0's DRAM library (`libdram`,
shipped only as a binary object), translated to C function for function
(`arch/arm/mach-sunxi/dram_sun8i_v831.c`). It is the D1/T113 controller of
mainline's `drivers/ram/sunxi/dram_sun20i_d1.c`, in an older revision of
the library, at other addresses.

CI and user docs live on the `openipc` branch:

- `.github/workflows/ci.yml` only calls `build-allwinner.yml@openipc`.
- `git show origin/openipc:CLAUDE.md` covers the workflow invariants.
- `git show origin/openipc-hisilicon:CLAUDE.md` covers the OpenIPC layer
  (default environment, `common/openipc.c`, env compatibility). It applies
  here unchanged.

Adding or dropping a SoC needs both branches:

- here: `build.sh` (`build_soc`, `ALL_SOCS`) and `openipc/<soc>_config`;
- there: `build-allwinner.yml`'s matrices and the README.

## Relationship to mainline

```sh
git diff --stat v2022.10 HEAD                  # the whole delta
git diff --diff-filter=M v2022.10 HEAD         # the mainline files we patched
```

**Added**

| Area | Location |
| --- | --- |
| DRAM controller (SPL) | `arch/arm/mach-sunxi/dram_sun8i_v831.c`, `arch/arm/include/asm/arch-sunxi/dram_sun8i_v831.h` |
| PSCI monitor: version, features, SYSTEM_RESET (watchdog), CPU_OFF | `arch/arm/cpu/armv7/sunxi/psci_v831.c` |
| U-Boot's device tree | `arch/arm/dts/sun8i-v831.dtsi`, `sun8i-v831-openipc.dts` |
| Board: PHY clock, the kernel's device tree | `board/sunxi/v83x.c`, `v83x_kernel_dtb.S`, `v83x-kernel.dts` |
| Config header | `include/configs/v83x.h` (sunxi-common.h, then `openipc-common.h`) |
| Kconfig fragments, the build | `openipc/`, `build.sh` |

**Patched mainline files**, each small:

- `arch/arm/Kconfig`: `ARCH_SUNXI` selects `BINMAN` except on the V831
  (binman needs pylibfdt, which no longer builds with current setuptools;
  `build.sh` puts the image together).
- `arch/arm/mach-sunxi/Kconfig`: `MACH_SUN8I_V831` (selects
  `SUN50I_GEN_H6`), `DRAM_SUN8I_V831` and the `DRAM_SUN8I_V831_*`
  parameters, SoC defaults (DRAM clock and ZQ, CPU 816 MHz, SPL stack in
  DDR, `SYS_CONFIG_NAME` "v83x"); `SUN50I_GEN_H6` selects FIT only on arm64.
- `arch/arm/mach-sunxi/{board.c,clock_sun50i_h6.c,cpu_info.c,spl_spi_sunxi.c,Makefile}`,
  `arch/arm/include/asm/arch-sunxi/{clock_sun50i_h6.h,dram.h,gpio.h}`:
  UART0 on PH9/PH10, the boot0 clock set-up, the H6's SPI0 on the H3's
  pins (PC0-PC3), the boot device, an LZMA U-Boot from SPI.
- `arch/arm/cpu/armv7/sunxi/Makefile`: `psci_v831.o` instead of `psci.o`.
- `arch/arm/dts/Makefile`, `drivers/clk/sunxi/{Kconfig,clk_sunxi.c}`
  (the V831 CCU is the H6's for every gate U-Boot uses),
  `drivers/pinctrl/sunxi/{Kconfig,pinctrl-sunxi.c}` (the V831's functions).
- `board/sunxi/{board.c,Makefile}`: the V831 hooks; `misc_init_r()` →
  `openipc_helper()`; `env_get_location()` falls back to the SPI flash.
- `boot/image-fdt.c`: with `MACH_SUN8I_V831`, a plain uImage boots with
  the device tree at `${fdtaddr}` (as on `openipc-novatek`).
- `common/spl/Kconfig`: the SPL BSS inside 64 MiB.
- `common/spl/spl_mmc.c`: an LZMA legacy image from the card is read
  whole and unpacked (the raw path read it to the load address as it is).
- `lib/lzma/LzmaTools.c`: `CONFIG_IS_ENABLED(LZMA)`, so SPL_LZMA links
  (fixed the same way in later mainline).
- `common/{Kconfig,Makefile}`: `OPENIPC`, `PRODUCT_SOC`, `PRODUCT_SOCMODEL`.

## Boot

The boot ROM reads the eGON.BT0 header at SPI NOR offset 0 and loads the
SPL (`spl/sunxi-spl.bin`, `mkimage -T sunxi_egon`) into SRAM A1 at
`0x20000`. The SPL:

1. sets the clocks as boot0 does (`clock_init_safe()`: PLL_PERI0 1.2 GHz,
   AHB 200 / APB1 100 MHz, MBUS reset released; the MBUS clock is a
   quarter of the DRAM clock, it has no mux);
2. brings up the DDR (`sunxi_dram_init()`), then the CPU at 816 MHz (the
   vendor U-Boot's `boot_clock`);
3. reads U-Boot from offset `0x8000` with the H6's SPI0 (PIO, 6 MHz): an
   LZMA-compressed legacy image (`build.sh`), unpacked to `0x42000000`.

U-Boot does not fit the 256 KiB boot partition uncompressed (~257 KiB);
packed, the whole image is ~170 KiB.

The NOR images do not rely on the boot ROM's boot-media byte in the
eGON header: their SPL has no MMC loader, so it takes any valid header as
an SPI boot (`sunxi_get_boot_device()`).

| Offset | Contents |
| --- | --- |
| `0x00000` | SPL (eGON.BT0), padded to 32 KiB |
| `0x08000` | U-Boot, LZMA legacy image |
| `0x40000` | the environment (not in the image) |
| `0x50000` | kernel (OpenIPC NOR layout) |

### SD card boot (`u-boot-<soc>-mmc.bin`)

The boot ROM tries the SD card (SMHC0) before the SPI NOR: an eGON.BT0 at
sector 16 (8 KiB) of the card wins. The `-mmc` images
(`openipc/mmc_config`: `SPL_MMC`) have the NOR images' layout and are
written to the card at 8 KiB (`dd ... bs=1k seek=8`), before the first
partition at 1 MiB; the SPL loads U-Boot 32 KiB after itself (sunxi's
`U_BOOT_SECTOR` 0x40, counted from the SPL). U-Boot is the NOR image's.

- This is the unbricking path: a camera with a dead or blank NOR boots
  from the card. `v83x_boot_source()` then sets `source=fatload mmc 0`
  (as `srcmmc` does) for that boot, not saved, so `ub`, `ur`, `uk` and
  `fwupd` read `u-boot-<soc>-nor.bin`, `rootfs.squashfs.<soc>` and
  `uImage.<soc>` from the card's FAT partition and write them to the NOR.
  The next boot without the card is from the NOR.
- The environment stays in the SPI NOR: `env_get_location()` falls back
  to it when booted from MMC (sunxi's picks FAT/MMC there, neither built
  in, and `env_init()` failed with -ENODEV before the console).
- The SPL's MMC loader read legacy images raw; with `SPL_LZMA` it now
  reads a compressed one whole and unpacks it (`common/spl/spl_mmc.c`).
- The SPL trusts the boot ROM's boot-media byte here (the NOR images do
  not: they take any valid header as an SPI boot). The H6's ROM writes it;
  that the V831's does is assumed, not seen. qemu-bench's ROM stand-in
  writes it.

## DRAM

`dram_sun8i_v831.c` is the vendor library, not a rewrite: the function
names are the library's, every register write is the library's, in its
order. It was checked against the vendor boot0 under QEMU: after the
vendor's boot0 (stopped at OP-TEE's entry) and after this SPL (stopped
right after `sunxi_dram_init()`), every MC_COM (`0x04002000`), MCTL/PHY
(`0x04003000`), PLL_DDR, MBUS, DRAM clock and PRCM ZQ register is equal,
for the V831 parameters and for the V833's (DDR3, rank/width/size scan,
the DDR3 AC remapping). QEMU's DRAM controller completes every training
step at once, so this proves the sequence, not the timing on silicon.

- The parameters are the vendor's `dram_para` words
  (`CONFIG_DRAM_SUN8I_V831_*`, from `sys_config.fex` `[dram_para]`):
  `openipc/v831_config` the V831 reference boards' (every V831 flash seen
  has the same), `openipc/v833_config` the V833 lindenis board's.
- Left out: the chip binding check (`bond_id_check()`), the PMIC DRAM
  supply (`dram_vol_set()`), the super standby resume path.
- `sunxi_dram_init()` checks the size the library computed with
  `get_ram_size()`. Under QEMU, which does not model the address mapping,
  the library's scan reports 4160 MiB, as the vendor's boot0 does there.

## Hand-off to the kernel

- OpenIPC's V83x kernels (Linux 4.9, `openipc/linux` `allwinner-v83x`)
  carry no device tree; the vendor U-Boot gives them its own.
  `board/sunxi/v83x-kernel.dts` is that tree: the Tina V83x SDK's
  reference board over the kernel's `sun8iw19p1.dtsi`, flattened, with
  what the vendor U-Boot changes in it before booting from SPI NOR (traced
  under QEMU from a camera's flash: SPI0 enabled; OP-TEE's memreserve
  dropped, there is no OP-TEE). It is gzipped into U-Boot, unpacked at
  `misc_init_r()` and booted from `${fdtaddr}`; `ft_board_setup()` sets
  `/dram/dram_para2` (the size, as the vendor does), bootm `/memory` and
  `/chosen`.
- The kernel runs in the normal world, entered in HYP mode
  (`ARMV7_NONSEC`), with PSCI 1.0 from U-Boot's monitor
  (`psci_v831.c`), as it expects from OP-TEE. The GIC is named by address
  (`CONFIG_ARM_GIC_BASE_ADDRESS`): QEMU's Cortex-A7 reads CBAR as 0.
- `mem=` is the whole DDR. The MTD name is `spi0.0`, the console `ttyS0`.
- The YI camera's own kernel (Linux 4.9 of a later Tina revision) wants
  `allwinner,sun50i-spi` and has no `cmdlinepart`: vendor kernels other
  than OpenIPC's may need their own tree.

## Peripherals

- **Console:** UART0 on PH9/PH10 (function 5), 115200, 24 MHz.
- **SPI NOR:** SPI0 (`spi-sunxi`, the H3 variant; 64-byte FIFO), PC0-PC3
  function 4; DM SPI flash, the environment at 0x40000.
- **SD:** SMHC0 on PF0-PF5, 4-bit (`sunxi_mmc`, the H6's new timing mode).
- **Ethernet:** the EMAC (`sun8i_emac`, A64 variant: no internal PHY),
  RMII on PD (function 5), the PHY found by scanning the MDIO bus, reset
  on PD19, its 25 MHz from the SoC (`EPHY_25M`, CCU `0x970`). The MAC
  comes from the SID, as on every sunxi board.
- **Reset:** the watchdog behind the timers (`0x030090A0`), from U-Boot
  and from the kernel's PSCI SYSTEM_RESET.

## Build

```sh
bash build.sh               # v831 v833; continues past failures, exits non-zero if any failed
bash build.sh v831          # one SoC (CROSS_COMPILE defaults to arm-linux-gnueabi-)
```

- The output is `output/u-boot-<soc>-nor.bin` and `-mmc.bin`, built out
  of tree in `output/build-<soc>-{nor,mmc}/`. It needs `xz` (for the LZMA
  image).
- `.config` is `openipc/openipc_config`, then `openipc/<soc>_config`
  (and for `-mmc`, `openipc/mmc_config`), then `olddefconfig`. There are
  no `configs/*_defconfig` for these targets.
- `build.sh` fails a build whose image is over the 256 KiB boot partition
  or whose SPL is over 32 KiB.

## Verification

The emulator is qemu-bench (OpenIPC/qemu-bench, formerly qemu-hisilicon),
machines `v831` and `v833` (`docs/allwinner-v83x.md` there).
`-M <soc>,flash-file=<image>` does the boot ROM's job; booting from an SD
card (`-drive if=sd`) needs qemu-bench #204.

- **Smoke (what CI will run):** two boots of an 8 MiB image holding only
  U-Boot (`-mmc`: an erased NOR and a card with the image at 8 KiB); the
  second must load the environment the first saved. Both SoCs, both
  kinds pass (V831 64 MiB, V833 128 MiB).
  - SD unbricking: an erased NOR and a card with the `-mmc` image and
    the three files on FAT: `run ub`, `run fwupd`, then a boot without
    the card mounts the root from the NOR.
- **Checks run by hand** (all passed under qemu-bench):
  - The DRAM register comparison with the vendor boot0 above (V831 and
    V833 parameters).
  - An 8 MiB image with an OpenIPC `allwinner-v83x` kernel at 0x50000
    (`v83x.generic.config`, OpenIPC toolchain) and a SquashFS root at
    0x250000 reaches a shell: PSCIv1.0, HYP, six `cmdlinepart`
    partitions on `spi0.0`, root on `mtdblock3`; its `sunxi-gmac` pings
    the host; `reboot -f` comes back through the SPL.
  - U-Boot: `ping` and `tftpboot` (`-nic user,...`), `mmc info` and
    `fatls` on a card (`-drive if=sd,...`), `saveenv` + `reset`, the boot
    limit (two failed boots, then `altbootcmd` with `failsafe`).
- **What QEMU does not check:** DRAM timing and training on silicon, the
  PLL rates and so the UART/SPI/MMC dividers, pad drive, the PHY reset and
  clock, the SPI controller's real timing. Not yet run on a camera.
