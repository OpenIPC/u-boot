# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this branch is

`openipc-sigmastar` is OpenIPC's U-Boot for SigmaStar's (MStar's)
Cortex-A7 camera SoCs of the infinity3 die: the MSC313E, MSC316DC and
MSC316DM. The tree is **mainline U-Boot v2022.10**, the same release as
`openipc-hisilicon`, `openipc-xm`, `openipc-gk710x` and `openipc-anyka`,
plus:

- the OpenIPC boot layer, carried over from `openipc-anyka`, which took it
  from `openipc-xm` and `openipc-hisilicon`;
- the infinity3 support, written from the vendor SDK
  `MSC3XX_SDK_V_1711291828-I3` (U-Boot 2015.01
  `arch/arm/cpu/armv7/infinity3` and `drivers/mstar`, kernel 3.18.30
  `drivers/mstar`).

No vendor driver comes along. The vendor's first boot stages, IPL and
IPL_CUST, are carried as binaries (see "Boot").

CI and user docs live on the `openipc` branch:

- `.github/workflows/ci.yml` only calls `build-sigmastar.yml@openipc`.
- `git show origin/openipc:CLAUDE.md` covers the workflow invariants.
- `git show origin/openipc-hisilicon:CLAUDE.md` covers the OpenIPC layer
  (default environment, `common/openipc.c`, env compatibility). It applies
  here unchanged.

Adding or dropping a SoC needs both branches:

- here: `build.sh` (`build_soc`, `ALL_SOCS`), `openipc/<soc>_config`, and
  `TARGET_*` in `arch/arm/mach-sigmastar/Kconfig`;
- there: `build-sigmastar.yml`'s matrices and the README.

The later SigmaStar dies (infinity2m, infinity6/6b0/6e: SSD20x, SSC33x)
have another boot chain and are not covered.

## Relationship to mainline

```sh
git diff --stat v2022.10 HEAD                  # the whole delta, small
git diff --diff-filter=M v2022.10 HEAD         # the mainline files we patched
```

**Added**

| Area | Location |
| --- | --- |
| SoC: timer, reset, DRAM size, MIU flush | `arch/arm/mach-sigmastar/` (`ARCH_SIGMASTAR`; `TARGET_MSC313E/MSC316DC/MSC316DM`, `SS_INFINITY3`; register map in `include/mach/sigmastar.h`) |
| Board: `misc_init_r` → `openipc_helper()`, `board_eth_init` | `board/sigmastar/infinity3/` |
| SPI NOR on the FSP, non-DM | `drivers/mtd/spi/ms_sflash.c` (`MS_SFLASH`). It provides `spi_flash_probe()` and fills `mtd->_read/_write/_erase`, as `ak_sflash` does on `openipc-anyka` |
| Ethernet, non-DM | `drivers/net/ms_emac.c` (`MS_EMAC`) |
| Memory map, console, timer | `include/configs/sigmastar.h`, ending in `#include "openipc-common.h"` |
| OpenIPC boot layer | `common/openipc.c`, `include/openipc.h` (as on `openipc-anyka`), `include/configs/openipc-common.h` (SigmaStar rewrite, NOR only, same env commands) |
| Kconfig fragments, vendor boot stages, the image packer, the build | `openipc/`, `build.sh` |

**Patched mainline files**, each small:

- `arch/arm/{Kconfig,Makefile}`: the `ARCH_SIGMASTAR` hook.
- `common/{Kconfig,Makefile}`: `OPENIPC`, `PRODUCT_SOC`, `PRODUCT_SOCMODEL`.
- `drivers/mtd/spi/{Kconfig,Makefile}`, `cmd/Kconfig` (`CMD_SF`) and
  `env/Kconfig` (`ENV_IS_IN_SPI_FLASH`): accept `MS_SFLASH`.
- `drivers/net/{Kconfig,Makefile}`, `include/netdev.h`: `MS_EMAC`;
  `NETDEVICES` selects `DM_ETH` only when `DM` is on, as on `openipc-xm`.
- `.gitattributes`: `openipc/*.bin` are binary (the tree's
  `* text eol=lf` would rewrite their CR LF pairs).

The console is mainline `ns16550`, non-DM.

## Boot

The vendor boot chain is kept; only the MXP partition `UBOOT` is ours.

- The mask ROM looks for the IPL at flash offset `0x4000` (the first
  16 KiB are zeros on every image seen). The **IPL** trains and sizes the
  DDR (it wraps past its end), runs a BIST, and loads **IPL_CUST** from
  the MXP partition table. IPL_CUST finds `UBOOT` in the table, reads
  the uImage there and unpacks it to `0x23E00000`, where the vendor
  U-Boot is linked (`SYS_TEXT_BASE`), whatever the header's load address
  (0, as the vendor's `create_img.sh` writes it).
- The uImage is the vendor's shape: OS U-Boot, type kernel, compression
  "lzma" holding **xz** data (CRC64), load and entry 0. Its name is not
  checked. `openipc/infinity3-image.py` makes it with Python's `lzma`.
- `openipc/infinity3-image.py` packs the 256 KiB boot partition as retail
  MSC313E cameras lay it out: zeros, the MXP table at `0x3000` (its
  records byte-identical to a camera's for the partitions it lists: ROM,
  MXPT, IPL, IPL_CUST, KEY_CUST, UBOOT, BOOT, UBOOT_ENV), the IPL at
  `0x4000`, IPL_CUST at `0x10000`, the uImage at `0x20000` (at most
  128 KiB, checked), 0xff to `0x40000`, where the environment starts. The
  SDK's own layout (MXP at `0x20000`, U-Boot at `0x30000`, environment at
  `0x4F000`) is not used: OpenIPC's environment is at `0x40000`.
- The vendor stages are the SDK's, `project/I3/image`:
  - `openipc/infinity3-msc313e-ipl.bin`: `image_QFN64M-MSC313E/IPL.bin`,
    "IPL_QFN64MW", d156225;
  - `openipc/infinity3-msc316-ipl.bin`: `image_QFN128M-MSC316DC/IPL.bin`,
    "IPL_EFUSE", the same file in every MSC316 image of the SDK;
  - `openipc/infinity3-ipl-cust.bin`: `IPL_CUST.bin`, the same in every
    image and on the retail camera.
  The retail camera carries a later MSC313E IPL (bf16da4); the SDK's is
  used because it is the vendor's release.
- No SPL, `SKIP_LOWLEVEL_INIT`. `board_init_f()`'s stack and global data
  sit 2 MiB below `SYS_TEXT_BASE`.
- **DRAM size**: probed up to the part's (`PHYS_SDRAM_1_SIZE`: 64 MiB on
  the MSC313E, 128 MiB on the MSC316s); the DDR wraps past its end.

