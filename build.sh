#!/bin/sh
#
# Build U-Boot for the SoCs named on the command line, or for every
# supported SoC when invoked without arguments. Failures are collected:
# every requested SoC is attempted and the script exits non-zero if any
# of them failed.

cd "$(dirname "$0")"

export ARCH=arm
export CROSS_COMPILE=${CROSS_COMPILE:-arm-linux-gnueabi-}

export BOOT_SRCDIR=$(pwd)

mkdir -p output

build_gzip() {
    if [ -x tools/utils/bin/gzip ] && [ -z "$GZIP_REBUILD" ]; then
        return 0
    fi
    echo "Building gzip from source (extras/gzip-1.11, WSIZE=0x2000)"
    # Keep autotools from regenerating Makefile.in/configure on a fresh checkout
    touch "${BOOT_SRCDIR}/extras/gzip-1.11/aclocal.m4" \
          "${BOOT_SRCDIR}/extras/gzip-1.11/configure" \
          "${BOOT_SRCDIR}/extras/gzip-1.11/Makefile.in" \
          "${BOOT_SRCDIR}/extras/gzip-1.11/lib/Makefile.in" \
          "${BOOT_SRCDIR}/extras/gzip-1.11/lib/config.hin" \
          "${BOOT_SRCDIR}/extras/gzip-1.11/doc/Makefile.in" \
          "${BOOT_SRCDIR}/extras/gzip-1.11/tests/Makefile.in" \
          "${BOOT_SRCDIR}/extras/gzip-1.11/doc/stamp-vti" \
          "${BOOT_SRCDIR}/extras/gzip-1.11/doc/version.texi" \
          "${BOOT_SRCDIR}/extras/gzip-1.11/doc/gzip.info" \
          "${BOOT_SRCDIR}/extras/gzip-1.11/gzip.1" \
          "${BOOT_SRCDIR}/extras/gzip-1.11/gunzip.1" \
          "${BOOT_SRCDIR}/extras/gzip-1.11/gzexe.1"
    rm -rf output/gzip-build
    mkdir -p output/gzip-build tools/utils/bin
    ( cd output/gzip-build && \
      "${BOOT_SRCDIR}/extras/gzip-1.11/configure" \
          CFLAGS="-O2 -DWSIZE=0x2000" >/dev/null && \
      make -j"$(nproc)" >/dev/null ) || return 1
    cp output/gzip-build/gzip tools/utils/bin/gzip || return 1
    strip tools/utils/bin/gzip 2>/dev/null || true
}

build_gzip || { echo "gzip build failed" >&2; exit 1; }

mkflags(){
    BOOT_BUILDDIR=${BOOT_SRCDIR}/output/build-${soc}
    MAKE_OPTS="-C ${BOOT_SRCDIR} O=${BOOT_BUILDDIR} -j`nproc`"
}

make distclean </dev/null >/dev/null 2>&1

