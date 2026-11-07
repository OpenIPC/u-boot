# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this branch is

`openipc-sigmastar` is OpenIPC's U-Boot for SigmaStar's (MStar's)
Cortex-A7 camera SoCs. The tree is **mainline U-Boot v2022.10**, the same
release as `openipc-hisilicon`, `openipc-xm`, `openipc-gk710x` and
`openipc-anyka`, plus:

- the OpenIPC boot layer, carried over from `openipc-anyka`, which took it
  from `openipc-xm` and `openipc-hisilicon`;
- the SigmaStar support, written from the vendor SDKs: U-Boot 2015.01
  `arch/arm/cpu/armv7/<die>` and `drivers/mstar`, and the kernels'
  `drivers/mstar`.

| Die | SoCs | Boot stages from | Status |
| --- | --- | --- | --- |
| infinity3 | `msc313e` `msc316dc` `msc316dm` | SDK `MSC3XX_SDK_V_1711291828-I3` | qemu, see "Verification" |
| infinity2m | `ssd201` `ssd202d` | SDK `i2m` `project/board/i2m/boot/ipl` | qemu, IPL to kernel |
| infinity6 | `ssc325` | OpenIPC/u-boot-sigmastar `ipl/` | **no silicon**; qemu per-die machine |
| infinity6b0 | `ssc333` `ssc335` `ssc335de` `ssc337` `ssc337de` | same | **no silicon**; qemu per-die machine |
| infinity6e | `ssc30kd` `ssc30kq` `ssc338q` | same | **no silicon**; qemu per-die machine |
| infinity6c | `ssc377` `ssc377d` `ssc377de` `ssc377qe` `ssc378de` `ssc378qe` | same | **no silicon**; qemu per-die machine |
| infinity2m | `ssr621q` | SDK `i2m` | **no silicon**; qemu `ssr621q` |
| infinity6 | `ssc325de` `ssc327de` | as `ssc325` | **no silicon**; qemu per-part machine |
| Iford | `ssc029a` `ssc029b` `ssc029c` | Iford SDK IMD00V2.1.0 boot.bin, `board/iford/boot/nor/ipl` | **no silicon**; only `ssc029c` boots in qemu |
| Ifado | `ssc375` | Dahua IPC-HDW1433V-A-IL2 NOR (IPL, CIS, IPL_CUST) | **no silicon**; qemu `ssc375`, to the vendor kernel's userspace |
| Ifliegen | `ssc034a` `ssc034b`, NOR and SPI NAND | Ifliegen SDK ILSSV01C12 `board/ifliegen/boot` (IPL, IPL_CUST, TF-A `u-bl32.bin`, flash.nri/flash.sni, flash lists) | **no silicon**; qemu `ssc034a`/`ssc034b`, to the SDK kernel's shell |
| SPI NAND | `<soc>-nand` for every infinity2m, 6, 6b0, 6e, 6c part, `ssc029a`, `ssc034a/b` | the dies' NAND IPLs (published images, i2m, Maruko, Iford and Ifliegen SDKs) | **no silicon**; qemu, openipc-hisilicon's 768k/256k/UBIFS layout |

OpenIPC/u-boot-sigmastar, the vendor's U-Boot 2015.01 with OpenIPC's
environment, is what OpenIPC ships for the infinity6 dies today, and it
publishes `u-boot-ssc*-nor.bin` to OpenIPC/firmware `latest`. This branch's
images go only to the CI repository's own `sigmastar` release: the publish
job of `build-sigmastar.yml` on `openipc` is commented out for now (and its
`PUBLISH_GLOB` kept to `u-boot-msc31*` for when it returns).

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

Not covered: the dies with no OpenIPC use or no IPL in the SDKs at hand
(infinity5, infinity7, pioneer3, mercury6, souffle); Iford's 029b/029c SPI
NAND (only RTOS/dual-OS sets in the SDK); Mercury6 (no IPL at hand).

## Relationship to mainline

```sh
git diff --stat v2022.10 HEAD                  # the whole delta, small
git diff --diff-filter=M v2022.10 HEAD         # the mainline files we patched
```

**Added**

| Area | Location |
| --- | --- |
| SoC: timer, reset, DRAM size, MIU flush, `memlx`/`memsz` | `arch/arm/mach-sigmastar/` (`ARCH_SIGMASTAR`; `TARGET_<SOC>` selecting `SS_<DIE>`; register map in `include/mach/sigmastar.h`) |
| Board: `misc_init_r` → `openipc_helper()`, `board_eth_init`, `board_mmc_init` | `board/sigmastar/ipc/` |
| SPI NOR on the FSP, non-DM | `drivers/mtd/spi/ms_sflash.c` (`MS_SFLASH`). It provides `spi_flash_probe()` and fills `mtd->_read/_write/_erase`, as `ak_sflash` does on `openipc-anyka` |
| Ethernet, non-DM | `drivers/net/ms_emac.c` (`MS_EMAC`) |
| SD card on SDIO0, non-DM | `drivers/mmc/ms_sdmmc.c` (`MMC_MS_SDMMC`), registered from `board_mmc_init()` |
| xz decompression | `lib/xz/` (xz-embedded, 0BSD, with CRC64; `XZ`), `include/linux/xz.h`, `unxz()` in `include/unxz.h` |
| Memory map, console, timer | `include/configs/sigmastar.h`, ending in `#include "openipc-common.h"` |
| OpenIPC boot layer | `common/openipc.c`, `include/openipc.h` (as on `openipc-anyka`), `include/configs/openipc-common.h` (SigmaStar rewrite, NOR only, same env commands) |
| Kconfig fragments, vendor boot stages, the image packers (`infinity3-image.py`, `sigmastar-image.py`), the build | `openipc/`, `build.sh` |

**Patched mainline files**, each small:

- `arch/arm/{Kconfig,Makefile}`: the `ARCH_SIGMASTAR` hook.
- `common/{Kconfig,Makefile}`: `OPENIPC`, `PRODUCT_SOC`, `PRODUCT_SOCMODEL`.
- `drivers/mtd/spi/{Kconfig,Makefile}`, `cmd/Kconfig` (`CMD_SF`) and
  `env/Kconfig` (`ENV_IS_IN_SPI_FLASH`): accept `MS_SFLASH`.
- `drivers/mmc/{Kconfig,Makefile}`: `MMC_MS_SDMMC`.
- `lib/{Kconfig,Makefile}`: `XZ`.
- `boot/image.c`: `image_decomp()` decodes xz data in an "lzma" uImage
  with `unxz()` (see "Hand-off to the kernel").
- `drivers/net/{Kconfig,Makefile}`, `include/netdev.h`: `MS_EMAC`;
  `NETDEVICES` selects `DM_ETH` only when `DM` is on, as on `openipc-xm`.
- `.gitattributes`: `openipc/*.bin` are binary (the tree's
  `* text eol=lf` would rewrite their CR LF pairs).

The console is mainline `ns16550`, non-DM.

## Boot

The vendor boot chain is kept on every die; only the partition `UBOOT` is
ours. What follows up to "Hand-off" is infinity3; "Later dies" says what
differs.

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

### Later dies (infinity2m, infinity6*)

`openipc/sigmastar-image.py` packs the same 256 KiB boot partition as
OpenIPC/u-boot-sigmastar's `make_boot_spinor.sh`, with the same bytes for
everything but the uImage: IPL at `0`, `openipc/sigmastar-mxp.bin` at `0xF000`
(infinity6c: `infinity6c-gcis.bin`, a table of flash parts, at `0x9000`),
IPL_CUST at `0x10000`, the uImage at `0x20000`.

- All parts of a die run the same IPL and IPL_CUST: the images in OpenIPC/firmware
  `latest` are byte-identical in that area, part to part.
- The MXP table is the one blob for all of them, and also serves infinity2m,
  whose SDK has no such layout (it has MXP at `0x20000`, U-Boot at `0x30000`,
  environment at `0x5D000`, which collide with OpenIPC's). Its entries for IPL
  (`0x4000`) and MXPT (`0x3000`) do not say where those are in this layout;
  infinity2m's stages boot from it regardless under qemu.
- The uImage: `xz -z` data (preset 6, an 8 MiB dictionary, CRC64), not the
  preset 9e of infinity3, and a 32-byte `MVX4##<die>...#CM_UBT1501#XVM` name, as
  `create_img.sh` gives it.

### SPI NAND

The layout and environment of openipc-hisilicon (`include/configs/openipc-common.h`,
`openipc/nand_config`): `768k(boot),256k(env),-(ubi)`, the environment at
`0xC0000` (128 KiB in the 256 KiB range, so one bad block there leaves the
other; openipc-hisilicon's is 256 KiB in 256), one UBI device from `0x100000` whose UBIFS
`rootfs` volume holds the kernel as `/boot/fitImage` or `/boot/uImage`, and
`rootfs_data` on the rest; `root=ubi0:rootfs rootfstype=ubifs ubi.mtd=2,2048`.
`common/openipc.c` is the common one (openipc-ingenic's, `OPENIPC_SPI_NAND`
taking `CONFIG_NAND_MS_SPINAND` too): the scan checks the UBI marker in the
first good eraseblock, the rootfs volume's UBIFS and its `/boot` kernel; `ur`
installs `rootfs.ubifs.<soc>`, sized to the image, keeping `rootfs_data`.

`./build.sh <soc>-nand` gives `output/u-boot-<soc>-nand.bin`, the boot
partition as far as it holds data (no `0xFF` padding, as the NOR images), one
copy of every stage, a stage a block:

- infinity6, 6b0, 6e, infinity2m (the SNI's `MSTARSEMIUSFDCIS` form and the
  older PNI, `(start block, blocks, type, backups)` records, a byte sum from
  offset 4): block 0 the SNI (page 0) and PNI (page 1), IPL in block 1
  (BL0PBA 1, BL1PBA 0: infinity2m's IPL loses IPL_CUST with a BL1PBA set),
  IPL_CUST in 2, U-Boot in 3-5. SNI byte `0x2E` is 2: infinity6's IPL_CUST
  looks for U-Boot from the block after it; the other IPLs read the PNI.
- Ifliegen: block 0 the CIS (SNIB with the flash list's W25N01GV SNIF,
  record 57, BL0PBA 1, one copy; the name-record PNI; the flash list), IPL,
  IPL_CUST, TF-A's SP_MIN in blocks 1-3, U-Boot in 4-5.
- `ms_spinand` takes geometry from the SNI. The bad-block table stays in
  memory, like the vendor's: nothing is written to the last blocks, which UBI
  owns. A part whose SNI gives a spare other than 64 or 128 bytes is refused:
  the core has no layout for it.
- A page load holds the chip select by QSPI's burst bit, and OUT_SIZE counts one
  more than BDMA sends. Whole page and spare go by BDMA; the vendor sends those
  through the FSP, so a spare write takes a path it never does.
- Where this branch is ahead of the other vendor branches (Qodo on
  OpenIPC/u-boot#109; they follow later): `ubwrite` writes only an image
  that fits the boot partition, checked before the erase, and into the
  partition, so a bad block cannot push it into `env`; recovery takes a
  NAND image only if it fits the free RAM, on NOR only files whose name
  carries this SoC as a word (`uImage.ssc337.img` is not `ssc337de`'s);
  a saved `altbootcmd` is migrated before recovery may run it. A bad block
  in blocks 1-5 still moves the stages off the blocks the PNI names: one
  copy each is what 768 KiB holds.
- Not compatible with OpenIPC/u-boot-sigmastar's NAND layout (UBI at
  `0x240000`, a `kernel` volume and a squashfs `rootfs`): a camera moving to
  this U-Boot takes the firmware's `rootfs.ubifs` too, as HiSilicon's do.

### Iford

`openipc/sigmastar-image.py --iford-cis`: the SDK's NOR boot.bin as it is, IPL at
`0`, the CIS area (flash.nri, PNI, flash list) at `0xE000`, IPL_CUST at
`0x18000`, our UBOOT at `0x1F000`, the environment at `0x40000`. Its IPL_CUST
takes the xz uImage like the older dies, but by the header's load address,
`0x23E00000`. Iford shares infinity6c's paths (`SS_I6C_FAMILY`): the chip select
in software, the PHY at bank `0x1518`; its FSP is at `0x100D`/`0x100E`, BDMA from
MIU reads channel 1 (path `0x2B41`), SD has its own pads (vendor
`mmc_host/src/iford`, SD0 defaults) and the SD clock's SCGP is bank `0x1135`.
The DDR size is bank `0x1659` reg `0x0D`, log2 MiB, which the IPL finds by
fill-and-wrap test; `dram_init()` still probes if it reads outside 64 MiB-2 GiB
(the kernel's decode takes [7:4] as a second term). `memlx`/`memsz` are the SDK
images' own (`set_config`) for 128, 256 and 512 MiB. No kernel hand-off was tried: its 5.10 kernel takes a DTB.
Under qemu `ssc029` (029b at `bond=0x10`, its IPL refuses other bonds; which a
real part carries is unknown): all three through their own IPL chain, `sf`, env
across reset, ping, TFTP, SD card detect, FAT load and raw read-back. 029a's IPL
logs a DQ training failure the model does not train for.

### Ifado

`ssc375` (Ifado, SSC032B, QFN88 DDR3). No SDK: the stages are Dahua's, from
an IPC-HDW1433V-A-IL2 NOR (`openipc/ifado-*.bin`, the flash's `0-0x8000`,
`0x8000-0x10000` and `0x10000-0x15000`, trailing `0xFF` cut). `--ifado-cis`:
IPL at `0`, the CIS area (NRI records, PNI at `0x9000`, NRIB) at `0x8000`,
IPL_CUST at `0x10000`, our xz uImage in the PNI's UBOOT at `0x20000`
(`0x23E00000`), the environment at `0x40000`, where Dahua's 8 KiB one was.
The Dahua `sbot` wrapper is not needed: IPL_CUST checks its magic only on a
path the shipped image does not take. The die is the infinity2m map
(`SS_IFADO`: DDR probed up to 128 MiB, PHY at bank `0x31`, SD at `0x1413`,
EMAC and FSP where infinity2m has them) with infinity6c's software chip
select (`SS_FSP_SW_CS`). No SD: the camera has no slot and the pads are
unread. Its IPL_CUST jumps with the MMU and D-cache on, page table at
`0x20004000`, where the kernel builds its own; `lowlevel_init.S`
`save_boot_params` cleans the cache and turns both off (a branch on the
other dies, whose loaders leave SCTLR.M clear; checked on each machine).
qemu `ssc375`: chain, `sf` unaligned write/read, raw file, env across
reset, `memlx` for 128 MiB, ping, TFTP CRC; Dahua's 4.9.84 kernel by
`bootm` with its built-in DTB passed, through the squashfs mount to Dahua's
userspace (which reboots without DHBoot's partitions). The Dahua dump is
for tests and reverse engineering only: U-Boot is not changed to run
Dahua's firmware, and on a flash still holding it the layout scan may take
Dahua's backup U-Boot at `0xC0000` for a kernel; that stays as it is.

### Ifliegen

`ssc034a` (QFN128, 128 MiB DDR3) and `ssc034b` (QFN88, 64 MiB), Cortex-A55
in AArch32. The SDK ships no images; `--ifliegen-tfa`/`-nri`/`-list` build
what its image.mk would, at offsets both packages' IPLs fit: IPL at `0`, the
CIS at `0xC000` (flash.nri, the PNI at `+0x1000`, the flash list with
nrigenerator's CRC32 at `+0x2000`), IPL_CUST at `0x16000`, TF-A's SP_MIN at
`0x1D000`, our xz uImage at `0x24000` (`0x23E00000`), the environment at
`0x40000`. The IPL runs TF-A, TF-A runs IPL_CUST, IPL_CUST U-Boot, in the
normal world. The die is Iford's in the SDK (`SS_IFORD_FAMILY`: FSP at
`0x100D`, BDMA from MIU by channel 1, no MIU pipe to flush, EMAC MCG off;
not Iford's MCM write at bank `0x1132`), with:

- UART0 left in the URDMA's DMA mode, the IPL's bytes going through its TX
  FIFO (bank `0x1107` reg `0x17`): `board_early_init_f()` waits for the FIFO,
  stops both DMAs and leaves DMA mode, as the SDK's `hal_uart_config()`, and
  clears PM sleep reg 9 bit 11 (RX off); the 8250 then serves the console.
- SP_MIN stays at `0x23FE0000` as the kernel's PSCI: U-Boot keeps to the first
  48 MiB, as the SDK's (`CONFIG_SGS_RAM_SIZE`), or the kernel hangs at its
  first PSCI call. DDR is probed (no size register known), up to 128 MiB.
- `memlx`/`memsz`: the SDK's set_config, `0x7FE0000`/`0x5000000` at 128 MiB,
  `0x3FE0000`/`0x2000000` at 64 MiB. Machine 5120 (MACH_TYPE_IFACKEL, its
  U-Boot's); the 4.9 kernel turns the ATAGs into its built-in DT.
- SD: SD0, the board's mmc0 (`mmc_host/src/ifliegen`): PADGPIO pads from
  `0x0F`, card detect at `0x16`, power on PAD_FUART_RTS (`0x34`), the clock
  as Iford's but SC_GP_CTRL at bank `0x1133`, the host's MCG off.
- SPI NAND (`ssc034a-nand`, `ssc034b-nand`): `--ifliegen-sni`, in OpenIPC's
  768 KiB (see "SPI NAND"). The IPL takes its timeouts from the SNIF: the CIS
  carries the flash list's W25N01GV, record 57; the SDK's generic record has
  none and the IPL fails to load.

The SGS kernels (Iford, Ifliegen, Ifado: `drivers/sgs_common/flash/nor`) name
the NOR `nor0`, not the older `NOR_FLASH`: `SFC` in
`openipc-common.h`. qemu `ssc034a`/`ssc034b`: the chain, `sf` unaligned
write/read and the raw file, env across reset, ping, TFTP CRC, the SDK's
4.9.84 kernel (mstar-06's build) from OpenIPC's NOR layout, both CPUs by
PSCI, the six partitions, squashfs root to a shell (`extras=init=/linuxrc`
for its rootfs). SD (mstar-06's model): detect, 4-bit, FAT load CRC, raw
write and read-back. NAND: the chain, `nand` erase/write/read against the
raw file, env across reset, the SDK kernel from an OpenIPC UBI to a shell
with the `ubiblock` override.

### SPI NAND on infinity6c and Iford

Both read the name-record PNI, as Ifliegen does, and need more than a block a
stage allows:

- infinity6c: its IPL reads a CIS copy past block 0, so the CIS is in blocks
  0 and 1 (the SNI the flash list's record 1, GD5F1GQ4UEYIH, BL0PBA 2, one
  copy, its CRC32), IPL in block 2, IPL_CUST in 3, U-Boot in 4-5. The
  generic `EE EE` SNI is refused. With the copy elsewhere (block 5) IPL_CUST
  loads but reads U-Boot back failing its CRC; why, not found.
- Iford (`ssc029a-nand`, the one SDK spinand set with plain Linux, an OP-TEE
  one): its IPL takes IPL_CUST, OP-TEE, then UBOOT by name, at any 2 KiB
  page, which is what lets the 338 KB OP-TEE in: the CIS (the SDK cis.bin's
  SNIB with W25N01JW's SNIF, BL0PBA 1, one copy, its CRCs; the PNI; its flash
  list) in block 0, IPL at `0x20000`, IPL_CUST, OP-TEE and our uImage each
  from the next page, U-Boot ending near `0xB8000`. Block-aligned, they would
  need 1 MiB. `ms_spinand` turns a W25N's buffer mode back on, which this IPL
  leaves off. 029b and 029c have only RTOS/dual-OS NAND sets in the SDK; not
  built.
- A board with another part needs that part's record in page 0, as the SDK's
  `nand probe` writes it: the IPLs take their timing from it.
- infinity6c's IPL reads the environment partition too and prints "env bad
  CRC" for the format it expects; U-Boot's own environment is unaffected.

## Hand-off to the kernel

On the later dies:

- ATAGs at `0x20000000`, machine number per die (`openipc/<die>_config`:
  infinity6c passes MERCURY6, 1399, as its vendor U-Boot does).
- No `mem=`. `LX_MEM=${memlx} mma_heap=...,sz=${memsz} cma=2M`, the arguments
  of OpenIPC/u-boot-sigmastar, whose saved environments have to keep working.
  `ss_set_mem_env()` sets `memlx` and `memsz` at every boot, before the
  environment is used, from its table of those values by DDR size. infinity2m has
  no such table upstream; its row uses infinity6c's, since its kernel warns
  about an LX_MEM that is not 1 MiB aligned.
- `CONFIG_ENV_OFFSET`/`SIZE` are those of OpenIPC/u-boot-sigmastar, so a
  saved environment loads as it is. Its `bootcmd` and `bootargs` are kept.
- **Flash on infinity6c:** its IPL leaves the chip select in software, held
  high (QSPI bank `0x17` reg `0x0a` = 3), and the vendor's driver frames each
  flash command by hand. `ms_sflash` does the same and leaves that state to the
  kernel. It reads by BDMA, or the FSP when short or unaligned, not by XIU;
  OUT_SIZE is the count plus one. The published U-Boot runs
  `flash/nor/drvSPINOR.c` + `fsp_qspi`: `flash_isp/infinity6c` is not its code.
- **DDR size:** on infinity6, 6b0 and 6e `1 << (MIU 0x1012 reg 0x69 >> 12)` MiB,
  on infinity6c `1 << (bank 0x1659 reg 0x0D)` MiB, since its IPL writes a fixed
  `0x6000` to reg 0x69. The IPL sets both; each is what that die's vendor
  `dram_init()` reads. infinity2m has no such register
  in its vendor code and probes as infinity3 does.
- **U-Boot stays in the first 64 MiB** (`board_get_usable_ram_top()`), like the
  vendor's `CONFIG_UBOOT_RAM_SIZE`. BDMA's address registers are wider than the
  12-bit mask in the IPLs: the vendor kernel's reach MIU `0x30000000`, on the
  qemu machines 31 bits.

On infinity3:

- The vendor kernels (3.18.30) take ATAGs at `0x20000100` and turn them into their built-in
  DTB (`early_atags_to_fdt`); machine number 1391 (`INFINITY3`), as the
  vendor U-Boot passes.
- Kernel uImages load at `0x20008000`; `baseaddr` is `0x21000000`, where
  the vendor U-Boot reads them.
- Vendor kernel partitions hold a uImage of type "lzma" with **xz** data
  (CRC64 check), which the vendor U-Boot unpacks with its own xz code.
  `image_decomp()` looks at the data of an "lzma" uImage: the xz magic
  goes to `unxz()` (`lib/xz`, xz-embedded with CRC64), anything else to
  the LZMA decoder as before. So vendor kernels boot as they are, and so
  do OpenIPC's (a zImage in a plain uImage) and real LZMA uImages.
  `lib/xz` is upstream's Linux tree except one line in `xz_private.h`
  (the standalone branch when `__UBOOT__`) and U-Boot's own
  `xz_config.h`; no BCJ filters.
- `mem=` is the whole DDR (`osmem` 64M/128M), as the vendor passes it; the
  media drivers take CMA from the kernel. The MTD name is `NOR_FLASH`
  (`SFC`), the console `ttyS0`.

## Peripherals

What differs by die is in `include/mach/sigmastar.h`, or under `CONFIG_SS_<DIE>`
in the driver:

| | infinity3 | infinity2m | infinity6, 6b0 | infinity6e | infinity6c |
| --- | --- | --- | --- | --- | --- |
| FSP bank | `0x16` | `0x16` | `0x16` | `0x16` on QFN88, else `0x100D` (CHIPTOP `0x48`) | `0x16` |
| BDMA reg 3 | untouched | 0 | `0x8000` | `0x8000` | `0x8000` |
| PHY banks | `0x31`- | `0x31`- | `0x31`- | `0x1514`- | `0x1518`- |
| EMAC clocks (SCGPCTRL) | `0x44`, `0x46` | same | same | same | `0x54`, `0x56` |
| PHY sadc timer | written | RMW | RMW | RMW, and LMS scale | RMW |
| SD host bank | `0x1410` | `0x1410` | `0x1413` | `0x1413` | `0x1413` |
| SD clock (CLKGEN) | `0x45` | `0x45` | `0x43` | `0x43` | `0x43` |
| SD `DDR_MOD` | 0 (bypass) | `0x6400` | `0x6400` | `0x6400` | `0x2400` |
| SD mode | CHIPTOP `0x08` | CHIPTOP | CHIPTOP | PADTOP `0x67`, `0x68` | PADTOP `0x67`, `0x68` |
| SD pads | PADTOP `0x50`- | same | same | PADTOP `0x12`- | PADGPIO `0x00`- |
| SD power | PADTOP `0x17` | PADTOP `0x20` | PADTOP `0x17` | PADTOP `0x25` and `0x10` | PADGPIO `0x22` |
| SD card detect | PM GPIO `0x47` | same | same | PADTOP `0x11` | PADGPIO `0x06` |

The rest of this section is infinity3's, which the table adjusts.

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
  TAR/TCR, MDIO through MAN (registered as the MDIO bus `ms_emac`, for
  `mii`), auto-negotiation as the vendor runs it (up to
  8 s without a link, at every network command).
- **SD:** SDIO0, the FCIE5 host at bank `0x1410` (`drivers/mmc/ms_sdmmc.c`),
  after the vendor's `hal_sdmmc_v5.c`, `hal_card_platform_iNF3.c` and
  `ms_sdmmc_ub.c` (the vendor's own glue to U-Boot's mmc core):
  - a job sends the command through the CMD/RSP FIFO (registers `0x20`-,
    byte 0 = `0x40 | index`, argument MSB first; the response comes back
    in the same bytes, byte 0 the index or `0x3F`) and, with `DTRX_EN`,
    moves the data by DMA (MIU addresses, `BLK_SIZE` x `JOB_BLK_CNT`);
    a write runs its data as a second job with `JOB_DIR`; `MIE_EVENT`
    gives the ends, `SD_STS` the errors (R3's CRC error is bogus and
    ignored); a job that never ends resets the IP (`FCIE_RST`);
  - 4-bit, the clock from CLKGEN `0x45` (select 0-7: 48, 43.2, 40, 36,
    32, 20, 12 MHz, 300 kHz; the fastest not above the one asked), bypass
    timing, the vendor's Nrc wait before each command;
  - card detect: PM GPIO `0x47` bit 2, low with a card in; card power:
    PADTOP `0x17` bit 4, low on; at each init the pads go to SDIO0 with
    pull-ups after the vendor's power-off, power-up, clock-on sequence
    (about 105 ms). No card: `mmc_init` fails at once.
  - `common/openipc.c` recovers from the card at every boot, as on
    `openipc-anyka`: `script.txt` (which then takes precedence), else
    `uImage.<soc>.img` and `rootfs.squashfs.<soc>.img`, uImages flashed
    where their load address says, without the header
    (`mkimage -T firmware -a 0x50000 -d uImage.<soc> uImage.<soc>.img`,
    `-a 0x250000` for the rootfs); an image already in flash is skipped.

## Build

```sh
bash build.sh               # all SoCs; continues past failures, exits non-zero if any failed
bash build.sh msc313e       # one or more SoCs (CROSS_COMPILE defaults to arm-linux-gnueabi-)
```

- The output is `output/u-boot-<soc>-nor.bin`, the 256 KiB boot
  partition, built out of tree in `output/build-<soc>-nor/`.
- `.config` is `openipc/openipc_config`, then `openipc/<die>_config` on the
  later dies, then `openipc/<soc>_config`, then `olddefconfig`. There are no `configs/*_defconfig` for these targets.
- Thumb-2 (`SYS_THUMB_BUILD`): `u-boot.bin` is about 180 KiB, 96 KiB in
  the uImage (SD, FAT, LZMA and xz included); the `UBOOT` partition holds 128 KiB. The image packer fails
  the build past that.
- Python 3 with `lzma` is needed for the image.

## Verification

The emulator is OpenIPC/qemu-hisilicon, machine `msc313e`
(`docs/sigmastar.md` there). `-M msc313e,flash-file=<image>` does the mask
ROM's job, so it boots exactly the flashed bytes, IPL and IPL_CUST
included. qemu-hisilicon master has no MSC316 machine. On `msc313e` the MSC316
IPL takes the efuse path that skips DDR set-up altogether. Branch
`sigmastar-i6-dies` adds `msc316dc` and `msc316dm`, where the IPL programs the
DDR PHY and MIU (`D-02.`); qemu trains nothing, so that runs the IPL's tables
without proving them.

- **CI smoke:** two boots of an 8 MiB image holding only U-Boot. The second
  must load the environment the first saved.
- **Checks run by hand** (all passed under qemu-hisilicon `msc313e`):
  - `saveenv` + `reset`; the variable is there after the reset.
  - `ping` to the qemu host; TFTP of 300000 bytes with the right CRC;
    `mii read 0 1` reads the PHY.
  - `sf erase` + `sf write` of 300000 bytes at `0x700123` + `sf read` +
    `cmp.b`; the bytes before the write read 0xff.
  - TFTP recovery from a flash holding only U-Boot
    (`-nic user,net=192.168.1.0/24,host=192.168.1.107,tftp=<dir>`): the
    rootfs to `0x250000`, the kernel to `0x50000`, a reset, and `bootflash`
    boots the kernel from flash.
  - The boot limit: two `reset`s, then `altbootcmd` boots with `failsafe`.
  - The retail camera's kernel partition as it is (xz in an "lzma"
    uImage), `sf read` + `bootm` with its own `mtdparts`: the 3.18.30
    kernel mounts its squashfs from flash and reaches its shell. The same
    kernel as a real LZMA uImage, and uncompressed, by TFTP: the same.
  - SD (`-drive if=sd,format=raw,file=<card>`, qemu-hisilicon's SDIO0
    model; a power-of-two card with an MBR and a FAT partition):
    `mmc info` (4-bit), `fatls`, `fatload` with the right CRC, 256 blocks
    of `mmc write` read back identical; `script.txt` at boot; recovery
    of kernel and rootfs from an empty flash, the kernel's checksum
    good from flash, nothing rewritten on the next boot with the card
    still in; no card: "MMC: no card present". The model was checked
    against the vendor U-Boot's SD driver first.
- **Not checked:** an OpenIPC firmware (no msc313e release is published),
  `fw_printenv` from Linux, and anything on silicon.
- **What qemu does not check:** the DDR training (the IPLs are the
  vendor's), the PHY's analog set-up, the FSP's, BDMA's and SD host's
  real timing, the SD pads and card power,
  whether the bootcount word in DDR survives the IPL after a real reset.

### The later dies

Run under qemu-hisilicon master plus its Iford machine (`ssc029`, not yet
upstream at the time). **Nothing here has run on silicon.**

- `ssd201`, `ssd202d`: the image boots through the real IPL and IPL_CUST, also
  with the MXP table above. `saveenv` + `reset`; TFTP of 300000 bytes with the
  right CRC; `ping`; `mii read`; an unaligned `sf write` of 300000 bytes + `sf
  read` + `cmp.b`. The SDK's 4.9.84 kernel by `bootm`: ATAGs, LX_MEM and
  mma_heap taken, two CPUs, up to the missing root file system.
- `ssc337` U-Boot under the infinity2m stages on `ssd202d`: DDR size from the
  MIU, `memlx`, `sf`, `ping`, TFTP. A flash holding the environment of
  OpenIPC/u-boot-sigmastar (stale `memlx` and `memsz` included): it loads, the
  values are refreshed, the saved `bootcmd` boots the kernel.
- The infinity6* U-Boots loaded in DDR on `ssc029`, with the MIU size register
  set by a stub (the IPL does not run): banner, DRAM, the 64 MiB top, `memlx`
  and `memsz` for 6, 6b0, 6e and 6c; SD at bank `0x1413` for 6, 6b0 and 6e.
  With its bond test inverted in a scratch copy, 6e's `sf` on the FSP at `0x100D`
  erases and reads; an unaligned write came back one byte short per partial
  page because `ssc029`'s FSP counts OUT_SIZE plus one. infinity6e does not:
  its vendor `halSERFLASH.c` writes the plain length, and `ssc338q` below is
  clean.
- SD with the `ssd201` U-Boot on the `msc313e` host: `mmc info`, write, read.
- On the per-die machines of qemu-hisilicon branch `sigmastar-i6-dies` (not
  merged at the time), `ssc325`, `ssc337`, `ssc338q` and `ssc377`, each also
  with the published vendor image as a control. Real IPL chain; `sf` erase, an
  unaligned write of 74565 bytes, read back, `cmp.b`, the raw flash file
  compared; `saveenv`, `reset`; ping and TFTP; SD; an OpenIPC kernel to `login:`
  (not past it, so no shell-side checks); a saved vendor environment upgraded.
  `memlx` and `memsz` per RAM size match the vendor's on 6 and 6b0 (64-256),
  6e (128-1024) and 6c (64-256). At 512 MiB the vendor's infinity6c falls
  back to its 64 MiB values; this takes the largest row under the size
  (256 MiB), as for any size a table lacks. The other parts of these dies reach their
  prompts, and `ssd201`/`ssd202d` pass SD at `0x1410`.
- SPI NAND, in the 768k/256k/UBIFS layout: from a blank flash, the scan
  finds no UBI, `fwupd` takes `rootfs.ubifs.<soc>` over TFTP into a new
  `rootfs` volume (and `rootfs_data`), the next scan finds `/boot/uImage`,
  and the kernel mounts `root=ubi0:rootfs` to `login:`, again after `saveenv`
  and `reset`: `ssc325de`, `ssc337de`, `ssc338q` with the published firmware's
  kernel and rootfs (repacked as UBIFS, the kernel in `/boot`), `ssc377` with
  mstar-06's Maruko kernel. A test UBIFS needs a journal its volume can hold
  (`mkfs.ubifs -j`), or U-Boot refuses its superblock. That Maruko kernel has
  no overlayfs, so `/init` leaves `/` the UBIFS rootfs and inittab remounts it
  read-write; killed then, its journal is dirty and U-Boot's read-only UBIFS
  cannot recover the master node, so the scan sends it to recovery, as on
  HiSilicon. Earlier, in the vendor layout, on `ssc337de`: `nand` erase,
  write and read against the raw file; a factory bad block and `markbad`,
  surviving a reboot, UBI seeing both; an injected uncorrectable read (-74), a
  corrected one and a failed erase; SD recovery from a FAT card, also on NOR
  (`ssc337`). A bad first eraseblock in `ubi` still finds the marker.
- Every image, NOR and NAND, as CI's smoke test runs it (two boots to the
  autoboot prompt, the second loading the environment the first saved), on
  its own machine: 52 of 52.
- **Not run on any machine:** media (ISP, encoders); `ssc378*` kernels; 4 KiB page
  NAND; the infinity2m card power pad as a real pad (PAD_TTL0, taken as already
  GPIO). A `ssc338q` at bond 0 (QFN88) boots U-Boot, but both the vendor's and
  this U-Boot then lose the kernel: see "Things to keep". From NAND at bond 0,
  both images stop in the vendor's IPL (`init failed!`).

## Things to keep

- infinity6e's bond: U-Boot follows the strap (FSP `0x16` at QFN88), but the
  vendor kernel's `halSERFLASH.c` hard-codes `u8BoundID = 0x10` and always takes
  FSP `0x100D`, so a QFN88 board panics at root mount under qemu. Reported by the
  qemu work; the only package evidence is that vendor's configs list QFN88 as
  FPGA-only (`SSC012A`), which needs a real board to settle.
- `mem=` stays the first kernel argument on infinity3, as on the other
  OpenIPC targets. The later dies have none (see "Hand-off").
- `u-boot-ssc*` must not reach OpenIPC/firmware `latest` from here before a
  board has run them.
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
