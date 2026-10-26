#!/bin/sh
#
# Build U-Boot for the SoCs named on the command line, or for every
# supported SoC when invoked without arguments. Failures are collected:
# every requested SoC is attempted and the script exits non-zero if any
# of them failed.

cd "$(dirname "$0")"

export ARCH=arm
export CROSS_COMPILE=${CROSS_COMPILE:-arm-linux-gnueabi-}
# nsboot runs from SRAM before DDR is up: any ARM compiler that builds
# freestanding code will do
NSBOOT_CROSS_COMPILE=${NSBOOT_CROSS_COMPILE:-${CROSS_COMPILE}}

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

# Grain Media: nsboot, from the SDK's sources in board/grainmedia/nsboot,
# is the first stage the boot ROM loads. One build serves every SoC: it
# picks the package's DDR table at run time.
NSBOOT=${BOOT_SRCDIR}/output/build-nsboot/nsboot.bin

build_nsboot() {
    [ -f "${NSBOOT}" ] && return 0
    rm -rf output/build-nsboot
    cp -r board/grainmedia/nsboot output/build-nsboot && \
    make -C output/build-nsboot CROSS_COMPILE="${NSBOOT_CROSS_COMPILE}" \
        </dev/null >/dev/null || { rm -f "${NSBOOT}"; return 1; }
}

# gm-image.py puts the NOR system header at 0, nsboot at 0x1000 and
# U-Boot at UBOOT_OFFSET, before the environment.
UBOOT_OFFSET=0x8000

build_grainmedia() {
    soc=$1
    builddir=${BOOT_SRCDIR}/output/build-${soc}-nor
    make_opts="-C ${BOOT_SRCDIR} O=${builddir} -j$(nproc)"

    build_nsboot || return 1
    rm -rf "${builddir}"
    mkconfig "${builddir}" openipc/openipc_config openipc/${soc}_config && \
    make ${make_opts} olddefconfig </dev/null >/dev/null && \
    make ${make_opts} </dev/null || return 1

    env=$(sed -n 's/^CONFIG_ENV_OFFSET=//p' "${builddir}/.config")
    python3 openipc/gm-image.py "${NSBOOT}" "${builddir}/u-boot.bin" \
        "${UBOOT_OFFSET}" "${env}" "output/u-boot-${soc}-nor.bin"
}

build_soc() {
    soc=$1
    case "${soc}" in
        gm8135|gm8136)
            build_grainmedia "${soc}" ;;
        *)
            echo "unsupported SoC: ${soc}" >&2
            return 1 ;;
    esac
}

ALL_SOCS="gm8136 gm8135"

SOCS="$@"
[ -z "${SOCS}" ] && SOCS="${ALL_SOCS}"

rm -f "${NSBOOT}"
rc=0
for soc in ${SOCS}; do
    build_soc "${soc}" || { echo "build failed for ${soc}" >&2; rc=1; }
done

exit ${rc}