# V500 family (hi3516cv500 / dv300 / av300): NOR + NAND, mini-boot
build_v500() {
    soc=$1
    mkflags

    rm -rf ${BOOT_BUILDDIR}
    mkdir -p ${BOOT_BUILDDIR}/drivers/ddr/
    cp -arf ${BOOT_SRCDIR}/drivers/ddr/hisilicon ${BOOT_BUILDDIR}/drivers/ddr/
    cp openipc/${soc}_reginfo.bin ${BOOT_BUILDDIR}/.reg
    # NOR
    cat ${BOOT_SRCDIR}/openipc/openipc_config > ${BOOT_BUILDDIR}/.config && \
    cat ${BOOT_SRCDIR}/openipc/${soc}_config >> ${BOOT_BUILDDIR}/.config && \
    echo "CONFIG_PRODUCT_SOC=\"${soc}\"" >> ${BOOT_BUILDDIR}/.config && \
    echo "CONFIG_PRODUCT_SOCMODEL=\"${model}\"" >> ${BOOT_BUILDDIR}/.config && \
    make ${MAKE_OPTS} olddefconfig </dev/null >/dev/null && \
    make ${MAKE_OPTS} KCFLAGS="-DVENDOR_HISILICON" </dev/null && \
    make ${MAKE_OPTS} u-boot-z.bin </dev/null && \
    cp ${BOOT_BUILDDIR}/u-boot-${soc}.bin ${BOOT_BUILDDIR}/../u-boot-${soc}-nor.bin || return 1
    # NAND
    cat ${BOOT_SRCDIR}/openipc/openipc_config > ${BOOT_BUILDDIR}/.config && \
    cat ${BOOT_SRCDIR}/openipc/${soc}_config >> ${BOOT_BUILDDIR}/.config && \
    cat ${BOOT_SRCDIR}/openipc/nand_config >> ${BOOT_BUILDDIR}/.config && \
    echo "CONFIG_PRODUCT_SOC=\"${soc}\"" >> ${BOOT_BUILDDIR}/.config && \
    make ${MAKE_OPTS} olddefconfig </dev/null >/dev/null && \
    make ${MAKE_OPTS} KCFLAGS="-DVENDOR_HISILICON" </dev/null && \
    make ${MAKE_OPTS} u-boot-z.bin </dev/null && \
    cp ${BOOT_BUILDDIR}/u-boot-${soc}.bin ${BOOT_BUILDDIR}/../u-boot-${soc}-nand.bin || return 1
}

# Signed boot images: the boot ROM of these SoCs starts a GSL first stage,
# which trains DDR from a register table and loads U-Boot. image_tool packs
# the three into boot-<soc>[-<model>]-nor.bin, one per DDR binning; U-Boot
# itself is the same for all of a SoC's binnings and is built once.
#
#   soc  model  reginfo/<table>  arch  packer  gsl
#
# arch picks the cross compiler (arm: CROSS_COMPILE, arm64: CROSS_COMPILE64);
# packer is the image_tool directory of the SoC's SDK generation; gsl is the
# GSL source the packer's input/gsl.bin is built from, a prebuilt GSL to pack
# instead of it, or "-" to use the prebuilt one it ships. A model of "-" names
# the image after the SoC alone.
#
# hi3516cv610 20s/20g pack image_tool/input/gsl-20250416.bin, the GSL of a
# production K662C6S (ddr param version 20250416): on that module the shipped
# gsl.bin hangs in DDR training right after the boot table (u-boot#69,
# defib#148). Only there: on a Hi3516CV608 with an FM25Q64A it trains, but
# Linux's bsp-sfc then fails to probe the flash ("spi nor register fail"),
# so it is not a replacement for gsl.bin.
BOOTIMG_TABLE="
hi3516cv610 10b     Hi3516CV610-DMEB_4L_DDR2_1333M_64MB_16bit-A7_950M_QFN.bin        arm   image_tool             -
hi3516cv610 20s     Hi3516CV610-DMEB_4L_DDR3_2133M_128MB_16bit-A7_950M_QFN.bin       arm   image_tool             image_tool/input/gsl-20250416.bin
hi3516cv610 20g     Hi3516CV610-DMEB_4L_DDR3_2133M_128MB_16bit-A7_950M_QFN.bin       arm   image_tool             image_tool/input/gsl-20250416.bin
hi3516cv610 00s     Hi3516CV610-DMEB_4L_DDR3_2133M_512MB_16bit-A7_950M_BGA.bin       arm   image_tool             -
hi3516cv610 00g     Hi3516CV610-DMEB_4L_DDR3_2133M_512MB_16bit-A7_950M_BGA.bin       arm   image_tool             -
hi3516cv608 -       Hi3516CV608-DMEB_4L_DDR2_1333M_64MB_16bit-A7_950M_QFN.bin        arm   image_tool             -
hi3519dv500 dmeb    Hi3519DV500-DMEB_6L_T_DDR4_2666M-2GB_16bitx2-A55_1G.bin          arm64 image_tool_hi3519dv500 gsl_hi3519dv500
hi3519dv500 dmebpro Hi3519DV500-DMEBPRO_4L_flyby_DDR4_2666M-2GB_16bitx2-A55_1G.bin   arm64 image_tool_hi3519dv500 gsl_hi3519dv500
"