## Hand-off to the kernel

- The vendor kernels (3.18.30) take ATAGs at `0x20000100` and turn them into their built-in
  DTB (`early_atags_to_fdt`); machine number 1391 (`INFINITY3`), as the
  vendor U-Boot passes.
- Kernel uImages load at `0x20008000`; `baseaddr` is `0x21000000`, where
  the vendor U-Boot reads them.
- Vendor kernel partitions hold a uImage of type "lzma" with **xz** data,
  which the vendor U-Boot unpacks with its own xz code. Mainline U-Boot
  has no xz in `bootm`: such a kernel has to be repacked (uncompressed,
  or as OpenIPC builds it, a zImage in a plain uImage).
- `mem=` is the whole DDR (`osmem` 64M/128M), as the vendor passes it; the
  media drivers take CMA from the kernel. The MTD name is `NOR_FLASH`
  (`SFC`), the console `ttyS0`.

## Peripherals

Everything is on the RIU, 16-bit registers: bank B at
`0x1F000000 + B * 0x200`, register R at `+ R * 4`. The vendor sources name
a register by its byte address `0xBBBB00 + R * 2`.

- **Console:** UART0 at `0x1F221000`, a DesignWare 8250 with a register
  every 8 bytes (`SYS_NS16550_REG_SIZE -8`), 172.8 MHz. The IPL has set it
  up already.
- **Timer:** PIU timer 0 (bank `0x30`, registers `0x10`-`0x15`), 32 bits
  at the 12 MHz crystal, free running; `lib/time.c` does the rest.
