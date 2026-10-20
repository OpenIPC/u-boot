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

# Xiongmai: U-Boot runs in place from flash, so the image is u-boot.bin as
# linked, with no compressed loader in front of it.
build_xm() {
    soc=$1
    kind=$2
    builddir=${BOOT_SRCDIR}/output/build-${soc}-${kind}
    make_opts="-C ${BOOT_SRCDIR} O=${builddir} -j$(nproc)"
    fragments="openipc/openipc_config openipc/${soc}_config"

    rm -rf "${builddir}"
    mkconfig "${builddir}" ${fragments} && \
    make ${make_opts} olddefconfig </dev/null >/dev/null && \
    make ${make_opts} </dev/null || return 1

    cp "${builddir}/u-boot.bin" "output/u-boot-${soc}-nor.bin"
}

build_soc() {
    soc=$1
    case "${soc}" in
        xm510|xm530|xm550)
            build_xm "${soc}" nor ;;
        *)
            echo "unsupported SoC: ${soc}" >&2
            return 1 ;;
    esac
}

ALL_SOCS="xm510 xm530 xm550"

SOCS="$@"
[ -z "${SOCS}" ] && SOCS="${ALL_SOCS}"

rc=0
for soc in ${SOCS}; do
    build_soc "${soc}" || { echo "build failed for ${soc}" >&2; rc=1; }
done

exit ${rc}
