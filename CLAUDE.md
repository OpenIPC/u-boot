# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this branch is

`openipc-anyka` is OpenIPC's U-Boot for Anyka's ARM926EJ-S camera SoCs:
the AK3918EV200 (Cloud39E), the AK391xEV300 family (AnyCloud39EV300) and
the AK391xEV330 family (AnyCloud V500). The tree is **mainline U-Boot
v2022.10**, the same release as `openipc-hisilicon`, `openipc-xm` and
`openipc-gk710x`, plus:

- the OpenIPC boot layer, carried over from `openipc-xm`, which took it from
  `openipc-hisilicon`;
- the AK39 support, written from the vendor sources: the AnyCloud39EV300
  SDK (U-Boot 2013.10 `arch-ak39`, kernel 3.4.35 `mach-ak39`), the
  AK3918EV200 4.4 kernel port and its U-Boot port, and the EV330 4.4 device
  trees.

No vendor driver comes along. The EV330's first boot stage, a vendor
binary, is carried (see "Boot").

CI and user docs live on the `openipc` branch:

- `.github/workflows/ci.yml` only calls `build-anyka.yml@openipc`.
- `git show origin/openipc:CLAUDE.md` covers the workflow invariants.
- `git show origin/openipc-hisilicon:CLAUDE.md` covers the OpenIPC layer
  (default environment, `common/openipc.c`, env compatibility). It applies
  here unchanged.

Adding or dropping a SoC needs both branches:

- here: `build.sh` (`build_soc`, `ALL_SOCS`), `openipc/<soc>_config`, and
  `TARGET_*` in `arch/arm/mach-anyka/Kconfig`;
- there: `build-anyka.yml`'s matrices and the README.

AK37D/AK37E appear only as `#ifdef`s in the vendor trees; they have a
different memory map and are not supported, nor modelled by qemu-hisilicon.

## Relationship to mainline

```sh
git diff --stat v2022.10 HEAD                  # the whole delta, small
git diff --diff-filter=M v2022.10 HEAD         # the mainline files we patched
```

**Added**

| Area | Location |
| --- | --- |
| SoC: ASIC clock, timer, watchdog reset, DRAM size | `arch/arm/mach-anyka/` (`ARCH_ANYKA`; `TARGET_AK39xxEVyyy`, `AK39_EV200/EV300/EV330`; register map in `include/mach/anyka.h`) |
| Board: Ethernet pins and clocks, `misc_init_r` → `openipc_helper()` | `board/anyka/ak39/` |
| UART0, non-DM | `drivers/serial/serial_ak39.c` (`AK39_SERIAL`) |
| SPI NOR on SPI0, non-DM | `drivers/mtd/spi/ak_sflash.c` (`AK_SFLASH`). It provides `spi_flash_probe()` and fills `mtd->_read/_write/_erase`, as `gk_sflash` does on `openipc-gk710x` |
| Memory map, timer | `include/configs/anyka.h`, ending in `#include "openipc-common.h"` |
| OpenIPC boot layer | `common/openipc.c`, `include/openipc.h` (as on `openipc-xm`), `include/configs/openipc-common.h` (Anyka rewrite, NOR only, same env commands) |
| Kconfig fragments, boot headers, the build | `openipc/`, `build.sh` |

**Patched mainline files**, each small:

- `arch/arm/{Kconfig,Makefile}`: the `ARCH_ANYKA` hook.
- `common/{Kconfig,Makefile}`: `OPENIPC`, `PRODUCT_SOC`, `PRODUCT_SOCMODEL`.
- `drivers/serial/{Kconfig,Makefile,serial.c}`: `AK39_SERIAL` and its
  registration in the legacy serial list.
- `drivers/mtd/spi/{Kconfig,Makefile}`, `cmd/Kconfig` (`CMD_SF`) and
  `env/Kconfig` (`ENV_IS_IN_SPI_FLASH`): accept `AK_SFLASH`.
- `drivers/net/Kconfig`: `NETDEVICES` selects `DM_ETH` only when `DM` is on,
  as on `openipc-xm`.

Ethernet is mainline DesignWare.

## Boot

The memory map is the same in all three generations: system control at
`0x08000000`, peripherals from `0x20000000`, 8 KiB of L2 memory at
`0x48000000`, DDR2 (in the package) at `0x80000000`.

