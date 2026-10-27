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

# SigmaStar infinity3: the vendor's IPL trains the DDR and loads IPL_CUST,
# which unpacks U-Boot from the MXP partition UBOOT to its link address.
# The boot partition holds all of it (openipc/infinity3-image.py).
build_infinity3() {
    soc=$1
    ipl=$2
    builddir=${BOOT_SRCDIR}/output/build-${soc}-nor
    make_opts="-C ${BOOT_SRCDIR} O=${builddir} -j$(nproc)"

    rm -rf "${builddir}"
    mkconfig "${builddir}" openipc/openipc_config openipc/${soc}_config && \
    make ${make_opts} olddefconfig </dev/null >/dev/null && \
    make ${make_opts} </dev/null || return 1

    python3 openipc/infinity3-image.py ${ipl} openipc/infinity3-ipl-cust.bin \
        "${builddir}/u-boot.bin" "${soc}" "output/u-boot-${soc}-nor.bin"
}

build_soc() {
    soc=$1
    case "${soc}" in
        msc313e)
            build_infinity3 "${soc}" openipc/infinity3-msc313e-ipl.bin ;;
        msc316dc|msc316dm)
            build_infinity3 "${soc}" openipc/infinity3-msc316-ipl.bin ;;
        *)
            echo "unsupported SoC: ${soc}" >&2
            return 1 ;;
    esac
}

ALL_SOCS="msc313e msc316dc msc316dm"

SOCS="$@"
[ -z "${SOCS}" ] && SOCS="${ALL_SOCS}"

rc=0
for soc in ${SOCS}; do
    build_soc "${soc}" || { echo "build failed for ${soc}" >&2; rc=1; }
done

exit ${rc}
