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

# Anyka AK39: the boot ROM runs the PLL and DDR set-up in the image's header
# (EV330: the first stage in the image does), then copies U-Boot to its link
# address and starts it there.
build_ak39() {
    soc=$1
    family=$2
    data=$3
    builddir=${BOOT_SRCDIR}/output/build-${soc}-nor
    make_opts="-C ${BOOT_SRCDIR} O=${builddir} -j$(nproc)"

    rm -rf "${builddir}"
    mkconfig "${builddir}" openipc/openipc_config openipc/${soc}_config && \
    make ${make_opts} olddefconfig </dev/null >/dev/null && \
    make ${make_opts} </dev/null || return 1

    text=$(sed -n 's/^CONFIG_SYS_TEXT_BASE=//p' "${builddir}/.config")
    python3 openipc/ak39-image.py ${family} ${data} \
        "${builddir}/u-boot.bin" "${text}" "output/u-boot-${soc}-nor.bin"
}

build_soc() {
    soc=$1
    case "${soc}" in
        ak3918ev200)
            build_ak39 "${soc}" ev200 openipc/ak3918ev200.rvs ;;
        ak3916ev300|ak3918ev300|ak3919ev300)
            build_ak39 "${soc}" ev300 openipc/ak39ev300.rvs ;;
        ak3918ev330|ak3919ev330)
            build_ak39 "${soc}" ev330 openipc/ak39ev330-boot.bin ;;
        *)
            echo "unsupported SoC: ${soc}" >&2
            return 1 ;;
    esac
}

ALL_SOCS="ak3918ev200 ak3916ev300 ak3918ev300 ak3919ev300 ak3918ev330 ak3919ev330"

SOCS="$@"
[ -z "${SOCS}" ] && SOCS="${ALL_SOCS}"

rc=0
for soc in ${SOCS}; do
    build_soc "${soc}" || { echo "build failed for ${soc}" >&2; rc=1; }
done

exit ${rc}