bootimg_rows() {
    echo "${BOOTIMG_TABLE}" | awk -v soc="$1" '$1 == soc'
}

build_bootimg() {
    soc=$1
    rows=$(bootimg_rows ${soc})
    arch=$(echo "${rows}" | awk 'NR == 1 {print $4}')
    cc=${CROSS_COMPILE}
    [ "${arch}" = arm64 ] && cc=${CROSS_COMPILE64:-aarch64-linux-gnu-}
    mkflags

    # The DDR training command binary builds in the source tree: the
    # ddr_training_clean drops what the SoC before left there, which may be
    # the other architecture's.
    rm -rf ${BOOT_BUILDDIR}
    mkdir -p ${BOOT_BUILDDIR}
    cat ${BOOT_SRCDIR}/openipc/openipc_config > ${BOOT_BUILDDIR}/.config && \
    cat ${BOOT_SRCDIR}/openipc/${soc}_config >> ${BOOT_BUILDDIR}/.config && \
    echo "CONFIG_PRODUCT_SOC=\"${soc}\"" >> ${BOOT_BUILDDIR}/.config && \
    make ${MAKE_OPTS} CROSS_COMPILE=${cc} olddefconfig </dev/null >/dev/null && \
    make ${MAKE_OPTS} CROSS_COMPILE=${cc} ddr_training_clean </dev/null >/dev/null && \
    make ${MAKE_OPTS} CROSS_COMPILE=${cc} KCFLAGS="-DVENDOR_HISILICON" </dev/null && \
    make ${MAKE_OPTS} CROSS_COMPILE=${cc} u-boot-z.bin </dev/null && \
    make ${MAKE_OPTS} CROSS_COMPILE=${cc} u-boot-z.clean </dev/null && \
    cp ${BOOT_BUILDDIR}/u-boot-${soc}.bin output/smoke-${soc}.bin || return 1

    # The packer reads one fixed input/gsl.bin. A row that names a prebuilt
    # GSL swaps it in for its own image only, so the tracked default is put
    # back before every other row and once the table is done.
    # A packer whose GSL is built from source ships no default.
    packer=$(echo "${rows}" | awk 'NR == 1 {print $5}')
    dflt=${BOOT_BUILDDIR}/gsl-default.bin
    rm -f ${dflt}
    [ -f ${packer}/input/gsl.bin ] && { cp ${packer}/input/gsl.bin ${dflt} || return 1; }
    echo "${rows}" | while read -r _ model reginfo _ packer gsl; do
        if [ "${gsl}" = - ]; then
            cp ${dflt} ${packer}/input/gsl.bin || return 1
        elif [ -f "${gsl}" ]; then
            cp ${gsl} ${packer}/input/gsl.bin || return 1
        else
            # Its Makefile takes include paths from $(PWD): build from inside.
            (cd ${gsl} && make CROSS_COMPILE=${cc} CHIP=${soc} </dev/null >/dev/null) && \
            cp ${gsl}/pub/gsl.bin ${packer}/input/gsl.bin || return 1
        fi
        out=boot-${soc}-${model}-nor.bin
        [ "${model}" = - ] && out=boot-${soc}-nor.bin
        # The packer reports failures but still exits 0: check its output.
        rm -f ${packer}/image/oem/boot_image.bin
        cp ${BOOT_BUILDDIR}/u-boot-${soc}.bin ${packer}/input/u-boot-original.bin && \
        cp reginfo/${reginfo} ${packer}/input/reg_info.bin && \
        (cd ${packer}/oem && python3 oem_quick_build.py >/dev/null) && \
        test -f ${packer}/image/oem/boot_image.bin && \
        mv ${packer}/image/oem/boot_image.bin output/${out} || return 1
    done || return 1
    [ -f ${dflt} ] && cp ${dflt} ${packer}/input/gsl.bin
    return 0
}

