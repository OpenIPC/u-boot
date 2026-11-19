#!/bin/sh
#
# Build U-Boot for the targets named on the command line, or for every
# target with an openipc/<target>_config when invoked without arguments.
# A target is <soc>-<flash>, e.g. ax620u-nand; the image is
# output/u-boot-<soc>-<flash>.bin (for AX620U also the first stage's,
# output/spl-<soc>-<flash>.bin; see ax620u_spl()). Failures are collected: every requested
# target is attempted and the script exits non-zero if any of them failed.

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

# AX620U: the first stage, openipc/ax620u-spl built here, laid out for the
# start of the spl partition (ax620u-spl-image.py): spl-<target>.bin.
# spl-<target>-stock.bin has the cameras' own first stage instead
# (openipc/ax620u-spl/stock.bin), the fallback. SPL=stock puts that one in
# spl-<target>.bin and builds nothing else. U-Boot's image is unchanged.
ax620u_spl() {
    target=$1
    builddir=$2
    limit=$(part_size "${builddir}/.config" spl)
    [ -n "${limit}" ] || { echo "${target}: no spl partition in mtdparts" >&2; return 1; }
    out=output/spl-${target}.bin
    stock=openipc/ax620u-spl/stock.bin
    case "${SPL:-source}" in
        stock)
            rm -f "output/spl-${target}-stock.bin"
            python3 openipc/ax620u-spl-image.py --signed "${stock}" "${out}" ;;
        source)
            spldir=${builddir}/spl
            make -C openipc/ax620u-spl O="${spldir}" CROSS_COMPILE=${CROSS_COMPILE} \
                -j${JOBS:-$(nproc)} </dev/null &&
            python3 openipc/ax620u-spl-image.py "${spldir}/spl.bin" \
                openipc/axera-sdk-test-key.pem "${out}" &&
            python3 openipc/ax620u-spl-image.py --signed "${stock}" \
                "output/spl-${target}-stock.bin" ;;
        *) echo "SPL=${SPL}: source or stock" >&2; false ;;
    esac || return 1
    for f in "${out}" "output/spl-${target}-stock.bin"; do
        [ ! -f "${f}" ] || [ "$(stat -c %s "${f}")" -le $((limit * 1024)) ] || {
            echo "${f}: larger than the ${limit} KiB spl partition" >&2
            return 1
        }
    done
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
        "output/u-boot-${target}.bin" $((limit * 1024)) || return 1

    case "${target}" in
        ax620u-*) ax620u_spl "${target}" "${builddir}" ;;
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
