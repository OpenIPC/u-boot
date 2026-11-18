#!/bin/sh
#
# Build U-Boot for the targets named on the command line, or for every
# target with an openipc/<target>_config when invoked without arguments.
# A target is <soc>-<flash>, e.g. ax620u-nand; the image is
# output/u-boot-<soc>-<flash>.bin. Failures are collected: every requested
# target is attempted and the script exits non-zero if any of them failed.

cd "$(dirname "$0")"

export ARCH=arm
export CROSS_COMPILE=${CROSS_COMPILE:-arm-linux-gnueabi-}

BOOT_SRCDIR=$(pwd)

mkdir -p output

# The boot partition's size, from the target's mtdparts: the image's limit
part_size() {
    sed -n 's/^CONFIG_MTDPARTS_DEFAULT=.*[:,]\([0-9]*\)k(boot).*/\1/p' "$1"
}

build_target() {
    target=$1
    config=openipc/${target}_config
    builddir=${BOOT_SRCDIR}/output/build-${target}
    make_opts="-C ${BOOT_SRCDIR} O=${builddir} -j${JOBS:-$(nproc)}"

    [ -f "${config}" ] || { echo "unsupported target: ${target}" >&2; return 1; }

    rm -rf "${builddir}"
    mkdir -p "${builddir}"
    cp "${config}" "${builddir}/.config"
    make ${make_opts} olddefconfig </dev/null >/dev/null && \
    make ${make_opts} </dev/null || return 1

    limit=$(part_size "${builddir}/.config")
    [ -n "${limit}" ] || { echo "${target}: no boot partition in mtdparts" >&2; return 1; }
    python3 openipc/axera-image.py "${builddir}/u-boot.bin" \
        "output/u-boot-${target}.bin" $((limit * 1024))
}

ALL_TARGETS=$(cd openipc && ls *_config | sed 's/_config$//')

TARGETS="$@"
[ -z "${TARGETS}" ] && TARGETS="${ALL_TARGETS}"

rc=0
for target in ${TARGETS}; do
    build_target "${target}" || { echo "build failed for ${target}" >&2; rc=1; }
done

exit ${rc}