# ARM926EJS families (hi3516cv200 / hi3518ev200, hi3516cv300 / hi3516ev100):
# NOR, plus NAND when the third argument is "nand"
build_nor_ddr() {
    soc=$1
    out=$2
    flash=$3
    mkflags

    rm -rf ${BOOT_BUILDDIR}
    mkdir -p ${BOOT_BUILDDIR}/drivers/ddr/
    cp -arf ${BOOT_SRCDIR}/drivers/ddr/hisilicon ${BOOT_BUILDDIR}/drivers/ddr/
    cp openipc/${soc}_reginfo.bin ${BOOT_BUILDDIR}/.reg
    cat ${BOOT_SRCDIR}/openipc/openipc_config > ${BOOT_BUILDDIR}/.config && \
    cat ${BOOT_SRCDIR}/openipc/${soc}_config >> ${BOOT_BUILDDIR}/.config && \
    echo "CONFIG_PRODUCT_SOC=\"${soc}\"" >> ${BOOT_BUILDDIR}/.config && \
    make ${MAKE_OPTS} olddefconfig </dev/null >/dev/null && \
    make ${MAKE_OPTS} KCFLAGS="-DVENDOR_HISILICON" BOOT_SRCDIR=${BOOT_SRCDIR} </dev/null && \
    make ${MAKE_OPTS} u-boot-z.bin </dev/null && \
    cp ${BOOT_BUILDDIR}/${out} ${BOOT_BUILDDIR}/../u-boot-${soc}-nor.bin || return 1
    [ "${flash}" = nand ] || return 0
    # NAND
    cat ${BOOT_SRCDIR}/openipc/openipc_config > ${BOOT_BUILDDIR}/.config && \
    cat ${BOOT_SRCDIR}/openipc/${soc}_config >> ${BOOT_BUILDDIR}/.config && \
    cat ${BOOT_SRCDIR}/openipc/nand_config >> ${BOOT_BUILDDIR}/.config && \
    echo "CONFIG_PRODUCT_SOC=\"${soc}\"" >> ${BOOT_BUILDDIR}/.config && \
    make ${MAKE_OPTS} olddefconfig </dev/null >/dev/null && \
    make ${MAKE_OPTS} KCFLAGS="-DVENDOR_HISILICON" BOOT_SRCDIR=${BOOT_SRCDIR} </dev/null && \
    make ${MAKE_OPTS} u-boot-z.bin </dev/null && \
    cp ${BOOT_BUILDDIR}/${out} ${BOOT_BUILDDIR}/../u-boot-${soc}-nand.bin || return 1
}

# hi3519v101/av200, hi3520, hi3536, hi3516a, hi3518ev100: NOR, reg_info splice
build_nor_splice() {
    soc=$1
    out=$2
    mkflags

    rm -rf ${BOOT_BUILDDIR}
    mkdir -p ${BOOT_BUILDDIR}
    cp openipc/${soc}_reginfo.bin ${BOOT_BUILDDIR}/.reg
    cat ${BOOT_SRCDIR}/openipc/openipc_config > ${BOOT_BUILDDIR}/.config && \
    cat ${BOOT_SRCDIR}/openipc/${soc}_config >> ${BOOT_BUILDDIR}/.config && \
    echo "CONFIG_PRODUCT_SOC=\"${soc}\"" >> ${BOOT_BUILDDIR}/.config && \
    make ${MAKE_OPTS} olddefconfig </dev/null >/dev/null && \
    make ${MAKE_OPTS} KCFLAGS="-DVENDOR_HISILICON" BOOT_SRCDIR=${BOOT_SRCDIR} </dev/null && \
    make ${MAKE_OPTS} u-boot-z.bin </dev/null && \
    cp ${BOOT_BUILDDIR}/${out} ${BOOT_BUILDDIR}/../u-boot-${soc}-nor.bin || return 1
}

