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

# SigmaStar infinity3: the vendor's IPL trains the DDR and loads IPL_CUST,
# which unpacks U-Boot from the MXP partition UBOOT to its link address.
# The boot partition holds all of it (openipc/infinity3-image.py).
build_infinity3() {
    soc=$1
    ipl=$2
    builddir=${BOOT_SRCDIR}/output/build-${soc}-nor
    make_opts="-C ${BOOT_SRCDIR} O=${builddir} -j$(nproc)"

    rm -rf "${builddir}"
    mkconfig "${builddir}" openipc/openipc_config openipc/${soc}_config && \
    make ${make_opts} olddefconfig </dev/null >/dev/null && \
    make ${make_opts} </dev/null || return 1

    python3 openipc/infinity3-image.py ${ipl} openipc/infinity3-ipl-cust.bin \
        "${builddir}/u-boot.bin" "${soc}" "output/u-boot-${soc}-nor.bin"
}

# SigmaStar infinity2m and infinity6*: the same IPL and IPL_CUST, the layout
# of OpenIPC's vendor U-Boot (openipc/sigmastar-image.py). All parts of a die
# share its stages; only U-Boot's Kconfig target differs.
build_die() {
    soc=$1
    die=$2
    tag=$3
    table=$4
    builddir=${BOOT_SRCDIR}/output/build-${soc}-nor
    make_opts="-C ${BOOT_SRCDIR} O=${builddir} -j$(nproc)"

    rm -rf "${builddir}"
    mkconfig "${builddir}" openipc/openipc_config openipc/${die}_config \
        openipc/${soc}_config && \
    make ${make_opts} olddefconfig </dev/null >/dev/null && \
    make ${make_opts} </dev/null || return 1

    python3 openipc/sigmastar-image.py --ipl openipc/${die}-ipl.bin \
        --ipl-cust openipc/${die}-ipl-cust.bin ${table} --tag "${tag}" \
        --uboot "${builddir}/u-boot.bin" --soc "${soc}" \
        --out "output/u-boot-${soc}-nor.bin"
}

# The same on SPI NAND: its own 2 MiB layout, SNI and PNI, the environment and
# firmware of OpenIPC's NAND images (openipc/nand_config).
build_nand() {
    soc=$1
    die=$2
    tag=$3
    builddir=${BOOT_SRCDIR}/output/build-${soc}-nand
    make_opts="-C ${BOOT_SRCDIR} O=${builddir} -j$(nproc)"

    rm -rf "${builddir}"
    mkconfig "${builddir}" openipc/openipc_config openipc/${die}_config \
        openipc/${soc}_config openipc/nand_config \
        $(ls openipc/${die}_nand_config 2>/dev/null) && \
    make ${make_opts} olddefconfig </dev/null >/dev/null && \
    make ${make_opts} </dev/null || return 1

    ipl=openipc/${die}-ipl.bin
    cust=openipc/${die}-ipl-cust.bin
    [ -f openipc/${die}-nand-ipl.bin ] && ipl=openipc/${die}-nand-ipl.bin
    [ -f openipc/${die}-nand-ipl-cust.bin ] && \
        cust=openipc/${die}-nand-ipl-cust.bin

    python3 openipc/sigmastar-image.py --nand --ipl ${ipl} --ipl-cust ${cust} \
        --gcis openipc/${die}-nand-sni.bin --pni openipc/${die}-nand-pni.bin \
        ${4} --tag "${tag}" --uboot "${builddir}/u-boot.bin" --soc "${soc}" \
        --out "output/u-boot-${soc}-nand.bin"
}

# SigmaStar Iford: the SDK's NOR boot.bin, U-Boot in place of its own
build_iford() {
    soc=$1
    builddir=${BOOT_SRCDIR}/output/build-${soc}-nor
    make_opts="-C ${BOOT_SRCDIR} O=${builddir} -j$(nproc)"

    rm -rf "${builddir}"
    mkconfig "${builddir}" openipc/openipc_config openipc/iford_config \
        openipc/${soc}_config && \
    make ${make_opts} olddefconfig </dev/null >/dev/null && \
    make ${make_opts} </dev/null || return 1

    python3 openipc/sigmastar-image.py --ipl openipc/iford-${soc}-ipl.bin \
        --ipl-cust openipc/iford-${soc}-ipl-cust.bin \
        --iford-cis openipc/iford-cis.bin --tag IFRD \
        --uboot "${builddir}/u-boot.bin" --soc "${soc}" \
        --out "output/u-boot-${soc}-nor.bin"
}

# SigmaStar Ifado: Dahua's NOR boot chain, U-Boot in place of its own
build_ifado() {
    soc=$1
    builddir=${BOOT_SRCDIR}/output/build-${soc}-nor
    make_opts="-C ${BOOT_SRCDIR} O=${builddir} -j$(nproc)"

    rm -rf "${builddir}"
    mkconfig "${builddir}" openipc/openipc_config openipc/ifado_config \
        openipc/${soc}_config && \
    make ${make_opts} olddefconfig </dev/null >/dev/null && \
    make ${make_opts} </dev/null || return 1

    python3 openipc/sigmastar-image.py --ipl openipc/ifado-ipl.bin \
        --ipl-cust openipc/ifado-ipl-cust.bin \
        --ifado-cis openipc/ifado-cis.bin --tag IFDO \
        --uboot "${builddir}/u-boot.bin" --soc "${soc}" \
        --out "output/u-boot-${soc}-nor.bin"
}

