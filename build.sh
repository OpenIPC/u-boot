#!/bin/sh
#
# Build U-Boot for the SoCs named on the command line, or for every
# supported SoC when invoked without arguments. Failures are collected:
# every requested SoC is attempted and the script exits non-zero if any
# of them failed.

cd "$(dirname "$0")"

export ARCH=mips
export CROSS_COMPILE=${CROSS_COMPILE:-mipsel-linux-gnu-}

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

# Ingenic T-series: the SPL (PLL, DDR) at the start of the NOR, U-Boot
# lzop-packed in a uImage after it (openipc/ingenic-image.py).
# The NAND image of a target, with openipc/nand_config after its fragments
# (as openipc-hisilicon's): the SPL in the 27 KiB SFC_NAND slot.
build_soc() {
    soc=$1
    flash=${2:-nor}
    builddir=${BOOT_SRCDIR}/output/build-${soc}-${flash}
    make_opts="-C ${BOOT_SRCDIR} O=${builddir} -j$(nproc)"
    fragments="openipc/openipc_config openipc/${soc}_config"
    [ "${flash}" = nand ] && fragments="${fragments} openipc/nand_config"

    if [ ! -f "openipc/${soc}_config" ]; then
        echo "unsupported SoC: ${soc}" >&2
        return 1
    fi

    rm -rf "${builddir}"
    mkconfig "${builddir}" ${fragments} && \
    make ${make_opts} olddefconfig </dev/null >/dev/null && \
    make ${make_opts} </dev/null || return 1

    text_base=$(sed -n 's/^CONFIG_SYS_TEXT_BASE=//p' "${builddir}/.config")
    lzop -f -9 -o "${builddir}/u-boot-lzo.bin" "${builddir}/u-boot.bin" && \
    "${builddir}/tools/mkimage" -A mips -O u-boot -T firmware -C lzo \
        -a "${text_base}" -e "${text_base}" -n "u-boot-lzo.img" \
        -d "${builddir}/u-boot-lzo.bin" "${builddir}/u-boot-lzo.img" \
        >/dev/null && \
    case "${soc}" in
        t40n|t40xp) image_opts="--pad-block --params" ;;
        t40*|t41*) image_opts="--pad-block" ;;
        *) image_opts="" ;;
    esac
    # The vendor's SFC_NAND SPLs have no boot ROM parameter block
    [ "${flash}" = nand ] && image_opts="--pad-block --nand"
    python3 openipc/ingenic-image.py ${image_opts} "${builddir}/spl/u-boot-spl.bin" \
        "${builddir}/u-boot-lzo.img" "output/u-boot-${soc}-${flash}.bin"
}

# The targets with an SPI NAND image as well
NAND_SOCS="t40n t40xp t40a"

build_target() {
    build_soc "$1" nor || return 1
    case " ${NAND_SOCS} " in
        *" $1 "*) build_soc "$1" nand ;;
    esac
}

ALL_SOCS="t10 t10l t20 t20l t20x t21n t23n t23zn t23x t30n t30l t30x t30a t30a1 t31n t31l t31x t31a t31al t31lc t40n t40xp t40a t41l t41lq t41n t41nq t41a t41zx t41xq"

SOCS="$@"
[ -z "${SOCS}" ] && SOCS="${ALL_SOCS}"

rc=0
for soc in ${SOCS}; do
    build_target "${soc}" || { echo "build failed for ${soc}" >&2; rc=1; }
done

exit ${rc}
