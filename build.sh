#!/bin/sh
#
# Build U-Boot for the SoCs named on the command line, or for every
# supported SoC when invoked without arguments. Failures are collected:
# every requested SoC is attempted and the script exits non-zero if any
# of them failed.

cd "$(dirname "$0")"

export ARCH=arm
export CROSS_COMPILE=${CROSS_COMPILE:-arm-linux-gnueabi-}
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

# The SDK's loader of a die (board/novatek/<die>/loader), built in a
# copy: its make system writes into its tree. CHIP, and the DDR clock,
# DDR size and model when given, override its ModelConfig.
build_loader() {
    loaderdir=$1
    die=$2
    chip=$3
    dram_clk=$4
    dram_size=$5
    model=$6

    rm -rf "${loaderdir}"
    cp -a "${BOOT_SRCDIR}/board/novatek/${die}/loader" "${loaderdir}"
    if ! make -C "${loaderdir}/MakeCommon" release \
            CROSS_COMPILE="${LOADER_CROSS_COMPILE}" \
            CHIP="${chip}" ${dram_clk:+DRAM1_CLK="${dram_clk}"} \
            ${dram_size:+SDRAM_SIZE="${dram_size}"} \
            ${model:+MODEL="${model}"} \
            >"${loaderdir}.log" 2>&1 </dev/null; then
        tail -n 30 "${loaderdir}.log" >&2
        return 1
    fi
}

# Novatek NA51089, NA51068: the mask ROM starts the loader, which trains
# the DDR, reads the device tree at 0x10000 and unpacks U-Boot from
# 0x20000 to its link address. The die defaults to NA51089.
build_nvt() {
    soc=$1
    chip=$2
    dram_clk=$3
    dram_size=$4
    die=${5:-na51089}
    model=$6
    builddir=${BOOT_SRCDIR}/output/build-${soc}-nor
    loaderdir=${BOOT_SRCDIR}/output/build-${soc}-loader
    loader=${loaderdir}/Project/Model/Loader${chip}_Data/Release/LD98${chip}A.bin
    make_opts="-C ${BOOT_SRCDIR} O=${builddir} -j$(nproc)"

    rm -rf "${builddir}"
    mkconfig "${builddir}" openipc/openipc_config openipc/${soc}_config && \
    make ${make_opts} olddefconfig </dev/null >/dev/null && \
    make ${make_opts} </dev/null && \
    make ${make_opts} scripts_dtc </dev/null >/dev/null || return 1

    "${builddir}/scripts/dtc/dtc" -q -I dts -O dtb \
        -o "${builddir}/${soc}.dtb" "openipc/${soc}.dts" || return 1
    build_loader "${loaderdir}" "${die}" "${chip}" "${dram_clk}" \
        "${dram_size}" "${model}" || return 1

    text=$(sed -n 's/^CONFIG_SYS_TEXT_BASE=//p' "${builddir}/.config")
    python3 openipc/nvt-image.py "${loader}" "${builddir}/${soc}.dtb" \
        "${builddir}/u-boot.bin" "${text}" "output/u-boot-${soc}-nor.bin"
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
            build_nvt "${soc}" 560 933 4096 ;;
        nt98561)
            build_nvt "${soc}" 561 522 ;;
        nt98562)
            build_nvt "${soc}" 562 666 ;;
        nt98566)
            build_nvt "${soc}" 566 933 ;;
        # NA51068: the SDK's NAND models with the SPI NOR, its own DDR
        # profile each (board/novatek/na51068/loader/Project/Model):
        # NT98321 one 4 Gb DDR3 die at 1066 MHz, NT98323 one 4 Gb DDR4
        # die at 1200 MHz, as the SDK's cfg_98323_DDR4_4GX1_NOR_EVB.
        nt98321)
            build_nvt "${soc}" 321 "" "" na51068 321_DDR3_4GX1_NOR_EVB ;;
        nt98323)
            build_nvt "${soc}" 323 "" "" na51068 323_DDR4_4GX1_NOR_EVB ;;
        *)
            echo "unsupported SoC: ${soc}" >&2
            return 1 ;;
    esac
}

ALL_SOCS="nt98560 nt98561 nt98562 nt98566 nt98321 nt98323"

SOCS="$@"
[ -z "${SOCS}" ] && SOCS="${ALL_SOCS}"

rc=0
for soc in ${SOCS}; do
    build_soc "${soc}" || { echo "build failed for ${soc}" >&2; rc=1; }
done

exit ${rc}
