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
    [ "${kind}" = nand ] && fragments="${fragments} openipc/nand_config"

    rm -rf "${builddir}"
    mkconfig "${builddir}" ${fragments} && \
    make ${make_opts} olddefconfig </dev/null >/dev/null && \
    make ${make_opts} </dev/null || return 1

    if [ "${kind}" = nor ]; then
        cp "${builddir}/u-boot.bin" "output/u-boot-${soc}-nor.bin" || return 1
    else
        # SPI NAND: a 4 KiB header, then U-Boot linked at the window + 4 KiB
        # (CONFIG_SYS_TEXT_BASE in nand_config). Nothing documents what the
        # boot ROM expects here; the header's first word jumps to U-Boot
        # wherever the ROM starts it (ldr pc, [pc, #-4] and the address).
        text=$(sed -n 's/^CONFIG_SYS_TEXT_BASE=//p' "${builddir}/.config")
        python3 - "${builddir}/u-boot.bin" "output/u-boot-${soc}-nand.bin" \
            "${text}" <<'PY' || return 1
import struct, sys
src, dst, text = sys.argv[1], sys.argv[2], int(sys.argv[3], 0)
hdr = bytearray(b'\xff' * 0x1000)
hdr[0:8] = struct.pack('<II', 0xe51ff004, text)
open(dst, 'wb').write(bytes(hdr) + open(src, 'rb').read())
PY
    fi
}

build_soc() {
    soc=$1
    case "${soc}" in
        xm510)
            build_xm "${soc}" nor ;;
        xm530|xm550)
            build_xm "${soc}" nor && build_xm "${soc}" nand ;;
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
