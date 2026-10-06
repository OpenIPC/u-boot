#!/bin/sh
set -e

cd "$(dirname "$0")"

export ARCH=arm
export CROSS_COMPILE=${CROSS_COMPILE:-arm-linux-gnueabi-}

export BOOT_SRCDIR=$(pwd)

mkdir -p output

build_gzip() {
    if [ -x tools/utils/bin/gzip ] && [ -z "$GZIP_REBUILD" ]; then
        return
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
      make -j"$(nproc)" >/dev/null )
    cp output/gzip-build/gzip tools/utils/bin/gzip
    strip tools/utils/bin/gzip 2>/dev/null || true
}
build_gzip


mkflags(){
    BOOT_BUILDDIR=${BOOT_SRCDIR}/output/build-${soc}
    MAKE_OPTS="-C ${BOOT_SRCDIR} O=${BOOT_BUILDDIR} -j`nproc`"
}

make distclean </dev/null >/dev/null 2>&1

# V500 family (hi3516cv500 / dv300 / av300): u-boot-z.bin + .reg
for soc in hi3516cv500 hi3516dv300 hi3516av300; do
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
    cp ${BOOT_BUILDDIR}/u-boot-${soc}.bin ${BOOT_BUILDDIR}/../u-boot-${soc}-nor.bin
    # NAND
    cat ${BOOT_SRCDIR}/openipc/openipc_config > ${BOOT_BUILDDIR}/.config && \
    cat ${BOOT_SRCDIR}/openipc/${soc}_config >> ${BOOT_BUILDDIR}/.config && \
    cat ${BOOT_SRCDIR}/openipc/nand_config >> ${BOOT_BUILDDIR}/.config && \
    echo "CONFIG_PRODUCT_SOC=\"${soc}\"" >> ${BOOT_BUILDDIR}/.config && \
    make ${MAKE_OPTS} olddefconfig </dev/null >/dev/null && \
    make ${MAKE_OPTS} KCFLAGS="-DVENDOR_HISILICON" </dev/null && \
    make ${MAKE_OPTS} u-boot-z.bin </dev/null && \
    cp ${BOOT_BUILDDIR}/u-boot-${soc}.bin ${BOOT_BUILDDIR}/../u-boot-${soc}-nand.bin
done

# CV6xx family: hi3516cv610 (5 DDR binnings) + hi3516cv608, combined boot images
cv6xx_build() {
    soc=$1
    model=$2
    reginfo=$3
    mkflags

    rm -rf ${BOOT_BUILDDIR}
    mkdir -p ${BOOT_BUILDDIR}
    cat ${BOOT_SRCDIR}/openipc/openipc_config > ${BOOT_BUILDDIR}/.config && \
    cat ${BOOT_SRCDIR}/openipc/${soc}_config >> ${BOOT_BUILDDIR}/.config && \
    echo "CONFIG_PRODUCT_SOC=\"${soc}\"" >> ${BOOT_BUILDDIR}/.config && \
    make ${MAKE_OPTS} olddefconfig </dev/null >/dev/null && \
    make ${MAKE_OPTS} KCFLAGS="-DVENDOR_HISILICON" </dev/null && \
    make ${MAKE_OPTS} u-boot-z.bin </dev/null && \
    make ${MAKE_OPTS} u-boot-z.clean </dev/null && \
    cp ${BOOT_BUILDDIR}/u-boot-${soc}.bin image_tool/input/u-boot-original.bin && \
    cp ${BOOT_BUILDDIR}/u-boot-${soc}.bin output/smoke-${soc}.bin && \
    cp reginfo/${reginfo} image_tool/input/reg_info.bin && \
    (cd image_tool/oem && python3 oem_quick_build.py >/dev/null) && \
    if [ -n "${model}" ]; then \
        mv image_tool/image/oem/boot_image.bin output/boot-${soc}-${model}-nor.bin; \
    else \
        mv image_tool/image/oem/boot_image.bin output/boot-${soc}-nor.bin; \
    fi
}