# SigmaStar Ifliegen: IPL, TF-A's SP_MIN, IPL_CUST, then U-Boot
build_ifliegen() {
    soc=$1
    builddir=${BOOT_SRCDIR}/output/build-${soc}-nor
    make_opts="-C ${BOOT_SRCDIR} O=${builddir} -j$(nproc)"

    rm -rf "${builddir}"
    mkconfig "${builddir}" openipc/openipc_config openipc/ifliegen_config \
        openipc/${soc}_config && \
    make ${make_opts} olddefconfig </dev/null >/dev/null && \
    make ${make_opts} </dev/null || return 1

    python3 openipc/sigmastar-image.py --ipl openipc/ifliegen-${soc}-ipl.bin \
        --ipl-cust openipc/ifliegen-${soc}-ipl-cust.bin \
        --ifliegen-tfa openipc/ifliegen-tf-a.bin \
        --ifliegen-nri openipc/ifliegen-flash.nri \
        --ifliegen-list openipc/ifliegen-flash-list.nri --tag IFLG \
        --uboot "${builddir}/u-boot.bin" --soc "${soc}" \
        --out "output/u-boot-${soc}-nor.bin"
}

build_iford_nand() {
    soc=$1
    builddir=${BOOT_SRCDIR}/output/build-${soc}-nand
    make_opts="-C ${BOOT_SRCDIR} O=${builddir} -j$(nproc)"

    rm -rf "${builddir}"
    mkconfig "${builddir}" openipc/openipc_config openipc/iford_config \
        openipc/${soc}_config openipc/nand_config openipc/iford_nand_config && \
    make ${make_opts} olddefconfig </dev/null >/dev/null && \
    make ${make_opts} </dev/null || return 1

    python3 openipc/sigmastar-image.py \
        --iford-nand-cis openipc/iford-${soc}-nand-cis.bin \
        --iford-nand-boot openipc/iford-${soc}-nand-boot.bin --tag IFRD \
        --uboot "${builddir}/u-boot.bin" --soc "${soc}" \
        --out "output/u-boot-${soc}-nand.bin"
}

build_soc() {
    soc=$1
    mxp="--mxp openipc/sigmastar-mxp.bin"
    case "${soc}" in
        msc313e)
            build_infinity3 "${soc}" openipc/infinity3-msc313e-ipl.bin ;;
        msc316dc|msc316dm)
            build_infinity3 "${soc}" openipc/infinity3-msc316-ipl.bin ;;
        ssd201|ssd202d|ssr621q)
            build_die "${soc}" infinity2m I2M "${mxp}" ;;
        ssc325|ssc325de|ssc327de)
            build_die "${soc}" infinity6 I6 "${mxp}" ;;
        ssc333|ssc335|ssc335de|ssc337|ssc337de)
            build_die "${soc}" infinity6b0 I6B0 "${mxp}" ;;
        ssc30kd|ssc30kq|ssc338q)
            build_die "${soc}" infinity6e I6E "${mxp}" ;;
        ssc377|ssc377d|ssc377de|ssc377qe|ssc378de|ssc378qe)
            build_die "${soc}" infinity6c I6C \
                "--gcis openipc/infinity6c-gcis.bin" ;;
        ssc029a|ssc029b|ssc029c)
            build_iford "${soc}" ;;
        ssc375)
            build_ifado "${soc}" ;;
        ssc034a|ssc034b)
            build_ifliegen "${soc}" ;;
        ssc029a-nand)
            build_iford_nand ssc029a ;;
        ssd201-nand|ssd202d-nand|ssr621q-nand)
            build_nand "${soc%-nand}" infinity2m I2M \
                "--pni-layout --nand-i2m --sni-part 2 --sni-list openipc/infinity2m-nand-sni-list.bin" ;;
        ssc377-nand|ssc377d-nand|ssc377de-nand|ssc377qe-nand|ssc378de-nand|ssc378qe-nand)
            build_nand "${soc%-nand}" infinity6c I6C \
                "--nand-6c --sni-list openipc/infinity6c-nand-sni-list.bin" ;;
        ssc325-nand|ssc325de-nand|ssc327de-nand)
            build_nand "${soc%-nand}" infinity6 I6 ;;
        ssc333-nand|ssc335-nand|ssc335de-nand|ssc337-nand|ssc337de-nand)
            build_nand "${soc%-nand}" infinity6b0 I6B0 ;;
        ssc30kd-nand|ssc30kq-nand|ssc338q-nand)
            build_nand "${soc%-nand}" infinity6e I6E ;;
        *)
            echo "unsupported SoC: ${soc}" >&2
            return 1 ;;
    esac
}

NOR_SOCS="msc313e msc316dc msc316dm ssd201 ssd202d ssr621q ssc325 ssc325de
    ssc327de ssc333 ssc335 ssc335de ssc337 ssc337de ssc30kd ssc30kq ssc338q
    ssc377 ssc377d ssc377de ssc377qe ssc378de ssc378qe ssc029a ssc029b
    ssc029c ssc375 ssc034a ssc034b"
NAND_SOCS="ssd201 ssd202d ssr621q ssc325 ssc325de ssc327de ssc333 ssc335
    ssc335de ssc337 ssc337de ssc30kd ssc30kq ssc338q ssc377 ssc377d ssc377de
    ssc377qe ssc378de ssc378qe"
ALL_SOCS="${NOR_SOCS} $(for s in ${NAND_SOCS}; do echo "${s}-nand"; done)
    ssc029a-nand"

SOCS="$@"
[ -z "${SOCS}" ] && SOCS="${ALL_SOCS}"

rc=0
for soc in ${SOCS}; do
    build_soc "${soc}" || { echo "build failed for ${soc}" >&2; rc=1; }
done

exit ${rc}
