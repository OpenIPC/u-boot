#!/bin/sh
#
# Build U-Boot for the SoCs named on the command line, or for every
# supported SoC when invoked without arguments. Failures are collected:
# every requested SoC is attempted and the script exits non-zero if any
# of them failed.

cd "$(dirname "$0")"

export ARCH=arm
export CROSS_COMPILE=${CROSS_COMPILE:-arm-linux-gnueabi-}
# The NVR dies' U-Boot is an AArch64 one (Debian/Ubuntu: gcc-aarch64-linux-gnu)
CROSS_COMPILE_A64=${CROSS_COMPILE_A64:-aarch64-linux-gnu-}
# The loader is bare-metal code linked against Novatek's libraries:
# a bare-metal toolchain if there is one, else U-Boot's
if [ -z "${LOADER_CROSS_COMPILE}" ]; then
    if command -v arm-none-eabi-gcc >/dev/null; then
        LOADER_CROSS_COMPILE=arm-none-eabi-
    else
        LOADER_CROSS_COMPILE=${CROSS_COMPILE}
    fi
fi

BOOT_SRCDIR=$(pwd)

mkdir -p output

# Concatenate the config fragments into the build directory's .config;
# later fragments override earlier ones.
mkconfig() {
    builddir=$1
    shift
    mkdir -p "${builddir}"
    cat "$@" > "${builddir}/.config"
}

# The SDK's loader of a die (board/novatek/<die>/loader), built in a copy:
# its make system writes into its tree. The rest are its make variables
# (CHIP=, DRAM1_CLK=, ...).
build_loader() {
    loaderdir=$1
    die=$2
    shift 2

    rm -rf "${loaderdir}"
    cp -a "${BOOT_SRCDIR}/board/novatek/${die}/loader" "${loaderdir}"
    if ! make -C "${loaderdir}/MakeCommon" release \
            CROSS_COMPILE="${LOADER_CROSS_COMPILE}" "$@" \
            >"${loaderdir}.log" 2>&1 </dev/null; then
        tail -n 30 "${loaderdir}.log" >&2
        return 1
    fi
}

# Novatek NA51089, NA51055 / NA51084, NS02501: the mask ROM starts the
# loader, which trains the DDR, reads the device tree at 0x10000 and
# unpacks U-Boot from 0x20000 to its link address. The NS02501 trees are
# packed too (nvt-image.py --fdt-lzma).
#   build_nvt <soc> <loader die> <chip> [loader make variables...]
build_nvt() {
    soc=$1
    die=$2
    chip=$3
    shift 3
    builddir=${BOOT_SRCDIR}/output/build-${soc}-nor
    loaderdir=${BOOT_SRCDIR}/output/build-${soc}-loader
    # The binary is named after the part (LD98533A for CHIP 533QD)
    part=$(echo "${chip}" | cut -c1-3)
    loader=${loaderdir}/Project/Model/Loader${chip}_Data/Release/LD98${part}A.bin
    image_opts=
    [ "${die}" = ns02501 ] && image_opts=--fdt-lzma
    make_opts="-C ${BOOT_SRCDIR} O=${builddir} -j$(nproc)"

    rm -rf "${builddir}"
    mkconfig "${builddir}" openipc/openipc_config openipc/${soc}_config && \
    make ${make_opts} olddefconfig </dev/null >/dev/null && \
    make ${make_opts} </dev/null && \
    make ${make_opts} scripts_dtc </dev/null >/dev/null || return 1

    "${builddir}/scripts/dtc/dtc" -q -I dts -O dtb \
        -o "${builddir}/${soc}.dtb" "openipc/${soc}.dts" || return 1
    build_loader "${loaderdir}" "${die}" CHIP="${chip}" "$@" || return 1

    text=$(sed -n 's/^CONFIG_SYS_TEXT_BASE=//p' "${builddir}/.config")
    python3 openipc/nvt-image.py ${image_opts} "${loader}" \
        "${builddir}/${soc}.dtb" "${builddir}/u-boot.bin" "${text}" \
        "output/u-boot-${soc}-nor.bin"
}

# Novatek NVR dies, AArch64: the board's loader and BL31 stay, the image
# is U-Boot's slot alone (openipc/nvt-nvr-image.py). FLASH is nor or nand,
# LAYOUT where the boards have their tree and U-Boot, and the boot area.
build_nvr() {
    soc=$1
    flash=$2
    layout=$3
    builddir=${BOOT_SRCDIR}/output/build-${soc}-${flash}
    make_opts="-C ${BOOT_SRCDIR} O=${builddir} -j$(nproc) CROSS_COMPILE=${CROSS_COMPILE_A64}"

    rm -rf "${builddir}"
    mkconfig "${builddir}" openipc/openipc_config openipc/${soc}_config && \
    make ${make_opts} olddefconfig </dev/null >/dev/null && \
    make ${make_opts} </dev/null || return 1

    text=$(sed -n 's/^CONFIG_SYS_TEXT_BASE=//p' "${builddir}/.config")
    python3 openipc/nvt-nvr-image.py "${soc}" "${builddir}/u-boot.bin" \
        "${text}" "output/u-boot-${soc}-${flash}.bin" || return 1

    # For the emulator's stand-in loader (CI): no loader, a plain tree and
    # the slot where the boards have theirs, as a boot area image
    make ${make_opts} scripts_dtc </dev/null >/dev/null &&
    "${builddir}/scripts/dtc/dtc" -q -I dts -O dtb \
        -o "${builddir}/${soc}.dtb" "openipc/${soc}.dts" || return 1
    python3 - "${builddir}/${soc}.dtb" "output/u-boot-${soc}-${flash}.bin" \
        "output/u-boot-${soc}-qemu.bin" ${layout} <<'EOF'
import sys
fdt, slot, size = (int(x, 0) for x in sys.argv[4:7])
img = bytearray(b'\xff' * size)
dtb = open(sys.argv[1], 'rb').read()
ub = open(sys.argv[2], 'rb').read()
img[fdt:fdt + len(dtb)] = dtb
img[slot:slot + len(ub)] = ub
open(sys.argv[3], 'wb').write(img)
EOF
}

