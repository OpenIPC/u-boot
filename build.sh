#!/bin/sh
#
# Build U-Boot for the SoCs named on the command line, or for every
# supported SoC when invoked without arguments. Failures are collected:
# every requested SoC is attempted and the script exits non-zero if any
# of them failed.

cd "$(dirname "$0")"

export ARCH=arm
export CROSS_COMPILE=${CROSS_COMPILE:-arm-linux-gnueabi-}

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

# Fullhan: the boot ROM runs the DDR script listed in the flash header,
# then loads the header's first-stage record -- U-Boot itself, or on FH865x
# and FH885x V100 the vendor's ramboot, which loads U-Boot -- and starts it.
# fullhan-image.py lays the header, its DDR scripts, the ramboot and
# U-Boot out in the boot partition, from openipc/<generation>.boot.
build_fullhan() {
    soc=$1
    boot=$2
    builddir=${BOOT_SRCDIR}/output/build-${soc}-nor
    make_opts="-C ${BOOT_SRCDIR} O=${builddir} -j$(nproc)"

    rm -rf "${builddir}"
    mkconfig "${builddir}" openipc/openipc_config openipc/${soc}_config && \
    make ${make_opts} olddefconfig </dev/null >/dev/null && \
    make ${make_opts} </dev/null || return 1

    text=$(sed -n 's/^CONFIG_SYS_TEXT_BASE=//p' "${builddir}/.config")
    env=$(sed -n 's/^CONFIG_ENV_OFFSET=//p' "${builddir}/.config")
    python3 openipc/fullhan-image.py "openipc/${boot}.boot" \
        "${builddir}/u-boot.bin" "${text}" "${env}" \
        "output/u-boot-${soc}-nor.bin"
}

build_soc() {
    soc=$1
    case "${soc}" in
        fh8852v200|fh8856v200|fh8858v200|fh8852v210|fh8856v210|fh8858v210)
            build_fullhan "${soc}" fh885xv200 ;;
        fh8652|fh8656|fh8658)
            build_fullhan "${soc}" fh865x ;;
        fh8852v100|fh8856v100)
            build_fullhan "${soc}" fh885xv100 ;;
        fh8626v100)
            build_fullhan "${soc}" fh8626v100 ;;
        fh8833v100)
            build_fullhan "${soc}" fh8632 ;;
        *)
            echo "unsupported SoC: ${soc}" >&2
            return 1 ;;
    esac
}

ALL_SOCS="fh8852v200 fh8856v200 fh8858v200 fh8852v210 fh8856v210 fh8858v210
fh8652 fh8656 fh8658 fh8852v100 fh8856v100 fh8626v100 fh8833v100"

SOCS="$@"
[ -z "${SOCS}" ] && SOCS="${ALL_SOCS}"

rc=0
for soc in ${SOCS}; do
    build_soc "${soc}" || { echo "build failed for ${soc}" >&2; rc=1; }
done

exit ${rc}