- **EV200 and EV300.** The mask ROM reads the 512-byte header at flash
  offset 0, replays its register script (PLL, DDR controller), copies the
  payload that follows to the load address, `0x80E00000`
  (`SYS_TEXT_BASE`), and starts it there. DDR is up before the first
  instruction: there is no SPL and `lowlevel_init` only ungates clocks and
  releases resets.
  - `openipc/ak39-image.py` builds the header: magic `AKSH_240` (EV200) or
    `ANYKA_H3` (EV300), the payload length, the load address, the script
    from `openipc/ak3918ev200.rvs` / `ak39ev300.rvs`, and an SPIP flash
    descriptor.
  - The scripts are the ones on every camera flash of that generation seen
    (`ak39ev300.rvs` is also the SDK's `AKinit.S`; `ak3918ev200.rvs` the
    EV200 U-Boot port's `ddr-script-ak3918ev200.txt`).
  - The SPIP descriptors are the vendor's, from the flash each script was
    read from. Their `table_block` points at the vendor partition table's
    block, which here is inside U-Boot; OpenIPC kernels take `mtdparts=`
    from the command line and do not read it.
- **EV330.** The ROM loads a 4 KiB first stage into L2 memory. It runs
  the DDR script of its own header, then copies 0x40000 bytes from flash
  offset `0x1200` to `0x80E01000` and starts them.
  - `openipc/ak39ev330-boot.bin` is those first 0x1200 bytes (ROM header
    and first stage) from an AK3919EV330 flash; the AK3918EV330 dumps
    differ from it only in the SPIP descriptor.
  - U-Boot is linked at `0x80E01000` (`ak39ev330`'s `SYS_TEXT_BASE`), and
    `BOARD_SIZE_LIMIT` keeps it under 0x40000 - 0x1200 bytes: the first
    stage then reads into the environment at 0x40000, which is harmless.
- **DRAM size** is the part's: 128 MiB on the AK3919s, 64 MiB on the
  others (`PHYS_SDRAM_1_SIZE`). `dram_init()` probes only within it; DDR
  past the end is not known to alias, and qemu-hisilicon does not model it.
- The image is in CPU byte order, as `sf write` stores it and an external
  programmer writes it.

## Hand-off to the kernel

- EV200 and EV300 kernels (3.4.35, OpenIPC's `ak3918ev300` included) take
  ATAGs, machine number 3892 (`ANYKA_AK39XX`, every vendor board file).
- EV330 kernels (4.4) take a device tree. The vendor keeps it in its own
  partition; OpenIPC's flow (`bootflash`) boots the uImage alone, so a DT
  kernel needs its DTB appended.
- **Vendor uImages** carry entry = load + 0x40: the vendor U-Boot reads
  them to the load address and runs the data in place. Mainline moves the
  data to the load address and enters 0x40 bytes into the zImage, which
  hangs. OpenIPC builds entry = load; vendor kernels need repacking with
  `mkimage -e <load>`.
- `mem=` is the whole DDR (`osmem` 64M/128M), as the vendor passes it. The
  MTD name is `spi0.0` (`SFC`): the vendor kernels' `ak_spiflash` takes
  the SPI device's name. The console is `ttySAK0`.

## Peripherals

- **Console:** UART0. It has no data register: TX bytes go into L2 memory
  at `0x48001000`, received ones into a 128-byte ring at `0x48001080`,
  and `DATA` bits 19:13 say where the next one lands. `tstc()` compares
  that with the next byte to read, as `getc()` does; the vendor's looked
  at the RX threshold flag, which says only that something arrived. The
  divider comes from the ASIC clock (`ak_asic_clk()`, from the PLL word
  at `0x08000008`). UART0's shared-pin bits are 1-2 on the EV200 and 2-3
  on the EV300 and EV330.
- **Clocks:** on the EV200, `arch_cpu_init()` sets the ASIC PLL to CPU
  400 / memory 200 / ASIC 100 MHz (`0x018213c8`), as the EV200 vendor
  U-Boot and the EV200 U-Boot port both do; out of reset an AK3918EV200
  runs its peripherals at 60 MHz. The EV300 and EV330 keep what the boot
  header set.
- **Timer:** timer 1, a 32-bit down counter off the 12 MHz crystal, read
  through `READ_SEL`; `lib/time.c` does the rest.
- **Reset:** the watchdog, armed as the vendor U-Boots arm it: an 8 s
  count, enable, enable + feed, then the same with a count of 1. A single
  "count 1, enable + feed" write hangs a real AK3918EV200.
- **SPI NOR:** SPI0, a byte-wise PIO master: CON picks send-only (`ARRM`)
  or receive-only (`TGDM`), CNT starts a transfer, 8-byte FIFOs a word at
  a time. Opcode and address are plain TX bytes. 64 KiB erase, 3-byte
  addresses (up to 16 MiB), block protection left by a vendor loader is
  cleared. Chip select is CON bit 5 (EV330 kernels drive it on GPIO 25
  instead).
  - It does what the vendor U-Boots do, traced under qemu from the gwell
    camera's EV200 U-Boot: SPI0 soft reset (bit 5) and its pads (`0x74`
    bit 25; `0xDC` 13:0 = `0x2003` on the EV200 (the DT's), `|= 0x3e03`
    on the EV300, the ROM's on the EV330) at each probe, fast read (`0x0B` + a dummy byte),
    at most 8 KiB per CNT transfer (`MAX_XFER_LEN`), about 17 MHz.
  - Fixed after a real AK3918EV200 (XM25QH64A) read back a kernel written
    with an earlier build with a bad data CRC. qemu never showed it: its
    SPI master stalls on a full FIFO, the real one is not known to.
- **Ethernet:** the Synopsys GMAC at `0x20300000`, a PHY on MDIO.
  `board/anyka/ak39/ak39.c` does what the vendor U-Boots do first: the MAC
  pins (GPIO 10-24, shared-pin register `0x7C`), the PHY clock out on GPIO
  47, the MAC clock (`0x14`), a MAC soft reset (bit 13).
  - EV300, from the SDK: RMII, 50 MHz, and a PHY reset pulse on GPIO 0.
    The EV330 has the same shared-pin layout (its 4.4 pinctrl table); that
    it needs exactly this is unverified.
  - EV200, from the V380E2 camera's vendor U-Boot, traced under qemu while
    it pinged: MII (`0x575f5a5`, the SDK's MII value too), 25 MHz, no PHY
    reset. Shared-pin register 0 is one bit narrower at the bottom than
    the EV300's, so GPIO 47's field is bits 10:9, not 11:10 (the EV200 4.4
    pinctrl table agrees; the 3.4 kernel keeps the old value commented
    out). The MAC pins in `0x7C` are the same in both tables.
  - MII vs RMII is the board's choice; these are the vendors' defaults.
- **SD:** MCI0 (`drivers/mmc/ak_mmc.c`, `MMC_AK39`), 1-bit, PIO through
  the FIFO, up to 20 MHz. After the EV200 U-Boot port's DM driver. Every
  vendor source wires a 1-bit bus on pads 31-33: `0xDC |= 0x1c0` (on the
  EV200 pad 33's field is two bits, so bit 9 is cleared too), which does
  not touch SPI0's pads. Card detect is a board GPIO (5, 24, 41, 57 in the
  vendor sources) and is not used: with no card the first command times
  out (one "did not respond" line, no delay).
  - EV200: the SD driver also muxes pad 34 to GPIO and drives GPIO 34 low
    (`wifi_en` in the EV200 reference board's DT, on the pad that is
    otherwise DAT1), and SPI0's pads 35/36 are function 2 (`0xDC` bits
    13:12 = `10`, as the DT and Linux set them). On a gwell AK3918EV200 the
    card answers only with both: the ROM leaves pad 34's field at `11`, and
    the gwell vendor U-Boot's SPI value `0x3003` puts `11` on pads 35/36.
    Found by setting the registers Linux leaves (`devmem`) one by one.
  - `common/openipc.c` recovers from it at every boot: `script.txt`, then
    `uImage.<soc>.img` and `rootfs.squashfs.<soc>.img`. These are uImages
    flashed where their load address says, without the header
    (`mkimage -T firmware -a 0x50000 -d uImage.<soc> uImage.<soc>.img`,
    `-a 0x250000` for the rootfs); nothing checks the offset, that is the
    card maker's call. A FIT kernel goes to `kernaddr`. An image the flash
    already holds is skipped, so a card left in reflashes nothing.
    `fatload mmc 0` and `srcmmc` work as on the other targets.

## Build

```sh
bash build.sh               # all six; continues past failures, exits non-zero if any failed
bash build.sh ak3918ev300   # one or more SoCs (CROSS_COMPILE defaults to arm-linux-gnueabi-)
```

- The output is `output/u-boot-<soc>-nor.bin`, built out of tree in
  `output/build-<soc>-nor/`.
- `.config` is `openipc/openipc_config`, then `openipc/<soc>_config`, then
  `olddefconfig`.
- There are no `configs/*_defconfig` for these targets.
- ARM926 has no Thumb-2: the images are ~233 KiB (EV330 ~237 KiB with its
  first stage). `fdt`, `bootelf`, `imxtract`, `random`, `loads` and `nfs`
  are left out to make room for SD. `BOARD_SIZE_LIMIT` keeps header + image under the 256 KiB
  boot partition. Check the size after enabling anything.

## Verification

The emulator is OpenIPC/qemu-hisilicon, machines `ak3918ev200`,
`ak3916ev300`, `ak3918ev300`, `ak3919ev300`, `ak3918ev330`, `ak3919ev330`
(`docs/anyka.md` there). `-M <soc>,flash-file=<image>` does the mask ROM's
job (on the EV330 including loading the first stage), so it boots exactly
the flashed bytes.

- **CI smoke:** two boots of an 8 MiB image holding only U-Boot. The second
  must load the environment the first saved.
- **Checks run by hand** (all passed under qemu-hisilicon):
  - An 8 MiB image with `uImage` at 0x50000 and rootfs at 0x250000 from an
    OpenIPC/firmware `ak3918ev300_lite` build reaches a shell on
    `ak3916ev300`, `ak3918ev300` and `ak3919ev300`; `/proc/mtd` matches
    `mtdparts`; `fw_printenv`/`fw_setenv` work.
  - `sf erase/write/read` + `cmp.b` with an unaligned offset and length.
  - `saveenv` + `reset` (the watchdog).
  - TFTP recovery from a flash holding only U-Boot
    (`-nic user,net=192.168.1.0/24,host=192.168.1.107,tftp=<dir>`).
  - The boot limit: two `reset`s, then `altbootcmd` boots with `failsafe`.
  - `ping` to the qemu host from U-Boot on `ak3918ev200` and
    `ak3918ev300`.
  - SD (`-drive if=sd,format=raw,file=<FAT image>`): `mmc info`, `fatls`,
    `script.txt` at boot on `ak3918ev200` and `ak3919ev330`; recovery of
    kernel and rootfs from an empty flash to login on `ak3918ev300`, and
    nothing rewritten on the next boot with the card still in.
  - Vendor kernels (repacked with entry = load): the V380E2 camera's
    3.4.35 on `ak3918ev200` and the AK3919EV330 camera's 4.4.192 with its
    DTB (`fdt_high=0xffffffff`, as its vendor environment sets) reach
    their login prompts.
- **What qemu does not check:** DDR timing (the scripts are the vendor's,
  so this is low risk), the PLL and so the UART and SPI dividers, pad
  control, the Ethernet clock and PHY reset, the SPI and MCI controllers'
  real timing.
- **On silicon:** an AK3918EV200 camera (gwell, XM25QH64A, no Ethernet)
  booted the first build to the prompt; its flash reads and its reset are
  what the SPI and watchdog changes above address.
- qemu's UART pushes input with no flow control: a pasted line longer than
  the 128-byte RX ring loses bytes. Type or send long lines in pieces.

## Things to keep

- `mem=` stays the first kernel argument, as on the other OpenIPC targets.
- The boot counter is the DRAM word at `0x81F20000`, like the other
  families' DDR + 0x1f20000. A healthy boot does not clear it, and the
  `ak3918ev300` firmware ships no openipc-failsafe. After two reboots the
  boot limit path therefore boots the same kernel with `failsafe`, which
  that firmware ignores, as on `openipc-xm` and `openipc-gk710x`.
- Variables U-Boot writes before relocation must be initialised (in
  `.data`): `.bss` is not cleared until after relocation. U-Boot runs from
  DDR from the first instruction, so `.data` is writable then.

## Conventions

- One topic per commit, `<area>: <what changed>` (`anyka:`, `ak_sflash:`,
  `openipc:`, `net:`, `ci:`).
- Commit bodies say what was verified, and where: the qemu machine, or a
  board.