build_soc() {
    soc=$1
    case "${soc}" in
        # The DDR clocks of Xiongmai's loaders for the NT98562 / NT98566,
        # and of the SDK's prebuilt LD98560A / LD98561A. The DDR of the
        # 561/562/566 is in the package: Configram560 writes its
        # parameters from its own tables for -package=561/562/566, and the
        # ConfigRam/<chip>/*.ini path the make system names, which does not
        # exist, is not read (a 560 profile of any clock and size gives
        # the same loader). The NT98560's DDR is on the board: its profile
        # is ConfigRam/560/51089A_DRAM1_<clock>_<size in Mbit>.ini, the
        # SDK's 933 MHz, 4 Gbit.
        nt98560)
            build_nvt "${soc}" na51089 560 DRAM1_CLK=933 SDRAM_SIZE=4096 ;;
        nt98561)
            build_nvt "${soc}" na51089 561 DRAM1_CLK=522 ;;
        nt98562)
            build_nvt "${soc}" na51089 562 DRAM1_CLK=666 ;;
        nt98566)
            build_nvt "${soc}" na51089 566 DRAM1_CLK=933 ;;
        # The NT9852x loader (MakeConfig.txt's tables). The NT98522 and
        # NT98523 have their DDR1 in the package (512 Mbit, 1 Gbit, at
        # 933 MHz), and no DDR2 here: the SDK board's external one is not
        # every board's. The others' DDR is on the board: the SDK
        # boards' 4 Gbit dies, DDR3 at 933 MHz (NT98520: DDR1 only;
        # NT98525: DDR1 and DDR2) or 1066 MHz (NT98528/529, the 528 model).
        # The 528 model's loader fills its 64 KiB with the LZMA decoder
        # only without its own SD card update (FW98528A.bin from a card);
        # U-Boot's recovery from SD is OpenIPC's.
        nt98520)
            build_nvt "${soc}" na51055 520 DRAM1_CLK=933 SDRAM_SIZE=4096 ;;
        nt98522|nt98523)
            build_nvt "${soc}" na51055 "${soc#nt98}" DRAM2_CLK=0 \
                SDRAM2_SIZE=0 ;;
        nt98525)
            build_nvt "${soc}" na51055 525 DRAM1_CLK=933 SDRAM_SIZE=4096 \
                DRAM2_CLK=933 SDRAM2_SIZE=4096 ;;
        nt98528|nt98529)
            # A Linux toolchain's build of the 528 flow stops after the
            # DDR training (arm-linux-gnueabi 12/13 under qemu-hisilicon's
            # nt98528); a bare-metal one's (arm-none-eabi 13.2, the SDK's
            # Linaro 6.4 arm-eabi) boots.
            case "${LOADER_CROSS_COMPILE}" in
                *-linux-*)
                    echo "${soc}: its loader needs a bare-metal toolchain" \
                        "(arm-none-eabi-), not ${LOADER_CROSS_COMPILE}" >&2
                    return 1 ;;
            esac
            build_nvt "${soc}" na51055 "${soc#nt98}" MODEL=EMU_EVB_528 \
                DRAM1_CLK=1066 SDRAM_SIZE=4096 DRAM2_CLK=1066 \
                SDRAM2_SIZE=4096 STORAGEEXT=NONE ;;
        # The NS02501 loader (ns02501_ldr). CHIP is the package
        # (ModelConfig_EMU_EVB.txt): QB..QD, BC..BE, the last letter the
        # DDR's size (B 512 Mbit, C 1 Gbit, D 2 Gbit, E 4 Gbit), whose
        # parameters Configram533 has itself (there is no ConfigRam
        # directory). The SDK's boards are the 532BD and 533QD, 2 Gbit at
        # its default 1066 MHz. Another package is the same U-Boot with a
        # loader built for its CHIP (and NVT_DDR_SIZE to match).
        nt98532)
            build_nvt "${soc}" ns02501 532BD DRAM1_CLK=1066 ;;
        nt98533)
            build_nvt "${soc}" ns02501 533QD DRAM1_CLK=1066 ;;
        # The NVRs: NA51103 on SPI NOR (Xiongmai's layout, OpenIPC's from
        # 0x40000), NA51090 on SPI NAND (the SDK's, env and UBI after it)
        nt98332)
            build_nvr "${soc}" nor "0x10000 0x20000 0x40000" ;;
        nt98633)
            build_nvr "${soc}" nand "0x40000 0xc0000 0x1a0000" ;;
        *)
            echo "unsupported SoC: ${soc}" >&2
            return 1 ;;
    esac
}

ALL_SOCS="nt98560 nt98561 nt98562 nt98566 nt98520 nt98522 nt98523 nt98525 nt98528 nt98529 nt98532 nt98533 nt98332 nt98633"

SOCS="$@"
[ -z "${SOCS}" ] && SOCS="${ALL_SOCS}"

rc=0
for soc in ${SOCS}; do
    build_soc "${soc}" || { echo "build failed for ${soc}" >&2; rc=1; }
done

exit ${rc}