# Goke/XMedia family: NOR + NAND
build_xmedia() {
    soc=$1
    cfg=$2
    kcflags=$3
    with_cmd_bin=$4
    mkflags

    rm -rf ${BOOT_BUILDDIR}
    mkdir -p ${BOOT_BUILDDIR}/drivers/ddr/xmedia/
    if [ "${with_cmd_bin}" = "yes" ]; then
        mkdir -p ${BOOT_BUILDDIR}/drivers/ddr/xmedia/xmedia/default/cmd_bin/
    fi
    cp -arf ${BOOT_SRCDIR}/drivers/ddr/xmedia/* ${BOOT_BUILDDIR}/drivers/ddr/xmedia/
    cp openipc/${soc}_reginfo.bin ${BOOT_BUILDDIR}/.reg
    # NOR
    cat ${BOOT_SRCDIR}/openipc/openipc_config > ${BOOT_BUILDDIR}/.config && \
    cat ${BOOT_SRCDIR}/openipc/${cfg} >> ${BOOT_BUILDDIR}/.config && \
    echo "CONFIG_PRODUCT_SOC=\"${soc}\"" >> ${BOOT_BUILDDIR}/.config && \
    make ${MAKE_OPTS} olddefconfig </dev/null >/dev/null && \
    make ${MAKE_OPTS} KCFLAGS="${kcflags}" BOOT_SRCDIR=${BOOT_SRCDIR} </dev/null && \
    make ${MAKE_OPTS} u-boot-z.bin </dev/null && \
    cp ${BOOT_BUILDDIR}/u-boot-xm*.bin ${BOOT_BUILDDIR}/../u-boot-${soc}-nor.bin || return 1
    # NAND
    cat ${BOOT_SRCDIR}/openipc/openipc_config > ${BOOT_BUILDDIR}/.config && \
    cat ${BOOT_SRCDIR}/openipc/${cfg} >> ${BOOT_BUILDDIR}/.config && \
    cat ${BOOT_SRCDIR}/openipc/nand_config >> ${BOOT_BUILDDIR}/.config && \
    echo "CONFIG_PRODUCT_SOC=\"${soc}\"" >> ${BOOT_BUILDDIR}/.config && \
    make ${MAKE_OPTS} olddefconfig </dev/null >/dev/null && \
    make ${MAKE_OPTS} KCFLAGS="${kcflags}" BOOT_SRCDIR=${BOOT_SRCDIR} </dev/null && \
    make ${MAKE_OPTS} u-boot-z.bin </dev/null && \
    cp ${BOOT_BUILDDIR}/u-boot-xm*.bin ${BOOT_BUILDDIR}/../u-boot-${soc}-nand.bin || return 1
}

# Goke GK7206V1 (xmedia xmorca): NOR + NAND. The register table
# is all that differs between the two dies; the other four (V10, V10B, V11,
# V11AT) have none in the SDK.
build_xmorca() {
    soc=$1
    mkflags

    rm -rf ${BOOT_BUILDDIR}
    mkdir -p ${BOOT_BUILDDIR}
    cp openipc/${soc}_reginfo.bin ${BOOT_BUILDDIR}/.reg
    for flash in nor nand; do
        cat ${BOOT_SRCDIR}/openipc/openipc_config > ${BOOT_BUILDDIR}/.config && \
        cat ${BOOT_SRCDIR}/openipc/gk7206v100_config >> ${BOOT_BUILDDIR}/.config && \
        if [ ${flash} = nand ]; then \
            cat ${BOOT_SRCDIR}/openipc/nand_config >> ${BOOT_BUILDDIR}/.config; \
        fi && \
        echo "CONFIG_PRODUCT_SOC=\"${soc}\"" >> ${BOOT_BUILDDIR}/.config && \
        make ${MAKE_OPTS} olddefconfig </dev/null >/dev/null && \
        make ${MAKE_OPTS} BOOT_SRCDIR=${BOOT_SRCDIR} </dev/null && \
        make ${MAKE_OPTS} u-boot-z.bin </dev/null && \
        cp ${BOOT_BUILDDIR}/u-boot-xmorca.bin ${BOOT_BUILDDIR}/../u-boot-${soc}-${flash}.bin || return 1
    done
}

build_soc() {
    soc=$1
    if [ -n "$(bootimg_rows ${soc})" ]; then
        build_bootimg ${soc}
        return
    fi
    case "${soc}" in
        hi3516cv500|hi3516dv300|hi3516av300)
            build_v500 ${soc} ;;
        hi3516cv200|hi3518ev200)
            build_nor_ddr ${soc} u-boot-hi3518ev200.bin ;;
        hi3516cv300)
            build_nor_ddr ${soc} u-boot-hi3516cv300.bin nand ;;
        hi3516ev100)
            build_nor_ddr ${soc} u-boot-hi3516cv300.bin ;;
        hi3516av200|hi3519v101)
            build_nor_splice ${soc} u-boot-${soc}.bin ;;
        hi3515av100|hi3520av100|hi3520dv100|hi3520dv200)
            build_nor_splice ${soc} u-boot-hi3520d.bin ;;
        hi3536cv100)
            build_nor_splice ${soc} u-boot-hi3536c.bin ;;
        hi3536dv100)
            build_nor_splice ${soc} u-boot-hi3536dv100.bin ;;
        hi3516av100|hi3516dv100)
            build_nor_splice ${soc} u-boot-hi3516a.bin ;;
        hi3518av100|hi3518cv100|hi3518ev100|hi3516cv100)
            build_nor_splice ${soc} u-boot-hi3518ev100.bin ;;
        gk7201v200|gk7201v300|gk7205v200|gk7205v300|gk7202v300|gk7605v100)
            build_xmedia ${soc} ${soc}_config "" no ;;
        hi3516dv200|hi3516ev200|hi3516ev300|hi3518ev300)
            build_xmedia ${soc} ${soc}_config "-DPRODUCT_SOC=${soc} -DVENDOR_HISILICON" yes ;;
        gk7205v500|gk7205v510|gk7205v530)
            build_xmedia ${soc} xm720xxx_config "" no ;;
        gk7206v11a|gk7206v12a)
            build_xmorca ${soc} ;;
        *)
            echo "unsupported SoC: ${soc}" >&2
            return 1 ;;
    esac
}

ALL_SOCS="hi3516cv500 hi3516dv300 hi3516av300 \
          hi3516cv610 hi3516cv608 hi3519dv500 \
          hi3516cv200 hi3518ev200 \
          hi3516cv300 hi3516ev100 \
          hi3516av200 hi3519v101 \
          hi3515av100 hi3520av100 hi3520dv100 hi3520dv200 \
          hi3536cv100 hi3536dv100 \
          hi3516av100 hi3516dv100 \
          hi3518av100 hi3518cv100 hi3518ev100 hi3516cv100 \
          gk7201v200 gk7201v300 gk7205v200 gk7205v300 gk7202v300 gk7605v100 \
          hi3516dv200 hi3516ev200 hi3516ev300 hi3518ev300 \
          gk7205v500 gk7205v510 gk7205v530 \
          gk7206v11a gk7206v12a"

SOCS="$@"
[ -z "${SOCS}" ] && SOCS="${ALL_SOCS}"

rc=0
for soc in ${SOCS}; do
    build_soc ${soc} || { echo "build failed for ${soc}" >&2; rc=1; }
done

make distclean </dev/null >/dev/null 2>&1
exit ${rc}
