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

# The boot partition: the environment starts at 0x40000. On an SD card
# the image starts at 8 KiB and must end before the first partition at
# 1 MiB; the NOR limit is the tighter.
BOOT_PART_SIZE=262144
# U-Boot proper's offset in the image, where the SPL reads it from
UBOOT_OFFSET=32768

mkdir -p output

# Concatenate the config fragments into the build directory's .config;
# later fragments override earlier ones.
mkconfig() {
    builddir=$1
    shift
    mkdir -p "${builddir}"
    cat "$@" > "${builddir}/.config"
}

# Allwinner V831/V833: the boot ROM loads the SPL (eGON.BT0) into SRAM,
# from SPI NOR offset 0 ("nor") or SD card sector 16 ("mmc"); the SPL
# brings up the DDR and unpacks U-Boot, an LZMA-compressed legacy image,
# from 0x8000 after itself to its link address.
build_v83x() {
    soc=$1
    kind=$2
    builddir=${BOOT_SRCDIR}/output/build-${soc}-${kind}
    make_opts="-C ${BOOT_SRCDIR} O=${builddir} -j$(nproc)"
    out=output/u-boot-${soc}-${kind}.bin
    fragments="openipc/openipc_config openipc/${soc}_config"
    [ "${kind}" = mmc ] && fragments="${fragments} openipc/mmc_config"

    rm -rf "${builddir}"
    mkconfig "${builddir}" ${fragments} && \
    make ${make_opts} olddefconfig </dev/null >/dev/null && \
    make ${make_opts} </dev/null || return 1

    spl=${builddir}/spl/sunxi-spl.bin
    if [ "$(stat -c %s "${spl}")" -gt ${UBOOT_OFFSET} ]; then
        echo "${soc}: SPL larger than ${UBOOT_OFFSET} bytes" >&2
        return 1
    fi

    text=$(sed -n 's/^CONFIG_SYS_TEXT_BASE=//p' "${builddir}/.config")
    xz --format=lzma -9 -c "${builddir}/u-boot.bin" \
        > "${builddir}/u-boot.bin.lzma" || return 1
    "${builddir}/tools/mkimage" -A arm -O u-boot -T firmware -C lzma \
        -a "${text}" -e "${text}" -n "U-Boot ${soc}" \
        -d "${builddir}/u-boot.bin.lzma" "${builddir}/u-boot-lzma.img" \
        >/dev/null || return 1

    # SPL, padded with 0xff to U-Boot's offset, then U-Boot
    { cat "${spl}"
      head -c $((UBOOT_OFFSET - $(stat -c %s "${spl}"))) /dev/zero | \
          tr '\0' '\377'
      cat "${builddir}/u-boot-lzma.img"; } > "${out}" || return 1

    size=$(stat -c %s "${out}")
    if [ "${size}" -gt ${BOOT_PART_SIZE} ]; then
        echo "${soc}: ${out} is ${size} bytes, over the" \
             "${BOOT_PART_SIZE}-byte boot partition" >&2
        rm -f "${out}"
        return 1
    fi
    echo "${out}: ${size} bytes"
}

build_soc() {
    soc=$1
    case "${soc}" in
        v831|v833)
            build_v83x "${soc}" nor && build_v83x "${soc}" mmc ;;
        *)
            echo "unsupported SoC: ${soc}" >&2
            return 1 ;;
    esac
}

ALL_SOCS="v831 v833"

SOCS="$@"
[ -z "${SOCS}" ] && SOCS="${ALL_SOCS}"

rc=0
for soc in ${SOCS}; do
    build_soc "${soc}" || { echo "build failed for ${soc}" >&2; rc=1; }
done

exit ${rc}
