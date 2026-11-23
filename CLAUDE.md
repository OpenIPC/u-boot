# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this branch is

OpenIPC's U-Boot for Rockchip's camera SoCs. The tree is **mainline U-Boot
v2022.10**, the same release as the other `openipc-*` branches, plus:

- the OpenIPC boot layer: `common/openipc.c` and `include/openipc.h` as
  `openipc-axera` has them (the copy kept in step with `openipc-hisilicon`
  that also drives mainline's SPI NAND, `CONFIG_MTD_SPI_NAND`);
- `include/configs/openipc-common.h`, the OpenIPC default environment,
  with the per-SoC values taken from `include/configs/<soc>.h`. It gained
  `OPENIPC_CONSOLE` (the kernel's `console=`, `ttyS0,115200` unless the SoC
  says otherwise);
- the SoC support under `arch/arm/mach-rockchip/<soc>/`,
  `board/rockchip/rv11xx/`, `arch/arm/dts/<soc>-openipc*.dts*`.

| SoC | Targets | Status |
| --- | --- | --- |
| RV1106 / RV1103 (Cortex-A7, 64-256 MiB in package) | `rv1106`, `rv1103` (NOR and NAND images) | qemu-bench `-M rv1106`: SPL, U-Boot, env across reset, Ethernet, SD, recovery; **no silicon** |
| RV1103B / RV1106B (Cortex-A7, 64 MiB in package, peripherals from 0x20000000) | `rv1103b` (NOR and NAND images) | qemu-bench `-M rv1103b`: the same checks; **no silicon** |
| RV1126 / RV1109 (4 / 2 Cortex-A7, DDR3 up to 2 GiB) | `rv1126`, `rv1109` (NOR and NAND images) | qemu-bench `-M rv1126`: SPL, U-Boot, env across reset, Ethernet, SD, recovery; **no silicon** |

`git show origin/openipc-hisilicon:CLAUDE.md` covers the OpenIPC layer
(default environment, `common/openipc.c`, env compatibility, the NOR and
NAND layouts); it applies here unchanged.

## Relationship to mainline

```sh
git diff --stat v2022.10 HEAD
git diff --diff-filter=M v2022.10 HEAD      # the mainline files patched
```

Mainline v2022.10 has no RV1106. Its SoC code is ported from Rockchip's
vendor U-Boot (rockchip-linux/u-boot `next-dev`) to the v2022.10 driver
model (`priv_auto`, `of_to_plat`, `DM_DRIVER_GET`, `dev_set_priv`):
`arch/arm/mach-rockchip/rv1106/` (the SPL's `arch_cpu_init`: firewall,
watchdog, reset and PVTPLL setup; the debug UART's pins),
`drivers/clk/rockchip/clk_rv1106.c`, `drivers/pinctrl/rockchip/pinctrl-rv1106.c`,
`arch/arm/include/asm/arch-rockchip/{cru,grf,ioc}_rv1106.h`,
`include/dt-bindings/clock/rv1106-cru.h`, and the same set for the RV1103B
(`rv1103b/`, `clk_rv1103b.c`, `pinctrl-rv1103b.c`,
`rockchip,rv1103b-cru.h`).

Patched mainline files:

- `arch/arm/mach-rockchip/{Kconfig,Makefile}` (`ROCKCHIP_RV1106`),
  `drivers/{clk,pinctrl}/rockchip/Makefile`, `arch/arm/dts/Makefile`;
- `arch/arm/mach-rockchip/spl-boot-order.c`: a flash on a SPI bus that no
  SPL SPI-flash driver takes (SPI NAND) is `BOOT_DEVICE_SPI`;
- `common/spl/spl_fit.c`, `lib/lzma/LzmaTools.c`: LZMA images in the SPL's
  FIT (backport; U-Boot and its device tree are LZMA in `u-boot.itb`);
- `drivers/pinctrl/rockchip/pinctrl-rockchip.h`: `PIN_BANK_IOMUX_FLAGS_OFFSET`
  (backport);
- `drivers/net/{Kconfig,Makefile}`, `drivers/net/dwc_eth_qos.{c,h}`: the
  `rockchip,rv1106-gmac`, `rv1103b-gmac` and `rv1126-gmac` entries,
  `DWC_ETH_QOS_ROCKCHIP` and its glue `drivers/net/dwc_eth_qos_rockchip.c`
  (a table per SoC: the GRF's RMII bits and the SoC's own 10/100 PHY, or
  the GRF interface select and the CRU speed clock for an external PHY;
  Linux's `snps,reset-gpio`), the way `openipc-axera` has
  `dwc_eth_qos_axera.c`;
- `common/{Kconfig,Makefile}` (`OPENIPC`, `PRODUCT_SOC`, `PRODUCT_SOCMODEL`,
  `OPENIPC_NOR_SPLIT`), `.gitignore` (`/output/`);
- `scripts/dtc/pylibfdt/Makefile`: the version without the `u-boot-`
  prefix, which current setuptools rejects.
- `drivers/pinctrl/rockchip/`: mainline's v2022.10..v2023.04 change to the
  core (mux routes moved into it, `IOMUX_L_SOURCE_PMU`,
  `PIN_BANK_IOMUX_FLAGS_OFFSET`) and to the drivers it touched, for the
  RV1126's;
- `tools/rkcommon.c`: mkimage's `rv1126` entry (`110B`, header v1), from
  v2023.04;
- `drivers/mtd/nand/spi/gigadevice.c`: the GD5F2GQ4UExxG (0xd2), from later
  mainline (the RV1109 camera's SPI NAND);
- `drivers/gpio/rk_gpio.c`: v2024.01's, which drives the v2 GPIO banks
  (hiword-masked, low and high halves) these SoCs have; v2022.10's writes
  the v1 layout. The RV1126's PHY reset is the first user.

The RV1126 comes from mainline v2023.04 (`arch/arm/mach-rockchip/rv1126/`,
`clk_rv1126.c`, `pinctrl-rv1126.c`, `cru_rv1126.h`, `grf_rv1126.h`, the
bindings) without its TPL: rkbin's DDR blob initialises the DRAM, as on
the other two.

Vendor driver changes worth knowing:

- `clk_rv1106.c` takes `ARMCLK` as the APLL. Mainline probes the reset and
  sysreset devices, which share the CRU's node, with the CRU's
  `assigned-clocks`; the vendor driver had no `ARMCLK` and failed them.
- Its reset device is bound at `PMUSOFTRST_CON00` (0xa00) with banks to the
  CRU's end: the binding's IDs are `(offset - 0xa00) / 4 * 16 + bit` over
  every CRU block (`SRST_A_MAC` is VO 0x1ca04 bit 8). The RV1103B's count
  the same way from PERICRU's 0xa00 to PMU1CRU at 0x80000.
- `clk_rv1103b.c` has no `CLK_GPLL_DIV12`, which the vendor tree assigns
  100 MHz: `rv1103b-openipc.dtsi` leaves it out (the kernel sets it).

## Build

```sh
./build.sh            # every SoC
./build.sh rv1106     # -> output/u-boot-rv1106-{nor,nand}.bin
./build.sh rv1103b    # -> output/u-boot-rv1103b-{nor,nand}.bin
./build.sh rv1126     # -> output/u-boot-rv1126-{nor,nand}.bin

Every SoC name gets its own NOR and NAND images: OpenIPC's ubwrite
fetches u-boot-${soc}-<flash>.bin by CONFIG_PRODUCT_SOC. The RV1103 and
RV1109 use their die's config fragment and device trees (build.sh's
dt_soc); only CONFIG_PRODUCT_SOC differs.
```

As on `openipc-hisilicon`, `build.sh` assembles `.config` from
`openipc/openipc_config`, `openipc/<soc>_config`, `openipc/nand_config`
(NAND only), the device tree and `CONFIG_PRODUCT_SOC`, then runs
`olddefconfig` and `make`. Binman needs swig, python3-dev, pyelftools and
`lzma_alone` (`lzma-alone` on Debian). The toolchain is
`arm-linux-gnueabi-` (`CROSS_COMPILE`).

The image, written at flash 0, is the boot partition:

- **ID block**: `openipc/rockchip-idb.py` packs it in the current format
  (`RKNS`, header v2: SHA-256 per image, the header's hash at 0x600) from
  the DDR-init blob, `openipc/rv1106_ddr_924MHz_v1.16.bin` (rkbin; its
  licence `openipc/rkbin-LICENSE` allows redistribution), and the SPL. The
  mask ROM runs the blob from SRAM, then loads the SPL at DRAM 0.
- **`u-boot.itb`**: the FIT (binman, `rv1106-u-boot.dtsi`) the SPL loads:
  U-Boot and its device tree, both LZMA.
- **NOR**: the ID block at 0x8000, `u-boot.itb` at
  `CONFIG_SYS_SPI_U_BOOT_OFFS` (0x1a000), all before the environment at
  0x40000. **Unverified on silicon**: the ROM is only known to take an ID
  block at 0x10000 (both XM camera dumps); 0x8000 is Rockchip's NOR layout.
- **NAND**: the ID block in block 0 and again in block 1, `u-boot.itb`
  from block 2 (0x40000), in the 768 KiB boot partition. The SPL reads it
  with `board/rockchip/rv11xx/spl_spi_nand.c` (v2022.10 has no SPI-NAND
  loader for the SPL), skipping bad blocks.

`build.sh` refuses an image whose ID block overruns `u-boot.itb` or whose
`u-boot.itb` overruns the boot partition. The NOR images are tight (6 to
10 KiB to spare). U-Boot has no gzip (`openipc/openipc_config`): OpenIPC's
Rockchip kernels are self-decompressing zImages, held uncompressed in a
FIT or uImage, so a gzip-compressed kernel image will not boot. The NAND
build keeps zlib for UBIFS. Also the device tree is `rv1106-openipc.dtsi`, the SoC as U-Boot sees
it, not the vendor's whole `rv1106.dtsi` (37 KiB), and the commands no
OpenIPC script uses are off (`openipc/openipc_config`). LTO saved only
4 KiB, and grew the SPL.

## RV1106 / RV1103

### Boot

- The SPL runs at DRAM 0. After init its stack is at 9 MiB with a 1 MiB
  simple heap below it (the FIT and LZMA buffers), clear of the ATAGs at
  0x1fe000 and U-Boot at 2 MiB.
- It boots from the flash the ROM came from (`u-boot,spl-boot-order =
  "same-as-spl"`): the boot source at SRAM 0xff6c0010 maps through
  `boot_devices[]` (`rv1106.c`) to the SFC's flash.
- U-Boot takes the DRAM size from the DDR blob's `ATAG_DDR_MEM` at
  0x1fe000 (`board/rockchip/rv11xx/rv11xx.c`).

### Layout and environment

- **NOR**: OpenIPC's (env at 0x40000, the kernel at 0x50000).
- **NAND**: `768k(boot),256k(env),-(ubi)`. The mainline spi-nand device is
  registered as `nand0` in `board_nand_init()`, as on `openipc-axera`.
- **`osmem`**: when unset, the board sets it to all of the DRAM. The
  Rockchip kernel reserves media memory itself (CMA from its device tree)
  rather than behind `mem=`.
- **Console**: `ttyFIQ0,115200`, the vendor kernel's FIQ debugger on UART2.
- **Reset**: the CRU's `GLB_SRST_FST` (0xff3b0c08 = 0xfdb9, mainline's
  `rockchip_sysreset`).

### Hardware

- **UART2**: 0xff4c0000, 24 MHz.
- **SFC**: 0xffac0000, NOR (`jedec,spi-nor`) or NAND (`spi-nand`), 75 MHz.
- **GMAC**: DWC QoS 4.20a, 0xffa80000. It runs RMII to the SoC's own
  10/100 PHY at MDIO address 2, enabled through the VO GRF `MACPHY_CON0`.
  The bandgap trim is left at 0; the kernel trims it from the OTP.
- **SD**: DW MMC, 0xffaa0000, pins on GPIO3_A1-A7 (pinctrl).

### Verification (qemu-bench `-M rv1106`)

```sh
# NOR: 16 MiB, the image at 0
qemu-system-arm -M rv1106,flash-file=nor.bin -nographic \
    -nic user,tftp=<dir> -drive if=sd,index=0,file=sd.img,format=raw
# NAND: 128 MiB W25N01GV
qemu-system-arm -M rv1106,flash-file=nand.bin,flash-type=w25n01gv ...
```

Checked on both images:

- two boots: the first saves the default env, the second loads it (`OK`)
  and keeps a variable set before `reset`;
- `dhcp` and TFTP (CRC32 checked);
- `fatload` from the card (CRC32 checked);
- `script.txt` from the card run by the recovery on a firmware-less boot.

## RV1103B / RV1106B

The RV1106's boot chain and OpenIPC layout on a die with its peripherals
from 0x20000000; the RV1106's sections above apply, with these values.

- **ID block**: `openipc/rv1103b_ddr_924MHz_v1.07.bin` (rkbin, the
  `RV1103BMINIALL.ini` blob) and the SPL. Boot source at IRAM 0x210f6010.
- **SPL** (`arch/arm/mach-rockchip/rv1103b/rv1103b.c`): the secure timer
  at 0x20500000 (its control at +0x4, not mainline's +0x10), the firewall,
  watchdog and global-reset setup, and the HPMCU, which the mask ROM
  leaves running, put back in reset before U-Boot
  (`spl_board_prepare_for_boot`).
- **Console**: UART0 at 0x20540000; the kernel's is `ttyFIQ0,115200`.
- **Reset**: TOPCRU `GLB_SRST_FST`, 0x20060c08.
- **Flash**: the FSPI at 0x20d40000, mainline's `rockchip,sfc` driver.
- **GMAC**: 0x20800000. The GRF registers are in SYS GRF (0x20150000:
  `GMAC_CLK_CON` +0xa4, `MACPHY_CON0/1` +0xb0/+0xb4), and RMII mode is
  bit 2. The glue picks them by compatible.
- **SD**: 0x20d20000, pins on GPIO1_A6-B4.

Checked on qemu-bench `-M rv1103b` (64 MiB): both images, two boots with
the env kept, `osmem=64M`, DHCP/TFTP and SD (CRC32), and `script.txt`
recovery.

## RV1126 / RV1109

The same boot chain and OpenIPC layout, with these differences.

- **ID block**: the older format (header v1: an RC4-scrambled header,
  `110B`), which `build.sh` makes with mainline's
  `mkimage -n rv1126 -T rksd -d ddr:spl`, the way mainline documents it for
  the RK3399 with rkbin's blob. The blob is
  `openipc/rv1126_ddr_924MHz_v1.16.bin` (rkbin `RV1126MINIALL.ini`; it
  starts with `110B`). Boot source at IRAM 0xff700010.
- **NOR**: the 36 KiB DDR blob leaves little room, so the SPL drops the
  clock framework and its reset driver. The DDR blob has set the PLLs,
  the SFC and UART run on the rates it left, and `reset_cpu()` is
  `rv1126.c`'s. The SPL goes from 47 to 36 KiB, and `u-boot.itb` follows
  the ID block at 0x1a800 (`openipc/rv1126_config`). The image ends at
  0x3e749, 6.2 KiB before the environment.
- **U-Boot at 0x600000**, as mainline and the vendor have it. The SPL runs
  at 0, and its stack, BSS and heap are as on the RV1106.
- **Console**: UART2 M1 at 0xff570000, whose pins mainline's
  `board_debug_uart_init` sets. The kernel's console is `ttyFIQ0,115200`.
- **Clocks**: the CRU takes the GPLL from the PMU CRU, so both CRUs and
  both GRFs are pre-relocation devices.
- **Reset**: `GLB_SRST_FST`, CRU 0xff490408.
- **SFC**: 0xffc90000 (the node is the vendor's; mainline has none).
- **SD**: 0xffc60000.
- **Ethernet**: an external PHY, configured in `rv1126-openipc-nand.dts`
  as the RV1109 SPI NAND camera's kernel device tree has it. That is RMII
  to the PHY at MDIO address 0, with the SoC driving the 50 MHz reference
  (`CLK_GMAC_SRC` from M0, `CLK_GMAC_TX_RX` in RMII mode). The pins are
  `rmiim0` plus the M0 clock-out on GPIO3 (function 2). The PHY reset is
  GPIO3_C5, active low, with 50 ms each side. The vendor U-Boot's own
  default, RGMII on M1, is its EVB's, not the camera's. The GPIO bank
  node is `gpio3@...` with `gpio-ranges`, from which `rk_gpio` takes the
  bank.

Checked on qemu-bench `-M rv1126` (1 GiB, the 256 MiB GD5F2GQ4UB model):
two boots with the env kept, `osmem=1024M`, DHCP/TFTP and `fatload` from
the card (CRC32), `script.txt` recovery, and the PHY reset released
(GPIO3 C5 an output, high).
