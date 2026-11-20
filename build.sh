#!/bin/sh
#
# Build U-Boot for the targets named on the command line, or for every
# target with an openipc/<target>_config when invoked without arguments.
# A target is <soc>-<flash>, e.g. ax620u-nand; the image is
# output/u-boot-<soc>-<flash>.bin, written at flash 0 (for the Axera SoCs
# also u-boot-<soc>-<flash>-stockspl.bin, the same with the cameras' own
# first stage). Failures are collected: every requested target is attempted and the script exits non-zero if any of them failed.

cd "$(dirname "$0")"

export ARCH=arm
export CROSS_COMPILE=${CROSS_COMPILE:-arm-linux-gnueabi-}
# The AArch64 SoCs (AX620E)
CROSS_COMPILE_64=${CROSS_COMPILE_64:-aarch64-linux-gnu-}

BOOT_SRCDIR=$(pwd)

mkdir -p output

# A partition's size in KiB (the boot partition's unless named), from the
# target's mtdparts: the image's limit
part_size() {
    sed -n "s/^CONFIG_MTDPARTS_DEFAULT=.*[:,]\([0-9]*\)k(${2:-boot}).*/\1/p" "$1"
}

# AX620U: the boot partition (768k(boot),256k(env),-(ubi), OpenIPC's NAND
# layout) in one image, from flash 0 (ax620u-image.py): the first stage and
# its backup in the first two blocks, U-Boot behind its Axera header
# (axera-image.py) at 0x40000, in three blocks with a fourth for a bad one.
# The first stage is openipc/ax620u-spl built here;
# u-boot-<target>-stockspl.bin has the cameras' own instead
# (openipc/ax620u-spl/stock.bin, changed to load U-Boot from 0x40000), the
# fallback. SPL=stock puts that one in u-boot-<target>.bin and builds
# nothing else.
ax620u_image() {
    target=$1
    builddir=$2
    boot=$(part_size "${builddir}/.config")
    [ -n "${boot}" ] ||
        { echo "${target}: no boot partition in mtdparts" >&2; return 1; }
    uboot=${builddir}/u-boot-boot.bin
    python3 openipc/axera-image.py "${builddir}/u-boot.bin" "${uboot}" \
        $((boot * 1024)) || return 1
    key=openipc/axera-sdk-test-key.pem
    # <out> <first stage arguments>
    pack() {
        out=$1
        shift
        python3 openipc/ax620u-image.py "$@" "${key}" "${uboot}" \
            $((boot * 1024)) "output/${out}"
    }
    stock=openipc/ax620u-spl/stock.bin
    case "${SPL:-source}" in
        stock)
            rm -f "output/u-boot-${target}-stockspl.bin"
            pack "u-boot-${target}.bin" --stock "${stock}"
            return ;;
        source) ;;
        *) echo "SPL=${SPL}: source or stock" >&2; return 1 ;;
    esac
    spldir=${builddir}/spl
    make -C openipc/ax620u-spl O="${spldir}" CROSS_COMPILE=${CROSS_COMPILE} \
        -j${JOBS:-$(nproc)} </dev/null &&
    pack "u-boot-${target}.bin" "${spldir}/spl.bin" &&
    pack "u-boot-${target}-stockspl.bin" --stock "${stock}"
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
# at 0x10000, all before the environment (ax620e-image.py). The first stage
# is openipc/ax620e-spl built here; u-boot-<target>-stockspl.bin has the
# Xiongmai cameras' own instead (openipc/ax620e-spl/stock.bin), the
# fallback. SPL=stock puts that one in u-boot-<target>.bin and builds
# nothing else.
ax620e_image() {
    target=$1
    builddir=$2
    tool=$(ax_gzip) || return 1
    text=$(sed -n 's/^CONFIG_SYS_TEXT_BASE=//p' "${builddir}/.config")
    env=$(sed -n 's/^CONFIG_ENV_OFFSET=//p' "${builddir}/.config")
    # <out> <first stage arguments>
    pack() {
        out=$1
        shift
        python3 openipc/ax620e-image.py "$@" "${builddir}/u-boot.bin" \
            "${text}" "${tool}" openipc/axera-sdk-test-key.pem "${env}" \
            "output/${out}"
    }
    stock=openipc/ax620e-spl/stock.bin
    case "${SPL:-source}" in
        stock)
            rm -f "output/u-boot-${target}-stockspl.bin"
            pack "u-boot-${target}.bin" --signed "${stock}"
            return ;;
        source) ;;
        *) echo "SPL=${SPL}: source or stock" >&2; return 1 ;;
    esac
    spldir=${builddir}/spl
    make -C openipc/ax620e-spl O="${spldir}" CROSS_COMPILE=${CROSS_COMPILE_64} \
        -j${JOBS:-$(nproc)} </dev/null &&
    pack "u-boot-${target}.bin" "${spldir}/spl.bin" &&
    pack "u-boot-${target}-stockspl.bin" --signed "${stock}"
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
        ax620e-*) ax620e_image "${target}" "${builddir}" ;;
        ax620u-*) ax620u_image "${target}" "${builddir}" ;;
        *) echo "${target}: no image layout" >&2; return 1 ;;
    esac
}

ALL_TARGETS=$(cd openipc && ls *_config | sed 's/_config$//')

TARGETS="$@"
[ -z "${TARGETS}" ] && TARGETS="${ALL_TARGETS}"

rc=0
for target in ${TARGETS}; do
    build_target "${target}" || { echo "build failed for ${target}" >&2; rc=1; }
done

exit ${rc}