- **Reset:** PM sleep register `0x2E` (`0x1F001CB8`): `0xFF`, then `0x79`.
- **SPI NOR:** as the vendor kernel and U-Boot do it (`halSERFLASH.c`
  with `FSP_READ_DIRECT` and `FSP_WRITE_BDMA`):
  - reads through the XIU window at `0x14000000`, in the read mode the
    IPL set (3-byte addresses: 16 MiB);
  - erase: one FSP run of WREN, `0xD8` with the address and RDSR, the
    FSP repeating RDSR until the flash is idle (`FSCHK`); 64 KiB sectors;
  - page program: BDMA channel 0 (path `0x2B40`, MIU to FSP) feeds the
    page to the FSP, which sends it in place of write-buffer byte 5 of
    WREN, PP, address, dummy, RDSR ("outside" buffer, `0x78`/`0x79`). The
    page is copied to a static buffer first and the MIU pipe flushed
    (`ss_miu_flush()`, the L3 bridge), as the vendor does;
  - block protection is cleared only if set, with a one-byte WRSR: status
    register 2 (quad enable, which the IPL's read mode may need) is not
    touched otherwise.
- **Ethernet:** the EMAC at `0x1F2A2000`, an Atmel AT91RM9200-style MAC.
  Each 32-bit register G is two RIU registers, bits 15:0 at `+2G`, 31:16
  at `+2G+4`, written low half first. After the vendor U-Boot as its
  `infinity3_defconfig` builds it (`ETHERNET_ALBANY`, `SOFTWARE_DESCRIPTOR`;
  its `mhal_emac.h` comments `ETHERNET_ALBANY` out, the defconfig turns it
  on): the internal PHY's clocks and analog set-up (the vendor's table,
  verbatim), RX descriptor mode 1, a ring of 128 descriptors with a
  0x600-byte buffer each (16 KiB aligned, as the vendor's), TX through
  TAR/TCR, MDIO through MAN, auto-negotiation as the vendor runs it (up to
  8 s without a link, at every network command).
- **SD:** not supported. The FCIE (`drivers/mstar/sdmmc`) is not ported, so
  `CONFIG_MMC` is off and `common/openipc.c` has no card to recover from.

## Build

```sh
bash build.sh               # all three; continues past failures, exits non-zero if any failed
bash build.sh msc313e       # one or more SoCs (CROSS_COMPILE defaults to arm-linux-gnueabi-)
```

- The output is `output/u-boot-<soc>-nor.bin`, the 256 KiB boot
  partition, built out of tree in `output/build-<soc>-nor/`.
- `.config` is `openipc/openipc_config`, then `openipc/<soc>_config`, then
  `olddefconfig`. There are no `configs/*_defconfig` for these targets.
- Thumb-2 (`SYS_THUMB_BUILD`): `u-boot.bin` is about 140 KiB, 73 KiB in
  the uImage; the `UBOOT` partition holds 128 KiB. The image packer fails
  the build past that.
- Python 3 with `lzma` is needed for the image.

## Verification

The emulator is OpenIPC/qemu-hisilicon, machine `msc313e`
(`docs/sigmastar.md` there). `-M msc313e,flash-file=<image>` does the mask
ROM's job, so it boots exactly the flashed bytes, IPL and IPL_CUST
included. There is no MSC316 machine; the MSC316 images boot on `msc313e`
(their IPL sizes its 64 MiB), which checks everything but the DDR set-up.

- **CI smoke:** two boots of an 8 MiB image holding only U-Boot. The second
  must load the environment the first saved.
- **Checks run by hand** (all passed under qemu-hisilicon `msc313e`):
  - `saveenv` + `reset`; the variable is there after the reset.
  - `ping` to the qemu host; TFTP of 300000 bytes with the right CRC.
  - `sf erase` + `sf write` of 300000 bytes at `0x700123` + `sf read` +
    `cmp.b`; the bytes before the write read 0xff.
  - TFTP recovery from a flash holding only U-Boot
    (`-nic user,net=192.168.1.0/24,host=192.168.1.107,tftp=<dir>`): the
    rootfs to `0x250000`, the kernel to `0x50000`, a reset, and `bootflash`
    boots the kernel from flash.
  - The boot limit: two `reset`s, then `altbootcmd` boots with `failsafe`.
  - The retail camera's 3.18.30 kernel, repacked uncompressed and booted
    by TFTP with its own `mtdparts`, mounts its squashfs from flash and
    reaches its shell.
- **Not checked:** an OpenIPC firmware (no msc313e release is published),
  `fw_printenv` from Linux, and anything on silicon.
- **What qemu does not check:** the DDR training (the IPLs are the
  vendor's), the PHY's analog set-up, the FSP's and BDMA's real timing,
  whether the bootcount word in DDR survives the IPL after a real reset.

## Things to keep

- `mem=` stays the first kernel argument, as on the other OpenIPC targets.
- The boot counter is the DRAM word at `0x21F20000`, like the other
  families' DDR + 0x1f20000. A healthy boot does not clear it. Whether the
  IPL's BIST leaves it alone on silicon is not known.
- Variables U-Boot writes before relocation must be initialised (in
  `.data`): `.bss` is not cleared until after relocation. U-Boot runs from
  DDR from the first instruction, so `.data` is writable then.

## Conventions

- One topic per commit, `<area>: <what changed>` (`sigmastar:`,
  `ms_sflash:`, `ms_emac:`, `openipc:`, `net:`, `ci:`).
- Commit bodies say what was verified, and where: the qemu machine, or a
  board.