cv6xx_build hi3516cv610 10b "Hi3516CV610-DMEB_4L_DDR2_1333M_64MB_16bit-A7_950M_QFN.bin"
cv6xx_build hi3516cv610 20s "Hi3516CV610-DMEB_4L_DDR3_2133M_128MB_16bit-A7_950M_QFN.bin"
cv6xx_build hi3516cv610 20g "Hi3516CV610-DMEB_4L_DDR3_2133M_128MB_16bit-A7_950M_QFN.bin"
cv6xx_build hi3516cv610 00s "Hi3516CV610-DMEB_4L_DDR3_2133M_512MB_16bit-A7_950M_BGA.bin"
cv6xx_build hi3516cv610 00g "Hi3516CV610-DMEB_4L_DDR3_2133M_512MB_16bit-A7_950M_BGA.bin"
cv6xx_build hi3516cv608 ""  "Hi3516CV608-DMEB_4L_DDR2_1333M_64MB_16bit-A7_950M_QFN.bin"

# CV200 family (hi3516cv200 / hi3518ev200), ARM926EJS
for soc in hi3516cv200 hi3518ev200; do
    mkflags

    rm -rf ${BOOT_BUILDDIR}
    mkdir -p ${BOOT_BUILDDIR}/drivers/ddr/
    cp -arf ${BOOT_SRCDIR}/drivers/ddr/hisilicon ${BOOT_BUILDDIR}/drivers/ddr/
    cp openipc/${soc}_reginfo.bin ${BOOT_BUILDDIR}/.reg
    # NOR
    cat ${BOOT_SRCDIR}/openipc/openipc_config > ${BOOT_BUILDDIR}/.config && \
    cat ${BOOT_SRCDIR}/openipc/${soc}_config >> ${BOOT_BUILDDIR}/.config && \
    echo "CONFIG_PRODUCT_SOC=\"${soc}\"" >> ${BOOT_BUILDDIR}/.config && \
    make ${MAKE_OPTS} olddefconfig </dev/null >/dev/null && \
    make ${MAKE_OPTS} KCFLAGS="-DVENDOR_HISILICON" BOOT_SRCDIR=${BOOT_SRCDIR} </dev/null && \
    make ${MAKE_OPTS} u-boot-z.bin </dev/null && \
    cp ${BOOT_BUILDDIR}/u-boot-hi3518ev200.bin ${BOOT_BUILDDIR}/../u-boot-${soc}-nor.bin
done

# CV300 family (hi3516cv300 / hi3516ev100), ARM926EJS
for soc in hi3516cv300 hi3516ev100; do
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
    cp ${BOOT_BUILDDIR}/u-boot-hi3516cv300.bin ${BOOT_BUILDDIR}/../u-boot-${soc}-nor.bin
done

