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
build_soc() {
    soc=$1
    builddir=${BOOT_SRCDIR}/output/build-${soc}-nor
    make_opts="-C ${BOOT_SRCDIR} O=${builddir} -j$(nproc)"

    if [ ! -f "openipc/${soc}_config" ]; then
        echo "unsupported SoC: ${soc}" >&2
        return 1
    fi

    rm -rf "${builddir}"
    mkconfig "${builddir}" openipc/openipc_config openipc/${soc}_config && \
    make ${make_opts} olddefconfig </dev/null >/dev/null && \
    make ${make_opts} </dev/null || return 1

    text_base=$(sed -n 's/^CONFIG_SYS_TEXT_BASE=//p' "${builddir}/.config")
    lzop -f -9 -o "${builddir}/u-boot-lzo.bin" "${builddir}/u-boot.bin" && \
    "${builddir}/tools/mkimage" -A mips -O u-boot -T firmware -C lzo \
        -a "${text_base}" -e "${text_base}" -n "u-boot-lzo.img" \
        -d "${builddir}/u-boot-lzo.bin" "${builddir}/u-boot-lzo.img" \
        >/dev/null && \
    python3 openipc/ingenic-image.py "${builddir}/spl/u-boot-spl.bin" \
        "${builddir}/u-boot-lzo.img" "output/u-boot-${soc}-nor.bin"
}

ALL_SOCS="t10 t10l t20 t20l t20x t21n t23n t23zn t23x t30n t30l t30x t30a t30a1 t31n t31l t31x t31a t31al t31lc"

SOCS="$@"
[ -z "${SOCS}" ] && SOCS="${ALL_SOCS}"

rc=0
for soc in ${SOCS}; do
    build_soc "${soc}" || { echo "build failed for ${soc}" >&2; rc=1; }
done

exit ${rc}
