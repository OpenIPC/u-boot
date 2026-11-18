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
# The AArch64 SoCs (AX620E)
CROSS_COMPILE_64=${CROSS_COMPILE_64:-aarch64-linux-gnu-}

BOOT_SRCDIR=$(pwd)

mkdir -p output

# The boot partition's size, from the target's mtdparts: the image's limit
part_size() {
    sed -n 's/^CONFIG_MTDPARTS_DEFAULT=.*[:,]\([0-9]*\)k(boot).*/\1/p' "$1"
}

# The AX620E's first stage unpacks U-Boot from Axera's own packed format,
# which only the SDK's ax_gzip writes: an x86-64 Linux binary, fetched from
# the AX620E SDK (Sipeed's copy, the commit that added it) and checked,
# unless AX_GZIP names one.
AX_GZIP_URL=https://raw.githubusercontent.com/sipeed/maix_ax620e_sdk/fe7545cf22412e410968a0a33a51b1880ffd9034/tools/ax_gzip_tool/ax_gzip
AX_GZIP_SHA256=4aa938092a2315e8b5e79e4495fe3de261c972ff0e6be0a85c1ff052eea2e877

ax_gzip() {
    if [ -n "${AX_GZIP}" ]; then
        echo "${AX_GZIP}"
        return
    fi
    tool=${BOOT_SRCDIR}/output/ax_gzip
    if ! echo "${AX_GZIP_SHA256}  ${tool}" | sha256sum -c - >/dev/null 2>&1; then
        curl -fsSL -o "${tool}.part" "${AX_GZIP_URL}" &&
        echo "${AX_GZIP_SHA256}  ${tool}.part" | sha256sum -c - >/dev/null &&
        mv "${tool}.part" "${tool}" && chmod +x "${tool}" || {
            echo "cannot fetch ax_gzip from ${AX_GZIP_URL}" >&2
            rm -f "${tool}.part"
            return 1
        }
    fi
    echo "${tool}"
}

# AX620E: the first stage at flash 0, then U-Boot packed behind its header
# at 0x10000, all before the environment (ax620e-image.py)
ax620e_image() {
    target=$1
    builddir=$2
    tool=$(ax_gzip) || return 1
    text=$(sed -n 's/^CONFIG_SYS_TEXT_BASE=//p' "${builddir}/.config")
    env=$(sed -n 's/^CONFIG_ENV_OFFSET=//p' "${builddir}/.config")
    python3 openipc/ax620e-image.py openipc/ax620e.spl \
        "${builddir}/u-boot.bin" "${text}" "${tool}" \
        openipc/axera-sdk-test-key.pem "${env}" "output/u-boot-${target}.bin"
}

build_target() {
    target=$1
    config=openipc/${target}_config
    builddir=${BOOT_SRCDIR}/output/build-${target}
    make_opts="-C ${BOOT_SRCDIR} O=${builddir} -j${JOBS:-$(nproc)}"

    [ -f "${config}" ] || { echo "unsupported target: ${target}" >&2; return 1; }

    case "${target}" in
        ax620e-*) cross=${CROSS_COMPILE_64} ;;
        *) cross=${CROSS_COMPILE} ;;
    esac
    make_opts="${make_opts} CROSS_COMPILE=${cross}"

    rm -rf "${builddir}"
    mkdir -p "${builddir}"
    cp "${config}" "${builddir}/.config"
    make ${make_opts} olddefconfig </dev/null >/dev/null && \
    make ${make_opts} </dev/null || return 1

    case "${target}" in
        ax620e-*)
            ax620e_image "${target}" "${builddir}"
            return ;;
    esac

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