# Goke/XMedia family (gk7205v200 / gk7205v300 / gk7202v300 / gk7605v100)
for soc in gk7201v200 gk7201v300 gk7205v200 gk7205v300 gk7202v300 gk7605v100; do
    mkflags

    rm -rf ${BOOT_BUILDDIR}
    mkdir -p ${BOOT_BUILDDIR}/drivers/ddr/xmedia/
    cp -arf ${BOOT_SRCDIR}/drivers/ddr/xmedia/* ${BOOT_BUILDDIR}/drivers/ddr/xmedia/
    cp openipc/${soc}_reginfo.bin ${BOOT_BUILDDIR}/.reg
    # NOR
    cat ${BOOT_SRCDIR}/openipc/openipc_config > ${BOOT_BUILDDIR}/.config && \
    cat ${BOOT_SRCDIR}/openipc/${soc}_config >> ${BOOT_BUILDDIR}/.config && \
    echo "CONFIG_PRODUCT_SOC=\"${soc}\"" >> ${BOOT_BUILDDIR}/.config && \
    make ${MAKE_OPTS} olddefconfig </dev/null >/dev/null && \
    make ${MAKE_OPTS} BOOT_SRCDIR=${BOOT_SRCDIR} </dev/null && \
    make ${MAKE_OPTS} u-boot-z.bin </dev/null && \
    cp ${BOOT_BUILDDIR}/u-boot-xm*.bin ${BOOT_BUILDDIR}/../u-boot-${soc}-nor.bin
    # NAND
    cat ${BOOT_SRCDIR}/openipc/openipc_config > ${BOOT_BUILDDIR}/.config && \
    cat ${BOOT_SRCDIR}/openipc/${soc}_config >> ${BOOT_BUILDDIR}/.config && \
    cat ${BOOT_SRCDIR}/openipc/nand_config >> ${BOOT_BUILDDIR}/.config && \
    echo "CONFIG_PRODUCT_SOC=\"${soc}\"" >> ${BOOT_BUILDDIR}/.config && \
    make ${MAKE_OPTS} olddefconfig </dev/null >/dev/null && \
    make ${MAKE_OPTS} BOOT_SRCDIR=${BOOT_SRCDIR} </dev/null && \
    make ${MAKE_OPTS} u-boot-z.bin </dev/null && \
    cp ${BOOT_BUILDDIR}/u-boot-xm*.bin ${BOOT_BUILDDIR}/../u-boot-${soc}-nand.bin
done

# Goke/XMedia family (gk7205v500 / gk7205v510 / gk7205v530, shared xm720xxx config)
for soc in gk7205v500 gk7205v510 gk7205v530; do
    mkflags

    rm -rf ${BOOT_BUILDDIR}
    mkdir -p ${BOOT_BUILDDIR}/drivers/ddr/xmedia/
    cp -arf ${BOOT_SRCDIR}/drivers/ddr/xmedia/* ${BOOT_BUILDDIR}/drivers/ddr/xmedia/
    cp openipc/${soc}_reginfo.bin ${BOOT_BUILDDIR}/.reg
    # NOR
    cat ${BOOT_SRCDIR}/openipc/openipc_config > ${BOOT_BUILDDIR}/.config && \
    cat ${BOOT_SRCDIR}/openipc/xm720xxx_config >> ${BOOT_BUILDDIR}/.config && \
    echo "CONFIG_PRODUCT_SOC=\"${soc}\"" >> ${BOOT_BUILDDIR}/.config && \
    make ${MAKE_OPTS} olddefconfig </dev/null >/dev/null && \
    make ${MAKE_OPTS} BOOT_SRCDIR=${BOOT_SRCDIR} </dev/null && \
    make ${MAKE_OPTS} u-boot-z.bin </dev/null && \
    cp ${BOOT_BUILDDIR}/u-boot-xm*.bin ${BOOT_BUILDDIR}/../u-boot-${soc}-nor.bin
    # NAND
    cat ${BOOT_SRCDIR}/openipc/openipc_config > ${BOOT_BUILDDIR}/.config && \
    cat ${BOOT_SRCDIR}/openipc/xm720xxx_config >> ${BOOT_BUILDDIR}/.config && \
    cat ${BOOT_SRCDIR}/openipc/nand_config >> ${BOOT_BUILDDIR}/.config && \
    echo "CONFIG_PRODUCT_SOC=\"${soc}\"" >> ${BOOT_BUILDDIR}/.config && \
    make ${MAKE_OPTS} olddefconfig </dev/null >/dev/null && \
    make ${MAKE_OPTS} BOOT_SRCDIR=${BOOT_SRCDIR} </dev/null && \
    make ${MAKE_OPTS} u-boot-z.bin </dev/null && \
    cp ${BOOT_BUILDDIR}/u-boot-xm*.bin ${BOOT_BUILDDIR}/../u-boot-${soc}-nand.bin
done

make distclean </dev/null >/dev/null 2>&1
