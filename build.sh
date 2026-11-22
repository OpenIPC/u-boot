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
# linked, with no compressed loader in front of it (the XM232's: behind a
# fixed 32-byte header).
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

    out="output/u-boot-${soc}-nor.bin"
    case "${soc}" in
        xm232)
            # The XM232's flash starts with the 32-byte header its vendor
            # images carry; U-Boot is linked behind it, at 0x60000020.
            { printf '\322\243\130\207\077\066\056\353'
              head -c 24 /dev/zero
              cat "${builddir}/u-boot.bin"; } > "${out}" ;;
        *)
            cp "${builddir}/u-boot.bin" "${out}" ;;
    esac
}

build_soc() {
    soc=$1
    case "${soc}" in
        xm510|xm530|xm550|xm530v200|xm533|xm535d1|xm535d2|xm535w1|xm232)
            build_xm "${soc}" nor ;;
        *)
            echo "unsupported SoC: ${soc}" >&2
            return 1 ;;
    esac
}

ALL_SOCS="xm510 xm530 xm550 xm530v200 xm533 xm535d1 xm535d2 xm535w1 xm232"

SOCS="$@"
[ -z "${SOCS}" ] && SOCS="${ALL_SOCS}"

rc=0
for soc in ${SOCS}; do
    build_soc "${soc}" || { echo "build failed for ${soc}" >&2; rc=1; }
done

exit ${rc}
