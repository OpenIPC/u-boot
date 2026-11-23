#!/bin/sh
#
# Build U-Boot for the SoCs named on the command line, or for every
# supported SoC when invoked without arguments. Failures are collected:
# every requested SoC is attempted and the script exits non-zero if any
# of them failed.
#
# Each SoC gives output/u-boot-<soc>-nor.bin and u-boot-<soc>-nand.bin,
# written at flash 0, within the boot partition: the ID block (the DDR
# blob from rkbin and the SPL, openipc/rockchip-idb.py) and u-boot.itb
# (U-Boot and its device tree, LZMA), which the SPL loads.
#   NOR:  the ID block at 0x8000, u-boot.itb at SYS_SPI_U_BOOT_OFFS (0x1a000), all
#         within 256 KiB (the environment is at 0x40000).
#   NAND: the ID block in block 0 and again in block 1, u-boot.itb from
#         block 2 (0x40000), all within the 768 KiB boot partition.

cd "$(dirname "$0")"

export ARCH=arm
export CROSS_COMPILE=${CROSS_COMPILE:-arm-linux-gnueabi-}

mkdir -p output

# NOR: the ID block at sector 64, as Rockchip's NOR layout has it; the
# environment starts at 0x40000
NOR_IDB=0x8000
NOR_BOOT_SIZE=0x40000
# NAND: the ID block and its copy in the first two 128 KiB blocks,
# u-boot.itb behind them (include/configs/<soc>.h, RV11XX_NAND_ITB)
NAND_IDB="0x0 0x20000"
NAND_ITB=0x40000
NAND_BOOT_SIZE=0xc0000

# The DDR-init blob each SoC's ID block carries (rkbin, openipc/)
ddr_blob() {
    case "$1" in
        rv1106|rv1103) echo openipc/rv1106_ddr_924MHz_v1.16.bin ;;
        rv1103b) echo openipc/rv1103b_ddr_924MHz_v1.07.bin ;;
        rv1126|rv1109) echo openipc/rv1126_ddr_924MHz_v1.16.bin ;;
    esac
}

# The die each SoC is: its device trees and config fragment. The RV1103
# is the RV1106 die, the RV1109 the RV1126's
dt_soc() {
    case "$1" in
        rv1103) echo rv1106 ;;
        rv1109) echo rv1126 ;;
        *) echo "$1" ;;
    esac
}

config_value() {
    sed -n "s/^CONFIG_$2=\(.*\)/\1/p" "$1"
}

# <soc> <flash>: .config from the fragments, build, pack
build_image() {
    soc=$1
    flash=$2
    builddir=output/build-${soc}-${flash}
    rm -rf "${builddir}"
    mkdir -p "${builddir}"
    {
        # A SoC sharing its die's config: only CONFIG_PRODUCT_SOC differs
        cat openipc/openipc_config "openipc/$(dt_soc "${soc}")_config"
        [ "${flash}" = nand ] && cat openipc/nand_config
        echo "CONFIG_DEFAULT_DEVICE_TREE=\"$(dt_soc "${soc}")-openipc-${flash}\""
        echo "CONFIG_PRODUCT_SOC=\"${soc}\""
    } > "${builddir}/.config"
    make O="${builddir}" olddefconfig </dev/null >/dev/null || return 1
    make O="${builddir}" -j"$(nproc)" </dev/null || return 1

    ddr=$(ddr_blob "${soc}")
    [ -n "${ddr}" ] || { echo "${soc}: no DDR blob" >&2; return 1; }
    idb=${builddir}/idblock.bin
    case "${soc}" in
    rv1126|rv1109)
        # The older ID block (header v1, RC4-scrambled header, "110B"):
        # mainline's mkimage, as for the RK3399 with rkbin's DDR blob
        "${builddir}/tools/mkimage" -n rv1126 -T rksd \
            -d "${ddr}:${builddir}/spl/u-boot-spl.bin" "${idb}" \
            >/dev/null || return 1 ;;
    *)
        python3 openipc/rockchip-idb.py "${ddr}" \
            "${builddir}/spl/u-boot-spl.bin" "${idb}" || return 1 ;;
    esac
    itb=${builddir}/u-boot.itb
    if [ "${flash}" = nand ]; then
        idbs=${NAND_IDB}
        itb_offset=${NAND_ITB}
        limit=${NAND_BOOT_SIZE}
    else
        idbs=${NOR_IDB}
        itb_offset=$(config_value "${builddir}/.config" SYS_SPI_U_BOOT_OFFS)
        limit=${NOR_BOOT_SIZE}
    fi
    itb_end=$((itb_offset + $(stat -c %s "${itb}")))
    for at in ${idbs}; do
        end=$((at + $(stat -c %s "${idb}")))
        # each copy before the next thing
        for next in ${idbs} ${itb_offset}; do
            if [ $((next)) -gt $((at)) ] && [ "${end}" -gt $((next)) ]; then
                printf '%s: the ID block at %s runs into 0x%x\n' \
                    "${soc}" "${at}" $((next)) >&2
                return 1
            fi
        done
    done
    if [ "${itb_end}" -gt $((limit)) ]; then
        printf '%s: u-boot.itb ends at 0x%x, past the boot partition (%s)\n' \
            "${soc}" "${itb_end}" "${limit}" >&2
        return 1
    fi
    out=output/u-boot-${soc}-${flash}.bin
    # 0xff (erased flash) up to the end of u-boot.itb
    tr '\0' '\377' < /dev/zero | head -c "${itb_end}" > "${out}"
    for at in ${idbs}; do
        dd if="${idb}" of="${out}" bs=1 seek=$((at)) conv=notrunc \
            status=none
    done
    dd if="${itb}" of="${out}" bs=1 seek=$((itb_offset)) conv=notrunc \
        status=none
    printf '%s: %s, %d bytes (ID block at %s, u-boot.itb to 0x%x)\n' \
        "${soc}" "${out}" "${itb_end}" "$(echo ${idbs} | tr ' ' ,)" \
        "${itb_end}"
}

build_soc() {
    build_image "$1" nor && build_image "$1" nand
}

ALL_SOCS="rv1106 rv1103 rv1103b rv1126 rv1109"

SOCS="$*"
[ -z "${SOCS}" ] && SOCS="${ALL_SOCS}"

rc=0
for soc in ${SOCS}; do
    build_soc "${soc}" || { echo "build failed for ${soc}" >&2; rc=1; }
done

exit ${rc}
