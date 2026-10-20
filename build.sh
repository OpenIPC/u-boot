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

# Goke GK710x: the boot ROM runs the DDR set-up in the image's header, then
# copies U-Boot to its link address and starts it there.
build_gk710x() {
    soc=$1
    builddir=${BOOT_SRCDIR}/output/build-${soc}-nor
    make_opts="-C ${BOOT_SRCDIR} O=${builddir} -j$(nproc)"

    rm -rf "${builddir}"
    mkconfig "${builddir}" openipc/openipc_config openipc/${soc}_config && \
    make ${make_opts} olddefconfig </dev/null >/dev/null && \
    make ${make_opts} </dev/null || return 1

    text=$(sed -n 's/^CONFIG_SYS_TEXT_BASE=//p' "${builddir}/.config")
    env=$(sed -n 's/^CONFIG_ENV_OFFSET=//p' "${builddir}/.config")
    python3 openipc/gk710x-image.py openipc/${soc}.rvs \
        "${builddir}/u-boot.bin" "${text}" "output/u-boot-${soc}-nor.bin" \
        "${env}"
}

build_soc() {
    soc=$1
    case "${soc}" in
        gk7101|gk7102|gk7101s|gk7102s)
            build_gk710x "${soc}" ;;
        *)
            echo "unsupported SoC: ${soc}" >&2
            return 1 ;;
    esac
}

ALL_SOCS="gk7101 gk7102 gk7101s gk7102s"

SOCS="$@"
[ -z "${SOCS}" ] && SOCS="${ALL_SOCS}"

rc=0
for soc in ${SOCS}; do
    build_soc "${soc}" || { echo "build failed for ${soc}" >&2; rc=1; }
done

exit ${rc}
